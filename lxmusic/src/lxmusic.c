/*
 *      lxmusic.c
 *      
 *      Copyright 2008 PCMan <pcman.tw@gmail.com>
 *      Copyright 2009 Jürgen Hötzel <juergen@archlinux.org>
 *      
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *      
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *      
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *      MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmsclient/xmmsclient.h>
#include <xmmsclient/xmmsclient-glib.h>

/* for status icon */
#include <X11/Xlib.h>
#include <gdk/gdkx.h>

#include "utils.h"

enum {
    COL_ID = 0,
    COL_ARTIST,
    COL_ALBUM,
    COL_TITLE,
    COL_LEN,
    COL_WEIGHT, /* font weight, used to show bold font for current track. */
    N_COLS
};

enum {
    REPEAT_NONE,
    REPEAT_CURRENT,
    REPEAT_ALL
};

enum {
    FILTER_ALL,
    FILTER_TITLE,
    FILTER_ARTIST,
    FILTER_ALBUM
};

typedef struct _UpdateTrack{
    uint32_t id;
    GtkTreeIter it;
}UpdateTrack;


const char* known_plugins[] = {
    "pulse",
    "alsa",
    "ao",
/*    "oss" */
};

static xmmsc_connection_t *con = NULL;
static GtkWidget *main_win = NULL;
static GtkWidget *tray_icon = NULL;
static GtkWidget *play_btn = NULL;
static GtkWidget *time_label = NULL;
static GtkWidget *progress_bar = NULL;
static GtkWidget *status_bar = NULL;
static GtkWidget *notebook = NULL;
static GtkWidget *volume_btn = NULL;
static GtkWidget *inner_vbox = NULL;
static GtkWidget *playlist_view = NULL;
static GtkWidget *repeat_mode_cb = NULL;

static GtkWidget *switch_pl_menu = NULL;
static GSList* switch_pl_menu_group = NULL;

static GtkWidget *add_to_pl_menu = NULL;
static GtkWidget *rm_from_pl_menu = NULL;

static char* cur_playlist = NULL;
static GSList* all_playlists = NULL;

static GQueue* pending_update_tracks = NULL;
static GtkListStore* list_store = NULL;

static int32_t playback_status = 0;
static uint32_t play_time = 0;
static uint32_t cur_track_duration = 0;
static int32_t cur_track_id = 0;
static GtkTreeIter cur_track_iter = {0};

static int repeat_mode = REPEAT_NONE;

static char* filter_keyword = NULL;
static uint32_t filter_timeout = 0;

/* config values */
static gboolean show_tray_icon = TRUE;
static gboolean show_playlist = TRUE;
static gboolean close_to_tray = TRUE;
static gboolean play_after_exit = FALSE;

static int filter_field = FILTER_ALL;
static uint32_t volume = 60;

/* window size */
static int win_width = 480;
static int win_height = 320;

/* window position */
static int win_xpos = 0;
static int win_ypos = 0;

void on_play_btn_clicked(GtkButton* btn, gpointer user_data);






static void load_config()
{
    char* path = g_build_filename(g_get_user_config_dir(), "lxmusic", "config", NULL );
    GKeyFile* kf = g_key_file_new();
    if( g_key_file_load_from_file(kf, path, 0, NULL) )
    {
        int v;
        const char grp[] = "Main";
        v = g_key_file_get_integer(kf, grp, "width", NULL);
        if( v > 0 )
            win_width = v;
        v = g_key_file_get_integer(kf, grp, "height", NULL);
        if( v > 0 )
            win_height = v;
		win_xpos = g_key_file_get_integer(kf, grp, "xpos", NULL);
		win_ypos = g_key_file_get_integer(kf, grp, "ypos", NULL);
        kf_get_bool(kf, grp, "show_tray_icon", &show_tray_icon);
        kf_get_bool(kf, grp, "show_playlist", &show_playlist);
        kf_get_bool(kf, grp, "close_to_tray", &close_to_tray);
        kf_get_bool(kf, grp, "play_after_exit", &play_after_exit);
        kf_get_int(kf, grp, "volume", &volume);
        kf_get_int(kf, grp, "filter", &filter_field);
    }
    g_free(path);
    g_key_file_free(kf);
}

static void save_config()
{
    FILE* f;
    char* dir = g_build_filename(g_get_user_config_dir(), "lxmusic", NULL);
    char* path = g_build_filename(dir, "config", NULL );

    g_mkdir_with_parents(dir, 0700);
    g_free( dir );

    f = fopen(path, "w");
    g_free(path);
    if( f )
    {
        fprintf(f, "[Main]\n");
        fprintf( f, "width=%d\n", win_width );
        fprintf( f, "height=%d\n", win_height );
        fprintf( f, "xpos=%d\n", win_xpos );
        fprintf( f, "ypos=%d\n", win_ypos );
        fprintf( f, "show_tray_icon=%d\n", show_tray_icon );
        fprintf( f, "close_to_tray=%d\n", close_to_tray );
        fprintf( f, "play_after_exit=%d\n", play_after_exit );
        fprintf( f, "show_playlist=%d\n", show_playlist );
        fprintf( f, "filter=%d\n", filter_field );
        fprintf( f, "volume=%d\n", (int)volume );
        fclose(f);
    }
}

static void free_update_track( UpdateTrack* ut )
{
    g_slice_free(UpdateTrack, ut);
}

static void cancel_pending_update_tracks()
{
    /* g_debug("try to cancel"); */
    if( ! g_queue_is_empty(pending_update_tracks) )
    {
        g_queue_foreach(pending_update_tracks, (GFunc)free_update_track, NULL);
        g_queue_clear(pending_update_tracks);
    }
}

static int on_xmms_quit(xmmsv_t *value, void *user_data)
{
    gtk_widget_destroy(main_win);
    gtk_main_quit();
    return TRUE;
}

void on_quit(GtkAction* act, gpointer user_data)
{
    cancel_pending_update_tracks();

    if( show_playlist )
        gtk_window_get_size(GTK_WINDOW(main_win), &win_width, &win_height);

    if(tray_icon)
        g_object_unref(tray_icon);

    /* FIXME: Is this apporpriate? */
    if( ! play_after_exit || playback_status == XMMS_PLAYBACK_STATUS_STOP )
    {
        /* quit the server */
        xmmsc_result_t* res = xmmsc_quit(con);
        xmmsc_result_notifier_set(res, on_xmms_quit, NULL);
        xmmsc_result_unref(res);
    }
    else
    {
        gtk_widget_destroy(main_win);
        gtk_main_quit();
    }
}

gboolean on_main_win_delete_event(GtkWidget* win, GdkEvent* evt, gpointer user_data)
{
    if( close_to_tray )
    {
        gtk_widget_hide( win );
        return TRUE;
    }
    else
        on_quit(NULL, NULL);
    return FALSE;
}

void on_main_win_destroy(GtkWidget* win)
{
    if( filter_timeout )
    {
        g_source_remove(filter_timeout);
        filter_timeout = 0;
    }
}

static void open_url(GtkAboutDialog* dlg, const char* url, gpointer user_data)
{
    const char* argv[] = {"xdg-open", NULL, NULL};
    argv[1] = url;
    g_spawn_async("/", (gchar**)argv, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, NULL, NULL);
}

void on_about(GtkWidget* mi, gpointer data)
{
    const char* authors[] = { "洪任諭 (Hong Jen Yee) <pcman.tw@gmail.com>", 
			      "Jürgen Hötzel <juergen@archlinux.org>", NULL };
    const char* artists[] = { N_("Official icon of xmms2 by Arnaud DIDRY"), NULL };
    GtkWidget* about;

    gtk_about_dialog_set_url_hook(open_url, NULL, NULL);

    about = gtk_about_dialog_new();
    gtk_about_dialog_set_name( (GtkAboutDialog*)about, "LXMusic" );
    gtk_about_dialog_set_logo_icon_name((GtkAboutDialog*)about, "lxmusic");
    gtk_about_dialog_set_version( (GtkAboutDialog*)about, VERSION );
    gtk_about_dialog_set_authors( (GtkAboutDialog*)about, authors );
    gtk_about_dialog_set_artists( (GtkAboutDialog*)about, artists );
    gtk_about_dialog_set_comments( (GtkAboutDialog*)about, _("Music Player for LXDE\nSimple GUI XMMS2 client") );
    gtk_about_dialog_set_license( (GtkAboutDialog*)about, "GNU General Public License" );
    gtk_about_dialog_set_website( (GtkAboutDialog*)about, "http://lxde.org/" );
    gtk_window_set_transient_for( (GtkWindow*)about, (GtkWindow*)main_win );
    gtk_dialog_run( (GtkDialog*)about );
    gtk_widget_destroy( about );
}

void on_pref_output_plugin_changed(GtkNotebook* nb, GtkComboBox* output)
{
    int i = gtk_combo_box_get_active(output);
    if( i >= 0 )
        gtk_notebook_set_current_page(nb, i);
}

static int on_pref_dlg_init_widget(xmmsv_t* value, void* user_data)
{
    GtkWidget* w = (GtkWidget*)user_data;
    char* val;
    if( xmmsv_get_string(value, (const char**)&val) )
    {
        /* g_debug("val = %s, w is a %s", val, G_OBJECT_TYPE_NAME(w)); */
        if( GTK_IS_SPIN_BUTTON(w) )
            gtk_spin_button_set_value(GTK_SPIN_BUTTON(w), atoi(val));
        else if( GTK_IS_ENTRY(w) )
            gtk_entry_set_text(GTK_ENTRY(w), val);
        else if( GTK_IS_COMBO_BOX_ENTRY(w) )
            gtk_entry_set_text(GTK_ENTRY(gtk_bin_get_child(GTK_BIN(w))), val);
        else
            g_debug("%s is not supported", G_OBJECT_TYPE_NAME(w));
    }
    return FALSE;
}

int on_pref_dlg_init_output_plugin(xmmsv_t* value, void* user_data)
{
    GtkWidget* w = (GtkWidget*)user_data;
    char* val;
    if( xmmsv_get_string(value, (const char**)&val) )
    {
        int i;
        for( i=0; i < G_N_ELEMENTS(known_plugins); ++i )
        {
            if( strcmp(val, known_plugins[i]) == 0 )
            {
                gtk_combo_box_set_active(GTK_COMBO_BOX(w), i);
                break;
            }
        }
    }
    return FALSE;
}

static void on_tray_icon_activate(GtkStatusIcon* icon, gpointer user_data)
{
    /* FIXME: should we unload the playlist to free resources here? */
    if( GTK_WIDGET_VISIBLE(main_win) )
    {
		/* save window position before we hide the window */
		gtk_window_get_position((GtkWindow*)main_win, &win_xpos, &win_ypos);
		save_config();
		gtk_widget_hide(main_win);
	}
    else
	{
        /* restore the window position */
		gtk_window_move((GtkWindow*)main_win, win_xpos, win_ypos);
		gtk_widget_show(main_win);
	}
}

void on_show_main_win(GtkAction* act, gpointer user_data)
{
	gtk_window_present(GTK_WINDOW(main_win));
}

static void on_tray_icon_popup_menu(GtkStatusIcon* icon, guint btn, guint time, gpointer user_data)
{
	/* init tray icon widgets */
	GtkBuilder *builder = gtk_builder_new ();
	if(gtk_builder_add_from_file(builder, PACKAGE_DATA_DIR "/lxmusic/popup.ui.glade", NULL))
	{
        	GtkWidget *tray_play_btn = (GtkWidget*)gtk_builder_get_object(builder, "play");
		GtkWidget *tray_popup = (GtkWidget*)gtk_builder_get_object(builder, "popup");
        	gtk_builder_connect_signals(builder, NULL);
		switch (playback_status)
		{
			case XMMS_PLAYBACK_STATUS_PLAY:
				g_object_set ( (GObject*)tray_play_btn, "stock-id", "gtk-media-pause", NULL);
				break;
			case XMMS_PLAYBACK_STATUS_PAUSE:
			case XMMS_PLAYBACK_STATUS_STOP:
				g_object_set ( (GObject*)tray_play_btn, "stock-id", "gtk-media-play", NULL);
				break;
		}
		gtk_menu_popup((GtkMenu*)tray_popup, NULL, NULL, NULL, NULL, btn, time);
	}
	g_object_unref(builder);

	return;
}

static void create_tray_icon()
{
    tray_icon = (GtkWidget*)gtk_status_icon_new_from_icon_name("lxmusic");
    gtk_status_icon_set_tooltip(GTK_STATUS_ICON(tray_icon), _("LXMusic"));
    g_signal_connect(tray_icon, "activate", G_CALLBACK(on_tray_icon_activate), NULL );
    g_signal_connect(tray_icon, "popup-menu", G_CALLBACK(on_tray_icon_popup_menu), NULL );
}

void on_preference(GtkAction* act, gpointer data)
{
    GtkBuilder* builder = gtk_builder_new();
    if( gtk_builder_add_from_file(builder, PACKAGE_DATA_DIR "/lxmusic/pref-dlg.ui.glade", NULL ) )
    {
        xmmsc_result_t* res;
        GtkWidget* show_tray_icon_btn = (GtkWidget*)gtk_builder_get_object(builder, "show_tray_icon");
        GtkWidget* close_to_tray_btn = (GtkWidget*)gtk_builder_get_object(builder, "close_to_tray");
        GtkWidget* play_after_exit_btn = (GtkWidget*)gtk_builder_get_object(builder, "play_after_exit");
        GtkWidget* output_plugin_cb = (GtkWidget*)gtk_builder_get_object(builder, "output_plugin_cb");
        GtkWidget* output_bufsize = (GtkWidget*)gtk_builder_get_object(builder, "output_bufsize");
        GtkWidget* alsa_device = (GtkWidget*)gtk_builder_get_object(builder, "alsa_device");
        GtkWidget* alsa_mixer = (GtkWidget*)gtk_builder_get_object(builder, "alsa_mixer");
        GtkWidget* ao_device = (GtkWidget*)gtk_builder_get_object(builder, "ao_device");
        GtkWidget* ao_driver = (GtkWidget*)gtk_builder_get_object(builder, "ao_driver");
        GtkWidget* cdrom = (GtkWidget*)gtk_builder_get_object(builder, "cdrom");
        GtkWidget* id3v1_encoding = (GtkWidget*)gtk_builder_get_object(builder, "id3v1_encoding");
        GtkWidget* dlg = (GtkWidget*)gtk_builder_get_object(builder, "pref_dlg");

        gtk_builder_connect_signals(builder, NULL);

        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(show_tray_icon_btn), show_tray_icon);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(close_to_tray_btn), close_to_tray);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(play_after_exit_btn), play_after_exit);

        res = xmmsc_configval_get(con, "output.plugin");
        xmmsc_result_notifier_set_full(res, on_pref_dlg_init_output_plugin, g_object_ref(output_plugin_cb), g_object_unref );
        xmmsc_result_unref(res);

        res = xmmsc_configval_get(con, "output.buffersize");
        xmmsc_result_notifier_set_full(res, on_pref_dlg_init_widget, g_object_ref(output_bufsize), g_object_unref );
        xmmsc_result_unref(res);

        res = xmmsc_configval_get(con, "alsa.device");
        xmmsc_result_notifier_set_full(res, on_pref_dlg_init_widget, g_object_ref(alsa_device), g_object_unref );
        xmmsc_result_unref(res);

        res = xmmsc_configval_get(con, "alsa.mixer");
        xmmsc_result_notifier_set_full(res, on_pref_dlg_init_widget, g_object_ref(alsa_mixer), g_object_unref );
        xmmsc_result_unref(res);

        res = xmmsc_configval_get(con, "ao.device");
        xmmsc_result_notifier_set_full(res, on_pref_dlg_init_widget, g_object_ref(ao_device), g_object_unref );
        xmmsc_result_unref(res);

        res = xmmsc_configval_get(con, "ao.driver");
        xmmsc_result_notifier_set_full(res, on_pref_dlg_init_widget, g_object_ref(ao_driver), g_object_unref );
        xmmsc_result_unref(res);

        res = xmmsc_configval_get(con, "cdda.device");
        xmmsc_result_notifier_set_full(res, on_pref_dlg_init_widget, g_object_ref(cdrom), g_object_unref );
        xmmsc_result_unref(res);

        res = xmmsc_configval_get(con, "mad.id3v1_encoding");
        xmmsc_result_notifier_set_full(res, on_pref_dlg_init_widget, g_object_ref(id3v1_encoding), g_object_unref );
        xmmsc_result_unref(res);

        if( gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_OK )
        {
            int i;
            char str[32];

            i = gtk_combo_box_get_active(GTK_COMBO_BOX(output_plugin_cb));
            res = xmmsc_configval_set( con, "output.plugin", known_plugins[i] );
            xmmsc_result_unref(res);

            g_snprintf(str, 32, "%u",(uint32_t)gtk_spin_button_get_value(GTK_SPIN_BUTTON(output_bufsize)));
            res = xmmsc_configval_set( con, "output.buffersize", str );
            xmmsc_result_unref(res);

            res = xmmsc_configval_set( con, "alsa.device", gtk_entry_get_text(GTK_ENTRY(alsa_device)) );
            xmmsc_result_unref(res);

            res = xmmsc_configval_set( con, "alsa.mixer", gtk_entry_get_text(GTK_ENTRY(alsa_mixer)) );
            xmmsc_result_unref(res);

            res = xmmsc_configval_set( con, "ao.device", gtk_entry_get_text(GTK_ENTRY(ao_device)) );
            xmmsc_result_unref(res);

            res = xmmsc_configval_set( con, "ao.driver", gtk_entry_get_text(GTK_ENTRY(ao_driver)) );
            xmmsc_result_unref(res);

            res = xmmsc_configval_set( con, "cdda.device", gtk_entry_get_text(GTK_ENTRY(cdrom)) );
            xmmsc_result_unref(res);

            res = xmmsc_configval_set( con, "mad.id3v1_encoding", gtk_entry_get_text(GTK_ENTRY(id3v1_encoding)) );
            xmmsc_result_unref(res);

            res = xmmsc_configval_set( con, "mpg123.id3v1_encoding", gtk_entry_get_text(GTK_ENTRY(id3v1_encoding)) );
            xmmsc_result_unref(res);

            show_tray_icon = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(show_tray_icon_btn));
            close_to_tray = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(close_to_tray_btn));
            play_after_exit = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(play_after_exit_btn));

            if( show_tray_icon )
            {
                if( ! tray_icon )
                    create_tray_icon();
            }
            else
            {
                if( tray_icon )
                {
                    g_object_unref(tray_icon);
                    tray_icon = NULL;
                }
            }
        }
        gtk_widget_destroy(dlg);
    }
    g_object_unref(builder);
}

static int on_track_info_received(xmmsv_t* value, void* user_data)
{
    GtkBuilder* builder = (GtkBuilder*)user_data;
    xmmsv_t *string_value, *int_value, *dict_value;
    
    GtkWidget* w;
    const char* keys[] = {
        "title", "album", "artist", "comment", "mime", NULL
    };
    const char** key;
    const char* val;
    int ival;
    /* xmmsc_result_propdict_foreach(res, dict_foreach, NULL); */

    /* file name */

    value = xmmsv_propdict_to_dict (value, NULL);
    if ( xmmsv_dict_get( value, "url", &string_value ) )
    {
	g_assert( xmmsv_get_string( string_value, &val ) );
        w = (GtkWidget*)gtk_builder_get_object(builder, "url");
        if( g_str_has_prefix(val, "file://") )
        {
            char* disp;
	    gchar *tmp;
	    gchar *decoded_val;

	    decoded_val = xmmsv_url_to_string ( string_value );
  
	    val += 7; 
	    /* skip file:// */
	    if ( decoded_val != NULL )  
	    {
		disp = g_filename_display_name(decoded_val + 7 ) ;
		g_free( decoded_val );
	    }
	    /* fallback, if decoding failed */
	    else 
		disp = g_filename_display_name(val + 7 ) ;		
            gtk_entry_set_text(GTK_ENTRY(w), disp);
            g_free(disp);
        }
        else
            gtk_entry_set_text(GTK_ENTRY(w), val);
    }

    for(key = keys; *key; ++key)
    {
	if ( xmmsv_dict_get( value, *key, &string_value ) ) 
	{
	    xmmsv_get_string( string_value, &val );
            w = (GtkWidget*)gtk_builder_get_object(builder, *key);
            if( GTK_IS_ENTRY(w) )
                gtk_entry_set_text((GtkEntry*)w, val);
            else if( GTK_IS_LABEL(w) )
                gtk_label_set_text((GtkLabel*)w, val);
        }
    }

    /* size & bitrate */
    if ( xmmsv_dict_get( value, "size", &int_value ) )
    {
        char buf[100];
	char *size_str;

	xmmsv_get_int( int_value, &ival );
	size_str = g_format_size_for_display( ival );
	strcpy( buf, size_str );
	g_free( size_str );

	if ( xmmsv_dict_get( value, "bitrate", &int_value ) ) 
	{
            int len;
	    int vbr = 0;
	    xmmsv_get_int( int_value, &ival );
	    
	    len = strlen(buf);

	    if ( xmmsv_dict_get( value, "isvbr", &int_value ) ) 
		xmmsv_get_int( int_value, &vbr );

	    g_snprintf(buf + len, 100 - len, " (%s%d Kbps%s)", _("Bitrate: "), ival/1000, vbr ? ", vbr" : "" );
	}
        w = (GtkWidget*)gtk_builder_get_object(builder, "size");
        gtk_label_set_text((GtkLabel*)w, buf);
    }
    xmmsv_unref( value );
}

void on_file_properties(GtkAction* act, gpointer data)
{
    GtkTreeSelection* sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(playlist_view));
    GtkTreeIter it;
    GtkTreeModel* model;
    GList* rows;
    if( rows = gtk_tree_selection_get_selected_rows(sel, &model) )
    {
        GtkTreePath* tp = (GtkTreePath*)rows->data;
        if( gtk_tree_model_get_iter(model, &it, tp) )
        {
            GtkBuilder* builder = gtk_builder_new();
            uint32_t id;
            gtk_tree_model_get( model, &it, COL_ID, &id, -1 );
            if( gtk_builder_add_from_file(builder, PACKAGE_DATA_DIR "/lxmusic/track-info.ui.glade", NULL ) )
            {
                xmmsc_result_t* res;
                GtkWidget* dlg = (GtkWidget*)gtk_builder_get_object(builder, "track_info_dlg");
                gtk_builder_connect_signals(builder, NULL);

                res = xmmsc_medialib_get_info(con, id);
                xmmsc_result_notifier_set_full(res, on_track_info_received, g_object_ref(builder), g_object_unref);
                xmmsc_result_unref(res);

                gtk_window_present(GTK_WINDOW(dlg));
            }
            g_object_unref(builder);
        }
        g_list_free(rows);
    }
}

void on_playlist_view_row_activated(GtkTreeView* view,
                              GtkTreePath* path,
                              GtkTreeViewColumn* col,
                              gpointer user_data)
{
    xmmsc_result_t* res;
    GtkTreeModelFilter* filter = (GtkTreeModelFilter*)gtk_tree_view_get_model(view);
    /* convert from model filter path to path of underlying liststore */
    path = gtk_tree_model_filter_convert_path_to_child_path(filter, path);
    if( path )
    {
        uint32_t pos = gtk_tree_path_get_indices(path)[0];
        /* FIXME: need to swtich to another playlist sometimes. */
        res = xmmsc_playlist_set_next( con, pos );
        xmmsc_result_unref(res);

        res = xmmsc_playback_tickle(con);
        xmmsc_result_unref(res);

        /* FIXME: just call play is not enough? */
        if( playback_status != XMMS_PLAYBACK_STATUS_PLAY )
            on_play_btn_clicked((GtkButton*)play_btn, NULL);
        gtk_tree_path_free(path);
    }
}

void on_playlist_view_drag_data_received(GtkWidget          *widget,
                                         GdkDragContext     *drag_ctx,
                                         gint                x,
                                         gint                y,
                                         GtkSelectionData   *data,
                                         uint32_t               info,
                                         uint32_t               time)
{
    char** uris;
    g_signal_stop_emission_by_name(widget, "drag-data-received");

    if( uris = gtk_selection_data_get_uris(data) )
    {
        char** uri;
        for( uri = uris; *uri; ++uri )
        {
            xmmsc_result_t* res;
            if( g_str_has_prefix(*uri, "file://") )
            {
                char* fn = g_filename_from_uri(*uri, NULL, NULL);
                if(fn)
                {
                    /* xmms2 doesn't use standard URL, but instead uses
                     * its own non-standard, weird url which only adds a prefix 
                     * 'file://' to the original file path without URL encoding.
                     */
                    char* url = g_strconcat( "file://", fn, NULL);
                    if(g_file_test(fn, G_FILE_TEST_IS_DIR))
                        res = xmmsc_playlist_radd(con, cur_playlist, url);
                    else
                        res = xmmsc_playlist_add_url(con, cur_playlist, url);
                    g_free(fn);
                    g_free(url);
                }
                else
                    res = xmmsc_playlist_radd_encoded(con, cur_playlist, *uri);
            }
            else
                res = xmmsc_playlist_radd_encoded(con, cur_playlist, *uri);

            xmmsc_result_unref(res);
        }
        g_strfreev(uris);
    }
}

gboolean on_playlist_view_drag_drop(GtkWidget      *widget,
                                    GdkDragContext *drag_ctx,
                                    gint            x,
                                    gint            y,
                                    uint32_t           time,
                                    gpointer        user_data)
{
    GdkAtom target = gdk_atom_intern_static_string("text/uri-list");

    /*  Don't call the default handler  */
//    g_signal_stop_emission_by_name( widget, "drag-drop" );

    if( g_list_find( drag_ctx->targets, target ) )
    {
        gtk_drag_get_data( widget, drag_ctx, target, time );
        return TRUE;
    }
    else
    {
    }
    gtk_drag_finish(drag_ctx, TRUE, FALSE, time);
    return TRUE;
}

static void refilter_and_keep_sel_visible()
{
    GtkTreeSelection* sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(playlist_view));
    GtkTreeModelFilter* mf;
    GList* sels, *l;
    GtkTreePath* tp;

    /* save paths of selected rows */
    sels = gtk_tree_selection_get_selected_rows(sel, (GtkTreeModel**)&mf);

    /* convert to child paths */
    for( l = sels; l; l = l->next )
    {
        tp = gtk_tree_model_filter_convert_path_to_child_path(mf, (GtkTreePath*)l->data);
        gtk_tree_path_free((GtkTreePath*)l->data);
        l->data = tp;
    }

    /* refilter */
    gtk_tree_model_filter_refilter(mf);

    /* scroll to selected rows */
    for( l = sels; l; l = l->next )
    {
        tp = gtk_tree_model_filter_convert_child_path_to_path( mf, (GtkTreePath*)l->data );
        if( tp )
        {
            gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(playlist_view), tp, NULL, FALSE, 0.0, 0.0 );
            gtk_tree_path_free(tp);
            break;
        }
    }
    g_list_foreach(sels, (GFunc)gtk_tree_path_free, NULL);
    g_list_free(sels);
}

void on_filter_field_changed(GtkComboBox* cb, gpointer user_data)
{
    filter_field = gtk_combo_box_get_active(cb);
/*
    GtkTreeModelFilter* mf = (GtkTreeModelFilter*)gtk_tree_view_get_model(playlist_view);
    gtk_tree_model_filter_refilter(mf);
*/
    refilter_and_keep_sel_visible();
}

static gboolean playlist_filter_func(GtkTreeModel* model, GtkTreeIter* it, gpointer user_data)
{
    char *artist=NULL, *album=NULL, *title=NULL;
    gboolean ret = FALSE;

    if( ! filter_keyword || '\0' == *filter_keyword )
        return TRUE;

    if( filter_field == FILTER_ALL || filter_field == FILTER_ARTIST )
        gtk_tree_model_get(model, it,
                           COL_ARTIST, &artist, -1);

    if( filter_field == FILTER_ALL || filter_field == FILTER_ALBUM )
        gtk_tree_model_get(model, it,
                           COL_ALBUM, &album, -1);

    if( filter_field == FILTER_ALL || filter_field == FILTER_TITLE )
        gtk_tree_model_get(model, it,
                           COL_TITLE, &title, -1);

    if( artist && utf8_strcasestr( artist, filter_keyword ) )
        ret = TRUE;
    else if( album && utf8_strcasestr( album, filter_keyword ) )
        ret = TRUE;
    else if( title && utf8_strcasestr( title, filter_keyword ) )
        ret = TRUE;

    g_free(artist);
    g_free(album);
    g_free(title);

    return ret;
}

static gboolean on_filter_timeout(GtkEntry* entry)
{
    GtkWidget* view = playlist_view;
    GtkTreeModelFilter* filter = (GtkTreeModelFilter*)gtk_tree_view_get_model(GTK_TREE_VIEW(view));
    g_free(filter_keyword);
    filter_keyword = g_strdup(gtk_entry_get_text(entry));
 //   gtk_tree_model_filter_refilter(filter);

    refilter_and_keep_sel_visible();

    filter_timeout = 0;
    return FALSE;
}

void on_filter_entry_changed(GtkEntry* entry, gpointer user_data)
{
    if( filter_timeout )
        g_source_remove(filter_timeout);
    filter_timeout = g_timeout_add( 600, (GSourceFunc)on_filter_timeout, entry );
}

static gboolean file_filter_fnuc(const GtkFileFilterInfo *inf, gpointer user_data)
{
    return g_str_has_prefix(inf->mime_type, "audio/");
}

static gpointer add_file( const char* file )
{
    gboolean is_dir = g_file_test( file, G_FILE_TEST_IS_DIR );
    xmmsc_result_t *res;
    char *url;

    /* Since xmms2 uses its own url format, this is annoying but inevitable. */
    url = g_strconcat( "file://", file, NULL );

    if( is_dir )
        res = xmmsc_playlist_radd( con, cur_playlist, url );
    else
        res = xmmsc_playlist_add_url( con, cur_playlist, url );
    g_free( url );

    if( res )
        xmmsc_result_unref( res );
    return NULL;
}

void on_add_files( GtkMenuItem* item, gpointer user_data )
{
    enum { RESPONSE_ADD = 1 };
    GtkWidget *dlg = gtk_file_chooser_dialog_new( NULL, (GtkWindow*)main_win,
                                                  GTK_FILE_CHOOSER_ACTION_OPEN,
                                                  GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                                  GTK_STOCK_ADD, RESPONSE_ADD, NULL );
    GtkFileFilter* filter;

    gtk_file_chooser_set_select_multiple( (GtkFileChooser*)dlg, TRUE );

    /* add a custom filter which filters autio files */
    filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, _("Audio Files"));
    gtk_file_filter_add_custom( filter, GTK_FILE_FILTER_MIME_TYPE, file_filter_fnuc, NULL, NULL );
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dlg), filter);

    filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, _("All Files"));
    gtk_file_filter_add_custom(filter, 0, (GtkFileFilterFunc)gtk_true, NULL, NULL );
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dlg), filter);

    if( gtk_dialog_run( (GtkDialog*)dlg ) == RESPONSE_ADD )
    {
        GSList* uris = gtk_file_chooser_get_uris( (GtkFileChooser*)dlg );
        GSList* uri;

        if( ! uris )
            return;

        for( uri = uris; uri; uri = uri->next )
        {
            gchar* file = g_filename_from_uri( uri->data, NULL, NULL );
            add_file( file );
            g_free( file );
            g_free( uri->data );
        }
        g_slist_free( uris );
    }
    gtk_widget_destroy( dlg );
}

void on_add_url( GtkMenuItem* item, gpointer user_data )
{
    GtkWidget *dlg = gtk_dialog_new_with_buttons(
            _("Input a URL"), (GtkWindow*)main_win, GTK_DIALOG_MODAL,
            GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
            GTK_STOCK_OK, GTK_RESPONSE_OK, NULL );
    GtkWidget *entry = gtk_entry_new();
    gtk_box_pack_start( (GtkBox*)((GtkDialog*)dlg)->vbox, entry, FALSE, FALSE, 4 );
    gtk_dialog_set_default_response( (GtkDialog*)dlg, GTK_RESPONSE_OK );
    gtk_entry_set_activates_default( (GtkEntry*)entry, TRUE );
    gtk_widget_show_all( dlg );
    if( gtk_dialog_run( (GtkDialog*)dlg ) == GTK_RESPONSE_OK )
    {
        xmmsc_result_t *res;
        const char* url = gtk_entry_get_text( (GtkEntry*)entry );
        res = xmmsc_playlist_add_encoded( con, cur_playlist, url );
        xmmsc_result_unref( res );
    }
    gtk_widget_destroy( dlg );
}

static void on_add_from_mlib( GtkMenuItem* item, gpointer user_data )
{
    /* FIXME: This might be implemented in the future */
}

void on_add_btn_clicked(GtkButton* btn, gpointer user_data)
{
    gtk_menu_popup(GTK_MENU(add_to_pl_menu), NULL, NULL, NULL, NULL, 1, GDK_CURRENT_TIME );
}

static int intcmp( gconstpointer a, gconstpointer b )
{
    return (int)b - (int)a;
}

void on_remove_all(GtkAction* act, gpointer user_data)
{
    xmmsc_result_t* res;
    res = xmmsc_playlist_clear(con, cur_playlist);
    xmmsc_result_unref(res);
}

void on_remove_selected(GtkAction* act, gpointer user_data)
{
    xmmsc_result_t* res;
    if( cur_playlist )
    {
        GtkTreeSelection* tree_sel;
        tree_sel = gtk_tree_view_get_selection( (GtkTreeView*)playlist_view );
        GList *sels = gtk_tree_selection_get_selected_rows( tree_sel, NULL );
        GList *sel;
        if( ! sels )
            return;

        for( sel = sels; sel; sel = sel->next )
        {
            GtkTreePath* path = (GtkTreePath*)sel->data;
            sel->data = (gpointer)gtk_tree_path_get_indices( path )[0];
            gtk_tree_path_free( path );
        }

        /*
            sort the list, and put rows with bigger indicies before those with smaller indicies.
            In this way, all indicies won't be changed during removing items.
        */
        sels = g_list_sort( sels, intcmp );

        for( sel = sels; sel; sel = sel->next )
        {
            xmmsc_result_t* res;
            int pos = (int)sel->data;
            res = xmmsc_playlist_remove_entry( con, cur_playlist, pos );
            xmmsc_result_unref( res );
        }
        g_list_free( sels );
    }
}

void on_remove_btn_clicked(GtkButton* btn, gpointer user_data)
{
    gtk_menu_popup(GTK_MENU(rm_from_pl_menu), NULL, NULL, NULL, NULL, 1, GDK_CURRENT_TIME );
}

void on_repeat_mode_changed(GtkComboBox* cb, gpointer user_data)
{
    xmmsc_result_t* res;
    const char* repeat_one = "0";
    const char* repeat_all = "0";
    switch(gtk_combo_box_get_active(cb))
    {
    case REPEAT_CURRENT:
        repeat_one = "1";
        break;
    case REPEAT_ALL:
        repeat_all = "1";
        break;
    }
    res = xmmsc_configval_set(con, "playlist.repeat_all", repeat_all);
    xmmsc_result_unref(res);
    res = xmmsc_configval_set(con, "playlist.repeat_one", repeat_one);
    xmmsc_result_unref(res);
}

void on_progress_bar_changed(GtkScale* bar, gpointer user_data)
{
    xmmsc_result_t* res;
    gdouble p = gtk_range_get_value(GTK_RANGE(bar));
    uint32_t new_play_time = p * cur_track_duration / 100;
    res = xmmsc_playback_seek_ms( con, new_play_time );
    xmmsc_result_unref(res);
}

void on_prev_btn_clicked(GtkButton* btn, gpointer user_data)
{
    xmmsc_result_t* res = xmmsc_playlist_set_next_rel(con, -1);
    xmmsc_result_unref(res);
    res = xmmsc_playback_tickle(con);
    xmmsc_result_unref(res);
}

void on_next_btn_clicked(GtkButton* btn, gpointer user_data)
{
    xmmsc_result_t* res = xmmsc_playlist_set_next_rel(con, 1);
    xmmsc_result_unref(res);
    res = xmmsc_playback_tickle(con);
    xmmsc_result_unref(res);
}

static int on_playback_started( xmmsv_t* value, void* user_data )
{
#if 0
    xmmsc_result_unref(res);
    /* FIXME: this can cause some problems sometimes... */
    res = xmmsc_playlist_current_pos(con, cur_playlist);
    xmmsc_result_notifier_set(res, on_playlist_pos_changed, NULL);
    xmmsc_result_unref(res);
#endif
    return TRUE;
}

void on_play_btn_clicked(GtkButton* btn, gpointer user_data)
{
    xmmsc_result_t *res;
    if( playback_status == XMMS_PLAYBACK_STATUS_PLAY )
    {
        res = xmmsc_playback_pause(con);
        xmmsc_result_notifier_set(res, on_playback_started, NULL);
        xmmsc_result_unref(res);
    }
    else
    {
        res = xmmsc_playback_start(con);
        xmmsc_result_unref(res);
    }
}

void on_stop_btn_clicked(GtkButton* btn, gpointer user_data)
{
    xmmsc_result_t* res = xmmsc_playback_stop(con);
    xmmsc_result_unref(res);
}


static void render_num( GtkTreeViewColumn* col, GtkCellRenderer* render,
                        GtkTreeModel* model, GtkTreeIter* it, gpointer data )
{
    GtkTreePath* path = gtk_tree_model_get_path( model, it );
    char buf[16];
    if( G_UNLIKELY( ! path ) )
        return;
    g_sprintf( buf, "%d", gtk_tree_path_get_indices( path )[0] + 1 );
    gtk_tree_path_free( path );
    g_object_set( render, "text", buf, NULL );
}


static int update_track( xmmsv_t *value, UpdateTrack* ut )
{
    const char *artist = NULL;
    const char *album = NULL;
    const char *title = NULL;
    xmmsv_t *string_value, *time_len_val;
    int32_t time_len = 0;
    char time_buf[32];
    /* g_debug("do update track: %d", ut->id); */

    /* OK, now it's time to send the next request.
     * This is inefficient, but it's used to overcome
     * some design flaws of xmms2d.
     * Some optimization can be done here by send about
     * 10 requets or so at the same time. */
    if( ! g_queue_is_empty(pending_update_tracks) )
    {
        xmmsc_result_t* res2;
        UpdateTrack* ut = (UpdateTrack*)g_queue_pop_head(pending_update_tracks);

        res2 = xmmsc_medialib_get_info( con, ut->id );
        xmmsc_result_notifier_set_full( res2, (xmmsc_result_notifier_t)update_track, ut, (xmmsc_user_data_free_func_t)free_update_track );
        xmmsc_result_unref( res2 );
    }

    if( xmmsv_is_error ( value ) ) {
        return FALSE;
    }
    

    value = xmmsv_propdict_to_dict (value, NULL);

    if ( xmmsv_dict_get( value, "artist", &string_value ) )
	xmmsv_get_string( string_value, &artist );
    if ( xmmsv_dict_get( value, "album", &string_value ) )
	xmmsv_get_string( string_value, &album );    
    if ( xmmsv_dict_get( value, "title", &string_value ) )
	xmmsv_get_string( string_value, &title );    
    if ( xmmsv_dict_get( value, "duration", &time_len_val ) )
	xmmsv_get_int( time_len_val, &time_len);

    timeval_to_str( time_len/1000, time_buf, G_N_ELEMENTS(time_buf) );

    /* use file name to replace track name if it doesn't have id3. */
    if( !title )
    {
        const char *url, *file;
	xmmsv_t *string_value;
	gchar *decoded_val;
	
	xmmsv_dict_get( value, "url", &string_value );

	/* try to decode URL */
	decoded_val = xmmsv_url_to_string ( string_value );

	/* undecoded url string */	
	if ( decoded_val == NULL )
	    xmmsv_get_string( string_value, &file );
	else
	    file = decoded_val;
	
	file = g_utf8_strrchr ( file, -1, '/' ) + 1;
	title = file;
	if ( decoded_val )
	    g_free ( decoded_val );
    }

    gtk_list_store_set( list_store, &ut->it,
                        COL_ARTIST, artist,
                        COL_ALBUM, album,
                        COL_TITLE, title,
                        COL_LEN, time_buf, -1 );

    xmmsv_unref( value );
    
    return TRUE;
}

static void queue_update_track( uint32_t id, GtkTreeIter* it )
{
    UpdateTrack* ut;
    /* if it's already in the queue */
    if( ! g_queue_is_empty( pending_update_tracks ) )
    {
        GList* l;
        for( l = pending_update_tracks->head; l; l = l->next )
        {
            ut = (UpdateTrack*)l->data;
            if( id == ut->id )
                return;
        }
    }

    ut = g_slice_new(UpdateTrack);
    ut->id = id;
    ut->it = *it;

    if( g_queue_is_empty( pending_update_tracks ) )
    {
        xmmsc_result_t *res;
        res = xmmsc_medialib_get_info( con, id );
        xmmsc_result_notifier_set_full( res, (xmmsc_result_notifier_t)update_track, ut, (xmmsc_user_data_free_func_t)free_update_track );
        xmmsc_result_unref( res );
    }
    else
    {
        g_queue_push_tail(pending_update_tracks, ut);
    }
}

static int on_playlist_content_received( xmmsv_t* value, GtkWidget* list_view )
{
    GtkTreeModel* mf;
    GtkTreeIter it;
    const char* pl_name = cur_playlist;
    int pl_size = xmmsv_list_get_size( value );
    int i;
    
    list_store = gtk_list_store_new(N_COLS, G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT );
    mf = gtk_tree_model_filter_new(GTK_TREE_MODEL(list_store), NULL);
    gtk_tree_model_filter_set_visible_func( GTK_TREE_MODEL_FILTER( mf ), 
					    playlist_filter_func, NULL, NULL );
    g_object_unref(list_store);

    cancel_pending_update_tracks();


    for ( i = 0; i < pl_size; i++ ) 
    {
        int32_t id;
	xmmsv_t *current_value;
        UpdateTrack* ut = g_slice_new(UpdateTrack);

        gtk_list_store_append( list_store, &it );
	xmmsv_list_get( value, i, &current_value );
	xmmsv_get_int( current_value, &id );

        gtk_list_store_set( list_store, &it,
                            COL_ID, id,
                            COL_WEIGHT, PANGO_WEIGHT_NORMAL, -1 );

        ut->id = id;
        ut->it = it;
        /* add this request to pending list */
        g_queue_push_tail( pending_update_tracks, ut );
    }

    if( !g_queue_is_empty(pending_update_tracks) )
    {
        UpdateTrack* ut = (UpdateTrack*)g_queue_pop_head(pending_update_tracks);
	xmmsc_result_t *res;
        /* only send the first request, and subsequent requests will
         * be sent after the previous one is returned. */
        res = xmmsc_medialib_get_info( con, ut->id );
        xmmsc_result_notifier_set_full( res, (xmmsc_result_notifier_t)update_track, ut, (xmmsc_user_data_free_func_t)free_update_track );
        xmmsc_result_unref( res );
    }

    if( GTK_WIDGET_REALIZED( list_view ) )
        gdk_window_set_cursor( list_view->window, NULL );

    gtk_tree_view_set_model( GTK_TREE_VIEW(list_view), mf );

    /* select the first item */
    if( gtk_tree_model_get_iter_first(GTK_TREE_MODEL(mf), &it) )
    {
        GtkTreeSelection* sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(list_view));
        gtk_tree_selection_select_iter(sel, &it);
    }

    return TRUE;
}

static int on_playlist_get_active(xmmsv_t* value, void* user_data)
{
    char* name;
    if( xmmsv_get_string(value, (const char**)&name) )
    {
        g_free(cur_playlist);
        cur_playlist = g_strdup(name);
    }
    return TRUE;
}

static void update_play_list( GtkWidget* list_view )
{
    xmmsc_result_t *res;
    const char* pl_name;

    if( GTK_WIDGET_REALIZED( list_view ) ) {
        GdkCursor* cur;
        cur = gdk_cursor_new( GDK_WATCH );
        gdk_window_set_cursor( list_view->window, cur );
        gdk_cursor_unref( cur );
    }
    res = xmmsc_playlist_list_entries( con, cur_playlist );
    xmmsc_result_notifier_set( res, (xmmsc_result_notifier_t)on_playlist_content_received, list_view );
    xmmsc_result_unref(res);
}

static GtkWidget* init_playlist(GtkWidget* list_view)
{
    GtkTreeSelection* tree_sel;
    GtkTreeViewColumn* col;
    GtkCellRenderer* render;

    render = gtk_cell_renderer_text_new();
    col = gtk_tree_view_column_new_with_attributes( "#", render, "weight", COL_WEIGHT, NULL );
    gtk_tree_view_column_set_cell_data_func( col, render, render_num, NULL, NULL );
    gtk_tree_view_append_column( (GtkTreeView*)list_view, col );

    render = gtk_cell_renderer_text_new();
    g_object_set( render, "ellipsize", PANGO_ELLIPSIZE_END, NULL );
    col = gtk_tree_view_column_new_with_attributes( _("Artist"), render,
                                                   "text", COL_ARTIST, "weight", COL_WEIGHT, NULL );
    gtk_tree_view_column_set_fixed_width( col, 80 );
    gtk_tree_view_column_set_sizing( col, GTK_TREE_VIEW_COLUMN_FIXED );
    gtk_tree_view_column_set_resizable( col, TRUE );
    gtk_tree_view_append_column( (GtkTreeView*)list_view, col );

    render = gtk_cell_renderer_text_new();
    g_object_set( render, "ellipsize", PANGO_ELLIPSIZE_END, NULL );
    col = gtk_tree_view_column_new_with_attributes( _("Album"), render,
                                                   "text", COL_ALBUM, "weight", COL_WEIGHT, NULL );
    gtk_tree_view_column_set_fixed_width( col, 100 );
    gtk_tree_view_column_set_sizing( col, GTK_TREE_VIEW_COLUMN_FIXED );
    gtk_tree_view_column_set_resizable( col, TRUE );
    gtk_tree_view_append_column( (GtkTreeView*)list_view, col );

    render = gtk_cell_renderer_text_new();
    g_object_set( render, "ellipsize", PANGO_ELLIPSIZE_END, NULL );
    col = gtk_tree_view_column_new_with_attributes( _("Title"), render,
                                                   "text", COL_TITLE, "weight", COL_WEIGHT, NULL );
    gtk_tree_view_column_set_expand( col, TRUE );
    gtk_tree_view_column_set_resizable( col, TRUE );
    gtk_tree_view_append_column( (GtkTreeView*)list_view, col );

    render = gtk_cell_renderer_text_new();
    col = gtk_tree_view_column_new_with_attributes( _("Length"), render,
                                                   "text", COL_LEN, "weight", COL_WEIGHT, NULL );
    gtk_tree_view_append_column( (GtkTreeView*)list_view, col );

    gtk_tree_view_set_search_column( (GtkTreeView*)list_view, COL_TITLE );
    tree_sel = gtk_tree_view_get_selection( (GtkTreeView*)list_view );
    gtk_tree_selection_set_mode( tree_sel, GTK_SELECTION_MULTIPLE );

    return list_view;
}

static void on_switch_to_playlist(GtkWidget* mi, char* pl_name)
{
    xmmsc_result_t* res;
    if( gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(mi)) )
    {
        /* if there are pending requests to current playlist, cancel them */
        cancel_pending_update_tracks();

        res = xmmsc_playlist_load(con, pl_name);
        xmmsc_result_unref(res);
    }
}

static int on_playlist_pos_changed( xmmsv_t* res, void* user_data );

static int on_playlist_loaded(xmmsv_t* value, gpointer user_data)
{
    char* name;
    if( !xmmsv_is_error(value) && xmmsv_get_string(value, (const char**)&name) )
    {
        xmmsc_result_t* res2;

        /* FIXME: is this possible? */
        if( cur_playlist && 0 == strcmp((char*)name, cur_playlist) )
            return;

        g_free(cur_playlist);
        cur_playlist = g_strdup(name);

        cur_track_iter.stamp = 0; /* invalidate the GtkTreeIter of currenyly played track. */
        cur_track_id = -1;

        /* update the menu */
        if( cur_playlist )
        {
            GSList* l;
            for( l = switch_pl_menu_group; l; l=l->next )
            {
                GtkRadioMenuItem* mi = (GtkRadioMenuItem*)l->data;
                char* pl_name = (char*)g_object_get_data(G_OBJECT(mi), "pl_name");
                if( strcmp(cur_playlist, pl_name) == 0 )
                {
                    g_signal_handlers_block_by_func(mi, on_switch_to_playlist, pl_name);
                    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(mi), TRUE);
                    g_signal_handlers_unblock_by_func(mi, on_switch_to_playlist, pl_name);
                    break;
                }
            }
        }

        /* if there are pending requests, cancel them */
        cancel_pending_update_tracks();

        update_play_list( playlist_view );
    }
    return TRUE;
}

static void add_playlist_to_menu(const char* pl_name, gboolean need_sort)
{
    GtkWidget* mi = gtk_radio_menu_item_new_with_label(switch_pl_menu_group, pl_name);
    char* name = g_strdup(pl_name);
    switch_pl_menu_group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(mi));
    g_object_set_data_full(G_OBJECT(mi), "pl_name", name, g_free);
    g_signal_connect( mi, "toggled", G_CALLBACK(on_switch_to_playlist), name);
    gtk_widget_show(mi);

    if( need_sort )
    {
        GSList* l;
        int i = 0;
        for(l=all_playlists; l; l = l->next, ++i)
        {
            if( g_utf8_collate(pl_name, (char*)l->data) < 0 )
                break;
        }
        gtk_menu_shell_insert(GTK_MENU_SHELL(switch_pl_menu), mi, i);
        all_playlists = g_slist_insert(all_playlists, name, i);
    }
    else
    {
        gtk_menu_shell_append(GTK_MENU_SHELL(switch_pl_menu), mi);
        all_playlists = g_slist_append(all_playlists, name );
    }

    /* toggle the menu item. This can trigger the load of the list into tree view */
    if( cur_playlist && !strcmp(pl_name, cur_playlist) )
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(mi), TRUE);
}

static void remove_playlist_from_menu(const char* pl_name)
{
    GSList* l;
    l = g_slist_find_custom(all_playlists, pl_name, (GCompareFunc)strcmp);
    all_playlists = g_slist_delete_link(all_playlists, l);

    for( l = switch_pl_menu_group; l; l = l->next )
    {
        GtkRadioMenuItem* mi = (GtkRadioMenuItem*)l->data;
        char* name = (char*)g_object_get_data(G_OBJECT(mi), "pl_name");
        if( name && strcmp(name, pl_name)==0 )
            break;
    }
    if( l )
    {
        /* switch_pl_menu_group might be out of date here. */
        if( l == switch_pl_menu_group )
            switch_pl_menu_group = l->next;
        gtk_widget_destroy(GTK_WIDGET(l->data));
    }
}

static int on_playlist_created( xmmsv_t* value, void* user_data )
{
    char* name = (char*)user_data;
    if( name && name[0] && name[0] != '_' )
        add_playlist_to_menu(name, TRUE);

    if( ! cur_playlist )
    {
        /* load the default list */
	xmmsc_result_t *res = xmmsc_playlist_load(con, name);
        xmmsc_result_unref(res);
    }
    return TRUE;
}

static int on_playlists_listed( xmmsv_t* value, void* user_data )
{
    GSList* lists = NULL, *l;
    int pl_size = xmmsv_list_get_size( value );
    int i;
    
    for ( i = 0; i < pl_size; i++ ) 
    {
        const char* str = NULL;
	xmmsv_t *string_value;
	xmmsv_list_get( value, i, &string_value );
	xmmsv_get_string( string_value, &str );
        if( str && str[0] && str[0] != '_' )
            lists = g_slist_prepend(lists, (gpointer) str);
    }
    lists = g_slist_sort(lists, (GCompareFunc)g_utf8_collate);
    for(l = lists; l; l = l->next)
        add_playlist_to_menu((char*)l->data, FALSE);
    g_slist_free(lists);

    /* If there is no playlist, create one */
    if( lists == NULL )
    {
        char* name = g_strdup(_("Default"));
        xmmsc_result_t *res = xmmsc_playlist_create(con, name);
        xmmsc_result_notifier_set_full(res, on_playlist_created, name, g_free);
        xmmsc_result_unref(res);
    }

    if( cur_playlist )
        update_play_list( playlist_view );
}

static int on_playlist_content_changed( xmmsv_t* value, void* user_data )
{
    int32_t id = 0;
    int type = 0, pos = -1;
    const char* name = NULL;
    xmmsv_t *string_value, *int_value;
    
    if( xmmsv_is_error( value ) )
	return TRUE;

    xmmsv_dict_get( value, "type", &int_value );
    if( !xmmsv_get_int( int_value, &type ) )
        return TRUE;
    
    xmmsv_dict_get( value, "name", &string_value );
    if ( !xmmsv_get_string( string_value, &name ) )
	return TRUE;

    if( ! name || !cur_playlist || strcmp(name, cur_playlist) )
	return TRUE;

    /* g_debug("type=%d, name=%s", type, name); */

    if( ! list_store )
	return TRUE;
    
    switch( type )
    {
        case XMMS_PLAYLIST_CHANGED_ADD:
        case XMMS_PLAYLIST_CHANGED_INSERT:
	    if ( ! xmmsv_dict_get( value, "position", &int_value ) ) 
		pos = gtk_tree_model_iter_n_children( (GtkTreeModel*)list_store, NULL );
	    else
		xmmsv_get_int( int_value, &pos );
	    
	    if ( xmmsv_dict_get( value, "id", &int_value ) ) 
	    {
                GtkTreeIter it;
		xmmsv_get_int( int_value, &id );
                gtk_list_store_insert_with_values( list_store, &it, pos,
                                                   COL_ID, id, -1 );
                /* g_debug("playlist_added: %d", id); */
                queue_update_track( id, &it );
            }
            break;
        case XMMS_PLAYLIST_CHANGED_REMOVE:
            if( xmmsv_dict_get( value, "position", &int_value ) ) {
                GtkTreePath* path;
                GtkTreeIter it;
		xmmsv_get_int( int_value, &pos );
                path = gtk_tree_path_new_from_indices( pos, -1 );
                if( gtk_tree_model_get_iter( (GtkTreeModel*)list_store, &it, path ) )
                    gtk_list_store_remove( list_store, &it );
                gtk_tree_path_free( path );
	    }
            break;
        case XMMS_PLAYLIST_CHANGED_CLEAR:
        {
            gtk_list_store_clear( list_store );
            break;
        }
        case XMMS_PLAYLIST_CHANGED_MOVE:
        {
	    /* hmm this seems not to be fully implemeted ? */
            int newpos;
	    g_assert( xmmsv_dict_get( value, "position", &int_value ) );
	    xmmsv_get_int( int_value, &pos );
	    g_assert( xmmsv_dict_get( value, "newposition", &int_value ) );	    
	    xmmsv_get_int( int_value, &newpos );
	    g_warning( "Move from %d to %d not implemented" , pos, newpos);
            break;
        }
        case XMMS_PLAYLIST_CHANGED_SORT:
        case XMMS_PLAYLIST_CHANGED_SHUFFLE:
        {
            update_play_list(playlist_view);
            break;
        }

	    
        case XMMS_PLAYLIST_CHANGED_UPDATE:
            break;
    }
    return TRUE;
}

static int on_playback_status_changed( xmmsv_t *value, void *user_data )
{
    GtkWidget* img;
    if ( !xmmsv_get_int(value, &playback_status) )
    {
        playback_status = XMMS_PLAYBACK_STATUS_STOP;
        return TRUE;
    }

    switch( playback_status )
    {
        case XMMS_PLAYBACK_STATUS_PLAY:
        {
            xmmsc_result_t* res2;
            gtk_widget_set_tooltip_text( play_btn, _("Pause") );
            img = gtk_bin_get_child( (GtkBin*)play_btn );
            gtk_image_set_from_stock( (GtkImage*)img, GTK_STOCK_MEDIA_PAUSE,
                                      GTK_ICON_SIZE_BUTTON );
            /* FIXME: this can cause some problems sometimes... */
            res2 = xmmsc_playlist_current_pos(con, cur_playlist);
            /* mark currently played track */
            xmmsc_result_notifier_set(res2, on_playlist_pos_changed, NULL);
            xmmsc_result_unref(res2);
            break;
        }
        case XMMS_PLAYBACK_STATUS_STOP:
            gtk_label_set_text( GTK_LABEL(time_label), "--:--" );
            gtk_range_set_value( GTK_RANGE(progress_bar), 0.0 );
			gtk_window_set_title ((GtkWindow*)main_win, "LXMusic");
        case XMMS_PLAYBACK_STATUS_PAUSE:
            gtk_widget_set_tooltip_text( play_btn, _("Play") );
            img = gtk_bin_get_child( (GtkBin*)play_btn );
            gtk_image_set_from_stock( (GtkImage*)img, GTK_STOCK_MEDIA_PLAY,
                                      GTK_ICON_SIZE_BUTTON );
            break;
    }

    return TRUE;
}

static int on_playback_playtime_changed( xmmsv_t* value, void* user_data )
{
    int32_t time;
    char buf[32];
    if ( xmmsv_is_error(value)
         || ! xmmsv_get_int(value, &time))
        return TRUE;
    
    time /= 1000;
    if( time == play_time )
        return TRUE;
    play_time = time;

    gtk_label_set_text( (GtkLabel*)time_label,
                        timeval_to_str( time, buf, G_N_ELEMENTS(buf) ) );

    if( cur_track_duration > 0 )
    {
        g_signal_handlers_block_by_func(progress_bar, on_progress_bar_changed, user_data);
        gtk_range_set_value( (GtkRange*)progress_bar,
                              (((gdouble)100000 * time) / cur_track_duration) );
        g_signal_handlers_unblock_by_func(progress_bar, on_progress_bar_changed, user_data);
    }
    return TRUE;
}

static int on_playback_track_loaded( xmmsv_t* value, void* user_data )
{
    const char* artist = NULL;
    const char* title = NULL;
    char* filename;
    const char* err;
    
    xmmsv_t *duration_value;
    xmmsv_t *string_value;
    GString* window_title;
    
    cur_track_duration = 0;    

    if (xmmsv_get_error (value, &err)) {
	g_warning( "Server error: %s", err );
	return TRUE;
    }
    value = xmmsv_propdict_to_dict (value, NULL);

    if( xmmsv_dict_get( value, "duration", &duration_value ) )
	xmmsv_get_int( duration_value, &cur_track_duration );

    artist = NULL;
    if ( xmmsv_dict_get( value, "artist", &string_value ) )
	xmmsv_get_string( string_value, &artist );

    if ( xmmsv_dict_get( value, "title", &string_value ) ) 
	xmmsv_get_string( string_value, &title );
    /* url fallback */
    else 
    {
	if ( xmmsv_dict_get( value, "url", &string_value ) ) 
	{
	    char* url;
            const char* file;
	    gchar *decoded_val;

	    /* try to decode URL */
	    decoded_val = xmmsv_url_to_string ( string_value );
	    
	    /* undecoded url string */	
	    if ( decoded_val == NULL )
		xmmsv_get_string( string_value, &file );
	    else
		file = decoded_val;
	    file = g_utf8_strrchr ( file, -1, '/' ) + 1;
	    title = file;
	    if ( decoded_val )
		g_free ( decoded_val );
        }
    }

    /* default */
    window_title = g_string_new( "LXMusic" );

    /* playing or pause */
    if ( playback_status == XMMS_PLAYBACK_STATUS_PLAY) 
    {
	if ( artist != NULL )
	    g_string_append_printf( window_title, " - %s", artist );
	if ( title != NULL )
	    g_string_append_printf( window_title, " - %s", title );
    }

    gtk_window_set_title( GTK_WINDOW(main_win), window_title->str );
    
    if( tray_icon )
        gtk_status_icon_set_tooltip(GTK_STATUS_ICON(tray_icon),
				    window_title->str);

    if( ! GTK_WIDGET_VISIBLE(main_win) /*|| ! GTK_WIDGET_HAS_FOCUS(main_win)*/ )
    {
        /* send notifications via notify-send command. */
        const char* argv[] = {
            "notify-send",
            "--urgency", "normal",
            "--expire-time", "3000",
            "--icon", "lxmusic",
            NULL, NULL, /* hint x */
            NULL, NULL, /* hint y */
            NULL, NULL, /* summary, body */
            NULL
        };
        char xhint[32], yhint[32];

        if( tray_icon )
        {
    #if GTK_CHECK_VERSION(2, 14, 0)
            guint32 wid = gtk_status_icon_get_x11_window_id(GTK_STATUS_ICON(tray_icon));
            Window root, child;
            int x, y, w, h, b, d, x2, y2;

            XGetGeometry(GDK_DISPLAY(), (Drawable) wid, &root, &x, &y, &w, &h, &b, &d);
            XTranslateCoordinates(GDK_DISPLAY(), (Window)wid, root, x, y, &x2, &y2, &child);

            argv[7] = "--hint";
            g_snprintf(xhint, 32, "int:x:%u", x2 + w/2);
            argv[8] = xhint;

            argv[9] = "--hint";
            g_snprintf(yhint, 32, "int:y:%u", y2 + h/2);
            argv[10] = yhint;

    #define REST_IDX    11
    #else
    #define REST_IDX    7
    #endif
        }

        argv[REST_IDX] = _("Now Playing:");
        argv[REST_IDX+1] = window_title->str;
    #undef REST_IDX
        g_spawn_async(NULL, (gchar**)argv, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, NULL, NULL);
    }
    g_string_free( window_title, TRUE );
    xmmsv_unref( value );
    return TRUE;
}

static int on_playback_cur_track_changed( xmmsv_t* value, void* user_data )
{
    if( xmmsv_get_int(value, &cur_track_id) )
    {
        xmmsc_result_t *res2;
        char* name;
        res2 = xmmsc_medialib_get_info(con, cur_track_id);
        xmmsc_result_notifier_set(res2, on_playback_track_loaded, NULL);
        xmmsc_result_unref(res2);
    }
    
    return TRUE;
}

static int on_playlist_pos_changed( xmmsv_t* val, void* user_data )
{
    const char* name;
    xmmsv_t *name_val;
    int32_t pos = 0;
    
    if( xmmsv_dict_get( val, "name", &name_val ) )
    {
	xmmsv_get_string( name_val, &name );
        if( name && cur_playlist && !strcmp(cur_playlist, name) )
        {
	    xmmsv_t *pos_val;
	    
            if( xmmsv_dict_get( val, "position", &pos_val ) ) 
            {
		xmmsv_get_int( pos_val, &pos );
                /* mark currently played song in the playlist with bold font. */
                GtkTreePath* path = gtk_tree_path_new_from_indices( pos, -1 );
                GtkTreeIter it;
                if( gtk_tree_model_get_iter( GTK_TREE_MODEL(list_store), &it, path ) )
                {
                    if( cur_track_iter.stamp )
                        gtk_list_store_set(list_store, &cur_track_iter, COL_WEIGHT, PANGO_WEIGHT_NORMAL, -1);
		    
                    gtk_list_store_set(list_store, &it, COL_WEIGHT, PANGO_WEIGHT_BOLD, -1);
                    cur_track_iter = it;
                }
                gtk_tree_path_free( path );
	    }
	}
    }
    return TRUE;
}


void on_locate_cur_track(GtkAction* act, gpointer user_data)
{
    if( cur_track_iter.stamp )
    {
        GtkTreeModelFilter* mf;
        GtkTreeIter it;
        mf = (GtkTreeModelFilter*)gtk_tree_view_get_model(GTK_TREE_VIEW(playlist_view));
        if( gtk_tree_model_filter_convert_child_iter_to_iter(mf, &it, &cur_track_iter) )
        {
            GtkTreePath* path;
            /*
            GtkTreeSelection* sel = gtk_tree_view_get_selection(playlist_view);
            gtk_tree_selection_select_iter(sel, &it);
            */
            if( path = gtk_tree_model_get_path(GTK_TREE_MODEL(mf), &it) )
            {
                gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(playlist_view), path, NULL, FALSE, 0.0, 0.0);
                gtk_tree_path_free(path);
            }
        }
    }
}

static void get_channel_volumes( const char *key, xmmsv_t *value, void *user_data)
{
    GSList** volumes = (GSList**)user_data;
    int32_t volume;
    xmmsv_get_int( value , &volume );
    *volumes = g_slist_prepend(*volumes, (gpointer)volume);
}

static void get_channel_volume_names(const char* key, xmmsv_t *value, void *user_data)
{
    GSList** volumes = (GSList**)user_data;
    *volumes = g_slist_prepend(*volumes, (gpointer)key);
}

static int on_volume_btn_set_volume(xmmsv_t *value, void* user_data)
{
    GSList* volumes = NULL, *l;
    uint32_t val = GPOINTER_TO_UINT(user_data);

    g_assert( value != NULL );
    xmmsv_dict_foreach( value, get_channel_volume_names, &volumes);

    for( l = volumes; l; l = l->next )
    {
        xmmsc_result_t* res2;
        res2 = xmmsc_playback_volume_set(con, (char*)l->data, val);
        xmmsc_result_unref(res2);
    }
    g_slist_free(volumes);
    return TRUE;
}

static void on_volume_btn_changed(GtkScaleButton* btn, gdouble val, gpointer user_data)
{
    xmmsc_result_t* res;
    res = xmmsc_playback_volume_get(con);
    xmmsc_result_notifier_set(res, on_volume_btn_set_volume, GUINT_TO_POINTER((uint32_t)val));
    xmmsc_result_unref(res);
}

static void on_volume_btn_scrolled(GtkWidget *widget, GdkEventScroll *event, gpointer user_data)
{
	guint volume;
	GtkAdjustment *vol_adj;
	xmmsc_result_t *res;
	res = xmmsc_playback_volume_get(con);
	if (event->type != GDK_SCROLL)
		return;
	vol_adj = gtk_scale_button_get_adjustment (GTK_SCALE_BUTTON(user_data));
	switch (event->direction)
	{
		case GDK_SCROLL_UP:
			volume = gtk_adjustment_get_value (vol_adj) + 2;
			gtk_adjustment_set_value (GTK_ADJUSTMENT(vol_adj), volume);
			break;
		case GDK_SCROLL_DOWN:
			volume = gtk_adjustment_get_value (vol_adj) - 2;
			gtk_adjustment_set_value (GTK_ADJUSTMENT(vol_adj), volume);
			break;
		default:
			return;
	}
    xmmsc_result_notifier_set(res, on_volume_btn_set_volume, GUINT_TO_POINTER((uint32_t)volume));
    xmmsc_result_unref(res);
}

static int on_playback_volume_changed( xmmsv_t* value, void* user_data )
{
    GSList* volumes = NULL, *l;
    uint32_t vol = 0;

    /* FIXME: OSS4 and pulse audio disconnect when playback is stopped,
     * and hence we will receive a empty result here when playback is stopped.
     * We need to handle this more gracefully in the future. */

    if( value != NULL )
    {
	xmmsv_dict_foreach( value, get_channel_volumes, &volumes );
        for( l = volumes; l; l = l->next )
        {
            if( vol < GPOINTER_TO_UINT(l->data) )
                vol = GPOINTER_TO_UINT(l->data);
        }
        g_slist_free(volumes);

        g_signal_handlers_block_by_func( volume_btn, on_volume_btn_changed, NULL );
        gtk_scale_button_set_value( GTK_SCALE_BUTTON(volume_btn), vol );
        g_signal_handlers_unblock_by_func( volume_btn, on_volume_btn_changed, NULL );
    }
    
    return TRUE;
}

static int on_collection_changed( xmmsv_t* dict, void* user_data )
{
    /* xmmsc_result_dict_foreach(res, dict_foreach, NULL); */
    
    xmmsv_t *string_value, *int_value;
    const char *name, *ns;
    gint32 type;
    
    
    xmmsv_dict_get (dict, "name", &string_value);
    xmmsv_get_string( string_value, &name);

    xmmsv_dict_get (dict, "namespace", &string_value);
    xmmsv_get_string( string_value, &ns );

    xmmsv_dict_get (dict, "type", &int_value);	
    xmmsv_get_int( int_value, &type );
    
    /* g_debug("name=%s, ns=%s, type=%d", name, ns, type); */

    /* currently we only care about playlists */
    if( ns && strcmp(ns, "Playlists") == 0 )
    {
        switch(type)
        {
        case XMMS_COLLECTION_CHANGED_ADD:
            add_playlist_to_menu( name, TRUE );
            break;
        case XMMS_COLLECTION_CHANGED_UPDATE:
            break;
        case XMMS_COLLECTION_CHANGED_RENAME:
            // rename_playlist( name, newname );
            break;
        case XMMS_COLLECTION_CHANGED_REMOVE:
            remove_playlist_from_menu( name );
            break;
        }
    }
    return TRUE;
}

static int on_media_lib_entry_changed(xmmsv_t* value, void* user_data)
{
    /* g_debug("mlib entry changed"); */
    int32_t id = 0;
    if( xmmsv_get_int (value, &id) )
    {
        GtkTreeModel* model = (GtkTreeModel*)list_store;
        GtkTreeIter it;
        if( !model )
            return;
        /* FIXME: This is damn inefficient, but I didn't have a
         * better way now.
         * Maybe it can be improved using custom tree model. :-( */
        if( gtk_tree_model_get_iter_first(model, &it) )
        {
            uint32_t _id;
            do{
                gtk_tree_model_get(model, &it, COL_ID, &_id, -1);
                if( _id == id )
                {
                    /* g_debug("found! update: %d", id); */
                    queue_update_track( id, &it );
                    break;
                }
            }while(gtk_tree_model_iter_next(model, &it));
        }
    }
    return TRUE;
}

static void config_changed_foreach(const char *key, xmmsv_t *value, void* user_data)
{
    if( strncmp( key, "playlist.", 9) == 0 )
    {
        const char* val;
	g_assert( xmmsv_get_string( value, &val ) );

        if( strcmp( key + 9, "repeat_one") == 0 )
        {
            if( val[0] == '1' )
                repeat_mode = REPEAT_CURRENT;
            else
            {
                if( repeat_mode == REPEAT_CURRENT )
                    repeat_mode = REPEAT_NONE;
            }
        }
        else if( strcmp( key + 9, "repeat_all") == 0 )
        {
            if( val[0] == '1' )
                repeat_mode = REPEAT_ALL;
            else
            {
                if( repeat_mode == REPEAT_ALL )
                    repeat_mode = REPEAT_NONE;
            }
        }
        g_signal_handlers_block_by_func(repeat_mode_cb, on_repeat_mode_changed, NULL );
        gtk_combo_box_set_active( GTK_COMBO_BOX(repeat_mode_cb), repeat_mode );
        g_signal_handlers_unblock_by_func(repeat_mode_cb, on_repeat_mode_changed, NULL );
    }
}

static void on_configval_changed(xmmsv_t* value, void* user_data)
{
    xmmsv_dict_foreach( value, config_changed_foreach, NULL );
}

static void setup_xmms_callbacks()
{
    xmmsc_result_t* res;
    /* play status */
    res = xmmsc_playback_status(con);
    on_playback_status_changed( xmmsc_result_get_value( res ), NULL );
    xmmsc_result_notifier_set(res, on_playback_status_changed, NULL);
    xmmsc_result_unref(res);
    XMMS_CALLBACK_SET( con, xmmsc_broadcast_playback_status,
                       on_playback_status_changed, NULL );

    /* play time */
    /* this is a signal rather than broadcast, so restart is needed in the callback func. */
    XMMS_CALLBACK_SET( con, xmmsc_signal_playback_playtime,
                       on_playback_playtime_changed, NULL);

    /* playlist changed */
    XMMS_CALLBACK_SET( con, xmmsc_broadcast_playlist_changed,
                       on_playlist_content_changed, NULL );

    /* playlist loaded */
    XMMS_CALLBACK_SET( con, xmmsc_broadcast_playlist_loaded,
                       on_playlist_loaded, NULL );

    /* current track info */
    res = xmmsc_playback_current_id( con );
    xmmsc_result_notifier_set( res, on_playback_cur_track_changed, NULL );
    xmmsc_result_unref(res);
    XMMS_CALLBACK_SET( con, xmmsc_broadcast_playback_current_id,
                       on_playback_cur_track_changed, NULL );

    /* current pos in playlist */
//    res = xmmsc_playlist_current_pos( con, "_active" );
//    xmmsc_result_notifier_set( res, on_playlist_pos_changed, NULL );
//    xmmsc_result_unref(res);
    XMMS_CALLBACK_SET( con, xmmsc_broadcast_playlist_current_pos,
                       on_playlist_pos_changed, NULL);

    /* volume */
    res = xmmsc_playback_volume_get(con);
    xmmsc_result_notifier_set(res, on_playback_volume_changed, NULL );
    xmmsc_result_unref(res);
    XMMS_CALLBACK_SET( con, xmmsc_broadcast_playback_volume_changed,
                       on_playback_volume_changed, NULL );

    /* media lib */
    XMMS_CALLBACK_SET( con, xmmsc_broadcast_medialib_entry_changed,
                       on_media_lib_entry_changed, NULL );

    XMMS_CALLBACK_SET( con, xmmsc_broadcast_collection_changed,
                       on_collection_changed, NULL );


    /* config values */
    XMMS_CALLBACK_SET( con, xmmsc_broadcast_configval_changed,
                       on_configval_changed, NULL );
}

static int on_cfg_repeat_all_received(xmmsv_t* value, void* user_data)
{
    char* val;
    GtkComboBox* cb = (GtkComboBox*)user_data;
    if( xmmsv_get_string(value, (const char**)&val) )
    {
        if( val && val[0] == '1' )
        {
            repeat_mode = REPEAT_ALL;
            gtk_combo_box_set_active( cb, repeat_mode );
        }
    }
    return TRUE;
}

static int on_cfg_repeat_one_received(xmmsv_t* value, void* user_data)
{
    GtkComboBox* cb = (GtkComboBox*)user_data;
    char* val;
    if( xmmsv_get_string( value, (const char**)&val) )
    {
        if( val && val[0] == '1' )
        {
            repeat_mode = REPEAT_CURRENT;
            gtk_combo_box_set_active( cb, repeat_mode );
        }
    }
    return TRUE;
}


static void setup_ui()
{
    GtkBuilder *builder;
    GtkUIManager* mgr;
    GtkWidget *hbox, *cb, *switch_pl_mi, *show_pl_mi;
    xmmsc_result_t* res;

    builder = gtk_builder_new();
    if( ! gtk_builder_add_from_file(builder, PACKAGE_DATA_DIR "/lxmusic/lxmusic.ui.glade", NULL) )
        exit(1);

    main_win = (GtkWidget*)gtk_builder_get_object(builder, "main_win");
    play_btn = (GtkWidget*)gtk_builder_get_object(builder, "play_btn");
    time_label = (GtkWidget*)gtk_builder_get_object(builder, "time_label");
    progress_bar = (GtkWidget*)gtk_builder_get_object(builder, "progress_bar");
    notebook = (GtkWidget*)gtk_builder_get_object(builder, "notebook");
    status_bar = (GtkWidget*)gtk_builder_get_object(builder, "status_bar");

    inner_vbox = (GtkWidget*)gtk_builder_get_object(builder, "inner_vbox");
    playlist_view = (GtkWidget*)gtk_builder_get_object(builder, "playlist_view");
    gtk_drag_dest_add_uri_targets(playlist_view);

    repeat_mode_cb = (GtkWidget*)gtk_builder_get_object(builder, "repeat_mode");

    mgr = (GtkUIManager*)gtk_builder_get_object(builder, "uimanager1");
    switch_pl_mi = gtk_ui_manager_get_widget( mgr, "/menubar/playlist_mi/switch_to_pl" );
    switch_pl_menu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(switch_pl_mi), switch_pl_menu);

    add_to_pl_menu = gtk_menu_item_get_submenu((GtkMenuItem*)gtk_ui_manager_get_widget( mgr, "/menubar/playlist_mi/add_to_pl" ));
    rm_from_pl_menu = gtk_menu_item_get_submenu((GtkMenuItem*)gtk_ui_manager_get_widget( mgr, "/menubar/playlist_mi/remove_from_pl" ));

    show_pl_mi = gtk_ui_manager_get_widget( mgr, "/menubar/view_mi/show_pl" );

    cb = (GtkWidget*)gtk_builder_get_object(builder, "repeat_mode");
    gtk_combo_box_set_active(GTK_COMBO_BOX(cb), REPEAT_NONE);
//    gtk_combo_box_set_active(cb, repeat_mode);
    res = xmmsc_configval_get( con, "playlist.repeat_all" );
    xmmsc_result_notifier_set( res, on_cfg_repeat_all_received, cb );
    xmmsc_result_unref(res);
    res = xmmsc_configval_get( con, "playlist.repeat_one" );
    xmmsc_result_notifier_set( res, on_cfg_repeat_one_received, cb );
    xmmsc_result_unref(res);

    cb = (GtkWidget*)gtk_builder_get_object(builder, "filter_field");
    gtk_combo_box_set_active(GTK_COMBO_BOX(cb), filter_field);

    /* add volume button */
    hbox = (GtkWidget*)gtk_builder_get_object(builder, "top_hbox");
    volume_btn = gtk_volume_button_new();
    gtk_scale_button_get_adjustment(GTK_SCALE_BUTTON(volume_btn))->upper = 100;
    gtk_widget_show(volume_btn);
    gtk_box_pack_start(GTK_BOX(hbox), volume_btn, FALSE, TRUE, 0);
    g_signal_connect(volume_btn, "value-changed", G_CALLBACK(on_volume_btn_changed), NULL);
    g_signal_connect(volume_btn, "scroll-event", G_CALLBACK(on_volume_btn_scrolled), volume_btn);

    /* init the playlist widget */
    init_playlist(playlist_view);

    /* signal handlers */
    gtk_builder_connect_signals(builder, NULL);

    /* window icon */
    gtk_window_set_icon_from_file(GTK_WINDOW(main_win), PACKAGE_DATA_DIR"/pixmaps/lxmusic.png", NULL );

    gtk_window_set_default_size(GTK_WINDOW(main_win), win_width, win_height);
    /* this can trigger signal handler and show or hide the playlist. */
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(show_pl_mi), show_playlist);

    g_object_unref(builder);
    gtk_widget_show_all(notebook);
    gtk_window_move(GTK_WINDOW(main_win), win_xpos, win_ypos);
	gtk_widget_show (main_win);

    /* tray icon */
    if( show_tray_icon )
        create_tray_icon();
}

void on_new_playlist(GtkAction* act, gpointer user_data)
{
    GtkWidget *dlg = gtk_dialog_new_with_buttons(
            _("Create new playlist"), (GtkWindow*)main_win, GTK_DIALOG_MODAL,
            GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
            GTK_STOCK_OK, GTK_RESPONSE_OK, NULL );
    GtkWidget *entry = gtk_entry_new();
    gtk_box_pack_start( (GtkBox*)((GtkDialog*)dlg)->vbox, entry, FALSE, FALSE, 4 );
    gtk_dialog_set_default_response( (GtkDialog*)dlg, GTK_RESPONSE_OK );
    gtk_entry_set_activates_default( (GtkEntry*)entry, TRUE );
    gtk_widget_show_all( dlg );
    if( gtk_dialog_run( (GtkDialog*)dlg ) == GTK_RESPONSE_OK )
    {
        xmmsc_result_t *res;
        const char* name = gtk_entry_get_text( (GtkEntry*)entry );
        res = xmmsc_playlist_create( con, name );
        xmmsc_result_unref( res );

        /* load the newly created list */
        xmmsc_playlist_load(con, name);
        xmmsc_result_unref( res );
    }
    gtk_widget_destroy( dlg );
}

void on_del_playlist(GtkAction* act, gpointer user_data)
{
    GSList *prev=NULL, *l;
    char* switch_to = NULL;

    /* NOTE: we should at least have one playlist */
    if( !switch_pl_menu_group || !switch_pl_menu_group )
        return;
    if( ! cur_playlist )
        return;

    /* switching to another playlist before removing the current one. */
    for( l = all_playlists; l; l = l->next )
    {
        char* name = (char*)l->data;
        if( 0 == strcmp( name, cur_playlist ) )
        {
            if( l->next )
                switch_to = (char*)l->next->data;
            else if(prev)
                switch_to = (char*)prev->data;
            break;
        }
        prev= l;
    }
    if( switch_to )
    {
        xmmsc_result_t* res;
        res = xmmsc_playlist_load(con, switch_to);
        xmmsc_result_unref(res);

        res = xmmsc_playlist_remove(con, cur_playlist);
        xmmsc_result_unref(res);
    }
}

void on_show_playlist(GtkAction* act, gpointer user_data)
{
    show_playlist = gtk_toggle_action_get_active(GTK_TOGGLE_ACTION(act));
    if(GTK_WIDGET_VISIBLE(inner_vbox))
    {
        if( ! show_playlist )
        {
            /* save current size to restore it later. */
            gtk_window_get_size(GTK_WINDOW(main_win), &win_width, &win_height );
            gtk_widget_hide(inner_vbox);
            /* Dirty trick used to shrink the window to its minimal size */
            gtk_window_resize(GTK_WINDOW(main_win), 1, 1);
        }
    }
    else
    {
        if( show_playlist )
        {
            gtk_window_resize(GTK_WINDOW(main_win), win_width, win_height );
            gtk_widget_show(inner_vbox);
        }
    }
}

void on_playlist_cut(GtkAction* act, gpointer user_data)
{
    show_error( GTK_WINDOW(main_win), NULL, "Not yet implemented");
}

void on_playlist_copy(GtkAction* act, gpointer user_data)
{
    show_error( GTK_WINDOW(main_win), NULL, "Not yet implemented");
}

void on_playlist_paste(GtkAction* act, gpointer user_data)
{
    show_error( GTK_WINDOW(main_win), NULL, "Not yet implemented");
}


int main (int argc, char *argv[])
{
    xmmsc_result_t* res;

#ifdef ENABLE_NLS
    bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
    bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
    textdomain (GETTEXT_PACKAGE);
#endif

    if( !(con = xmmsc_init ("lxmusic")) )
        return EXIT_FAILURE;

    /* try to connect to xmms2d, and launch the daemon if needed. */
    if( !xmmsc_connect(con, getenv("XMMS_PATH")) )
    {
        if( ! g_spawn_command_line_sync( "xmms2-launcher", NULL, NULL, NULL, NULL )
            || !xmmsc_connect(con, getenv ("XMMS_PATH")) )
        {
            fprintf(stderr, "Connection failed: %s\n",
                     xmmsc_get_last_error (con));
            return EXIT_FAILURE;
        }
    }

    gtk_init(&argc, &argv);
    xmmsc_mainloop_gmain_init(con);

    load_config();

    /* build the GUI */
    setup_ui();

    /* some dirty hacks to show the window earlier :-D */
    while( gtk_events_pending() )
        gtk_main_iteration();
    gdk_display_sync(gtk_widget_get_display(main_win));

    pending_update_tracks = g_queue_new();

    /* display currently active playlist */
    res = xmmsc_playlist_current_active(con);
    xmmsc_result_notifier_set(res, on_playlist_get_active, NULL);
    xmmsc_result_unref(res);

    /* load all existing playlists and add them to the menu */
    res = xmmsc_playlist_list( con );
    xmmsc_result_notifier_set(res, on_playlists_listed, NULL);
    xmmsc_result_unref(res);

    /* register callbacks */
    setup_xmms_callbacks();

    gtk_main ();

    g_queue_free(pending_update_tracks);
    save_config();

    return 0;
}

