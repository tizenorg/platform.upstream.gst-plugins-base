%bcond_with cdparanoia
%bcond_with x
%define gst_branch 1.0

Name:           gst-plugins-base
Version:        1.4.1
Release:        6
License:        LGPL-2.1+ and GPL-2.0+
Summary:        GStreamer Streaming-Media Framework Plug-Ins
Url:            http://gstreamer.freedesktop.org/
Group:          Multimedia/Framework
Source:         http://gstreamer.freedesktop.org/src/gst-plugins-base/gst-plugins-base-%{version}.tar.xz
Source100:      common.tar.bz2
BuildRequires:  gettext-tools
BuildRequires:  glib2-devel >= 2.32
BuildRequires:  gstreamer-devel >= 1.0.0
BuildRequires:  orc >= 0.4.16
BuildRequires:  python
BuildRequires:  update-desktop-files
%if %{with x}
BuildRequires:  pkgconfig(ice)
BuildRequires:  pkgconfig(sm)
BuildRequires:  pkgconfig(xext)
BuildRequires:  pkgconfig(xv)
BuildRequires:  pkgconfig(xfixes)
BuildRequires:  pkgconfig(dri2proto)
BuildRequires:  pkgconfig(libdri2)
%endif
BuildRequires:  gobject-introspection-devel >= 1.31.1
%if %{with cdparanoia}
BuildRequires:  pkgconfig(cdparanoia-3)
%endif
BuildRequires:  pkgconfig(alsa) >= 0.9.1
BuildRequires:  pkgconfig(freetype2) >= 2.0.9
BuildRequires:  pkgconfig(libxml-2.0)
BuildRequires:  pkgconfig(ogg) >= 1.0
BuildRequires:  pkgconfig(pango) >= 1.22.0
BuildRequires:  pkgconfig(pangocairo) >= 1.22.0
BuildRequires:  pkgconfig(theoradec) >= 1.1
BuildRequires:  pkgconfig(theoraenc) >= 1.1
BuildRequires:  pkgconfig(vorbis) >= 1.0
BuildRequires:  pkgconfig(vorbisenc) >= 1.0
BuildRequires:  pkgconfig(zlib)
BuildRequires:  pkgconfig(libdrm)
BuildRequires:  pkgconfig(libtbm)
BuildRequires:  pkgconfig(mm-ta)
BuildRequires:  pkgconfig(mm-common)
Requires:       gstreamer >= 1.0.0
Supplements:    gstreamer

%description
GStreamer is a streaming media framework based on graphs of filters
that operate on media data. Applications using this library can do
anything media-related, from real-time sound processing to playing
videos. Its plug-in-based architecture means that new data types or
processing capabilities can be added simply by installing new plug-ins.


%package devel
Summary:        Include files and Libraries
Requires: 	   %{name} = %{version}

%description devel
This package contains all necessary include files and libraries needed
to compile and link applications that use gstreamer-plugins-base.


%prep
%setup -q
%setup -q -T -D -a 100


%build
# FIXME: GTKDOC_CFLAGS, GST_OBJ_CFLAGS:
# Silently ignored compilation of uninstalled gtk-doc scanners without RPM_OPT_FLAGS.
export V=1
NOCONFIGURE=1 ./autogen.sh
export CFLAGS="%{optflags} -fno-strict-aliasing\
 %ifarch %{arm}
 -DGST_EXT_XV_ENHANCEMENT\
 -DGST_EXT_LINK_FIMCCONVERT\
 -DGST_EXT_MIME_TYPES
 %endif
 "
%configure\
	--disable-static\
	--enable-experimental\
	--disable-gtk-doc\
	--enable-introspection\
	--disable-encoding\
	--disable-examples
make %{?_smp_mflags}

%install
rm -rf %{buildroot}
%make_install
%find_lang %{name}-%{gst_branch}
mv %{name}-%{gst_branch}.lang %{name}.lang


%post -p /sbin/ldconfig


%postun -p /sbin/ldconfig


%lang_package

%files
%manifest %{name}.manifest
%defattr(-, root, root)
%license COPYING COPYING.LIB
%{_bindir}/gst-device-monitor-%{gst_branch}
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
#%{_libdir}/gstreamer-%{gst_branch}/libgstencodebin.so
%doc %{_mandir}/man1/gst-device-monitor-*
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

%{_libdir}/libgstapp*.so.*
%{_libdir}/libgstaudio*.so.*
%{_libdir}/libgstallocators*.so.*
%{_libdir}/libgstfft*.so.*
%{_libdir}/girepository-1.0/GstApp-*.typelib
%{_libdir}/girepository-1.0/GstAudio-*.typelib
%{_libdir}/girepository-1.0/GstAllocators-*.typelib
%{_libdir}/girepository-1.0/GstFft-*.typelib
%{_libdir}/girepository-1.0/GstRiff-*.typelib
%{_libdir}/girepository-1.0/GstRtp-*.typelib
%{_libdir}/girepository-1.0/GstRtsp-*.typelib
%{_libdir}/girepository-1.0/GstSdp-*.typelib
%{_libdir}/girepository-1.0/GstTag-*.typelib
%{_libdir}/girepository-1.0/GstVideo-*.typelib
%{_libdir}/girepository-1.0/GstPbutils-*.typelib
%{_libdir}/libgstpbutils*.so.*
%{_libdir}/libgstriff*.so.*
%{_libdir}/libgstrtp*.so.*
%{_libdir}/libgstrtsp*.so.*
%{_libdir}/libgstsdp*.so.*
%{_libdir}/libgsttag*.so.*
%dir %{_datadir}/gst-plugins-base/
%dir %{_datadir}/gst-plugins-base/%{gst_branch}/
%{_datadir}/gst-plugins-base/%{gst_branch}/license-translations.dict
%{_libdir}/libgstvideo*.so.*


%files devel
%manifest %{name}.manifest
%defattr(-, root, root)
%{_includedir}/gstreamer-%{gst_branch}/*
%{_libdir}/*.so
%{_libdir}/pkgconfig/*.pc
%{_datadir}/gir-1.0/*.gir
