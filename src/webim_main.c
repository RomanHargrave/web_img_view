#define _POSIX_C_SOURCE 199309L

#include <webim/standard.h>
#include <webim/config.h>
#include <webim/defish.h>
#include <webim/display.h>

#include <gtk/gtk.h>

#include <wand/MagickWand.h>
#include <magick/log.h>

#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>

static char*      HelpText             = \
    "Parameters:\n"
    "   -i  --image     - Image path\n"
    "Options:\n"
    "   -d              - Enable image dewarping (default: false)\n"
    "   -rN             - Image refresh rate in seconds (default: 5)\n"
    "   -xN --rx=N      - Resize the image on X to N\n"
    "   -yN --ry=N      - Resize the image on Y to N\n"
    "   If only one of Y or X is specified, the image will be scaled\n"
    "   to preserve the aspect ratio while resizing that edge to N\n"
    "\n"
    "   -f  --ifov=N    - Lens FOV for dewarping (default: 180)\n"
    "   -p  --ofov=N    - Projection FOV for dewarping (default: 120)\n";

typedef struct {
    webim_config*   appConfig;
    wiv_display*    display;
    time_t          lastCallTime;
} wiv_idle_data;

static inline time_t
wiv_current_time_sec (void)
{
    struct timespec time;
    clock_gettime(CLOCK_REALTIME, &time);
    return time.tv_sec;
}

static inline wiv_idle_data*
wiv_idle_data_new (webim_config* config, wiv_display* display)
{
    wiv_idle_data* data = wiv_new(wiv_idle_data);
    data->appConfig     = config;
    data->display       = display;
    data->lastCallTime  = 0;

    return data;
}

/**
 * Main loop implementation
 */
static gboolean
wiv_idle_event (gpointer data)
{
    wiv_idle_data* idleData = (wiv_idle_data*) data;

    {
        time_t now     = wiv_current_time_sec();
        time_t elapsed = now - idleData->lastCallTime;

        if (idleData->lastCallTime == 0 || elapsed >= idleData->appConfig->refreshRate) {
            idleData->lastCallTime = now;

            // Refresh step 1: Fetch & Scale image
            MagickWand* wand = NewMagickWand();
            {
                if (!MagickReadImage(wand, idleData->appConfig->imagePath)) {
                    fprintf(stderr, "could not read image\n");
                    return TRUE;
                }

                {
                    MagickSizeType sz;
                    MagickGetImageLength(wand, &sz);
                }

                wiv_uint32 resizeX = idleData->appConfig->resizeX;
                wiv_uint32 resizeY = idleData->appConfig->resizeY;

                if (resizeX > 0 || resizeY > 0) {
                    
                    if (resizeX == 0 || resizeY == 0) {
                        size_t dimX   = MagickGetImageWidth(wand);
                        size_t dimY   = MagickGetImageHeight(wand);
                        double aspect = ((double) dimX) / ((double) dimY);

                        if (resizeX == 0) {
                            resizeX = resizeY * aspect;
                        } else if (resizeY == 0) {
                            resizeY = resizeX / aspect;
                        }
                    }


                    MagickResizeImage(wand, resizeX, resizeY, LanczosFilter, 1);
                }
            }

            // Step 2: Defish
            if (idleData->appConfig->shouldDewarp) {
                webim_defish_image(
                        wand, 
                        idleData->appConfig->lensFOV, 
                        idleData->appConfig->projectionFOV
                        );
            }

            // Step 3: Set as active image in frame
            {
                GdkPixbuf* pixBuf = NULL;
                int err = wiv_magick_write_pixbuf(wand, &pixBuf);

                if (pixBuf) {
                    wiv_update_image(idleData->display, pixBuf);             
                } else {
                    fprintf(stderr, "write pb failed: %i\n", err);
                }
            }

            DestroyMagickWand(wand);
        }
    }

    return TRUE;
}

int
main (int argc, char** argv)
{
    if (argc == 1) {
        fprintf(stderr, HelpText);
        return 0;
    } 


    // Getopt config
    char* optionsShort = "dr:x:y:f:p:i:";

    struct option optionsLong[] = {
        /**
         * Resize on X to this size
         */
        {"rx",      required_argument, NULL, 'x'},

        /**
         * Resize on Y to this size
         */
        {"ry",      required_argument, NULL, 'y'},

        /**
         * Field-of-view of the lens the displayed image was taken with
         * Used when -d is specified.
         * Defaults to 180
         */
        {"ifov",    required_argument, NULL, 'f'},

        /**
         * Field-of-view that the image projection should have
         * Used when -d is specified         * Defaults to 120
         * Defaults to 120
         */
        {"ofov",    required_argument, NULL, 'p'},

        {"image",   required_argument, NULL, 'i'},
        {0, 0, 0, 0}
    };

    webim_config* config = webim_config_new();

    {
        int optionIndex = 0;
        while (true) {
            int c = getopt_long(argc, argv, optionsShort, optionsLong, &optionIndex);

            switch (c) {
                case 'd':
                    config->shouldDewarp = true;
                    break;
                case 'r':
                    config->refreshRate = atoi(optarg);
                    break;
                case 'x':
                    config->resizeX = atoi(optarg);
                    break;
                case 'y':
                    config->resizeY = atoi(optarg);
                    break;
                case 'f':
                    config->lensFOV = atoi(optarg);
                    break;
                case 'p':
                    config->projectionFOV = atoi(optarg);
                    break;
                case 'i':
                    config->imagePath = strdup(optarg);
                    break;
            };
            if (c == -1) break;
        };

        if (config->imagePath == NULL) {
            fprintf(stderr, "An image must be specified (-i --image)\n");
            return EINVAL;
        }

    };
    
    gtk_init(&argc, &argv);

    // Main loop
    {
        MagickWandGenesis();
        /* SetLogEventMask("Wand,Coder,Policy"); */

        // Create window
        wiv_display* disp       = wiv_init_display();
        wiv_idle_data* idleData = wiv_idle_data_new(config, disp);

        g_idle_add(&wiv_idle_event, idleData);

        gtk_main();

        MagickWandTerminus();
    }

    return 0;
}

