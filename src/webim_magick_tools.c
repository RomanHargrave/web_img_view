#include <webim/standard.h>
#include <wand/MagickWand.h>
#include <syscall.h>
#include <stdio.h>
#include <stdbool.h>

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gio/gio.h>

/**
 * memfd_create syscall frontend
 */
static inline int
wiv_memfd_create (char* name, wiv_uint32 flags)
{
    return syscall(SYS_memfd_create, name, flags);
}

/**
 * GDK Pixbuf destructor callback to destroy pixel data
 */
void
wiv_gdk_pixbuf_destroy_cb (wiv_uint8* pixelBuf, void* unused /* closureData */)
{
    wiv_free(pixelBuf);
}

/**
 * Allocates memory for an the image from ImageMagick, opens an FD pointing to that location, 
 * and writes to it.
 *
 * @return error, 0 if none
 */
int // err type
wiv_magick_write_pixbuf (MagickWand* wand, GdkPixbuf** pixBuf)
{

    /**
     * Q: What's going on here?
     * A: ImageMagick does not (natively) support writing the contents of a wand
     *    to anything other than a file descriptor.
     *    Since we don't want to write the remote image to disk every N seconds, 
     *    we should optimally store it in memory. In order to do this, we will write
     *    the image to a shared-memory object.
     */

    size_t imageLen = 0;

    char* shmName;
    {
        asprintf(&shmName, "webimg_magickhack_fd.%i", getpid());
    }

    int memFD   = wiv_memfd_create(shmName, 0x0);

    // Create a shared memory object
    if (memFD < 0) {
        fprintf(stderr, "Could not open memfd: syscall returned %i\n", memFD);
        return memFD; 
    }


    // Resize the shared memory object to match the size of the image
    ftruncate(memFD, imageLen);

    // Create a w+ stream for general purpose IO (working with raw FD's is clunky)
    FILE* memStream;
    {
        int tmpFD = dup(memFD);
        memStream = fdopen(tmpFD, "w+");

        if (memStream == NULL) {
            fprintf(stderr, "Could not open a stream to our memFD\n");
        }

    }

    // Write the image to the shared memory object & rewind
    {
        if (!MagickWriteImageFile(wand, memStream)) {
            fprintf(stderr, "Could not write image to memory\n");
            fclose(memStream);
            close(memFD);
            return 1;
        }
        imageLen = ftell(memStream);
        rewind(memStream);
    }

    {
        GInputStream* stream;
        wiv_uint8* streamContent;
        {
            streamContent = malloc(sizeof(wiv_uint8) * imageLen);
            fprintf(stderr, "streamContent = %p\n", streamContent);
            fread(streamContent, sizeof(wiv_uint8), imageLen, memStream);
            stream = g_memory_input_stream_new_from_data(streamContent, imageLen, NULL);
            fprintf(stderr, "streamContent = %p\n", streamContent);
        }

        GError* errorData    = NULL;
        *pixBuf              = gdk_pixbuf_new_from_stream(stream, NULL, &errorData);
        g_input_stream_close(stream, NULL, &errorData);
        wiv_free(streamContent);
    }

    fclose(memStream);
    close(memFD);

    wiv_free(shmName);

    return 0;
}
