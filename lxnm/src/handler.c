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
#include <sys/wait.h>
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

static int lxnm_handler_execute(const gchar *filename, GIOChannel *gio, gint cmd_id)
{
	int pid;
	int pfd[2];
	int status;
	gchar buffer[1024];

	/* create pipe */
	if (pipe(pfd)<0)
		return -1;

	/* fork to execute external program or scripts */
	pid = fork();
	signal(SIGCLD,SIG_IGN);

	if(pid==0) {
		close(STDOUT_FILENO);
		dup(pfd[1]);
		execl(filename, filename, NULL);
		exit(0);
	}

	/* waiting for external process */
	while(waitpid((pid_t)-1, &status, WNOHANG));

	while(read(pfd[0], buffer, sizeof(buffer))) {
		lxnm_send_message(gio, cmd_id, buffer);
	}
}

int lxnm_handler_ethernet_up(void *arg)
{
	LXNMPID id;
	char *p;
	LxThread *lxthread = arg;

	id = lxnm_pid_register(lxthread->gio);
	/* interface name */
	p = strtok((char *)lxthread->cmd, " ");
	p = strtok(NULL, "");

	if (lxnm_isifname(p)) {
		if (lxnm->setting->eth_up->method==LXNM_HANDLER_METHOD_EXECUTE) {
			setenv("LXNM_IFNAME", p, 1);
			lxnm_handler_execute(lxnm->setting->eth_up->value, lxthread->gio, id);
		}
	}

	lxnm_pid_unregister(lxthread->gio, id);
	g_free(lxthread);
	return 0;
}

int lxnm_handler_ethernet_down(void *arg)
{
	LXNMPID id;
	char *p;
	LxThread *lxthread = arg;

	id = lxnm_pid_register(lxthread->gio);
	/* interface name */
	p = strtok((char *)lxthread->cmd, " ");
	p = strtok(NULL, "");

	if (lxnm_isifname(p)) {
		if (lxnm->setting->eth_down->method==LXNM_HANDLER_METHOD_EXECUTE) {
			setenv("LXNM_IFNAME", p, 1);
			lxnm_handler_execute(lxnm->setting->eth_down->value, lxthread->gio, id);
		}
	}

	lxnm_pid_unregister(lxthread->gio, id);
	g_free(lxthread);
	return 0;
}

