/*
 *      batt_sys.h
 *      
 *      Copyright 2008 Juergen Hötzel <juergen@archlinux.org>
 * 		
 * 	Parts shameless stolen and glibified from acpi package  
 * 	Copyright (C) 2001  Grahame Bowland <grahame@angrygoats.net>
 *	(C) 2008-2009  Michael Meskes  <meskes@debian.org> 
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


#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "batt_sys.h"
#include <glib/gstdio.h>

/* shrug: get rid of this */
#include <stdlib.h>
#include <string.h>

struct file_list {
    char *file;
    char *attr;
};

static struct file_list sys_list[] = {
    {"current_now", "current_now"},
    {"charge_now", "charge_now"},
    {"energy_now", "energy_now"},
    {"voltage_now", "voltage_now"},
    {"voltage_min_design", "voltage_min_design"},
    {"charge_full", "charge_full"},
    {"energy_full", "energy_full"},
    {"charge_full_design", "charge_full_design"},
    {"energy_full_design", "energy_full_design"},
    {"online", "online"},
    {"status", "charging state"},
    {"type", "type"},
    {"temp", "sys_temp"},
    {"trip_point_0_type", "trip_point_0_type"},
    {"trip_point_0_temp", "trip_point_0_temp"},
    {"trip_point_1_type", "trip_point_1_type"},
    {"trip_point_1_temp", "trip_point_1_temp"},
    {"trip_point_2_type", "trip_point_2_type"},
    {"trip_point_2_temp", "trip_point_2_temp"},
    {"trip_point_3_type", "trip_point_3_type"},
    {"trip_point_3_temp", "trip_point_3_temp"},
    {"trip_point_4_type", "trip_point_4_type"},
    {"trip_point_4_temp", "trip_point_4_temp"},
    {"cur_state", "cur_state"},
    {"max_state", "max_state"},
    {NULL, NULL}
};

struct field {
    char *attr;
    char *value;
};


static struct field *parse_field(char *buf, char *given_attr)
{
    struct field *rval;
    char *p;
    char *attr;
    char *value;
    int has_attr = 0;

    attr = calloc(BUF_SIZE, sizeof(char));
    value = calloc(BUF_SIZE, sizeof(char));
    rval = malloc(sizeof(struct field));
    if (!rval || !attr || !value) {
	fprintf(stderr, "Out of memory. Could not allocate memory in parse_field.\n");
	exit(1);
    }

    p = buf;
    if (!given_attr) {
	while (*(p++)) {
	    if (*p == ':') {
		strncpy(attr, buf, p - buf);
		has_attr = 1;
		break;
	    }
	}
	if (!has_attr) {
	    free(attr);
	    free(value);
	    free(rval);
	    return NULL;
	}
	if (*p == ' ')
	    p++;
	while (*(p++)) {
	    if (*p != ' ')
		break;
	}
    } else {
	strncpy(attr, given_attr, BUF_SIZE);
    }
    strncpy(value, p, BUF_SIZE);
    if (attr[strlen(attr) - 1] == '\n')
	attr[strlen(attr) - 1] = '\0';
    if (value[strlen(value) - 1] == '\n')
	value[strlen(value) - 1] = '\0';
    rval->attr = attr;
    rval->value = value;
    return rval;
}

static void parse_info_file(GHashTable *hash_table, 
			    char *filename, char *given_attr)
{
    FILE *fd;
    char *buf = NULL;

    if ( g_file_get_contents( filename, &buf, NULL, NULL) == TRUE ) 
    {
	/* shrug: we need to rewrite this */
	struct field *f;
	f = parse_field(buf, given_attr);
	if (f) 
	    g_hash_table_insert(hash_table, f->attr, f->value );
	g_free( buf );
    }
}


GHashTable *acpi_sys_get_info(const gchar *device_name ) {

    int i = 0;
    GHashTable *hash_table = g_hash_table_new( g_str_hash, g_str_equal );
    while ( sys_list[i].file != NULL ) 
    {
	GString *filename = g_string_new( ACPI_PATH_SYS_POWER_SUPPY );
	g_string_append_printf ( filename, "/%s/%s", device_name, 
				 sys_list[i].file );
	parse_info_file(hash_table, filename->str, sys_list[i].attr);
	g_string_free( filename, TRUE );
	i++;
    }

    return hash_table;
}

void  ghcallback (gpointer key_p,
			       gpointer value_p,
			       gpointer user_data) 
{
  gchar *str = (gchar*) key_p;
  gchar *val = (gchar* ) value_p;
  
  g_message("str:%s:%s", str, val);
  
}


GList  *acpi_sys_find_devices()
{
    GList *devices = NULL;
    GError * error = NULL;
    const gchar *entry;
    GDir * dir = g_dir_open( ACPI_PATH_SYS_POWER_SUPPY, 0, &error );
    if ( dir == NULL ) 
    {
	g_warning( "NO ACPI/sysfs support in kernel: %s", error->message );
	return NULL;
    }
    while ( ( entry = g_dir_read_name (dir) ) != NULL )  
    {
	g_message( "Parsing %s", entry );
	
	GHashTable *hash_table = acpi_sys_get_info(entry);
	g_hash_table_foreach( hash_table, ghcallback, NULL );
	devices = g_list_append ( devices, hash_table );
    }
    g_dir_close( dir );
    return devices;
    
}


