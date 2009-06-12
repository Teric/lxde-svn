/* =====================================================================================
 *
 *       Filename:  gmenu-dlg.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  06/11/09 12:39:42
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Abel Alfonso Firvida Donestevez (), aafirvida@estudiantes.uci.cu
 *        Company:  UCI
 *
 * ====================================================================================*/
 

#include <gtk/gtk.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <lxpanel/plugin.h>
#include "gmenu-dlg.h"

#define RCFILE  PACKAGE_DATA_DIR  "/lxpanel/gmenu/gtkrc"

typedef struct {
    GtkWidget * button;
    char * icon;
    char * label;
    char * cmd;
}btn_t;

void on_click(GtkWidget * button, char * cmd)
{
    g_spawn_command_line_async( cmd , NULL );
}

void  gmenu_button_new( btn_t * btn,
                        gchar * icon,
                        gchar * label,
                        gchar * cmd ,
                        GtkIconSize size )
{
    btn->icon   = g_strdup(icon);
    btn->label  = g_strdup(label);
    btn->cmd    = g_strdup(cmd);
    btn->button = gtk_button_new_with_label(label);

    gtk_button_set_image((GtkButton*)btn->button,
                         gtk_image_new_from_icon_name(icon,size));
   
    gtk_button_set_relief((GtkButton*)btn->button,GTK_RELIEF_NONE);

    gtk_button_set_alignment ((GtkButton*)btn->button,0.0,0.5);

    g_signal_connect (G_OBJECT (btn->button), "clicked",
              G_CALLBACK (on_click),btn->cmd);
    
    return;
}

GtkWidget * gmenu_create_right_menubar()
{
    GtkWidget * right_menubar = gtk_vbox_new(FALSE,6);
   
    btn_t home; 
    gmenu_button_new( &home, GTK_STOCK_HOME, _("My Documents"), "pcmanfm ~/",
                      GTK_ICON_SIZE_LARGE_TOOLBAR );
    gtk_box_pack_start (GTK_BOX (right_menubar), home.button , FALSE, FALSE,0);

    btn_t package_manager; 
    gmenu_button_new( &package_manager, "summon", _("Package manager"), "",
                      GTK_ICON_SIZE_LARGE_TOOLBAR );
    gtk_box_pack_start (GTK_BOX (right_menubar), package_manager.button , FALSE, FALSE,0);

    gtk_box_pack_start (GTK_BOX (right_menubar), gtk_hseparator_new() , FALSE, FALSE,0);

    btn_t terminal; 
    gmenu_button_new( &terminal, "gnome-terminal", _("Terminal"), "lxterminal",
                      GTK_ICON_SIZE_LARGE_TOOLBAR );
    gtk_box_pack_start (GTK_BOX (right_menubar), terminal.button , FALSE, FALSE,0);

    btn_t run; 
    gmenu_button_new( &run, GTK_STOCK_EXECUTE, _("Run"), "gtkrun",
                      GTK_ICON_SIZE_LARGE_TOOLBAR );
    gtk_box_pack_start (GTK_BOX (right_menubar), run.button , FALSE, FALSE,0);

    btn_t lock; 
    gmenu_button_new( &lock, "system-lock-screen", _("Lock Screen"), "guano-lock",
                      GTK_ICON_SIZE_LARGE_TOOLBAR );
    gtk_box_pack_start (GTK_BOX (right_menubar), lock.button , FALSE, FALSE,0);

    btn_t quit; 
    gmenu_button_new( &quit, GTK_STOCK_QUIT, _("Quit"), "guano-logout",
                      GTK_ICON_SIZE_LARGE_TOOLBAR );
    gtk_box_pack_start (GTK_BOX (right_menubar), quit.button , FALSE, FALSE,0);

    return right_menubar;
}

GtkWidget * gmenu_create_pref_menubar()
{
    GtkWidget * pref_menubar = gtk_vbox_new(FALSE,6);
    gtk_widget_set_size_request(pref_menubar,170,-1);

    return pref_menubar;
}


GtkWidget * gmenu_create_all_app_menu( GtkWidget * menu )
{
    GtkWidget * mitem,
              * menubar,
              * image;

    menubar = gtk_menu_bar_new ();
    image = gtk_image_new_from_icon_name(GTK_STOCK_INDEX,
                                         GTK_ICON_SIZE_LARGE_TOOLBAR);
    mitem = gtk_image_menu_item_new_with_label ( " All Programs" );
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM(mitem),image);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(mitem),menu);
    gtk_container_set_border_width (GTK_CONTAINER (mitem), 6);
    gtk_menu_shell_append (GTK_MENU_SHELL (menubar), mitem);
    g_object_set (menubar,"pack-direction",GTK_PACK_DIRECTION_BTT,NULL);
    return menubar;
}

GtkWidget * gmenu_create_dlg_menu( GtkWidget * menu )
{
    gtk_rc_parse(RCFILE);
    GtkWidget * dlg_menu = gtk_window_new(GTK_WINDOW_TOPLEVEL),
              * hbox  = gtk_hbox_new (FALSE, 5),
              * thbox = gtk_hbox_new (FALSE, 5),
              * vbox  = gtk_vbox_new (FALSE, 3),
              * tvbox = gtk_vbox_new (FALSE, 3);

    gtk_window_set_decorated(GTK_WINDOW(dlg_menu), FALSE);
    gtk_container_set_border_width(GTK_CONTAINER(dlg_menu), 5);
    gtk_window_set_default_size(GTK_WINDOW(dlg_menu), 351, 354);
    gtk_window_set_skip_taskbar_hint(GTK_WINDOW(dlg_menu), TRUE);
    gtk_window_set_skip_pager_hint(GTK_WINDOW(dlg_menu), TRUE);
    gtk_window_set_type_hint(GTK_WINDOW(dlg_menu), GDK_WINDOW_TYPE_HINT_DIALOG);
    gtk_widget_set_name ( dlg_menu , "Gmenu");

    gtk_container_set_border_width (GTK_CONTAINER (vbox), 6); 
    gtk_box_pack_start (GTK_BOX (hbox), gmenu_create_all_app_menu( menu ) , FALSE, FALSE,0);
    gtk_box_pack_end   (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (tvbox),gmenu_create_pref_menubar(), FALSE, FALSE,0);
    gtk_box_pack_start (GTK_BOX (thbox),tvbox, FALSE, FALSE,1);
    gtk_box_pack_start (GTK_BOX (thbox),gmenu_create_right_menubar() , FALSE, FALSE,3);
    gtk_box_pack_start (GTK_BOX (vbox), thbox, TRUE, TRUE, 0);
    gtk_container_add  (GTK_CONTAINER ( dlg_menu ), vbox);

    return dlg_menu;
}
