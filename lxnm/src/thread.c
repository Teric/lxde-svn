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

extern LxND *lxnm;

void lxnm_send_message(GIOChannel *gio, const gchar *msg)
{
	g_io_channel_write_chars(gio, msg, -1, NULL, NULL);
	g_io_channel_flush(gio, NULL);
}

LXNMPID lxnm_pid_register(GIOChannel *gio, gint command)
{
	gchar msg[32];

	sprintf(msg, "+OK %d %u\n", command, lxnm->cur_id);
	g_io_channel_write_chars(gio, msg, -1, NULL, NULL);
	g_io_channel_flush(gio, NULL);

	return lxnm->cur_id++;
}

void lxnm_pid_unregister(GIOChannel *gio, LXNMPID id)
{
	gchar msg[32];

	sprintf(msg, "+DONE %u\n", id);
	g_io_channel_write_chars(gio, msg, -1, NULL, NULL);
	g_io_channel_flush(gio, NULL);
}
