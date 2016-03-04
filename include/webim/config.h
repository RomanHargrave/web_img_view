#include <webim/standard.h>
#include <stdbool.h>
#include <stdlib.h>

static wiv_uint16 DefaultRefresh       = 5;
static wiv_uint32 DefaultXDim          = 0;
static wiv_uint32 DefaultYDim          = 0;
static wiv_uint16 DefaultLensFOV       = 180;
static wiv_uint16 DefaultProjectionFOV = 120;
static bool       DefaultDewarp        = false;

typedef struct {
    
    /**
     * Path to image
     */
    wiv_str     imagePath;

    /**
     * Refresh rate in seconds
     */
    wiv_uint16  refreshRate;

    /**
     * Defish?
     */
    bool        shouldDewarp;

    /**
     * Field of view angle for defishing
     */
    wiv_uint16  lensFOV;

    /**
     * Projection fov angle
     */
    wiv_uint16  projectionFOV;

    wiv_uint32  resizeX;

    wiv_uint32  resizeY;

} webim_config;

static inline webim_config*
webim_config_new (void)
{
    webim_config* config  = wiv_new(webim_config);
    config->imagePath     = NULL;
    config->refreshRate   = DefaultRefresh;
    config->shouldDewarp  = DefaultDewarp;
    config->lensFOV       = DefaultLensFOV;
    config->projectionFOV = DefaultProjectionFOV;
    config->resizeX       = DefaultXDim;
    config->resizeY       = DefaultYDim;
    return config;
}
