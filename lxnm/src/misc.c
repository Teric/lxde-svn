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
#include <sys/ioctl.h>
#include <net/if.h>
#include <iwlib.h>
#include "lxnm.h"
#include "misc.h"

extern LxND *lxnm;

char *hex2asc(char *hexsrc)
{
	char *buf, *tmp;
	char c[3];

	buf = malloc(sizeof(char)+strlen(hexsrc)/2);
	tmp = buf;

	for (;*hexsrc!='\0';hexsrc+=2) {
		c[0] = *hexsrc;
		c[1] = *(hexsrc+1);
		c[2] = '\0';

		*tmp = strtol(c, NULL, 16);
		tmp++;
	}

	*tmp = '\0';
	return buf;
}

gboolean lxnm_isifname(const char *ifname)
{
#ifdef OS_Linux
	struct ifreq ifr;
	bzero(&ifr, sizeof(ifr));

	strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
	if (ioctl(lxnm->sockfd, SIOCGIFFLAGS, &ifr)<0)
		return FALSE;

	return TRUE;
#else
	/* FIXME: Help us to support other operating systems */
	return FALSE;
#endif
}

gboolean lxnm_isppp(const char *ifname)
{
#ifdef OS_Linux
	struct ifreq ifr;
	bzero(&ifr, sizeof(ifr));

	strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
	if (ioctl(lxnm->sockfd, SIOCGIFFLAGS, &ifr)<0)
		return FALSE;

	if (ifr.ifr_flags & IFF_POINTOPOINT)
		return TRUE;
	else
		return FALSE;
#else
	/* FIXME: Help us to support other operating systems */
	return FALSE;
#endif
}

gboolean lxnm_iswireless(const gchar *ifname)
{
#ifdef OS_Linux
	gint iwsockfd;
	gint has_iwrange;
	struct iw_range iwrange;

	/* open socket for wireless */
	iwsockfd = iw_sockets_open();

	/* get wireless device informations */
	has_iwrange = (iw_get_range_info(iwsockfd, ifname, &iwrange)>=0);

	if (!(has_iwrange) || (iwrange.we_version_compiled < 14))
		return FALSE;
	else
		return TRUE;
#else
	/* FIXME: Help us to support other operating systems */
	return FALSE;
#endif
}
