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

#ifndef _WINDOW_H_
#define _WINDOW_H_

#include <gtk/gtk.h>

G_BEGIN_DECLS

GtkWidget* term_window_new();

int term_window_add_page( GtkWidget* win, GtkWidget* page );
int term_window_add_page_with_cmd( GtkWidget* win, const char* cwd, const char* exec, char** env );
int term_window_get_cur_tab();
GtkWidget* term_window_get_nth_tab( GtkWidget* win, int idx );

G_END_DECLS

#endif

