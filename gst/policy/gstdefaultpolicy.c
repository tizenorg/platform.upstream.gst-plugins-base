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
 * SECTION:element-defaultpolicy
 * @see_also: autopolicy
 *
 * Defaultpolicy implements a default policy, which does nothing at all with the
 * stream. It's the element that autopolicy element selects in the absence of any
 * other policy element.
 *
 * <refsect2>
 * <title>Example launch line (with policy-specific debugging)</title>
 * |[
 * gst-launch -v -m --gst-debug=policy:5 fakesrc ! defaultpolicy role=testrole ! fakesink silent=TRUE
 * ]|
 * </refsect2>
 * <refsect2>
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gst/gst.h>

#include "gstdefaultpolicy.h"

#define gst_default_policy_parent_class parent_class
G_DEFINE_TYPE (GstDefaultPolicy, gst_default_policy, GST_TYPE_POLICY);

static void
gst_default_policy_class_init (GstDefaultPolicyClass * klass)
{
  GstElementClass *gstelement_class;
  const GstElementDetails details = {
      (gchar *)"Default policy", 
      (gchar *)"Policy",
      (gchar *)"Default policy element that does nothing",
      (gchar *)"Alexander Kanavin <alexander.kanavin@intel.com>"
  };

  gstelement_class = (GstElementClass *) klass;

  gst_element_class_set_details (gstelement_class, &details);
}

static void
gst_default_policy_init (GstDefaultPolicy * policy)
{
}
