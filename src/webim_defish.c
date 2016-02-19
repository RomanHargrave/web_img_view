#include <webim/standard.h>
#include <stdio.h>
#include <stdlib.h>
#include <wand/MagickWand.h>
#include <math.h>

// This whole mess was transcribed from a bash script
#define _info(str) fputs(str "\n", stderr)

static inline wiv_uint32
pix_get_rgb(PixelWand* pw)
{
    wiv_uint16 red   = (wiv_uint16)(PixelGetRed(pw) * 255);
    wiv_uint16 green = (wiv_uint16)(PixelGetGreen(pw) * 255);
    wiv_uint16 blue  = (wiv_uint16)(PixelGetBlue(pw) * 255);
    
    return (red << 16) | (green << 8) | blue;
}

static inline void
pix_set_rgb(PixelWand* pw, wiv_uint32 rgb)
{
    double red   = (rgb >> 16) / 255.0;
    double green = ((rgb >> 8) & 0x00FF) / 255.0;
    double blue  = (rgb & 0x0000FF) / 255.0;

    PixelSetRed(pw, red);
    PixelSetGreen(pw, green);
    PixelSetBlue(pw, blue);
}

void 
webim_defish_image ( MagickWand* mw, double fovIn, double fovOut )
{
    // Get the half-size
    double lesserEdge;
    double xcd, ycd;
    double dimX, dimY;
    {
        dimX       = (double) MagickGetImageWidth(mw);
        dimY       = (double) MagickGetImageHeight(mw);
        lesserEdge = fminf(dimX, dimY);
        xcd        = (dimX - 1) / 2.0;
        ycd        = (dimY - 1) / 2.0;
    }

    // Compute focal angles
    double ofoc, ofoc_inverse, ifoc;
    {
        ofoc         = lesserEdge / (2.0 * tan(fovOut * M_PI / 360.0));
        ofoc_inverse = 1.0 / ofoc;
        ifoc         = (lesserEdge * 180.0) / (fovIn * M_PI);
    }

    // Process image by-pixel
    {
        PixelIterator* pxi = NewPixelIterator(mw);
        {
            size_t wandCount     = 0;
            wiv_uint32 colOffset = 0;
            PixelWand** row      = NULL;
            size_t pixMatrix[(size_t) dimY][(size_t) dimX];

            // Pass 1: Create a matrix representing the colour value at the corresponding pixel
            //         we need to refer back to this during the transform pass (pass 2), and
            //         doing this is a small memory sacrifice to increase performance, as we don't
            //         have to seek to the correct PixelWand for each pixel
            do {
                row = PixelGetNextIteratorRow(pxi, &wandCount);

                // Get color count
                for (size_t rowOffset = 0; rowOffset < wandCount; ++rowOffset) {
                    PixelWand* pw = *(row + rowOffset);

                    pixMatrix[colOffset][rowOffset] = pix_get_rgb(pw);
                }

                ++colOffset;
                PixelSyncIterator(pxi);
            } while (row != NULL);

            // Pass 2: Compute dewarp movement per-pixel and recolor based on that
            
            colOffset = 0;
            row       = NULL;
            PixelResetIterator(pxi);

            do {
                row = PixelGetNextIteratorRow(pxi, &wandCount);

                // Compute new value here
                for (size_t rowOffset = 0; rowOffset < wandCount; ++rowOffset) {
                    PixelWand* pw = *(row + rowOffset);

                    double xd     = rowOffset - xcd;
                    double yd     = colOffset - ycd;
                    double rd     = hypot(xd, yd);
                    double phiang = atan(ofoc_inverse * rd);
                    double rr     = fovIn * phiang;
                    size_t xs     = (size_t) ((rd ? rr / rd : 0) * xd + xcd);
                    size_t ys     = (size_t) ((rd ? rr / rd : 0) * yd + ycd);

                    pix_set_rgb(pw, pixMatrix[ys][xs]);
                }

                ++colOffset;
                PixelSyncIterator(pxi);
            } while (row != NULL);

            // Done!
            PixelSyncIterator(pxi);
            DestroyPixelIterator(pxi);
        }
    }
}
