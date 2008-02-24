#include <locale.h>
#include <gtk/gtk.h>
#include <xine.h>
#include "globals.h"
#include "lxine.h"
#include "gtkxine.h"
#include "callbacks.h"
#include "mixer.h"

void set_pixmap(GtkWidget *pixmap, gchar *filename)
{
    gchar *pathname;

    if (filename || filename[0]) {
        gchar *pathname = g_strdup_printf(PIXMAP_DIR "/%s", filename);
        gtk_image_set_from_file(pixmap, pathname);
        g_free(pathname); 
    }
}

void lxine_logo(void)
{
    gtk_xine_open(player->xine, "eplogo.png");
    gtk_xine_play(player->xine, 0, 0);

    if (player->mrl)
        gtk_xine_open(player->xine, player->mrl);
}

void lxine_open(gchar *mrl)
{
    gchar *pathname = g_strdup_printf(PIXMAP_DIR "/black.mpv");
    gtk_xine_open(GTK_XINE(player->xine), pathname);
    gtk_xine_open(GTK_XINE(player->xine), mrl);
    player->mrl = mrl;
    player->status = LXINE_STATUS_READY;
    set_pixmap (player->play_bi, "play.xpm");
}

void lxine_play(void)
{
    switch(player->status) {
        case LXINE_STATUS_READY:
           gtk_xine_play(player->xine, 0, 0);
           player->status = LXINE_STATUS_PLAY;
           set_pixmap (player->play_bi, "pause.xpm");
           break; 
        case LXINE_STATUS_PLAY:
           gtk_xine_set_param (player->xine, XINE_PARAM_SPEED, XINE_SPEED_PAUSE);
           player->status = LXINE_STATUS_PAUSE;
           set_pixmap (player->play_bi, "play.xpm");
           break;
        case LXINE_STATUS_PAUSE:
           gtk_xine_set_param (player->xine, XINE_PARAM_SPEED, XINE_SPEED_NORMAL);
           player->status = LXINE_STATUS_PLAY;
           set_pixmap (player->play_bi, "pause.xpm");
           break;
    }
}

void lxine_stop(void)
{
        gtk_xine_stop(GTK_XINE(player->xine));
        lxine_open(player->mrl);
}
