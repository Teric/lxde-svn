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
#include <string.h>
#include <glib.h>
#include <glib/gi18n.h>
#include "netstat.h"
#include "lxnd_client.h"
#include "wireless.h"
#include "passwd_gui.h"

struct passwd_resp {
	ap_setting *aps;
	GtkEntry *entry;
};

char *wireless_auth_name[] = {
	"OFF",
	"WEP",
	"WPA-PSK"
};

static void passwd_gui_on_response(GtkDialog* dlg, gint response, struct passwd_resp *pr)
{
	char *cmdargs;
	//GtkEntry* entry = (GtkEntry*)user_data;

	if(G_LIKELY(response == GTK_RESPONSE_OK)) {

		cmdargs = g_strdup_printf("%s %s %s \"%s\" %s",
					pr->aps->ifname,
					asc2hex(pr->aps->essid),
					wireless_auth_name[pr->aps->en_type],
					gtk_entry_get_text(pr->entry),
					pr->aps->apaddr);

/*
		cmdargs = g_strdup_printf("%s %s WEP %s %s",
					pr->aps->ifname,
					pr->aps->essid,
					gtk_entry_get_text(pr->entry),
					pr->aps->apaddr);
*/
		lxnetdaemon_send_command(pr->aps->gio, LXND_WIRELESS_CONNECT, cmdargs);
	}

	g_source_remove_by_user_data(pr->entry); /* remove timeout */
	gtk_widget_destroy((GtkWidget*)dlg);
}

void passwd_gui_set_style(struct pgui *pg, GtkStyle *style)
{
	gtk_widget_set_style(pg->dlg, style);
}

void passwd_gui_destroy(struct pgui *pg)
{
	gtk_widget_destroy((GtkWidget*)pg->dlg);
}

struct pgui *passwd_gui_new(ap_setting *aps)
{
	GtkWidget *msg, *inputlabel;
	GtkWidget *inputbox;
	struct pgui *pwdgui;
	struct passwd_resp *pr;

	pwdgui = malloc(sizeof(struct pgui));
	pr = malloc(sizeof(struct passwd_resp));

	/* create dialog */
	pwdgui->dlg = gtk_dialog_new_with_buttons(_("Setting Encryption Key"),
                                       NULL,
                                       GTK_DIALOG_NO_SEPARATOR,
                                       GTK_STOCK_OK, GTK_RESPONSE_OK,
                                       GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                       NULL );
	gtk_dialog_set_default_response(GTK_WINDOW(pwdgui->dlg), GTK_RESPONSE_OK);
	gtk_window_set_position(GTK_WINDOW(pwdgui->dlg), GTK_WIN_POS_CENTER);

	/* messages */
	msg = gtk_label_new(_("This wireless network was encrypted.\nYou must have the encryption key."));
	gtk_box_pack_start(((GtkDialog*)(pwdgui->dlg))->vbox, msg, FALSE, FALSE, 8);

	/* entry Box */
	inputbox = gtk_hbox_new(FALSE, 0);
	inputlabel = gtk_label_new(_("Encryption Key:"));
	gtk_box_pack_start(GTK_BOX(inputbox), inputlabel, TRUE, TRUE, 4);
	pwdgui->pentry = gtk_entry_new();
	gtk_entry_set_visibility(GTK_ENTRY(pwdgui->pentry), FALSE);
	gtk_entry_set_activates_default(GTK_ENTRY(pwdgui->pentry), TRUE);
	gtk_box_pack_start(GTK_BOX(inputbox), pwdgui->pentry, FALSE, FALSE, 4);
	gtk_box_pack_start(((GtkDialog*)(pwdgui->dlg))->vbox, inputbox, FALSE, FALSE, 8);

	/* passwd_resp structure */
	pr->aps = aps;
	pr->entry = pwdgui->pentry;

	/* g_signal */
	g_signal_connect(pwdgui->dlg, "response", G_CALLBACK(passwd_gui_on_response), pr);
//	g_object_weak_ref(pwdgui->dlg, g_free, pr);

	gtk_widget_show_all(pwdgui->dlg);

	return pwdgui;
}