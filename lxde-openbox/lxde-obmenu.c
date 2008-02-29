/*
 *      lxde-obmenu.c - generate localized menu for OpenBox under LXDE
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

#include <stdio.h>
#include <string.h>
#include <glib/gi18n.h>
#include <locale.h>

void print_config_menu()
{
    printf(
        "<openbox_pipe_menu>"
        "<item label=\"" );
    printf( _( "Window Behavior" ) );
    printf(
        "\">"
        "<action name=\"Execute\">"
            "<startupnotify><enabled>yes</enabled><icon>openbox</icon></startupnotify>"
            "<command>obconf</command>"
        "</action>"
        "</item>"
        /*
        <item label="Reconfigure">
        <action name="Reconfigure" />
        </item>
        */
        "</openbox_pipe_menu>"
    );
}

void print_root_menu()
{
    printf(
        "<openbox_pipe_menu>\n"
        "<separator label=\"LXDE\" />\n"
        "<menu id=\"client-list-menu\" />\n"
        "<separator />\n" );

    /* The Settings sub menu */
    printf(
        "<menu id=\"config-menu\" label=\"" );
    printf( _("Settings") );
    printf( "\" execute=\"lxde-obmenu config\" />\n"
        "<separator />\n"
        "<item label=\"" );
    printf( _("Logout")  );
    printf(
        "\">"
        "<action name=\"Execute\"><command>lxde-logout</command></action>"
        "</item>\n"
        "</openbox_pipe_menu>"
    );
}

int main(int argc, char** argv)
{
    setlocale( LC_ALL, "" );

#ifdef ENABLE_NLS
    bindtextdomain ( GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR );
    bind_textdomain_codeset ( GETTEXT_PACKAGE, "UTF-8" );
    textdomain ( GETTEXT_PACKAGE );
#endif

    printf( "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" );

    if( argc < 2 )
    {
        print_root_menu();
        return 0;
    }

    if( 0 == strcmp( argv[1], "config" ) )
        print_config_menu();

    return 0;
}
