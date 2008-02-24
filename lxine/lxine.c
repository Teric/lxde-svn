#include <stdio.h>
#include <locale.h>
#include <gtk/gtk.h>
#include <xine.h>
#include "lxine.h"
#include "callbacks.h"
#include "gtkxine.h"
#include "mixer.h"
#include "actions.h"

#define TARGET_STRING 0
#define TARGET_URL    1

static GtkTargetEntry target_table[] = {
  { "STRING",     0, TARGET_STRING },
  { "text/plain", 0, TARGET_STRING },
  { "text/uri-list", 0, TARGET_URL }
};

Lxine *player;
static gchar *video_driver_id=NULL;
static gchar *audio_driver_id=NULL;

Lxine *init_lxine(void)
{
    Lxine *new_win;

    new_win = g_malloc(sizeof(Lxine));
    new_win->status = LXINE_STATUS_NOMRL;

    return new_win;
}

static void xine_event_listener(void *user_data, const xine_event_t *event) {
    switch(event->type) {
    case XINE_EVENT_UI_PLAYBACK_FINISHED:
            lxine_stop();
            lxine_play();
        break;
/*
    case XINE_EVENT_PROGRESS:
        {
            xine_progress_data_t *pevent = (xine_progress_data_t *) event->data;
            printf("%s [%d%%]\n", pevent->description, pevent->percent);
            break;
        }
*/
    }
}

GtkWidget *create_pixmap(gchar *filename)
{
    gchar *pathname;
    GtkWidget *pixmap;

    if (!filename || !filename[0])
        return gtk_image_new ();
    else {
        gchar *pathname = g_strdup_printf(PIXMAP_DIR "/%s", filename);
        pixmap = gtk_image_new_from_file(pathname);
        g_free(pathname);
    }

    return pixmap;
}

GdkPixbuf *load_pixmap (const gchar *filename)
{
    gchar *pathname;
    GdkPixbuf *pixmap;

    if (!filename || !filename[0])
        return gtk_image_new ();
    else {
        gchar *pathname = g_strdup_printf(PIXMAP_DIR "/%s", filename);
        pixmap = gdk_pixbuf_new_from_file(pathname, NULL);
        g_free(pathname);
    }

    return pixmap;

}


GtkWidget *create_ctlbar(void)
{
    gint vol;
    GtkWidget *bar;
    /* button */
    GtkWidget *play_b;
    GtkWidget *stop_b;
    GtkWidget *play_i;
    GtkWidget *stop_i;
    /* volume */
    GtkWidget *volbox;
    GtkWidget *volctl;
    GtkWidget *volimg;

    /* create button box */
    bar = gtk_fixed_new();
    gtk_widget_show (bar);

    /* create buttons */

    /* play button */
    play_b = gtk_button_new ();
    gtk_widget_show (play_b);
    gtk_fixed_put (GTK_FIXED (bar), play_b, 0, 2);
    gtk_widget_set_size_request (play_b, 40, 40);
    gtk_button_set_relief (GTK_BUTTON (play_b), GTK_RELIEF_HALF);
    g_signal_connect (G_OBJECT (play_b), "clicked",
              G_CALLBACK (play_clicked), (gpointer) "cool button");

    play_i = create_pixmap ("play.xpm");
    gtk_widget_show (play_i);
    gtk_container_add (GTK_CONTAINER (play_b), play_i);
    player->play_bi = play_i;

    /* stop button */
    stop_b = gtk_button_new ();
    gtk_widget_show (stop_b);
    gtk_fixed_put (GTK_FIXED (bar), stop_b, 40, 2);
    gtk_widget_set_size_request (stop_b, 40, 40);
    gtk_button_set_relief (GTK_BUTTON (stop_b), GTK_RELIEF_HALF);
    g_signal_connect (G_OBJECT (stop_b), "clicked",
              G_CALLBACK (stop_clicked), (gpointer) "cool button");

    stop_i = create_pixmap ("stop.xpm");
    gtk_widget_show (stop_i);
    gtk_container_add (GTK_CONTAINER (stop_b), stop_i);

    /* create volume controller */
    /* volume control */
    mixer_get("vol", &vol);
    volctl = gtk_hscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (vol, 0, 100, 0, 0, 0)));
    gtk_widget_show (volctl);
    gtk_fixed_put (GTK_FIXED (bar), volctl, 135, 12);
    gtk_widget_set_size_request (volctl, 100, 16);
    gtk_scale_set_draw_value (GTK_SCALE (volctl), FALSE);
    gtk_scale_set_digits (GTK_SCALE (volctl), 0);
    g_signal_connect ((gpointer) volctl, "value_changed",
                      G_CALLBACK (on_vol_value_changed),
                      NULL);

    /* volume control image */
    volimg = create_pixmap ("speaker.xpm");
    gtk_widget_show (volimg);
    gtk_fixed_put (GTK_FIXED (bar), volimg, 100, 0);
    gtk_widget_set_size_request (volimg, 35, 40);

    return bar;
}

gint create_window(void)
{
    GtkWidget *widgetbox;
    GtkWidget *gtkxine;
//    GtkWidget *silder;
    GtkWidget *bar;

    player->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(player->window), _("Lxine Media Player"));
    gtk_window_set_icon(GTK_WINDOW(player->window), load_pixmap("lxine.png"));

    /* setting that when get exit signal, call delete_event() */
    g_signal_connect (G_OBJECT (player->window), "delete_event",
                      G_CALLBACK (delete_event), NULL);
    g_signal_connect (G_OBJECT (player->window), "destroy",
                      G_CALLBACK (destroy), NULL);

    /* create widgetbox */
    widgetbox = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (widgetbox);
    gtk_container_add (GTK_CONTAINER (player->window), widgetbox);

    /* create xine */
    gtkxine = gtk_xine_new (video_driver_id, audio_driver_id);
    gtk_widget_show (gtkxine);
    gtk_box_pack_start (GTK_BOX(widgetbox), gtkxine, TRUE, TRUE, 0);
    gtk_drag_dest_set (gtkxine, GTK_DEST_DEFAULT_DROP,
                       target_table, 3,
                       GDK_ACTION_LINK | GDK_ACTION_COPY | GDK_ACTION_MOVE);
    player->xine = gtkxine;

    /* create seeker */
/*    silder = gtk_hscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (0.0, 0.0, 65536.0, 1.0, 10.0, 10.0)));
    gtk_widget_show(silder);
    gtk_box_pack_start (GTK_BOX (widgetbox), silder, FALSE, FALSE, 0);
    gtk_scale_set_draw_value (GTK_SCALE(silder), FALSE);
    g_signal_connect( GTK_OBJECT (silder), "value-changed",
                      G_CALLBACK (on_seeker_value_changed), NULL );
    player->silder = silder;
*/
    /* create control bar */
    bar = create_ctlbar();
    gtk_box_pack_start (GTK_BOX (widgetbox), bar, FALSE, FALSE, 0);

    gtk_widget_show(player->window);
}

gint main(int argc, char **argv)
{
    gint i;
    char *mrl = NULL;

    g_thread_init(NULL);
    gdk_threads_init();

    /* Init GTK */
    gtk_set_locale();
    gtk_init(&argc, &argv);

    player = init_lxine();
    init_mixer();
    create_window();
    //gtk_timeout_add (1000, update_slider_cb, NULL);

    /* event handling */
    {
      xine_event_queue_t *queue;

      queue = xine_event_new_queue (GTK_XINE(player->xine)->stream);

      xine_event_create_listener_thread (queue, xine_event_listener, NULL);
    }

    gtk_xine_set_param (player->xine, XINE_PARAM_VO_DEINTERLACE, 1);
    gtk_xine_set_vis (player->xine, g_strdup ("goom"));

    /* parsing command line */
    for (i = 1; i < argc; i++) {
        mrl = argv[i];
    }

    if (mrl) {
        lxine_open(mrl);
        lxine_play();
    }// else
//        lxine_logo();

    /* main */
    gtk_main ();

    return 0;
}
