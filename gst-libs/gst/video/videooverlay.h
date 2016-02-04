/* GStreamer Video Overlay Interface
 * Copyright (C) 2003 Ronald Bultje <rbultje@ronald.bitfreak.net>
 * Copyright (C) 2003 Julien Moutte <julien@moutte.net>
 * Copyright (C) 2011 Tim-Philipp Müller <tim@centricular.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef __GST_VIDEO_OVERLAY_H__
#define __GST_VIDEO_OVERLAY_H__

#include <gst/gst.h>

G_BEGIN_DECLS

#define GST_TYPE_VIDEO_OVERLAY \
    (gst_video_overlay_get_type ())
#define GST_VIDEO_OVERLAY(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST ((obj), GST_TYPE_VIDEO_OVERLAY, GstVideoOverlay))
#define GST_IS_VIDEO_OVERLAY(obj) \
    (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GST_TYPE_VIDEO_OVERLAY))
#define GST_VIDEO_OVERLAY_GET_INTERFACE(inst) \
    (G_TYPE_INSTANCE_GET_INTERFACE ((inst), GST_TYPE_VIDEO_OVERLAY, GstVideoOverlayInterface))

/**
 * GstVideoOverlay:
 *
 * Opaque #GstVideoOverlay interface structure
 */
typedef struct _GstVideoOverlay GstVideoOverlay;
typedef struct _GstVideoOverlayInterface GstVideoOverlayInterface;

/**
 * GstVideoOverlayInterface:
 * @iface: parent interface type.
 * @expose: virtual method to handle expose events
 * @handle_events: virtual method to handle events
 * @set_render_rectangle: virtual method to set the render rectangle
 * @set_window_handle: virtual method to configure the window handle
 *
 * #GstVideoOverlay interface
 */
struct _GstVideoOverlayInterface {
  GTypeInterface iface;

  /* virtual functions */
  void (*expose)               (GstVideoOverlay *overlay);

  void (*handle_events)        (GstVideoOverlay *overlay, gboolean handle_events);

  void (*set_render_rectangle) (GstVideoOverlay *overlay,
                                gint x, gint y,
                                gint width, gint height);

  void (*set_window_handle)    (GstVideoOverlay *overlay, guintptr handle);
#ifdef GST_EXT_WAYLAND_ENHANCEMENT
  void (*set_wl_window_wl_surface_id)   (GstVideoOverlay * overlay, guintptr wl_surface_id);
#endif

};

GType   gst_video_overlay_get_type (void);

/* virtual function wrappers */

gboolean        gst_video_overlay_set_render_rectangle  (GstVideoOverlay * overlay,
                                                         gint              x,
                                                         gint              y,
                                                         gint              width,
                                                         gint              height);

void            gst_video_overlay_expose                (GstVideoOverlay * overlay);

void            gst_video_overlay_handle_events         (GstVideoOverlay * overlay,
                                                         gboolean          handle_events);

void            gst_video_overlay_set_window_handle     (GstVideoOverlay * overlay,
                                                         guintptr handle);

/* public methods to dispatch bus messages */
void            gst_video_overlay_got_window_handle     (GstVideoOverlay * overlay,
                                                         guintptr          handle);

void            gst_video_overlay_prepare_window_handle (GstVideoOverlay * overlay);

gboolean        gst_is_video_overlay_prepare_window_handle_message (GstMessage * msg);
#ifdef GST_EXT_WAYLAND_ENHANCEMENT
void gst_video_overlay_set_wl_window_wl_surface_id (GstVideoOverlay * overlay,
    guintptr wl_surface_id);
#endif

G_END_DECLS

#endif /* __GST_VIDEO_OVERLAY_H__ */
