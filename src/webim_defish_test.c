#include <webim/standard.h>
#include <webim/defish.h>
#include <stdio.h>

int 
main (int argc, char** argv) 
{
    ++argv;
    if (argc < 3) {
        fputs("Provide an input and output image\n", stderr);
        return 1;
    }

    char* input  = *argv++;
    char* output = *argv++;

    MagickWandGenesis();

    MagickWand* mw = NewMagickWand();

    MagickReadImage(mw, input);

    webim_defish_image(mw, 180.0, 120.0);

    MagickWriteImage(mw, output);

    DestroyMagickWand(mw);

    MagickWandTerminus();

    return 0;
}
