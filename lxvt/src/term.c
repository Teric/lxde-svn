/*
 *      term.c
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
#include <vte/vte.h>

typedef struct
{
    VteTerminal *vte;
    GPid pid;
}TermData;

GtkWidget* term_page_new( const char* cwd, const char* exec, char** env )
{
    VteTerminal* vte;
    GPid pid;

    GdkColor black = {0};

    vte = vte_terminal_new();

    vte_terminal_set_font_from_string( vte, "monospace 11" );
    vte_terminal_set_color_background( vte, &black );

    if( ! exec )
        exec = g_getenv( "SHELL" );

    if( exec )
    {
        char** argv;
        int argc;
        if( g_shell_parse_argv( exec, &argc, &argv, NULL ) )
        {
            pid = vte_terminal_fork_command(vte, argv[0], argv, env, cwd, FALSE, FALSE, FALSE );
        }
    }
    return (GtkWidget*)vte;
}

GtkAdjustment* term_page_get_adjustment( GtkWidget* page )
{
    return vte_terminal_get_adjustment( (VteTerminal*)page );
}

void term_page_get_geometry( GtkWidget* page, GdkGeometry* hints )
{
    VteTerminal* vte = (VteTerminal*)page;
    hints->base_width = vte->char_width;
    hints->base_height = vte->char_height;
    hints->min_width = vte->char_width;
    hints->min_height = vte->char_height;
    hints->width_inc = vte->char_width;
    hints->height_inc = vte->char_height;
}
