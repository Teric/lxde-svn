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
#include <unistd.h>
#include <sys/wait.h>
#include "lxnm.h"
#include "thread.h"
#include "handler.h"
#include "status.h"

extern LxND *lxnm;

static InterfaceInfo *lxnm_status_info_init(DeviceType devtype)
{
	switch(devtype) {
		case LXNM_CONNECTION_TYPE_ETHERNET:
			return (InterfaceInfo *)g_new0(EthernetInfo, 1);
		case LXNM_CONNECTION_TYPE_PPP:
			return (InterfaceInfo *)g_new0(EthernetInfo, 1);
		case LXNM_CONNECTION_TYPE_WIRELESS:
			return (InterfaceInfo *)g_new0(WirelessInfo, 1);
	}
}

void lxnm_status_register(const gchar *ifname, DeviceType devtype, LXNMClient *client)
{
	InterfaceStatus *ifstat;
	GList *list;
	gboolean has_reg = FALSE;

	for (list=lxnm->ifstatus;list;g_list_next(list)) {
		ifstat = (InterfaceStatus *)list->data;

		/* the interface has been registered */
		if (strcmp(ifstat->ifname, ifname)==0) {
			ifstat->ref++;
			ifstat->clients = g_list_append(ifstat->clients, client);
			has_reg = TRUE;
			break;
		}
	}

	if (!has_reg) {
		ifstat = g_new0(InterfaceStatus, 1);
		ifstat->ref = 1;
		ifstat->ifname = g_strdup(ifname);
		ifstat->type = devtype;
		ifstat->info = lxnm_status_info_init(devtype);
		ifstat->clients = g_list_append(ifstat->clients, client);

		/* add to list */
		lxnm->ifstatus = g_list_append(lxnm->ifstatus, ifstat);
	}
}

void lxnm_status_unregister(const gchar *ifname, LXNMClient *client)
{
	InterfaceStatus *ifstat;
	GList *list;

	for (list=lxnm->ifstatus;list;g_list_next(list)) {
		ifstat = (InterfaceStatus *)list->data;
		if (strcmp(ifstat->ifname, ifname)==0) {
			if (ifstat->ref>1) {
				/* more than one client need status of this interface */
				ifstat->clients = g_list_remove(ifstat->clients, client);
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

void lxnm_status_push(InterfaceStatus *ifstat, const gchar *msg)
{
	LXNMClient *client;
	GList *list;

	/* push status to all of client */
	for (list=ifstat->clients;list;g_list_next(list)) {
		client = (LXNMClient *)list->data;
		lxnm_send_message(client->gio, msg);
	}
}

gint lxnm_status_info_update(InterfaceStatus *ifstat)
{
	int len;
	int pfd[2];
	int status;
	pid_t pid;
	gchar buffer[1024];

	/* create pipe */
	if (pipe(pfd)<0)
		return -1;

	setenv("LXNM_IFNAME", ifstat->ifname, 1);

	/* fork to execute external program or scripts */
	pid = fork();
	if (pid<0) {
		return -2;
	} else if (pid==0) {
		/* child process */
		dup2(pfd[1], STDOUT_FILENO);
		close(STDIN_FILENO);

		switch(ifstat->type) {
			case LXNM_CONNECTION_TYPE_ETHERNET:
				execlp(lxnm->setting->eth_info->value, lxnm->setting->eth_info->value, NULL);
			case LXNM_CONNECTION_TYPE_PPP:
				execlp(lxnm->setting->eth_info->value, lxnm->setting->eth_info->value, NULL);
			case LXNM_CONNECTION_TYPE_WIRELESS:
				execlp(lxnm->setting->wifi_info->value, lxnm->setting->wifi_info->value, NULL);
		}

		exit(0);
	} else { 
		/* parent process */
		close(pfd[1]);

		while((len=read(pfd[0], buffer, 1023))>0) {
			buffer[len] = '\0';
			/* FIXME: parsing me */
		}
	}
}

void lxnm_status_start()
{
	InterfaceStatus *ifstat;
	GList *list;

	/* check all of interfaces */
	for (list=lxnm->ifstatus;list;g_list_next(list)) {
		ifstat = (InterfaceStatus *)list->data;
		//lxnm_status_push(ifstat, );
	}
}
