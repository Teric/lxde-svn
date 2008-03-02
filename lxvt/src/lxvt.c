/*
 *      lxvt.c - Yet another VTE-based terminal emulator
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
#include <glib/gstdio.h>
#include <glib/gi18n.h>

#include "window.h"
#include "term.h"

static char* exec_cmd = NULL;
static char* login_shell = NULL;
static char* title = NULL;
static gboolean new_tab = FALSE;

static GOptionEntry opt_ents[] = {
    { "execute", 'e', 0, G_OPTION_ARG_STRING, &exec_cmd, N_("Execute command"), NULL },
    { "login", 'l', 0, G_OPTION_ARG_NONE, &login_shell, N_("Login shell"), NULL },
    { "title", 't', 0, G_OPTION_ARG_STRING, &title, N_("Set window title"), NULL },
    { "new-tab", 'n', 0, G_OPTION_ARG_NONE, &new_tab, N_("Open in new tab of last used window"), NULL },
    { NULL }
};

int main(int argc, char** argv)
{
    GtkWidget* win;
    GError* err = NULL;

#ifdef ENABLE_NLS
    bindtextdomain ( GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR );
    bind_textdomain_codeset ( GETTEXT_PACKAGE, "UTF-8" );
    textdomain ( GETTEXT_PACKAGE );
#endif

    if( G_UNLIKELY( ! gtk_init_with_args( &argc, &argv, "", &opt_ents, GETTEXT_PACKAGE, &err ) ) )
        return 1;

    win = term_window_new();
    term_window_add_page_with_cmd( win, g_get_current_dir(), g_getenv("SHELL"), NULL );

    gtk_widget_show( win );
    gtk_main();
    return 0;
}
