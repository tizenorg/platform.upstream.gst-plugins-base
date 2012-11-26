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


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/gst.h>
#include "gstdefaultpolicy.h"
#include "gstautopolicy.h"
#include "gstpolicyplugin.h"

GST_DEBUG_CATEGORY (gst_policy_debug);
#define GST_CAT_DEFAULT gst_policy_debug

static gboolean
policy_init (GstPlugin * policy)
{
  GST_DEBUG_CATEGORY_INIT (gst_policy_debug, "policy", 0, "Policy debugging");

  return gst_element_register (policy, "defaultpolicy", GST_RANK_MARGINAL,
      GST_TYPE_DEFAULT_POLICY) &&
      gst_element_register (policy, "autopolicy", GST_RANK_NONE,
      GST_TYPE_AUTO_POLICY);
}

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    "policy",
    "various policy elements", policy_init, VERSION, GST_LICENSE,
    GST_PACKAGE_NAME, GST_PACKAGE_ORIGIN)
