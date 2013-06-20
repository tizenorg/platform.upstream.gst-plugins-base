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


#ifndef __GST_POLICY_H__
#define __GST_POLICY_H__

#include <gst/gst.h>

G_BEGIN_DECLS

/* #defines don't like whitespacey bits */
#define GST_TYPE_POLICY \
  (gst_policy_get_type())
#define GST_POLICY(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_POLICY,GstPolicy))
#define GST_POLICY_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_POLICY,GstPolicyClass))
#define GST_IS_POLICY(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_POLICY))
#define GST_IS_POLICY_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_POLICY))

typedef struct _GstPolicy      GstPolicy;
typedef struct _GstPolicyClass GstPolicyClass;

/**
 * GstPolicy:
 *
 * Opaque #GstPolicy.
 */
struct _GstPolicy
{
  GstElement element;

  gchar* role;
};

/**
 * GstPolicyClass:
 * @parent_class: the parent class structure.
 *
 * #GstPolicy class.
 */
struct _GstPolicyClass 
{
  GstElementClass parent_class;
};

GType gst_policy_get_type (void);

G_END_DECLS

#endif /* __GST_POLICY_H__ */
