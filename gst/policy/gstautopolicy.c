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
 * SECTION:element-autopolicy
 * @see_also: defaultpolicy
 *
 * Autopolicy is a wrapper element for platform-specific policy elements.
 * It automatically detects an appropriate policy element to use by scanning 
 * the registry for all elements that have <quote>Policy</quote> in the class 
 * field of their element information. The list is sorted by rank and only 
 * elements with a non-zero autoplugging rank are included.
 *
 * <refsect2>
 * <title>Example launch line (with policy-specific debugging)</title>
 * |[
 * gst-launch -v -m --gst-debug=policy:5 fakesrc ! autopolicy actual-policy::role=testrole ! fakesink silent=TRUE
 * ]|
 * </refsect2>
 * <refsect2>
 * <title>Usage</title>
 * Autopolicy provides an unlimited number of request sink pads. You can obtain 
 * the corresponding source pad by listening to a "pad-added" signal while 
 * requesting the sink pad.
 * 
 * You can also get and set the stream role property #GstPolicy:role by using 
 * #GstChildProxy interface that autopolicy implements. The name of the role 
 * property is "actual-policy::role".
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gst/gst.h>
#include <string.h>

#include "gstautopolicy.h"
#include "gstpolicyplugin.h"

GST_DEBUG_CATEGORY_EXTERN (gst_policy_debug);
#define GST_CAT_DEFAULT gst_policy_debug

enum
{
  LAST_SIGNAL
};

enum
{
  PROP_0
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

#define gst_auto_policy_parent_class parent_class
G_DEFINE_TYPE (GstAutoPolicy, gst_auto_policy, GST_TYPE_BIN);

static GstPad *
gst_auto_policy_request_new_pad (GstElement * element,
    GstPadTemplate * templ, const gchar * name/*, const GstCaps * caps*/)
{
  GstPad *srcpad = NULL;
  GstPad *sinkpad = NULL;
  GstPad *kid_sink_pad = NULL;
  GstPad *kid_src_pad = NULL;
  GstIterator *it = NULL;
  GstAutoPolicy *self = GST_AUTO_POLICY (element);

  kid_sink_pad = gst_element_get_request_pad (self->kid, "sink_%d");
  it = gst_pad_iterate_internal_links (kid_sink_pad);
  if (!it) {
      gst_object_unref(kid_sink_pad);
      return NULL;
  }
  gst_iterator_next (it, (gpointer *)&kid_src_pad);
  gst_iterator_free (it);

  sinkpad = gst_ghost_pad_new (name, kid_sink_pad);
  srcpad = gst_ghost_pad_new (NULL, kid_src_pad);

  gst_pad_set_element_private (sinkpad, srcpad);
  gst_pad_set_element_private (srcpad, sinkpad);

  gst_pad_set_active (srcpad, TRUE);
  gst_pad_set_active (sinkpad, TRUE);

  gst_element_add_pad (GST_ELEMENT (self), sinkpad);
  gst_element_add_pad (GST_ELEMENT (self), srcpad);

  gst_object_unref (kid_sink_pad);
  gst_object_unref (kid_src_pad);

  return sinkpad;
}

static void
gst_auto_policy_release_pad (GstElement * element, GstPad * pad)
{
  GstAutoPolicy *self = GST_AUTO_POLICY (element);
  GstPad *srcpad = gst_pad_get_element_private (pad);

  gst_pad_set_element_private (pad, NULL);
  gst_pad_set_element_private (srcpad, NULL);

  gst_element_remove_pad (GST_ELEMENT_CAST (self), srcpad);
  gst_element_remove_pad (GST_ELEMENT_CAST (self), pad);
}

static void
gst_auto_policy_dispose (GstAutoPolicy * auto_policy)
{
  gst_element_set_state (auto_policy->kid, GST_STATE_NULL);
  gst_bin_remove (GST_BIN (auto_policy), auto_policy->kid);
  auto_policy->kid = NULL;

  G_OBJECT_CLASS (parent_class)->dispose ((GObject *) auto_policy);
}

/* initialize the policy's class */
static void
gst_auto_policy_class_init (GstAutoPolicyClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;
  const GstElementDetails details = { 
      (gchar *)"Auto policy", (gchar *)"Policy",
      (gchar *)"Wrapper element for automatically detected policy element",
      (gchar *)"Alexander Kanavin <alexander.kanavin@intel.com>" 
  };

  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass *) klass;

  gobject_class->dispose = (GObjectFinalizeFunc) gst_auto_policy_dispose;

  gst_element_class_set_details(gstelement_class, &details);

  gst_element_class_add_pad_template (gstelement_class,
      gst_static_pad_template_get (&src_factory));
  gst_element_class_add_pad_template (gstelement_class,
      gst_static_pad_template_get (&sink_factory));

  gstelement_class->request_new_pad = gst_auto_policy_request_new_pad;
  gstelement_class->release_pad = gst_auto_policy_release_pad;
}

static gboolean
gst_auto_policy_factory_filter (GstPluginFeature * feature, gpointer data)
{
  guint rank;
  const gchar *klass;

  /* we only care about element factories */
  if (!GST_IS_ELEMENT_FACTORY (feature))
    return FALSE;

  /* policy elements */
  klass = gst_element_factory_get_klass (GST_ELEMENT_FACTORY (feature));
  if (strstr (klass, "Policy") == NULL)
    return FALSE;

  /* only select elements with autoplugging rank */
  rank = gst_plugin_feature_get_rank (feature);
  if (rank < GST_RANK_MARGINAL)
    return FALSE;

  return TRUE;
}

static gint
gst_auto_policy_compare_ranks (GstPluginFeature * f1, GstPluginFeature * f2)
{
  return gst_plugin_feature_get_rank (f2) - gst_plugin_feature_get_rank (f1);
}

static GstElement *
gst_auto_policy_discover_actual_policy (GstAutoPolicy * auto_policy)
{
  GList *list, *item;
  GstElement *choice = NULL;

  list = gst_registry_feature_filter (gst_registry_get_default (),
      (GstPluginFeatureFilter) gst_auto_policy_factory_filter, FALSE,
      auto_policy);
  list = g_list_sort (list, (GCompareFunc) gst_auto_policy_compare_ranks);

  for (item = list; item != NULL; item = item->next) {
    GstElementFactory *f = GST_ELEMENT_FACTORY (item->data);
    GST_DEBUG_OBJECT (auto_policy, "Testing %s", GST_OBJECT_NAME (f));
    choice = gst_element_factory_create (f, "actual-policy");
    if (choice) {
      GST_DEBUG_OBJECT (auto_policy, "Selected %s", GST_OBJECT_NAME (f));
      break;
    }
  }
  gst_plugin_feature_list_free (list);
  return choice;
}

static void
gst_auto_policy_init (GstAutoPolicy * auto_policy)
{
  auto_policy->kid = gst_auto_policy_discover_actual_policy (auto_policy);
  //At least default policy should always be available
  g_assert (auto_policy->kid);
  gst_bin_add (GST_BIN (auto_policy), auto_policy->kid);

}
