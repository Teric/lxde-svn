#ifndef HAVE_LXINE_H
#define HAVE_LXINE_H

#define PACKAGE "lxine"
#define LOCALEDIR "/home/fred/svn/lxde/trunk/lxine/mo"
#define _(STRING) gettext(STRING)

#define PIXMAP_DIR "/usr/local/share/lxine/pixmaps"

#define LXINE_STATUS_NOMRL  0
#define LXINE_STATUS_READY  1
#define LXINE_STATUS_PLAY   2
#define LXINE_STATUS_PAUSE  3
#define LXINE_STATUS_STOP   4

typedef struct {
    GtkWindow *window;
    GtkWidget *xine;

    GtkWidget *play_bi;

    gint status;
    gchar *mrl;
} Lxine;

#endif
