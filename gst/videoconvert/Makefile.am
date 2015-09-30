plugin_LTLIBRARIES = libgstvideoconvert.la

libgstvideoconvert_la_SOURCES = gstvideoconvert.c
nodist_libgstvideoconvert_la_SOURCES = $(ORC_NODIST_SOURCES)
libgstvideoconvert_la_CFLAGS = \
	$(GST_PLUGINS_BASE_CFLAGS) \
	$(GST_CFLAGS)
libgstvideoconvert_la_LIBADD = \
	$(top_builddir)/gst-libs/gst/video/libgstvideo-$(GST_API_VERSION).la \
	$(GST_BASE_LIBS) \
	$(GST_LIBS) \
	$(LIBM)
libgstvideoconvert_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)
libgstvideoconvert_la_LIBTOOLFLAGS = $(GST_PLUGIN_LIBTOOLFLAGS)

noinst_HEADERS = gstvideoconvert.h

if BOARD_USE_TBM_BUF
libgstvideoconvert_la_SOURCES += gsttbmbufferpool.c
libgstvideoconvert_la_CFLAGS += -DUSE_TBM_BUFFER $(MMCOMMON_CFLAGS) $(TBM_CFLAGS)
libgstvideoconvert_la_LIBADD += $(TBM_LIBS) $(MM_COMMON_LIBS)
libgstvideoconvert_la_LDFLAGS += $(TBM_LDFLAGS) $(MM_COMMON_LDFLAGS)
noinst_HEADERS += gsttbmbufferpool.h
endif

