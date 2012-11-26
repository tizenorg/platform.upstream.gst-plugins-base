libgstpolicyincludedir = $(includedir)/gstreamer-@GST_MAJORMINOR@/gst/policy

libgstpolicyinclude_HEADERS = gstpolicy.h

lib_LTLIBRARIES = libgstpolicy-@GST_MAJORMINOR@.la

libgstpolicy_@GST_MAJORMINOR@_la_SOURCES = gstpolicy.c

libgstpolicy_@GST_MAJORMINOR@_la_CFLAGS = $(GST_PLUGINS_BASE_CFLAGS) $(GST_BASE_CFLAGS) $(GST_CFLAGS) $(GIO_CFLAGS)
libgstpolicy_@GST_MAJORMINOR@_la_LIBADD = $(GST_LIBS) $(GIO_LIBS)
libgstpolicy_@GST_MAJORMINOR@_la_LDFLAGS = $(GST_LIB_LDFLAGS) $(GST_ALL_LDFLAGS) $(GST_LT_LDFLAGS)

if HAVE_INTROSPECTION
BUILT_GIRSOURCES = GstPolicy-@GST_MAJORMINOR@.gir

gir_headers=$(patsubst %,$(srcdir)/%, $(libgstpolicyinclude_HEADERS))
gir_sources=$(patsubst %,$(srcdir)/%, $(libgstpolicy_@GST_MAJORMINOR@_la_SOURCES))
gir_cincludes=$(patsubst %,--c-include='gst/policy/%',$(libgstpolicyinclude_HEADERS))

GstPolicy-@GST_MAJORMINOR@.gir: $(INTROSPECTION_SCANNER) libgstpolicy-@GST_MAJORMINOR@.la
	$(AM_V_GEN)PKG_CONFIG_PATH="$(GST_PKG_CONFIG_PATH)" \
		GST_PLUGIN_SYSTEM_PATH="" GST_PLUGIN_PATH="" GST_REGISTRY_UPDATE=no \
		$(INTROSPECTION_SCANNER) -v --namespace GstPolicy \
		--nsversion=@GST_MAJORMINOR@ \
		--strip-prefix=Gst \
		--warn-all \
		$(gir_cincludes) \
		-I$(top_srcdir)/gst-libs \
		--add-include-path=`PKG_CONFIG_PATH="$(GST_PKG_CONFIG_PATH)" $(PKG_CONFIG) --variable=girdir gstreamer-@GST_MAJORMINOR@` \
		--library=libgstpolicy-@GST_MAJORMINOR@.la \
		--include=Gst-@GST_MAJORMINOR@ \
		--libtool="$(top_builddir)/libtool" \
		--pkg gstreamer-@GST_MAJORMINOR@ \
		--pkg-export gstreamer-policy-@GST_MAJORMINOR@ \
        --add-init-section="gst_init(NULL,NULL);" \
		--output $@ \
		$(gir_headers) \
		$(gir_sources)

# INTROSPECTION_GIRDIR/INTROSPECTION_TYPELIBDIR aren't the right place to
# install anything - we need to install inside our prefix.
girdir = $(datadir)/gir-1.0
gir_DATA = $(BUILT_GIRSOURCES)

typelibsdir = $(libdir)/girepository-1.0/

typelibs_DATA = $(BUILT_GIRSOURCES:.gir=.typelib)

%.typelib: %.gir $(INTROSPECTION_COMPILER)
	$(AM_V_GEN)PKG_CONFIG_PATH="$(GST_PKG_CONFIG_PATH)" \
		$(INTROSPECTION_COMPILER) \
		--includedir=$(srcdir) \
		--includedir=$(builddir) \
		--includedir=`PKG_CONFIG_PATH="$(GST_PKG_CONFIG_PATH)" $(PKG_CONFIG) --variable=girdir gstreamer-@GST_MAJORMINOR@` \
		$(INTROSPECTION_COMPILER_OPTS) $< -o $(@F)

CLEANFILES = $(BUILT_GIRSOURCES) $(typelibs_DATA)
endif

Android.mk: Makefile.am $(BUILT_SOURCES)
	androgenizer -:PROJECT libgstpolicy -:SHARED libgstpolicy-@GST_MAJORMINOR@ \
	 -:TAGS eng debug \
         -:REL_TOP $(top_srcdir) -:ABS_TOP $(abs_top_srcdir) \
	 -:SOURCES $(libgstpolicy_@GST_MAJORMINOR@_la_SOURCES) \
	 -:CFLAGS $(DEFS) $(DEFAULT_INCLUDES) $(libgstpolicy_@GST_MAJORMINOR@_la_CFLAGS) \
	 -:LDFLAGS $(libgstpolicy_@GST_MAJORMINOR@_la_LDFLAGS) \
	           $(libgstpolicy_@GST_MAJORMINOR@_la_LIBADD) \
	           -ldl \
	 -:HEADER_TARGET gstreamer-@GST_MAJORMINOR@/gst/policy \
	 -:HEADERS $(libgstpolicyinclude_HEADERS) \
	 -:PASSTHROUGH LOCAL_ARM_MODE:=arm \
	> $@
