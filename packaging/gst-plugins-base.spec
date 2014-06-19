%bcond_with introspection
%bcond_with cdparanoia
%bcond_with x

Name:           gst-plugins-base
Version:        1.2.4
Release:        0
License:        LGPL-2.1+ and GPL-2.0+
Summary:        GStreamer Streaming-Media Framework Plug-Ins
%define gst_branch 1.0
Url:            http://gstreamer.freedesktop.org/
Group:          Multimedia/Framework
Source:         http://download.gnome.org/sources/gst-plugins-base/1.0/%{name}-%{version}.tar.xz
Source2:        baselibs.conf
Source100:      common.tar.bz2
Source1001: 	gst-plugins-base.manifest
BuildRequires:  gettext-tools
BuildRequires:  glib2-devel >= 2.32
BuildRequires:  gstreamer-devel >= 1.0.0
BuildRequires:  gstreamer-utils > 0.11
BuildRequires:  orc >= 0.4.16
BuildRequires:  python
BuildRequires:  update-desktop-files
%if %{with x}
BuildRequires:  pkgconfig(ice)
BuildRequires:  pkgconfig(sm)
BuildRequires:  pkgconfig(xext)
BuildRequires:  pkgconfig(xv)
%endif
%if %{with introspection}
BuildRequires:  gobject-introspection-devel >= 1.31.1
%endif
%if %{with cdparanoia}
BuildRequires:  pkgconfig(cdparanoia-3)
%endif
BuildRequires:  pkgconfig(alsa) >= 0.9.1
BuildRequires:  pkgconfig(freetype2) >= 2.0.9
#BuildRequires:  pkgconfig(iso-codes)
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
# We want to have base modules installed:
Requires:       %{name}

%description -n libgstapp
GStreamer is a streaming media framework based on graphs of filters
that operate on media data. Applications using this library can do
anything media-related, from real-time sound processing to playing
videos. Its plug-in-based architecture means that new data types or
processing capabilities can be added simply by installing new plug-ins.

%package -n typelib-GstApp
Summary:        GStreamer Streaming-Media Framework Plug-Ins -- Introspection bindings

%description -n typelib-GstApp
GStreamer is a streaming media framework based on graphs of filters
that operate on media data. Applications using this library can do
anything media-related, from real-time sound processing to playing
videos. Its plug-in-based architecture means that new data types or
processing capabilities can be added simply by installing new plug-ins.

This package provides the GObject Introspection bindings for GStreamer
plug-ins.

%package -n libgstaudio
Summary:        GStreamer Streaming-Media Framework Plug-Ins
Requires:       %{name}

%description -n libgstaudio
GStreamer is a streaming media framework based on graphs of filters
that operate on media data. Applications using this library can do
anything media-related, from real-time sound processing to playing
videos. Its plug-in-based architecture means that new data types or
processing capabilities can be added simply by installing new plug-ins.

%package -n typelib-GstAudio
Summary:        GStreamer Streaming-Media Framework Plug-Ins -- Introspection bindings

%description -n typelib-GstAudio
GStreamer is a streaming media framework based on graphs of filters
that operate on media data. Applications using this library can do
anything media-related, from real-time sound processing to playing
videos. Its plug-in-based architecture means that new data types or
processing capabilities can be added simply by installing new plug-ins.

This package provides the GObject Introspection bindings for GStreamer
plug-ins.

%package -n libgstallocators
Summary:        GStreamer Streaming-Media Framework Plug-Ins
Requires:       %{name}

%description -n libgstallocators
GStreamer is a streaming media framework based on graphs of filters
that operate on media data. Applications using this library can do
anything media-related, from real-time sound processing to playing
videos. Its plug-in-based architecture means that new data types or
processing capabilities can be added simply by installing new plug-ins.

%package -n typelib-GstAllocators
Summary:        GStreamer Streaming-Media Framework Plug-Ins -- Introspection bindings

%description -n typelib-GstAllocators
GStreamer is a streaming media framework based on graphs of filters
that operate on media data. Applications using this library can do
anything media-related, from real-time sound processing to playing
videos. Its plug-in-based architecture means that new data types or
processing capabilities can be added simply by installing new plug-ins.

This package provides the GObject Introspection bindings for GStreamer
plug-ins.

%package -n libgstfft
Summary:        GStreamer Streaming-Media Framework Plug-Ins
Requires:       %{name}

%description -n libgstfft
GStreamer is a streaming media framework based on graphs of filters
that operate on media data. Applications using this library can do
anything media-related, from real-time sound processing to playing
videos. Its plug-in-based architecture means that new data types or
processing capabilities can be added simply by installing new plug-ins.

%package -n typelib-GstFft
Summary:        GStreamer Streaming-Media Framework Plug-Ins -- Introspection bindings

%description -n typelib-GstFft
GStreamer is a streaming media framework based on graphs of filters
that operate on media data. Applications using this library can do
anything media-related, from real-time sound processing to playing
videos. Its plug-in-based architecture means that new data types or
processing capabilities can be added simply by installing new plug-ins.

This package provides the GObject Introspection bindings for GStreamer
plug-ins.

%package -n libgstpbutils
Summary:        GStreamer Streaming-Media Framework Plug-Ins
Requires:       %{name}

%description -n libgstpbutils
GStreamer is a streaming media framework based on graphs of filters
that operate on media data. Applications using this library can do
anything media-related, from real-time sound processing to playing
videos. Its plug-in-based architecture means that new data types or
processing capabilities can be added simply by installing new plug-ins.

%package -n typelib-GstPbutils
Summary:        GStreamer Streaming-Media Framework Plug-Ins -- Introspection bindings

%description -n typelib-GstPbutils
GStreamer is a streaming media framework based on graphs of filters
that operate on media data. Applications using this library can do
anything media-related, from real-time sound processing to playing
videos. Its plug-in-based architecture means that new data types or
processing capabilities can be added simply by installing new plug-ins.

This package provides the GObject Introspection bindings for GStreamer
plug-ins.

%package -n libgstriff
Summary:        GStreamer Streaming-Media Framework Plug-Ins
Requires:       %{name}

%description -n libgstriff
GStreamer is a streaming media framework based on graphs of filters
that operate on media data. Applications using this library can do
anything media-related, from real-time sound processing to playing
videos. Its plug-in-based architecture means that new data types or
processing capabilities can be added simply by installing new plug-ins.

%package -n typelib-GstRiff
Summary:        GStreamer Streaming-Media Framework Plug-Ins -- Introspection bindings

%description -n typelib-GstRiff
GStreamer is a streaming media framework based on graphs of filters
that operate on media data. Applications using this library can do
anything media-related, from real-time sound processing to playing
videos. Its plug-in-based architecture means that new data types or
processing capabilities can be added simply by installing new plug-ins.

This package provides the GObject Introspection bindings for GStreamer
plug-ins.

%package -n libgstrtp
Summary:        GStreamer Streaming-Media Framework Plug-Ins
# We want to have base modules installed:
Requires:       %{name}

%description -n libgstrtp
GStreamer is a streaming media framework based on graphs of filters
that operate on media data. Applications using this library can do
anything media-related, from real-time sound processing to playing
videos. Its plug-in-based architecture means that new data types or
processing capabilities can be added simply by installing new plug-ins.

%package -n typelib-GstRtp
Summary:        GStreamer Streaming-Media Framework Plug-Ins -- Introspection bindings

%description -n typelib-GstRtp
GStreamer is a streaming media framework based on graphs of filters
that operate on media data. Applications using this library can do
anything media-related, from real-time sound processing to playing
videos. Its plug-in-based architecture means that new data types or
processing capabilities can be added simply by installing new plug-ins.

This package provides the GObject Introspection bindings for GStreamer
plug-ins.

%package -n libgstrtsp
Summary:        GStreamer Streaming-Media Framework Plug-Ins
# We want to have base modules installed:
Requires:       %{name}

%description -n libgstrtsp
GStreamer is a streaming media framework based on graphs of filters
that operate on media data. Applications using this library can do
anything media-related, from real-time sound processing to playing
videos. Its plug-in-based architecture means that new data types or
processing capabilities can be added simply by installing new plug-ins.

%package -n typelib-GstRtsp
Summary:        GStreamer Streaming-Media Framework Plug-Ins -- Introspection bindings

%description -n typelib-GstRtsp
GStreamer is a streaming media framework based on graphs of filters
that operate on media data. Applications using this library can do
anything media-related, from real-time sound processing to playing
videos. Its plug-in-based architecture means that new data types or
processing capabilities can be added simply by installing new plug-ins.

This package provides the GObject Introspection bindings for GStreamer
plug-ins.

%package -n libgstsdp
Summary:        GStreamer Streaming-Media Framework Plug-Ins
# We want to have base modules installed:
Requires:       %{name}

%description -n libgstsdp
GStreamer is a streaming media framework based on graphs of filters
that operate on media data. Applications using this library can do
anything media-related, from real-time sound processing to playing
videos. Its plug-in-based architecture means that new data types or
processing capabilities can be added simply by installing new plug-ins.

%package -n typelib-GstSdp
Summary:        GStreamer Streaming-Media Framework Plug-Ins -- Introspection bindings

%description -n typelib-GstSdp
GStreamer is a streaming media framework based on graphs of filters
that operate on media data. Applications using this library can do
anything media-related, from real-time sound processing to playing
videos. Its plug-in-based architecture means that new data types or
processing capabilities can be added simply by installing new plug-ins.

This package provides the GObject Introspection bindings for GStreamer
plug-ins.

%package -n libgsttag
Summary:        GStreamer Streaming-Media Framework Plug-Ins
# We want to have base modules installed:
Requires:       %{name}

%description -n libgsttag
GStreamer is a streaming media framework based on graphs of filters
that operate on media data. Applications using this library can do
anything media-related, from real-time sound processing to playing
videos. Its plug-in-based architecture means that new data types or
processing capabilities can be added simply by installing new plug-ins.

%package -n typelib-GstTag
Summary:        GStreamer Streaming-Media Framework Plug-Ins -- Introspection bindings

%description -n typelib-GstTag
GStreamer is a streaming media framework based on graphs of filters
that operate on media data. Applications using this library can do
anything media-related, from real-time sound processing to playing
videos. Its plug-in-based architecture means that new data types or
processing capabilities can be added simply by installing new plug-ins.

This package provides the GObject Introspection bindings for GStreamer
plug-ins.

%package -n libgstvideo
Summary:        GStreamer Streaming-Media Framework Plug-Ins
# We want to have base modules installed:
Requires:       %{name}

%description -n libgstvideo
GStreamer is a streaming media framework based on graphs of filters
that operate on media data. Applications using this library can do
anything media-related, from real-time sound processing to playing
videos. Its plug-in-based architecture means that new data types or
processing capabilities can be added simply by installing new plug-ins.

%package -n typelib-GstVideo
Summary:        GStreamer Streaming-Media Framework Plug-Ins -- Introspection bindings

%description -n typelib-GstVideo
GStreamer is a streaming media framework based on graphs of filters
that operate on media data. Applications using this library can do
anything media-related, from real-time sound processing to playing
videos. Its plug-in-based architecture means that new data types or
processing capabilities can be added simply by installing new plug-ins.

This package provides the GObject Introspection bindings for GStreamer
plug-ins.

%package devel
Summary:        Include files and Libraries
Requires:       libgstapp = %{version}
Requires:       libgstaudio = %{version}
Requires:       libgstallocators = %{version}
Requires:       libgstfft = %{version}
Requires:       libgstpbutils = %{version}
Requires:       libgstriff = %{version}
Requires:       libgstrtp = %{version}
Requires:       libgstrtsp = %{version}
Requires:       libgstsdp = %{version}
Requires:       libgsttag = %{version}
Requires:       libgstvideo = %{version}
%if %{with introspection}
Requires:       typelib-GstApp = %{version}
Requires:       typelib-GstAudio = %{version}
Requires:       typelib-GstFft = %{version}
Requires:       typelib-GstPbutils = %{version}
Requires:       typelib-GstRiff = %{version}
Requires:       typelib-GstRtp = %{version}
Requires:       typelib-GstRtsp = %{version}
Requires:       typelib-GstSdp = %{version}
Requires:       typelib-GstTag = %{version}
Requires:       typelib-GstVideo = %{version}
%endif
Provides:       gst-plugins-base-devel = %{version}

%description devel
This package contains all necessary include files and libraries needed
to compile and link applications that use gstreamer-plugins-base.


%prep
%setup -q
%setup -q -T -D -a 100
cp %{SOURCE1001} .

%build
# FIXME: GTKDOC_CFLAGS, GST_OBJ_CFLAGS:
# Silently ignored compilation of uninstalled gtk-doc scanners without RPM_OPT_FLAGS.
export V=1
NOCONFIGURE=1 ./autogen.sh
export CFLAGS="%{optflags} -fno-strict-aliasing"
%configure\
	--disable-static\
	--enable-experimental\
	--disable-gtk-doc\
%if %{with introspection}
    --enable-introspection\
%endif
	--disable-examples
make %{?_smp_mflags}

%install
%make_install
%find_lang %{name}-%{gst_branch}
mv %{name}-%{gst_branch}.lang %{name}.lang

%post -n libgstapp -p /sbin/ldconfig

%postun -n libgstapp -p /sbin/ldconfig

%post -n libgstaudio -p /sbin/ldconfig

%postun -n libgstaudio -p /sbin/ldconfig

%post -n libgstallocators -p /sbin/ldconfig

%postun -n libgstallocators -p /sbin/ldconfig

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

%lang_package

%files
%manifest %{name}.manifest
%defattr(-, root, root)
%license COPYING COPYING.LIB
%{_bindir}/gst-discoverer-%{gst_branch}
%{_bindir}/gst-play-%{gst_branch}
%{_libdir}/gstreamer-%{gst_branch}/libgstadder.so
%{_libdir}/gstreamer-%{gst_branch}/libgstalsa.so
%{_libdir}/gstreamer-%{gst_branch}/libgstapp.so
%{_libdir}/gstreamer-%{gst_branch}/libgstaudioconvert.so
%{_libdir}/gstreamer-%{gst_branch}/libgstaudioresample.so
%{_libdir}/gstreamer-%{gst_branch}/libgstaudiotestsrc.so
%{_libdir}/gstreamer-%{gst_branch}/libgstaudiorate.so
%{_libdir}/gstreamer-%{gst_branch}/libgstgio.so
%{_libdir}/gstreamer-%{gst_branch}/libgstogg.so
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
%{_libdir}/gstreamer-%{gst_branch}/libgstencodebin.so
%doc %{_mandir}/man1/gst-discoverer-*
%doc %{_mandir}/man1/gst-play-*

%{_libdir}/gstreamer-%{gst_branch}/libgstpango.so
%if %{with x}
%{_libdir}/gstreamer-%{gst_branch}/libgstximagesink.so
%{_libdir}/gstreamer-%{gst_branch}/libgstxvimagesink.so
%endif
%if %{with cdparanoia}
%{_libdir}/gstreamer-%{gst_branch}/libgstcdparanoia.so
%endif

%files -n libgstapp
%manifest %{name}.manifest
%defattr(-, root, root)
%{_libdir}/libgstapp*.so.*

%files -n libgstaudio
%manifest %{name}.manifest
%defattr(-, root, root)
%{_libdir}/libgstaudio*.so.*

%files -n libgstallocators
%manifest %{name}.manifest
%defattr(-, root, root)
%{_libdir}/libgstallocators*.so.*

%files -n libgstfft
%manifest %{name}.manifest
%defattr(-, root, root)
%{_libdir}/libgstfft*.so.*

%if %{with introspection}
%files -n typelib-GstApp
%manifest %{name}.manifest
%defattr(-, root, root)
%{_libdir}/girepository-1.0/GstApp-*.typelib

%files -n typelib-GstAudio
%manifest %{name}.manifest
%defattr(-, root, root)
%{_libdir}/girepository-1.0/GstAudio-*.typelib

%files -n typelib-GstAllocators
%manifest %{name}.manifest
%defattr(-, root, root)
%{_libdir}/girepository-1.0/GstAllocators-*.typelib

%files -n typelib-GstFft
%manifest %{name}.manifest
%defattr(-, root, root)
%{_libdir}/girepository-1.0/GstFft-*.typelib

%files -n typelib-GstRiff
%manifest %{name}.manifest
%defattr(-, root, root)
%{_libdir}/girepository-1.0/GstRiff-*.typelib

%files -n typelib-GstRtp
%manifest %{name}.manifest
%defattr(-, root, root)
%{_libdir}/girepository-1.0/GstRtp-*.typelib

%files -n typelib-GstRtsp
%manifest %{name}.manifest
%defattr(-, root, root)
%{_libdir}/girepository-1.0/GstRtsp-*.typelib

%files -n typelib-GstSdp
%manifest %{name}.manifest
%defattr(-, root, root)
%{_libdir}/girepository-1.0/GstSdp-*.typelib

%files -n typelib-GstTag
%manifest %{name}.manifest
%defattr(-, root, root)
%{_libdir}/girepository-1.0/GstTag-*.typelib

%files -n typelib-GstVideo
%manifest %{name}.manifest
%defattr(-, root, root)
%{_libdir}/girepository-1.0/GstVideo-*.typelib

%files -n typelib-GstPbutils
%manifest %{name}.manifest
%defattr(-, root, root)
%{_libdir}/girepository-1.0/GstPbutils-*.typelib

%endif

%files -n libgstpbutils
%manifest %{name}.manifest
%defattr(-, root, root)
%{_libdir}/libgstpbutils*.so.*


%files -n libgstriff
%manifest %{name}.manifest
%defattr(-, root, root)
%{_libdir}/libgstriff*.so.*


%files -n libgstrtp
%manifest %{name}.manifest
%defattr(-, root, root)
%{_libdir}/libgstrtp*.so.*


%files -n libgstrtsp
%manifest %{name}.manifest
%defattr(-, root, root)
%{_libdir}/libgstrtsp*.so.*


%files -n libgstsdp
%manifest %{name}.manifest
%defattr(-, root, root)
%{_libdir}/libgstsdp*.so.*


%files -n libgsttag
%manifest %{name}.manifest
%defattr(-, root, root)
%{_libdir}/libgsttag*.so.*
%dir %{_datadir}/gst-plugins-base/
%dir %{_datadir}/gst-plugins-base/%{gst_branch}/
%{_datadir}/gst-plugins-base/%{gst_branch}/license-translations.dict


%files -n libgstvideo
%manifest %{name}.manifest
%defattr(-, root, root)
%{_libdir}/libgstvideo*.so.*


%files devel
%manifest %{name}.manifest
%defattr(-, root, root)
%{_includedir}/gstreamer-%{gst_branch}/*
%{_libdir}/*.so
%{_libdir}/pkgconfig/*.pc
%if %{with introspection}
%{_datadir}/gir-1.0/*.gir
%endif
