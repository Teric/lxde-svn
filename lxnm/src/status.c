/*
 *      Copyright 2009 Fred Chien <cfsghost@gmail.com>
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

#include <glib.h>
#include "lxnm.h"
#include "status.h"

extern LxND *lxnm;

void lxnm_status_register(const gchar *ifname, DeviceType devtype)
{
	InterfaceStatus *ifstat;
	GList *list;
	gboolean has_reg = FALSE;

	for (list=lxnm->ifstatus;list;g_list_next(list)) {
		ifstat = (InterfaceStatus *)list->data;
		if (strcmp(ifstat->ifname, ifname)==0) {
			ifstat->ref++;
			has_reg = TRUE;
			break;
		}
	}

	if (!has_reg) {
		ifstat = g_new0(InterfaceStatus, 1);
		ifstat->ref = 1;
		ifstat->ifname = g_strdup(ifname);
		ifstat->type = devtype;
		ifstat->info = NULL;

		/* add to list */
		lxnm->ifstatus = g_list_append(lxnm->ifstatus, ifstat);
	}
}

void lxnm_status_unregister(const gchar *ifname)
{
	InterfaceStatus *ifstat;
	GList *list;

	for (list=lxnm->ifstatus;list;g_list_next(list)) {
		ifstat = (InterfaceStatus *)list->data;
		if (strcmp(ifstat->ifname, ifname)==0) {
			if (ifstat->ref>1) {
				ifstat->ref--;
			} else {
				lxnm->ifstatus = g_list_remove(lxnm->ifstatus, ifstat);

				/* release */
				g_free(ifstat->ifname);
				g_free(ifstat->info);
				g_free(ifstat);
			}
			break;
		}
	}
}

DeviceType lxnm_status_get_device_type(const gchar *ifname)
{
	if (lxnm_iswireless(ifname))
		return LXNM_CONNECTION_TYPE_WIRELESS;
	else if (lxnm_isppp(ifname))
		return LXNM_CONNECTION_TYPE_PPP;
	else
		return LXNM_CONNECTION_TYPE_ETHERNET;
}
