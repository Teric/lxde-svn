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
    char *buf = NULL;

    if ( g_file_get_contents( filename, &buf, NULL, NULL) == TRUE ) 
    {
	/* shrug: we need to rewrite this */
	struct field *f;
	f = parse_field(buf, given_attr);
	if (f) 
	{
	    g_hash_table_insert(hash_table, g_ascii_strdown( f->attr, - 1 ), g_ascii_strdown( f->value, - 1 ) );
	    free( f->value );
	    free( f->attr );
	    free( f );
	}
	g_free( buf );
    }
}

static int get_unit_value(char *value)
{
    int n = -1;
    sscanf(value, "%d", &n);
    return n;
}

void print_battery_information(GHashTable *hash_table, int show_capacity)
{
    int battery_num = 1;
    int remaining_capacity = -1;
    int remaining_energy = -1;
    int present_rate = -1;
    int voltage = -1;
    int design_capacity = -1;
    int design_capacity_unit = -1;
    int last_capacity = -1;
    int last_capacity_unit = -1;
    int hours, minutes, seconds;
    int percentage;
    char *state = NULL, *poststr;
    int type_battery = TRUE;
    char capacity_unit[4] = "mAh";
    gpointer value_p;
    
    if ( ( value_p = g_hash_table_lookup( hash_table, "remaining capacity" ) ) != NULL ) 
    {
	remaining_capacity = get_unit_value((gchar*) value_p);
	if (!state)
	    state = "available";
    }
    if ( ( value_p = g_hash_table_lookup( hash_table, "charge_now") ) != NULL ) 
    {
	remaining_capacity = get_unit_value((gchar*) value_p ) / 1000;
	if (!state)
	    state = "available";
    }
    if ( ( value_p = g_hash_table_lookup( hash_table, "energy_now") ) != NULL ) 
    {
	remaining_energy = get_unit_value((gchar*) value_p) / 1000;
	if (!state)
	    state = ("available");
    }
    
    if ( ( value_p = g_hash_table_lookup( hash_table, "present rate") ) != NULL ) 
	present_rate = get_unit_value((gchar*) value_p);

    if ( ( value_p = g_hash_table_lookup( hash_table, "current_now") ) != NULL ) 
	present_rate = get_unit_value((gchar*) value_p)/ 1000;

    if ( ( value_p = g_hash_table_lookup( hash_table, "last full capacity") ) != NULL )  
    {
	last_capacity = get_unit_value((gchar*) value_p);
    	if (!state)
	    state = ("available");
    }

    if ( ( value_p = g_hash_table_lookup( hash_table, "charge_full") ) != NULL )  
    {
	last_capacity = get_unit_value((gchar*) value_p) / 1000;
    	if (!state)
	    state = ("available");
    }

    if ( ( value_p = g_hash_table_lookup( hash_table, "energy_full") ) != NULL )  
    {
	last_capacity_unit = get_unit_value((gchar*) value_p) / 1000;
    	if (!state)
	    state = ("available");
    }
    
    if ( ( value_p = g_hash_table_lookup( hash_table, "charge_full_design") ) != NULL )  
	design_capacity = get_unit_value((gchar*) value_p) / 1000;

    if ( ( value_p = g_hash_table_lookup( hash_table, "energy_full_design") ) != NULL )  
	design_capacity_unit = get_unit_value((gchar*) value_p) / 1000;

    if ( ( value_p = g_hash_table_lookup( hash_table, "type") ) != NULL )  
	type_battery = (strcasecmp((gchar*) value_p, "battery") == 0);

    if ( ( value_p = g_hash_table_lookup( hash_table, "charging state") )!= NULL )      
	state = (gchar*) value_p;

    if ( ( value_p = g_hash_table_lookup( hash_table, "state") ) != NULL )      
	state = (gchar*) value_p;

    if ( ( value_p = g_hash_table_lookup( hash_table, "voltage_now") ) != NULL )  
	voltage = get_unit_value((gchar*) value_p) / 1000;
    
    if ( type_battery )
    {
	if (state) {
	    /* convert energy values (in mWh) to charge values (in mAh) if needed and possible */
	    if (last_capacity_unit != -1 && last_capacity == -1) {
		if (voltage != -1) {
		    last_capacity = last_capacity_unit * 1000 / voltage;
		} else {
		    last_capacity = last_capacity_unit;
		    strcpy(capacity_unit, "mWh");
		}
	    }
	    if (design_capacity_unit != -1 && design_capacity == -1) {
		if (voltage != -1) {
		    design_capacity = design_capacity_unit * 1000 / voltage;
		} else {
		    design_capacity = design_capacity_unit;
		    strcpy(capacity_unit, "mWh");
		}
	    }
	    if (remaining_energy != -1 && remaining_capacity == -1) {
		if (voltage != -1) {
		    remaining_capacity = remaining_energy * 1000 / voltage;
		    present_rate = present_rate * 1000 / voltage;
		} else {
		    remaining_capacity = remaining_energy;
		}
	    }
	    if (last_capacity < MIN_CAPACITY)
		percentage = 0;
	    else
		percentage = remaining_capacity * 100 / last_capacity;
	    
		if (percentage > 100)
		    percentage = 100;

		printf("%s %d: %s, %d%%", BATTERY_DESC, battery_num - 1, state, percentage);
		
		if (present_rate == -1) {
		    poststr = "rate information unavailable";
		    seconds = -1;
		} else if (!strcasecmp(state, "charging")) {
		    if (present_rate > MIN_PRESENT_RATE) {
			seconds = 3600 * (last_capacity - remaining_capacity) / present_rate;
			poststr = " until charged";
		    } else {
			poststr = "charging at zero rate - will never fully charge.";
			seconds = -1;
		    }
		} else if (!strcasecmp(state, "discharging")) {
		    if (present_rate > MIN_PRESENT_RATE) {
			seconds = 3600 * remaining_capacity / present_rate;
			poststr = " remaining";
		    } else {
			poststr = "discharging at zero rate - will never fully discharge.";
			seconds = -1;
		    }
		} else {
		    poststr = NULL;
		    seconds = -1;
		}

		if (seconds > 0) {
		    hours = seconds / 3600;
		    seconds -= 3600 * hours;
		    minutes = seconds / 60;
		    seconds -= 60 * minutes;
		    printf(", %02d:%02d:%02d%s", hours, minutes, seconds, poststr);
		} else if (poststr != NULL) {
		    printf(", %s", poststr);
		}
		
		printf("\n");

		if (show_capacity && design_capacity > 0) {
		    if (last_capacity <= 100) {
			/* some broken systems just give a percentage here */
			percentage = last_capacity;
			last_capacity = percentage * design_capacity / 100;
		    } else {
			percentage = last_capacity * 100 / design_capacity;
		    }
		    if (percentage > 100)
			percentage = 100;

		    printf ("%s %d: design capacity %d %s, last full capacity %d %s = %d%%\n",
			    BATTERY_DESC, battery_num - 1, design_capacity, capacity_unit, last_capacity, capacity_unit, percentage);
		}
	}
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
	gchar *type;
	GHashTable *hash_table = acpi_sys_get_info(entry);
	type = g_hash_table_lookup( hash_table, "type" );
	if ( ( type != NULL ) &&  ! ( g_ascii_strcasecmp( type, "battery" ) ) )
	{
	    g_hash_table_foreach( hash_table, ghcallback, NULL );
	    devices = g_list_append ( devices, hash_table );
	}
	else 
	    g_hash_table_destroy( hash_table );
    }
    g_dir_close( dir );
    return devices;
    
}


