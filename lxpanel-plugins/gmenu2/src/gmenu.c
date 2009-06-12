/**
 * Copyright (c) 2008 LxDE Developers, see the file AUTHORS for details.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <gtk/gtk.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <lxpanel/plugin.h>
#include "gmenu-dlg.h"
#include "menu.h"

#define ICON_MENU "/usr/share/icons/hicolor/48x48/apps/menu.png"

typedef struct {
    Plugin* plugin;
    GtkWidget *mainw;
    GtkWidget *tray_icon;
    GtkWidget *dlg;
    GtkWidget *vscale;
    guint vscale_handler;
    int show;
}gmenu_t;

static gboolean focus_out_event(GtkWidget *widget, GdkEvent *event, gmenu_t *menu)
{
    gtk_widget_hide(menu->dlg);
    menu->show = 0;
    return FALSE;
}

static gboolean tray_icon_press(GtkWidget *widget, GdkEventButton *event, gmenu_t *menu)
{
    if( event->button == 3 )  /* right button */
    {
        GtkMenu* popup = lxpanel_get_panel_menu( menu->plugin->panel, menu->plugin, FALSE );
        gtk_menu_popup( popup, NULL, NULL, NULL, NULL, event->button, event->time );
        return TRUE;
    }

    if (menu->show==0) {
        gtk_window_set_position(GTK_WINDOW(menu->dlg), GTK_WIN_POS_MOUSE);
        gtk_widget_show_all(menu->dlg);
        menu->show = 1;
    } else {
        gtk_widget_hide(menu->dlg);
        menu->show = 0;
    }
    return TRUE;
}

static void update_display(gmenu_t* menu)
{
     gtk_image_set_from_file(menu->tray_icon, ICON_MENU);
}

static void panel_init(Plugin *p)
{
    gmenu_t *menu = p->priv;
    GtkWidget * lxmenu = ptk_app_menu_new();
    /* set show flags */
    menu->show = 0;

    /* create a new window */

    menu->dlg = gmenu_create_dlg_menu(lxmenu);
    /* Focus-out signal */
    g_signal_connect (G_OBJECT (menu->dlg), "focus_out_event",
              G_CALLBACK (focus_out_event), menu);
}

static void
gmenu_destructor(Plugin *p)
{
    gmenu_t *menu = (gmenu_t *) p->priv;

    ENTER;

    if (menu->dlg)
        gtk_widget_destroy(menu->dlg);

    g_free(menu);
    RET();
}

static int
gmenu_constructor(Plugin *p, char **fp)
{
    gmenu_t *menu;
    line s;
    GdkPixbuf *icon;
    GtkWidget *image;
    GtkIconTheme* theme;
    GtkIconInfo* info;

    ENTER;
    s.len = 256;
    menu = g_new0(gmenu_t, 1);
    menu->plugin = p;
    g_return_val_if_fail(menu != NULL, 0);
    p->priv = menu;

    /* initializing */
    panel_init(p);

    /* main */
    menu->mainw = gtk_event_box_new();

    gtk_widget_add_events(menu->mainw, GDK_BUTTON_PRESS_MASK);
    //gtk_widget_set_size_request( menu->mainw, -1 , -1 );

    g_signal_connect(G_OBJECT(menu->mainw), "button-press-event",
                         G_CALLBACK(tray_icon_press), menu);

    /* tray icon */
    menu->tray_icon = gtk_image_new();
    update_display( menu );

    gtk_container_add(GTK_CONTAINER(menu->mainw), menu->tray_icon);

    gtk_widget_show_all(menu->mainw);

    gtk_widget_set_tooltip_text( menu->mainw, _("Menu II"));

    p->pwid = menu->mainw;

    RET(1);
}


PluginClass gmenu_plugin_class = {
    fname: NULL,
    count: 0,

    type : "gmenu",
    name : N_("Menu II"),
    version: "0.1",
    description : "Windows like menu",

    constructor : gmenu_constructor,
    destructor  : gmenu_destructor,
    config : NULL,
    save : NULL
};
