#include <stdio.h>
#include <gtk/gtk.h>

GtkWidget* main_win;
GtkWidget* image;
GdkPixbuf* pix;
GdkPixbuf* scaled_pix = NULL;
gint oldx = -1, oldy = -1;

static void on_btn_press( GtkWidget* evtbox,
                          GdkEventButton* evt,
                          gpointer user_data)
{
    GdkCursor* cursor = gdk_cursor_new( GDK_HAND2 );
    gtk_grab_add( evtbox);
    gdk_window_set_cursor( evtbox->window, cursor );
    gdk_window_get_pointer( gdk_get_default_root_window(),
                            &oldx, &oldy, NULL );
}

static void on_btn_release( GtkWidget* evtbox,
                            GdkEventButton* evt,
                            gpointer user_data)
{
    gdk_window_set_cursor( evtbox->window, NULL );
    gtk_grab_remove( evtbox);
}

static void on_motion_notify( GtkWidget* evtbox,
                              GdkEventMotion* evt,
                              gpointer user_data)
{
    GtkAdjustment *hadj, *vadj;
    GtkWidget *scroll;
    gint x, y, dx, dy;

    gdk_window_get_pointer( gdk_get_default_root_window(),
                            &x, &y, NULL );

    if( gtk_grab_get_current() != evtbox )
        return;

    scroll = (GtkWidget*)user_data;

    gdk_window_freeze_updates(evtbox->window);

    vadj = gtk_scrolled_window_get_vadjustment( scroll );
    hadj = gtk_scrolled_window_get_hadjustment( scroll );
    dy = oldy - y;
    dx = oldx - x;

    x = (int)gtk_adjustment_get_value(hadj) + dx;
    if( (x + hadj->page_size) > hadj->upper )
        x = (int)hadj->upper - hadj->page_size;
    y = (int)gtk_adjustment_get_value(vadj) + dy;
    if( (y + vadj->page_size) > vadj->upper )
        y = (int)vadj->upper - vadj->page_size;
    if( dy != 0 ) {
        gtk_adjustment_set_value( vadj, y );
    }
    if( dx != 0 ) {
        gtk_adjustment_set_value( hadj, x );
    }
    gdk_window_get_pointer( gdk_get_default_root_window(),
                            &x, &y, NULL );
    oldx = x;
    oldy = y;

    gdk_window_thaw_updates(evtbox->window);

}

void create_main_win()
{
    GtkWidget *scroll;
    GtkWidget *evtbox;
    main_win = gtk_window_new( GTK_WINDOW_TOPLEVEL );
    g_signal_connect( main_win, "delete-event",
                      G_CALLBACK(gtk_main_quit), NULL );
    scroll = gtk_scrolled_window_new( NULL, NULL );
    image = gtk_image_new_from_pixbuf( pix );
    evtbox = gtk_event_box_new();
    gtk_widget_add_events( evtbox,
                           GDK_POINTER_MOTION_HINT_MASK|
                           GDK_BUTTON_PRESS_MASK|
                           GDK_BUTTON_RELEASE_MASK );
    g_signal_connect( evtbox, "button-press-event",
                      G_CALLBACK( on_btn_press ), NULL );
    g_signal_connect( evtbox, "motion-notify-event",
                      G_CALLBACK( on_motion_notify ), scroll );
    g_signal_connect( evtbox, "button-release-event",
                      G_CALLBACK( on_btn_release ), NULL );
    gtk_container_add( evtbox, image );
    gtk_scrolled_window_add_with_viewport( scroll, evtbox );
    gtk_container_add( main_win, scroll );
}

int main( int argc, char* argv[] )
{
    GError* err = NULL;
    gtk_init( &argc, &argv );
    if( argc < 2 )
        return 1;

    pix = gdk_pixbuf_new_from_file( argv[1], &err );
    if( err ) {
        g_print("%s\n", err->message );
        g_error_free( err );
        return 1;
    }

    create_main_win();
    gtk_window_set_title( main_win, argv[1] );
    gtk_window_maximize( main_win );
    gtk_widget_show_all( main_win );

    gtk_main();
    g_object_unref( pix );
    return 0;
}
