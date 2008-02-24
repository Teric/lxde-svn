#include <locale.h>
#include <gtk/gtk.h>
#include "globals.h"
#include "lxine.h"
#include "gtkxine.h"
#include "callbacks.h"
#include "mixer.h"
#include "actions.h"

void on_seeker_value_changed (GtkRange *range, gpointer user_data)
{
//    if (gtk_xine_get_stream_info (GTK_XINE (player->xine), XINE_STREAM_INFO_SEEKABLE))
//        gtk_xine_play (GTK_XINE (player->xine), (gint) gtk_range_get_value(range), 0);
}

void on_vol_value_changed (GtkRange *range, gpointer user_data)
{
    mixer_set("pcm", gtk_range_get_value(range));
    mixer_set("vol", gtk_range_get_value(range));
}

void play_clicked(GtkWidget *widget, gpointer data)
{
    lxine_play();
}

void stop_clicked(GtkWidget *widget, gpointer data)
{
    lxine_stop();
}

gint delete_event(GtkWidget *widget, GdkEvent *event, gpointer data)
{
    /* return FALSE then to call destroy to exit */
    return FALSE;
}

void destroy(GtkWidget *widget, gpointer data)
{
    uninit_mixer();
    gtk_main_quit ();
}
