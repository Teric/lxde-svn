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
 * $Id: gtkxine.c,v 1.63 2003/04/08 22:30:27 guenter Exp $
 *
 * the xine engine in a widget - implementation
 *
 * some code originating from totem's gtkxine widget
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <pwd.h>
#include <sys/types.h>
#include <pthread.h>

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/cursorfont.h>
#include <X11/Xatom.h>
#ifdef HAVE_XTESTEXTENSION
#include <X11/extensions/XTest.h>
#endif /* HAVE_XTESTEXTENSION */

#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <gtk/gtkwidget.h>
#include <gtk/gtkmain.h>
#include <gdk/gdkx.h>
#include <xine.h>

#include "gtkxine.h"
//#include "i18n.h"

/*
 * global variable
 */

int verbosity = 0;

#define DEFAULT_WIDTH  420
#define DEFAULT_HEIGHT 315

/* missing stuff from X includes */
#ifndef XShmGetEventBase
extern int XShmGetEventBase(Display *);
#endif

static void gtk_xine_class_init    (GtkXineClass   *klass);
static void gtk_xine_instance_init (GtkXine        *gxine);

static void gtk_xine_finalize      (GObject        *object);
static void gtk_xine_realize       (GtkWidget      *widget);
static void gtk_xine_unrealize     (GtkWidget      *widget);

static gint gtk_xine_expose        (GtkWidget      *widget,
                    GdkEventExpose *event);

static void gtk_xine_size_allocate (GtkWidget      *widget,
                    GtkAllocation  *allocation);

static GtkWidgetClass *parent_class = NULL;

GtkType gtk_xine_get_type (void) {
  static GtkType gtk_xine_type = 0;


  if (!gtk_xine_type) {
    static const GTypeInfo gtk_xine_info = {
      sizeof (GtkXineClass),
      (GBaseInitFunc) NULL,
      (GBaseFinalizeFunc) NULL,
      (GClassInitFunc) gtk_xine_class_init,
      (GClassFinalizeFunc) NULL,
      NULL /* class_data */,
      sizeof (GtkXine),
      0 /* n_preallocs */,
      (GInstanceInitFunc) gtk_xine_instance_init,
    };

    gtk_xine_type = g_type_register_static (GTK_TYPE_WIDGET,
                        "GtkXine", &gtk_xine_info, (GTypeFlags)0);
  }

  return gtk_xine_type;
}

static void gtk_xine_class_init (GtkXineClass *class) {

  GObjectClass    *object_class;
  GtkWidgetClass  *widget_class;

  object_class = (GObjectClass *) class;
  widget_class = (GtkWidgetClass *) class;

  parent_class = gtk_type_class (gtk_widget_get_type ());

  /* GtkWidget */
  widget_class->realize       = gtk_xine_realize;
  widget_class->unrealize     = gtk_xine_unrealize;
  widget_class->size_allocate = gtk_xine_size_allocate;
  widget_class->expose_event  = gtk_xine_expose;

  /* GObject */
  object_class->set_property  = NULL;
  object_class->get_property  = NULL;
  object_class->finalize      = gtk_xine_finalize;
}

static void gtk_xine_instance_init (GtkXine *this) {

  this->widget.requisition.width  = DEFAULT_WIDTH;
  this->widget.requisition.height = DEFAULT_HEIGHT;

  /*
   * create a new xine instance, load config values
   */

  this->xine            = xine_new();

  snprintf (this->configfile, 255, "%s/.gxine/config", getenv ("HOME"));
  xine_config_load (this->xine, this->configfile);

  xine_engine_set_param (this->xine, XINE_ENGINE_PARAM_VERBOSITY, verbosity);

  this->stream          = NULL;
  this->video_port      = NULL;
  this->audio_port      = NULL;
  this->display         = NULL;
  this->fullscreen_mode = FALSE;
  this->oldwidth        = 0;
  this->oldheight       = 0;
  this->resize_factor   = 1.0;

  xine_init (this->xine);

  {
    int a, b, c, d;

    this->have_xtest = XTestQueryExtension (gdk_display, &a, &b, &c, &d);
    if (this->have_xtest == True)
      this->xtest_keycode = XKeysymToKeycode (gdk_display, XK_Shift_L);
  }

  this->vis_plugin    = NULL;
  this->vis_plugin_id = NULL;

#if 0
  printf ("gtkxine: extensions: '%s'\n",
      xine_get_file_extensions (this->xine));
#endif

}

static void gtk_xine_finalize (GObject *object) {

  GtkXine *gtx = (GtkXine *) object;

  G_OBJECT_CLASS (parent_class)->finalize (object);

  gtx = NULL;
}



static void dest_size_cb (void *gxine_gen,
              int video_width, int video_height,
              double video_pixel_aspect,
              int *dest_width, int *dest_height,
              double *dest_pixel_aspect)  {

  GtkXine   *gxine = (GtkXine *) gxine_gen;

  /* correct size with video_pixel_aspect */
  if (video_pixel_aspect >= gxine->display_ratio)
    video_width  = video_width * video_pixel_aspect / gxine->display_ratio + .5;
  else
    video_height = video_height * gxine->display_ratio / video_pixel_aspect + .5;

  if (gxine->fullscreen_mode) {
    *dest_width  = gxine->fullscreen_width;
    *dest_height = gxine->fullscreen_height;
  } else {
    if (gxine->resize_factor == 0.0) {
      *dest_width  = gxine->widget.allocation.width;
      *dest_height = gxine->widget.allocation.height;
    } else {
      *dest_width  = video_width*gxine->resize_factor;
      *dest_height = video_height*gxine->resize_factor;
    }
  }

  *dest_pixel_aspect = gxine->display_ratio;
}

static gboolean gtk_xine_idle_resize (GtkXine *gtx) {

  GtkWindow *toplevel;
  int        video_width  = gtx->oldwidth;
  int        video_height = gtx->oldheight;

#ifdef LOG
  printf ("gtkxine: idle resize to %d x %d\n",
      video_width, video_height);
#endif

  gdk_threads_enter ();

  toplevel = GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (gtx)));

  gtk_window_set_resizable (toplevel, FALSE);
  gtx->widget.allocation.width = video_width;
  gtx->widget.allocation.height = video_height;
  gtk_widget_set_size_request (&gtx->widget,
                   video_width, video_height);
  gtk_widget_queue_resize (gtk_widget_get_parent (GTK_WIDGET (gtx)));
  while (gtk_events_pending ())
    gtk_main_iteration ();

  gtk_window_set_resizable (toplevel, TRUE);

  gdk_threads_leave ();

#ifdef LOG
  printf ("gtkxine: idle signal done\n");
#endif

  return FALSE;
}



static void frame_output_cb (void *gxine_gen,
                 int video_width, int video_height,
                 double video_pixel_aspect,
                 int *dest_x, int *dest_y,
                 int *dest_width, int *dest_height,
                 double *dest_pixel_aspect,
                 int *win_x, int *win_y) {

  GtkXine   *gtx = (GtkXine *) gxine_gen;

  if (gtx == NULL)
    return;

  /* correct size with video_pixel_aspect */
  if (video_pixel_aspect >= gtx->display_ratio)
    video_width  = video_width * video_pixel_aspect / gtx->display_ratio + .5;
  else
    video_height = video_height * gtx->display_ratio / video_pixel_aspect + .5;

  *dest_x = 0;
  *dest_y = 0;

  if (gtx->fullscreen_mode) {
    *win_x = 0;
    *win_y = 0;

    *dest_width  = gtx->fullscreen_width;
    *dest_height = gtx->fullscreen_height;

  } else {

    *win_x = gtx->xpos;
    *win_y = gtx->ypos;

    if (gtx->resize_factor != 0.0) { /* => auto-resize */

      video_width  *= gtx->resize_factor;
      video_height *= gtx->resize_factor;

      /* size changed? */

      if ( (video_width != gtx->oldwidth) ||
       (video_height != gtx->oldheight) ) {


    gtx->oldwidth = video_width;
    gtx->oldheight = video_height;

    /* why can't we do this right here? */
    g_idle_add ((GSourceFunc) gtk_xine_idle_resize, gtx);
      }

      gtx->resize_factor = 0.0;
    }

    *dest_width  = gtx->widget.allocation.width;
    *dest_height = gtx->widget.allocation.height;
  }

  *dest_pixel_aspect = gtx->display_ratio;
}

static xine_video_port_t *load_video_out_driver(GtkXine *this) {

  double                   res_h, res_v;
  x11_visual_t             vis;
  const char              *video_driver_id;
  xine_video_port_t        *video_port;

  vis.display           = this->display;
  vis.screen            = this->screen;
  vis.d                 = this->video_window;
  res_h                 = (DisplayWidth  (this->display, this->screen)*1000
               / DisplayWidthMM (this->display, this->screen));
  res_v                 = (DisplayHeight (this->display, this->screen)*1000
               / DisplayHeightMM (this->display, this->screen));
  this->display_ratio   = res_v / res_h;

  if (fabs(this->display_ratio - 1.0) < 0.01) {
    this->display_ratio   = 1.0;
  }

  vis.dest_size_cb      = dest_size_cb;
  vis.frame_output_cb   = frame_output_cb;
  vis.user_data         = this;

  if (this->video_driver_id)
    video_driver_id = this->video_driver_id;
  else {

    char **driver_ids = (char **) xine_list_video_output_plugins (this->xine) ;
    char **choices;
    int i;

    choices = malloc (sizeof (char *) * 100);
    choices[0] = "auto"; i=0;
    while (driver_ids[i]) {
      choices[i+1] = strdup(driver_ids[i]);
      i++;
    }
    choices[i+1]=0;


    /* try to init video with stored information */
    i = xine_config_register_enum (this->xine,
                   "video.driver", 0,
                   choices,
                   "video driver to use",
                   NULL, 10, NULL, NULL);
    video_driver_id = choices[i];
  }
  if (strcmp (video_driver_id, "auto")) {

    video_port=xine_open_video_driver (this->xine,
                      video_driver_id,
                      XINE_VISUAL_TYPE_X11,
                      (void *) &vis);
    if (video_port)
      return video_port;
    else
      printf ("gtkxine: video driver %s failed.\n",
          video_driver_id); /* => auto-detect */
  }

  return xine_open_video_driver (this->xine, NULL,
                 XINE_VISUAL_TYPE_X11,
                 (void *) &vis);
}

static xine_audio_port_t *load_audio_out_driver (GtkXine *this) {

  xine_audio_port_t       *audio_port;
  const char             *audio_driver_id;

  if (this->audio_driver_id)
    audio_driver_id = this->audio_driver_id;
  else {

    char **driver_ids = (char **) xine_list_audio_output_plugins (this->xine) ;
    char **choices;
    int i;

    choices = malloc (sizeof (char *) * 100);
    choices[0] = "auto"; i=0;
    while (driver_ids[i]) {
      choices[i+1] = strdup(driver_ids[i]);
      i++;
    }
    choices[i+1]=0;


    /* try to init audio with stored information */
    i = xine_config_register_enum (this->xine,
                   "audio.driver", 0,
                   choices,
                   "audio driver to use",
                   NULL, 10, NULL, NULL);
    audio_driver_id = choices[i];
  }

  if (!strcmp (audio_driver_id, "null"))
    return NULL;

  if (strcmp (audio_driver_id, "auto")) {

    audio_port = xine_open_audio_driver (this->xine, audio_driver_id, NULL);
    if (audio_port)
      return audio_port;
    else
      printf ("audio driver %s failed\n", audio_driver_id);
  }

  /* autoprobe */
  return xine_open_audio_driver (this->xine, NULL, NULL);
}

static void* xine_thread (void *this_gen) {

  GtkXine    *this   = (GtkXine *) this_gen;
  /*   GtkWidget  *widget = &this->widget; */

  while (1) {

    XEvent event;
g_debug("BEFORE");
    XNextEvent (this->display, &event);

    /* printf ("gtkxine: got an event (%d)\n", event.type);  */
g_debug( "HERE: %d", event.type );
continue;

    switch (event.type) {
    case Expose:

      if (event.xexpose.count != 0)
    break;
#if 0
      xine_gui_send_vo_data (this->stream,
                 XINE_GUI_SEND_EXPOSE_EVENT, &event);
#endif
      break;


    case FocusOut: /* when in fullscreen mode, keep the focus */
      if (this->fullscreen_mode) {
    XLockDisplay (this->display);
    XSetInputFocus (this->display,
            this->fullscreen_window, RevertToNone, CurrentTime);
    XUnlockDisplay (this->display);
      }
      break;

    case MotionNotify:
      {
    XMotionEvent     *mevent = (XMotionEvent *) &event;
    x11_rectangle_t   rect;
    xine_event_t      event;
    xine_input_data_t input;

#ifdef LOG
    printf("gtkxine: mouse event: mx=%d my=%d\n",mevent->x, mevent->y);
#endif

    rect.x = mevent->x;
    rect.y = mevent->y;
    rect.w = 0;
    rect.h = 0;

    xine_gui_send_vo_data (this->stream,
                   XINE_GUI_SEND_TRANSLATE_GUI_TO_VIDEO,
                   (void*)&rect);

    event.type        = XINE_EVENT_INPUT_MOUSE_MOVE;
    event.data        = &input;
    event.data_length = sizeof(input);
    input.button      = 0; /*  no buttons, just motion */
    input.x           = rect.x;
    input.y           = rect.y;
    xine_event_send (this->stream, &event);

    if (this->fullscreen_mode && !this->cursor_visible) {

      /*
       * make mouse pointer visible
       */

      XLockDisplay (this->display);
      XDefineCursor (this->display, this->fullscreen_window,
             this->on_cursor);
      XFlush (this->display);
      XUnlockDisplay (this->display);

      this->cursor_visible = TRUE;
    }
      }
      break;

    case ButtonPress:
      {
    XButtonEvent *bevent = (XButtonEvent *) &event;

#ifdef LOG
    printf("gtkxine: mouse button event: mx=%d my=%d\n",
           bevent->x, bevent->y);
#endif

    if (bevent->button == Button1) {

      x11_rectangle_t   rect;
      xine_event_t      event;
      xine_input_data_t input;

      rect.x = bevent->x;
      rect.y = bevent->y;
      rect.w = 0;
      rect.h = 0;

      xine_gui_send_vo_data (this->stream,
                 XINE_GUI_SEND_TRANSLATE_GUI_TO_VIDEO,
                 (void*)&rect);

      event.type        = XINE_EVENT_INPUT_MOUSE_BUTTON;
      event.data        = &input;
      event.data_length = sizeof(input);
      input.button      = 1;
      input.x           = rect.x;
      input.y           = rect.y;
      xine_event_send (this->stream, &event);
    }
      }
      break;

    case KeyPress:
      {
    GdkEventKey    gdk_event;
    XKeyEvent     *kevent = (XKeyEvent *) &event;
    char           buffer [20];
    int            bufsize=20;
    KeySym         keysym;
    XComposeStatus compose;
    int            ret;

    XLookupString (kevent, buffer, bufsize,
               &keysym, &compose);

    gdk_event.keyval = keysym;

    gdk_event.type   = GDK_KEY_PRESS;
    gdk_event.window = this->toplevel;
    gdk_event.time   = kevent->time;
    gdk_event.state  = (GdkModifierType) kevent->state;
    gdk_event.string = strdup (buffer);
    gdk_event.length = 1;

    gtk_signal_emit_by_name (GTK_OBJECT (gtk_widget_get_toplevel (GTK_WIDGET (this))),
                 "key_press_event",
                 &gdk_event, &ret);
      }

      break;
    }

    if (event.type == this->completion_event) {
      xine_gui_send_vo_data (this->stream,
                 XINE_GUI_SEND_COMPLETION_EVENT, &event);
      /* printf ("gtkxine: completion event\n"); */
    }
  }

  pthread_exit(NULL);
  return NULL;
}

static gboolean configure_cb  (GtkWidget *widget,
                   GdkEventConfigure *event,
                   gpointer user_data) {

  GtkXine      *this;

  this = GTK_XINE(user_data);

  this->xpos = event->x;
  this->ypos = event->y;

  return FALSE;
}

static void reset_screen_saver (GtkXine *this) {

  /* printf ("gtkxine: disabling screen saver\n"); */

#ifdef HAVE_XTESTEXTENSION
  if (this->have_xtest == True) {
    XTestFakeKeyEvent (gdk_display, this->xtest_keycode, True, CurrentTime);
    XTestFakeKeyEvent (gdk_display, this->xtest_keycode, False, CurrentTime);
    XSync (gdk_display, False);
  } else
#endif
    {
      XResetScreenSaver (gdk_display);
    }
}

static gint gtk_timeout_cb (gpointer data) {

  GtkXine *gtx = (GtkXine *) data;

  if (gtx->fullscreen_mode && gtx->cursor_visible) {
    XLockDisplay (gtx->display);
    XDefineCursor (gtx->display, gtx->fullscreen_window,
           gtx->no_cursor);
    XFlush (gtx->display);
    gtx->cursor_visible = FALSE;
    XUnlockDisplay (gtx->display);
  }

  if (xine_get_status (gtx->stream) == XINE_STATUS_PLAY)
    reset_screen_saver (gtx);

  return TRUE;
}

static unsigned char bm_no_data[] = { 0,0,0,0, 0,0,0,0 };

static void gtk_xine_realize (GtkWidget *widget) {

  /* GdkWindowAttr attributes; */
  /* gint          attributes_mask; */
  GtkXine      *this;
  XGCValues     values;
  Pixmap        bm_no;
  int           screen;
  unsigned long black_pixel;

  g_return_if_fail(widget != NULL);
  g_return_if_fail(GTK_IS_XINE(widget));

  this = GTK_XINE(widget);

  /* set realized flag */

  GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED);

  /*
   * create our own video window
   */

  screen = DefaultScreen (gdk_display);
  black_pixel = BlackPixel (gdk_display, screen);

  this->video_window = XCreateSimpleWindow (gdk_display,
                        GDK_WINDOW_XWINDOW (gtk_widget_get_parent_window(widget)),
                        0, 0,
                        widget->allocation.width,
                        widget->allocation.height, 1,
                        black_pixel, black_pixel);

  widget->window = gdk_window_foreign_new (this->video_window);

  /*
   * prepare for fullscreen playback
   */

  this->fullscreen_width  = DisplayWidth (gdk_display, screen);
  this->fullscreen_height = DisplayHeight (gdk_display, screen);
  this->fullscreen_mode   = 0;

  this->toplevel = GDK_WINDOW_XWINDOW (gdk_window_get_toplevel (gtk_widget_get_parent_window(widget)));

  /*
   * track configure events of toplevel window
   */
  g_signal_connect (gtk_widget_get_toplevel (widget),
            "configure-event",
            G_CALLBACK (configure_cb), this);


  if (!XInitThreads ()) {
    printf ("gtkxine: XInitThreads failed - looks like you don't have a thread-safe xlib.\n");
    return;
  }

  this->display = XOpenDisplay (NULL);

  if (!this->display) {
    printf ("gtkxine: XOpenDisplay failed!\n");
    return;
  }

  XLockDisplay (this->display);

  this->screen = DefaultScreen (this->display);

  if (XShmQueryExtension (this->display) == True) {
    this->completion_event = XShmGetEventBase (this->display) + ShmCompletion;
  } else {
    this->completion_event = -1;
  }

  XSelectInput (this->display, this->video_window,
        StructureNotifyMask | ExposureMask |
        ButtonPressMask | PointerMotionMask);

  /*
   * load audio, video drivers
   */

  this->video_port = load_video_out_driver(this) ;

  if (!this->video_port) {
    printf ("gtkxine: couldn't open video driver\n");
    return ;
  }

  this->audio_port = load_audio_out_driver(this);

  /*
   * create a stream object
   */

  this->stream = xine_stream_new (this->xine, this->audio_port,
                  this->video_port);

  values.foreground = BlackPixel (this->display, this->screen);
  values.background = WhitePixel (this->display, this->screen);

  this->gc = XCreateGC (this->display, this->video_window,
            (GCForeground | GCBackground),
            &values);


  XUnlockDisplay (this->display);

  /*
   * create mouse cursors
   */

  bm_no = XCreateBitmapFromData(this->display,
                                this->video_window,
                                bm_no_data, 8, 8);
  this->no_cursor = XCreatePixmapCursor(this->display, bm_no, bm_no,
                    (XColor *) &black_pixel,
                    (XColor *) &black_pixel,
                    0, 0);
  this->on_cursor = XCreateFontCursor (this->display, XC_left_ptr);
  this->cursor_visible = TRUE;

  /*
   * now, create a xine thread
   */

  pthread_create (&this->thread, NULL, xine_thread, this);

  /*
   * add timeout to switch off mouse cursor, disable screen saver
   */

  gtk_timeout_add (4000, gtk_timeout_cb, this);

  return;
}


static void gtk_xine_unrealize (GtkWidget *widget) {
  GtkXine* this;

  g_return_if_fail(widget != NULL);
  g_return_if_fail(GTK_IS_XINE(widget));

  this = GTK_XINE(widget);

  /* stop event thread */

  pthread_cancel (this->thread);

  /* save configuration */

  xine_config_save (this->xine, this->configfile);

  /* exit xine */

  xine_exit (this->xine);

  /* Hide all windows */

  if (GTK_WIDGET_MAPPED (widget))
    gtk_widget_unmap (widget);

  GTK_WIDGET_UNSET_FLAGS (widget, GTK_MAPPED);

  /* This destroys widget->window and unsets the realized flag */
  if (GTK_WIDGET_CLASS(parent_class)->unrealize)
    (* GTK_WIDGET_CLASS(parent_class)->unrealize) (widget);
}


GtkWidget *gtk_xine_new (const gchar *video_driver_id,
             const gchar *audio_driver_id) {

  GtkWidget *this=GTK_WIDGET (gtk_type_new (gtk_xine_get_type ()));

  if (video_driver_id)
    GTK_XINE(this)->video_driver_id = strdup(video_driver_id);
  else
    GTK_XINE(this)->video_driver_id = NULL;

  if (audio_driver_id)
    GTK_XINE(this)->audio_driver_id = strdup(audio_driver_id);
  else
    GTK_XINE(this)->audio_driver_id = NULL;

  return this;
}


static gint gtk_xine_expose (GtkWidget *widget,
                 GdkEventExpose *event) {
/*
  GtkXine *this = GTK_XINE (widget);
*/

  return FALSE;
}

static void gtk_xine_size_allocate (GtkWidget *widget, GtkAllocation *allocation) {

  GtkXine *this;

  g_return_if_fail (widget != NULL);
  g_return_if_fail(GTK_IS_XINE(widget));

  this = GTK_XINE(widget);

  widget->allocation = *allocation;

  if (GTK_WIDGET_REALIZED (widget)) {
    gdk_window_move_resize (widget->window,
                allocation->x,
                allocation->y,
                allocation->width,
                allocation->height);
  }
}

gint gtk_xine_open (GtkXine *gtx, const gchar *mrl) {

  g_return_val_if_fail (gtx != NULL, -1);
  g_return_val_if_fail (GTK_IS_XINE (gtx), -1);
  g_return_val_if_fail (gtx->xine != NULL, -1);

  if (verbosity)
    printf ("gtkxine: calling xine_open, mrl = '%s'\n",
        mrl);

  return xine_open (gtx->stream, mrl);
}

gint gtk_xine_play (GtkXine *gtx,
            gint pos, gint start_time) {

  gint res;

  g_return_val_if_fail (gtx != NULL, -1);
  g_return_val_if_fail (GTK_IS_XINE (gtx), -1);
  g_return_val_if_fail (gtx->xine != NULL, -1);

  if (verbosity)
    printf ("gtkxine: calling xine_play start_pos = %d, start_time = %d\n",
        pos, start_time);

  /*
   * visualization
   */

  if (!xine_get_stream_info (gtx->stream, XINE_STREAM_INFO_HAS_VIDEO)) {

    if (!gtx->vis_plugin && gtx->vis_plugin_id) {

      xine_post_out_t  *audio_source;
      xine_post_in_t   *input;
      xine_cfg_entry_t  cfg;

      gtx->vis_plugin = xine_post_init (gtx->xine, gtx->vis_plugin_id, 0,
                    &gtx->audio_port,
                    &gtx->video_port);

      xine_config_lookup_entry (gtx->xine, "post.goom_height", &cfg);
      cfg.num_value = 240;
      xine_config_update_entry (gtx->xine, &cfg);

      xine_config_lookup_entry (gtx->xine, "post.goom_width", &cfg);
      cfg.num_value = 480;
      xine_config_update_entry (gtx->xine, &cfg);


      audio_source = xine_get_audio_source (gtx->stream);
      input = xine_post_input (gtx->vis_plugin, "audio in");

      xine_post_wire (audio_source, input);

    }
  } else {
    if (gtx->vis_plugin) {
      xine_post_out_t *pp;

      pp = xine_get_audio_source (gtx->stream);
      xine_post_wire_audio_port (pp, gtx->audio_port);
      xine_post_dispose (gtx->xine, gtx->vis_plugin);
      gtx->vis_plugin = NULL;
    }
  }

  res=xine_play (gtx->stream, pos, start_time);

  return res;
}

gint gtk_xine_trick_mode (GtkXine *gtx,
              gint mode, gint value) {

  g_return_val_if_fail (gtx != NULL, 0);
  g_return_val_if_fail (GTK_IS_XINE (gtx), 0);
  g_return_val_if_fail (gtx->stream != NULL, 0);

  return xine_trick_mode (gtx->stream, mode, value);
}

gint gtk_xine_get_pos_length (GtkXine *gtx, gint *pos_stream,
                  gint *pos_time,
                  gint *length_time) {
  g_return_val_if_fail (gtx != NULL, 0);
  g_return_val_if_fail (GTK_IS_XINE (gtx), 0);
  g_return_val_if_fail (gtx->stream != NULL, 0);

  return xine_get_pos_length (gtx->stream, pos_stream, pos_time, length_time);
}

void gtk_xine_stop (GtkXine *gtx) {

  g_return_if_fail (gtx != NULL);
  g_return_if_fail (GTK_IS_XINE (gtx));
  g_return_if_fail (gtx->stream != NULL);

  xine_stop (gtx->stream);
}

gint gtk_xine_get_error (GtkXine *gtx) {
  g_return_val_if_fail (gtx != NULL, 0);
  g_return_val_if_fail (GTK_IS_XINE (gtx), 0);
  g_return_val_if_fail (gtx->stream != NULL, 0);

  return xine_get_error (gtx->stream);
}

gint gtk_xine_get_status (GtkXine *gtx) {
  g_return_val_if_fail (gtx != NULL, 0);
  g_return_val_if_fail (GTK_IS_XINE (gtx), 0);
  g_return_val_if_fail (gtx->stream != NULL, 0);

  return xine_get_status (gtx->stream);
}

void gtk_xine_set_param (GtkXine *gtx, gint param, gint value) {

  g_return_if_fail (gtx != NULL);
  g_return_if_fail (GTK_IS_XINE (gtx));
  g_return_if_fail (gtx->stream != NULL);

  xine_set_param (gtx->stream, param, value);
}

gint gtk_xine_get_param (GtkXine *gtx, gint param) {

  g_return_val_if_fail (gtx != NULL, 0);
  g_return_val_if_fail (GTK_IS_XINE (gtx), 0);
  g_return_val_if_fail (gtx->stream != NULL, 0);

  return xine_get_param (gtx->stream, param);
}

gint gtk_xine_get_audio_lang (GtkXine *gtx, gint channel, gchar *lang) {

  g_return_val_if_fail (gtx != NULL, 0);
  g_return_val_if_fail (GTK_IS_XINE (gtx), 0);
  g_return_val_if_fail (gtx->stream != NULL, 0);

  return xine_get_audio_lang (gtx->stream, channel, lang);
}

gint gtk_xine_get_spu_lang (GtkXine *gtx, gint channel, gchar *lang) {

  g_return_val_if_fail (gtx != NULL, 0);
  g_return_val_if_fail (GTK_IS_XINE (gtx), 0);
  g_return_val_if_fail (gtx->stream != NULL, 0);

  return xine_get_spu_lang (gtx->stream, channel, lang);
}

gint gtk_xine_get_stream_info (GtkXine *gtx, gint info) {

  g_return_val_if_fail (gtx != NULL, 0);
  g_return_val_if_fail (GTK_IS_XINE (gtx), 0);
  g_return_val_if_fail (gtx->stream != NULL, 0);

  return xine_get_stream_info (gtx->stream, info);
}

const gchar *gtk_xine_get_meta_info (GtkXine *gtx, gint info) {

  g_return_val_if_fail (gtx != NULL, 0);
  g_return_val_if_fail (GTK_IS_XINE (gtx), 0);
  g_return_val_if_fail (gtx->stream != NULL, 0);

  return xine_get_meta_info (gtx->stream, info);
}

#define MWM_HINTS_DECORATIONS   (1L << 1)
#define PROP_MWM_HINTS_ELEMENTS 5
typedef struct {
  uint32_t                    flags;
  uint32_t                    functions;
  uint32_t                    decorations;
  int32_t                     input_mode;
  uint32_t                    status;
} MWMHints;

void gtk_xine_set_fullscreen (GtkXine *gtx, gint fullscreen) {

  static char          *window_title = "gxine fullscreen window";
  XSizeHints            hint;
  Atom                  prop;
  MWMHints              mwmhints;
  XEvent                xev;

  g_return_if_fail (gtx != NULL);
  g_return_if_fail (GTK_IS_XINE (gtx));
  g_return_if_fail (gtx->stream != NULL);

  if (fullscreen == gtx->fullscreen_mode)
    return;

  XLockDisplay (gtx->display);

  if (fullscreen) {

    int screen = DefaultScreen (gtx->display);

    gtx->fullscreen_window = XCreateSimpleWindow (gtx->display,
                          RootWindow (gtx->display, screen),
                          0, 0,
                          gtx->fullscreen_width,
                          gtx->fullscreen_height, 1,
                          BlackPixel(gtx->display, screen),
                          BlackPixel(gtx->display, screen));

    hint.flags       = PPosition | PSize | PMinSize | PWinGravity;
    hint.x           = 0;
    hint.y           = 0;
    hint.width       = gtx->fullscreen_width;
    hint.height      = gtx->fullscreen_height;
    hint.min_width   = gtx->fullscreen_width;
    hint.min_height  = gtx->fullscreen_height;
    hint.win_gravity = StaticGravity;

    /* set Properties for window manager (always before mapping) */
    XSetStandardProperties (gtx->display, gtx->fullscreen_window,
                window_title, window_title, None, NULL, 0, &hint);

    XSetWMNormalHints (gtx->display, gtx->fullscreen_window, &hint);

    /* XSetWMHints (gtx->display, gtx->fullscreen_window, &hint);*/

    /*
     * layer above most other things, like gnome panel
     * WIN_LAYER_ABOVE_DOCK  = 10
     *
     */
    {
      static Atom           XA_WIN_LAYER = None;
      long                  propvalue[1];

      if (XA_WIN_LAYER == None)
    XA_WIN_LAYER = XInternAtom(gtx->display, "_WIN_LAYER", False);

      propvalue[0] = 20;
      XChangeProperty(gtx->display, gtx->fullscreen_window, XA_WIN_LAYER,
              XA_CARDINAL, 32, PropModeReplace,
              (unsigned char *)propvalue, 1);
    }

    /*
     * fullscreen the modern (e.g. metacity) way
     */

    {
      static Atom           XA_WIN_STATE = None;
      long                  propvalue[2];

      if (XA_WIN_STATE == None)
    XA_WIN_STATE = XInternAtom (gtx->display, "_NET_WM_STATE", False);

      propvalue[0] = XInternAtom (gtx->display, "_NET_WM_STATE_FULLSCREEN",
                  False);
      propvalue[1] = 0;

      XChangeProperty (gtx->display, gtx->fullscreen_window,
               XA_WIN_STATE, XA_ATOM,
               32, PropModeReplace,
               (unsigned char *)propvalue, 1);
      XFlush(gtx->display);

    }

    /*
     * wm, no borders please
     */

    prop = XInternAtom(gtx->display, "_MOTIF_WM_HINTS", False);
    mwmhints.flags = MWM_HINTS_DECORATIONS;
    mwmhints.decorations = 0;
    XChangeProperty (gtx->display, gtx->fullscreen_window, prop, prop, 32,
             PropModeReplace, (unsigned char *) &mwmhints,
             PROP_MWM_HINTS_ELEMENTS);

    XSetTransientForHint(gtx->display, gtx->fullscreen_window, None);
    XRaiseWindow(gtx->display, gtx->fullscreen_window);

    XSelectInput(gtx->display, gtx->fullscreen_window,
         ExposureMask | KeyPressMask | ButtonPressMask
         | StructureNotifyMask | FocusChangeMask | PointerMotionMask);

    XMapWindow (gtx->display, gtx->fullscreen_window);

    XFlush(gtx->display);

    /* Wait for map. */

    do  {
      XMaskEvent(gtx->display,
         StructureNotifyMask,
         &xev) ;
    } while (xev.type != MapNotify || xev.xmap.event != gtx->fullscreen_window);

    XSetInputFocus (gtx->display, gtx->fullscreen_window,
            RevertToNone, CurrentTime);

    XMoveWindow (gtx->display, gtx->fullscreen_window, 0, 0);

    xine_gui_send_vo_data (gtx->stream,
               XINE_GUI_SEND_DRAWABLE_CHANGED,
               (void*)gtx->fullscreen_window);

    /*
     * switch off mouse cursor
     */

    XDefineCursor(gtx->display, gtx->fullscreen_window,
          gtx->no_cursor);
    XFlush(gtx->display);

    gtx->cursor_visible = FALSE;

  } else {

    xine_gui_send_vo_data (gtx->stream,
               XINE_GUI_SEND_DRAWABLE_CHANGED,
               (void*)gtx->video_window);

    XDestroyWindow (gtx->display, gtx->fullscreen_window);

    XFlush(gtx->display);
  }

  gtx->fullscreen_mode = fullscreen;

  XUnlockDisplay (gtx->display);
}

gint gtk_xine_is_fullscreen (GtkXine *gtx) {

  g_return_val_if_fail (gtx != NULL, 0);
  g_return_val_if_fail (GTK_IS_XINE (gtx), 0);
  g_return_val_if_fail (gtx->stream != NULL, 0);

  return gtx->fullscreen_mode;
}

gint gtk_xine_get_current_frame (GtkXine *gtx,
                 gint *width,
                 gint *height,
                 gint *ratio_code,
                 gint *format,
                 uint8_t *img) {

  g_return_val_if_fail (gtx != NULL, 0);
  g_return_val_if_fail (GTK_IS_XINE (gtx), 0);
  g_return_val_if_fail (gtx->stream != NULL, 0);

  return xine_get_current_frame (gtx->stream, width, height, ratio_code, format,
                 img);
}

gint gtk_xine_get_log_section_count (GtkXine *gtx) {

  g_return_val_if_fail (gtx != NULL, 0);
  g_return_val_if_fail (GTK_IS_XINE (gtx), 0);
  g_return_val_if_fail (gtx->xine != NULL, 0);

  return xine_get_log_section_count (gtx->xine);
}

gchar **gtk_xine_get_log_names (GtkXine *gtx) {
  g_return_val_if_fail (gtx != NULL, NULL);
  g_return_val_if_fail (GTK_IS_XINE (gtx), NULL);
  g_return_val_if_fail (gtx->xine != NULL, NULL);

  return (gchar **) xine_get_log_names (gtx->xine);
}

gchar **gtk_xine_get_log (GtkXine *gtx,
              gint buf) {

  g_return_val_if_fail (gtx != NULL, NULL);
  g_return_val_if_fail (GTK_IS_XINE (gtx), NULL);
  g_return_val_if_fail (gtx->xine != NULL, NULL);

  return (gchar **) xine_get_log (gtx->xine, buf);
}

void gtk_xine_register_log_cb (GtkXine *gtx,
                   xine_log_cb_t cb,
                   void *user_data) {
  g_return_if_fail (gtx != NULL);
  g_return_if_fail (GTK_IS_XINE (gtx));
  g_return_if_fail (gtx->xine != NULL);

  return xine_register_log_cb (gtx->xine, cb, user_data);
}

gchar **gtk_xine_get_browsable_input_plugin_ids (GtkXine *gtx) {

  g_return_val_if_fail (gtx != NULL, NULL);
  g_return_val_if_fail (GTK_IS_XINE (gtx), NULL);
  g_return_val_if_fail (gtx->xine != NULL, NULL);

  return (gchar **) xine_get_browsable_input_plugin_ids (gtx->xine);
}

xine_mrl_t **gtk_xine_get_browse_mrls (GtkXine *gtx,
                       const gchar *plugin_id,
                       const gchar *start_mrl,
                       gint *num_mrls) {
  g_return_val_if_fail (gtx != NULL, NULL);
  g_return_val_if_fail (GTK_IS_XINE (gtx), NULL);
  g_return_val_if_fail (gtx->xine != NULL, NULL);

  return (xine_mrl_t **) xine_get_browse_mrls (gtx->xine, plugin_id, start_mrl, num_mrls);
}


gchar **gtk_xine_get_autoplay_input_plugin_ids (GtkXine *gtx) {

  g_return_val_if_fail (gtx != NULL, NULL);
  g_return_val_if_fail (GTK_IS_XINE (gtx), NULL);
  g_return_val_if_fail (gtx->xine != NULL, NULL);

  return (gchar **) xine_get_autoplay_input_plugin_ids (gtx->xine);
}

gchar **gtk_xine_get_autoplay_mrls (GtkXine *gtx,
                    const gchar *plugin_id,
                    gint *num_mrls) {
  g_return_val_if_fail (gtx != NULL, NULL);
  g_return_val_if_fail (GTK_IS_XINE (gtx), NULL);
  g_return_val_if_fail (gtx->xine != NULL, NULL);

  return (gchar **) xine_get_autoplay_mrls (gtx->xine, plugin_id, num_mrls);
}

gchar *gtk_xine_get_file_extensions (GtkXine *gtx) {

  g_return_val_if_fail (gtx != NULL, NULL);
  g_return_val_if_fail (GTK_IS_XINE (gtx), NULL);
  g_return_val_if_fail (gtx->xine != NULL, NULL);

  return (gchar *) xine_get_file_extensions (gtx->xine);
}

gchar *gtk_xine_get_mime_types (GtkXine *gtx) {

  g_return_val_if_fail (gtx != NULL, NULL);
  g_return_val_if_fail (GTK_IS_XINE (gtx), NULL);
  g_return_val_if_fail (gtx->xine != NULL, NULL);

  return (gchar *) xine_get_mime_types (gtx->xine);
}

const gchar *gtk_xine_config_register_string (GtkXine *gtx,
                          const gchar *key,
                          const gchar *def_value,
                          const gchar *description,
                          const gchar *help,
                          gint   exp_level,
                          xine_config_cb_t changed_cb,
                          void *cb_data) {

  g_return_val_if_fail (gtx != NULL, NULL);
  g_return_val_if_fail (GTK_IS_XINE (gtx), NULL);
  g_return_val_if_fail (gtx->xine != NULL, NULL);

  return (gchar *) xine_config_register_string (gtx->xine, key, def_value,
                        description, help,
                        exp_level, changed_cb, cb_data);
}

gint gtk_xine_config_register_range  (GtkXine *gtx,
                      const gchar *key,
                      gint def_value,
                      gint min, gint max,
                      const gchar *description,
                      const gchar *help,
                      gint   exp_level,
                      xine_config_cb_t changed_cb,
                      void *cb_data) {
  g_return_val_if_fail (gtx != NULL, 0);
  g_return_val_if_fail (GTK_IS_XINE (gtx), 0);
  g_return_val_if_fail (gtx->xine != NULL, 0);

  return xine_config_register_range (gtx->xine, key, def_value, min, max,
                     description, help,
                     exp_level, changed_cb, cb_data);
}

gint gtk_xine_config_register_enum   (GtkXine *gtx,
                      const gchar *key,
                      gint def_value,
                      gchar **values,
                      const gchar *description,
                      const gchar *help,
                      gint   exp_level,
                      xine_config_cb_t changed_cb,
                      void *cb_data) {
  g_return_val_if_fail (gtx != NULL, 0);
  g_return_val_if_fail (GTK_IS_XINE (gtx), 0);
  g_return_val_if_fail (gtx->xine != NULL, 0);

  return xine_config_register_enum (gtx->xine, key, def_value, values,
                    description, help,
                    exp_level, changed_cb, cb_data);
}

gint gtk_xine_config_register_num  (GtkXine *gtx,
                    const gchar *key,
                    gint def_value,
                    const gchar *description,
                    const gchar *help,
                    gint   exp_level,
                    xine_config_cb_t changed_cb,
                    void *cb_data) {
  g_return_val_if_fail (gtx != NULL, 0);
  g_return_val_if_fail (GTK_IS_XINE (gtx), 0);
  g_return_val_if_fail (gtx->xine != NULL, 0);

  return xine_config_register_num (gtx->xine, key, def_value,
                   description, help,
                   exp_level, changed_cb, cb_data);
}

gint gtk_xine_config_register_bool (GtkXine *gtx,
                    const gchar *key,
                    gint def_value,
                    const gchar *description,
                    const gchar *help,
                    gint   exp_level,
                    xine_config_cb_t changed_cb,
                    void *cb_data) {

  g_return_val_if_fail (gtx != NULL, 0);
  g_return_val_if_fail (GTK_IS_XINE (gtx), 0);
  g_return_val_if_fail (gtx->xine != NULL, 0);

  return xine_config_register_bool (gtx->xine, key, def_value,
                    description, help,
                    exp_level, changed_cb, cb_data);
}

int gtk_xine_config_get_first_entry (GtkXine *gtx, xine_cfg_entry_t *entry) {
  g_return_val_if_fail (gtx != NULL, 0);
  g_return_val_if_fail (GTK_IS_XINE (gtx), 0);
  g_return_val_if_fail (gtx->xine != NULL, 0);

  return xine_config_get_first_entry (gtx->xine, entry);
}

int gtk_xine_config_get_next_entry (GtkXine *gtx, xine_cfg_entry_t *entry) {
  g_return_val_if_fail (gtx != NULL, 0);
  g_return_val_if_fail (GTK_IS_XINE (gtx), 0);
  g_return_val_if_fail (gtx->xine != NULL, 0);

  return xine_config_get_next_entry (gtx->xine, entry);
}

int gtk_xine_config_lookup_entry (GtkXine *gtx,
                  const gchar *key,
                  xine_cfg_entry_t *entry) {
  g_return_val_if_fail (gtx != NULL, 0);
  g_return_val_if_fail (GTK_IS_XINE (gtx), 0);
  g_return_val_if_fail (gtx->xine != NULL, 0);

  return xine_config_lookup_entry (gtx->xine, key, entry);
}

void gtk_xine_config_update_entry (GtkXine *gtx,
                   xine_cfg_entry_t *entry) {
  g_return_if_fail (gtx != NULL);
  g_return_if_fail (GTK_IS_XINE (gtx));
  g_return_if_fail (gtx->xine != NULL);

  xine_config_update_entry (gtx->xine, entry);
}

void gtk_xine_config_load (GtkXine *gtx,
               const gchar *cfg_filename) {
  g_return_if_fail (gtx != NULL);
  g_return_if_fail (GTK_IS_XINE (gtx));
  g_return_if_fail (gtx->xine != NULL);

  xine_config_load (gtx->xine, cfg_filename);
}

void gtk_xine_config_save (GtkXine *gtx,
               const gchar *cfg_filename) {
  g_return_if_fail (gtx != NULL);
  g_return_if_fail (GTK_IS_XINE (gtx));
  g_return_if_fail (gtx->xine != NULL);

  xine_config_save (gtx->xine, cfg_filename);
}

void gtk_xine_config_reset (GtkXine *gtx) {

  g_return_if_fail (gtx != NULL);
  g_return_if_fail (GTK_IS_XINE (gtx));
  g_return_if_fail (gtx->xine != NULL);

  xine_config_reset (gtx->xine);
}

void gtk_xine_event_send (GtkXine *gtx,
              const xine_event_t *event) {

  g_return_if_fail (gtx != NULL);
  g_return_if_fail (GTK_IS_XINE (gtx));
  g_return_if_fail (gtx->stream != NULL);

  xine_event_send (gtx->stream, event);
}


void gtk_xine_set_resize_factor (GtkXine *gtx,
                 double factor /* 0.0 => don't resize */) {

  g_return_if_fail (gtx != NULL);
  g_return_if_fail (GTK_IS_XINE (gtx));
  g_return_if_fail (gtx->xine != NULL);

  gtx->resize_factor = factor;
}

void  gtk_xine_set_vis (GtkXine *gtx,
            char *id) /* NULL to disable */ {

  g_return_if_fail (gtx != NULL);
  g_return_if_fail (GTK_IS_XINE (gtx));

  if (gtx->vis_plugin_id)
    free (gtx->vis_plugin_id);
  if (id)
    gtx->vis_plugin_id = strdup (id);
  else
    gtx->vis_plugin_id = NULL;


  if (gtx->stream) {
    if (xine_get_status (gtx->stream) == XINE_STATUS_PLAY) {

      if (gtx->vis_plugin) {
    xine_post_out_t *pp;

    pp = xine_get_audio_source (gtx->stream);
    xine_post_wire_audio_port (pp, gtx->audio_port);
    xine_post_dispose (gtx->xine, gtx->vis_plugin);
    gtx->vis_plugin = NULL;
      }

      if (gtx->vis_plugin_id) {
    xine_post_out_t  *audio_source;
    xine_post_in_t   *input;
    xine_cfg_entry_t  cfg;

    gtx->vis_plugin = xine_post_init (gtx->xine, gtx->vis_plugin_id, 0,
                      &gtx->audio_port,
                      &gtx->video_port);

    xine_config_lookup_entry (gtx->xine, "post.goom_height", &cfg);
    cfg.num_value = 240;
    xine_config_update_entry (gtx->xine, &cfg);

    xine_config_lookup_entry (gtx->xine, "post.goom_width", &cfg);
    cfg.num_value = 480;
    xine_config_update_entry (gtx->xine, &cfg);


    audio_source = xine_get_audio_source (gtx->stream);
    input = xine_post_input (gtx->vis_plugin, "audio in");

    xine_post_wire (audio_source, input);
      }
    }
  }
}

const char *const *gtk_xine_list_post_plugins_typed (GtkXine *gtx, int type) {
  g_return_val_if_fail (gtx != NULL, 0);
  g_return_val_if_fail (GTK_IS_XINE (gtx), 0);
  g_return_val_if_fail (gtx->xine != NULL, 0);

  return xine_list_post_plugins_typed (gtx->xine, type);
}
