/* GStreamer
 * Copyright (C) <2005> Julien Moutte <julien@moutte.net>
 *               <2009>,<2010> Stefan Kost <stefan.kost@nokia.com>
 * Copyright (C) 2012, 2013 Samsung Electronics Co., Ltd.
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
 *
 * Modifications by Samsung Electronics Co., Ltd.
 * 1. Add display related properties
 * 2. Support samsung extension format to improve performance
 * 3. Support video texture overlay of OSP layer
 */

/**
 * SECTION:element-xvimagesink
 *
 * XvImageSink renders video frames to a drawable (XWindow) on a local display
 * using the XVideo extension. Rendering to a remote display is theoretically
 * possible but i doubt that the XVideo extension is actually available when
 * connecting to a remote display. This element can receive a Window ID from the
 * application through the #GstVideoOverlay interface and will then render
 * video frames in this drawable. If no Window ID was provided by the
 * application, the element will create its own internal window and render
 * into it.
 *
 * <refsect2>
 * <title>Scaling</title>
 * <para>
 * The XVideo extension, when it's available, handles hardware accelerated
 * scaling of video frames. This means that the element will just accept
 * incoming video frames no matter their geometry and will then put them to the
 * drawable scaling them on the fly. Using the #GstXvImageSink:force-aspect-ratio
 * property it is possible to enforce scaling with a constant aspect ratio,
 * which means drawing black borders around the video frame.
 * </para>
 * </refsect2>
 * <refsect2>
 * <title>Events</title>
 * <para>
 * XvImageSink creates a thread to handle events coming from the drawable. There
 * are several kind of events that can be grouped in 2 big categories: input
 * events and window state related events. Input events will be translated to
 * navigation events and pushed upstream for other elements to react on them.
 * This includes events such as pointer moves, key press/release, clicks etc...
 * Other events are used to handle the drawable appearance even when the data
 * is not flowing (GST_STATE_PAUSED). That means that even when the element is
 * paused, it will receive expose events from the drawable and draw the latest
 * frame with correct borders/aspect-ratio.
 * </para>
 * </refsect2>
 * <refsect2>
 * <title>Pixel aspect ratio</title>
 * <para>
 * When changing state to GST_STATE_READY, XvImageSink will open a connection to
 * the display specified in the #GstXvImageSink:display property or the
 * default display if nothing specified. Once this connection is open it will
 * inspect the display configuration including the physical display geometry and
 * then calculate the pixel aspect ratio. When receiving video frames with a
 * different pixel aspect ratio, XvImageSink will use hardware scaling to
 * display the video frames correctly on display's pixel aspect ratio.
 * Sometimes the calculated pixel aspect ratio can be wrong, it is
 * then possible to enforce a specific pixel aspect ratio using the
 * #GstXvImageSink:pixel-aspect-ratio property.
 * </para>
 * </refsect2>
 * <refsect2>
 * <title>Examples</title>
 * |[
 * gst-launch -v videotestsrc ! xvimagesink
 * ]| A pipeline to test hardware scaling.
 * When the test video signal appears you can resize the window and see that
 * video frames are scaled through hardware (no extra CPU cost).
 * |[
 * gst-launch -v videotestsrc ! xvimagesink force-aspect-ratio=true
 * ]| Same pipeline with #GstXvImageSink:force-aspect-ratio property set to true
 * You can observe the borders drawn around the scaled image respecting aspect
 * ratio.
 * |[
 * gst-launch -v videotestsrc ! navigationtest ! xvimagesink
 * ]| A pipeline to test navigation events.
 * While moving the mouse pointer over the test signal you will see a black box
 * following the mouse pointer. If you press the mouse button somewhere on the
 * video and release it somewhere else a green box will appear where you pressed
 * the button and a red one where you released it. (The navigationtest element
 * is part of gst-plugins-good.) You can observe here that even if the images
 * are scaled through hardware the pointer coordinates are converted back to the
 * original video frame geometry so that the box can be drawn to the correct
 * position. This also handles borders correctly, limiting coordinates to the
 * image area
 * |[
 * gst-launch -v videotestsrc ! video/x-raw, pixel-aspect-ratio=(fraction)4/3 ! xvimagesink
 * ]| This is faking a 4/3 pixel aspect ratio caps on video frames produced by
 * videotestsrc, in most cases the pixel aspect ratio of the display will be
 * 1/1. This means that XvImageSink will have to do the scaling to convert
 * incoming frames to a size that will match the display pixel aspect ratio
 * (from 320x240 to 320x180 in this case). Note that you might have to escape
 * some characters for your shell like '\(fraction\)'.
 * |[
 * gst-launch -v videotestsrc ! xvimagesink hue=100 saturation=-100 brightness=100
 * ]| Demonstrates how to use the colorbalance interface.
 * </refsect2>
 */

/* for developers: there are two useful tools : xvinfo and xvattr */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/* Our interfaces */
#include <gst/video/navigation.h>
#include <gst/video/videooverlay.h>
#include <gst/video/colorbalance.h>
/* Helper functions */
#include <gst/video/gstvideometa.h>

/* Object header */
#include "xvimagesink.h"
#include "xvimageallocator.h"

#ifdef GST_EXT_XV_ENHANCEMENT
/* Samsung extension headers */
/* For xv extension header for buffer transfer (output) */
#include "xv_types.h"

/* for performance checking */
#include <mm_ta.h>

typedef enum {
        BUF_SHARE_METHOD_PADDR = 0,
        BUF_SHARE_METHOD_FD,
        BUF_SHARE_METHOD_TIZEN_BUFFER
} buf_share_method_t;

#define _EVENT_THREAD_CHECK_INTERVAL    15000   /* us */
#endif /* GST_EXT_XV_ENHANCEMENT */


/* Debugging category */
#include <gst/gstinfo.h>

/* for XkbKeycodeToKeysym */
#include <X11/XKBlib.h>

GST_DEBUG_CATEGORY_EXTERN (gst_debug_xvimagesink);
GST_DEBUG_CATEGORY_EXTERN (GST_CAT_PERFORMANCE);
#define GST_CAT_DEFAULT gst_debug_xvimagesink

#ifdef GST_EXT_XV_ENHANCEMENT
#define GST_TYPE_XVIMAGESINK_DISPLAY_MODE (gst_xvimagesink_display_mode_get_type())

static GType
gst_xvimagesink_display_mode_get_type(void)
{
   static GType xvimagesink_display_mode_type = 0;
   static const GEnumValue display_mode_type[] = {
       { 0, "Default mode", "DEFAULT"},
       { 1, "Primary video ON and Secondary video FULL SCREEN mode", "PRI_VIDEO_ON_AND_SEC_VIDEO_FULL_SCREEN"},
       { 2, "Primary video OFF and Secondary video FULL SCREEN mode", "PRI_VIDEO_OFF_AND_SEC_VIDEO_FULL_SCREEN"},
       { 3, NULL, NULL},
   };

   if (!xvimagesink_display_mode_type) {
       xvimagesink_display_mode_type = g_enum_register_static("GstXVImageSinkDisplayModeType", display_mode_type);
   }

   return xvimagesink_display_mode_type;
}

enum {
    DEGREE_0,
    DEGREE_90,
    DEGREE_180,
    DEGREE_270,
    DEGREE_NUM,
};

#define GST_TYPE_XVIMAGESINK_ROTATE_ANGLE (gst_xvimagesink_rotate_angle_get_type())

static GType
gst_xvimagesink_rotate_angle_get_type(void)
{
   static GType xvimagesink_rotate_angle_type = 0;
   static const GEnumValue rotate_angle_type[] = {
       { 0, "No rotate", "DEGREE_0"},
       { 1, "Rotate 90 degree", "DEGREE_90"},
       { 2, "Rotate 180 degree", "DEGREE_180"},
       { 3, "Rotate 270 degree", "DEGREE_270"},
       { 4, NULL, NULL},
   };

   if (!xvimagesink_rotate_angle_type) {
       xvimagesink_rotate_angle_type = g_enum_register_static("GstXVImageSinkRotateAngleType", rotate_angle_type);
   }

   return xvimagesink_rotate_angle_type;
}

enum {
    DISP_GEO_METHOD_LETTER_BOX = 0,
    DISP_GEO_METHOD_ORIGIN_SIZE,
    DISP_GEO_METHOD_FULL_SCREEN,
    DISP_GEO_METHOD_CROPPED_FULL_SCREEN,
    DISP_GEO_METHOD_ORIGIN_SIZE_OR_LETTER_BOX,
    DISP_GEO_METHOD_CUSTOM_DST_ROI,
    DISP_GEO_METHOD_NUM,
};

enum {
    ROI_DISP_GEO_METHOD_FULL_SCREEN = 0,
    ROI_DISP_GEO_METHOD_LETTER_BOX,
    ROI_DISP_GEO_METHOD_NUM,
};

#define DEF_DISPLAY_GEOMETRY_METHOD         DISP_GEO_METHOD_LETTER_BOX
#define DEF_ROI_DISPLAY_GEOMETRY_METHOD     ROI_DISP_GEO_METHOD_FULL_SCREEN

enum {
    FLIP_NONE = 0,
    FLIP_HORIZONTAL,
    FLIP_VERTICAL,
    FLIP_BOTH,
    FLIP_NUM,
};
#define DEF_DISPLAY_FLIP            FLIP_NONE

#define GST_TYPE_XVIMAGESINK_DISPLAY_GEOMETRY_METHOD (gst_xvimagesink_display_geometry_method_get_type())
#define GST_TYPE_XVIMAGESINK_ROI_DISPLAY_GEOMETRY_METHOD (gst_xvimagesink_roi_display_geometry_method_get_type())
#define GST_TYPE_XVIMAGESINK_FLIP                    (gst_xvimagesink_flip_get_type())

static GType
gst_xvimagesink_display_geometry_method_get_type(void)
{
   static GType xvimagesink_display_geometry_method_type = 0;
   static const GEnumValue display_geometry_method_type[] = {
       { 0, "Letter box", "LETTER_BOX"},
       { 1, "Origin size", "ORIGIN_SIZE"},
       { 2, "Full-screen", "FULL_SCREEN"},
       { 3, "Cropped full-screen", "CROPPED_FULL_SCREEN"},
       { 4, "Origin size(if screen size is larger than video size(width/height)) or Letter box(if video size(width/height) is larger than screen size)", "ORIGIN_SIZE_OR_LETTER_BOX"},
       { 5, "Explicitly described destination ROI", "CUSTOM_DST_ROI"},
       { 6, NULL, NULL},
   };

   if (!xvimagesink_display_geometry_method_type) {
       xvimagesink_display_geometry_method_type = g_enum_register_static("GstXVImageSinkDisplayGeometryMethodType", display_geometry_method_type);
   }

   return xvimagesink_display_geometry_method_type;
}

static GType
gst_xvimagesink_roi_display_geometry_method_get_type(void)
{
       static GType xvimagesink_roi_display_geometry_method_type = 0;
       static const GEnumValue roi_display_geometry_method_type[] = {
               { 0, "ROI-Full-screen", "FULL_SCREEN"},
               { 1, "ROI-Letter box", "LETTER_BOX"},
               { 2, NULL, NULL},
       };

       if (!xvimagesink_roi_display_geometry_method_type) {
               xvimagesink_roi_display_geometry_method_type = g_enum_register_static("GstXVImageSinkROIDisplayGeometryMethodType", roi_display_geometry_method_type);
       }

       return xvimagesink_roi_display_geometry_method_type;
}

static GType
gst_xvimagesink_flip_get_type(void)
{
   static GType xvimagesink_flip_type = 0;
   static const GEnumValue flip_type[] = {
       { FLIP_NONE,       "Flip NONE", "FLIP_NONE"},
       { FLIP_HORIZONTAL, "Flip HORIZONTAL", "FLIP_HORIZONTAL"},
       { FLIP_VERTICAL,   "Flip VERTICAL", "FLIP_VERTICAL"},
       { FLIP_BOTH,       "Flip BOTH", "FLIP_BOTH"},
       { FLIP_NUM, NULL, NULL},
   };

   if (!xvimagesink_flip_type) {
       xvimagesink_flip_type = g_enum_register_static("GstXVImageSinkFlipType", flip_type);
   }

   return xvimagesink_flip_type;
}

#define g_marshal_value_peek_pointer(v)  (v)->data[0].v_pointer
void
gst_xvimagesink_BOOLEAN__POINTER (GClosure         *closure,
                                     GValue         *return_value G_GNUC_UNUSED,
                                     guint          n_param_values,
                                     const GValue   *param_values,
                                     gpointer       invocation_hint G_GNUC_UNUSED,
                                     gpointer       marshal_data)
{
  typedef gboolean (*GMarshalFunc_BOOLEAN__POINTER) (gpointer     data1,
                                                     gpointer     arg_1,
                                                     gpointer     data2);
  register GMarshalFunc_BOOLEAN__POINTER callback;
  register GCClosure *cc = (GCClosure*) closure;
  register gpointer data1, data2;

  gboolean v_return;

  g_return_if_fail (return_value != NULL);
  g_return_if_fail (n_param_values == 2);

  if (G_CCLOSURE_SWAP_DATA (closure)) {
    data1 = closure->data;
    data2 = g_value_peek_pointer (param_values + 0);
  } else {
    data1 = g_value_peek_pointer (param_values + 0);
    data2 = closure->data;
  }
  callback = (GMarshalFunc_BOOLEAN__POINTER) (marshal_data ? marshal_data : cc->callback);

  v_return = callback (data1,
                      g_marshal_value_peek_pointer (param_values + 1),
                      data2);

  g_value_set_boolean (return_value, v_return);
}

enum
{
    SIGNAL_FRAME_RENDER_ERROR,
    LAST_SIGNAL
};
#endif /* GST_EXT_XV_ENHANCEMENT */

typedef struct
{
  unsigned long flags;
  unsigned long functions;
  unsigned long decorations;
  long input_mode;
  unsigned long status;
}
MotifWmHints, MwmHints;

#define MWM_HINTS_DECORATIONS   (1L << 1)

static gboolean gst_xvimagesink_open (GstXvImageSink * xvimagesink);
static void gst_xvimagesink_close (GstXvImageSink * xvimagesink);
static void gst_xvimagesink_xwindow_update_geometry (GstXvImageSink *
    xvimagesink);
static void gst_xvimagesink_expose (GstVideoOverlay * overlay);

/* Default template - initiated with class struct to allow gst-register to work
   without X running */
static GstStaticPadTemplate gst_xvimagesink_sink_template_factory =
GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("video/x-raw, "
        "framerate = (fraction) [ 0, MAX ], "
        "width = (int) [ 1, MAX ], " "height = (int) [ 1, MAX ]")
    );

enum
{
  PROP_0,
  PROP_CONTRAST,
  PROP_BRIGHTNESS,
  PROP_HUE,
  PROP_SATURATION,
  PROP_DISPLAY,
  PROP_SYNCHRONOUS,
  PROP_PIXEL_ASPECT_RATIO,
  PROP_FORCE_ASPECT_RATIO,
  PROP_HANDLE_EVENTS,
  PROP_DEVICE,
  PROP_DEVICE_NAME,
  PROP_HANDLE_EXPOSE,
  PROP_DOUBLE_BUFFER,
  PROP_AUTOPAINT_COLORKEY,
  PROP_COLORKEY,
  PROP_DRAW_BORDERS,
  PROP_WINDOW_WIDTH,
  PROP_WINDOW_HEIGHT,
#ifdef GST_EXT_XV_ENHANCEMENT
  PROP_DISPLAY_MODE,
  PROP_ROTATE_ANGLE,
  PROP_FLIP,
  PROP_DISPLAY_GEOMETRY_METHOD,
  PROP_VISIBLE,
  PROP_ZOOM,
  PROP_ZOOM_POS_X,
  PROP_ZOOM_POS_Y,
  PROP_ORIENTATION,
  PROP_DST_ROI_MODE,
  PROP_DST_ROI_X,
  PROP_DST_ROI_Y,
  PROP_DST_ROI_W,
  PROP_DST_ROI_H,
#endif /* GST_EXT_XV_ENHANCEMENT */
};

/* ============================================================= */
/*                                                               */
/*                       Public Methods                          */
/*                                                               */
/* ============================================================= */

/* =========================================== */
/*                                             */
/*          Object typing & Creation           */
/*                                             */
/* =========================================== */
static void gst_xvimagesink_navigation_init (GstNavigationInterface * iface);
static void gst_xvimagesink_video_overlay_init (GstVideoOverlayInterface *
    iface);
static void gst_xvimagesink_colorbalance_init (GstColorBalanceInterface *
    iface);
#define gst_xvimagesink_parent_class parent_class
G_DEFINE_TYPE_WITH_CODE (GstXvImageSink, gst_xvimagesink, GST_TYPE_VIDEO_SINK,
    G_IMPLEMENT_INTERFACE (GST_TYPE_NAVIGATION,
        gst_xvimagesink_navigation_init);
    G_IMPLEMENT_INTERFACE (GST_TYPE_VIDEO_OVERLAY,
        gst_xvimagesink_video_overlay_init);
    G_IMPLEMENT_INTERFACE (GST_TYPE_COLOR_BALANCE,
        gst_xvimagesink_colorbalance_init));


/* ============================================================= */
/*                                                               */
/*                       Private Methods                         */
/*                                                               */
/* ============================================================= */


/* This function puts a GstXvImage on a GstXvImageSink's window. Returns FALSE
 * if no window was available  */
static gboolean
gst_xvimagesink_xvimage_put (GstXvImageSink * xvimagesink, GstBuffer * xvimage)
{
  GstXvImageMemory *mem;
  GstVideoCropMeta *crop;
  GstVideoRectangle result;
  gboolean draw_border = FALSE;
  GstVideoRectangle src, dst;
  GstVideoRectangle mem_crop;
  GstXWindow *xwindow;
#ifdef GST_EXT_XV_ENHANCEMENT
  static Atom atom_rotation = None;
  static Atom atom_hflip = None;
  static Atom atom_vflip = None;
  gboolean set_hflip = FALSE;
  gboolean set_vflip = FALSE;

  GstVideoRectangle src_origin = { 0, 0, 0, 0};
  GstVideoRectangle src_input  = { 0, 0, 0, 0};

  gint res_rotate_angle = 0;
  int rotate        = 0;
  int ret           = 0;

  gst_xvimagesink_xwindow_update_geometry( xvimagesink );
#endif /* GST_EXT_XV_ENHANCEMENT */

  /* We take the flow_lock. If expose is in there we don't want to run
     concurrently from the data flow thread */
  g_mutex_lock (&xvimagesink->flow_lock);

#ifdef GST_EXT_XV_ENHANCEMENT
  if (xvimagesink->xid_updated) {
    if (xvimage) {
      GST_WARNING_OBJECT (xvimagesink, "set xvimage to NULL, new xid was set right after creation of new xvimage");
      xvimage = NULL;
    }
    xvimagesink->xid_updated = FALSE;
  }
#endif /* GST_EXT_XV_ENHANCEMENT */

  if (G_UNLIKELY ((xwindow = xvimagesink->xwindow) == NULL)) {
    g_mutex_unlock (&xvimagesink->flow_lock);
    return FALSE;
  }

  /* Draw borders when displaying the first frame. After this
     draw borders only on expose event or after a size change. */
  if (!xvimagesink->cur_image || xvimagesink->redraw_border) {
    draw_border = xvimagesink->draw_borders;
    xvimagesink->redraw_border = FALSE;
  }

  /* Store a reference to the last image we put, lose the previous one */
  if (xvimage && xvimagesink->cur_image != xvimage) {
    if (xvimagesink->cur_image) {
      GST_LOG_OBJECT (xvimagesink, "unreffing %p", xvimagesink->cur_image);
      gst_buffer_unref (xvimagesink->cur_image);
    }
    GST_LOG_OBJECT (xvimagesink, "reffing %p as our current image", xvimage);
    xvimagesink->cur_image = gst_buffer_ref (xvimage);
  }

  /* Expose sends a NULL image, we take the latest frame */
  if (!xvimage) {
    if (xvimagesink->cur_image) {
      draw_border = TRUE;
      xvimage = xvimagesink->cur_image;
    } else {
#ifdef GST_EXT_XV_ENHANCEMENT
      GST_WARNING_OBJECT(xvimagesink, "cur_image is NULL. Skip xvimage_put.");
      /* no need to release gem handle */
#endif /* GST_EXT_XV_ENHANCEMENT */
      g_mutex_unlock (&xvimagesink->flow_lock);
      return TRUE;
    }
  }

  mem = (GstXvImageMemory *) gst_buffer_peek_memory (xvimage, 0);
#ifdef GST_EXT_XV_ENHANCEMENT
  if (!xvimagesink->visible) {
    GST_INFO("visible[%d]. Skip xvimage_put.",
             xvimagesink->visible);
    g_mutex_unlock(&xvimagesink->flow_lock);
    return TRUE;
  }

  res_rotate_angle = xvimagesink->rotate_angle;

  src.x = src.y = 0;
  src_origin.x = src_origin.y = src_input.x = src_input.y = 0;

  src_input.w = src_origin.w = xvimagesink->video_width;
  src_input.h = src_origin.h = xvimagesink->video_height;

  if (xvimagesink->rotate_angle == DEGREE_0 ||
      xvimagesink->rotate_angle == DEGREE_180) {
    src.w = src_origin.w;
    src.h = src_origin.h;
  } else {
    src.w = src_origin.h;
    src.h = src_origin.w;
  }

  dst.w = xwindow->render_rect.w;
  dst.h = xwindow->render_rect.h;

  switch (xvimagesink->display_geometry_method)
  {
    case DISP_GEO_METHOD_LETTER_BOX:
      gst_video_sink_center_rect (src, dst, &result, TRUE);
      result.x += xwindow->render_rect.x;
      result.y += xwindow->render_rect.y;
      break;

    case DISP_GEO_METHOD_ORIGIN_SIZE_OR_LETTER_BOX:
      GST_WARNING_OBJECT(xvimagesink, "not supported API, set ORIGIN_SIZE mode");
    case DISP_GEO_METHOD_ORIGIN_SIZE:
      gst_video_sink_center_rect (src, dst, &result, FALSE);
      gst_video_sink_center_rect (dst, src, &src_input, FALSE);

      if (xvimagesink->rotate_angle == DEGREE_90 ||
          xvimagesink->rotate_angle == DEGREE_270) {
        src_input.x = src_input.x ^ src_input.y;
        src_input.y = src_input.x ^ src_input.y;
        src_input.x = src_input.x ^ src_input.y;

        src_input.w = src_input.w ^ src_input.h;
        src_input.h = src_input.w ^ src_input.h;
        src_input.w = src_input.w ^ src_input.h;
      }
      break;

   case DISP_GEO_METHOD_FULL_SCREEN:
      result.x = result.y = 0;
      result.w = xwindow->width;
      result.h = xwindow->height;
      break;

   case DISP_GEO_METHOD_CROPPED_FULL_SCREEN:
      gst_video_sink_center_rect(dst, src, &src_input, TRUE);

      result.x = result.y = 0;
      result.w = dst.w;
      result.h = dst.h;

      if (xvimagesink->rotate_angle == DEGREE_90 ||
          xvimagesink->rotate_angle == DEGREE_270) {
        src_input.x = src_input.x ^ src_input.y;
        src_input.y = src_input.x ^ src_input.y;
        src_input.x = src_input.x ^ src_input.y;

        src_input.w = src_input.w ^ src_input.h;
        src_input.h = src_input.w ^ src_input.h;
        src_input.w = src_input.w ^ src_input.h;
      }
      break;

    case DISP_GEO_METHOD_CUSTOM_DST_ROI:
    {
      GstVideoRectangle dst_roi_cmpns;
      dst_roi_cmpns.w = xvimagesink->dst_roi.w;
      dst_roi_cmpns.h = xvimagesink->dst_roi.h;
      dst_roi_cmpns.x = xvimagesink->dst_roi.x;
      dst_roi_cmpns.y = xvimagesink->dst_roi.y;

      /* setting for DST ROI mode */
      switch (xvimagesink->dst_roi_mode) {
      case ROI_DISP_GEO_METHOD_FULL_SCREEN:
        break;
      case ROI_DISP_GEO_METHOD_LETTER_BOX:
       {
        GstVideoRectangle roi_result;
        if (xvimagesink->orientation == DEGREE_0 ||
            xvimagesink->orientation == DEGREE_180) {
          src.w = src_origin.w;
          src.h = src_origin.h;
        } else {
          src.w = src_origin.h;
          src.h = src_origin.w;
        }
        dst.w = xvimagesink->dst_roi.w;
        dst.h = xvimagesink->dst_roi.h;

        gst_video_sink_center_rect (src, dst, &roi_result, TRUE);
        dst_roi_cmpns.w = roi_result.w;
        dst_roi_cmpns.h = roi_result.h;
        dst_roi_cmpns.x = xvimagesink->dst_roi.x + roi_result.x;
        dst_roi_cmpns.y = xvimagesink->dst_roi.y + roi_result.y;
       }
        break;
      default:
        break;
      }

      /* calculating coordinates according to rotation angle for DST ROI */
      switch (xvimagesink->rotate_angle) {
      case DEGREE_90:
        result.w = dst_roi_cmpns.h;
        result.h = dst_roi_cmpns.w;

        result.x = dst_roi_cmpns.y;
        result.y = xvimagesink->xwindow->height - dst_roi_cmpns.x - dst_roi_cmpns.w;
        break;
      case DEGREE_180:
        result.w = dst_roi_cmpns.w;
        result.h = dst_roi_cmpns.h;

        result.x = xvimagesink->xwindow->width - result.w - dst_roi_cmpns.x;
        result.y = xvimagesink->xwindow->height - result.h - dst_roi_cmpns.y;
        break;
      case DEGREE_270:
        result.w = dst_roi_cmpns.h;
        result.h = dst_roi_cmpns.w;

        result.x = xvimagesink->xwindow->width - dst_roi_cmpns.y - dst_roi_cmpns.h;
        result.y = dst_roi_cmpns.x;
        break;
      default:
        result.x = dst_roi_cmpns.x;
        result.y = dst_roi_cmpns.y;
        result.w = dst_roi_cmpns.w;
        result.h = dst_roi_cmpns.h;
        break;
      }

      /* orientation setting for auto rotation in DST ROI */
      if (xvimagesink->orientation) {
        res_rotate_angle = (xvimagesink->rotate_angle - xvimagesink->orientation);
        if (res_rotate_angle < 0) {
          res_rotate_angle += DEGREE_NUM;
        }
        GST_LOG_OBJECT(xvimagesink, "changing rotation value internally by ROI orientation[%d] : rotate[%d->%d]",
                     xvimagesink->orientation, xvimagesink->rotate_angle, res_rotate_angle);
      }

      GST_LOG_OBJECT(xvimagesink, "rotate[%d], dst ROI: orientation[%d], mode[%d], input[%d,%d,%dx%d]->result[%d,%d,%dx%d]",
                     xvimagesink->rotate_angle, xvimagesink->orientation, xvimagesink->dst_roi_mode,
                     xvimagesink->dst_roi.x, xvimagesink->dst_roi.y, xvimagesink->dst_roi.w, xvimagesink->dst_roi.h,
                     result.x, result.y, result.w, result.h);
      break;
    }
    default:
      break;
  }

  if (xvimagesink->zoom > 1.0 && xvimagesink->zoom <= 9.0) {
    GST_LOG_OBJECT(xvimagesink, "before zoom[%lf], src_input[x:%d,y:%d,w:%d,h:%d]",
                   xvimagesink->zoom, src_input.x, src_input.y, src_input.w, src_input.h);
    gint default_offset_x = 0;
    gint default_offset_y = 0;
    gfloat w = (gfloat)src_input.w;
    gfloat h = (gfloat)src_input.h;
    if (xvimagesink->orientation == DEGREE_0 ||
        xvimagesink->orientation == DEGREE_180) {
      default_offset_x = ((gint)(w - (w/xvimagesink->zoom)))>>1;
      default_offset_y = ((gint)(h - (h/xvimagesink->zoom)))>>1;
    } else {
      default_offset_y = ((gint)(w - (w/xvimagesink->zoom)))>>1;
      default_offset_x = ((gint)(h - (h/xvimagesink->zoom)))>>1;
    }
    GST_LOG_OBJECT(xvimagesink, "default offset x[%d] y[%d], orientation[%d]", default_offset_x, default_offset_y, xvimagesink->orientation);
    if (xvimagesink->zoom_pos_x == -1) {
      src_input.x += default_offset_x;
    } else {
      if (xvimagesink->orientation == DEGREE_0 ||
          xvimagesink->orientation == DEGREE_180) {
        if ((w/xvimagesink->zoom) > w - xvimagesink->zoom_pos_x) {
          xvimagesink->zoom_pos_x = w - (w/xvimagesink->zoom);
        }
        src_input.x += xvimagesink->zoom_pos_x;
      } else {
        if ((h/xvimagesink->zoom) > h - xvimagesink->zoom_pos_x) {
          xvimagesink->zoom_pos_x = h - (h/xvimagesink->zoom);
        }
        src_input.y += (h - h/xvimagesink->zoom) - xvimagesink->zoom_pos_x;
      }
    }
    if (xvimagesink->zoom_pos_y == -1) {
      src_input.y += default_offset_y;
    } else {
      if (xvimagesink->orientation == DEGREE_0 ||
          xvimagesink->orientation == DEGREE_180) {
        if ((h/xvimagesink->zoom) > h - xvimagesink->zoom_pos_y) {
          xvimagesink->zoom_pos_y = h - (h/xvimagesink->zoom);
        }
        src_input.y += xvimagesink->zoom_pos_y;
      } else {
        if ((w/xvimagesink->zoom) > w - xvimagesink->zoom_pos_y) {
          xvimagesink->zoom_pos_y = w - (w/xvimagesink->zoom);
        }
        src_input.x += (xvimagesink->zoom_pos_y);
      }
    }
    src_input.w = (gint)(w/xvimagesink->zoom);
    src_input.h = (gint)(h/xvimagesink->zoom);
    GST_LOG_OBJECT(xvimagesink, "after zoom[%lf], src_input[x:%d,y:%d,w:%d,h%d], zoom_pos[x:%d,y:%d]",
                   xvimagesink->zoom, src_input.x, src_input.y, src_input.w, src_input.h, xvimagesink->zoom_pos_x, xvimagesink->zoom_pos_y);
  }

  switch( res_rotate_angle )
  {
    /* There's slightly weired code (CCW? CW?) */
    case DEGREE_0:
      break;
    case DEGREE_90:
      rotate = 270;
      break;
    case DEGREE_180:
      rotate = 180;
      break;
    case DEGREE_270:
      rotate = 90;
      break;
    default:
      GST_WARNING_OBJECT( xvimagesink, "Unsupported rotation [%d]... set DEGREE 0.",
          res_rotate_angle );
      break;
  }

  /* Trim as proper size */
  if (src_input.w % 2 == 1) {
      src_input.w += 1;
  }
  if (src_input.h % 2 == 1) {
      src_input.h += 1;
  }

  GST_LOG_OBJECT( xvimagesink, "window[%dx%d],method[%d],rotate[%d],zoom[%d],dp_mode[%d],src[%dx%d],dst[%d,%d,%dx%d],input[%d,%d,%dx%d],result[%d,%d,%dx%d]",
    xwindow->width, xwindow->height,
    xvimagesink->display_geometry_method, rotate, xvimagesink->zoom, xvimagesink->config.display_mode,
    src_origin.w, src_origin.h,
    dst.x, dst.y, dst.w, dst.h,
    src_input.x, src_input.y, src_input.w, src_input.h,
    result.x, result.y, result.w, result.h );

  memcpy (&src, &src_input, sizeof (src_input));

#ifdef HAVE_XSHM
  /* set display rotation */
  if (atom_rotation == None) {
    atom_rotation = XInternAtom(xvimagesink->context->disp,
                                "_USER_WM_PORT_ATTRIBUTE_ROTATION", False);
  }

  GST_LOG("set HFLIP %d, VFLIP %d", set_hflip, set_vflip);

  ret = XvSetPortAttribute(xvimagesink->context->disp, xvimagesink->context->xv_port_id, atom_rotation, rotate);
  if (ret != Success) {
    GST_ERROR_OBJECT( mem, "XvSetPortAttribute failed[%d]. disp[%x],xv_port_id[%d],atom[%x],rotate[%d]",
      ret, xvimagesink->context->disp, xvimagesink->context->xv_port_id, atom_rotation, rotate );
    return ret;
  }

  /* set display flip */
  if (atom_hflip == None) {
    atom_hflip = XInternAtom(xvimagesink->context->disp,
                             "_USER_WM_PORT_ATTRIBUTE_HFLIP", False);
  }
  if (atom_vflip == None) {
    atom_vflip = XInternAtom(xvimagesink->context->disp,
                             "_USER_WM_PORT_ATTRIBUTE_VFLIP", False);
  }

  switch (xvimagesink->flip) {
  case FLIP_HORIZONTAL:
    set_hflip = TRUE;
    set_vflip = FALSE;
    break;
  case FLIP_VERTICAL:
    set_hflip = FALSE;
    set_vflip = TRUE;
    break;
  case FLIP_BOTH:
    set_hflip = TRUE;
    set_vflip = TRUE;
    break;
  case FLIP_NONE:
  default:
    set_hflip = FALSE;
    set_vflip = FALSE;
    break;
  }

  ret = XvSetPortAttribute(xvimagesink->context->disp, xvimagesink->context->xv_port_id, atom_hflip, set_hflip);
  if (ret != Success) {
    GST_WARNING("set HFLIP failed[%d]. disp[%x],xv_port_id[%d],atom[%x],hflip[%d]",
                ret, xvimagesink->context->disp, xvimagesink->context->xv_port_id, atom_hflip, set_hflip);
  }
  ret = XvSetPortAttribute(xvimagesink->context->disp, xvimagesink->context->xv_port_id, atom_vflip, set_vflip);
  if (ret != Success) {
    GST_WARNING("set VFLIP failed[%d]. disp[%x],xv_port_id[%d],atom[%x],vflip[%d]",
                ret, xvimagesink->context->disp, xvimagesink->context->xv_port_id, atom_vflip, set_vflip);
  }
#endif /* HAVE_XSHM */
#else /* GST_EXT_XV_ENHANCEMENT */
  gst_xvimage_memory_get_crop (mem, &mem_crop);

  crop = gst_buffer_get_video_crop_meta (xvimage);

  if (crop) {
    src.x = crop->x + mem_crop.x;
    src.y = crop->y + mem_crop.y;
    src.w = crop->width;
    src.h = crop->height;
    GST_LOG_OBJECT (xvimagesink,
        "crop %dx%d-%dx%d", crop->x, crop->y, crop->width, crop->height);
  } else {
    src = mem_crop;
  }

  if (xvimagesink->keep_aspect) {
    GstVideoRectangle s;

    /* We take the size of the source material as it was negotiated and
     * corrected for DAR. This size can be different from the cropped size in
     * which case the image will be scaled to fit the negotiated size. */
    s.w = GST_VIDEO_SINK_WIDTH (xvimagesink);
    s.h = GST_VIDEO_SINK_HEIGHT (xvimagesink);
    dst.w = xwindow->render_rect.w;
    dst.h = xwindow->render_rect.h;

    gst_video_sink_center_rect (s, dst, &result, TRUE);
    result.x += xwindow->render_rect.x;
    result.y += xwindow->render_rect.y;
  } else {
    memcpy (&result, &xwindow->render_rect, sizeof (GstVideoRectangle));
  }
#endif /* GST_EXT_XV_ENHANCEMENT */

  gst_xvimage_memory_render (mem, &src, xwindow, &result, draw_border);

  g_mutex_unlock (&xvimagesink->flow_lock);

  return TRUE;
}

static void
gst_xvimagesink_xwindow_set_title (GstXvImageSink * xvimagesink,
    GstXWindow * xwindow, const gchar * media_title)
{
  if (media_title) {
    g_free (xvimagesink->media_title);
    xvimagesink->media_title = g_strdup (media_title);
  }
  if (xwindow) {
    /* we have a window */
    const gchar *app_name;
    const gchar *title = NULL;
    gchar *title_mem = NULL;

    /* set application name as a title */
    app_name = g_get_application_name ();

    if (app_name && xvimagesink->media_title) {
      title = title_mem = g_strconcat (xvimagesink->media_title, " : ",
          app_name, NULL);
    } else if (app_name) {
      title = app_name;
    } else if (xvimagesink->media_title) {
      title = xvimagesink->media_title;
    }

    gst_xwindow_set_title (xwindow, title);
    g_free (title_mem);
  }
}

/* This function handles a GstXWindow creation
 * The width and height are the actual pixel size on the display */
static GstXWindow *
gst_xvimagesink_xwindow_new (GstXvImageSink * xvimagesink,
    gint width, gint height)
{
  GstXWindow *xwindow = NULL;
  GstXvContext *context;

  g_return_val_if_fail (GST_IS_XVIMAGESINK (xvimagesink), NULL);

  context = xvimagesink->context;

#ifdef GST_EXT_XV_ENHANCEMENT
  /* 0 or 180 */
  if (xvimagesink->rotate_angle == 0 || xvimagesink->rotate_angle == 2) {
      xwindow = gst_xvcontext_create_xwindow (context, height, width);
  /* 90 or 270 */
  } else {
      xwindow = gst_xvcontext_create_xwindow (context, width, height);
  }
#else
  xwindow = gst_xvcontext_create_xwindow (context, width, height);
#endif /* GST_EXT_XV_ENHANCEMENT */

  /* set application name as a title */
  gst_xvimagesink_xwindow_set_title (xvimagesink, xwindow, NULL);

  gst_xwindow_set_event_handling (xwindow, xvimagesink->handle_events);

  gst_video_overlay_got_window_handle (GST_VIDEO_OVERLAY (xvimagesink),
      xwindow->win);

  return xwindow;
}

static void
gst_xvimagesink_xwindow_update_geometry (GstXvImageSink * xvimagesink)
{
  g_return_if_fail (GST_IS_XVIMAGESINK (xvimagesink));

  /* Update the window geometry */
  g_mutex_lock (&xvimagesink->flow_lock);
  if (G_LIKELY (xvimagesink->xwindow))
    gst_xwindow_update_geometry (xvimagesink->xwindow);
  g_mutex_unlock (&xvimagesink->flow_lock);
}

/* This function commits our internal colorbalance settings to our grabbed Xv
   port. If the context is not initialized yet it simply returns */
static void
gst_xvimagesink_update_colorbalance (GstXvImageSink * xvimagesink)
{
  GstXvContext *context;

  g_return_if_fail (GST_IS_XVIMAGESINK (xvimagesink));

  /* If we haven't initialized the X context we can't update anything */
  if ((context = xvimagesink->context) == NULL)
    return;

  gst_xvcontext_update_colorbalance (context, &xvimagesink->config);
}

/* This function handles XEvents that might be in the queue. It generates
   GstEvent that will be sent upstream in the pipeline to handle interactivity
   and navigation. It will also listen for configure events on the window to
   trigger caps renegotiation so on the fly software scaling can work. */
static void
gst_xvimagesink_handle_xevents (GstXvImageSink * xvimagesink)
{
  XEvent e;
  guint pointer_x = 0, pointer_y = 0;
  gboolean pointer_moved = FALSE;
  gboolean exposed = FALSE, configured = FALSE;

  g_return_if_fail (GST_IS_XVIMAGESINK (xvimagesink));

#ifdef GST_EXT_XV_ENHANCEMENT
  GST_LOG("check x event");
#endif /* GST_EXT_XV_ENHANCEMENT */

  /* Handle Interaction, produces navigation events */

  /* We get all pointer motion events, only the last position is
     interesting. */
  g_mutex_lock (&xvimagesink->flow_lock);
  g_mutex_lock (&xvimagesink->context->lock);
  while (XCheckWindowEvent (xvimagesink->context->disp,
          xvimagesink->xwindow->win, PointerMotionMask, &e)) {
    g_mutex_unlock (&xvimagesink->context->lock);
    g_mutex_unlock (&xvimagesink->flow_lock);

    switch (e.type) {
      case MotionNotify:
        pointer_x = e.xmotion.x;
        pointer_y = e.xmotion.y;
        pointer_moved = TRUE;
        break;
      default:
        break;
    }
    g_mutex_lock (&xvimagesink->flow_lock);
    g_mutex_lock (&xvimagesink->context->lock);
  }

  if (pointer_moved) {
    g_mutex_unlock (&xvimagesink->context->lock);
    g_mutex_unlock (&xvimagesink->flow_lock);

    GST_DEBUG ("xvimagesink pointer moved over window at %d,%d",
        pointer_x, pointer_y);
    gst_navigation_send_mouse_event (GST_NAVIGATION (xvimagesink),
        "mouse-move", 0, e.xbutton.x, e.xbutton.y);

    g_mutex_lock (&xvimagesink->flow_lock);
    g_mutex_lock (&xvimagesink->context->lock);
  }

  /* We get all events on our window to throw them upstream */
  while (XCheckWindowEvent (xvimagesink->context->disp,
          xvimagesink->xwindow->win,
          KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask,
          &e)) {
    KeySym keysym;
    const char *key_str = NULL;

    /* We lock only for the X function call */
    g_mutex_unlock (&xvimagesink->context->lock);
    g_mutex_unlock (&xvimagesink->flow_lock);

    switch (e.type) {
      case ButtonPress:
        /* Mouse button pressed over our window. We send upstream
           events for interactivity/navigation */
        GST_DEBUG ("xvimagesink button %d pressed over window at %d,%d",
            e.xbutton.button, e.xbutton.x, e.xbutton.y);
        gst_navigation_send_mouse_event (GST_NAVIGATION (xvimagesink),
            "mouse-button-press", e.xbutton.button, e.xbutton.x, e.xbutton.y);
        break;
      case ButtonRelease:
        /* Mouse button released over our window. We send upstream
           events for interactivity/navigation */
        GST_DEBUG ("xvimagesink button %d released over window at %d,%d",
            e.xbutton.button, e.xbutton.x, e.xbutton.y);
        gst_navigation_send_mouse_event (GST_NAVIGATION (xvimagesink),
            "mouse-button-release", e.xbutton.button, e.xbutton.x, e.xbutton.y);
        break;
      case KeyPress:
      case KeyRelease:
        /* Key pressed/released over our window. We send upstream
           events for interactivity/navigation */
        g_mutex_lock (&xvimagesink->context->lock);
        keysym = XkbKeycodeToKeysym (xvimagesink->context->disp,
            e.xkey.keycode, 0, 0);
        if (keysym != NoSymbol) {
          key_str = XKeysymToString (keysym);
        } else {
          key_str = "unknown";
        }
        g_mutex_unlock (&xvimagesink->context->lock);
        GST_DEBUG_OBJECT (xvimagesink,
            "key %d pressed over window at %d,%d (%s)",
            e.xkey.keycode, e.xkey.x, e.xkey.y, key_str);
        gst_navigation_send_key_event (GST_NAVIGATION (xvimagesink),
            e.type == KeyPress ? "key-press" : "key-release", key_str);
        break;
      default:
        GST_DEBUG_OBJECT (xvimagesink, "xvimagesink unhandled X event (%d)",
            e.type);
    }
    g_mutex_lock (&xvimagesink->flow_lock);
    g_mutex_lock (&xvimagesink->context->lock);
  }

  /* Handle Expose */
  while (XCheckWindowEvent (xvimagesink->context->disp,
          xvimagesink->xwindow->win, ExposureMask | StructureNotifyMask, &e)) {
    switch (e.type) {
      case Expose:
        exposed = TRUE;
        break;
      case ConfigureNotify:
        g_mutex_unlock (&xvimagesink->context->lock);
        g_mutex_unlock (&xvimagesink->flow_lock);

#ifdef GST_EXT_XV_ENHANCEMENT
        GST_WARNING("Call gst_xvimagesink_xwindow_update_geometry!");
#endif /* GST_EXT_XV_ENHANCEMENT */
        gst_xvimagesink_xwindow_update_geometry (xvimagesink);
#ifdef GST_EXT_XV_ENHANCEMENT
        GST_WARNING("Return gst_xvimagesink_xwindow_update_geometry!");
#endif /* GST_EXT_XV_ENHANCEMENT */
        g_mutex_lock (&xvimagesink->flow_lock);
        g_mutex_lock (&xvimagesink->context->lock);
        configured = TRUE;
        break;
      default:
        break;
    }
  }

  if (xvimagesink->handle_expose && (exposed || configured)) {
    g_mutex_unlock (&xvimagesink->context->lock);
    g_mutex_unlock (&xvimagesink->flow_lock);

    gst_xvimagesink_expose (GST_VIDEO_OVERLAY (xvimagesink));

    g_mutex_lock (&xvimagesink->flow_lock);
    g_mutex_lock (&xvimagesink->context->lock);
  }

  /* Handle Display events */
  while (XPending (xvimagesink->context->disp)) {
    XNextEvent (xvimagesink->context->disp, &e);

    switch (e.type) {
      case ClientMessage:{
#ifdef GST_EXT_XV_ENHANCEMENT
        XClientMessageEvent *cme = (XClientMessageEvent *)&e;
        Atom buffer_atom = XInternAtom(xvimagesink->context->disp, "XV_RETURN_BUFFER", False);
#endif /* GST_EXT_XV_ENHANCEMENT */
        Atom wm_delete;

#ifdef GST_EXT_XV_ENHANCEMENT
        GST_LOG_OBJECT(xvimagesink, "message type %d, buffer atom %d", cme->message_type, buffer_atom);
        if (cme->message_type == buffer_atom) {
          unsigned int gem_name[XV_BUF_PLANE_NUM] = { 0, };

          GST_DEBUG("data.l[0] -> %d, data.l[1] -> %d",
                    cme->data.l[0], cme->data.l[1]);

          gem_name[0] = cme->data.l[0];
          gem_name[1] = cme->data.l[1];

          gst_xvcontext_remove_displaying_buffer(xvimagesink->context, gem_name);
          break;
        }
#endif /* GST_EXT_XV_ENHANCEMENT */

        wm_delete = XInternAtom (xvimagesink->context->disp,
            "WM_DELETE_WINDOW", True);
        if (wm_delete != None && wm_delete == (Atom) e.xclient.data.l[0]) {
          /* Handle window deletion by posting an error on the bus */
          GST_ELEMENT_ERROR (xvimagesink, RESOURCE, NOT_FOUND,
              ("Output window was closed"), (NULL));

          g_mutex_unlock (&xvimagesink->context->lock);
          gst_xwindow_destroy (xvimagesink->xwindow);
          xvimagesink->xwindow = NULL;
          g_mutex_lock (&xvimagesink->context->lock);
        }
        break;
      }
      default:
        break;
    }
  }

  g_mutex_unlock (&xvimagesink->context->lock);
  g_mutex_unlock (&xvimagesink->flow_lock);
}

static gpointer
gst_xvimagesink_event_thread (GstXvImageSink * xvimagesink)
{
  g_return_val_if_fail (GST_IS_XVIMAGESINK (xvimagesink), NULL);

  GST_OBJECT_LOCK (xvimagesink);
  while (xvimagesink->running) {
    GST_OBJECT_UNLOCK (xvimagesink);

    if (xvimagesink->xwindow) {
      gst_xvimagesink_handle_xevents (xvimagesink);
    }

#ifdef GST_EXT_XV_ENHANCEMENT
    g_usleep (_EVENT_THREAD_CHECK_INTERVAL);
#else /* GST_EXT_XV_ENHANCEMENT */
    /* FIXME: do we want to align this with the framerate or anything else? */
    g_usleep (G_USEC_PER_SEC / 20);
#endif /* GST_EXT_XV_ENHANCEMENT */

    GST_OBJECT_LOCK (xvimagesink);
  }
  GST_OBJECT_UNLOCK (xvimagesink);

  return NULL;
}

static void
gst_xvimagesink_manage_event_thread (GstXvImageSink * xvimagesink)
{
  GThread *thread = NULL;

  /* don't start the thread too early */
  if (xvimagesink->context == NULL) {
    return;
  }

  GST_OBJECT_LOCK (xvimagesink);
  if (xvimagesink->handle_expose || xvimagesink->handle_events) {
    if (!xvimagesink->event_thread) {
      /* Setup our event listening thread */
      GST_DEBUG_OBJECT (xvimagesink, "run xevent thread, expose %d, events %d",
          xvimagesink->handle_expose, xvimagesink->handle_events);
      xvimagesink->running = TRUE;
      xvimagesink->event_thread = g_thread_try_new ("xvimagesink-events",
          (GThreadFunc) gst_xvimagesink_event_thread, xvimagesink, NULL);
    }
  } else {
    if (xvimagesink->event_thread) {
      GST_DEBUG_OBJECT (xvimagesink, "stop xevent thread, expose %d, events %d",
          xvimagesink->handle_expose, xvimagesink->handle_events);
      xvimagesink->running = FALSE;
      /* grab thread and mark it as NULL */
      thread = xvimagesink->event_thread;
      xvimagesink->event_thread = NULL;
    }
  }
  GST_OBJECT_UNLOCK (xvimagesink);

  /* Wait for our event thread to finish */
  if (thread)
    g_thread_join (thread);

}

/* Element stuff */

static GstCaps *
gst_xvimagesink_getcaps (GstBaseSink * bsink, GstCaps * filter)
{
  GstXvImageSink *xvimagesink;
  GstCaps *caps;

  xvimagesink = GST_XVIMAGESINK (bsink);

  if (xvimagesink->context) {
    if (filter)
      return gst_caps_intersect_full (filter, xvimagesink->context->caps,
          GST_CAPS_INTERSECT_FIRST);
    else
      return gst_caps_ref (xvimagesink->context->caps);
  }

  caps = gst_pad_get_pad_template_caps (GST_VIDEO_SINK_PAD (xvimagesink));
  if (filter) {
    GstCaps *intersection;

    intersection =
        gst_caps_intersect_full (filter, caps, GST_CAPS_INTERSECT_FIRST);
    gst_caps_unref (caps);
    caps = intersection;
  }
  return caps;
}

static gboolean
gst_xvimagesink_setcaps (GstBaseSink * bsink, GstCaps * caps)
{
  GstXvImageSink *xvimagesink;
  GstXvContext *context;
  GstStructure *structure;
  GstBufferPool *newpool, *oldpool;
  GstVideoInfo info;
  guint32 im_format = 0;
  gint video_par_n, video_par_d;        /* video's PAR */
  gint display_par_n, display_par_d;    /* display's PAR */
  guint num, den;
  gint size;

  xvimagesink = GST_XVIMAGESINK (bsink);
  context = xvimagesink->context;

  GST_DEBUG_OBJECT (xvimagesink,
      "In setcaps. Possible caps %" GST_PTR_FORMAT ", setting caps %"
      GST_PTR_FORMAT, context->caps, caps);

  if (!gst_caps_can_intersect (context->caps, caps))
    goto incompatible_caps;

  if (!gst_video_info_from_caps (&info, caps))
    goto invalid_format;

  xvimagesink->fps_n = info.fps_n;
  xvimagesink->fps_d = info.fps_d;

  xvimagesink->video_width = info.width;
  xvimagesink->video_height = info.height;

  im_format = gst_xvcontext_get_format_from_info (context, &info);
  if (im_format == -1)
    goto invalid_format;

  gst_xvcontext_set_colorimetry (context, &info.colorimetry);

  size = info.size;

  /* get aspect ratio from caps if it's present, and
   * convert video width and height to a display width and height
   * using wd / hd = wv / hv * PARv / PARd */

  /* get video's PAR */
  video_par_n = info.par_n;
  video_par_d = info.par_d;

  /* get display's PAR */
  if (xvimagesink->par) {
    display_par_n = gst_value_get_fraction_numerator (xvimagesink->par);
    display_par_d = gst_value_get_fraction_denominator (xvimagesink->par);
  } else {
    display_par_n = 1;
    display_par_d = 1;
  }

  if (!gst_video_calculate_display_ratio (&num, &den, info.width,
          info.height, video_par_n, video_par_d, display_par_n, display_par_d))
    goto no_disp_ratio;

  GST_DEBUG_OBJECT (xvimagesink,
      "video width/height: %dx%d, calculated display ratio: %d/%d",
      info.width, info.height, num, den);

  /* now find a width x height that respects this display ratio.
   * prefer those that have one of w/h the same as the incoming video
   * using wd / hd = num / den */

  /* start with same height, because of interlaced video */
  /* check hd / den is an integer scale factor, and scale wd with the PAR */
  if (info.height % den == 0) {
    GST_DEBUG_OBJECT (xvimagesink, "keeping video height");
    GST_VIDEO_SINK_WIDTH (xvimagesink) = (guint)
        gst_util_uint64_scale_int (info.height, num, den);
    GST_VIDEO_SINK_HEIGHT (xvimagesink) = info.height;
  } else if (info.width % num == 0) {
    GST_DEBUG_OBJECT (xvimagesink, "keeping video width");
    GST_VIDEO_SINK_WIDTH (xvimagesink) = info.width;
    GST_VIDEO_SINK_HEIGHT (xvimagesink) = (guint)
        gst_util_uint64_scale_int (info.width, den, num);
  } else {
    GST_DEBUG_OBJECT (xvimagesink, "approximating while keeping video height");
    GST_VIDEO_SINK_WIDTH (xvimagesink) = (guint)
        gst_util_uint64_scale_int (info.height, num, den);
    GST_VIDEO_SINK_HEIGHT (xvimagesink) = info.height;
  }
  GST_DEBUG_OBJECT (xvimagesink, "scaling to %dx%d",
      GST_VIDEO_SINK_WIDTH (xvimagesink), GST_VIDEO_SINK_HEIGHT (xvimagesink));

  /* Notify application to set xwindow id now */
  g_mutex_lock (&xvimagesink->flow_lock);
  if (!xvimagesink->xwindow) {
    g_mutex_unlock (&xvimagesink->flow_lock);
    gst_video_overlay_prepare_window_handle (GST_VIDEO_OVERLAY (xvimagesink));
  } else {
    g_mutex_unlock (&xvimagesink->flow_lock);
  }

  /* Creating our window and our image with the display size in pixels */
  if (GST_VIDEO_SINK_WIDTH (xvimagesink) <= 0 ||
      GST_VIDEO_SINK_HEIGHT (xvimagesink) <= 0)
    goto no_display_size;

  g_mutex_lock (&xvimagesink->flow_lock);
  if (!xvimagesink->xwindow) {
    xvimagesink->xwindow = gst_xvimagesink_xwindow_new (xvimagesink,
        GST_VIDEO_SINK_WIDTH (xvimagesink),
        GST_VIDEO_SINK_HEIGHT (xvimagesink));
  }

  xvimagesink->info = info;

  /* After a resize, we want to redraw the borders in case the new frame size
   * doesn't cover the same area */
  xvimagesink->redraw_border = TRUE;

  /* create a new pool for the new configuration */
  newpool = gst_xvimage_buffer_pool_new (xvimagesink->allocator);

  structure = gst_buffer_pool_get_config (newpool);
  gst_buffer_pool_config_set_params (structure, caps, size, 2, 0);
  if (!gst_buffer_pool_set_config (newpool, structure))
    goto config_failed;

  oldpool = xvimagesink->pool;
  /* we don't activate the pool yet, this will be done by downstream after it
   * has configured the pool. If downstream does not want our pool we will
   * activate it when we render into it */
  xvimagesink->pool = newpool;
  g_mutex_unlock (&xvimagesink->flow_lock);

  /* unref the old sink */
  if (oldpool) {
    /* we don't deactivate, some elements might still be using it, it will
     * be deactivated when the last ref is gone */
    gst_object_unref (oldpool);
  }

  return TRUE;

  /* ERRORS */
incompatible_caps:
  {
    GST_ERROR_OBJECT (xvimagesink, "caps incompatible");
    return FALSE;
  }
invalid_format:
  {
    GST_DEBUG_OBJECT (xvimagesink,
        "Could not locate image format from caps %" GST_PTR_FORMAT, caps);
    return FALSE;
  }
no_disp_ratio:
  {
    GST_ELEMENT_ERROR (xvimagesink, CORE, NEGOTIATION, (NULL),
        ("Error calculating the output display ratio of the video."));
    return FALSE;
  }
no_display_size:
  {
    GST_ELEMENT_ERROR (xvimagesink, CORE, NEGOTIATION, (NULL),
        ("Error calculating the output display ratio of the video."));
    return FALSE;
  }
config_failed:
  {
    GST_ERROR_OBJECT (xvimagesink, "failed to set config.");
    g_mutex_unlock (&xvimagesink->flow_lock);
    return FALSE;
  }
}

static GstStateChangeReturn
gst_xvimagesink_change_state (GstElement * element, GstStateChange transition)
{
  GstStateChangeReturn ret = GST_STATE_CHANGE_SUCCESS;
  GstXvImageSink *xvimagesink;

  xvimagesink = GST_XVIMAGESINK (element);

  switch (transition) {
    case GST_STATE_CHANGE_NULL_TO_READY:
#ifdef GST_EXT_XV_ENHANCEMENT
      GST_WARNING("NULL_TO_READY start");
#endif /* GST_EXT_XV_ENHANCEMENT */
      if (!gst_xvimagesink_open (xvimagesink))
        goto error;
#ifdef GST_EXT_XV_ENHANCEMENT
      GST_WARNING("NULL_TO_READY done");
#endif /* GST_EXT_XV_ENHANCEMENT */
      break;
    case GST_STATE_CHANGE_READY_TO_PAUSED:
      break;
    case GST_STATE_CHANGE_PAUSED_TO_PLAYING:
#ifdef GST_EXT_XV_ENHANCEMENT
        GST_WARNING("PAUSED_TO_PLAYING done");
#endif /* GST_EXT_XV_ENHANCEMENT */
      break;
    case GST_STATE_CHANGE_PAUSED_TO_READY:
      break;
    default:
      break;
  }

  ret = GST_ELEMENT_CLASS (parent_class)->change_state (element, transition);

  switch (transition) {
    case GST_STATE_CHANGE_PLAYING_TO_PAUSED:
#ifdef GST_EXT_XV_ENHANCEMENT
      GST_WARNING("PLAYING_TO_PAUSED start");
      /* init displayed buffer count */
      xvimagesink->context->displayed_buffer_count = 0;

      GST_WARNING("PLAYING_TO_PAUSED done");
#endif /* GST_EXT_XV_ENHANCEMENT */
      break;
    case GST_STATE_CHANGE_PAUSED_TO_READY:
      xvimagesink->fps_n = 0;
      xvimagesink->fps_d = 1;
      GST_VIDEO_SINK_WIDTH (xvimagesink) = 0;
      GST_VIDEO_SINK_HEIGHT (xvimagesink) = 0;
      g_mutex_lock (&xvimagesink->flow_lock);
      if (xvimagesink->pool)
        gst_buffer_pool_set_active (xvimagesink->pool, FALSE);
      g_mutex_unlock (&xvimagesink->flow_lock);
#ifdef GST_EXT_XV_ENHANCEMENT
      /* close drm */
      gst_xvcontext_drm_fini(xvimagesink->context);

      GST_WARNING("PAUSED_TO_READY done");
#endif /* GST_EXT_XV_ENHANCEMENT */
      break;
    case GST_STATE_CHANGE_READY_TO_NULL:
#ifdef GST_EXT_XV_ENHANCEMENT
      GST_WARNING("READY_TO_NULL start");
#endif /* GST_EXT_XV_ENHANCEMENT */
      gst_xvimagesink_close (xvimagesink);
#ifdef GST_EXT_XV_ENHANCEMENT
      GST_WARNING("READY_TO_NULL done");
#endif /* GST_EXT_XV_ENHANCEMENT */
      break;
    default:
      break;
  }
  return ret;

error:
  {
    return GST_STATE_CHANGE_FAILURE;
  }
}

static void
gst_xvimagesink_get_times (GstBaseSink * bsink, GstBuffer * buf,
    GstClockTime * start, GstClockTime * end)
{
  GstXvImageSink *xvimagesink;

  xvimagesink = GST_XVIMAGESINK (bsink);

  if (GST_BUFFER_TIMESTAMP_IS_VALID (buf)) {
    *start = GST_BUFFER_TIMESTAMP (buf);
    if (GST_BUFFER_DURATION_IS_VALID (buf)) {
      *end = *start + GST_BUFFER_DURATION (buf);
    } else {
      if (xvimagesink->fps_n > 0) {
        *end = *start +
            gst_util_uint64_scale_int (GST_SECOND, xvimagesink->fps_d,
            xvimagesink->fps_n);
      }
    }
  }
}

static GstFlowReturn
gst_xvimagesink_show_frame (GstVideoSink * vsink, GstBuffer * buf)
{
  GstFlowReturn res;
  GstXvImageSink *xvimagesink;
  GstBuffer *to_put = NULL;
  GstMemory *mem;
  gboolean ret = FALSE;
#ifdef GST_EXT_XV_ENHANCEMENT
  GstMapInfo mem_info = GST_MAP_INFO_INIT;
  SCMN_IMGB *scmn_imgb = NULL;
#endif /* GST_EXT_XV_ENHANCEMENT */

  xvimagesink = GST_XVIMAGESINK (vsink);

  if (gst_buffer_n_memory (buf) == 1 && (mem = gst_buffer_peek_memory (buf, 0))
      && gst_xvimage_memory_is_from_context (mem, xvimagesink->context)) {
    /* If this buffer has been allocated using our buffer management we simply
       put the ximage which is in the PRIVATE pointer */
    GST_LOG_OBJECT (xvimagesink, "buffer %p from our pool, writing directly",
        buf);
#ifdef GST_EXT_XV_ENHANCEMENT
    xvimagesink->xid_updated = FALSE;
#endif /* GST_EXT_XV_ENHANCEMENT */
    to_put = buf;
    res = GST_FLOW_OK;
  } else {
    GstVideoFrame src, dest;
    GstBufferPoolAcquireParams params = { 0, };

    /* Else we have to copy the data into our private image, */
    /* if we have one... */
    GST_LOG_OBJECT (xvimagesink, "buffer %p not from our pool, copying", buf);

    /* we should have a pool, configured in setcaps */
    if (xvimagesink->pool == NULL)
      goto no_pool;

    if (!gst_buffer_pool_set_active (xvimagesink->pool, TRUE))
      goto activate_failed;

    /* take a buffer from our pool, if there is no buffer in the pool something
     * is seriously wrong, waiting for the pool here might deadlock when we try
     * to go to PAUSED because we never flush the pool then. */
    params.flags = GST_BUFFER_POOL_ACQUIRE_FLAG_DONTWAIT;
    res = gst_buffer_pool_acquire_buffer (xvimagesink->pool, &to_put, &params);
    if (res != GST_FLOW_OK)
      goto no_buffer;

    GST_CAT_LOG_OBJECT (GST_CAT_PERFORMANCE, xvimagesink,
        "slow copy buffer %p into bufferpool buffer %p", buf, to_put);
#ifdef GST_EXT_XV_ENHANCEMENT
    switch (GST_VIDEO_INFO_FORMAT(&xvimagesink->info)) {
      /* Cases for specified formats of Samsung extension */
     case GST_VIDEO_FORMAT_SN12:
     case GST_VIDEO_FORMAT_ST12:
     case GST_VIDEO_FORMAT_ITLV:
     {
       GstXvImageMemory* img_mem = NULL;
       XV_DATA_PTR img_data = NULL;

       GST_LOG("Samsung EXT format - name:%s, display mode:%d, Rotate angle:%d", GST_VIDEO_INFO_NAME(&xvimagesink->info),
               xvimagesink->config.display_mode, xvimagesink->rotate_angle);

       img_mem = (GstXvImageMemory*)gst_buffer_peek_memory(to_put, 0);
       img_data = (XV_DATA_PTR) gst_xvimage_memory_get_xvimage(img_mem)->data;
       if (img_data) {
         memset(img_data, 0x0, sizeof(XV_DATA));
         XV_INIT_DATA(img_data);

         mem = gst_buffer_peek_memory(buf, 1);
         gst_memory_map(mem, &mem_info, GST_MAP_READ);
         scmn_imgb = (SCMN_IMGB *)mem_info.data;
         gst_memory_unmap(mem, &mem_info);
         if (scmn_imgb == NULL) {
           GST_DEBUG_OBJECT( xvimagesink, "scmn_imgb is NULL. Skip xvimage put..." );
           g_mutex_unlock (&xvimagesink->flow_lock);
           return GST_FLOW_OK;
         }

         if (scmn_imgb->buf_share_method == BUF_SHARE_METHOD_PADDR) {
           img_data->YBuf = (unsigned int)scmn_imgb->p[0];
           img_data->CbBuf = (unsigned int)scmn_imgb->p[1];
           img_data->CrBuf = (unsigned int)scmn_imgb->p[2];
           img_data->BufType = XV_BUF_TYPE_LEGACY;
         } else if (scmn_imgb->buf_share_method == BUF_SHARE_METHOD_FD) {
           /* open drm to use gem */
           if (xvimagesink->context->drm_fd < 0) {
               gst_xvcontext_drm_init(xvimagesink->context);
           }

           if (scmn_imgb->buf_share_method == BUF_SHARE_METHOD_FD) {
             /* keep dma-buf fd. fd will be converted in gst_xvimagesink_xvimage_put */
             img_data->dmabuf_fd[0] = scmn_imgb->dmabuf_fd[0];
             img_data->dmabuf_fd[1] = scmn_imgb->dmabuf_fd[1];
             img_data->dmabuf_fd[2] = scmn_imgb->dmabuf_fd[2];
             img_data->BufType = XV_BUF_TYPE_DMABUF;
             GST_DEBUG("DMABUF fd %u,%u,%u", img_data->dmabuf_fd[0], img_data->dmabuf_fd[1], img_data->dmabuf_fd[2]);
           } else {
             /* keep bo. bo will be converted in gst_xvimagesink_xvimage_put */
             img_data->bo[0] = scmn_imgb->bo[0];
             img_data->bo[1] = scmn_imgb->bo[1];
             img_data->bo[2] = scmn_imgb->bo[2];
             GST_DEBUG("TBM bo %p %p %p", img_data->bo[0], img_data->bo[1], img_data->bo[2]);
           }

           /* check secure contents path */
           /* NOTE : does it need to set 0 during playing(recovery case)? */
           if (scmn_imgb->tz_enable) {
             if (!xvimagesink->is_secure_path) {
               Atom atom_secure = None;
               g_mutex_lock (&xvimagesink->context->lock);
               atom_secure = XInternAtom(xvimagesink->context->disp, "_USER_WM_PORT_ATTRIBUTE_SECURE", False);
               if (atom_secure != None) {
                 if (XvSetPortAttribute(xvimagesink->context->disp, xvimagesink->context->xv_port_id, atom_secure, 1) != Success) {
                   GST_ERROR_OBJECT(xvimagesink, "%d: XvSetPortAttribute: secure setting failed.\n", atom_secure);
                 } else {
                   GST_WARNING_OBJECT(xvimagesink, "secure contents path is enabled.\n");
                 }
                   XSync (xvimagesink->context->disp, FALSE);
               }
               g_mutex_unlock (&xvimagesink->context->lock);
               xvimagesink->is_secure_path = TRUE;
             }
           }

           /* set current buffer */
           gst_xvimage_memory_set_buffer(img_mem, buf);
        } else {
          GST_WARNING("unknown buf_share_method type [%d]. skip xvimage put...",
                      scmn_imgb->buf_share_method);
          g_mutex_unlock (&xvimagesink->flow_lock);
          return GST_FLOW_OK;
        }

       } else {
          GST_WARNING_OBJECT( xvimagesink, "xvimage->data is NULL. skip xvimage put..." );
          return GST_FLOW_OK;
        }
        break;
      }
      default:
      {
        GST_DEBUG("Normal format activated. Name = %s", GST_VIDEO_INFO_NAME(&xvimagesink->info));
        if (!gst_video_frame_map (&src, &xvimagesink->info, buf, GST_MAP_READ))
          goto invalid_buffer;

        if (!gst_video_frame_map (&dest, &xvimagesink->info, to_put, GST_MAP_WRITE)) {
          gst_video_frame_unmap (&src);
          goto invalid_buffer;
        }

        gst_video_frame_copy (&dest, &src);

        gst_video_frame_unmap (&dest);
        gst_video_frame_unmap (&src);
        break;
      }
    }
#else /* GST_EXT_XV_ENHANCEMENT */
    if (!gst_video_frame_map (&src, &xvimagesink->info, buf, GST_MAP_READ))
      goto invalid_buffer;

    if (!gst_video_frame_map (&dest, &xvimagesink->info, to_put, GST_MAP_WRITE)) {
      gst_video_frame_unmap (&src);
      goto invalid_buffer;
    }

    gst_video_frame_copy (&dest, &src);

    gst_video_frame_unmap (&dest);
    gst_video_frame_unmap (&src);
#endif /* GST_EXT_XV_ENHANCEMENT */
  }

  ret = gst_xvimagesink_xvimage_put(xvimagesink, to_put);

  if (!ret) {
    goto no_window;
  }

done:
  if (to_put != buf)
    gst_buffer_unref (to_put);

  return res;

  /* ERRORS */
no_pool:
  {
    GST_ELEMENT_ERROR (xvimagesink, RESOURCE, WRITE,
        ("Internal error: can't allocate images"),
        ("We don't have a bufferpool negotiated"));
    return GST_FLOW_ERROR;
  }
no_buffer:
  {
    /* No image available. That's very bad ! */
    GST_WARNING_OBJECT (xvimagesink, "could not create image");
    return GST_FLOW_OK;
  }
invalid_buffer:
  {
    /* No Window available to put our image into */
    GST_WARNING_OBJECT (xvimagesink, "could not map image");
    res = GST_FLOW_OK;
    goto done;
  }
no_window:
  {
    /* No Window available to put our image into */
    GST_WARNING_OBJECT (xvimagesink, "could not output image - no window");
    res = GST_FLOW_ERROR;
    goto done;
  }
activate_failed:
  {
    GST_ERROR_OBJECT (xvimagesink, "failed to activate bufferpool.");
    res = GST_FLOW_ERROR;
    goto done;
  }
}

static gboolean
gst_xvimagesink_event (GstBaseSink * sink, GstEvent * event)
{
  GstXvImageSink *xvimagesink = GST_XVIMAGESINK (sink);

  switch (GST_EVENT_TYPE (event)) {
    case GST_EVENT_TAG:{
      GstTagList *l;
      gchar *title = NULL;

      gst_event_parse_tag (event, &l);
      gst_tag_list_get_string (l, GST_TAG_TITLE, &title);

      if (title) {
        GST_DEBUG_OBJECT (xvimagesink, "got tags, title='%s'", title);
        gst_xvimagesink_xwindow_set_title (xvimagesink, xvimagesink->xwindow,
            title);

        g_free (title);
      }
      break;
    }
    default:
      break;
  }
  return GST_BASE_SINK_CLASS (parent_class)->event (sink, event);
}

static gboolean
gst_xvimagesink_propose_allocation (GstBaseSink * bsink, GstQuery * query)
{
  GstXvImageSink *xvimagesink = GST_XVIMAGESINK (bsink);
  GstBufferPool *pool;
  GstStructure *config;
  GstCaps *caps;
  guint size;
  gboolean need_pool;

  gst_query_parse_allocation (query, &caps, &need_pool);

  if (caps == NULL)
    goto no_caps;

  g_mutex_lock (&xvimagesink->flow_lock);
  if ((pool = xvimagesink->pool))
    gst_object_ref (pool);
  g_mutex_unlock (&xvimagesink->flow_lock);

  if (pool != NULL) {
    GstCaps *pcaps;

    /* we had a pool, check caps */
    GST_DEBUG_OBJECT (xvimagesink, "check existing pool caps");
    config = gst_buffer_pool_get_config (pool);
    gst_buffer_pool_config_get_params (config, &pcaps, &size, NULL, NULL);

    if (!gst_caps_is_equal (caps, pcaps)) {
      GST_DEBUG_OBJECT (xvimagesink, "pool has different caps");
      /* different caps, we can't use this pool */
      gst_object_unref (pool);
      pool = NULL;
    }
    gst_structure_free (config);
  }
  if (pool == NULL && need_pool) {
    GstVideoInfo info;

    if (!gst_video_info_from_caps (&info, caps))
      goto invalid_caps;

    GST_DEBUG_OBJECT (xvimagesink, "create new pool");
    pool = gst_xvimage_buffer_pool_new (xvimagesink->allocator);

    /* the normal size of a frame */
    size = info.size;

    config = gst_buffer_pool_get_config (pool);
    gst_buffer_pool_config_set_params (config, caps, size, 0, 0);
    if (!gst_buffer_pool_set_config (pool, config))
      goto config_failed;
  }
  if (pool) {
    /* we need at least 2 buffer because we hold on to the last one */
    gst_query_add_allocation_pool (query, pool, size, 2, 0);
    gst_object_unref (pool);
  }

  /* we also support various metadata */
  gst_query_add_allocation_meta (query, GST_VIDEO_META_API_TYPE, NULL);
  gst_query_add_allocation_meta (query, GST_VIDEO_CROP_META_API_TYPE, NULL);

  return TRUE;

  /* ERRORS */
no_caps:
  {
    GST_DEBUG_OBJECT (bsink, "no caps specified");
    return FALSE;
  }
invalid_caps:
  {
    GST_DEBUG_OBJECT (bsink, "invalid caps specified");
    return FALSE;
  }
config_failed:
  {
    GST_DEBUG_OBJECT (bsink, "failed setting config");
    gst_object_unref (pool);
    return FALSE;
  }
}

/* Interfaces stuff */
static void
gst_xvimagesink_navigation_send_event (GstNavigation * navigation,
    GstStructure * structure)
{
  GstXvImageSink *xvimagesink = GST_XVIMAGESINK (navigation);
  GstPad *peer;

  if ((peer = gst_pad_get_peer (GST_VIDEO_SINK_PAD (xvimagesink)))) {
    GstEvent *event;
    GstVideoRectangle src, dst, result;
    gdouble x, y, xscale = 1.0, yscale = 1.0;
    GstXWindow *xwindow;

    event = gst_event_new_navigation (structure);

    /* We take the flow_lock while we look at the window */
    g_mutex_lock (&xvimagesink->flow_lock);

    if (!(xwindow = xvimagesink->xwindow)) {
      g_mutex_unlock (&xvimagesink->flow_lock);
      return;
    }

    if (xvimagesink->keep_aspect) {
      /* We get the frame position using the calculated geometry from _setcaps
         that respect pixel aspect ratios */
      src.w = GST_VIDEO_SINK_WIDTH (xvimagesink);
      src.h = GST_VIDEO_SINK_HEIGHT (xvimagesink);
      dst.w = xwindow->render_rect.w;
      dst.h = xwindow->render_rect.h;

      gst_video_sink_center_rect (src, dst, &result, TRUE);
      result.x += xwindow->render_rect.x;
      result.y += xwindow->render_rect.y;
    } else {
      memcpy (&result, &xwindow->render_rect, sizeof (GstVideoRectangle));
    }

    g_mutex_unlock (&xvimagesink->flow_lock);

    /* We calculate scaling using the original video frames geometry to include
       pixel aspect ratio scaling. */
    xscale = (gdouble) xvimagesink->video_width / result.w;
    yscale = (gdouble) xvimagesink->video_height / result.h;

    /* Converting pointer coordinates to the non scaled geometry */
    if (gst_structure_get_double (structure, "pointer_x", &x)) {
      x = MIN (x, result.x + result.w);
      x = MAX (x - result.x, 0);
      gst_structure_set (structure, "pointer_x", G_TYPE_DOUBLE,
          (gdouble) x * xscale, NULL);
    }
    if (gst_structure_get_double (structure, "pointer_y", &y)) {
      y = MIN (y, result.y + result.h);
      y = MAX (y - result.y, 0);
      gst_structure_set (structure, "pointer_y", G_TYPE_DOUBLE,
          (gdouble) y * yscale, NULL);
    }

    gst_pad_send_event (peer, event);
    gst_object_unref (peer);
  }
}

static void
gst_xvimagesink_navigation_init (GstNavigationInterface * iface)
{
  iface->send_event = gst_xvimagesink_navigation_send_event;
}

static void
gst_xvimagesink_set_window_handle (GstVideoOverlay * overlay, guintptr id)
{
  XID xwindow_id = id;
  GstXvImageSink *xvimagesink = GST_XVIMAGESINK (overlay);
  GstXWindow *xwindow = NULL;
  GstXvContext *context;
#ifdef GST_EXT_XV_ENHANCEMENT
  GstState current_state = GST_STATE_NULL;
#endif /* GST_EXT_XV_ENHANCEMENT */

  g_return_if_fail (GST_IS_XVIMAGESINK (xvimagesink));

  g_mutex_lock (&xvimagesink->flow_lock);

#ifdef GST_EXT_XV_ENHANCEMENT
  gst_element_get_state(GST_ELEMENT(xvimagesink), &current_state, NULL, 0);
  GST_WARNING_OBJECT(xvimagesink, "ENTER, id : %d, current state : %d",
                                  xwindow_id, current_state);
#endif /* GST_EXT_XV_ENHANCEMENT */

  /* If we already use that window return */
  if (xvimagesink->xwindow && (xwindow_id == xvimagesink->xwindow->win)) {
    g_mutex_unlock (&xvimagesink->flow_lock);
    return;
  }

  /* If the element has not initialized the X11 context try to do so */
  if (!xvimagesink->context &&
      !(xvimagesink->context =
          gst_xvcontext_new (&xvimagesink->config, NULL))) {
    g_mutex_unlock (&xvimagesink->flow_lock);
    /* we have thrown a GST_ELEMENT_ERROR now */
    return;
  }

  context = xvimagesink->context;

  gst_xvimagesink_update_colorbalance (xvimagesink);

  /* If a window is there already we destroy it */
  if (xvimagesink->xwindow) {
    gst_xwindow_destroy (xvimagesink->xwindow);
    xvimagesink->xwindow = NULL;
  }

  /* If the xid is 0 we go back to an internal window */
  if (xwindow_id == 0) {
    /* If no width/height caps nego did not happen window will be created
       during caps nego then */
    if (GST_VIDEO_SINK_WIDTH (xvimagesink)
        && GST_VIDEO_SINK_HEIGHT (xvimagesink)) {
      xwindow =
          gst_xvimagesink_xwindow_new (xvimagesink,
          GST_VIDEO_SINK_WIDTH (xvimagesink),
          GST_VIDEO_SINK_HEIGHT (xvimagesink));
    }
  } else {
    xwindow = gst_xvcontext_create_xwindow_from_xid (context, xwindow_id);
    gst_xwindow_set_event_handling (xwindow, xvimagesink->handle_events);
  }

  if (xwindow)
    xvimagesink->xwindow = xwindow;

#ifdef GST_EXT_XV_ENHANCEMENT
  xvimagesink->xid_updated = TRUE;
#endif /* GST_EXT_XV_ENHANCEMENT */

  g_mutex_unlock (&xvimagesink->flow_lock);

#ifdef GST_EXT_XV_ENHANCEMENT
  if (current_state == GST_STATE_PAUSED) {
    GstBuffer *last_buffer = NULL;
    g_object_get(G_OBJECT(xvimagesink), "last-buffer", &last_buffer, NULL);
    GST_WARNING_OBJECT(xvimagesink, "PASUED state: window handle is updated. last buffer %p", last_buffer);
    if (last_buffer) {
      gst_xvimagesink_show_frame((GstVideoSink *)xvimagesink, last_buffer);
      gst_buffer_unref(last_buffer);
      last_buffer = NULL;
    }
  }
#endif /* GST_EXT_XV_ENHANCEMENT */
}

static void
gst_xvimagesink_expose (GstVideoOverlay * overlay)
{
  GstXvImageSink *xvimagesink = GST_XVIMAGESINK (overlay);

  GST_DEBUG ("doing expose");
  gst_xvimagesink_xwindow_update_geometry (xvimagesink);
  gst_xvimagesink_xvimage_put (xvimagesink, NULL);
}

static void
gst_xvimagesink_set_event_handling (GstVideoOverlay * overlay,
    gboolean handle_events)
{
  GstXvImageSink *xvimagesink = GST_XVIMAGESINK (overlay);

  g_mutex_lock (&xvimagesink->flow_lock);
  xvimagesink->handle_events = handle_events;
  if (G_LIKELY (xvimagesink->xwindow))
    gst_xwindow_set_event_handling (xvimagesink->xwindow, handle_events);
  g_mutex_unlock (&xvimagesink->flow_lock);
}

static void
gst_xvimagesink_set_render_rectangle (GstVideoOverlay * overlay, gint x, gint y,
    gint width, gint height)
{
  GstXvImageSink *xvimagesink = GST_XVIMAGESINK (overlay);

  g_mutex_lock (&xvimagesink->flow_lock);
  if (G_LIKELY (xvimagesink->xwindow))
    gst_xwindow_set_render_rectangle (xvimagesink->xwindow, x, y, width,
        height);
  g_mutex_unlock (&xvimagesink->flow_lock);
}

static void
gst_xvimagesink_video_overlay_init (GstVideoOverlayInterface * iface)
{
  iface->set_window_handle = gst_xvimagesink_set_window_handle;
  iface->expose = gst_xvimagesink_expose;
  iface->handle_events = gst_xvimagesink_set_event_handling;
  iface->set_render_rectangle = gst_xvimagesink_set_render_rectangle;
}

static const GList *
gst_xvimagesink_colorbalance_list_channels (GstColorBalance * balance)
{
  GstXvImageSink *xvimagesink = GST_XVIMAGESINK (balance);

  g_return_val_if_fail (GST_IS_XVIMAGESINK (xvimagesink), NULL);

  if (xvimagesink->context)
    return xvimagesink->context->channels_list;
  else
    return NULL;
}

static void
gst_xvimagesink_colorbalance_set_value (GstColorBalance * balance,
    GstColorBalanceChannel * channel, gint value)
{
  GstXvImageSink *xvimagesink = GST_XVIMAGESINK (balance);

  g_return_if_fail (GST_IS_XVIMAGESINK (xvimagesink));
  g_return_if_fail (channel->label != NULL);

  xvimagesink->config.cb_changed = TRUE;

  /* Normalize val to [-1000, 1000] */
  value = floor (0.5 + -1000 + 2000 * (value - channel->min_value) /
      (double) (channel->max_value - channel->min_value));

  if (g_ascii_strcasecmp (channel->label, "XV_HUE") == 0) {
    xvimagesink->config.hue = value;
  } else if (g_ascii_strcasecmp (channel->label, "XV_SATURATION") == 0) {
    xvimagesink->config.saturation = value;
  } else if (g_ascii_strcasecmp (channel->label, "XV_CONTRAST") == 0) {
    xvimagesink->config.contrast = value;
  } else if (g_ascii_strcasecmp (channel->label, "XV_BRIGHTNESS") == 0) {
    xvimagesink->config.brightness = value;
  } else {
    g_warning ("got an unknown channel %s", channel->label);
    return;
  }

  gst_xvimagesink_update_colorbalance (xvimagesink);
}

static gint
gst_xvimagesink_colorbalance_get_value (GstColorBalance * balance,
    GstColorBalanceChannel * channel)
{
  GstXvImageSink *xvimagesink = GST_XVIMAGESINK (balance);
  gint value = 0;

  g_return_val_if_fail (GST_IS_XVIMAGESINK (xvimagesink), 0);
  g_return_val_if_fail (channel->label != NULL, 0);

  if (g_ascii_strcasecmp (channel->label, "XV_HUE") == 0) {
    value = xvimagesink->config.hue;
  } else if (g_ascii_strcasecmp (channel->label, "XV_SATURATION") == 0) {
    value = xvimagesink->config.saturation;
  } else if (g_ascii_strcasecmp (channel->label, "XV_CONTRAST") == 0) {
    value = xvimagesink->config.contrast;
  } else if (g_ascii_strcasecmp (channel->label, "XV_BRIGHTNESS") == 0) {
    value = xvimagesink->config.brightness;
  } else {
    g_warning ("got an unknown channel %s", channel->label);
  }

  /* Normalize val to [channel->min_value, channel->max_value] */
  value = channel->min_value + (channel->max_value - channel->min_value) *
      (value + 1000) / 2000;

  return value;
}

static GstColorBalanceType
gst_xvimagesink_colorbalance_get_balance_type (GstColorBalance * balance)
{
  return GST_COLOR_BALANCE_HARDWARE;
}

static void
gst_xvimagesink_colorbalance_init (GstColorBalanceInterface * iface)
{
  iface->list_channels = gst_xvimagesink_colorbalance_list_channels;
  iface->set_value = gst_xvimagesink_colorbalance_set_value;
  iface->get_value = gst_xvimagesink_colorbalance_get_value;
  iface->get_balance_type = gst_xvimagesink_colorbalance_get_balance_type;
}

#if 0
static const GList *
gst_xvimagesink_probe_get_properties (GstPropertyProbe * probe)
{
  GObjectClass *klass = G_OBJECT_GET_CLASS (probe);
  static GList *list = NULL;

  if (!list) {
    list = g_list_append (NULL, g_object_class_find_property (klass, "device"));
    list =
        g_list_append (list, g_object_class_find_property (klass,
            "autopaint-colorkey"));
    list =
        g_list_append (list, g_object_class_find_property (klass,
            "double-buffer"));
    list =
        g_list_append (list, g_object_class_find_property (klass, "colorkey"));
  }

  return list;
}

static void
gst_xvimagesink_probe_probe_property (GstPropertyProbe * probe,
    guint prop_id, const GParamSpec * pspec)
{
  GstXvImageSink *xvimagesink = GST_XVIMAGESINK (probe);

  switch (prop_id) {
    case PROP_DEVICE:
    case PROP_AUTOPAINT_COLORKEY:
    case PROP_DOUBLE_BUFFER:
    case PROP_COLORKEY:
      GST_DEBUG_OBJECT (xvimagesink,
          "probing device list and get capabilities");
      if (!xvimagesink->context) {
        GST_DEBUG_OBJECT (xvimagesink, "generating context");
        xvimagesink->context = gst_xvimagesink_context_get (xvimagesink);
      }
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (probe, prop_id, pspec);
      break;
  }
}

static gboolean
gst_xvimagesink_probe_needs_probe (GstPropertyProbe * probe,
    guint prop_id, const GParamSpec * pspec)
{
  GstXvImageSink *xvimagesink = GST_XVIMAGESINK (probe);
  gboolean ret = FALSE;

  switch (prop_id) {
    case PROP_DEVICE:
    case PROP_AUTOPAINT_COLORKEY:
    case PROP_DOUBLE_BUFFER:
    case PROP_COLORKEY:
      if (xvimagesink->context != NULL) {
        ret = FALSE;
      } else {
        ret = TRUE;
      }
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (probe, prop_id, pspec);
      break;
  }

  return ret;
}

static GValueArray *
gst_xvimagesink_probe_get_values (GstPropertyProbe * probe,
    guint prop_id, const GParamSpec * pspec)
{
  GstXvImageSink *xvimagesink = GST_XVIMAGESINK (probe);
  GValueArray *array = NULL;

  if (G_UNLIKELY (!xvimagesink->context)) {
    GST_WARNING_OBJECT (xvimagesink, "we don't have any context, can't "
        "get values");
    goto beach;
  }

  switch (prop_id) {
    case PROP_DEVICE:
    {
      guint i;
      GValue value = { 0 };

      array = g_value_array_new (xvimagesink->context->nb_adaptors);
      g_value_init (&value, G_TYPE_STRING);

      for (i = 0; i < xvimagesink->context->nb_adaptors; i++) {
        gchar *adaptor_id_s = g_strdup_printf ("%u", i);

        g_value_set_string (&value, adaptor_id_s);
        g_value_array_append (array, &value);
        g_free (adaptor_id_s);
      }
      g_value_unset (&value);
      break;
    }
    case PROP_AUTOPAINT_COLORKEY:
      if (xvimagesink->have_autopaint_colorkey) {
        GValue value = { 0 };

        array = g_value_array_new (2);
        g_value_init (&value, G_TYPE_BOOLEAN);
        g_value_set_boolean (&value, FALSE);
        g_value_array_append (array, &value);
        g_value_set_boolean (&value, TRUE);
        g_value_array_append (array, &value);
        g_value_unset (&value);
      }
      break;
    case PROP_DOUBLE_BUFFER:
      if (xvimagesink->have_double_buffer) {
        GValue value = { 0 };

        array = g_value_array_new (2);
        g_value_init (&value, G_TYPE_BOOLEAN);
        g_value_set_boolean (&value, FALSE);
        g_value_array_append (array, &value);
        g_value_set_boolean (&value, TRUE);
        g_value_array_append (array, &value);
        g_value_unset (&value);
      }
      break;
    case PROP_COLORKEY:
      if (xvimagesink->have_colorkey) {
        GValue value = { 0 };

        array = g_value_array_new (1);
        g_value_init (&value, GST_TYPE_INT_RANGE);
        gst_value_set_int_range (&value, 0, 0xffffff);
        g_value_array_append (array, &value);
        g_value_unset (&value);
      }
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (probe, prop_id, pspec);
      break;
  }

beach:
  return array;
}

static void
gst_xvimagesink_property_probe_interface_init (GstPropertyProbeInterface *
    iface)
{
  iface->get_properties = gst_xvimagesink_probe_get_properties;
  iface->probe_property = gst_xvimagesink_probe_probe_property;
  iface->needs_probe = gst_xvimagesink_probe_needs_probe;
  iface->get_values = gst_xvimagesink_probe_get_values;
}
#endif

/* =========================================== */
/*                                             */
/*              Init & Class init              */
/*                                             */
/* =========================================== */

static void
gst_xvimagesink_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstXvImageSink *xvimagesink;

  g_return_if_fail (GST_IS_XVIMAGESINK (object));

  xvimagesink = GST_XVIMAGESINK (object);

  switch (prop_id) {
    case PROP_HUE:
      xvimagesink->config.hue = g_value_get_int (value);
      xvimagesink->config.cb_changed = TRUE;
      gst_xvimagesink_update_colorbalance (xvimagesink);
      break;
    case PROP_CONTRAST:
      xvimagesink->config.contrast = g_value_get_int (value);
      xvimagesink->config.cb_changed = TRUE;
      gst_xvimagesink_update_colorbalance (xvimagesink);
      break;
    case PROP_BRIGHTNESS:
      xvimagesink->config.brightness = g_value_get_int (value);
      xvimagesink->config.cb_changed = TRUE;
      gst_xvimagesink_update_colorbalance (xvimagesink);
      break;
    case PROP_SATURATION:
      xvimagesink->config.saturation = g_value_get_int (value);
      xvimagesink->config.cb_changed = TRUE;
      gst_xvimagesink_update_colorbalance (xvimagesink);
      break;
    case PROP_DISPLAY:
      g_free (xvimagesink->config.display_name);
      xvimagesink->config.display_name = g_strdup (g_value_get_string (value));
      break;
    case PROP_SYNCHRONOUS:
      xvimagesink->synchronous = g_value_get_boolean (value);
      if (xvimagesink->context) {
        gst_xvcontext_set_synchronous (xvimagesink->context,
            xvimagesink->synchronous);
      }
      break;
    case PROP_PIXEL_ASPECT_RATIO:
      g_free (xvimagesink->par);
      xvimagesink->par = g_new0 (GValue, 1);
      g_value_init (xvimagesink->par, GST_TYPE_FRACTION);
      if (!g_value_transform (value, xvimagesink->par)) {
        g_warning ("Could not transform string to aspect ratio");
        gst_value_set_fraction (xvimagesink->par, 1, 1);
      }
      GST_DEBUG_OBJECT (xvimagesink, "set PAR to %d/%d",
          gst_value_get_fraction_numerator (xvimagesink->par),
          gst_value_get_fraction_denominator (xvimagesink->par));
      break;
    case PROP_FORCE_ASPECT_RATIO:
      xvimagesink->keep_aspect = g_value_get_boolean (value);
      break;
    case PROP_HANDLE_EVENTS:
      gst_xvimagesink_set_event_handling (GST_VIDEO_OVERLAY (xvimagesink),
          g_value_get_boolean (value));
      gst_xvimagesink_manage_event_thread (xvimagesink);
      break;
    case PROP_DEVICE:
      xvimagesink->config.adaptor_nr = atoi (g_value_get_string (value));
      break;
    case PROP_HANDLE_EXPOSE:
      xvimagesink->handle_expose = g_value_get_boolean (value);
      gst_xvimagesink_manage_event_thread (xvimagesink);
      break;
    case PROP_DOUBLE_BUFFER:
      xvimagesink->double_buffer = g_value_get_boolean (value);
      break;
    case PROP_AUTOPAINT_COLORKEY:
      xvimagesink->config.autopaint_colorkey = g_value_get_boolean (value);
      break;
    case PROP_COLORKEY:
      xvimagesink->config.colorkey = g_value_get_int (value);
      break;
    case PROP_DRAW_BORDERS:
      xvimagesink->draw_borders = g_value_get_boolean (value);
      break;
#ifdef GST_EXT_XV_ENHANCEMENT
    case PROP_DISPLAY_MODE:
    {
      int set_mode = g_value_get_enum (value);

      g_mutex_lock(&xvimagesink->flow_lock);
      g_mutex_lock(&xvimagesink->context->lock);

      if (xvimagesink->config.display_mode != set_mode) {
        if (xvimagesink->context) {
          /* set display mode */
          if (gst_xvcontext_set_display_mode(xvimagesink->context, set_mode)) {
            xvimagesink->config.display_mode = set_mode;
          } else {
            GST_WARNING_OBJECT(xvimagesink, "display mode[%d] set failed.", set_mode);
          }
        } else {
          /* "xcontext" is not created yet. It will be applied when xcontext is created. */
          GST_INFO_OBJECT(xvimagesink, "xcontext is NULL. display-mode will be set later.");
          xvimagesink->config.display_mode = set_mode;
        }
      } else {
        GST_INFO_OBJECT(xvimagesink, "skip display mode %d, because current mode is same", set_mode);
      }

      g_mutex_unlock(&xvimagesink->context->lock);
      g_mutex_unlock(&xvimagesink->flow_lock);
    }
      break;
    case PROP_DISPLAY_GEOMETRY_METHOD:
      xvimagesink->display_geometry_method = g_value_get_enum (value);
      GST_LOG("Overlay geometry changed. update it");
      if (GST_STATE(xvimagesink) == GST_STATE_PAUSED) {
        gst_xvimagesink_xvimage_put (xvimagesink, xvimagesink->cur_image);
      }
      break;
    case PROP_FLIP:
      xvimagesink->flip = g_value_get_enum(value);
      break;
    case PROP_ROTATE_ANGLE:
      xvimagesink->rotate_angle = g_value_get_enum (value);
      if (GST_STATE(xvimagesink) == GST_STATE_PAUSED) {
        gst_xvimagesink_xvimage_put (xvimagesink, xvimagesink->cur_image);
      }
      break;
    case PROP_VISIBLE:
      g_mutex_lock( &xvimagesink->flow_lock );
      g_mutex_lock( &xvimagesink->context->lock );

      GST_WARNING_OBJECT(xvimagesink, "set visible %d", g_value_get_boolean(value));

      if (xvimagesink->visible && (g_value_get_boolean(value) == FALSE)) {
        if (xvimagesink->context) {
#if 0
          Atom atom_stream = XInternAtom( xvimagesink->context->disp,
                                          "_USER_WM_PORT_ATTRIBUTE_STREAM_OFF", False );
          if (atom_stream != None) {
            GST_WARNING_OBJECT(xvimagesink, "Visible FALSE -> CALL STREAM_OFF");
            if (XvSetPortAttribute(xvimagesink->context->disp,
                                   xvimagesink->context->xv_port_id,
                                   atom_stream, 0 ) != Success) {
              GST_WARNING_OBJECT( xvimagesink, "Set visible FALSE failed" );
            }
          }
#endif
          xvimagesink->visible = g_value_get_boolean (value);
          XvStopVideo(xvimagesink->context->disp, xvimagesink->context->xv_port_id, xvimagesink->xwindow->win);

          XSync( xvimagesink->context->disp, FALSE );
        } else {
          GST_WARNING_OBJECT( xvimagesink, "xcontext is null");
          xvimagesink->visible = g_value_get_boolean (value);
        }

      } else if (!xvimagesink->visible && (g_value_get_boolean(value) == TRUE)) {
        g_mutex_unlock( &xvimagesink->context->lock );
        g_mutex_unlock( &xvimagesink->flow_lock );
        xvimagesink->visible = g_value_get_boolean (value);
        gst_xvimagesink_xvimage_put (xvimagesink, xvimagesink->cur_image);
        g_mutex_lock( &xvimagesink->flow_lock );
        g_mutex_lock( &xvimagesink->context->lock );
      }

      GST_INFO("set visible(%d) done", xvimagesink->visible);

      g_mutex_unlock( &xvimagesink->context->lock );
      g_mutex_unlock( &xvimagesink->flow_lock );
      break;
    case PROP_ZOOM:
      xvimagesink->zoom = g_value_get_float (value);
      if (GST_STATE(xvimagesink) == GST_STATE_PAUSED) {
        gst_xvimagesink_xvimage_put (xvimagesink, xvimagesink->cur_image);
      }
      break;
    case PROP_ZOOM_POS_X:
      xvimagesink->zoom_pos_x = g_value_get_int (value);
      break;
    case PROP_ZOOM_POS_Y:
      xvimagesink->zoom_pos_y = g_value_get_int (value);
      if (GST_STATE(xvimagesink) == GST_STATE_PAUSED) {
        gst_xvimagesink_xvimage_put (xvimagesink, xvimagesink->cur_image);
      }
      break;
    case PROP_ORIENTATION:
      xvimagesink->orientation = g_value_get_enum (value);
      GST_INFO("Orientation(%d) is changed", xvimagesink->orientation);
      break;
    case PROP_DST_ROI_MODE:
      xvimagesink->dst_roi_mode = g_value_get_enum (value);
      GST_INFO("Overlay geometry(%d) for ROI is changed", xvimagesink->dst_roi_mode);
      break;
    case PROP_DST_ROI_X:
      xvimagesink->dst_roi.x = g_value_get_int (value);
      break;
    case PROP_DST_ROI_Y:
      xvimagesink->dst_roi.y = g_value_get_int (value);
      break;
    case PROP_DST_ROI_W:
      xvimagesink->dst_roi.w = g_value_get_int (value);
      break;
    case PROP_DST_ROI_H:
      xvimagesink->dst_roi.h = g_value_get_int (value);
      break;
#endif /* GST_EXT_XV_ENHANCEMENT */
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_xvimagesink_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  GstXvImageSink *xvimagesink;

  g_return_if_fail (GST_IS_XVIMAGESINK (object));

  xvimagesink = GST_XVIMAGESINK (object);

  switch (prop_id) {
    case PROP_HUE:
      g_value_set_int (value, xvimagesink->config.hue);
      break;
    case PROP_CONTRAST:
      g_value_set_int (value, xvimagesink->config.contrast);
      break;
    case PROP_BRIGHTNESS:
      g_value_set_int (value, xvimagesink->config.brightness);
      break;
    case PROP_SATURATION:
      g_value_set_int (value, xvimagesink->config.saturation);
      break;
    case PROP_DISPLAY:
      g_value_set_string (value, xvimagesink->config.display_name);
      break;
    case PROP_SYNCHRONOUS:
      g_value_set_boolean (value, xvimagesink->synchronous);
      break;
    case PROP_PIXEL_ASPECT_RATIO:
      if (xvimagesink->par)
        g_value_transform (xvimagesink->par, value);
      break;
    case PROP_FORCE_ASPECT_RATIO:
      g_value_set_boolean (value, xvimagesink->keep_aspect);
      break;
    case PROP_HANDLE_EVENTS:
      g_value_set_boolean (value, xvimagesink->handle_events);
      break;
    case PROP_DEVICE:
    {
      char *adaptor_nr_s =
          g_strdup_printf ("%u", xvimagesink->config.adaptor_nr);

      g_value_set_string (value, adaptor_nr_s);
      g_free (adaptor_nr_s);
      break;
    }
    case PROP_DEVICE_NAME:
      if (xvimagesink->context && xvimagesink->context->adaptors) {
        g_value_set_string (value,
            xvimagesink->context->adaptors[xvimagesink->config.adaptor_nr]);
      } else {
        g_value_set_string (value, NULL);
      }
      break;
    case PROP_HANDLE_EXPOSE:
      g_value_set_boolean (value, xvimagesink->handle_expose);
      break;
    case PROP_DOUBLE_BUFFER:
      g_value_set_boolean (value, xvimagesink->double_buffer);
      break;
    case PROP_AUTOPAINT_COLORKEY:
      g_value_set_boolean (value, xvimagesink->config.autopaint_colorkey);
      break;
    case PROP_COLORKEY:
      g_value_set_int (value, xvimagesink->config.colorkey);
      break;
    case PROP_DRAW_BORDERS:
      g_value_set_boolean (value, xvimagesink->draw_borders);
      break;
    case PROP_WINDOW_WIDTH:
      if (xvimagesink->xwindow)
        g_value_set_uint64 (value, xvimagesink->xwindow->width);
      else
        g_value_set_uint64 (value, 0);
      break;
    case PROP_WINDOW_HEIGHT:
      if (xvimagesink->xwindow)
        g_value_set_uint64 (value, xvimagesink->xwindow->height);
      else
        g_value_set_uint64 (value, 0);
      break;
#ifdef GST_EXT_XV_ENHANCEMENT
    case PROP_DISPLAY_MODE:
      g_value_set_enum (value, xvimagesink->config.display_mode);
      break;
    case PROP_DISPLAY_GEOMETRY_METHOD:
      g_value_set_enum (value, xvimagesink->display_geometry_method);
      break;
    case PROP_FLIP:
      g_value_set_enum(value, xvimagesink->flip);
      break;
    case PROP_ROTATE_ANGLE:
      g_value_set_enum (value, xvimagesink->rotate_angle);
      break;
    case PROP_VISIBLE:
      g_value_set_boolean (value, xvimagesink->visible);
      break;
    case PROP_ZOOM:
      g_value_set_float (value, xvimagesink->zoom);
      break;
    case PROP_ZOOM_POS_X:
      g_value_set_int (value, xvimagesink->zoom_pos_x);
      break;
    case PROP_ZOOM_POS_Y:
      g_value_set_int (value, xvimagesink->zoom_pos_y);
      break;
    case PROP_ORIENTATION:
      g_value_set_enum (value, xvimagesink->orientation);
      break;
    case PROP_DST_ROI_MODE:
      g_value_set_enum (value, xvimagesink->dst_roi_mode);
      break;
    case PROP_DST_ROI_X:
      g_value_set_int (value, xvimagesink->dst_roi.x);
      break;
    case PROP_DST_ROI_Y:
      g_value_set_int (value, xvimagesink->dst_roi.y);
      break;
    case PROP_DST_ROI_W:
      g_value_set_int (value, xvimagesink->dst_roi.w);
      break;
    case PROP_DST_ROI_H:
      g_value_set_int (value, xvimagesink->dst_roi.h);
      break;
#endif /* GST_EXT_XV_ENHANCEMENT */
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static gboolean
gst_xvimagesink_open (GstXvImageSink * xvimagesink)
{
  GError *error = NULL;

  /* Initializing the XvContext unless already done through GstVideoOverlay */
  if (!xvimagesink->context) {
    GstXvContext *context;
    if (!(context = gst_xvcontext_new (&xvimagesink->config, &error)))
      goto no_context;

    GST_OBJECT_LOCK (xvimagesink);
    xvimagesink->context = context;
  } else
    GST_OBJECT_LOCK (xvimagesink);
  /* make an allocator for this context */
  xvimagesink->allocator = gst_xvimage_allocator_new (xvimagesink->context);
  GST_OBJECT_UNLOCK (xvimagesink);

  /* update object's par with calculated one if not set yet */
  if (!xvimagesink->par) {
    xvimagesink->par = g_new0 (GValue, 1);
    gst_value_init_and_copy (xvimagesink->par, xvimagesink->context->par);
    GST_DEBUG_OBJECT (xvimagesink, "set calculated PAR on object's PAR");
  }
  /* call XSynchronize with the current value of synchronous */
  gst_xvcontext_set_synchronous (xvimagesink->context,
      xvimagesink->synchronous);
  gst_xvimagesink_update_colorbalance (xvimagesink);
  gst_xvimagesink_manage_event_thread (xvimagesink);

  return TRUE;

no_context:
  {
    gst_element_message_full (GST_ELEMENT (xvimagesink), GST_MESSAGE_ERROR,
        error->domain, error->code, g_strdup ("Could not initialise Xv output"),
        error->message, __FILE__, GST_FUNCTION, __LINE__);
    return FALSE;
  }
}

static void
gst_xvimagesink_close (GstXvImageSink * xvimagesink)
{
  GThread *thread;
  GstXvContext *context;

  GST_OBJECT_LOCK (xvimagesink);
  xvimagesink->running = FALSE;
  /* grab thread and mark it as NULL */
  thread = xvimagesink->event_thread;
  xvimagesink->event_thread = NULL;
  GST_OBJECT_UNLOCK (xvimagesink);

  /* Wait for our event thread to finish before we clean up our stuff. */
  if (thread)
    g_thread_join (thread);

  if (xvimagesink->cur_image) {
    gst_buffer_unref (xvimagesink->cur_image);
    xvimagesink->cur_image = NULL;
  }

  g_mutex_lock (&xvimagesink->flow_lock);

  if (xvimagesink->pool) {
    gst_object_unref (xvimagesink->pool);
    xvimagesink->pool = NULL;
  }

  if (xvimagesink->xwindow) {
    gst_xwindow_clear (xvimagesink->xwindow);
    gst_xwindow_destroy (xvimagesink->xwindow);
    xvimagesink->xwindow = NULL;
  }
  g_mutex_unlock (&xvimagesink->flow_lock);

  if (xvimagesink->allocator) {
    gst_object_unref (xvimagesink->allocator);
    xvimagesink->allocator = NULL;
  }

  GST_OBJECT_LOCK (xvimagesink);
  /* grab context and mark it as NULL */
  context = xvimagesink->context;
  xvimagesink->context = NULL;
  GST_OBJECT_UNLOCK (xvimagesink);

  if (context)
    gst_xvcontext_unref (context);
}

/* Finalize is called only once, dispose can be called multiple times.
 * We use mutexes and don't reset stuff to NULL here so let's register
 * as a finalize. */
static void
gst_xvimagesink_finalize (GObject * object)
{
  GstXvImageSink *xvimagesink;

  xvimagesink = GST_XVIMAGESINK (object);

  gst_xvimagesink_close (xvimagesink);

  gst_xvcontext_config_clear (&xvimagesink->config);

  if (xvimagesink->par) {
    g_free (xvimagesink->par);
    xvimagesink->par = NULL;
  }
  g_mutex_clear (&xvimagesink->flow_lock);
  g_free (xvimagesink->media_title);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gst_xvimagesink_init (GstXvImageSink * xvimagesink)
{
  xvimagesink->config.display_name = NULL;
  xvimagesink->config.adaptor_nr = 0;
  xvimagesink->config.autopaint_colorkey = TRUE;
  xvimagesink->config.double_buffer = TRUE;
  /* on 16bit displays this becomes r,g,b = 1,2,3
   * on 24bit displays this becomes r,g,b = 8,8,16
   * as a port atom value */
  xvimagesink->config.colorkey = (8 << 16) | (8 << 8) | 16;
  xvimagesink->config.hue = xvimagesink->config.saturation = 0;
  xvimagesink->config.contrast = xvimagesink->config.brightness = 0;
  xvimagesink->config.cb_changed = FALSE;
#ifdef GST_EXT_XV_ENHANCEMENT
  xvimagesink->config.display_mode = DISPLAY_MODE_DEFAULT;
#endif /* GST_EXT_XV_ENHANCEMENT */

  xvimagesink->context = NULL;
  xvimagesink->xwindow = NULL;
  xvimagesink->cur_image = NULL;

  xvimagesink->fps_n = 0;
  xvimagesink->fps_d = 0;
  xvimagesink->video_width = 0;
  xvimagesink->video_height = 0;

  g_mutex_init (&xvimagesink->flow_lock);

  xvimagesink->pool = NULL;

  xvimagesink->synchronous = FALSE;
  xvimagesink->running = FALSE;
  xvimagesink->keep_aspect = TRUE;
  xvimagesink->handle_events = TRUE;
  xvimagesink->par = NULL;
  xvimagesink->handle_expose = TRUE;

  xvimagesink->draw_borders = TRUE;
#ifdef GST_EXT_XV_ENHANCEMENT
  xvimagesink->xid_updated = FALSE;
  xvimagesink->display_geometry_method = DEF_DISPLAY_GEOMETRY_METHOD;
  xvimagesink->flip = DEF_DISPLAY_FLIP;
  xvimagesink->rotate_angle = DEGREE_270;
  xvimagesink->visible = TRUE;
  xvimagesink->zoom = 1.0;
  xvimagesink->zoom_pos_x = -1;
  xvimagesink->zoom_pos_y = -1;
  xvimagesink->dst_roi_mode = DEF_ROI_DISPLAY_GEOMETRY_METHOD;
  xvimagesink->orientation = DEGREE_0;
  xvimagesink->dst_roi.x = 0;
  xvimagesink->dst_roi.y = 0;
  xvimagesink->dst_roi.w = 0;
  xvimagesink->dst_roi.h = 0;

  xvimagesink->is_zero_copy_format = FALSE;
  xvimagesink->is_secure_path = FALSE;
#endif /* GST_EXT_XV_ENHANCEMENT */
}

static void
gst_xvimagesink_class_init (GstXvImageSinkClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;
  GstBaseSinkClass *gstbasesink_class;
  GstVideoSinkClass *videosink_class;

  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass *) klass;
  gstbasesink_class = (GstBaseSinkClass *) klass;
  videosink_class = (GstVideoSinkClass *) klass;

  parent_class = g_type_class_peek_parent (klass);

  gobject_class->set_property = gst_xvimagesink_set_property;
  gobject_class->get_property = gst_xvimagesink_get_property;

  g_object_class_install_property (gobject_class, PROP_CONTRAST,
      g_param_spec_int ("contrast", "Contrast", "The contrast of the video",
          -1000, 1000, 0, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (gobject_class, PROP_BRIGHTNESS,
      g_param_spec_int ("brightness", "Brightness",
          "The brightness of the video", -1000, 1000, 0,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (gobject_class, PROP_HUE,
      g_param_spec_int ("hue", "Hue", "The hue of the video", -1000, 1000, 0,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (gobject_class, PROP_SATURATION,
      g_param_spec_int ("saturation", "Saturation",
          "The saturation of the video", -1000, 1000, 0,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (gobject_class, PROP_DISPLAY,
      g_param_spec_string ("display", "Display", "X Display name", NULL,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (gobject_class, PROP_SYNCHRONOUS,
      g_param_spec_boolean ("synchronous", "Synchronous",
          "When enabled, runs the X display in synchronous mode. "
          "(unrelated to A/V sync, used only for debugging)", FALSE,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (gobject_class, PROP_PIXEL_ASPECT_RATIO,
      g_param_spec_string ("pixel-aspect-ratio", "Pixel Aspect Ratio",
          "The pixel aspect ratio of the device", "1/1",
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (gobject_class, PROP_FORCE_ASPECT_RATIO,
      g_param_spec_boolean ("force-aspect-ratio", "Force aspect ratio",
          "When enabled, scaling will respect original aspect ratio", TRUE,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (gobject_class, PROP_HANDLE_EVENTS,
      g_param_spec_boolean ("handle-events", "Handle XEvents",
          "When enabled, XEvents will be selected and handled", TRUE,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (gobject_class, PROP_DEVICE,
      g_param_spec_string ("device", "Adaptor number",
          "The number of the video adaptor", "0",
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (gobject_class, PROP_DEVICE_NAME,
      g_param_spec_string ("device-name", "Adaptor name",
          "The name of the video adaptor", NULL,
          G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));
  /**
   * GstXvImageSink:handle-expose
   *
   * When enabled, the current frame will always be drawn in response to X
   * Expose.
   */
  g_object_class_install_property (gobject_class, PROP_HANDLE_EXPOSE,
      g_param_spec_boolean ("handle-expose", "Handle expose",
          "When enabled, "
          "the current frame will always be drawn in response to X Expose "
          "events", TRUE, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  /**
   * GstXvImageSink:double-buffer
   *
   * Whether to double-buffer the output.
   */
  g_object_class_install_property (gobject_class, PROP_DOUBLE_BUFFER,
      g_param_spec_boolean ("double-buffer", "Double-buffer",
          "Whether to double-buffer the output", TRUE,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  /**
   * GstXvImageSink:autopaint-colorkey
   *
   * Whether to autofill overlay with colorkey
   */
  g_object_class_install_property (gobject_class, PROP_AUTOPAINT_COLORKEY,
      g_param_spec_boolean ("autopaint-colorkey", "Autofill with colorkey",
          "Whether to autofill overlay with colorkey", TRUE,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  /**
   * GstXvImageSink:colorkey
   *
   * Color to use for the overlay mask.
   */
  g_object_class_install_property (gobject_class, PROP_COLORKEY,
      g_param_spec_int ("colorkey", "Colorkey",
          "Color to use for the overlay mask", G_MININT, G_MAXINT, 0,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  /**
   * GstXvImageSink:draw-borders
   *
   * Draw black borders when using GstXvImageSink:force-aspect-ratio to fill
   * unused parts of the video area.
   */
  g_object_class_install_property (gobject_class, PROP_DRAW_BORDERS,
      g_param_spec_boolean ("draw-borders", "Draw Borders",
          "Draw black borders to fill unused area in force-aspect-ratio mode",
          TRUE, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  /**
   * GstXvImageSink:window-width
   *
   * Actual width of the video window.
   */
  g_object_class_install_property (gobject_class, PROP_WINDOW_WIDTH,
      g_param_spec_uint64 ("window-width", "window-width",
          "Width of the window", 0, G_MAXUINT64, 0,
          G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

  /**
   * GstXvImageSink:window-height
   *
   * Actual height of the video window.
   */
  g_object_class_install_property (gobject_class, PROP_WINDOW_HEIGHT,
      g_param_spec_uint64 ("window-height", "window-height",
          "Height of the window", 0, G_MAXUINT64, 0,
          G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

#ifdef GST_EXT_XV_ENHANCEMENT
  /**
   * GstXvImageSink:display-mode
   *
   * select display mode
   */
  g_object_class_install_property(gobject_class, PROP_DISPLAY_MODE,
    g_param_spec_enum("display-mode", "Display Mode",
      "Display device setting",
      GST_TYPE_XVIMAGESINK_DISPLAY_MODE, DISPLAY_MODE_DEFAULT,
      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  /**
   * GstXvImageSink:display-geometry-method
   *
   * Display geometrical method setting
   */
  g_object_class_install_property(gobject_class, PROP_DISPLAY_GEOMETRY_METHOD,
    g_param_spec_enum("display-geometry-method", "Display geometry method",
      "Geometrical method for display",
      GST_TYPE_XVIMAGESINK_DISPLAY_GEOMETRY_METHOD, DEF_DISPLAY_GEOMETRY_METHOD,
      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  /**
   * GstXvImageSink:display-flip
   *
   * Display flip setting
   */
  g_object_class_install_property(gobject_class, PROP_FLIP,
    g_param_spec_enum("flip", "Display flip",
      "Flip for display",
      GST_TYPE_XVIMAGESINK_FLIP, DEF_DISPLAY_FLIP,
      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  /**
   * GstXvImageSink:rotate
   *
   * Draw rotation angle setting
   */
  g_object_class_install_property(gobject_class, PROP_ROTATE_ANGLE,
    g_param_spec_enum("rotate", "Rotate angle",
      "Rotate angle of display output",
      GST_TYPE_XVIMAGESINK_ROTATE_ANGLE, DEGREE_270,
      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  /**
   * GstXvImageSink:visible
   *
   * Whether reserve original src size or not
   */
  g_object_class_install_property (gobject_class, PROP_VISIBLE,
      g_param_spec_boolean ("visible", "Visible",
          "Draws screen or blacks out, true means visible, false blacks out",
          TRUE, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  /**
   * GstXvImageSink:zoom
   *
   * Scale small area of screen to 1X~ 9
   */
  g_object_class_install_property (gobject_class, PROP_ZOOM,
      g_param_spec_float ("zoom", "Zoom",
          "Zooms screen as nX", 1.0, 9.0, 1.0,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  /**
   * GstXvImageSink:zoom-pos-x
   *
   * Standard x-position of zoom
   */
  g_object_class_install_property (gobject_class, PROP_ZOOM_POS_X,
      g_param_spec_int ("zoom-pos-x", "Zoom Position X",
          "Standard x-position of zoom", 0, 3840, 0,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  /**
   * GstXvImageSink:zoom-pos-y
   *
   * Standard y-position of zoom
   */
  g_object_class_install_property (gobject_class, PROP_ZOOM_POS_Y,
      g_param_spec_int ("zoom-pos-y", "Zoom Position Y",
          "Standard y-position of zoom", 0, 3840, 0,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  /**
   * GstXvImageSink:dst-roi-mode
   *
   * Display geometrical method of ROI setting
   */
  g_object_class_install_property(gobject_class, PROP_DST_ROI_MODE,
    g_param_spec_enum("dst-roi-mode", "Display geometry method of ROI",
      "Geometrical method of ROI for display",
      GST_TYPE_XVIMAGESINK_ROI_DISPLAY_GEOMETRY_METHOD, DEF_ROI_DISPLAY_GEOMETRY_METHOD,
      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  /**
   * GstXvImageSink:orientation
   *
   * Orientation information which will be used for ROI/ZOOM
   */
  g_object_class_install_property(gobject_class, PROP_ORIENTATION,
    g_param_spec_enum("orientation", "Orientation information used for ROI/ZOOM",
      "Orientation information for display",
      GST_TYPE_XVIMAGESINK_ROTATE_ANGLE, DEGREE_0,
      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  /**
   * GstXvImageSink:dst-roi-x
   *
   * X value of Destination ROI
   */
  g_object_class_install_property (gobject_class, PROP_DST_ROI_X,
      g_param_spec_int ("dst-roi-x", "Dst-ROI-X",
          "X value of Destination ROI(only effective \"CUSTOM_ROI\")", 0, XV_SCREEN_SIZE_WIDTH, 0,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  /**
   * GstXvImageSink:dst-roi-y
   *
   * Y value of Destination ROI
   */
  g_object_class_install_property (gobject_class, PROP_DST_ROI_Y,
      g_param_spec_int ("dst-roi-y", "Dst-ROI-Y",
          "Y value of Destination ROI(only effective \"CUSTOM_ROI\")", 0, XV_SCREEN_SIZE_HEIGHT, 0,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  /**
   * GstXvImageSink:dst-roi-w
   *
   * W value of Destination ROI
   */
  g_object_class_install_property (gobject_class, PROP_DST_ROI_W,
      g_param_spec_int ("dst-roi-w", "Dst-ROI-W",
          "W value of Destination ROI(only effective \"CUSTOM_ROI\")", 0, XV_SCREEN_SIZE_WIDTH, 0,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  /**
   * GstXvImageSink:dst-roi-h
   *
   * H value of Destination ROI
   */
  g_object_class_install_property (gobject_class, PROP_DST_ROI_H,
      g_param_spec_int ("dst-roi-h", "Dst-ROI-H",
          "H value of Destination ROI(only effective \"CUSTOM_ROI\")", 0, XV_SCREEN_SIZE_HEIGHT, 0,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
#endif /* GST_EXT_XV_ENHANCEMENT */

  gobject_class->finalize = gst_xvimagesink_finalize;

  gst_element_class_set_static_metadata (gstelement_class,
      "Video sink", "Sink/Video",
      "A Xv based videosink", "Julien Moutte <julien@moutte.net>");

  gst_element_class_add_pad_template (gstelement_class,
      gst_static_pad_template_get (&gst_xvimagesink_sink_template_factory));

  gstelement_class->change_state =
      GST_DEBUG_FUNCPTR (gst_xvimagesink_change_state);

  gstbasesink_class->get_caps = GST_DEBUG_FUNCPTR (gst_xvimagesink_getcaps);
  gstbasesink_class->set_caps = GST_DEBUG_FUNCPTR (gst_xvimagesink_setcaps);
  gstbasesink_class->get_times = GST_DEBUG_FUNCPTR (gst_xvimagesink_get_times);
  gstbasesink_class->propose_allocation =
      GST_DEBUG_FUNCPTR (gst_xvimagesink_propose_allocation);
  gstbasesink_class->event = GST_DEBUG_FUNCPTR (gst_xvimagesink_event);

  videosink_class->show_frame = GST_DEBUG_FUNCPTR (gst_xvimagesink_show_frame);
}
