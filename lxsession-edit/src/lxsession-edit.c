/*
 *      lxsession-edit.c
 *      
 *      Copyright 2008 PCMan <pcman.tw@gmail.com>
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
#include <config.h>
#endif

#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <stdio.h>
#include <string.h>

enum {
    COL_ENABLED,
    COL_ICON,
    COL_NAME,
    COL_COMMENT,
    COL_DESKTOP_ID,
    N_COLS
};

static char* session_name = "LXDE";
static GtkListStore* autostart_list = NULL;
static const char grp[] = "Desktop Entry";


static gboolean is_desktop_file_enabled(GKeyFile* kf)
{
    char** not_show_in;
    char** only_show_in;
    gsize n, i;

    not_show_in = g_key_file_get_string_list(kf, grp, "NotShowIn", &n, NULL);
    if( not_show_in )
    {
        for( i = 0; i < n; ++i )
            if(strcmp(not_show_in[i], session_name) == 0)
            {
                g_strfreev(not_show_in);
                return FALSE;
            }
        g_strfreev(not_show_in);
    }

    only_show_in = g_key_file_get_string_list(kf, grp, "OnlyShowIn", &n, NULL);
    if( only_show_in )
    {
        for( i = 0; i < n; ++i )
            if(strcmp(only_show_in[i], session_name) == 0)
                break;
        g_strfreev(only_show_in);
        if( i >= n )
            return FALSE;
    }

    return TRUE;
}

static gboolean is_desktop_file_valid(GKeyFile* kf)
{
    char* tmp;
    /* FIXME: is this correct? */
    if( g_key_file_get_boolean(kf, grp, "NoDisplay", NULL) )
        return FALSE;
    if( tmp = g_key_file_get_string(kf, grp, "Type", NULL) )
    {
        if( strcmp(tmp, "Application") )
        {
            g_free(tmp);
            return FALSE;
        }
        g_free(tmp);
    }
    if( tmp = g_key_file_get_string(kf, grp, "TryExec", NULL) )
    {
        char* prg = g_find_program_in_path(tmp);
        g_free(tmp);
        if(!prg)
            return FALSE;
        g_free(prg);
    }
    return TRUE;
}

static void get_autostart_files_in_dir( GHashTable* hash, const char* session_name, const char* base_dir )
{
    char* dir_path = g_build_filename( base_dir, "autostart", NULL );
    GDir* dir = g_dir_open( dir_path, 0, NULL );
    if( dir )
    {
        char *path;
        const char *name;

        while( (name = g_dir_read_name( dir )) && g_str_has_suffix( name, ".desktop" ) )
        {
            path = g_build_filename( dir_path, name, NULL );
            g_hash_table_replace( hash, g_strdup(name), path );
        }
        g_dir_close( dir );
    }
    g_free( dir_path );
}

static void add_autostart_file(char* desktop_id, char* file, GKeyFile* kf)
{
    GtkTreeIter it;
    if( g_key_file_load_from_file( kf, file, 0, NULL ) )
    {
        if( is_desktop_file_valid(kf) )
        {
            char* name = g_key_file_get_locale_string(kf, grp, "Name", NULL, NULL);
            char* icon = g_key_file_get_locale_string(kf, grp, "Icon", NULL, NULL);
            char* comment = g_key_file_get_locale_string(kf, grp, "Comment", NULL, NULL);
            gboolean enabled = is_desktop_file_enabled(kf);
            gtk_list_store_append(autostart_list, &it);
            gtk_list_store_set( autostart_list, &it,
                                COL_ENABLED, enabled,
                                COL_NAME, name,
                                /* COL_ICON, pix, */
                                COL_COMMENT, comment,
                                COL_DESKTOP_ID, desktop_id,
                                -1 );
            g_free(name);
            g_free(icon);
            g_free(comment);
        }
    }
}

static void load_autostart()
{
    const char* const *dirs = g_get_system_config_dirs();
    const char* const *dir;
    GHashTable* hash = g_hash_table_new_full( g_str_hash, g_str_equal, g_free, g_free );

    /* get system-wide autostart files */
    for( dir = dirs; *dir; ++dir )
        get_autostart_files_in_dir( hash, session_name, *dir );

    /* get user-specific autostart files */
    get_autostart_files_in_dir( hash, session_name, g_get_user_config_dir() );

    if( g_hash_table_size( hash ) > 0 )
    {
        GKeyFile* kf = g_key_file_new();
        g_hash_table_foreach( hash, (GHFunc)add_autostart_file, kf );
        g_key_file_free( kf );
    }
    g_hash_table_destroy( hash );
}

static void save_autostart()
{

}

static void init_list_view( GtkTreeView* view )
{
    GtkTreeViewColumn* col;
    GtkCellRenderer* render;
    autostart_list = gtk_list_store_new(N_COLS,
                                        G_TYPE_BOOLEAN,
                                        GDK_TYPE_PIXBUF,
                                        G_TYPE_STRING,
                                        G_TYPE_STRING,
                                        G_TYPE_STRING );

    render = gtk_cell_renderer_toggle_new();
    col = gtk_tree_view_column_new_with_attributes(_("Enabled"), render, "active", COL_ENABLED, NULL );
    gtk_tree_view_append_column(view, col);

    render = gtk_cell_renderer_pixbuf_new();
    col = gtk_tree_view_column_new_with_attributes(_("Application"), render, "pixbuf", COL_ICON, NULL );
    gtk_tree_view_append_column(view, col);

    render = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_start( col, render, TRUE );
    gtk_tree_view_column_set_attributes( col, render, "text", COL_NAME, NULL );

    render = gtk_cell_renderer_text_new();
    col = gtk_tree_view_column_new_with_attributes(_("Comment"), render, "text", COL_COMMENT, NULL );
    gtk_tree_view_append_column(view, col);
}

int main(int argc, char** argv)
{
    GtkBuilder *builder;
    GtkWidget *dlg, *autostarts, *wm;
    GKeyFile* kf;
    char* cfg;

#ifdef ENABLE_NLS
    bindtextdomain ( GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR );
    bind_textdomain_codeset ( GETTEXT_PACKAGE, "UTF-8" );
    textdomain ( GETTEXT_PACKAGE );
#endif

    gtk_init( &argc, &argv );
    if( argc > 1 )
        session_name = argv[1];

    builder = gtk_builder_new();
    if( !gtk_builder_add_from_file( builder, PACKAGE_DATA_DIR "/lxsession-edit/lxsession-edit.ui", NULL ) )
        return 1;

    dlg = gtk_builder_get_object( builder, "dlg" );
    autostarts = gtk_builder_get_object( builder, "autostarts" );
    wm = gtk_builder_get_object( builder, "wm" );
    g_object_unref(builder);

    gtk_dialog_set_alternative_button_order(dlg, GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, -1);

    /* autostart list */
    init_list_view(autostarts);
    load_autostart();
    gtk_tree_view_set_model( autostarts, autostart_list );

#if 0
    /* wm */
    kf = g_key_file_new();
    if( g_key_file_load_from_file(kf, file, 0, NULL )
    {

    }
    g_key_file_free(kf);
#endif

    if( gtk_dialog_run(dlg) == GTK_RESPONSE_OK )
    {
        save_autostart();
    }

    gtk_widget_destroy(dlg);
    return 0;
}
