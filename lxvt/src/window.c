/*
 *      window.c
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

#include <gdk/gdkkeysyms.h>

#include "window.h"
#include "term.h"
#include "glib-mem.h"

typedef struct{
    GtkWidget* win;
    GtkNotebook* nb;
    GtkScrollbar* scroll;
    GtkBox* hbox;
}TermWin;

static void on_switch_page( GtkNotebook* nb, GtkWidget* page, gint n, TermWin* term_win );
static gboolean on_close( GtkWidget* w, GdkEvent* evt, TermWin* term_win );
static gboolean on_key_press( GtkWidget* w, GdkEventKey* evt, TermWin* term_win );

static n_windows = 0;
static GQuark term_win_data_id = 0;

#define term_win_get_data(win)  (TermWin*)g_object_get_qdata( (GObject*)win, term_win_data_id );

GtkWidget* term_window_new()
{
    TermWin* term_win = g_slice_new0( TermWin );
    term_win->win = gtk_window_new( GTK_WINDOW_TOPLEVEL );

    term_win->hbox = (GtkBox*)gtk_hbox_new(  FALSE, 0 );
    term_win->nb = (GtkNotebook*)gtk_notebook_new();
    term_win->scroll = (GtkScrollbar*)gtk_vscrollbar_new(NULL);

    gtk_box_pack_start( term_win->hbox, term_win->nb, TRUE, TRUE, 0 );
    gtk_box_pack_start( term_win->hbox, (GtkWidget*)term_win->scroll, FALSE, TRUE, 0 );

    gtk_widget_show_all( (GtkWidget*)term_win->hbox );
    gtk_container_add( (GtkContainer*)term_win->win, (GtkWidget*)term_win->hbox );

    g_signal_connect( term_win->nb, "switch_page", G_CALLBACK( on_switch_page ), term_win );
    g_signal_connect( term_win->win, "delete-event", G_CALLBACK(on_close), term_win );
    g_signal_connect( term_win->win, "key-press-event", G_CALLBACK(on_key_press), term_win );

    if( G_UNLIKELY( 0 == term_win_data_id ) )
        term_win_data_id = g_quark_from_static_string( "TermWinData" );

    g_object_set_qdata( (GObject*)term_win->win, term_win_data_id, term_win );

    ++n_windows;

    return term_win->win;
}

int term_window_add_page( GtkWidget* win, GtkWidget* page )
{
    int idx;
    TermWin* term_win = term_win_get_data( win );
    gtk_widget_show( page );

    idx = gtk_notebook_append_page( term_win->nb, page, gtk_label_new("Term") );
#if GTK_CHECK_VERSION( 2, 10, 0 )
    gtk_notebook_set_tab_reorderable( term_win->nb, page, TRUE );
#endif

    return idx;
}

int term_window_add_page_with_cmd( GtkWidget* win, const char* cwd, const char* exec, char** env )
{
    GtkWidget* page;
    page = term_page_new( cwd, exec, env );
    return term_window_add_page( win, page );
}

int term_window_get_cur_tab()
{

}

GtkWidget* term_window_get_nth_tab( GtkWidget* win, int idx )
{
    return NULL;
}

void term_window_update_geometry( GtkWidget* win, GtkWidget* page )
{
    TermWin* term_win = term_win_get_data(win);
    GdkGeometry hints;
    term_page_get_geometry( page, &hints );
    gtk_window_set_geometry_hints (GTK_WINDOW (win),
                       page,
                       &hints,
                       GDK_HINT_RESIZE_INC |
                       GDK_HINT_MIN_SIZE |
                       GDK_HINT_BASE_SIZE);
}

void on_switch_page( GtkNotebook* nb, GtkWidget* page, gint n, TermWin* term_win )
{
    GtkWidget* term_page = gtk_notebook_get_nth_page( nb, n );
    gtk_range_set_adjustment( (GtkRange*)term_win->scroll, term_page_get_adjustment(term_page) );
    gtk_widget_grab_focus( term_page );
    term_window_update_geometry( term_win->win, term_page );
}

gboolean on_close( GtkWidget* w, GdkEvent* evt, TermWin* term_win )
{
    --n_windows;
    if( 0 == n_windows )
        gtk_main_quit();

    g_slice_free( TermWin, term_win );

    gtk_widget_destroy( w );
    return FALSE;
}

gboolean on_key_press( GtkWidget* w, GdkEventKey* evt, TermWin* term_win )
{
    gboolean alt = ((evt->state & GDK_MOD1_MASK) == GDK_MOD1_MASK);
    gboolean ctrl_shift = ((evt->state & (GDK_CONTROL_MASK|GDK_SHIFT_MASK)) == (GDK_CONTROL_MASK|GDK_SHIFT_MASK));
    int key = g_ascii_toupper( evt->keyval );

    if( alt )
    {
        /* Alt + 0 ~ 9 switch tabs */
        if( key >= GDK_0 && key <= GDK_9 )
        {
            int idx = key - GDK_1;
            if( idx < 0 )
                idx = 9;
            if( idx < gtk_notebook_get_n_pages( term_win->nb ) )
            {
                gtk_notebook_set_current_page( term_win->nb, idx );
                gtk_widget_grab_focus( gtk_notebook_get_nth_page(term_win->nb, idx) );
            }
        }
        else if( key == GDK_T ) /* Alt + T => new tab */
        {
            int idx = term_window_add_page_with_cmd( w, NULL, NULL, NULL );
            gtk_notebook_set_current_page( term_win->nb, idx );
            gtk_widget_grab_focus( gtk_notebook_get_nth_page(term_win->nb, idx) );
        }
        else if( key == GDK_N ) /* Alt + N => new window */
        {
            GtkWidget* win = term_window_new();
            term_window_add_page_with_cmd( win, g_get_current_dir(), g_getenv("SHELL"), NULL );
            gtk_widget_show( win );
        }
        else if( key == GDK_Z ) /* Alt + Z => previous tab */
        {
            int idx = gtk_notebook_get_current_page( term_win->nb );
            if( --idx >= 0 )
            {
                gtk_notebook_set_current_page( term_win->nb, idx );
                gtk_widget_grab_focus( gtk_notebook_get_nth_page(term_win->nb, idx) );
            }
        }
        else if( key == GDK_X ) /* Alt + X => next tab */
        {
            int idx = gtk_notebook_get_current_page( term_win->nb );
            if( ++idx < gtk_notebook_get_n_pages( term_win->nb ) )
            {
                gtk_notebook_set_current_page( term_win->nb, idx );
                gtk_widget_grab_focus( gtk_notebook_get_nth_page(term_win->nb, idx) );
            }
        }
        else
        {
            return FALSE;
        }
        return TRUE;
    }
    return FALSE;
}
