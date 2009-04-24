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
#include "lxnm.h"
#include "thread.h"
#include "handler.h"

extern LxND *lxnm;

LXNMHandler *lxnm_handler_new(const gchar *strings)
{
	LXNMHandler *handler;

	handler = g_new0(LXNMHandler, 0);

	if (!strings) {
		handler->method = LXNM_HANDLER_METHOD_INTERNAL;
		handler->value = NULL;
	} else if (strcmp(strings, "Execute:")==0) {
		handler->method = LXNM_HANDLER_METHOD_EXECUTE;
		handler->value = g_strdup(strings+8);
	}

	return handler;
}

int lxnm_handler_ethernet_up(void *arg)
{
	LXNMPID id;
	char *p;
	LxThread *lxthread = arg;

	id = lxnm_pid_register(lxthread->gio);
	/* interface name */
	p = strtok((char *)lxthread->cmd+2, " ");
	if (lxnm_isifname(p)) {
		setenv("LXNM_IFNAME", p, 1);
		return system(lxnm->setting->eth_up);
	}

	lxnm_pid_unregister(lxthread->gio, id);
	g_free(lxthread);
	return 0;
}

