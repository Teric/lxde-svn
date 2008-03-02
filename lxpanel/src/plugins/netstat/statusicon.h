#ifndef HAVE_NS_STATUSICON_H
#define HAVE_NS_STATUSICON_H

#include "fnetdaemon.h"
#include <gtk/gtk.h>

struct statusicon {
        GtkWidget *main;
        GtkWidget *icon;
        GtkTooltips *tooltips;
};

struct statusicon *create_statusicon(GtkWidget *box, const char *filename,
		const char *tooltips);
void statusicon_destroy(struct statusicon *icon);
void set_statusicon_image_from_file(struct statusicon *widget, const char *filename);
void set_statusicon_tooltips(struct statusicon *widget, const char *tooltips);
void set_statusicon_visible(struct statusicon *widget, gboolean b);

#endif
