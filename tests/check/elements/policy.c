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

#include <gst/check/gstcheck.h>
#include <gst/policy/gstpolicy.h>


static void
test_pads (GstElement * policy)
{
  GstPad *sinkpad1;
  GstPad *sinkpad2;

  sinkpad1 = gst_element_get_request_pad (policy, "sink_%u");

  assert_equals_int (policy->numpads, 2);
  assert_equals_int (policy->numsinkpads, 1);
  assert_equals_int (policy->numsrcpads, 1);

  sinkpad2 = gst_element_get_request_pad (policy, "sink_%u");

  assert_equals_int (policy->numpads, 4);
  assert_equals_int (policy->numsinkpads, 2);
  assert_equals_int (policy->numsrcpads, 2);

  gst_element_release_request_pad (policy, sinkpad2);
  g_object_unref (sinkpad2);
  assert_equals_int (policy->numpads, 2);
  assert_equals_int (policy->numsinkpads, 1);
  assert_equals_int (policy->numsrcpads, 1);

  gst_element_release_request_pad (policy, sinkpad1);
  g_object_unref (sinkpad1);
  assert_equals_int (policy->numpads, 0);
  assert_equals_int (policy->numsinkpads, 0);
  assert_equals_int (policy->numsrcpads, 0);
}

GST_START_TEST (test_autopolicy)
{
  GstElement *policy;
  const gchar *role1;
  gchar *role2;

  policy = gst_check_setup_element ("autopolicy");

  test_pads (policy);

  role1 = "megarole";
  gst_child_proxy_set (GST_CHILD_PROXY (policy), "actual-policy::role", role1,
      NULL);
  gst_child_proxy_get (GST_CHILD_PROXY (policy), "actual-policy::role", &role2,
      NULL);
  assert_equals_string (role1, role2);
  g_free (role2);

  gst_check_teardown_element (policy);
}

GST_END_TEST;


GST_START_TEST (test_defaultpolicy)
{
  GstElement *policy;
  const gchar *role1;
  gchar *role2;

  policy = gst_check_setup_element ("defaultpolicy");

  test_pads (policy);

  role1 = "megarole";
  g_object_set (policy, "role", role1, NULL);
  g_object_get (policy, "role", &role2, NULL);
  assert_equals_string (role1, role2);
  g_free (role2);

  gst_check_teardown_element (policy);
}

GST_END_TEST;

static Suite *
policy_suite (void)
{
  Suite *s = suite_create ("policy");
  TCase *tc_chain = tcase_create ("general");

  suite_add_tcase (s, tc_chain);
  tcase_add_test (tc_chain, test_defaultpolicy);
  tcase_add_test (tc_chain, test_autopolicy);

  return s;
}

GST_CHECK_MAIN (policy);
