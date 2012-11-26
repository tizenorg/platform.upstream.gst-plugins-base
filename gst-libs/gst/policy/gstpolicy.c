/* GStreamer
 * Copyright (C) <2012> Intel Corporation
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
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */


/**
 * SECTION:gstpolicy
 * @short_description: Simple base class for platform-specific policy implementations
 * @see_also: autopolicy
 *
 * Any platform-specific policy element should be implemented by subclassing
 * #GstPolicy. Applications should be using #autopolicy element that finds and 
 * and wraps an appropriate policy element, or just #playbin element which 
 * encapsulates autopolicy element.
 *
 * #GstPolicy provides an unlimited number of request sink pads. You can obtain 
 * the corresponding source pad by listening to a "pad-added" signal while 
 * requesting the sink pad.
 * </refsect2>
 */
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gst/gst.h>

#include "gstpolicy.h"

GST_DEBUG_CATEGORY_STATIC (gst_policy_debug);
#define GST_CAT_DEFAULT gst_policy_debug

enum
{
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_ROLE
};

static GstStaticPadTemplate sink_factory = GST_STATIC_PAD_TEMPLATE ("sink_%d",
    GST_PAD_SINK,
    GST_PAD_REQUEST,
    GST_STATIC_CAPS ("ANY")
    );

static GstStaticPadTemplate src_factory = GST_STATIC_PAD_TEMPLATE ("src_%d",
    GST_PAD_SRC,
    GST_PAD_SOMETIMES,
    GST_STATIC_CAPS ("ANY")
    );

GST_BOILERPLATE(GstPolicy, gst_policy, GstElement, GST_TYPE_ELEMENT);

static void gst_policy_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void gst_policy_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);
static void gst_policy_finalize (GObject * object);

static GstPad *gst_policy_request_new_pad (GstElement * element,
    GstPadTemplate * templ, const gchar * name);
static void gst_policy_release_pad (GstElement * element, GstPad * pad);

static gboolean gst_policy_sink_event (GstPad * pad, GstEvent * event);
static gboolean gst_policy_src_event (GstPad * pad, GstEvent * event);
static GstFlowReturn gst_policy_chain (GstPad * pad, GstBuffer * buf);

static GstIterator *gst_policy_iterate_internal_links (GstPad *
    pad, GstObject * parent);

static void
gst_policy_class_init (GstPolicyClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;

  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass *) klass;

  GST_DEBUG_CATEGORY_INIT (gst_policy_debug, "policy", 0, "Policy debugging");

  gobject_class->set_property = gst_policy_set_property;
  gobject_class->get_property = gst_policy_get_property;
  gobject_class->finalize = gst_policy_finalize;

  /**
   * GstPolicy:role
   *
   * Set the role of the stream. This property can be used by the platform policy subsystem
   * to make policy decisions that affect the stream (for example routing and enforced 
   * pause/playback).
   */
  g_object_class_install_property (gobject_class, PROP_ROLE,
      g_param_spec_string ("role", "Stream role",
          "Stream role for the platform policy sybsystem", NULL,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  // element class methods
  gstelement_class->request_new_pad = gst_policy_request_new_pad;
  gstelement_class->release_pad = gst_policy_release_pad;
}

static void
gst_policy_init (GstPolicy * policy, GstPolicyClass *klass)
{
  GST_DEBUG_CATEGORY_INIT (gst_policy_debug, "policy", 0, "Policy");

  policy->role = NULL;
}

static void
gst_policy_base_init (gpointer klass) 
{
  GstElementClass *gstelement_class = GST_ELEMENT_CLASS (klass);

  gst_element_class_add_pad_template (gstelement_class,
      gst_static_pad_template_get (&src_factory));
  gst_element_class_add_pad_template (gstelement_class,
      gst_static_pad_template_get (&sink_factory));

}

static void
gst_policy_finalize (GObject * object)
{
  GstPolicy *policy;

  policy = GST_POLICY (object);

  g_free (policy->role);
  policy->role = NULL;

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gst_policy_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstPolicy *policy = GST_POLICY (object);

  switch (prop_id) {
    case PROP_ROLE:
      g_free (policy->role);
      policy->role = g_strdup (g_value_get_string (value));
      GST_DEBUG_OBJECT (policy, "Setting stream role to %s", policy->role);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_policy_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  GstPolicy *policy = GST_POLICY (object);

  switch (prop_id) {
    case PROP_ROLE:
      GST_DEBUG_OBJECT (policy, "Getting stream role: %s", policy->role);
      g_value_set_string (value, policy->role);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}
static void
gst_policy_release_pad (GstElement * element, GstPad * pad)
{
  GstPolicy *self = GST_POLICY (element);
  GstPad *srcpad = gst_pad_get_element_private (pad);

  gst_pad_set_element_private (pad, NULL);
  gst_pad_set_element_private (srcpad, NULL);

  gst_element_remove_pad (GST_ELEMENT_CAST (self), srcpad);
  gst_element_remove_pad (GST_ELEMENT_CAST (self), pad);
}

static GstIterator *
gst_policy_iterate_internal_links (GstPad * pad, GstObject * parent)
{
  GstIterator *it = NULL;
  GstPad *opad;

  opad = gst_pad_get_element_private (pad);
  if (opad) {
    it = gst_iterator_new_single (GST_TYPE_PAD, (gpointer)opad, 
             (GstCopyFunction)gst_object_ref, 
             (GFreeFunc)gst_object_unref);
  }

  return it;
}

static gboolean
gst_policy_sink_event (GstPad * pad, GstEvent * event)
{
  gboolean ret;

  switch (GST_EVENT_TYPE (event)) {
    default:
      ret = gst_pad_event_default (pad, event);
      break;
  }
  return ret;
}

static gboolean
gst_policy_src_event (GstPad * pad, GstEvent * event)
{
  gboolean ret;

  switch (GST_EVENT_TYPE (event)) {
    default:
      ret = gst_pad_event_default (pad, event);
      break;
  }
  return ret;
}

static GstFlowReturn
gst_policy_chain (GstPad * pad, GstBuffer * buf)
{
  /* just push out the incoming buffer without touching it */
  return gst_pad_push (gst_pad_get_element_private (pad), buf);
}

static GstPad *
gst_policy_request_new_pad (GstElement * element,
    GstPadTemplate * templ, const gchar * name)
{
  GstPad *srcpad;
  GstPad *sinkpad;
  GstPolicy *self = GST_POLICY (element);

  sinkpad = gst_pad_new_from_template (templ, name);
  srcpad = gst_pad_new_from_static_template (&src_factory, NULL);

  gst_pad_set_element_private (sinkpad, srcpad);
  gst_pad_set_element_private (srcpad, sinkpad);

  gst_pad_set_chain_function (sinkpad, 
      GST_DEBUG_FUNCPTR (gst_policy_chain));
  gst_pad_set_iterate_internal_links_function (sinkpad,
      (GstPadIterIntLinkFunction)gst_policy_iterate_internal_links);
  gst_pad_set_event_function (sinkpad, 
      GST_DEBUG_FUNCPTR (gst_policy_sink_event));

  gst_pad_set_iterate_internal_links_function (srcpad,
      (GstPadIterIntLinkFunction)gst_policy_iterate_internal_links);
  gst_pad_set_event_function (srcpad, 
      GST_DEBUG_FUNCPTR (gst_policy_src_event));

  gst_pad_set_active (srcpad, TRUE);
  gst_pad_set_active (sinkpad, TRUE);

  gst_element_add_pad (GST_ELEMENT (self), sinkpad);
  gst_element_add_pad (GST_ELEMENT (self), srcpad);

  return sinkpad;
}

