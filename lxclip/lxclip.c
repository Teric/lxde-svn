/*
 *      lxclip.c - a small program used to hold text in the clipboard.
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

#include <gtk/gtk.h>

static char* clipboard_text = NULL;

static void on_owner_changed( GtkClipboard* clip, GdkEvent* evt, gpointer user_data )
{
    gchar* text = gtk_clipboard_wait_for_text(clip);
    if( text )
    {
        g_free( clipboard_text );
        clipboard_text = text;
    }
    else
    {
        if( G_LIKELY( clipboard_text ) )
            gtk_clipboard_set_text( clip, clipboard_text, -1 );
    }
}

int main(int argc, char** argv)
{
    GtkClipboard* clip;
    gtk_init( &argc, &argv );
    clip = gtk_clipboard_get( GDK_SELECTION_CLIPBOARD );
    g_signal_connect( clip, "owner-change", G_CALLBACK( on_owner_changed ), NULL);
    gtk_main();
    return 0;
}
