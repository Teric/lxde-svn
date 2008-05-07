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

static gboolean
lxnetctl_read_channel(GIOChannel *gio, GIOCondition condition, gpointer data)
{
	GIOStatus ret;
	GError *err = NULL;
	gchar *msg;
	gsize len;

	if (condition & G_IO_HUP)
		g_error ("Read end of pipe died!\n");

	ret = g_io_channel_read_line (gio, &msg, &len, NULL, &err);
	if (ret == G_IO_STATUS_ERROR)
		g_error ("Error reading: %s\n", err->message);

	printf ("Read %u bytes: %s\n", len, msg);

	g_free (msg);

	return TRUE;
}

int
main(void)
{
	GIOChannel *gio;
	gsize len;
	int sockfd;
	struct sockaddr_un sa_un;

	/* create socket */
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

	gio = g_io_channel_unix_new(sockfd);
	g_io_channel_set_encoding(gio, NULL, NULL);
	g_io_add_watch(gio, G_IO_IN | G_IO_HUP, lxnetctl_read_channel, NULL);

	/* send command */
	if (g_io_channel_write_chars(gio,
				     "7 ath0 1F WEP testest",
				     -1, &len, NULL) == G_IO_STATUS_ERROR)
		g_error("Error writing!");

	g_io_channel_flush(gio, NULL);

	return 0;
}

