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

#include <glib.h>
#include <glib/gi18n.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "lxnm.h"
#include "thread.h"
#include "handler.h"
#include "wireless.h"

extern LxND *lxnm;

static const char *protocol_name[] = {
	"NONE",
	"WEP",
	"WPA",
};

static const char *cypher_name[] = {
	"NONE",
	"WEP40",
	"TKIP",
	"WRAP",
	"CCMP",
	"WEP104",
};

static const char *key_mgmt_name[] = {
	"NONE",
	"IEEE8021X",
	"WPA-PSK",
};

LXNMHandler *lxnm_handler_new(const gchar *strings)
{
	LXNMHandler *handler;

	handler = g_new0(LXNMHandler, 1);

	if (!strings) {
		handler->method = LXNM_HANDLER_METHOD_INTERNAL;
		handler->value = NULL;
	} else if (strncmp(strings, "Execute:", 8)==0) {
		handler->method = LXNM_HANDLER_METHOD_EXECUTE;
		handler->value = g_strdup(strings+8);
	}

	return handler;
}

static int lxnm_handler_execute(const gchar *filename, GIOChannel *gio, LXNMPID cmd_id, gboolean response)
{
	int pid;
	int pfd[2];
	int status;
	int len;
	gchar cmdid[8];
	gchar buffer[1024] = { 0 };

	/* create pipe */
	if (pipe(pfd)<0)
		return -1;

	/* initalizing environment variable */
	sprintf(cmdid, "%u", cmd_id);
	setenv("LXNM_CMDID", cmdid, 1);

	/* fork to execute external program or scripts */
	pid = fork();
	signal(SIGCLD, SIG_IGN);

	if(pid==0) {
		close(STDOUT_FILENO);
		dup(pfd[1]);
		execlp(filename, filename, NULL);
		exit(0);
	}

	close(pfd[1]);

	//while(waitpid((pid_t)pid, &status, WNOHANG));

	while((len=read(pfd[0], buffer, 1023))>0) {
		buffer[len] = '\0';

		if (response) lxnm_send_message(gio, buffer);
	}

	close(pfd[0]);
	close(pfd[2]);
}

int lxnm_handler_version(LxThread *lxthread)
{
	LXNMPID id;
	gchar *msg;

	id = lxnm_pid_register(lxthread->gio, LXNM_VERSION);
	msg = g_strdup_printf("+%u " LXNM_PROTOCOL "\n", id);
	lxnm_send_message(lxthread->gio, msg);
	lxnm_pid_unregister(lxthread->gio, id);
	g_free(msg);
	return 0;
}

int lxnm_handler_ethernet_up(LxThread *lxthread)
{
	LXNMPID id;
	char *p;

	id = lxnm_pid_register(lxthread->gio, LXNM_ETHERNET_UP);
	/* interface name */
	p = strtok(NULL, " ");

	if (lxnm_isifname(p)) {
		if (lxnm->setting->eth_up->method==LXNM_HANDLER_METHOD_EXECUTE) {
			setenv("LXNM_IFNAME", p, 1);
			lxnm_handler_execute(lxnm->setting->eth_up->value, lxthread->gio, id, FALSE);
		}
	}

	lxnm_pid_unregister(lxthread->gio, id);
	return 0;
}

int lxnm_handler_ethernet_down(LxThread *lxthread)
{
	LXNMPID id;
	char *p;

	id = lxnm_pid_register(lxthread->gio, LXNM_ETHERNET_DOWN);
	/* interface name */
	p = strtok(NULL, "");

	if (lxnm_isifname(p)) {
		if (lxnm->setting->eth_down->method==LXNM_HANDLER_METHOD_EXECUTE) {
			setenv("LXNM_IFNAME", p, 1);
			lxnm_handler_execute(lxnm->setting->eth_down->value, lxthread->gio, id, FALSE);
		}
	}

	lxnm_pid_unregister(lxthread->gio, id);
	return 0;
}

int lxnm_handler_ethernet_repair(LxThread *lxthread)
{
	LXNMPID id;
	char *p;

	id = lxnm_pid_register(lxthread->gio, LXNM_ETHERNET_REPAIR);
	/* interface name */
	p = strtok(NULL, "");

	if (lxnm_isifname(p)) {
		if (lxnm->setting->eth_repair->method==LXNM_HANDLER_METHOD_EXECUTE) {
			setenv("LXNM_IFNAME", p, 1);
			lxnm_handler_execute(lxnm->setting->eth_repair->value, lxthread->gio, id, FALSE);
		}
	}

	lxnm_pid_unregister(lxthread->gio, id);
	return 0;
}

int lxnm_handler_wireless_up(LxThread *lxthread)
{
	LXNMPID id;
	char *p;

	id = lxnm_pid_register(lxthread->gio, LXNM_WIRELESS_UP);
	/* interface name */
	p = strtok(NULL, " ");

	if (lxnm_isifname(p)) {
		if (lxnm->setting->eth_up->method==LXNM_HANDLER_METHOD_EXECUTE) {
			setenv("LXNM_IFNAME", p, 1);
			lxnm_handler_execute(lxnm->setting->wifi_up->value, lxthread->gio, id, FALSE);
		}
	}

	lxnm_pid_unregister(lxthread->gio, id);
	return 0;
}

int lxnm_handler_wireless_down(LxThread *lxthread)
{
	LXNMPID id;
	char *p;

	id = lxnm_pid_register(lxthread->gio, LXNM_WIRELESS_DOWN);
	/* interface name */
	p = strtok(NULL, "");

	if (lxnm_isifname(p)) {
		if (lxnm->setting->eth_down->method==LXNM_HANDLER_METHOD_EXECUTE) {
			setenv("LXNM_IFNAME", p, 1);
			lxnm_handler_execute(lxnm->setting->wifi_down->value, lxthread->gio, id, FALSE);
		}
	}

	lxnm_pid_unregister(lxthread->gio, id);
	return 0;
}

int lxnm_handler_wireless_repair(LxThread *lxthread)
{
	LXNMPID id;
	char *p;

	id = lxnm_pid_register(lxthread->gio, LXNM_WIRELESS_REPAIR);
	/* interface name */
	p = strtok(NULL, "");

	if (lxnm_isifname(p)) {
		if (lxnm->setting->wifi_repair->method==LXNM_HANDLER_METHOD_EXECUTE) {
			setenv("LXNM_IFNAME", p, 1);
			lxnm_handler_execute(lxnm->setting->wifi_repair->value, lxthread->gio, id, FALSE);
		}
	}

	lxnm_pid_unregister(lxthread->gio, id);
	return 0;
}

int lxnm_handler_wireless_connect(LxThread *lxthread)
{
	LXNMPID id;
	char *p;

	id = lxnm_pid_register(lxthread->gio, LXNM_WIRELESS_CONNECT);
	/* <interface> <essid> <apaddr> <key> <protocol> <key_mgmt> <grpup> <pairwise> */
	/* interface name */
	p = strtok(NULL, "");

	if (lxnm_isifname(p)) {
		if (lxnm->setting->wifi_repair->method==LXNM_HANDLER_METHOD_EXECUTE) {
			setenv("LXNM_IFNAME", p, 1);
			/* ESSID */
			p = strtok(NULL, " ");
			setenv("LXNM_WIFI_ESSID", hex2asc(p), 1);
			/* AP Addr */
			p = strtok(NULL, " ");
			setenv("LXNM_WIFI_APADDR", p, 1);
			/* key */
			p = strtok(NULL, " ");
			setenv("LXNM_WIFI_KEY", hex2asc(p), 1);
			/* protocol */
			p = strtok(NULL, " ");
			setenv("LXNM_WIFI_PROTO", protocol_name[atoi(p)], 1);
			/* key_mgmt */
			p = strtok(NULL, " ");
			setenv("LXNM_WIFI_KEYMGMT", key_mgmt_name[atoi(p)], 1);
			/* group */
			p = strtok(NULL, " ");
			setenv("LXNM_WIFI_GROUP", cypher_name[atoi(p)], 1);
			/* pairwise */
			p = strtok(NULL, " ");
			setenv("LXNM_WIFI_PAIRWISE", cypher_name[atoi(p)], 1);

			lxnm_handler_execute(lxnm->setting->wifi_connect->value, lxthread->gio, id, FALSE);
		}
	}

	lxnm_pid_unregister(lxthread->gio, id);
	return 0;
}

int lxnm_handler_wireless_scan(LxThread *lxthread)
{
	LXNMPID id;
	int iwsockfd;
	char *p;
	APLIST *aplist;
	APLIST *ptr;
	ap_info *apinfo;

	/* interface name */
	p = strtok(NULL, "");

	if (lxnm_isifname(p)) {
		id = lxnm_pid_register(lxthread->gio, LXNM_WIRELESS_SCAN);

		switch (lxnm->setting->wifi_scan->method) {
			case LXNM_HANDLER_METHOD_INTERNAL:
				iwsockfd = iw_sockets_open();
				aplist = wireless_scanning(iwsockfd, p);
				if (aplist) {
					ptr = aplist;
					do {
						apinfo = ptr->info;
						ptr = ptr->next;
					} while (ptr);
				}
				break;
			case LXNM_HANDLER_METHOD_EXECUTE:
				setenv("LXNM_IFNAME", p, 1);
				lxnm_handler_execute(lxnm->setting->wifi_scan->value, lxthread->gio, id, TRUE);
				break;
		}
		lxnm_pid_unregister(lxthread->gio, id);
	}

	return 0;
}
