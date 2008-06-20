/*
 *      Copyright 2008 Fred Chien <cfsghost@gmail.com>
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

#include <X11/Xutil.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <glib/gi18n.h>

#include "lxsys-nettool.h"

static gchar helpmsg[] = {
	"Usage:\n"
	"  lxsys-nettool [Options...] - LXSys-nettool is a network configurator.\n\n"
	"Options:\n"
	"  -i <interface>             Execute the argument to this option inside the terminal\n"
};

void lxsysnettool_interface_dialog(const gchar *ifname)
{
	GtkWidget *dlg;
	GtkWidget *box;
	GtkWidget *notebook;
	GtkWidget *btn_box;
	GtkWidget *btn_apply;
	GtkWidget *btn_done;
	GtkWidget *btn_cancel;
	GtkWidget *general_label;
	GtkWidget *general_box;
	GtkWidget *ipaddr_box;
	GtkWidget *ipaddr_label;
	GtkWidget *ipaddr_entry;
	GtkWidget *mask_box;
	GtkWidget *mask_label;
	GtkWidget *mask_entry;
	GtkWidget *gateway_box;
	GtkWidget *gateway_label;
	GtkWidget *gateway_entry;
	GtkWidget *dns_box;
	GtkWidget *dns_label;
	GtkWidget *dns_entry;
	gchar *dlg_title;

	/* generate title of dialog */
	dlg_title = g_strdup_printf(_("%s Settings"), ifname);

	/* create window */
	dlg = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(dlg), dlg_title);
	g_object_weak_ref(dlg, gtk_main_quit, NULL);

	/* create box for putting menubar and notebook */
	box = gtk_vbox_new(FALSE, 1);
    gtk_container_add(GTK_CONTAINER(dlg), box);

	/* create notebook */
	notebook = gtk_notebook_new();

	gtk_notebook_set_scrollable(GTK_NOTEBOOK(notebook), TRUE);
	gtk_notebook_set_show_tabs(GTK_NOTEBOOK(notebook), FALSE);
//    g_signal_connect(notebook, "switch-page", G_CALLBACK(terminal_switch_tab), terminal);
	gtk_box_pack_start(GTK_BOX(box), notebook, TRUE, TRUE, 0);

	/* create General page */
	general_label = gtk_label_new(_("General"));
	general_box = gtk_vbox_new(FALSE, 1);

	/* IP Address */
	ipaddr_box = gtk_hbox_new(FALSE, 1);
	ipaddr_label = gtk_label_new(_("IP Address:"));
	ipaddr_entry = gtk_entry_new();
	gtk_misc_set_padding(GTK_MISC(ipaddr_label), 5, 5);
	gtk_box_pack_start(GTK_BOX(ipaddr_box), ipaddr_label, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(ipaddr_box), gtk_label_new(""), TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(ipaddr_box), ipaddr_entry, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(general_box), ipaddr_box, TRUE, TRUE, 0);

	/* Netmask */
	mask_box = gtk_hbox_new(FALSE, 1);
	mask_label = gtk_label_new(_("Netmask:"));
	mask_entry = gtk_entry_new();
	gtk_misc_set_padding(GTK_MISC(mask_label), 5, 5);
	gtk_box_pack_start(GTK_BOX(mask_box), mask_label, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(mask_box), gtk_label_new(""), TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(mask_box), mask_entry, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(general_box), mask_box, TRUE, TRUE, 0);

	/* Gateway */
	gateway_box = gtk_hbox_new(FALSE, 1);
	gateway_label = gtk_label_new(_("Gateway:"));
	gateway_entry = gtk_entry_new();
	gtk_misc_set_padding(GTK_MISC(gateway_label), 5, 5);
	gtk_box_pack_start(GTK_BOX(gateway_box), gateway_label, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(gateway_box), gtk_label_new(""), TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(gateway_box), gateway_entry, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(general_box), gateway_box, TRUE, TRUE, 0);

	/* DNS */
	dns_box = gtk_hbox_new(FALSE, 1);
	dns_label = gtk_label_new(_("Name Server:"));
	dns_entry = gtk_entry_new();
	gtk_misc_set_padding(GTK_MISC(dns_label), 5, 5);
	gtk_box_pack_start(GTK_BOX(dns_box), dns_label, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(dns_box), gtk_label_new(""), TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(dns_box), dns_entry, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(general_box), dns_box, TRUE, TRUE, 0);

	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), general_box, general_label);
//    term->index = gtk_notebook_get_n_pages(GTK_NOTEBOOK(terminal->notebook)) - 1;
//    g_ptr_array_add(terminal->terms, term);
//
	/* create button */
	btn_box = gtk_hbox_new(FALSE, 1);
	btn_apply = gtk_button_new_with_mnemonic(_("_Apply"));
	btn_done = gtk_button_new_with_mnemonic(_("_OK"));
	btn_cancel = gtk_button_new_with_mnemonic(_("_Cancel"));
	gtk_button_set_image(btn_apply, gtk_image_new_from_stock(GTK_STOCK_APPLY, GTK_ICON_SIZE_BUTTON));
	gtk_button_set_image(btn_done, gtk_image_new_from_stock(GTK_STOCK_OK, GTK_ICON_SIZE_BUTTON));
	gtk_button_set_image(btn_cancel, gtk_image_new_from_stock(GTK_STOCK_CANCEL, GTK_ICON_SIZE_BUTTON));
	gtk_box_pack_start(GTK_BOX(btn_box), gtk_label_new(""), TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(btn_box), btn_apply, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(btn_box), btn_done, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(btn_box), btn_cancel, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(box), btn_box, FALSE, FALSE, 0);

	gtk_widget_show_all(dlg);
}

int main(gint argc, gchar** argv)
{
	gint i;

	gtk_init(&argc, &argv);

#ifdef ENABLE_NLS
	bindtextdomain ( GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR );
	bind_textdomain_codeset ( GETTEXT_PACKAGE, "UTF-8" );
	textdomain ( GETTEXT_PACKAGE );
#endif

	if (argc>1) {
		for (i=1;i<argc;i++) {
			if (strncmp(argv[i], "-i", 2)==0&&i+1<argc) {
				lxsysnettool_interface_dialog(argv[++i]);
				break;
			}
			printf("%s\n", helpmsg);
			return 0;
		}
	}

	gtk_main();

    return 0;
}
