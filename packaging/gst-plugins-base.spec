Name:           gst-plugins-base
Version:        1.0.0
Release:        2.3
%define gst_branch 1.0
Url:            http://gstreamer.freedesktop.org/
Summary:        GStreamer Streaming-Media Framework Plug-Ins
License:        LGPL-2.1+ and GPL-2.0+
Group:          Productivity/Multimedia/Other
Source:         http://download.gnome.org/sources/gst-plugins-base/1.0/%{name}-%{version}.tar.xz
Source2:        baselibs.conf
BuildRequireS:  gstreamer-utils > 0.11
BuildRequires:  glib2-devel >= 2.32
BuildRequires:  gstreamer-devel >= 1.0.0
BuildRequires:  gtk-doc >= 1.12
BuildRequires:  libICE-devel
BuildRequires:  libSM-devel
BuildRequires:  libXext-devel
BuildRequires:  libXv-devel
BuildRequires:  orc >= 0.4.16
BuildRequires:  python-base
BuildRequires:  update-desktop-files
BuildRequires:  gettext-tools
BuildRequires:  gobject-introspection-devel >= 1.31.1
BuildRequires:  pkgconfig(alsa) >= 0.9.1
BuildRequires:  pkgconfig(freetype2) >= 2.0.9
BuildRequires:  pkgconfig(iso-codes)
BuildRequires:  pkgconfig(libxml-2.0)
BuildRequires:  pkgconfig(ogg) >= 1.0
BuildRequires:  pkgconfig(pango) >= 1.22.0
BuildRequires:  pkgconfig(pangocairo) >= 1.22.0
BuildRequires:  pkgconfig(theoradec) >= 1.1
BuildRequires:  pkgconfig(theoraenc) >= 1.1
BuildRequires:  pkgconfig(vorbis) >= 1.0
BuildRequires:  pkgconfig(vorbisenc) >= 1.0
BuildRequires:  pkgconfig(zlib)
Requires:       gstreamer >= 1.0.0
Supplements:    gstreamer

%description
GStreamer is a streaming media framework based on graphs of filters
that operate on media data. Applications using this library can do
anything media-related, from real-time sound processing to playing
videos. Its plug-in-based architecture means that new data types or
processing capabilities can be added simply by installing new plug-ins.

%package -n libgstapp
Summary:        GStreamer Streaming-Media Framework Plug-Ins
Group:          System/Libraries
# We want to have base modules installed:
Requires:       %{name}

%description -n libgstapp
GStreamer is a streaming media framework based on graphs of filters
that operate on media data. Applications using this library can do
anything media-related, from real-time sound processing to playing
videos. Its plug-in-based architecture means that new data types or
processing capabilities can be added simply by installing new plug-ins.

%package -n typelib-1_0-GstApp-1_0
Summary:        GStreamer Streaming-Media Framework Plug-Ins -- Introspection bindings
Group:          System/Libraries

%description -n typelib-1_0-GstApp-1_0
GStreamer is a streaming media framework based on graphs of filters
that operate on media data. Applications using this library can do
anything media-related, from real-time sound processing to playing
videos. Its plug-in-based architecture means that new data types or
processing capabilities can be added simply by installing new plug-ins.

This package provides the GObject Introspection bindings for GStreamer
plug-ins.

%package -n libgstaudio
Summary:        GStreamer Streaming-Media Framework Plug-Ins
Group:          System/Libraries
# We want to have base modules installed:
Requires:       %{name}

%description -n libgstaudio
GStreamer is a streaming media framework based on graphs of filters
that operate on media data. Applications using this library can do
anything media-related, from real-time sound processing to playing
videos. Its plug-in-based architecture means that new data types or
processing capabilities can be added simply by installing new plug-ins.

%package -n typelib-1_0-GstAudio-1_0
Summary:        GStreamer Streaming-Media Framework Plug-Ins -- Introspection bindings
Group:          System/Libraries

%description -n typelib-1_0-GstAudio-1_0
GStreamer is a streaming media framework based on graphs of filters
that operate on media data. Applications using this library can do
anything media-related, from real-time sound processing to playing
videos. Its plug-in-based architecture means that new data types or
processing capabilities can be added simply by installing new plug-ins.

This package provides the GObject Introspection bindings for GStreamer
plug-ins.

%package -n libgstfft
Summary:        GStreamer Streaming-Media Framework Plug-Ins
Group:          System/Libraries
# We want to have base modules installed:
Requires:       %{name}

%description -n libgstfft
GStreamer is a streaming media framework based on graphs of filters
that operate on media data. Applications using this library can do
anything media-related, from real-time sound processing to playing
videos. Its plug-in-based architecture means that new data types or
processing capabilities can be added simply by installing new plug-ins.

%package -n typelib-1_0-GstFft-1_0
Summary:        GStreamer Streaming-Media Framework Plug-Ins -- Introspection bindings
Group:          System/Libraries

%description -n typelib-1_0-GstFft-1_0
GStreamer is a streaming media framework based on graphs of filters
that operate on media data. Applications using this library can do
anything media-related, from real-time sound processing to playing
videos. Its plug-in-based architecture means that new data types or
processing capabilities can be added simply by installing new plug-ins.

This package provides the GObject Introspection bindings for GStreamer
plug-ins.

%package -n libgstpbutils
Summary:        GStreamer Streaming-Media Framework Plug-Ins
Group:          System/Libraries
# We want to have base modules installed:
Requires:       %{name}

%description -n libgstpbutils
GStreamer is a streaming media framework based on graphs of filters
that operate on media data. Applications using this library can do
anything media-related, from real-time sound processing to playing
videos. Its plug-in-based architecture means that new data types or
processing capabilities can be added simply by installing new plug-ins.

%package -n typelib-1_0-GstPbutils-1_0
Summary:        GStreamer Streaming-Media Framework Plug-Ins -- Introspection bindings
Group:          System/Libraries

%description -n typelib-1_0-GstPbutils-1_0
GStreamer is a streaming media framework based on graphs of filters
that operate on media data. Applications using this library can do
anything media-related, from real-time sound processing to playing
videos. Its plug-in-based architecture means that new data types or
processing capabilities can be added simply by installing new plug-ins.

This package provides the GObject Introspection bindings for GStreamer
plug-ins.

%package -n libgstriff
Summary:        GStreamer Streaming-Media Framework Plug-Ins
Group:          System/Libraries
# We want to have base modules installed:
Requires:       %{name}

%description -n libgstriff
GStreamer is a streaming media framework based on graphs of filters
that operate on media data. Applications using this library can do
anything media-related, from real-time sound processing to playing
videos. Its plug-in-based architecture means that new data types or
processing capabilities can be added simply by installing new plug-ins.

%package -n typelib-1_0-GstRiff-1_0
Summary:        GStreamer Streaming-Media Framework Plug-Ins -- Introspection bindings
Group:          System/Libraries

%description -n typelib-1_0-GstRiff-1_0
GStreamer is a streaming media framework based on graphs of filters
that operate on media data. Applications using this library can do
anything media-related, from real-time sound processing to playing
videos. Its plug-in-based architecture means that new data types or
processing capabilities can be added simply by installing new plug-ins.

This package provides the GObject Introspection bindings for GStreamer
plug-ins.

%package -n libgstrtp
Summary:        GStreamer Streaming-Media Framework Plug-Ins
Group:          System/Libraries
# We want to have base modules installed:
Requires:       %{name}

%description -n libgstrtp
GStreamer is a streaming media framework based on graphs of filters
that operate on media data. Applications using this library can do
anything media-related, from real-time sound processing to playing
videos. Its plug-in-based architecture means that new data types or
processing capabilities can be added simply by installing new plug-ins.

%package -n typelib-1_0-GstRtp-1_0
Summary:        GStreamer Streaming-Media Framework Plug-Ins -- Introspection bindings
Group:          System/Libraries

%description -n typelib-1_0-GstRtp-1_0
GStreamer is a streaming media framework based on graphs of filters
that operate on media data. Applications using this library can do
anything media-related, from real-time sound processing to playing
videos. Its plug-in-based architecture means that new data types or
processing capabilities can be added simply by installing new plug-ins.

This package provides the GObject Introspection bindings for GStreamer
plug-ins.

%package -n libgstrtsp
Summary:        GStreamer Streaming-Media Framework Plug-Ins
Group:          System/Libraries
# We want to have base modules installed:
Requires:       %{name}

%description -n libgstrtsp
GStreamer is a streaming media framework based on graphs of filters
that operate on media data. Applications using this library can do
anything media-related, from real-time sound processing to playing
videos. Its plug-in-based architecture means that new data types or
processing capabilities can be added simply by installing new plug-ins.

%package -n typelib-1_0-GstRtsp-1_0
Summary:        GStreamer Streaming-Media Framework Plug-Ins -- Introspection bindings
Group:          System/Libraries

%description -n typelib-1_0-GstRtsp-1_0
GStreamer is a streaming media framework based on graphs of filters
that operate on media data. Applications using this library can do
anything media-related, from real-time sound processing to playing
videos. Its plug-in-based architecture means that new data types or
processing capabilities can be added simply by installing new plug-ins.

This package provides the GObject Introspection bindings for GStreamer
plug-ins.

%package -n libgstsdp
Summary:        GStreamer Streaming-Media Framework Plug-Ins
Group:          System/Libraries
# We want to have base modules installed:
Requires:       %{name}

%description -n libgstsdp
GStreamer is a streaming media framework based on graphs of filters
that operate on media data. Applications using this library can do
anything media-related, from real-time sound processing to playing
videos. Its plug-in-based architecture means that new data types or
processing capabilities can be added simply by installing new plug-ins.

%package -n typelib-1_0-GstSdp-1_0
Summary:        GStreamer Streaming-Media Framework Plug-Ins -- Introspection bindings
Group:          System/Libraries

%description -n typelib-1_0-GstSdp-1_0
GStreamer is a streaming media framework based on graphs of filters
that operate on media data. Applications using this library can do
anything media-related, from real-time sound processing to playing
videos. Its plug-in-based architecture means that new data types or
processing capabilities can be added simply by installing new plug-ins.

This package provides the GObject Introspection bindings for GStreamer
plug-ins.

%package -n libgsttag
Summary:        GStreamer Streaming-Media Framework Plug-Ins
Group:          System/Libraries
# We want to have base modules installed:
Requires:       %{name}

%description -n libgsttag
GStreamer is a streaming media framework based on graphs of filters
that operate on media data. Applications using this library can do
anything media-related, from real-time sound processing to playing
videos. Its plug-in-based architecture means that new data types or
processing capabilities can be added simply by installing new plug-ins.

%package -n typelib-1_0-GstTag-1_0
Summary:        GStreamer Streaming-Media Framework Plug-Ins -- Introspection bindings
Group:          System/Libraries

%description -n typelib-1_0-GstTag-1_0
GStreamer is a streaming media framework based on graphs of filters
that operate on media data. Applications using this library can do
anything media-related, from real-time sound processing to playing
videos. Its plug-in-based architecture means that new data types or
processing capabilities can be added simply by installing new plug-ins.

This package provides the GObject Introspection bindings for GStreamer
plug-ins.

%package -n libgstvideo
Summary:        GStreamer Streaming-Media Framework Plug-Ins
Group:          System/Libraries
# We want to have base modules installed:
Requires:       %{name}

%description -n libgstvideo
GStreamer is a streaming media framework based on graphs of filters
that operate on media data. Applications using this library can do
anything media-related, from real-time sound processing to playing
videos. Its plug-in-based architecture means that new data types or
processing capabilities can be added simply by installing new plug-ins.

%package -n typelib-1_0-GstVideo-1_0
Summary:        GStreamer Streaming-Media Framework Plug-Ins -- Introspection bindings
Group:          System/Libraries

%description -n typelib-1_0-GstVideo-1_0
GStreamer is a streaming media framework based on graphs of filters
that operate on media data. Applications using this library can do
anything media-related, from real-time sound processing to playing
videos. Its plug-in-based architecture means that new data types or
processing capabilities can be added simply by installing new plug-ins.

This package provides the GObject Introspection bindings for GStreamer
plug-ins.

%package devel
Summary:        Include files and libraries mandatory for development with gstreamer-plugins-base
Group:          Development/Libraries/C and C++
Requires:       libgstapp = %{version}
Requires:       libgstaudio = %{version}
Requires:       libgstfft = %{version}
Requires:       libgstpbutils = %{version}
Requires:       libgstriff = %{version}
Requires:       libgstrtp = %{version}
Requires:       libgstrtsp = %{version}
Requires:       libgstsdp = %{version}
Requires:       libgsttag = %{version}
Requires:       libgstvideo = %{version}
Requires:       typelib-1_0-GstApp-1_0 = %{version}
Requires:       typelib-1_0-GstAudio-1_0 = %{version}
Requires:       typelib-1_0-GstFft-1_0 = %{version}
Requires:       typelib-1_0-GstPbutils-1_0 = %{version}
Requires:       typelib-1_0-GstRiff-1_0 = %{version}
Requires:       typelib-1_0-GstRtp-1_0 = %{version}
Requires:       typelib-1_0-GstRtsp-1_0 = %{version}
Requires:       typelib-1_0-GstSdp-1_0 = %{version}
Requires:       typelib-1_0-GstTag-1_0 = %{version}
Requires:       typelib-1_0-GstVideo-1_0 = %{version}
# Generic name, never used in SuSE:
Provides:       gst-plugins-base-devel = %{version}

%description devel
This package contains all necessary include files and libraries needed
to compile and link applications that use gstreamer-plugins-base.

%package doc
Summary:        Documentation for gstreamer-plugins-base
Group:          Development/Libraries/C and C++
Requires:       %{name} = %{version}
# Generic name, never used in SuSE:
Provides:       gst-plugins-base-doc = %{version}

%description doc
This package contains documentation for the gstreamer-plugins-base
package.

%lang_package
%prep
%setup -q -n %{name}-%{version}

%build
# FIXME: GTKDOC_CFLAGS, GST_OBJ_CFLAGS:
# Silently ignored compilation of uninstalled gtk-doc scanners without RPM_OPT_FLAGS.
export CFLAGS="$RPM_OPT_FLAGS -fno-strict-aliasing"
%configure\
	--disable-static\
	--enable-experimental\
	--disable-gtk-doc\
	--disable-examples\
        --enable-introspection\
	%{nil}
make %{?jobs:-j%jobs}

%install
%make_install
%find_lang %{name}-%{gst_branch}
mv %{name}-%{gst_branch}.lang %{name}.lang

%clean
rm -rf $RPM_BUILD_ROOT

%post -n libgstapp -p /sbin/ldconfig

%postun -n libgstapp -p /sbin/ldconfig

%post -n libgstaudio -p /sbin/ldconfig

%postun -n libgstaudio -p /sbin/ldconfig

%post -n libgstfft -p /sbin/ldconfig

%postun -n libgstfft -p /sbin/ldconfig

%post -n libgstpbutils -p /sbin/ldconfig

%postun -n libgstpbutils -p /sbin/ldconfig

%post -n libgstriff -p /sbin/ldconfig

%postun -n libgstriff -p /sbin/ldconfig

%post -n libgstrtp -p /sbin/ldconfig

%postun -n libgstrtp -p /sbin/ldconfig

%post -n libgstrtsp -p /sbin/ldconfig

%postun -n libgstrtsp -p /sbin/ldconfig

%post -n libgstsdp -p /sbin/ldconfig

%postun -n libgstsdp -p /sbin/ldconfig

%post -n libgsttag -p /sbin/ldconfig

%postun -n libgsttag -p /sbin/ldconfig

%post -n libgstvideo -p /sbin/ldconfig

%postun -n libgstvideo -p /sbin/ldconfig

%files
%defattr(-, root, root)
%doc AUTHORS COPYING COPYING.LIB NEWS README RELEASE REQUIREMENTS
%{_bindir}/gst-discoverer-%{gst_branch}
%{_libdir}/gstreamer-%{gst_branch}/libgstadder.so
%{_libdir}/gstreamer-%{gst_branch}/libgstalsa.so
%{_libdir}/gstreamer-%{gst_branch}/libgstapp.so
%{_libdir}/gstreamer-%{gst_branch}/libgstaudioconvert.so
%{_libdir}/gstreamer-%{gst_branch}/libgstaudioresample.so
%{_libdir}/gstreamer-%{gst_branch}/libgstaudiotestsrc.so
%{_libdir}/gstreamer-%{gst_branch}/libgstaudiorate.so
#%{_libdir}/gstreamer-%{gst_branch}/libgstcdparanoia.so
#%{_libdir}/gstreamer-%{gst_branch}/libgstdecodebin.so
#%{_libdir}/gstreamer-%{gst_branch}/libgstdecodebin2.so
#%{_libdir}/gstreamer-%{gst_branch}/libgstffmpegcolorspace.so
%{_libdir}/gstreamer-%{gst_branch}/libgstgio.so
#%{_libdir}/gstreamer-%{gst_branch}/libgstlibvisual.so
%{_libdir}/gstreamer-%{gst_branch}/libgstogg.so
%{_libdir}/gstreamer-%{gst_branch}/libgstpango.so
%{_libdir}/gstreamer-%{gst_branch}/libgstplayback.so
%{_libdir}/gstreamer-%{gst_branch}/libgstsubparse.so
%{_libdir}/gstreamer-%{gst_branch}/libgsttcp.so
%{_libdir}/gstreamer-%{gst_branch}/libgsttheora.so
%{_libdir}/gstreamer-%{gst_branch}/libgsttypefindfunctions.so
%{_libdir}/gstreamer-%{gst_branch}/libgstvideoconvert.so
%{_libdir}/gstreamer-%{gst_branch}/libgstvideorate.so
%{_libdir}/gstreamer-%{gst_branch}/libgstvideoscale.so
%{_libdir}/gstreamer-%{gst_branch}/libgstvideotestsrc.so
%{_libdir}/gstreamer-%{gst_branch}/libgstvolume.so
%{_libdir}/gstreamer-%{gst_branch}/libgstvorbis.so
%{_libdir}/gstreamer-%{gst_branch}/libgstximagesink.so
%{_libdir}/gstreamer-%{gst_branch}/libgstxvimagesink.so
#Moved to -bad for now... likely to come pack later
#%{_libdir}/gstreamer-%{gst_branch}/libgstgdp.so
%{_libdir}/gstreamer-%{gst_branch}/libgstencodebin.so
%doc %{_mandir}/man1/gst-discoverer-*

%files -n libgstapp
%defattr(-, root, root)
%{_libdir}/libgstapp*.so.*

%files -n typelib-1_0-GstApp-1_0
%defattr(-, root, root)
%{_libdir}/girepository-1.0/GstApp-*.typelib

%files -n libgstaudio
%defattr(-, root, root)
%{_libdir}/libgstaudio*.so.*

%files -n typelib-1_0-GstAudio-1_0
%defattr(-, root, root)
%{_libdir}/girepository-1.0/GstAudio-*.typelib

%files -n libgstfft
%defattr(-, root, root)
%{_libdir}/libgstfft*.so.*

%files -n typelib-1_0-GstFft-1_0
%defattr(-, root, root)
%{_libdir}/girepository-1.0/GstFft-*.typelib

%files -n libgstpbutils
%defattr(-, root, root)
%{_libdir}/libgstpbutils*.so.*

%files -n typelib-1_0-GstPbutils-1_0
%defattr(-, root, root)
%{_libdir}/girepository-1.0/GstPbutils-*.typelib

%files -n libgstriff
%defattr(-, root, root)
%{_libdir}/libgstriff*.so.*

%files -n typelib-1_0-GstRiff-1_0
%defattr(-, root, root)
%{_libdir}/girepository-1.0/GstRiff-*.typelib

%files -n libgstrtp
%defattr(-, root, root)
%{_libdir}/libgstrtp*.so.*

%files -n typelib-1_0-GstRtp-1_0
%defattr(-, root, root)
%{_libdir}/girepository-1.0/GstRtp-*.typelib

%files -n libgstrtsp
%defattr(-, root, root)
%{_libdir}/libgstrtsp*.so.*

%files -n typelib-1_0-GstRtsp-1_0
%defattr(-, root, root)
%{_libdir}/girepository-1.0/GstRtsp-*.typelib

%files -n libgstsdp
%defattr(-, root, root)
%{_libdir}/libgstsdp*.so.*

%files -n typelib-1_0-GstSdp-1_0
%defattr(-, root, root)
%{_libdir}/girepository-1.0/GstSdp-*.typelib

%files -n libgsttag
%defattr(-, root, root)
%{_libdir}/libgsttag*.so.*
%dir %{_datadir}/gst-plugins-base/
%dir %{_datadir}/gst-plugins-base/%{gst_branch}/
%{_datadir}/gst-plugins-base/%{gst_branch}/license-translations.dict

%files -n typelib-1_0-GstTag-1_0
%defattr(-, root, root)
%{_libdir}/girepository-1.0/GstTag-*.typelib

%files -n libgstvideo
%defattr(-, root, root)
%{_libdir}/libgstvideo*.so.*

%files -n typelib-1_0-GstVideo-1_0
%defattr(-, root, root)
%{_libdir}/girepository-1.0/GstVideo-*.typelib

%files devel
%defattr(-, root, root)
%{_includedir}/gstreamer-%{gst_branch}/*
%{_libdir}/*.so
%{_libdir}/pkgconfig/*.pc
%{_datadir}/gir-1.0/*.gir

%files doc
%defattr(-, root, root)
%{_datadir}/gtk-doc/html/gst-plugins-base-libs-%{gst_branch}
%{_datadir}/gtk-doc/html/gst-plugins-base-plugins-%{gst_branch}