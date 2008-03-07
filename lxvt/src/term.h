/*
 *      term.h
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

#ifndef _TERM_H_
#define _TERM_H_

#include <gtk/gtk.h>

G_BEGIN_DECLS

GtkWidget* term_page_new( const char* cwd, const char* exec, char** env );

GtkAdjustment* term_page_get_adjustment( GtkWidget* page );

void term_page_get_geometry( GtkWidget* page, GdkGeometry* hints );

G_END_DECLS

#endif