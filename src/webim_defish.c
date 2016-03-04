#include <webim/standard.h>
#include <stdio.h>
#include <stdlib.h>
#include <wand/MagickWand.h>
#include <math.h>

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
        MagickWand* orig   = CloneMagickWand(mw);
        PixelIterator* pxi = NewPixelIterator(mw);
        {
            size_t wandCount     = 0;
            wiv_uint32 colOffset = 0;
            PixelWand** row      = NULL;

            do {
                row = PixelGetNextIteratorRow(pxi, &wandCount);

                // Compute new value here
                for (size_t rowOffset = 0; rowOffset < wandCount; ++rowOffset) {
                    PixelWand* pw = *(row + rowOffset);

                    double xd     = rowOffset - xcd;
                    double yd     = colOffset - ycd;
                    double rd     = hypot(xd, yd);
                    double phiang = atan(ofoc_inverse * rd);
                    double rr     = ifoc * phiang;
                    size_t xs     = (size_t) ((rd ? rr / rd : 0) * xd + xcd);
                    size_t ys     = (size_t) ((rd ? rr / rd : 0) * yd + ycd);

                    MagickGetImagePixelColor(orig, xs, ys, pw);
                }

                ++colOffset;
                PixelSyncIterator(pxi);
            } while (row != NULL);

            // Done!
            PixelSyncIterator(pxi);
        }
        DestroyMagickWand(orig);
        DestroyPixelIterator(pxi);
    }
}
