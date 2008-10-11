/*
 *      lxshortcut.c
 *
 *      Copyright 2008  <pcman.tw@gmail.com>
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
#include <string.h>

#define GRP_NAME    "Desktop Entry"

static GKeyFile* kf;
static GtkWidget *dlg, *icon, *change_icon, *name, *cmd, *browse, *tooltip, *terminal, *use_sn;

static gboolean no_display = FALSE;
static char* ifile = NULL;
static char* ofile = NULL;
static GOptionEntry opt_entries[] =
{
    {"no-display", 'n', 0, G_OPTION_ARG_NONE, &no_display, NULL, NULL},
    {"input", 'i', 0, G_OPTION_ARG_FILENAME, &ifile, N_("input file"), N_("file name or desktop id")},
    {"output", 'o', 0, G_OPTION_ARG_FILENAME, &ofile, N_("file name"), NULL},
    { NULL }
};

static void on_change_icon( GtkButton* btn, GtkImage* icon )
{

}

static void on_browse_cmd( GtkButton* btn, gpointer user_data )
{

}

static GdkPixbuf* load_icon_file( const char* file_name, int size )
{
    GdkPixbuf* icon = NULL;
    char* file_path;
    const gchar** dirs = (const gchar**) g_get_system_data_dirs();
    const gchar** dir;
    for( dir = dirs; *dir; ++dir )
    {
        file_path = g_build_filename( *dir, "pixmaps", file_name, NULL );
        icon = gdk_pixbuf_new_from_file_at_scale( file_path, size, size, TRUE, NULL );
        g_free( file_path );
        if( icon )
            break;
    }
    return icon;
}

static GdkPixbuf* vfs_load_icon( GtkIconTheme* theme, const char* icon_name, int size )
{
    GdkPixbuf* icon = NULL;
    const char* file;
    GtkIconInfo* inf = gtk_icon_theme_lookup_icon( theme, icon_name, size,
                                             GTK_ICON_LOOKUP_USE_BUILTIN );
    if( G_UNLIKELY( ! inf ) )
        return NULL;

    file = gtk_icon_info_get_filename( inf );
    if( G_LIKELY( file ) )
        icon = gdk_pixbuf_new_from_file_at_scale( file, size, size, TRUE, NULL );
    else
        icon = gtk_icon_info_get_builtin_pixbuf( inf );
    gtk_icon_info_free( inf );

    if( G_LIKELY( icon ) )  /* scale down the icon if it's too big */
    {
        int width, height;
        height = gdk_pixbuf_get_height(icon);
        width = gdk_pixbuf_get_width(icon);

        if( G_UNLIKELY( height > size || width > size ) )
        {
            GdkPixbuf* scaled;
            if( height > width )
            {
                width = size * height / width;
                height = size;
            }
            else if( height < width )
            {
                height = size * width / height;
                width = size;
            }
            else
                height = width = size;
            scaled = gdk_pixbuf_scale_simple( icon, width, height, GDK_INTERP_BILINEAR );
            g_object_unref( icon );
            icon = scaled;
        }
    }
    return icon;
}

static GdkPixbuf* load_icon( const char* name, int size, gboolean use_fallback )
{
    GtkIconTheme* theme;
    char *icon_name = NULL, *suffix;
    GdkPixbuf* icon = NULL;

    if( name )
    {
        if( g_path_is_absolute( name) )
        {
            icon = gdk_pixbuf_new_from_file_at_scale( name,
                                                     size, size, TRUE, NULL );
        }
        else
        {
            theme = gtk_icon_theme_get_default();
            suffix = strchr( name, '.' );
            if( suffix ) /* has file extension, it's a basename of icon file */
            {
                /* try to find it in pixmaps dirs */
                icon = load_icon_file( name, size );
                if( G_UNLIKELY( ! icon ) )  /* unfortunately, not found */
                {
                    /* Let's remove the suffix, and see if this name can match an icon
                         in current icon theme */
                    icon_name = g_strndup( name,
                                           (suffix - name) );
                    icon = vfs_load_icon( theme, icon_name, size );
                    g_free( icon_name );
                }
            }
            else  /* no file extension, it could be an icon name in the icon theme */
            {
                icon = vfs_load_icon( theme, name, size );
            }
        }
    }
    if( G_UNLIKELY( ! icon ) && use_fallback )  /* fallback to generic icon */
    {
        theme = gtk_icon_theme_get_default();
        icon = vfs_load_icon( theme, "application-x-executable", size );
        if( G_UNLIKELY( ! icon ) )  /* fallback to generic icon */
        {
            icon = vfs_load_icon( theme, "gnome-mime-application-x-executable", size );
        }
    }
    return icon;
}

int main(int argc, char** argv)
{
    GtkBuilder* builder;
    GError* err = NULL;
    char* str = NULL;
    GdkPixbuf* pix;
    gboolean no_eng_name = FALSE;
    gsize len;

#ifdef ENABLE_NLS
    bindtextdomain ( GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR );
    bind_textdomain_codeset ( GETTEXT_PACKAGE, "UTF-8" );
    textdomain ( GETTEXT_PACKAGE );
#endif

    /* initialize GTK+ and parse the command line arguments */
    if( G_UNLIKELY( ! gtk_init_with_args( &argc, &argv, "", opt_entries, GETTEXT_PACKAGE, &err ) ) )
    {
        g_print( "Error: %s\n", err->message );
        return 1;
    }

    if( ! ifile && ! ofile )
        return 1;

    /* build the UI */
    builder = gtk_builder_new();

    gtk_builder_add_from_file( builder, PACKAGE_DATA_DIR "/lxshortcut.ui", NULL );

    dlg = gtk_builder_get_object( builder, "dlg" );
    icon = gtk_builder_get_object( builder, "icon" );
    change_icon = gtk_builder_get_object( builder, "change_icon" );
    name = gtk_builder_get_object( builder, "name" );
    cmd = gtk_builder_get_object( builder, "cmd" );
    browse = gtk_builder_get_object( builder, "browse" );
    tooltip = gtk_builder_get_object( builder, "tooltip" );
    terminal = gtk_builder_get_object( builder, "terminal" );
    use_sn = gtk_builder_get_object( builder, "use_sn" );

    gtk_dialog_set_alternative_button_order( dlg, GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, -1 );

    g_object_unref( builder );

    /* signals */
    g_signal_connect( change_icon, "clicked", G_CALLBACK(on_change_icon), icon );
    g_signal_connect( browse, "clicked", G_CALLBACK(on_browse_cmd), cmd );

    /* read the desktop entry flie */
    kf = g_key_file_new();
    if( ifile )
    {
        // FIXME: this won't work for files without full path
        if( strchr( ifile, '/' ) )  /* if this is a file path */
        {
            g_key_file_load_from_file( kf, ifile, G_KEY_FILE_KEEP_COMMENTS|G_KEY_FILE_KEEP_TRANSLATIONS, NULL );
        }
        else /* otherwise treat it as desktop id */
        {
            str = g_strconcat( "applications/", ifile, NULL );
            g_key_file_load_from_data_dirs( kf, str, NULL, G_KEY_FILE_KEEP_COMMENTS|G_KEY_FILE_KEEP_TRANSLATIONS, NULL );
            g_free( str );

            if( ! ofile )
                ofile = g_build_filename( g_get_user_data_dir(), "applications", ifile, NULL );
        }

        /* icon */
        str = g_key_file_get_string( kf, GRP_NAME, "Icon", NULL );
        pix = load_icon( str, 48, TRUE );
        gtk_image_set_from_pixbuf( icon, pix );
        g_object_unref( pix );
        g_free( str );

        /* name */
        str = g_key_file_get_locale_string( kf, GRP_NAME, "Name", NULL, NULL );
        if( ! str )
            str = g_key_file_get_string( kf, GRP_NAME, "Name", NULL );
        if( str )
        {
            gtk_entry_set_text( name, str );
            g_free( str );
        }
        else
        {
            no_eng_name = TRUE;
        }

        /* exec */
        str = g_key_file_get_string( kf, GRP_NAME, "Exec", NULL );
        if( str )
        {
            gtk_entry_set_text( cmd, str );
            g_free( str );
        }

        /* comment/tooltip */
        str = g_key_file_get_locale_string( kf, GRP_NAME, "Comment", NULL, NULL );
        if( ! str )
            str = g_key_file_get_string( kf, GRP_NAME, "Comment", NULL );
        if( str )
        {
            gtk_entry_set_text( tooltip, str );
            g_free( str );
        }

        /* terminal */
        gtk_toggle_button_set_active( terminal, g_key_file_get_boolean( kf, GRP_NAME, "Terminal", NULL ) );

        /* startup notify */
        gtk_toggle_button_set_active( use_sn, g_key_file_get_boolean( kf, GRP_NAME, "StartupNotify", NULL ) );
    }
    else /* no input file, create new shortcut */
    {
        pix = load_icon( NULL, 48, TRUE );  /* load fallback icon */
        gtk_image_set_from_pixbuf( icon, pix );
        g_object_unref( pix );

        gtk_toggle_button_set_active( use_sn, TRUE );
    }

    if( gtk_dialog_run( (GtkDialog*)dlg ) == GTK_RESPONSE_OK )
    {
        const gchar* const *langs = g_get_language_names();
        char* lang = NULL;
        /* get locale name */
        if( strcmp( langs[0], "C" ) )
        {
            /* remove encoding from locale name */
            char* sep = strchr( langs[0], '.' );
            if( sep )
                lang = g_strndup( langs[0], sep - langs[0] );
            else
                lang = g_strdup( langs[0] );
        }
        g_key_file_set_string( kf, GRP_NAME, "Encoding", "UTF-8" );
        g_key_file_set_string( kf, GRP_NAME, "Type", "Application" );

        if( ! lang || !ifile || no_eng_name )
            g_key_file_set_string( kf, GRP_NAME, "Name", gtk_entry_get_text( name ) );

        if( lang )
            g_key_file_set_locale_string( kf, GRP_NAME, "Name", lang, gtk_entry_get_text( name ) );

        g_key_file_set_string( kf, GRP_NAME, "Exec", gtk_entry_get_text( cmd ) );

        if( lang )
            g_key_file_set_locale_string( kf, GRP_NAME, "Comment", lang, gtk_entry_get_text( tooltip ) );
        else
            g_key_file_set_string( kf, GRP_NAME, "Comment", gtk_entry_get_text( tooltip ) );

        /* if Terminal key is changed, save it */
        if( gtk_toggle_button_get_active(terminal) != g_key_file_get_boolean(kf, GRP_NAME, "Terminal", NULL) )
            g_key_file_set_boolean( kf, GRP_NAME, "Terminal", gtk_toggle_button_get_active(terminal) );

        /* if StartupNotify key is changed, save it */
        if( gtk_toggle_button_get_active(use_sn) != g_key_file_get_boolean(kf, GRP_NAME, "StartupNotify", NULL) )
            g_key_file_set_boolean( kf, GRP_NAME, "StartupNotify", gtk_toggle_button_get_active(use_sn) );

        /* if this shortcut shouldn't be displayed in menu */
        if( no_display )
            g_key_file_set_boolean( kf, GRP_NAME, "NoDisplay", TRUE );

        g_free( lang );

        if( str = g_key_file_to_data( kf, &len, NULL ) )
        {
            if( ! g_file_set_contents( ofile, str, len, &err ) )
            {
                g_error_free( err );
            }
        }
    }

    gtk_widget_destroy( dlg );

    g_key_file_free( kf );

	return 0;
}
