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


#ifndef __GST_DEFAULT_POLICY_H__
#define __GST_DEFAULT_POLICY_H__

#include <gst/gst.h>
#include "gst/policy/gstpolicy.h"

G_BEGIN_DECLS

#define GST_TYPE_DEFAULT_POLICY \
  (gst_default_policy_get_type())
#define GST_DEFAULT_POLICY(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_DEFAULT_POLICY,GstDefaultPolicy))
#define GST_DEFAULT_POLICY_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_DEFAULT_POLICY,GstDefaultPolicyClass))
#define GST_IS_DEFAULT_POLICY(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_DEFAULT_POLICY))
#define GST_IS_DEFAULT_POLICY_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_DEFAULT_POLICY))

typedef struct _GstDefaultPolicy      GstDefaultPolicy;
typedef struct _GstDefaultPolicyClass GstDefaultPolicyClass;

struct _GstDefaultPolicy
{
  GstPolicy policy;

};

struct _GstDefaultPolicyClass 
{
  GstPolicyClass parent_class;
};

GType gst_default_policy_get_type (void);

G_END_DECLS

#endif /* __GST_DEFAULT_POLICY_H__ */
