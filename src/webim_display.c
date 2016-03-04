#include <stdlib.h>
#include <webim/standard.h>
#include <webim/display.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

wiv_display*
wiv_init_display (void) 
{
    wiv_display* display = wiv_new(wiv_display);

    display->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    {
        gtk_window_set_title(display->window, "Web Image Viewer");
    }
    display->image = gtk_image_new();
    {
        gtk_container_add(GTK_CONTAINER (display->window), display->image);
    }
    
    display->currentImage = NULL;

    gtk_widget_show(display->window);
    gtk_widget_show(display->image);

    return display;
}

void
wiv_update_image (wiv_display* display, GdkPixbuf* pixBuf)
{
    gtk_image_set_from_pixbuf(display->image, pixBuf);
}
