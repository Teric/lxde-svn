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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gdk-pixbuf-xlib/gdk-pixbuf-xlib.h>
#include <gdk/gdk.h>
#include <glib/gi18n.h>

#include "panel.h"
#include "misc.h"
#include "plugin.h"
#include "icon.xpm"
#include "gtkbar.h"

/*
 * 2006.09.10 modified by Hong Jen Yee (PCMan) pcman.tw (AT) gmail.com
 * Following features are added:
 * 1. Add XUrgencyHint support. (Flashing task bar buttons, can be disabled)
 * 2. Raise window when files get dragged over taskbar buttons.
 * 3. Add Restore & Maximize menu items to popup menu of task bar buttons.
 */

#include "dbg.h"

struct _taskbar;
typedef struct _task{
    struct _taskbar *tb;
    Window win;
    char *name, *iname;
    GtkWidget *button, *label;
    GtkWidget *image;

    GdkPixbuf *pixbuf;

    int refcount;
    XClassHint ch;
    int pos_x;
    int width;
    int desktop;
    NetWMState nws;
    NetWMWindowType nwwt;
    guint flash_timeout;
    unsigned int focused:1;
    unsigned int iconified:1;
    unsigned int urgency:1;
    unsigned int using_netwm_icon:1;
    unsigned int flash:1;
    unsigned int flash_state:1;
} task;

typedef struct _taskbar{
    Plugin *plug;
    Window *wins;
    Window topxwin;
    int win_num;
    GHashTable  *task_list;
    GtkWidget *bar, *menu;
    GdkPixbuf *gen_pixbuf;
    GtkStateType normal_state;
    GtkStateType focused_state;
    int num_tasks;
    int vis_task_num;
    int spacing;
    int cur_desk;
    task *focused;
    task *ptk;
    task *menutask;
    char **desk_names;
    int desk_namesno;
    int desk_num;
    guint dnd_activate;

    gboolean iconsize;
    int task_width_max;
    gboolean accept_skip_pager;// : 1;
    gboolean show_iconified;// : 1;
    gboolean show_mapped;// : 1;
    gboolean show_all_desks;// : 1;
    gboolean tooltips;// : 1;
    gboolean icons_only;// : 1;
    gboolean use_mouse_wheel;// : 1;
    gboolean use_urgency_hint;// : 1;
    gboolean flat_button;
} taskbar;

static gchar *taskbar_rc = "style 'taskbar-style'\n"
"{\n"
"GtkWidget::focus-line-width = 0\n"
"GtkWidget::focus-padding = 0\n"
"GtkButton::default-border = { 0, 0, 0, 0 }\n"
"GtkButton::default-outside-border = { 0, 0, 0, 0 }\n"
"GtkButton::default_border = { 0, 0, 0, 0 }\n"
"GtkButton::default_outside_border = { 0, 0, 0, 0 }\n"
"}\n"
"widget '*.taskbar.*' style 'taskbar-style'";

static gboolean use_net_active=FALSE;

#define DRAG_ACTIVE_DELAY   1000
#define TASK_WIDTH_MAX   200
#define TASK_PADDING     4

#define ALL_WORKSPACES (0xFFFFFFFF)

static void tk_display(taskbar *tb, task *tk);
static void tb_propertynotify(taskbar *tb, XEvent *ev);
static GdkFilterReturn tb_event_filter( XEvent *, GdkEvent *, taskbar *);
static void taskbar_destructor(Plugin *p);

static gboolean tk_has_urgency( task* tk );

static void tk_flash_window( task *tk );
static void tk_unflash_window( task *tk );
static void tk_raise_window( task *tk, guint32 time );

static void
update_label_orient( GtkWidget* child, gpointer user_data );


#define TASK_VISIBLE(tb, tk) \
 ((tk)->desktop == (tb)->cur_desk || (tk)->desktop == -1 /* 0xFFFFFFFF */ )

static int
task_visible(taskbar *tb, task *tk)
{
    ENTER;
    if (tk->desktop != -1 && !tb->show_all_desks && tk->desktop != tb->cur_desk)
        RET(0);
    if (tk->iconified) {
        if (!tb->show_iconified)
            RET(0);
    } else {
        if (!tb->show_mapped)
            RET(0);
    }
    RET(1);
}

static int
accept_net_wm_state(NetWMState *nws, int accept_skip_pager)
{
    ENTER;
    RET(!(nws->skip_taskbar || (accept_skip_pager && nws->skip_pager)));
}

static int
accept_net_wm_window_type(NetWMWindowType *nwwt)
{
    ENTER;
    RET(!(nwwt->desktop || nwwt->dock || nwwt->splash));
}



inline static void
tk_free_names(task *tk)
{
    ENTER;
    DBG("tk->name %s\n", tk->name);
    DBG("tk->iname %s\n", tk->iname);
    g_free(tk->name);
    g_free(tk->iname);

    tk->name = tk->iname = NULL;
    RET();
}

static void
tk_set_names(task *tk)
{
    char *name;

    ENTER;
    tk_free_names(tk);

    /*name = get_utf8_property(tk->win,  a_NET_WM_VISIBLE_NAME);
    DBG2("a_NET_WM_VISIBLE_NAME:%s\n", name);
    if (!name) {
    */
    name = get_utf8_property(tk->win,  a_NET_WM_NAME);
    DBG("a_NET_WM_NAME:%s\n", name);
    if (!name) {
        name = get_textproperty(tk->win,  XA_WM_NAME);
        DBG("XA_WM_NAME:%s\n", name);
    }

    if (name) {
    tk->name = g_strdup_printf(" %s ", name);
    tk->iname = g_strdup_printf("[%s]", name);
    g_free(name);
        name = tk->iconified ? tk->iname : tk->name;
    }
    gtk_label_set_text(GTK_LABEL(tk->label), name);
    if (tk->tb->tooltips)
        gtk_widget_set_tooltip_text( tk->button, tk->name );
    RET();
}



static task *
find_task (taskbar * tb, Window win)
{
    ENTER;
    RET(g_hash_table_lookup(tb->task_list, &win));
}


static void
del_task (taskbar * tb, task *tk, int hdel)
{
    ENTER;
    DBG("deleting(%d)  %08x %s\n", hdel, tk->win, tk->name);
    if( tk->flash_timeout )
        g_source_remove( tk->flash_timeout );
    gtk_widget_destroy(tk->button);
    --tb->num_tasks;
    tk_free_names(tk);
    if (tb->focused == tk)
        tb->focused = NULL;
    if (hdel)
        g_hash_table_remove(tb->task_list, &tk->win);
    g_free(tk);
    RET();
}



static GdkColormap*
get_cmap (GdkPixmap *pixmap)
{
  GdkColormap *cmap;

  ENTER;
  cmap = gdk_drawable_get_colormap (pixmap);
  if (cmap)
    g_object_ref (G_OBJECT (cmap));

  if (cmap == NULL)
    {
      if (gdk_drawable_get_depth (pixmap) == 1)
        {
          /* try null cmap */
          cmap = NULL;
        }
      else
        {
          /* Try system cmap */
          GdkScreen *screen = gdk_drawable_get_screen (GDK_DRAWABLE (pixmap));
          cmap = gdk_screen_get_system_colormap (screen);
          g_object_ref (G_OBJECT (cmap));
        }
    }

  /* Be sure we aren't going to blow up due to visual mismatch */
  if (cmap &&
      (gdk_colormap_get_visual (cmap)->depth !=
       gdk_drawable_get_depth (pixmap)))
    cmap = NULL;

  RET(cmap);
}

/* These functions with the prefix wnck are taken from libwnck
 * Copyright (C) 2001 Havoc Pennington
 * slightly modified by Hong Jen Yee for LXPanel
 */
void
_wnck_change_workspace (Screen     *screen,
            Window      xwindow,
                        int         new_space)
{
  XEvent xev;

  xev.xclient.type = ClientMessage;
  xev.xclient.serial = 0;
  xev.xclient.send_event = True;
  xev.xclient.display = gdk_display;
  xev.xclient.window = xwindow;
  xev.xclient.message_type = a_NET_WM_DESKTOP;
  xev.xclient.format = 32;
  xev.xclient.data.l[0] = new_space;
  xev.xclient.data.l[1] = 0;
  xev.xclient.data.l[2] = 0;
  xev.xclient.data.l[3] = 0;
  xev.xclient.data.l[4] = 0;

  XSendEvent (gdk_display,
          RootWindowOfScreen (screen),
              False,
          SubstructureRedirectMask | SubstructureNotifyMask,
          &xev);
}

static GdkPixbuf*
_wnck_gdk_pixbuf_get_from_pixmap (GdkPixbuf   *dest,
                                  Pixmap       xpixmap,
                                  int          src_x,
                                  int          src_y,
                                  int          dest_x,
                                  int          dest_y,
                                  int          width,
                                  int          height)
{
    GdkDrawable *drawable;
    GdkPixbuf *retval;
    GdkColormap *cmap;

    ENTER;
    retval = NULL;

    drawable = gdk_xid_table_lookup (xpixmap);

    if (drawable)
        g_object_ref (G_OBJECT (drawable));
    else
        drawable = gdk_pixmap_foreign_new (xpixmap);

    cmap = get_cmap (drawable);

    /* GDK is supposed to do this but doesn't in GTK 2.0.2,
     * fixed in 2.0.3
     */
    if (width < 0)
        gdk_drawable_get_size (drawable, &width, NULL);
    if (height < 0)
        gdk_drawable_get_size (drawable, NULL, &height);

    retval = gdk_pixbuf_get_from_drawable (dest,
          drawable,
          cmap,
          src_x, src_y,
          dest_x, dest_y,
          width, height);

    if (cmap)
        g_object_unref (G_OBJECT (cmap));
    g_object_unref (G_OBJECT (drawable));

    RET(retval);
}

static GdkPixbuf*
apply_mask (GdkPixbuf *pixbuf,
            GdkPixbuf *mask)
{
  int w, h;
  int i, j;
  GdkPixbuf *with_alpha;
  guchar *src;
  guchar *dest;
  int src_stride;
  int dest_stride;

  ENTER;
  w = MIN (gdk_pixbuf_get_width (mask), gdk_pixbuf_get_width (pixbuf));
  h = MIN (gdk_pixbuf_get_height (mask), gdk_pixbuf_get_height (pixbuf));

  with_alpha = gdk_pixbuf_add_alpha (pixbuf, FALSE, 0, 0, 0);

  dest = gdk_pixbuf_get_pixels (with_alpha);
  src = gdk_pixbuf_get_pixels (mask);

  dest_stride = gdk_pixbuf_get_rowstride (with_alpha);
  src_stride = gdk_pixbuf_get_rowstride (mask);

  i = 0;
  while (i < h)
    {
      j = 0;
      while (j < w)
        {
          guchar *s = src + i * src_stride + j * 3;
          guchar *d = dest + i * dest_stride + j * 4;

          /* s[0] == s[1] == s[2], they are 255 if the bit was set, 0
           * otherwise
           */
          if (s[0] == 0)
            d[3] = 0;   /* transparent */
          else
            d[3] = 255; /* opaque */

          ++j;
        }

      ++i;
    }

  RET(with_alpha);
}

/* set the iconification destination */
static void set_iconification_dest( Window win, int x, int y, int w, int h )
{
  guint32 data[4];

  data[0] = x;
  data[1] = y;
  data[2] = w;
  data[3] = h;
  XChangeProperty(GDK_DISPLAY(), win,
        gdk_x11_get_xatom_by_name("_NET_WM_ICON_GEOMETRY"),
        XA_CARDINAL, 32, PropModeReplace, (guchar *)&data, 4);
}

static GdkPixbuf *
get_netwm_icon(Window tkwin, int iw, int ih)
{
    XWMHints *wmhints;
    GdkPixbuf *ret = NULL;

    ENTER;
    wmhints = XGetWMHints( GDK_DISPLAY(), GDK_ROOT_WINDOW() );

    /*
     * IconPixmapHint flag indicates that wmhints->icon_pixmap contains
     * valid data that is already in pixdata format, so we could process
     * it and turn it into a GTK image.
     */
    if( wmhints && (wmhints->flags & IconPixmapHint) ) {
        GdkPixmap *gdkPixmap;
        GdkPixbuf *gdkPixbuf = NULL;
    GdkColormap *colormap;

        colormap = gdk_colormap_get_system();

        gdkPixmap = gdk_pixmap_foreign_new(wmhints->icon_pixmap);
        gdkPixbuf = gdk_pixbuf_get_from_drawable(
                    NULL, gdkPixmap, colormap, 0, 0, 0, 0, iw, ih );
        ret = gdk_pixbuf_scale_simple( gdkPixbuf, 24, 24, GDK_INTERP_BILINEAR );
        gdk_pixbuf_unref(gdkPixbuf);
    XFree(wmhints);
    }

    RET(ret);
}

static void
free_pixels(guchar *pixels, gpointer data)
{
    g_free(pixels);
}

static GdkPixbuf *
get_wm_icon(Window tkwin, int iw, int ih)
{
    XWMHints *hints;
    Pixmap xpixmap = None, xmask = None;
    Window win;
    unsigned int w, h;
    int sd, result, format;
    GdkPixbuf *ret=NULL, *masked=NULL, *pixmap = NULL, *mask = NULL;
    Atom type = None;
    gulong *data = NULL;
    gulong  nitems;
    gulong  bytes_after;
    guchar *pixdata=NULL, *p=NULL;
    int     i;

    /* Important Notes:
     * According to freedesktop.org document:
     * http://standards.freedesktop.org/wm-spec/wm-spec-1.4.html#id2552223
     * _NET_WM_ICON contains an array of 32-bit packed CARDINAL ARGB.
     * However, this is incorrect. Actually it's an array of long integers.
     * Toolkits like gtk+ use unsigned long here to store icons.
     * Besides, according to manpage of XGetWindowProperty, when returned format,
     * is 32, the property data will be stored as an array of longs
     * (which in a 64-bit application will be 64-bit values that are
     * padded in the upper 4 bytes).
     */

    result = XGetWindowProperty(GDK_DISPLAY(),
                                tkwin,
                                gdk_x11_get_xatom_by_name("_NET_WM_ICON"),
                                0, G_MAXLONG,
                                0, XA_CARDINAL, &type, &format, &nitems,
                                &bytes_after, (void*)&data);
    /* g_debug("type=%d, format=%d, nitems=%d", type, format, nitems); */
    if(type != XA_CARDINAL || nitems <= 0 )
    {
        if(data) {
            XFree(data);
	    data=NULL;
        }
        result = -1;
    }

    if(result == Success)
    {
        gulong* pdata = data;
        gulong* pdata_end = data + nitems;
        gulong* max_icon = NULL;
        gulong max_w = 0, max_h = 0;

        /* get the largest icon available. */
        /* FIXME: should we try to find an icon whose size is closest to
         * iw and ih to reduce unnecessary resizing? */
        while(pdata + 2 < pdata_end)
        {
            gulong w = pdata[0];
            gulong h = pdata[1];
            gulong size = (w * h);

            pdata += 2;
            if( pdata + size > pdata_end ) /* corrupt icon */
                break;

            if( w > max_w && h > max_h )
            {
                max_icon = pdata;
                max_w = w;
                max_h = h;
            }

            /* rare special case: the desire size is the same as icon size */
            if( iw == w && ih == h )
                break;

            pdata += size;
        }

        if( max_icon )
        {
            gulong len = max_w * max_h;
            pixdata = g_new(guchar, len * 4);
            p = pixdata;

            i = 0;
            while(i < len)
            {
                guint argb, rgba;
                
                argb = max_icon[i];
                rgba = (argb << 8) | (argb >> 24);

                *p = rgba >> 24;
                ++p;
                *p = (rgba >> 16) & 0xff;
                ++p;
                *p = (rgba >> 8) & 0xff;
                ++p;
                *p = rgba & 0xff;
                ++p;

                ++i;
            }
            
            pixmap = gdk_pixbuf_new_from_data(pixdata,
                                              GDK_COLORSPACE_RGB,
                                              1, 8,
                                              max_w, max_h, max_w * 4,
                                              free_pixels,
                                              NULL);
        }
	else
	{
	    result = -1;
	}
        XFree(data);
	data=NULL;
    }

    if(result != Success)
    {
        hints = XGetWMHints(GDK_DISPLAY(), tkwin);
        result = (hints != NULL) ? Success : -1;

        if(result == Success)
        {
            if ((hints->flags & IconPixmapHint))
                xpixmap = hints->icon_pixmap;
            if ((hints->flags & IconMaskHint))
                xmask = hints->icon_mask;
            XFree(hints);
	    hints=NULL;
            result = (xpixmap != None)?Success:-1;
        }

        if(result != Success)
        {
            Pixmap *icons = NULL;
            Atom kwin_win_icon_atom = gdk_x11_get_xatom_by_name("KWM_WIN_ICON");
            result = XGetWindowProperty(GDK_DISPLAY(), tkwin,
                                        kwin_win_icon_atom,
                                        0, G_MAXLONG,
                                        False,
                                        kwin_win_icon_atom,
                                        &type, &format, &nitems,
                                        &bytes_after, (void*)&icons);
            if(type != kwin_win_icon_atom)
            {
	        if( icons ) {
                    XFree(icons);
		    icons=NULL;
		}
                result = -1;
            }
            if(result == Success)
            {
                xpixmap = icons[0];
                xmask   = icons[1];
                result = (xpixmap != None) ? Success : -1;
            }
        }

        if(result == Success)
        {
            result = XGetGeometry(GDK_DISPLAY(),
                                  xpixmap, &win,
                                  &sd, &sd, &w, &h,
                                  (guint *)&sd, (guint *)&sd) ? Success : -1;
        }

        if(result == Success) 
        {
            DBG("tkwin=%x icon pixmap w=%d h=%d\n", tkwin, w, h);
            pixmap = _wnck_gdk_pixbuf_get_from_pixmap (NULL, xpixmap, 0, 0, 0, 0, w, h);
            result = pixmap?Success:-1;
        }

        if(result == Success)
        {
            if (xmask != None && XGetGeometry(GDK_DISPLAY(), xmask,
                                              &win, &sd, &sd, &w, &h,
                                              (guint *)&sd, (guint *)&sd))
            {
                mask = _wnck_gdk_pixbuf_get_from_pixmap (NULL, xmask, 0, 0, 0, 0, w, h);
                if (mask)
                {
                    masked = apply_mask (pixmap, mask);
                    g_object_unref (G_OBJECT (pixmap));
                    g_object_unref (G_OBJECT (mask));
                    pixmap = masked;
                }
            }
        }
    }

    if (!pixmap)
        RET(NULL);

    ret = gdk_pixbuf_scale_simple (pixmap, iw, ih, GDK_INTERP_TILES);
    g_object_unref(pixmap);

    return ret;
}

inline static GdkPixbuf*
get_generic_icon(taskbar *tb)
{
    ENTER;
    g_object_ref(tb->gen_pixbuf);
    RET(tb->gen_pixbuf);
}

static void
tk_update_icon (taskbar *tb, task *tk, Atom a)
{
    GdkPixbuf *pixbuf;

    ENTER;
    g_assert ((tb != NULL) && (tk != NULL));
    g_return_if_fail(tk != NULL);

    pixbuf = tk->pixbuf;
    if (a == a_NET_WM_ICON || a == None) {
        tk->pixbuf = get_netwm_icon(tk->win, tb->iconsize, tb->iconsize);
        tk->using_netwm_icon = (tk->pixbuf != NULL);
    }
    if (!tk->using_netwm_icon)
        tk->pixbuf = get_wm_icon(tk->win, tb->iconsize, tb->iconsize);
    if (!tk->pixbuf)
        tk->pixbuf = get_generic_icon(tb); // always exists
    if (pixbuf != tk->pixbuf) {
        if (pixbuf)
            g_object_unref(pixbuf);
    }
    RET();
}

static gboolean on_flash_win( task *tk )
{
    tk->flash_state = !tk->flash_state;
    gtk_widget_set_state(tk->button,
          tk->flash_state ? GTK_STATE_SELECTED : tk->tb->normal_state);
    gtk_widget_queue_draw(tk->button);
    return TRUE;
}

static void
tk_flash_window( task *tk )
{
    gint interval;
    tk->flash = 1;
    tk->flash_state = !tk->flash_state;
    if (tk->flash_timeout)
        return;
    g_object_get( gtk_widget_get_settings(tk->button),
          "gtk-cursor-blink-time", &interval, NULL );
    tk->flash_timeout = g_timeout_add(interval, (GSourceFunc)on_flash_win, tk);
}

static void
tk_unflash_window( task *tk )
{
    tk->flash = tk->flash_state = 0;
    if (tk->flash_timeout) {
        g_source_remove(tk->flash_timeout);
        tk->flash_timeout = 0;
    }
}

static void
tk_raise_window( task *tk, guint32 time )
{
    if (tk->desktop != -1 && tk->desktop != tk->tb->cur_desk)
    {
        Xclimsg(GDK_ROOT_WINDOW(), a_NET_CURRENT_DESKTOP, tk->desktop, 0, 0, 0, 0);
        XSync (gdk_display, False);
    }
    XSetInputFocus (GDK_DISPLAY(), tk->win, RevertToNone, CurrentTime);
    XRaiseWindow (GDK_DISPLAY(), tk->win);
    Xclimsg(tk->win, a_NET_ACTIVE_WINDOW, 2, time, 0, 0, 0);
    DBG("XRaiseWindow %x\n", tk->win);
}

static void
on_tk_leave( GtkWidget *widget, task *tk)
{
    ENTER;
/*
    gtk_widget_set_state(widget,
          (tk->focused) ? tk->tb->focused_state : tk->tb->normal_state);
*/
    RET();
}


static void
on_tk_enter( GtkWidget *widget, task *tk )
{
    ENTER;
/*
    gtk_widget_set_state(widget,
          (tk->focused) ? tk->tb->focused_state : tk->tb->normal_state);
*/
    RET();
}

static gboolean delay_active_win(task* tk)
{
    /* FIXME: gtk_get_current_event_time() often returns 0.
       However, passing 0 as time for this function is not OK. */
    tk_raise_window(tk, gtk_get_current_event_time() );
    tk->tb->dnd_activate = 0;
    return FALSE;
}

static gboolean
on_tk_drag_motion( GtkWidget *widget,
      GdkDragContext *drag_context,
      gint x, gint y,
      guint time, task *tk)
{
    /* prevent excessive motion notification */
    if (!tk->tb->dnd_activate) {
        tk->tb->dnd_activate = g_timeout_add(DRAG_ACTIVE_DELAY,
              (GSourceFunc)delay_active_win, tk);
    }
    gdk_drag_status (drag_context,0,time);
    return TRUE;
}

static void
on_tk_drag_leave (GtkWidget *widget,
      GdkDragContext *drag_context,
      guint time, task *tk)
{
    if (tk->tb->dnd_activate) {
        g_source_remove(tk->tb->dnd_activate);
        tk->tb->dnd_activate = 0;
    }
    return;
}

#if 0
static gboolean
on_tk_expose(GtkWidget *widget, GdkEventExpose *event, task *tk)
{
    GtkStateType state;
    ENTER;
    state = (tk->focused) ? tk->tb->focused_state : tk->tb->normal_state;
    if (GTK_WIDGET_STATE(widget) != state) {
        gtk_widget_set_state(widget, state);
        gtk_widget_queue_draw(widget);
    } else {
        if( ! tk->flash || 0 == tk->flash_state ) {
            gtk_paint_box (widget->style, widget->window,
                  state,
                  (tk->focused) ? GTK_SHADOW_IN : GTK_SHADOW_OUT,
                  &event->area, widget, "button",
                  widget->allocation.x, widget->allocation.y,
                  widget->allocation.width, widget->allocation.height);
        } else {
            gdk_draw_rectangle( widget->window,
                                widget->style->bg_gc[GTK_STATE_SELECTED],
                                TRUE, 0, 0,
                                widget->allocation.width,
                                widget->allocation.height );
        }
        /*
        _gtk_button_paint(GTK_BUTTON(widget), &event->area, state,
              (tk->focused) ? GTK_SHADOW_IN : GTK_SHADOW_OUT,
              "button",  "buttondefault");
        */
        gtk_container_propagate_expose(GTK_CONTAINER(widget), GTK_BIN(widget)->child, event);
    }
    RET(FALSE);
}
#endif

static gint
on_tk_scroll_event (GtkWidget *widget, GdkEventScroll *event, task *tk)
{
    ENTER;
    if( ! tk->tb->use_mouse_wheel )
        return TRUE;
    if (event->direction == GDK_SCROLL_UP) {
        GdkWindow *gdkwindow;

        gdkwindow = gdk_xid_table_lookup (tk->win);
        if (gdkwindow)
            gdk_window_show (gdkwindow);
        else
            XMapRaised (GDK_DISPLAY(), tk->win);
        XSetInputFocus (GDK_DISPLAY(), tk->win, RevertToNone, CurrentTime);
        DBG("XMapRaised  %x\n", tk->win);
    } else if (event->direction == GDK_SCROLL_DOWN) {
        DBG("tb->ptk = %x\n", (tk->tb->ptk) ? tk->tb->ptk->win : 0);
        XIconifyWindow (GDK_DISPLAY(), tk->win, DefaultScreen(GDK_DISPLAY()));
        DBG("XIconifyWindow %x\n", tk->win);
    }

    XSync (gdk_display, False);
    RET(TRUE);
}

static gboolean
on_tk_btn_press_event(GtkWidget *widget, GdkEventButton *event, task *tk)
{
    if( event->type == GDK_BUTTON_PRESS && event->button == 3 )
    {
        tk->tb->menutask = tk;
        gtk_menu_popup (GTK_MENU (tk->tb->menu), NULL, NULL, NULL, NULL, event->button, event->time);
        return TRUE;
    }
    return FALSE;
}

static gboolean
on_tk_btn_release_event(GtkWidget *widget, GdkEventButton *event, task *tk)
{
    XWindowAttributes xwa;

    if( event->type != GDK_BUTTON_RELEASE )
        return FALSE;

    if( event->button == 1 )
    {
	if ((tk->desktop != -1) && (tk->desktop != tk->tb->cur_desk))
            Xclimsg(GDK_ROOT_WINDOW(), a_NET_CURRENT_DESKTOP, tk->desktop, 0, 0, 0, 0);

        if (tk->iconified)
        {
            if(use_net_active)
                Xclimsg(tk->win, a_NET_ACTIVE_WINDOW, 2, event->time, 0, 0, 0);
            else
            {
                GdkWindow *gdkwindow;
                gdkwindow = gdk_xid_table_lookup (tk->win);
                if (gdkwindow)
                    gdk_window_show (gdkwindow);
                else
                    XMapRaised (GDK_DISPLAY(), tk->win);
                XSync (GDK_DISPLAY(), False);
            }
            /* if window isn't on current viewport, we change viewport */
            XGetWindowAttributes(GDK_DISPLAY(), tk->win, &xwa);
            Xclimsg(tk->win, a_NET_DESKTOP_VIEWPORT, xwa.x, xwa.y, 0, 0, 0);
        }
        else
        {
            if (tk->focused || tk == tk->tb->ptk)
                XIconifyWindow (GDK_DISPLAY(), tk->win, DefaultScreen(GDK_DISPLAY()));
            else
                tk_raise_window( tk, event->time );
        }
    }
    else if (event->button == 2)
    {
        Xclimsg(tk->win, a_NET_WM_STATE,
              2 /*a_NET_WM_STATE_TOGGLE*/,
              a_NET_WM_STATE_SHADED,
              0, 0, 0);
    }
    XSync (gdk_display, False);

    return FALSE;
}


static void
tk_update(gpointer key, task *tk, taskbar *tb)
{
    ENTER;
    g_assert ((tb != NULL) && (tk != NULL));
    if (task_visible(tb, tk)) {
        /* g_debug( "SET_ACTIVE: %p, %d", tk->button, tk->focused ); */
        if( gtk_toggle_button_get_active( (GtkToggleButton*)tk->button) != tk->focused )
            gtk_toggle_button_set_active( (GtkToggleButton*)tk->button, tk->focused );

        gtk_widget_show(tk->button);
        if (tb->tooltips) {
            //DBG2("tip %x %s\n", tk->win, tk->name);
            gtk_widget_set_tooltip_text( tk->button, tk->name );
        }
    RET();
    }
    gtk_widget_hide(tk->button);
    RET();
}

static void
tk_display(taskbar *tb, task *tk)
{
    ENTER;
    tk_update(NULL, tk, tb);
    RET();
}

static void
tb_display(taskbar *tb)
{
    ENTER;
    if (tb->wins)
        g_hash_table_foreach(tb->task_list, (GHFunc) tk_update, (gpointer) tb);
    RET();

}

static void on_tk_btn_size_allocate(GtkWidget* btn, GtkAllocation* alloc, task* tk)
{
    int x, y;
    if( ! GTK_WIDGET_REALIZED(btn) )
        return;
    gdk_window_get_origin(GTK_BUTTON(btn)->event_window, &x, &y);
    set_iconification_dest( tk->win, x, y, alloc->width, alloc->height );
}

static void
tk_build_gui(taskbar *tb, task *tk)
{
    GtkWidget *w1;

    ENTER;
    g_assert ((tb != NULL) && (tk != NULL));

    /* NOTE
     * 1. the extended mask is sum of taskbar and pager needs
     * see bug [ 940441 ] pager loose track of windows
     *
     * Do not change event mask to gtk windows spwaned by this gtk client
     * this breaks gtk internals */
    if (!FBPANEL_WIN(tk->win))
        XSelectInput (GDK_DISPLAY(), tk->win, PropertyChangeMask | StructureNotifyMask);

    /* button */

    tk->button = gtk_toggle_button_new();
    if( tb->flat_button )
        gtk_button_set_relief( (GtkButton*)tk->button, GTK_RELIEF_NONE );
    else
        gtk_button_set_relief( (GtkButton*)tk->button, GTK_RELIEF_NORMAL);

    gtk_widget_show(tk->button);
    gtk_container_set_border_width(GTK_CONTAINER(tk->button), 0);
    gtk_widget_add_events (tk->button, GDK_BUTTON_RELEASE_MASK );
    g_signal_connect(tk->button, "button_press_event",
          G_CALLBACK(on_tk_btn_press_event), (gpointer)tk);
    g_signal_connect(tk->button, "button_release_event",
          G_CALLBACK(on_tk_btn_release_event), (gpointer)tk);
/*
    g_signal_connect_after (G_OBJECT (tk->button), "leave",
          G_CALLBACK (on_tk_leave), (gpointer) tk);
    g_signal_connect_after (G_OBJECT (tk->button), "enter",
          G_CALLBACK (on_tk_enter), (gpointer) tk);
*/
    g_signal_connect(tk->button, "size-allocate",
          G_CALLBACK(on_tk_btn_size_allocate), (gpointer)tk);

#if 0
    g_signal_connect_after (G_OBJECT (tk->button), "expose-event",
          G_CALLBACK (on_tk_expose), (gpointer) tk);
#endif
    gtk_drag_dest_set( tk->button, 0, NULL, 0, 0);
    g_signal_connect (G_OBJECT (tk->button), "drag-motion",
          G_CALLBACK (on_tk_drag_motion), (gpointer) tk);
    g_signal_connect (G_OBJECT (tk->button), "drag-leave",
          G_CALLBACK (on_tk_drag_leave), (gpointer) tk);
    g_signal_connect_after(G_OBJECT(tk->button), "scroll-event",
            G_CALLBACK(on_tk_scroll_event), (gpointer)tk);

    /* pix and name */
    w1 = tb->plug->panel->my_box_new(FALSE, 1);
    gtk_container_set_border_width(GTK_CONTAINER(w1), 0);

    /* pix */
    //get_wmclass(tk);
    tk_update_icon(tb, tk, None);
    tk->image = gtk_image_new_from_pixbuf(tk->pixbuf );
    gtk_widget_show(tk->image);
    gtk_box_pack_start(GTK_BOX(w1), tk->image, FALSE, FALSE, 0);

    /* name */
    tk->label = gtk_label_new(tk->iconified ? tk->iname : tk->name);
    update_label_orient( tk->label, tb->plug );
    if (!tb->icons_only)
        gtk_widget_show(tk->label);
    gtk_box_pack_start(GTK_BOX(w1), tk->label, TRUE, TRUE, 0);
    gtk_widget_show(w1);
    gtk_container_add (GTK_CONTAINER (tk->button), w1);

    //gtk_container_add (GTK_CONTAINER (tk->eb), tk->button);
    gtk_box_pack_start(GTK_BOX(tb->bar), tk->button, FALSE, TRUE, 0);
    GTK_WIDGET_UNSET_FLAGS (tk->button, GTK_CAN_FOCUS);
    GTK_WIDGET_UNSET_FLAGS (tk->button, GTK_CAN_DEFAULT);

    gtk_widget_show(tk->button);
    if (!task_visible(tb, tk)) {
        gtk_widget_hide(tk->button);
    }

    if (tk->urgency) {
        /* Flash button for window with urgency hint */
        tk_flash_window(tk);
    }
    RET();
}

/* tell to remove element with zero refcount */
static gboolean
tb_remove_stale_tasks(Window *win, task *tk, gpointer data)
{
    ENTER;
    if (tk->refcount-- == 0) {
        //DBG("tb_net_list <del>: 0x%x %s\n", tk->win, tk->name);
        del_task(tk->tb, tk, 0);
        RET(TRUE);
    }
    RET(FALSE);
}

/*****************************************************
 * handlers for NET actions                          *
 *****************************************************/


static void
tb_net_client_list(GtkWidget *widget, taskbar *tb)
{
    int i;
    task *tk;

    ENTER;
    if (tb->wins)
        XFree(tb->wins);
    tb->wins = get_xaproperty (GDK_ROOT_WINDOW(), a_NET_CLIENT_LIST, XA_WINDOW, &tb->win_num);
    if (!tb->wins)
        RET();
    for (i = 0; i < tb->win_num; i++) {
        if ((tk = g_hash_table_lookup(tb->task_list, &tb->wins[i]))) {
            ++tk->refcount;
        } else {
            NetWMWindowType nwwt;
            NetWMState nws;

            get_net_wm_state(tb->wins[i], &nws);
            if (!accept_net_wm_state(&nws, tb->accept_skip_pager))
                continue;
            get_net_wm_window_type(tb->wins[i], &nwwt);
            if (!accept_net_wm_window_type(&nwwt))
                continue;

            tk = g_new0(task, 1);
            tk->refcount = 1;
            ++tb->num_tasks;
            tk->win = tb->wins[i];
            tk->tb = tb;
            tk->iconified = (get_wm_state(tk->win) == IconicState);
            tk->desktop = get_net_wm_desktop(tk->win);
            tk->nws = nws;
            tk->nwwt = nwwt;
            if( tb->use_urgency_hint && tk_has_urgency(tk)) {
                tk->urgency = 1;
            }

            tk_build_gui(tb, tk);
            tk_set_names(tk);
            g_hash_table_insert(tb->task_list, &tk->win, tk);
            DBG("adding %08x(%p) %s\n", tk->win, FBPANEL_WIN(tk->win), tk->name);
        }
    }

    /* remove windows that arn't in the NET_CLIENT_LIST anymore */
    g_hash_table_foreach_remove(tb->task_list, (GHRFunc) tb_remove_stale_tasks, NULL);
    tb_display(tb);
    RET();
}

static void
tb_net_current_desktop(GtkWidget *widget, taskbar *tb)
{
    ENTER;
    tb->cur_desk = get_net_current_desktop();
    tb_display(tb);
    RET();
}

static void
tb_net_number_of_desktops(GtkWidget *widget, taskbar *tb)
{
    ENTER;
    tb->desk_num = get_net_number_of_desktops();
    tb_display(tb);
    RET();
}


/* set new active window. if that happens to be us, then remeber
 * current focus to use it for iconify command */
static void
tb_net_active_window(GtkWidget *widget, taskbar *tb)
{
    Window *f;
    task *ntk, *ctk;
    int drop_old, make_new;

    ENTER;
    g_assert (tb != NULL);
    drop_old = make_new = 0;
    ctk = tb->focused;
    ntk = NULL;
    f = get_xaproperty(GDK_ROOT_WINDOW(), a_NET_ACTIVE_WINDOW, XA_WINDOW, 0);
    DBG("FOCUS=%x\n", f ? *f : 0);
    if (!f) {
        drop_old = 1;
        tb->ptk = NULL;
    } else {
        if (*f == tb->topxwin) {
            if (ctk) {
                tb->ptk = ctk;
                drop_old = 1;
            }
        } else {
            tb->ptk = NULL;
            ntk = find_task(tb, *f);
            if (ntk != ctk) {
                drop_old = 1;
                make_new = 1;
            }
        }
        XFree(f);
    }
    if (ctk && drop_old) {
        ctk->focused = 0;
        tb->focused = NULL;
        tk_display(tb, ctk);
        DBG("old focus was dropped\n");
    }
    if (ntk && make_new) {
        ntk->focused = 1;
        tb->focused = ntk;
        tk_display(tb, ntk);
        DBG("new focus was set\n");
    }
    RET();
}

/* For older Xlib headers */
#ifndef XUrgencyHint
#define XUrgencyHint (1 << 8)
#endif

static gboolean
tk_has_urgency( task* tk )
{
    XWMHints* hints;

    tk->urgency = 0;
    hints = (XWMHints *) get_xaproperty (tk->win, XA_WM_HINTS, XA_WM_HINTS, 0);
    if (hints) {
        if (hints->flags & XUrgencyHint) /* Got urgency hint */
            tk->urgency = 1;
        XFree( hints );
    }
    return tk->urgency;
}

static void
tb_propertynotify(taskbar *tb, XEvent *ev)
{
    Atom at;
    Window win;

    ENTER;
    DBG("win=%x\n", ev->xproperty.window);

    /* The property is deleted */
    if( ((XPropertyEvent*)ev)->state == 1 )
        return;

    at = ev->xproperty.atom;
    win = ev->xproperty.window;
    if (win != GDK_ROOT_WINDOW()) {
        task *tk = find_task(tb, win);

        if (!tk) RET();
            DBG("win=%x\n", ev->xproperty.window);
        if (at == a_NET_WM_DESKTOP) {
                DBG("NET_WM_DESKTOP\n");
            tk->desktop = get_net_wm_desktop(win);
            tb_display(tb);
        }  else if (at == XA_WM_NAME) {
                DBG("WM_NAME\n");
            tk_set_names(tk);
            //tk_display(tb, tk);
        }  else if (at == XA_WM_CLASS) {
                DBG("WM_CLASS\n");
                //get_wmclass(tk);
        } else if (at == a_WM_STATE)    {
                DBG("WM_STATE\n");
            /* iconified state changed? */
            tk->iconified = (get_wm_state (tk->win) == IconicState);
                tk_set_names(tk);
            //tk_display(tb, tk);
        } else if (at == XA_WM_HINTS)   {
            /* some windows set their WM_HINTS icon after mapping */
            DBG("XA_WM_HINTS\n");
                //get_wmclass(tk);
            tk_update_icon (tb, tk, XA_WM_HINTS);
            gtk_image_set_from_pixbuf (GTK_IMAGE(tk->image), tk->pixbuf);
                if (tb->use_urgency_hint) {
                    if (tk_has_urgency(tk)) {
                        //tk->urgency = 1;
                        tk_flash_window(tk);
                    } else {
                        //tk->urgency = 0;
                        tk_unflash_window(tk);
                    }
                }
            } else if (at == a_NET_WM_STATE) {
                NetWMState nws;

            DBG("_NET_WM_STATE\n");
            get_net_wm_state(tk->win, &nws);
                if (!accept_net_wm_state(&nws, tb->accept_skip_pager)) {
            del_task(tb, tk, 1);
            tb_display(tb);
            }
        } else if (at == a_NET_WM_ICON) {
            DBG("_NET_WM_ICON\n");
                DBG("#0 %d\n", GDK_IS_PIXBUF (tk->pixbuf));
                tk_update_icon (tb, tk, a_NET_WM_ICON);
                DBG("#1 %d\n", GDK_IS_PIXBUF (tk->pixbuf));
            gtk_image_set_from_pixbuf (GTK_IMAGE(tk->image), tk->pixbuf);
                DBG("#2 %d\n", GDK_IS_PIXBUF (tk->pixbuf));
        } else if (at == a_NET_WM_WINDOW_TYPE) {
                NetWMWindowType nwwt;

            DBG("_NET_WM_WINDOW_TYPE\n");
            get_net_wm_window_type(tk->win, &nwwt);
                if (!accept_net_wm_window_type(&nwwt)) {
            del_task(tb, tk, 1);
            tb_display(tb);
            }
        } else {
                DBG("at = %d\n", at);
        }
    }
    RET();
}

static GdkFilterReturn
tb_event_filter( XEvent *xev, GdkEvent *event, taskbar *tb)
{

    ENTER;
    //RET(GDK_FILTER_CONTINUE);
    g_assert(tb != NULL);
    if (xev->type == PropertyNotify )
        tb_propertynotify(tb, xev);
    RET(GDK_FILTER_CONTINUE);
}

static void
menu_close_window(GtkWidget *widget, taskbar *tb)
{
    ENTER;
    DBG("win %x\n", tb->menutask->win);
    XSync (GDK_DISPLAY(), 0);
    //XKillClient(GDK_DISPLAY(), tb->menutask->win);
    Xclimsgwm(tb->menutask->win, a_WM_PROTOCOLS, a_WM_DELETE_WINDOW);
    XSync (GDK_DISPLAY(), 0);
    RET();
}


static void
menu_raise_window(GtkWidget *widget, taskbar *tb)
{
    ENTER;
    DBG("win %x\n", tb->menutask->win);
    if (tb->menutask->desktop != -1 && tb->menutask->desktop != tb->cur_desk)
        Xclimsg(GDK_ROOT_WINDOW(), a_NET_CURRENT_DESKTOP, tb->menutask->desktop, 0, 0, 0, 0);
    XMapRaised(GDK_DISPLAY(), tb->menutask->win);
    RET();
}


static void
menu_iconify_window(GtkWidget *widget, taskbar *tb)
{
    ENTER;
    DBG("win %x\n", tb->menutask->win);
    XIconifyWindow (GDK_DISPLAY(), tb->menutask->win, DefaultScreen(GDK_DISPLAY()));
    RET();
}

static void
menu_restore_window(GtkWidget *widget, taskbar *tb)
{
    GdkWindow* win;
    ENTER;
    DBG("win %x\n", tb->menutask->win);
    win = gdk_window_foreign_new( tb->menutask->win );
    gdk_window_unmaximize( win );
    gdk_window_unref( win );
    RET();
}

static void
menu_maximize_window(GtkWidget *widget, taskbar *tb)
{
    GdkWindow* win;
    ENTER;
    DBG("win %x\n", tb->menutask->win);
    win = gdk_window_foreign_new( tb->menutask->win );
    gdk_window_maximize( win );
    gdk_window_unref( win );
    RET();
}

static void
menu_move_to_workspace( GtkWidget* mi, taskbar* tb )
{
    GdkWindow* win;
    int num = GPOINTER_TO_INT( g_object_get_data( G_OBJECT(mi), "num" ) );
    _wnck_change_workspace( DefaultScreenOfDisplay(GDK_DISPLAY()), tb->menutask->win, num );
}

static GtkWidget *
taskbar_make_menu(taskbar *tb)
{
    GtkWidget *mi, *menu, *workspace_menu = NULL;
    int i;
    char label[128];

    ENTER;
    menu = gtk_menu_new ();

    mi = gtk_menu_item_new_with_mnemonic (_("_Raise"));
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), mi);
    g_signal_connect(G_OBJECT(mi), "activate", (GCallback)menu_raise_window, tb);

    mi = gtk_menu_item_new_with_mnemonic (_("R_estore"));
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), mi);
    g_signal_connect(G_OBJECT(mi), "activate", (GCallback)menu_restore_window, tb);

    mi = gtk_menu_item_new_with_mnemonic (_("Ma_ximize"));
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), mi);
    g_signal_connect(G_OBJECT(mi), "activate", (GCallback)menu_maximize_window, tb);

    mi = gtk_menu_item_new_with_mnemonic (_("Ico_nify"));
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), mi);
    g_signal_connect(G_OBJECT(mi), "activate", (GCallback)menu_iconify_window, tb);

    if( tb->desk_num > 1 )
    {
        workspace_menu = gtk_menu_new();
        for( i = 1; i <= tb->desk_num; ++i )
        {
            if (i <= 9)
            {
                g_snprintf( label, 128, _("Workspace _%d"), i);
                mi = gtk_menu_item_new_with_mnemonic( label );
            }
            else
            {
                g_snprintf( label, 128, _("Workspace %d"), i);
                mi = gtk_menu_item_new_with_label( label );
            }
            g_object_set_data( G_OBJECT(mi), "num", GINT_TO_POINTER(i - 1) );
            g_signal_connect( mi, "activate", G_CALLBACK(menu_move_to_workspace), tb );
            gtk_menu_shell_append( (GtkMenuShell*)workspace_menu, mi );
        }
        gtk_menu_shell_append( GTK_MENU_SHELL (workspace_menu),
                                                   gtk_separator_menu_item_new());
        mi = gtk_menu_item_new_with_mnemonic(_("_All workspaces"));
        g_object_set_data( G_OBJECT(mi), "num", GINT_TO_POINTER(ALL_WORKSPACES) );
        g_signal_connect( mi, "activate", G_CALLBACK(menu_move_to_workspace), tb );
        gtk_menu_shell_append( (GtkMenuShell*)workspace_menu, mi );

        gtk_widget_show_all( workspace_menu );

        mi = gtk_menu_item_new_with_mnemonic (_("_Move to Workspace"));
        gtk_menu_shell_append (GTK_MENU_SHELL (menu), mi);

        gtk_menu_item_set_submenu( GTK_MENU_ITEM(mi), workspace_menu );
        workspace_menu = mi;
    }

    /* we want this item to be farest from mouse pointer */
    mi = gtk_menu_item_new_with_mnemonic (_("_Close Window"));
    if (tb->plug->panel->edge == EDGE_BOTTOM)
    {
        gtk_menu_shell_prepend (GTK_MENU_SHELL (menu), gtk_separator_menu_item_new());
        gtk_menu_shell_prepend (GTK_MENU_SHELL (menu), mi);
    }
    else
    {
//        gtk_menu_shell_prepend (GTK_MENU_SHELL (menu), workspace_menu);

        gtk_menu_shell_append (GTK_MENU_SHELL (menu), gtk_separator_menu_item_new());
        gtk_menu_shell_append (GTK_MENU_SHELL (menu), mi);
    }
    g_signal_connect(G_OBJECT(mi), "activate", (GCallback)menu_close_window, tb);
    gtk_widget_show_all (menu);

    RET(menu);
}


static void
taskbar_build_gui(Plugin *p)
{
    taskbar *tb = (taskbar *)p->priv;
    GtkOrientation  bo;

    ENTER;

    bo = (tb->plug->panel->orientation == ORIENT_HORIZ) ? GTK_ORIENTATION_HORIZONTAL : GTK_ORIENTATION_VERTICAL;
    tb->bar = gtk_bar_new(bo, tb->spacing);
    if (tb->icons_only) {
        gtk_bar_set_max_child_size(GTK_BAR(tb->bar),
              GTK_WIDGET(p->panel->box)->allocation.height -2);
     } else
        gtk_bar_set_max_child_size(GTK_BAR(tb->bar), tb->task_width_max);
    gtk_container_add (GTK_CONTAINER (p->pwid), tb->bar);
    gtk_widget_show(tb->bar);

    tb->gen_pixbuf =  gdk_pixbuf_new_from_xpm_data((const char **)icon_xpm);

    gdk_window_add_filter(NULL, (GdkFilterFunc)tb_event_filter, tb );

    g_signal_connect (G_OBJECT (fbev), "current_desktop",
          G_CALLBACK (tb_net_current_desktop), (gpointer) tb);
    g_signal_connect (G_OBJECT (fbev), "active_window",
          G_CALLBACK (tb_net_active_window), (gpointer) tb);
    g_signal_connect (G_OBJECT (fbev), "number_of_desktops",
          G_CALLBACK (tb_net_number_of_desktops), (gpointer) tb);
    g_signal_connect (G_OBJECT (fbev), "client_list",
          G_CALLBACK (tb_net_client_list), (gpointer) tb);

    tb->desk_num = get_net_number_of_desktops();
    tb->cur_desk = get_net_current_desktop();
    tb->focused = NULL;

    tb->menu = taskbar_make_menu(tb);
    gtk_container_set_border_width(GTK_CONTAINER(p->pwid), 0);
    gtk_widget_show_all(tb->bar);
    RET();
}

void net_active_detect()
{
    int nitens;
    Atom *data;

    data = get_xaproperty(GDK_ROOT_WINDOW(), a_NET_SUPPORTED, XA_ATOM, &nitens);
    if (!data)
    return;

    while (nitens > 0)
    if(data[--nitens]==a_NET_ACTIVE_WINDOW) {
        use_net_active = TRUE;
            break;
        }

    XFree(data);
}

static int
taskbar_constructor(Plugin *p, char** fp)
{
    taskbar *tb;
    line s;
    GtkRequisition req;

    ENTER;

    gtk_rc_parse_string(taskbar_rc);

    /* FIXME: Is there any better way to do this? */
    p->pwid = gtk_event_box_new();
    GTK_WIDGET_SET_FLAGS( p->pwid, GTK_NO_WINDOW );
    gtk_widget_set_name(p->pwid, "taskbar");

    get_button_spacing(&req, GTK_CONTAINER(p->pwid), "");

    net_active_detect();

    tb = g_new0(taskbar, 1);
    tb->plug = p;
    p->priv = tb;

    if (p->panel->orientation == ORIENT_HORIZ) {
        tb->iconsize = GTK_WIDGET(p->panel->box)->allocation.height - req.height;
        DBG("pwid height = %d\n", GTK_WIDGET(p->pwid)->allocation.height);
    } else
        tb->iconsize = 24;
    tb->topxwin           = p->panel->topxwin;
    tb->tooltips          = 1;
    tb->icons_only        = 0;
    tb->accept_skip_pager = 1;
    tb->show_iconified    = 1;
    tb->show_mapped       = 1;
    tb->show_all_desks    = 0;
    tb->task_width_max    = TASK_WIDTH_MAX;
    tb->task_list         = g_hash_table_new(g_int_hash, g_int_equal);
    tb->focused_state     = GTK_STATE_ACTIVE;
    tb->normal_state      = GTK_STATE_NORMAL;
    tb->spacing           = 1;
    tb->use_mouse_wheel   = 1;
    tb->use_urgency_hint  = 1;
    s.len = 256;
    if( fp )
    {
        while (lxpanel_get_line(fp, &s) != LINE_BLOCK_END) {
            if (s.type == LINE_NONE) {
                ERR( "taskbar: illegal token %s\n", s.str);
                goto error;
            }
            if (s.type == LINE_VAR) {
                if (!g_ascii_strcasecmp(s.t[0], "tooltips")) {
                    tb->tooltips = str2num(bool_pair, s.t[1], 1);
                } else if (!g_ascii_strcasecmp(s.t[0], "IconsOnly")) {
                    tb->icons_only = str2num(bool_pair, s.t[1], 0);
                } else if (!g_ascii_strcasecmp(s.t[0], "AcceptSkipPager")) {
                    tb->accept_skip_pager = str2num(bool_pair, s.t[1], 1);
                } else if (!g_ascii_strcasecmp(s.t[0], "ShowIconified")) {
                    tb->show_iconified = str2num(bool_pair, s.t[1], 1);
                } else if (!g_ascii_strcasecmp(s.t[0], "ShowMapped")) {
                    tb->show_mapped = str2num(bool_pair, s.t[1], 1);
                } else if (!g_ascii_strcasecmp(s.t[0], "ShowAllDesks")) {
                    tb->show_all_desks = str2num(bool_pair, s.t[1], 0);
                } else if (!g_ascii_strcasecmp(s.t[0], "MaxTaskWidth")) {
                    tb->task_width_max = atoi(s.t[1]);
                    DBG("task_width_max = %d\n", tb->task_width_max);
                } else if (!g_ascii_strcasecmp(s.t[0], "spacing")) {
                    tb->spacing = atoi(s.t[1]);
                } else if (!g_ascii_strcasecmp(s.t[0], "UseMouseWheel")) {
                    tb->use_mouse_wheel = str2num(bool_pair, s.t[1], 1);
                } else if (!g_ascii_strcasecmp(s.t[0], "UseUrgencyHint")) {
                    tb->use_urgency_hint = str2num(bool_pair, s.t[1], 1);
                } else if (!g_ascii_strcasecmp(s.t[0], "FlatButton")) {
                    tb->flat_button = str2num(bool_pair, s.t[1], 1);
                } else {
                    ERR( "taskbar: unknown var %s\n", s.t[0]);
                    goto error;
                }
            } else {
                ERR( "taskbar: illegal in this context %s\n", s.str);
                goto error;
            }
        }
    }
    taskbar_build_gui(p);
    tb_net_client_list(NULL, tb);
    tb_display(tb);
    tb_net_active_window(NULL, tb);
    RET(1);

 error:
    taskbar_destructor(p);
    RET(0);
}


static void
taskbar_destructor(Plugin *p)
{
    taskbar *tb = (taskbar *)p->priv;
    g_hash_table_foreach( tb->task_list, (GHFunc)del_task, NULL );

    g_signal_handlers_disconnect_by_func(G_OBJECT (fbev), tb_net_current_desktop, tb);
    g_signal_handlers_disconnect_by_func(G_OBJECT (fbev), tb_net_active_window, tb);
    g_signal_handlers_disconnect_by_func(G_OBJECT (fbev), tb_net_number_of_desktops, tb);
    g_signal_handlers_disconnect_by_func(G_OBJECT (fbev), tb_net_client_list, tb);
    gdk_window_remove_filter(NULL, (GdkFilterFunc)tb_event_filter, tb );
    g_hash_table_destroy(tb->task_list);
    /* The widget is destroyed in plugin_stop().
    gtk_widget_destroy(tb->bar);
    */
    gtk_widget_destroy(tb->menu);
}

static void
update_task_button( gpointer key, task* tk, taskbar* tb )
{
    if( tb->icons_only )
        gtk_widget_hide( tk->label );
    else
        gtk_widget_show( tk->label );

    if( tb->flat_button )
        gtk_button_set_relief( (GtkButton*)tk->button, GTK_RELIEF_NONE );
    else
        gtk_button_set_relief( (GtkButton*)tk->button, GTK_RELIEF_NORMAL);
}

static void apply_config( Plugin* p )
{
    taskbar *tb = (taskbar *)p->priv;
    if( tb->tooltips )
        gtk_container_foreach( GTK_CONTAINER(tb->bar), (GtkCallback)gtk_widget_set_has_tooltip, (gpointer)TRUE );
    else
        gtk_container_foreach( GTK_CONTAINER(tb->bar), (GtkCallback)gtk_widget_set_has_tooltip, (gpointer)FALSE );

    gtk_bar_set_max_child_size(GTK_BAR(tb->bar), ((tb->icons_only) ? GTK_WIDGET(p->panel->box)->allocation.height - 2 : tb->task_width_max));

    gtk_box_set_spacing( GTK_BOX(tb->bar), tb->spacing );
    tb_net_client_list(NULL, tb);
    g_hash_table_foreach( tb->task_list,
                          (GHFunc)update_task_button,
                          (gpointer)tb );
}

static void taskbar_config( Plugin* p, GtkWindow* parent )
{
    GtkWidget* dlg;
    taskbar *tb = (taskbar *)p->priv;

    dlg =  create_generic_config_dlg(
                _(p->class->name),
                GTK_WIDGET(parent),
                (GSourceFunc) apply_config, (gpointer) p,
                _("Show tooltips"), &tb->tooltips, CONF_TYPE_BOOL,
                _("Icons only"), &tb->icons_only, CONF_TYPE_BOOL,
                _("Flat Buttons"), &tb->flat_button, CONF_TYPE_BOOL,
                _("Accept SkipPager"), &tb->accept_skip_pager, CONF_TYPE_BOOL,
                _("Show Iconified windows"), &tb->show_iconified, CONF_TYPE_BOOL,
                _("Show mapped windows"), &tb->show_mapped, CONF_TYPE_BOOL,
                _("Show windows from all desktops"), &tb->show_all_desks, CONF_TYPE_BOOL,
                _("Use mouse wheel"), &tb->use_mouse_wheel, CONF_TYPE_BOOL,
                _("Flash when there is any window requiring attention"), &tb->use_urgency_hint, CONF_TYPE_BOOL,
                _("Max width of task button"), &tb->task_width_max, CONF_TYPE_INT,
                _("Spacing"), &tb->spacing, CONF_TYPE_INT,
                NULL );
    gtk_window_present( GTK_WINDOW(dlg) );
}

static void save_config( Plugin* p, FILE* fp )
{
    taskbar *tb = (taskbar *)p->priv;
    lxpanel_put_bool( fp, "tooltips", tb->tooltips );
    lxpanel_put_bool( fp, "IconsOnly", tb->icons_only );
    lxpanel_put_bool( fp, "AcceptSkipPager", tb->accept_skip_pager );
    lxpanel_put_bool( fp, "ShowIconified", tb->show_iconified );
    lxpanel_put_bool( fp, "ShowMapped", tb->show_mapped );
    lxpanel_put_bool( fp, "ShowAllDesks", tb->show_all_desks );
    lxpanel_put_bool( fp, "UseMouseWheel", tb->use_mouse_wheel );
    lxpanel_put_bool( fp, "UseUrgencyHint", tb->use_urgency_hint );
    lxpanel_put_bool( fp, "FlatButton", tb->flat_button );
    lxpanel_put_int( fp, "MaxTaskWidth", tb->task_width_max );
    lxpanel_put_int( fp, "spacing", tb->spacing );
}

static void
update_label_orient( GtkWidget* child, gpointer user_data )
{
    /* FIXME: gtk+ has only limited support for this, sigh! */
    Plugin* p = (Plugin*)user_data;
    if( GTK_IS_LABEL(child) ) {
        gdouble angle;
        if( p->panel->edge == EDGE_LEFT ) {
            angle = 90.0;
            /* FIXME: ellipsize cannot be used in conjunction with angle.
                      This is the limit of gtk+, and turn off ellipsize do
                      cause problems here. How can this be solved? Sigh!
            */
            gtk_label_set_ellipsize( GTK_LABEL(child), PANGO_ELLIPSIZE_NONE );
        }
        else if( p->panel->edge == EDGE_RIGHT ) {
            angle = 270.0;
            gtk_label_set_ellipsize( GTK_LABEL(child), PANGO_ELLIPSIZE_NONE );
        }
        else {
            angle = 0.0;
            gtk_label_set_ellipsize( GTK_LABEL(child), PANGO_ELLIPSIZE_END );
        }
        gtk_label_set_angle( GTK_LABEL(child), angle );
        gtk_misc_set_alignment(GTK_MISC(child), 0.0, 0.5);
    }
}

static void orientation_changed( Plugin* p )
{
    taskbar *tb = (taskbar *)p->priv;
    GList *child, *children;

    children = gtk_container_get_children( GTK_CONTAINER (tb->bar) );
    for( child = children; child; child = child->next ) {
        GtkWidget *button = GTK_WIDGET(child->data);
        GtkBox *box = (GtkBox*)gtk_bin_get_child( GTK_BIN(button) );
        GtkBox *newbox = GTK_BOX(recreate_box( box, p->panel->orientation ));
        if( newbox != box ) {
            gtk_container_add( GTK_CONTAINER(button), GTK_WIDGET(newbox) );
        }
        gtk_container_foreach( GTK_CONTAINER(newbox),
                               update_label_orient, p );
    }
    g_list_free( children );

    gtk_widget_destroy( tb->menu );
    tb->menu = taskbar_make_menu( tb );

    gtk_bar_set_orientation( GTK_BAR(tb->bar), p->panel->orientation );
}

PluginClass taskbar_plugin_class = {
    fname: NULL,
    count: 0,

    type : "taskbar",
    name : N_("Task Bar (Window List)"),
    version: "1.0",
    description : N_("Taskbar shows all opened windows and allow to iconify them, shade or get focus"),

    constructor : taskbar_constructor,
    destructor  : taskbar_destructor,
    config : taskbar_config,
    save : save_config,
    orientation : orientation_changed
};

