/**
 * GConf purge: Remove unnecessary translations from gconf schema
 * 
 * Copyright (c) 2006.10.15 Hong Jen Yee (PCMan)   <pcman.tw( at )gmail.com>
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
 *
 * NOTE: This program requires glib 2.4+.
 * To compile this program, type following command:
 * 
 * gcc `pkg-config glib-2.0 --cflags --libs` -o gconf-purge gconf-purge.c
 *
 * Usage: sudo gconf-purge <dir path>
 * <dir path> is the directory containing gconf schemas.
 * Normally it's /usr/share/gconf/schemas
 */

#include <glib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

static gsize saved_size = 0;

static void purge_file( const char* file_path, struct stat* statbuf )
{
    gsize ori_len;
    char* data;

    if( g_file_get_contents( file_path, &data, &ori_len, NULL ) )
    {
        char* line;
        GString* result = g_string_sized_new(ori_len);

        if( G_LIKELY(line = strtok( data, "\n" )) )
        {
            gboolean do_purge = FALSE;
            gboolean skip_comment = FALSE;
            do
            {
                const char** langs = NULL;
                char* locale_tag;
                char* comment;
                comment = strstr(line, "<!--");
                if( comment ) /* remove useless xml comments */
                {
                    *comment = 0;
                    g_string_append(result, line);
                    skip_comment = TRUE;
                    continue;
                }
                // if( line[0] == '\0' ) /* remove empty lines */
                //    continue;
                if( G_UNLIKELY(skip_comment) )
                {
                    if( ( comment = strstr(line, "-->") ) ) /* end of comment */
                    {
                        comment += 3;
                        skip_comment = FALSE;
                        if( *comment )
                            g_string_append( result, comment );
                    }
                    continue;
                }
                else if( do_purge )
                {
                    if( locale_tag = strstr(line, "</locale>") )
                        do_purge = FALSE;
                    continue;
                }

                if( (locale_tag = strstr(line, "<locale")) )
                {
                    if( locale_tag = strstr(locale_tag, "name") )
                    {
                        char* lang = locale_tag + 4;
                        gsize lang_len = 0;
                        while( *lang && *lang != '\"' )
                            ++lang;
                        if( G_LIKELY(*lang == '\"') )
                            ++lang;

                        while( lang[lang_len] && lang[lang_len] != '\"' )
                            ++lang_len;

                        for( langs = g_get_language_names(); *langs; ++langs )
                        {
                            if( 0 == g_ascii_strncasecmp( *langs, lang, lang_len ) )
                                break;
                        }

                        if( ! *langs && strncmp( lang, "en_US", 5 ) && lang[0] != 'C' )
                        {
                            do_purge = TRUE;
                            continue;
                        }
                    }
                }
                g_string_append( result, line );
                g_string_append_c( result, '\n' );
            }while( line = strtok(NULL, "\n") );

            if( G_LIKELY( result->len < statbuf->st_size ) )
            {
                int fd = open( file_path, O_CREAT|O_WRONLY|O_TRUNC );
                if( G_LIKELY(fd != -1) )
                {
                    if( write( fd, result->str, result->len ) != -1 )
                    {
                        statbuf->st_size -= result->len;
                        saved_size += statbuf->st_size;
                        g_print( "%s purged, save %d bytes\n", file_path, statbuf->st_size );
                    }
                    else
                        g_print( "Error: %s\n", g_strerror( errno ) );
                    close( fd );
                }
                else
                    g_print( "Error: %s\n", g_strerror( errno ) );
            }
            else
                g_print( "Skip %s\n", file_path );
        }
        g_string_free( result, TRUE );
    }
}

static void do_purge( const char* dir_path )
{
    char* file_name;
    GDir *dir;
    GKeyFile* file = g_key_file_new();
    struct stat statbuf;

    g_print("Purging %s\n", dir_path);

    if( ! (dir = g_dir_open( dir_path, 0, NULL )) )
        return;
    while( file_name = g_dir_read_name( dir ) )
    {
        char* file_path;

        file_path = g_build_filename( dir_path, file_name, NULL );
        if( stat( file_path, &statbuf ) == -1 )
        {
            g_free( file_path );
            continue;
        }

        if( G_LIKELY( S_ISREG(statbuf.st_mode) ) )
        {
            if( G_LIKELY( g_str_has_suffix(file_name, ".schemas") ) )
                purge_file( file_path, &statbuf );
        }
        g_free( file_path );
    }
    g_key_file_free(file);
    g_dir_close( dir );
}

int main( int argc, char** argv )
{
    const char* dir;
    if( argc < 2 )
        dir = "/usr/share/gconf/schemas";
    else
        dir = argv[1];
    g_print("GConf purge 0.1\nDeveloped by Hong Jen Yee (PCMan) <pcman.tw@gmail.com>\n\n");
    do_purge( dir );
    g_print( "%d KB saved\n", saved_size/1024 );
    return 0;
}
