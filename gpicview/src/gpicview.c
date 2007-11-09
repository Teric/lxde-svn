/***************************************************************************
 *   Copyright (C) 2007 by PCMan (Hong Jen Yee)   *
 *   pcman.tw@gmail.com   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <string.h>

#include "pref.h"
#include "main-win.h"

static char** files = NULL;

static GOptionEntry opt_entries[] =
{
    {G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_FILENAME_ARRAY, &files, NULL, N_("[FILE]")},
    { NULL }
};

#define PIXMAP_DIR        PACKAGE_DATA_DIR "/gpicview/pixmaps/"

void register_icons()
{
    GtkIconFactory* factory = gtk_icon_factory_new();
    GdkPixbuf* pix;
    GtkIconSet* set = NULL;

#ifndef GTK_STOCK_FULLSCREEN    // before gtk+ 2.8
#define GTK_STOCK_FULLSCREEN    "gtk-fullscreen"
    pix = gdk_pixbuf_new_from_file( PIXMAP_DIR "fullscreen.png", NULL);
    set = gtk_icon_set_new_from_pixbuf(pix);
    g_object_unref( pix );
    gtk_icon_factory_add( factory, GTK_STOCK_FULLSCREEN, set );
#endif
    pix = gdk_pixbuf_new_from_file( PIXMAP_DIR"clockwise.png", NULL);
    set = gtk_icon_set_new_from_pixbuf(pix);
    g_object_unref( pix );
    gtk_icon_factory_add( factory, "gtk-clockwise", set );

    pix = gdk_pixbuf_new_from_file( PIXMAP_DIR"counterclockwise.png", NULL);
    set = gtk_icon_set_new_from_pixbuf(pix);
    g_object_unref( pix );
    gtk_icon_factory_add( factory, "gtk-counterclockwise", set );

    gtk_icon_factory_add_default( factory );
}

int main(int argc, char *argv[])
{
    GError *error = NULL;
    GOptionContext *context;
    MainWin* win;

    context = g_option_context_new ("- simple image viewer");
    g_option_context_add_main_entries (context, opt_entries, GETTEXT_PACKAGE);
    g_option_context_add_group (context, gtk_get_option_group (TRUE));
    g_option_context_parse (context, &argc, &argv, &error);
//    gtk_init( &argc, &argv );    // this is not needed if g_option_context_parse is called

#ifdef ENABLE_NLS
    bindtextdomain ( GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR );
    bind_textdomain_codeset ( GETTEXT_PACKAGE, "UTF-8" );
    textdomain ( GETTEXT_PACKAGE );
#endif

    register_icons();

    load_preferences();

    win = main_win_new();

    // FIXME: need to process multiple files...
    if( files )
    {
        if( G_UNLIKELY( *files[0] != '/' && strstr( files[0], "://" )) )    // This is an URI
        {
            char* path = g_filename_from_uri( files[0], NULL, NULL );
            main_win_open( win, path, ZOOM_NONE );
            g_free( path );
        }
        else
            main_win_open( win, files[0], ZOOM_NONE );
    }

    gtk_widget_show( GTK_WIDGET(win) );

    gtk_main();

    save_preferences();

    return 0;
}
