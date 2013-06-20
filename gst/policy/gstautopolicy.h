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



#ifndef __GST_AUTO_POLICY_H__
#define __GST_AUTO_POLICY_H__

#include <gst/gst.h>

G_BEGIN_DECLS

#define GST_TYPE_AUTO_POLICY \
  (gst_auto_policy_get_type())
#define GST_AUTO_POLICY(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_AUTO_POLICY,GstAutoPolicy))
#define GST_AUTO_POLICY_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_AUTO_POLICY,GstAutoPolicyClass))
#define GST_IS_AUTO_POLICY(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_AUTO_POLICY))
#define GST_IS_AUTO_POLICY_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_AUTO_POLICY))

typedef struct _GstAutoPolicy      GstAutoPolicy;
typedef struct _GstAutoPolicyClass GstAutoPolicyClass;

struct _GstAutoPolicy
{
  GstBin parent;
  
  GstElement *kid;
};

struct _GstAutoPolicyClass 
{
  GstBinClass parent_class;
};

GType gst_auto_policy_get_type (void);

G_END_DECLS

#endif /* __GST_POLICY_H__ */
