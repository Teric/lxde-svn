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

#include <glib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#define LXNM_SOCKET "/var/run/lxnm.socket"

typedef unsigned int LXNMPID;

typedef struct {
	LXNMPID id;
	gint command;
	void (*callback)(LXNMPID id, gpointer data);
} Tasklist;

static gchar helpmsg[] = {
	"Usage:\n"
	"  lxnetctl [interface] up \n"
	"           [interface] down \n"
	"           [interface] scan \n"
};

static GList *list = NULL;

#if 0
static void
lxnetctl_send_message(GIOChannel *gio, const char *cmd, ...)
{
	GString *str;
	va_list args;
	const char *cmdarg;

	str = g_string_new(cmd);

	va_start(args, cmd);

	cmdarg = va_arg(args, const char *);
	while(cmdarg != NULL) {
		str = g_string_append_c(str, ' ');
		str = g_string_append(str, cmdarg);

		cmdarg = va_arg(args, const char *);
	}

	va_end(args);

	str = g_string_append_c(str, '\n');

	//netcommon_write_chars_all (channel, str->str, str->len, NULL);
}
#endif

static void lxnetctl_command_parser(gchar *cmd)
{
	Tasklist *task;
	gchar *p;

	p = strtok(cmd, " ");

	if (strcmp(p, "+OK")) {
		p = strtok(NULL, " ");

		task = g_new0(Tasklist, 1);
		task->command = (unsigned int)atoi(p);
		task->id = (unsigned int)atoi(p);

		/* add to task list */
		list = g_list_append(list, task);
	}
}

static gboolean
lxnetctl_read_channel(GIOChannel *gio, GIOCondition condition, gpointer data)
{
	GIOStatus ret;
	GError *err = NULL;
	gchar *msg;
	gchar *cmd;
	gsize len;

//	if (condition & G_IO_HUP)
//		return FALSE;

	ret = g_io_channel_read_line(gio, &msg, &len, NULL, &err);
	if (ret == G_IO_STATUS_ERROR)
		g_error ("Error reading: %s\n", err->message);

	if (len > 0) {
		printf("%s", msg);
		cmd = g_strdup(msg);
		lxnetctl_command_parser(cmd);
	}

	g_free(msg);

	if (condition & G_IO_HUP)
		return FALSE;

	return TRUE;
}

int
main(gint argc, gchar** argv)
{
	GIOChannel *gio;
	gchar *command;
	gsize len;
	int i;
	int sockfd;
	int flags;
	struct sockaddr_un sa_un;

	if (argc<3) {
		printf("%s\n", helpmsg);
		return 0;
	}

	/* crate socket */
	sockfd = socket(PF_UNIX, SOCK_STREAM, 0);
	if (sockfd < 0) {
		printf("Cannot create socket!");
		return 1;
	}

	/* Initiate socket */
	bzero(&sa_un, sizeof(sa_un));

	/* setting UNIX socket */
	sa_un.sun_family = AF_UNIX;
	snprintf(sa_un.sun_path, sizeof(sa_un.sun_path), LXNM_SOCKET);

	if (connect(sockfd, (struct sockaddr *) &sa_un, sizeof (sa_un)) < 0) {
		printf("Cannot connect!");
		return 1;
	}

	flags = fcntl(sockfd, F_GETFL, 0);
        fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

	gio = g_io_channel_unix_new(sockfd);
	g_io_channel_set_encoding(gio, NULL, NULL);
	g_io_add_watch(gio, G_IO_IN | G_IO_HUP, lxnetctl_read_channel, NULL);

	/* send command */
/* "7 ath0 1F WEP testest",*/
	if (strncmp(argv[2], "up", 2)==0) {
		command = g_strdup_printf("1 %s\n", argv[1]);

		if (g_io_channel_write_chars(gio, command, -1, &len, NULL)==G_IO_STATUS_ERROR)
			g_error("Error writing!");

		g_free(command);
		g_io_channel_flush(gio, NULL);
	} else if (strncmp(argv[2], "down", 4)==0) {
		command = g_strdup_printf("2 %s\n", argv[1]);

		if (g_io_channel_write_chars(gio, command, -1, &len, NULL)==G_IO_STATUS_ERROR)
			g_error("Error writing!");

		g_free(command);
		g_io_channel_flush(gio, NULL);
	} else if (strncmp(argv[2], "scan", 4)==0) {
		command = g_strdup_printf("8 %s\n", argv[1]);

		if (g_io_channel_write_chars(gio, command, -1, &len, NULL)==G_IO_STATUS_ERROR)
			g_error("Error writing!");

		g_free(command);
		g_io_channel_flush(gio, NULL);

		{
			GMainLoop *loop = g_main_loop_new(NULL, FALSE);
			g_main_loop_run(loop);
		}
	}
	return 0;
}

