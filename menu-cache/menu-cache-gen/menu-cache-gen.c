/*
 *      menu-cache-gen.c
 *      
 *      Copyright 2008 PCMan <pcman@thinkpad>
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
#include <gio/gio.h>
#include <stdio.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "gmenu-tree.h"
#include "menu-cache.h"

static char* ifile = NULL;
static char* ofile = NULL;
static gboolean force = FALSE;

GOptionEntry opt_entries[] =
{
	{"force", 'f', 0, G_OPTION_ARG_NONE, &force, "Force regeneration of cache even if it's up-to-date.", NULL },
	{"input", 'i', 0, G_OPTION_ARG_FILENAME, &ifile, "Source *.menu file to read", NULL },
	{"output", 'o', 0, G_OPTION_ARG_FILENAME, &ofile, "Output file to write cache to", NULL },
	{0}
};

GHashTable* dir_hash = NULL;

void write_item_ex_info( FILE* of, const char* desktop_file )
{
	gsize len;
	GKeyFile* kf = g_key_file_new();
	if( g_key_file_load_from_file( kf, desktop_file, 0, NULL ) )
	{
		char** keys = g_key_file_get_keys( kf, "Desktop Entry", &len, NULL );
		char** key;
		char* val;
		char** vals;
		gsize n_vals;

		for( key = keys; *key; ++key )
		{
			if( g_str_has_prefix( *key, "X-" ) )
			{
				char* val = g_key_file_get_value( kf, "Desktop Entry", *key, NULL );
				fprintf( of, "%s=%s\n", *key, val );
				g_free( val );
			}
		}
		g_strfreev( keys );
	}
	g_key_file_free( kf );
}

void write_dir( FILE* of, GMenuTreeDirectory* dir )
{
	GSList* l;
	const char* str;

	fprintf( of, "+%s\n", gmenu_tree_directory_get_menu_id( dir ) );
	fprintf( of, "%s\n", gmenu_tree_directory_get_name( dir ) );
	str = gmenu_tree_directory_get_comment( dir );
	fprintf( of, "%s\n", str ? str : "" );
	fprintf( of, "%s\n", gmenu_tree_directory_get_icon( dir ) );

	fprintf( of, "%s\n", gmenu_tree_directory_get_desktop_file_path( dir ) );

	write_item_ex_info( of, gmenu_tree_directory_get_desktop_file_path( dir ) );
	fprintf( of, "\n" );	/* end of item info */

    for( l = gmenu_tree_directory_get_contents(dir); l; l = l->next )
    {
        GMenuTreeItem* item = (GMenuTreeItem*)l->data;
        GMenuTreeItemType type = gmenu_tree_item_get_type(item);

        if( type == GMENU_TREE_ITEM_DIRECTORY )
		{
			write_dir( of, (GMenuTreeDirectory*)item );
		}
        else if( type == GMENU_TREE_ITEM_ENTRY )
        {
			char* tmp;
			fprintf( of, "-%s\n", gmenu_tree_entry_get_desktop_file_id( (GMenuTreeEntry*)item ) );
            if( gmenu_tree_entry_get_is_nodisplay(item) /* || gmenu_tree_entry_get_is_excluded(item) */ )
                continue;

			fprintf( of, "%s\n", gmenu_tree_entry_get_name( item ) );
			str = gmenu_tree_entry_get_comment( item );
			fprintf( of, "%s\n", str ? str : "" );
			fprintf( of, "%s\n", gmenu_tree_entry_get_icon( item ) );

			tmp = g_path_get_dirname( gmenu_tree_entry_get_desktop_file_path( item ) );
			fprintf( of, "%s\n", tmp);
			g_free( tmp );
			fprintf( of, "%s\n", gmenu_tree_entry_get_exec( item ) );
			fprintf( of, "%c\n", gmenu_tree_entry_get_launch_in_terminal( item ) ? '1' : '0' );

			write_item_ex_info(of, gmenu_tree_entry_get_desktop_file_path( item ));
			fputs( "\n", of );
        }
		else if( type == GMENU_TREE_ITEM_SEPARATOR )
			fputs( "-\n", of );
    }
	fputs( "\n", of );
}

static void get_all_involved_dirs( MenuCacheItem* menu, GHashTable* hash )
{
	GSList* l = menu_cache_dir_get_children( menu );
	for( ; l; l = l->next )
	{
		MenuCacheItem* item = MENU_CACHE_ITEM(l->data);
		MenuCacheType type = menu_cache_item_get_type(item);
		if( type == MENU_CACHE_TYPE_APP )
		{
			if( ! g_hash_table_lookup(hash, menu_cache_app_get_file_dir( MENU_CACHE_APP(item) )) )
				g_hash_table_insert( hash, g_strdup( menu_cache_app_get_file_dir( MENU_CACHE_APP(item)) ), GINT_TO_POINTER(TRUE) );
		}
		else if( type == MENU_CACHE_TYPE_DIR )
		{
			char* dir_path = g_path_get_dirname( menu_cache_dir_get_file(MENU_CACHE_DIR(item)) );
			if( ! g_hash_table_lookup(hash, dir_path ) )
				g_hash_table_insert( hash, dir_path, GINT_TO_POINTER(TRUE) );
			else
				g_free( dir_path );
			get_all_involved_dirs( item, hash );
		}
	}
}

static gboolean is_src_newer( const char* src, const char* dest )
{
	struct stat src_st, dest_st;

	if( stat(dest, &dest_st) == -1 )
		return FALSE;

	if( stat(src, &src_st) == -1 )
		return FALSE;

	return (src_st.st_mtime > dest_st.st_mtime);
}

static gboolean is_menu_uptodate()
{
	MenuCacheDir* menu;
	struct stat menu_st, cache_st;
	GHashTable* hash;
	GList *dirs, *l;
	gboolean ret = TRUE;

	if( is_src_newer( ifile, ofile ) )
		return FALSE;

	/* FIXME: this is quite dirty and probably buggy.
	 * There should be a better way to detect changed.
	 */

	/* load the cache and check all files involved */
	menu = menu_cache_new( ofile, NULL, NULL );
	if( ! menu )
		return FALSE;

	hash = g_hash_table_new_full( g_str_hash, g_str_equal, g_free, NULL );

	get_all_involved_dirs( menu, hash );
	dirs = g_hash_table_get_keys( hash );

	for( l = dirs; l; l = l->next )
	{
		if( is_src_newer( (char*)l->data, ofile ) )
		{
			ret = FALSE;
			break;
		}
	}
	g_list_free( dirs );
	g_hash_table_destroy( hash );

	menu_cache_item_unref( menu );

	return ret;
}

int main(int argc, char** argv)
{
	GOptionContext* opt_ctx;
	GError* err = NULL;
	GMenuTree* menu_tree = NULL;
	GMenuTreeDirectory* root_dir;
	GSList* l;
	FILE *of;
	int ofd;
	char *tmp;

	opt_ctx = g_option_context_new("Generate cache for freedeskotp.org compliant menus.");
	g_option_context_add_main_entries( opt_ctx, opt_entries, NULL );
	if( ! g_option_context_parse( opt_ctx, &argc, &argv, &err ) )
	{
		g_print( err->message );
		g_error_free( err );
		return 1;
	}

	/* if the cache is already up-to-date, just leave it. */
	if( !force && is_menu_uptodate() )
	{
		return 0;
	}

    menu_tree = gmenu_tree_lookup( ifile, GMENU_TREE_FLAGS_NONE );
	if( ! menu_tree )
	{
		g_print("Error loading source menu file: %s\n", ifile);
		return 1;
	}

	/* write the tree to cache. */
	tmp = g_malloc( strlen( ofile ) + 7 );
	strcpy( tmp, ofile );
	strcat( tmp, "XXXXXX" );
	ofd = g_mkstemp( tmp );
	if( ofd == -1 )
	{
		g_print( "Error writing output file: %s\n", g_strerror(errno) );
		return 1;
	}

	of = fdopen( ofd, "w" );
	if( ! of )
	{
		g_print( "Error writing output file: %s\n", ofile );
		return 1;
	}

    root_dir = gmenu_tree_get_root_directory( menu_tree );
	write_dir( of, root_dir );

	fclose( of );

    gmenu_tree_unref( menu_tree );

	if( g_rename( tmp, ofile ) == -1 )
	{
		g_print( "Error writing output file: %s\n", g_strerror( errno ) );
	}
	g_free( tmp );

	return 0;
}
