#pragma once
#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

typedef struct {
    GtkWidget* window;
    GtkWidget* image;
    GdkPixbuf* currentImage;
} wiv_display;

wiv_display* wiv_init_display (void);

void wiv_update_image (wiv_display*, GdkPixbuf*);

