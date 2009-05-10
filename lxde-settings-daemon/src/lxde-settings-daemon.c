/*
 *      lxde-settings.c - XSettings daemon of LXDE
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

#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <locale.h>
#include <X11/X.h>
#include <X11/Xproto.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/XKBlib.h>

#include "xsettings-manager.h"
#include "xutils.h"

typedef enum{
	LXS_RELOAD,
	LXS_EXIT,
	LXS_LAST_CMD
}LXS_CMD;

static Display* dpy;
static XSettingsManager **managers = NULL;

static Atom CMD_ATOM; /* for private client message */

static void terminate_cb (void *data)
{
	gboolean *terminated = data;

	if (*terminated)
		return;

	*terminated = TRUE;
	exit( 0 );
//	gtk_main_quit ();
}

static void merge_xrdb(const char* content, int len)
{
    char* argv[] = { "xrdb", "-merge", "-", NULL };
    GPid pid;
    int stdi, status;
    if( g_spawn_async_with_pipes(NULL, argv, NULL, G_SPAWN_SEARCH_PATH,
                        NULL, NULL, &pid, &stdi, NULL, NULL, NULL ) )
    {
        write( stdi, content, len < 0 ? strlen(content) : len );
        close(stdi);
        waitpid( pid, &status, 0 );
    }
}


/* This function is taken from Gnome's control-center 2.6.0.3 (gnome-settings-mouse.c) and was modified*/
#define DEFAULT_PTR_MAP_SIZE 128
static void set_left_handed_mouse( gboolean mouse_left_handed )
{
    unsigned char *buttons;
    gint n_buttons, i;
    gint idx_1 = 0, idx_3 = 1;

    buttons = g_alloca (DEFAULT_PTR_MAP_SIZE);
    n_buttons = XGetPointerMapping (dpy, buttons, DEFAULT_PTR_MAP_SIZE);
    if (n_buttons > DEFAULT_PTR_MAP_SIZE)
    {
        buttons = g_alloca (n_buttons);
        n_buttons = XGetPointerMapping (dpy, buttons, n_buttons);
    }

    for (i = 0; i < n_buttons; i++)
    {
        if (buttons[i] == 1)
            idx_1 = i;
        else if (buttons[i] == ((n_buttons < 3) ? 2 : 3))
            idx_3 = i;
    }

    if ((mouse_left_handed && idx_1 < idx_3) ||
        (!mouse_left_handed && idx_1 > idx_3))
    {
        buttons[idx_1] = ((n_buttons < 3) ? 2 : 3);
        buttons[idx_3] = 1;
        XSetPointerMapping (dpy, buttons, n_buttons);
    }
}

static void configure_input(GKeyFile* kf)
{
    XKeyboardControl values;

    /* Mouse settings */
    int accel_factor, accel_threshold, delay, interval;
    gboolean left_handed, beep;

    accel_factor = g_key_file_get_integer(kf, "Mouse", "AccFactor", NULL);
    accel_threshold = g_key_file_get_integer(kf, "Mouse", "AccThreshold", NULL);
    if( accel_factor || accel_threshold )
    {
        XChangePointerControl(dpy, accel_factor != 0, accel_threshold != 0,
                                 accel_factor, 10, accel_threshold);
    }

    left_handed = g_key_file_get_integer(kf, "Mouse", "LeftHanded", NULL);
    set_left_handed_mouse(left_handed);

    /* Keyboard settings */
    if(XkbGetAutoRepeatRate(dpy, XkbUseCoreKbd, &delay, &interval))
    {
        int val;
        val = g_key_file_get_integer(kf, "Keyboard", "Delay", NULL);
        if(val > 0)
            delay = val;
        val = g_key_file_get_integer(kf, "Keyboard", "Interval", NULL);
        if(val > 0)
            interval = val;
        if( val > 0 )
        {
            XkbSetAutoRepeatRate(dpy, XkbUseCoreKbd, delay, interval);
        }
    }

    beep = g_key_file_get_integer(kf, "Keyboard", "Beep", NULL);
    values.bell_percent = beep ? -1 : 0;
    XChangeKeyboardControl(dpy, KBBellPercent, &values);
}

static void load_settings()
{
	gboolean ret;
	char* file;
    GString* buf;
    char* str;
    int val;
	GKeyFile* kf = g_key_file_new();

	file = g_build_filename( g_get_user_config_dir(), "lxde/config", NULL );
	ret = g_key_file_load_from_file( kf, file, 0, NULL );
	g_free( file );

	if( ! ret )
		ret = g_key_file_load_from_file( kf, PACKAGE_DATA_DIR"/config", 0, NULL );

	if( ret )
	{
		int i;
        const char group[] = "GTK";
        char** keys, **key;

        /* Mouse cursor (does this work?) */
        str = g_key_file_get_string( kf, group, "sGtk/CursorThemeName", NULL);
        val = g_key_file_get_integer(kf, group, "iGtk/CursorThemeSize", NULL);
        if(str || val > 0)
        {
            buf = g_string_sized_new(100);
            if(str)
            {
                if(*str)
                    g_string_append_printf(buf, "Xcursor.theme:%s\n", str);
                g_free(str);
            }
            g_string_append(buf, "Xcursor.theme_core:true\n");
            if(val > 0)
                g_string_append_printf(buf, "Xcursor.size:%d\n", val);
            merge_xrdb( buf->str, buf->len );
            g_string_free(buf, TRUE);
        }

        /* Load mouse and keyboard settings */
        configure_input(kf);

        /* Load GTK+ settings */
        keys = g_key_file_get_keys( kf, group, NULL, NULL );
        for( key = keys; *key; ++key )
        {
            const char* name = *key + 1;

            switch( **key )
            {
                case 's':	/* string */
                {
                    str = g_key_file_get_string( kf, group, *key, NULL );
                    if( str )
                    {
                        for( i = 0; managers[i]; ++i )
                            xsettings_manager_set_string( managers [i], name, str );
                        g_free( str );
                    }
                    else
                    {
                        for( i = 0; managers[i]; ++i )
                            xsettings_manager_delete_setting( managers[i], name );
                    }
                    break;
                }
                case 'i':	/* integer */
                {
                    val = g_key_file_get_integer( kf, group, *key, NULL );
                    for( i = 0; managers[i]; ++i )
                        xsettings_manager_set_int( managers [i], name, val );
                    break;
                }
                case 'c':	/* color */
                {
                    gsize len = 0;
                    int* vals = g_key_file_get_integer_list( kf, group, *key, &len, NULL );
                    if( vals && len >= 3 )
                    {
                        XSettingsColor color;
                        color.red = (gushort)vals[0];
                        color.green = (gushort)vals[1];
                        color.blue = (gushort)vals[2];
                        color.alpha = (gushort)( len >3 ? vals[3] : 65535 );
                        for( i = 0; managers[i]; ++i )
                            xsettings_manager_set_color( managers [i], name, &color );
                    }
                    else
                    {
                        for( i = 0; managers[i]; ++i )
                            xsettings_manager_delete_setting( managers[i], name );
                    }
                    g_free( vals );
                    break;
                }
            }
		}

		for( i = 0; managers[i]; ++i )
			xsettings_manager_notify( managers [i] );
	}
	g_key_file_free( kf );
}

static gboolean create_settings()
{
	int n_screens = ScreenCount(dpy);
	int i;
	gboolean terminated = FALSE;

	if (xsettings_manager_check_running( dpy, n_screens) )
	{
		g_error ("You can only run one xsettings manager at a time; exiting\n");
		return FALSE;
	}

	managers = g_new (XSettingsManager *, n_screens + 1);
	for( i = 0; i < n_screens; ++i )
	{
		Screen *screen;
		screen = ScreenOfDisplay( dpy, i );
		managers [i] = xsettings_manager_new ( dpy, i, terminate_cb, &terminated);
		if (!managers [i]) {
			g_error ("Could not create xsettings manager for screen %d!\n", i);
			return FALSE;
		}
		XSelectInput( dpy, RootWindow(dpy, i), SubstructureNotifyMask | PropertyChangeMask );
	}
	managers [i] = NULL;

	return TRUE;
}

static void send_internal_command( int cmd )
{
	Window root = DefaultRootWindow(dpy);
    XEvent ev;

	memset(&ev, 0, sizeof(ev) );
	ev.xclient.type = ClientMessage;
	ev.xclient.window = root;
	ev.xclient.message_type = CMD_ATOM;
	ev.xclient.format = 8;

	ev.xclient.data.l[0] = cmd;

	XSendEvent(dpy, root, False,
			   SubstructureRedirectMask|SubstructureNotifyMask, &ev);
	XSync(dpy, False);
}

int main(int argc, char** argv)
{
	setlocale( LC_ALL, "" );

	/* Currently i18n is not needed */
/*
	bindtextdomain (GETTEXT_PACKAGE, GNOMELOCALEDIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);
*/

	dpy = XOpenDisplay( g_getenv("DISPLAY") );
	if( ! dpy )
		return 1;

	CMD_ATOM = XInternAtom( dpy, "LXDE_SETTINGS", False );

	if( argc > 1 )
	{
		if(  0 == strcmp( argv[1] , "reload") )
			send_internal_command( LXS_RELOAD );
		else if(  0 == strcmp( argv[1] , "exit") )
			send_internal_command( LXS_EXIT );
		else
			goto out;
		XCloseDisplay( dpy );
		return 0;
	}
	out:

	XGrabServer( dpy );
	if( XGetSelectionOwner( dpy, CMD_ATOM ) )
	{
		g_error( "Only one setting daemon can be executed." );
		XUngrabServer( dpy );
		XCloseDisplay( dpy );
		return 1;
	}
	XSetSelectionOwner( dpy, CMD_ATOM, DefaultRootWindow( dpy ), CurrentTime );
	XUngrabServer( dpy );

	if( ! create_settings() )
		return 1;

	load_settings();

	while( TRUE )
	{
		XEvent evt;
		XSettingsManager**mgr;

		XNextEvent( dpy, &evt );
		if( evt.type == ClientMessage && evt.xproperty.atom == CMD_ATOM )
		{
			int cmd = evt.xclient.data.b[0];
			switch( cmd )
			{
			case LXS_RELOAD:	/* reload all settings */
				load_settings();
				break;
			case LXS_EXIT:
				goto _exit;
				break;
			}
		}
		else if( evt.type ==  SelectionClear )
		{
			for( mgr = managers; *mgr; ++mgr )
			{
				if( xsettings_manager_get_window( *mgr ) == evt.xany.window )
					xsettings_manager_process_event( *mgr, &evt );
			}
		}
	}

_exit:
	XGrabServer( dpy );
	XSetSelectionOwner( dpy, CMD_ATOM, DefaultRootWindow( dpy ), None );
	XUngrabServer( dpy );

	XCloseDisplay( dpy );

	return 0;
}
