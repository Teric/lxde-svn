/*
 * Copyright (C) 2001-2003 the xine project
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * $Id: gtkxine.h,v 1.26 2003/03/08 23:27:27 guenter Exp $
 *
 * the xine engine in a widget - header
 */

#ifndef HAVE_GTK_XINE_H
#define HAVE_GTK_XINE_H

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/extensions/XShm.h>
#include <xine.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define GTK_XINE(obj)              (GTK_CHECK_CAST ((obj), gtk_xine_get_type (), GtkXine))
#define GTK_XINE_CLASS(klass)      (GTK_CHECK_CLASS_CAST ((klass), gtk_xine_get_type (), GtkXineClass))
#define GTK_IS_XINE(obj)           (GTK_CHECK_TYPE (obj, gtk_xine_get_type ()))
#define GTK_IS_XINE_CLASS(klass)   (GTK_CHECK_CLASS_TYPE ((klass), gtk_xine_get_type ()))

  typedef struct _GtkXine      GtkXine;
  typedef struct _GtkXineClass GtkXineClass;

  struct _GtkXine
  {
    GtkWidget                widget;

    xine_t                  *xine;
    xine_stream_t           *stream;

    char                     configfile[256];

    char                    *video_driver_id;
    char                    *audio_driver_id;

    xine_video_port_t       *video_port;
    xine_audio_port_t       *audio_port;
    x11_visual_t             vis;
    double                   display_ratio;
    Display                 *display;
    int                      screen;
    Window                   video_window;
    GC                       gc;
    pthread_t                thread;
    int                      completion_event;
    int                      child_pid;

    int                      xpos, ypos;
    float                    resize_factor;
    int                      oldwidth, oldheight;

    /* fullscreen stuff */

    int                      fullscreen_mode;
    int                      fullscreen_width, fullscreen_height;
    Window                   fullscreen_window, toplevel;
    Cursor                   no_cursor;
    Cursor                   on_cursor;
    gboolean                 cursor_visible;

    GdkVisibilityState       visibility;

    int                      have_xtest;
    int                      xtest_keycode;

    /* visualization */
    xine_post_t             *vis_plugin;
    char                    *vis_plugin_id;

  };

  struct _GtkXineClass
  {
    GtkWidgetClass parent_class;
  };

  GtkType    gtk_xine_get_type          (void);
  GtkWidget* gtk_xine_new               (const gchar *video_driver_id,
					 const gchar *audio_driver_id);
  void       gtk_xine_set_visibility    (GtkXine *gtx,
					 GdkVisibilityState state);
  void       gtk_xine_resize            (GtkXine *gtx,
					 gint x, gint y,
					 gint width,
					 gint height);
  gint       gtk_xine_open              (GtkXine *gtx,
					 const gchar *mrl);
  gint       gtk_xine_play              (GtkXine *gtx,
				         gint pos,
					 gint start_time);
  gint       gtk_xine_trick_mode        (GtkXine *gtx,
					 gint mode,
					 gint value);
  gint       gtk_xine_get_stream_info   (GtkXine *gtx,
					 gint info);
  const gchar*gtk_xine_get_meta_info    (GtkXine *gtx,
					 gint info);
  gint       gtk_xine_get_pos_length    (GtkXine *gtx,
					 gint *pos_stream,  /* 0..65535     */
					 gint *pos_time,    /* milliseconds */
					 gint *length_time);/* milliseconds */
  void       gtk_xine_stop              (GtkXine *gtx);

  gint       gtk_xine_get_error         (GtkXine *gtx);
  gint       gtk_xine_get_status        (GtkXine *gtx);

  void       gtk_xine_set_param         (GtkXine *gtx,
					 gint param,
					 gint value);
  gint       gtk_xine_get_param         (GtkXine *gtx,
					 gint param);

  gint       gtk_xine_get_audio_lang    (GtkXine *gtx,
					 gint channel,
					 gchar *lang);
  gint       gtk_xine_get_spu_lang      (GtkXine *gtx,
					 gint channel,
					 gchar *lang);

  void       gtk_xine_set_fullscreen    (GtkXine *gtx,
					 gint fullscreen);

  gint       gtk_xine_is_fullscreen     (GtkXine *gtx);

  void       gtk_xine_set_resize_factor (GtkXine *gtx,
					 double factor /* 0.0 => don't resize */);

  gint       gtk_xine_get_current_frame (GtkXine *gtx,
					 gint *width,
					 gint *height,
					 gint *ratio_code,
					 gint *format,
					 uint8_t *img);
  gint       gtk_xine_get_log_section_count(GtkXine *gtx);
  gchar    **gtk_xine_get_log_names     (GtkXine *gtx);
  gchar    **gtk_xine_get_log           (GtkXine *gtx,
					 gint buf);
  void       gtk_xine_register_log_cb   (GtkXine *gtx,
					 xine_log_cb_t cb,
					 void *user_data);

  gchar    **gtk_xine_get_browsable_input_plugin_ids (GtkXine *gtx) ;
  xine_mrl_t **gtk_xine_get_browse_mrls (GtkXine *gtx,
					 const gchar *plugin_id,
					 const gchar *start_mrl,
					 gint *num_mrls);
  gchar    **gtk_xine_get_autoplay_input_plugin_ids (GtkXine *gtx);
  gchar    **gtk_xine_get_autoplay_mrls (GtkXine *gtx,
					 const gchar *plugin_id,
					 gint *num_mrls);
  gchar     *gtk_xine_get_file_extensions (GtkXine *gtx);
  gchar     *gtk_xine_get_mime_types      (GtkXine *gtx);

  const char *gtk_xine_config_register_string (GtkXine *gtx,
					       const char *key,
					       const char *def_value,
					       const char *description,
					       const char *help,
					       int   exp_level,
					       xine_config_cb_t changed_cb,
					       void *cb_data);
  
  int   gtk_xine_config_register_range  (GtkXine *gtx,
					 const char *key,
					 int def_value,
					 int min, int max,
					 const char *description,
					 const char *help,
					 int   exp_level,
					 xine_config_cb_t changed_cb,
					 void *cb_data);
  
  int   gtk_xine_config_register_enum   (GtkXine *gtx,
					 const char *key,
					 int def_value,
					 char **values,
					 const char *description,
					 const char *help,
					 int   exp_level,
					 xine_config_cb_t changed_cb,
					 void *cb_data);
  
  int   gtk_xine_config_register_num    (GtkXine *gtx,
					 const char *key,
					 int def_value,
					 const char *description,
					 const char *help,
					 int   exp_level,
					 xine_config_cb_t changed_cb,
					 void *cb_data);
  
  int   gtk_xine_config_register_bool   (GtkXine *gtx,
					 const char *key,
					 int def_value,
					 const char *description,
					 const char *help,
					 int   exp_level,
					 xine_config_cb_t changed_cb,
					 void *cb_data);

  int   gtk_xine_config_get_first_entry (GtkXine *gtx, xine_cfg_entry_t *entry);
  int   gtk_xine_config_get_next_entry (GtkXine *gtx, xine_cfg_entry_t *entry);
  int   gtk_xine_config_lookup_entry (GtkXine *gtx,
				    const gchar *key, xine_cfg_entry_t *entry);
  void  gtk_xine_config_update_entry (GtkXine *gtx,
				      xine_cfg_entry_t *entry);
  void  gtk_xine_config_load         (GtkXine *gtx,
				      const gchar *cfg_filename);
  void  gtk_xine_config_save         (GtkXine *gtx,
				      const gchar *cfg_filename);
  void  gtk_xine_config_reset        (GtkXine *gtx);

  xine_event_queue_t *gtk_xine_event_new_queue (GtkXine *gtx);

  void  gtk_xine_event_send          (GtkXine *gtx,
				      const xine_event_t *event);

  const char *const *gtk_xine_list_post_plugins_typed (GtkXine *gtx, int type);
  void  gtk_xine_set_vis             (GtkXine *gtx,
				      char *id); /* NULL to disable */

  /* FIXME: OSD missing */


#ifdef __cplusplus
}
#endif /* __cplusplus */



#endif /* HAVE_GTK_XINE_H */

