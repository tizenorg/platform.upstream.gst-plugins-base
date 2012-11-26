plugin_LTLIBRARIES = libgstpolicy.la

libgstpolicy_la_SOURCES = \
    gstpolicyplugin.c \
	gstautopolicy.c \
	gstdefaultpolicy.c

nodist_libgstpolicy_la_SOURCES = $(built_sources)
libgstpolicy_la_CFLAGS = $(GST_PLUGINS_BASE_CFLAGS) $(GST_CFLAGS)
libgstpolicy_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)
libgstpolicy_la_LIBADD = \
    $(top_builddir)/gst-libs/gst/policy/libgstpolicy-$(GST_MAJORMINOR).la \
	$(GST_LIBS)
libgstpolicy_la_LIBTOOLFLAGS = --tag=disable-static

noinst_HEADERS = \
	gstautopolicy.h \
	gstdefaultpolicy.h

BUILT_SOURCES = $(built_headers) $(built_sources)

CLEANFILES = $(BUILT_SOURCES)

Android.mk: Makefile.am $(BUILT_SOURCES)
	androgenizer \
	-:PROJECT libgstpolicy -:SHARED libgstpolicy \
	 -:TAGS eng debug \
         -:REL_TOP $(top_srcdir) -:ABS_TOP $(abs_top_srcdir) \
	 -:SOURCES $(libgstpolicy_la_SOURCES) \
	           $(nodist_libgstpolicy_la_SOURCES) \
	 -:CFLAGS $(DEFS) $(DEFAULT_INCLUDES) $(libgstpolicy_la_CFLAGS) $(csp_cflags) \
	 -:LDFLAGS $(libgstpolicy_la_LDFLAGS) \
	           $(libgstpolicy_la_LIBADD) \
	           -ldl \
	 -:PASSTHROUGH LOCAL_ARM_MODE:=arm \
		       LOCAL_MODULE_PATH:='$$(TARGET_OUT)/lib/gstreamer-0.10' \
	> $@
