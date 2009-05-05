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
#include <sys/stat.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include "lxnm.h"
#include "misc.h"
#include "thread.h"
#include "handler.h"

LxND *lxnm;

static void
lxnm_parse_command(LxThread *lxthread)
{
	gchar *p, *cmdstr;
	gchar *msg;
	gint command;
	gint len;
	pthread_t actionThread;

	/* Command */
	p = strtok((char *)lxthread->cmd, " ");
	command = atoi(p);
	switch(command) {
		case LXNM_VERSION:
			lxnm_handler_version(lxthread);
			break;
		case LXNM_DEVICE_LIST:
			lxnm_handler_device_list(lxthread);
			break;
		case LXNM_DEVICE_STATUS:
			lxnm_handler_device_status(lxthread);
			break;
		case LXNM_ETHERNET_UP:
			lxnm_handler_ethernet_up(lxthread);
			break;
		case LXNM_ETHERNET_DOWN:
			lxnm_handler_ethernet_down(lxthread);
			break;
		case LXNM_ETHERNET_REPAIR:
			lxnm_handler_ethernet_repair(lxthread);
			break;
		case LXNM_WIRELESS_UP:
			lxnm_handler_wireless_up(lxthread);
			break;
		case LXNM_WIRELESS_DOWN:
			lxnm_handler_wireless_down(lxthread);
			break;
		case LXNM_WIRELESS_REPAIR:
			lxnm_handler_wireless_repair(lxthread);
			break;
		case LXNM_WIRELESS_CONNECT:
			lxnm_handler_wireless_connect(lxthread);
			break;
		case LXNM_WIRELESS_SCAN:
			lxnm_handler_wireless_scan(lxthread);
			break;
		default:
			printf("Unknown command");
			break;
	}

}

static gboolean
lxnm_read_channel(GIOChannel *gio, GIOCondition condition, gpointer data)
{
	GIOStatus ret;
	GError *err = NULL;
	gchar *msg;
	gsize len;
	gsize term;
	LxThread *lxthread;

	if (condition & G_IO_HUP) {
		g_free(data);
		return FALSE;
	}

	ret = g_io_channel_read_line(gio, &msg, &len, &term, &err);
	if (ret == G_IO_STATUS_ERROR)
		g_error("Error reading: %s\n", err->message);

	if (len > 0) {
		/* initializing thread data structure */
		lxthread = g_new0(LxThread, 1);
		lxthread->client = (LXNMClient *)data;

//		cmd = (int)*msg;
		msg[term] = '\0';
//		printf("Command: %d\n", cmd);

		lxthread->cmd = g_strdup(msg);

		lxnm_parse_command(lxthread);
		g_free(lxthread->cmd);
		g_free(lxthread);
	}
	g_free(msg);

	return TRUE;
}

static gboolean
lxnm_accept_client(GIOChannel *source, GIOCondition condition, gpointer data G_GNUC_UNUSED)
{
	if (condition & G_IO_IN) {
		LXNMClient *client;
		GIOChannel *gio;
		int fd;
		int flags;

		/* new connection */
		fd = accept(g_io_channel_unix_get_fd(source), NULL, NULL);
		if (fd < 0)
			g_error("Accept failed: %s\n", g_strerror(errno));

		flags = fcntl(fd, F_GETFL, 0);
		fcntl(fd, F_SETFL, flags | O_NONBLOCK);

		gio = g_io_channel_unix_new(fd);
		if (!gio)
			g_error("Cannot create new GIOChannel!\n");

		g_io_channel_set_flags(gio,(GIOFlags)(g_io_channel_get_flags(gio) | G_IO_FLAG_NONBLOCK), NULL);
		g_io_channel_set_encoding(gio, NULL, NULL);

		/* Initializing Client own data structure */
		client = g_new0(LXNMClient, 1);
		client->id = lxnm->cur_cid++;
		client->gio = gio;

		g_io_add_watch(gio, G_IO_IN | G_IO_HUP, lxnm_read_channel, client);

		g_io_channel_unref(gio);
	}

	/* our listener socket hung up - we are dead */
	if (condition & G_IO_HUP)
		g_error("Server listening socket died!\n");

	return TRUE;
}

static void
lxnm_init_socket()
{
	struct sockaddr_un skaddr;
	GIOChannel *gio;

	/* create socket */
	lxnm->sockfd = socket(PF_UNIX, SOCK_STREAM, 0);
	if (lxnm->sockfd < 0)
		g_error("Cannot create socket!");

	/* Initiate socket */
	unlink(LXNM_SOCKET);
	bzero(&skaddr, sizeof(skaddr));

	/* setting UNIX socket */
	skaddr.sun_family = AF_UNIX;
	snprintf(skaddr.sun_path, sizeof(skaddr.sun_path), LXNM_SOCKET);

	/* bind to socket */
	if (bind(lxnm->sockfd, (struct sockaddr *)&skaddr, sizeof(skaddr)) < 0)
		g_error("Bind on socket failed: %s\n", g_strerror(errno));

	/* listen on socket */
	if (listen(lxnm->sockfd, 5) < 0)
		g_error("Listen on socket failed: %s\n", g_strerror(errno));

	/* owner and permision */
	if (chown(LXNM_SOCKET, 0, 0) < 0)
		g_error("Change LXNM_SOCKET owner failed: %s\n", g_strerror(errno));
	if (chmod(LXNM_SOCKET, 0666) < 0)
		g_error("Change LXNM_SOCKET permision failed: %s\n", g_strerror(errno));

	/* create I/O channel */
	gio = g_io_channel_unix_new(lxnm->sockfd);
	if (!gio)
		g_error("Cannot create new GIOChannel!\n");

	/* setting encoding */
	g_io_channel_set_encoding(gio, NULL, NULL);
	g_io_channel_set_buffered(gio, FALSE);

	/* I/O channel into the main event loop */
	if (!g_io_add_watch(gio, G_IO_IN | G_IO_HUP, lxnm_accept_client, NULL))
		g_error("Cannot add watch on GIOChannel\n");

	/* channel will automatically shutdown when the watch returns FALSE */
	g_io_channel_set_close_on_unref(gio, TRUE);
	g_io_channel_unref(gio);
}

int
main(void)
{
	GKeyFile *keyfile;
	GKeyFileFlags flags;
	GError *error = NULL;
	gchar *strings;
	pid_t pid;

	/* Run daemon in the background */
//	pid = fork();
//	if (pid>0) {
//		return 0;
//	}

	/* initiate socket for network device */
	lxnm = (LxND *)malloc(sizeof(lxnm));
	lxnm->cur_id = 0;
	lxnm->cur_cid = 0;
	lxnm->ifstatus = NULL;

	/* initiate key_file */
	keyfile = g_key_file_new();
	flags = G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS;

	/* Load config */
	if (!g_key_file_load_from_file (keyfile, PACKAGE_DATA_DIR "/lxnm/lxnm.conf", flags, &error)) {
		g_error ("[conf-file] %s", error->message);
		return -1;
	}


	lxnm->setting = (Setting *)malloc(sizeof(Setting));

	/* general setting */
	strings = g_key_file_get_string(keyfile, "general", "iflist", NULL);
	lxnm->setting->iflist = lxnm_handler_new(strings);
	g_free(strings);

	/* ethernet setting */
	strings = g_key_file_get_string(keyfile, "ethernet", "up", NULL);
	lxnm->setting->eth_up = lxnm_handler_new(strings);
	g_free(strings);

	strings = g_key_file_get_string(keyfile, "ethernet", "down", NULL);
	lxnm->setting->eth_down = lxnm_handler_new(strings);
	g_free(strings);

	strings = g_key_file_get_string(keyfile, "ethernet", "repair", NULL);
	lxnm->setting->eth_repair = lxnm_handler_new(strings);
	g_free(strings);

	strings = g_key_file_get_string(keyfile, "ethernet", "info", NULL);
	lxnm->setting->eth_info = lxnm_handler_new(strings);
	g_free(strings);

	/* wireless setting */
	strings = g_key_file_get_string(keyfile, "wireless", "up", NULL);
	lxnm->setting->wifi_up = lxnm_handler_new(strings);
	g_free(strings);

	strings = g_key_file_get_string(keyfile, "wireless", "down", NULL);
	lxnm->setting->wifi_down = lxnm_handler_new(strings);
	g_free(strings);

	strings = g_key_file_get_string(keyfile, "wireless", "repair", NULL);
	lxnm->setting->wifi_repair = lxnm_handler_new(strings);
	g_free(strings);

	strings = g_key_file_get_string(keyfile, "wireless", "connect", NULL);
	lxnm->setting->wifi_connect = lxnm_handler_new(strings);
	g_free(strings);

	strings = g_key_file_get_string(keyfile, "wireless", "scan", NULL);
	lxnm->setting->wifi_scan = lxnm_handler_new(strings);
	g_free(strings);

	strings = g_key_file_get_string(keyfile, "wireless", "info", NULL);
	lxnm->setting->wifi_info = lxnm_handler_new(strings);
	g_free(strings);

	/* LXNM main loop */
	{
		GMainLoop *loop = g_main_loop_new(NULL, FALSE);
		lxnm_init_socket();
		g_main_loop_run(loop); /* Wheee! */
	}

	close(lxnm->sockfd);
	return 0;
}

