/**
 * Copyright (c) 2006 LxDE Developers, see the file AUTHORS for details.
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "plugin.h"

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gdk-pixbuf-xlib/gdk-pixbuf-xlib.h>
#include <gdk/gdk.h>
#include <string.h>
#include <stdlib.h>

#include "misc.h"
#include "bg.h"
#include "gtkbgbox.h"


//#define DEBUG
#include "dbg.h"

static GList *pcl = NULL;


/* counter for static (built-in) plugins must be greater then zero
 * so lxpanel will not try to unload them */

#define REGISTER_PLUGIN_CLASS(pc, dynamic) \
do {\
    extern plugin_class pc;\
    register_plugin_class(&pc, dynamic);\
} while (0)


static void
register_plugin_class(plugin_class *pc, int dynamic)
{
    pcl = g_list_append(pcl, pc);
    pc->dynamic = dynamic;
    if (!pc->dynamic)
        pc->count++;
    /* reloading tray results in segfault due to registering static type in dll.
     * so keep it always onboard until bug fix */
    if (!strcmp(pc->type, "tray"))
        pc->count++;
}

static void
init_plugin_class_list()
{
    ENTER;
#ifdef STATIC_SEPARATOR
    REGISTER_PLUGIN_CLASS(separator_plugin_class, 0);
#endif

#ifdef STATIC_IMAGE
    REGISTER_PLUGIN_CLASS(image_plugin_class, 0);
#endif

#ifdef STATIC_LAUNCHBAR
    REGISTER_PLUGIN_CLASS(launchbar_plugin_class, 0);
#endif

#ifdef STATIC_DCLOCK
    REGISTER_PLUGIN_CLASS(dclock_plugin_class, 0);
#endif

#ifdef STATIC_WINCMD
    REGISTER_PLUGIN_CLASS(wincmd_plugin_class, 0);
#endif

#ifdef STATIC_DIRMENU
    REGISTER_PLUGIN_CLASS(dirmenu_plugin_class, 0);
#endif

#ifdef STATIC_TASKBAR
    REGISTER_PLUGIN_CLASS(taskbar_plugin_class, 0);
#endif

#ifdef STATIC_PAGER
    REGISTER_PLUGIN_CLASS(pager_plugin_class, 0);
#endif

#ifdef STATIC_TRAY
    REGISTER_PLUGIN_CLASS(tray_plugin_class, 0);
#endif

#ifdef STATIC_MENU
    REGISTER_PLUGIN_CLASS(menu_plugin_class, 0);
#endif

#ifdef STATIC_SPACE
    REGISTER_PLUGIN_CLASS(space_plugin_class, 0);
#endif

#ifdef STATIC_ICONS
    REGISTER_PLUGIN_CLASS(icons_plugin_class, 0);
#endif

    RET();
}

GList* plugin_find_class( const char* type )
{
    GList *tmp;
    plugin_class *pc = NULL;
    for (tmp = pcl; tmp; tmp = g_list_next(tmp)) {
        pc = (plugin_class *) tmp->data;
        if (!g_ascii_strcasecmp(type, pc->type)) {
            LOG(LOG_INFO, "   already have it\n");
            break;
        }
    }
    return tmp;
}

static plugin_class*
plugin_load_dynamic( const char* type, const char* path )
{
    plugin_class *pc = NULL;
    GModule *m;
    gpointer tmpsym;
    char class_name[ 128 ];
    m = g_module_open(path, G_MODULE_BIND_LAZY);
    if (!m) {
        /* ERR("error is %s\n", g_module_error()); */
        RET(NULL);
    }
    g_snprintf( class_name, 128, "%s_plugin_class", type );

    if (!g_module_symbol(m, class_name, &tmpsym)
         || (pc = tmpsym) == NULL
         || strcmp(type, pc->type)) {
        g_module_close(m);
        ERR("%s.so is not a lxpanel plugin\n", type);
        RET(NULL);
    }
    pc->gmodule = m;
    register_plugin_class(pc, 1);
    return pc;
}

plugin *
plugin_load(char *type)
{
    GList *tmp;
    plugin_class *pc = NULL;
    plugin *plug = NULL;

    ENTER;
    if (!pcl)
        init_plugin_class_list();

    tmp = plugin_find_class( type );

    if( tmp ) {
        pc = (plugin_class *) tmp->data;
    }
#ifndef DISABLE_PLUGINS_LOADING
    else if ( g_module_supported() ) {
        char* path[ PATH_MAX ];
        g_snprintf(path, PATH_MAX, "%s/.lxpanel/plugins/%s.so", getenv("HOME"), type);
        pc = plugin_load_dynamic( type, path );
        if( !pc ) {
            g_snprintf(path, PATH_MAX, PACKAGE_LIB_DIR "/lxpanel/plugins/%s.so", type);
            pc = plugin_load_dynamic( type, path );
        }
    }
#endif 	/* DISABLE_PLUGINS_LOADING */

    /* nothing was found */
    if (!pc)
        RET(NULL);

    plug = g_new0(plugin, 1);
    g_return_val_if_fail (plug != NULL, NULL);
    plug->class = pc;
    pc->count++;
    RET(plug);
}


void plugin_put(plugin *this)
{
    plugin_class *pc = this->class;
    ENTER;
    plugin_class_unref( pc );
    g_free(this);
    RET();
}

int
plugin_start(plugin *this, char** fp)
{
    ENTER;

    DBG("%s\n", this->class->type);
    if (!this->class->invisible) {
        this->pwid = gtk_bgbox_new();
        gtk_widget_set_name(this->pwid, this->class->type);
        gtk_box_pack_start(GTK_BOX(this->panel->box), this->pwid, this->expand, TRUE,
              this->padding);
        gtk_container_set_border_width(GTK_CONTAINER(this->pwid), this->border);
        if (this->panel->transparent) {
            gtk_bgbox_set_background(this->pwid, BG_ROOT, this->panel->tintcolor, this->panel->alpha);
        }
        gtk_widget_show(this->pwid);
    }
    if (!this->class->constructor(this, fp)) {
        if (!this->class->invisible)
            gtk_widget_destroy(this->pwid);
        RET(0);
    }
    RET(1);
}


void plugin_stop(plugin *this)
{
    ENTER;
    DBG("%s\n", this->class->type);
    this->class->destructor(this);
    this->panel->plug_num--;
    if (!this->class->invisible)
        gtk_widget_destroy(this->pwid);
    RET();
}

void plugin_class_unref( plugin_class* pc )
{
    --pc->count;
    if (pc->count == 0 && pc->dynamic) {
        pcl = g_list_remove(pcl, pc);
        /* pc points now somewhere inside loaded lib, so if g_module_close
         * will touch it after dlclose (and 2.6 does) it will result in segfault */
        g_module_close(pc->gmodule);
    }
}

/*
   Get a list of all available plugin classes
   Return a newly allocated GList which should be freed with
   plugin_class_list_free( list );
*/
GList* plugin_get_available_classes()
{
    GList* classes = NULL;
    char *path, *dir_path;
    const char* file;
    GDir* dir;
    GList* l;
    plugin_class *pc;

    for( l = pcl; l; l = l->next ) {
        pc = (plugin_class*)l->data;
        classes = g_list_prepend( classes, pc );
        ++pc->count;
    }

#ifndef DISABLE_PLUGINS_LOADING
    dir_path = g_build_filename( g_get_home_dir(), ".lxpanel/plugins", NULL );
    if( dir = g_dir_open( dir_path, 0, NULL ) ) {
        while( file = g_dir_read_name( dir ) ) {
            GModule *m;
            char* type;
            if( ! g_str_has_suffix( file, ".so" ) )
                  continue;
            type = g_strndup( file, strlen(file) - 3 );
            l = plugin_find_class( type );
            if( l == NULL ) { /* If it has not been loaded */
                path = g_build_filename( dir_path, file, NULL );
                if( pc = plugin_load_dynamic( type, path ) ) {
                    ++pc->count;
                    classes = g_list_prepend( classes, pc );
                }
                g_free( path );
            }
            g_free( type );
        }
        g_dir_close( dir );
    }
    g_free( dir_path );

    if( dir = g_dir_open( PACKAGE_LIB_DIR "/lxpanel/plugins", 0, NULL ) ) {
        while( file = g_dir_read_name( dir ) ) {
            GModule *m;
            char* type;
            if( ! g_str_has_suffix( file, ".so" ) )
                  continue;
            type = g_strndup( file, strlen(file) - 3 );
            l = plugin_find_class( type );
            if( l == NULL ) { /* If it has not been loaded */
                path = g_build_filename( PACKAGE_LIB_DIR "/lxpanel/plugins", file, NULL );
                if( pc = plugin_load_dynamic( type, path ) ) {
                    ++pc->count;
                    classes = g_list_prepend( classes, pc );
                }
                g_free( path );
            }
            g_free( type );
        }
        g_dir_close( dir );
    }
#endif
    /* classes = g_list_reverse( classes ); */
    return classes;
}

void plugin_class_list_free( GList* classes )
{
   g_list_foreach( classes, plugin_class_unref, NULL );
   g_list_free( classes );
}
