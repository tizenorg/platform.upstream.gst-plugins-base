%bcond_with x
%define gst_branch 1.0
%define _lib_gstreamer_dir %{_libdir}/gstreamer-%{gst_branch}
%define _libdebug_dir %{_libdir}/debug/usr/lib

Name:           gst-plugins-base
Version:        1.6.1
Release:        5
License:        LGPL-2.0+
Summary:        GStreamer Streaming-Media Framework Plug-Ins
Url:            http://gstreamer.freedesktop.org/
Group:          Multimedia/Framework
Source:         http://gstreamer.freedesktop.org/src/gst-plugins-base/gst-plugins-base-%{version}.tar.xz
Source100:      common.tar.gz
BuildRequires:  gettext-tools
BuildRequires:  pkgconfig(glib-2.0) >= 2.32
BuildRequires:  pkgconfig(gstreamer-1.0)
BuildRequires:  orc >= 0.4.16
BuildRequires:  python
BuildRequires:  update-desktop-files
BuildRequires:  pkgconfig(gobject-introspection-1.0) >= 1.31.1
BuildRequires:  pkgconfig(alsa) >= 0.9.1
BuildRequires:  pkgconfig(freetype2) >= 2.0.9
BuildRequires:  pkgconfig(libxml-2.0)
BuildRequires:  pkgconfig(ogg) >= 1.0
BuildRequires:  pkgconfig(theoradec) >= 1.1
BuildRequires:  pkgconfig(theoraenc) >= 1.1
BuildRequires:  pkgconfig(vorbis) >= 1.0
BuildRequires:  pkgconfig(vorbisenc) >= 1.0
BuildRequires:  pkgconfig(zlib)

BuildRequires:  pkgconfig(libdrm)
BuildRequires:  pkgconfig(libtbm)
BuildRequires:  pkgconfig(mm-common)
%if %{with x}
BuildRequires:  pkgconfig(ice)
BuildRequires:  pkgconfig(sm)
BuildRequires:  pkgconfig(xext)
BuildRequires:  pkgconfig(xv)
BuildRequires:  pkgconfig(xfixes)
BuildRequires:  pkgconfig(dri2proto)
BuildRequires:  pkgconfig(libdri2)
%endif

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
Requires:       %{name} = %{version}

%description devel
This package contains all necessary include files and libraries needed
to compile and link applications that use gstreamer-plugins-base.

%prep
%setup -q -n gst-plugins-base-%{version}
%setup -q -T -D -a 100

%build
# FIXME: GTKDOC_CFLAGS, GST_OBJ_CFLAGS:
# Silently ignored compilation of uninstalled gtk-doc scanners without RPM_OPT_FLAGS.
export V=1
NOCONFIGURE=1 ./autogen.sh
export CFLAGS="%{optflags} -fno-strict-aliasing\
 -DGST_EXT_WAYLAND_ENHANCEMENT\
%ifarch %{arm}
 -DGST_EXT_AUDIODECODER_MODIFICATION\
 -DGST_EXT_LINK_FIMCCONVERT\
 -DGST_EXT_MIME_TYPES\
 -DGST_TIZEN_MODIFICATION\
 -DGST_TIZEN_SUBPARSE_MODIFICATION\
%endif
 "
%configure\
        --disable-static\
        --enable-experimental\
        --disable-gtk-doc\
        --enable-introspection\
        --disable-encoding\
        --disable-examples\
        --disable-adder\
%if "%{?profile}" == "tv"
		--enable-tv\
%endif
        --enable-use-tbmbuf
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
%license COPYING.LIB

%if "%{?profile}" != "tv"
%{_bindir}/gst-device-monitor-%{gst_branch}
%{_bindir}/gst-discoverer-%{gst_branch}
%{_bindir}/gst-play-%{gst_branch}
%endif

%define _lib_gstreamer_dir %{_libdir}/gstreamer-%{gst_branch}
%define _libdebug_dir %{_libdir}/debug/usr/lib

#%{_lib_gstreamer_dir}/libgstadder.so
%{_lib_gstreamer_dir}/libgstalsa.so
%{_lib_gstreamer_dir}/libgstapp.so
%{_lib_gstreamer_dir}/libgstaudioconvert.so
%{_lib_gstreamer_dir}/libgstaudioresample.so
%{_lib_gstreamer_dir}/libgstaudiotestsrc.so
%{_lib_gstreamer_dir}/libgstaudiorate.so
%{_lib_gstreamer_dir}/libgstgio.so
%{_lib_gstreamer_dir}/libgstogg.so
%{_lib_gstreamer_dir}/libgstplayback.so
%{_lib_gstreamer_dir}/libgstsubparse.so
%{_lib_gstreamer_dir}/libgsttcp.so
%{_lib_gstreamer_dir}/libgsttheora.so
%{_lib_gstreamer_dir}/libgsttypefindfunctions.so
%{_lib_gstreamer_dir}/libgstvideoconvert.so
%{_lib_gstreamer_dir}/libgstvideorate.so
%{_lib_gstreamer_dir}/libgstvideoscale.so
%{_lib_gstreamer_dir}/libgstvideotestsrc.so
%{_lib_gstreamer_dir}/libgstvolume.so
%{_lib_gstreamer_dir}/libgstvorbis.so

%if "%{?profile}" != "tv"
%doc %{_mandir}/man1/gst-device-monitor-*
%doc %{_mandir}/man1/gst-discoverer-*
%doc %{_mandir}/man1/gst-play-*
%endif

%if %{with x}
%{_libdir}/gstreamer-%{gst_branch}/libgstximagesink.so
%{_libdir}/gstreamer-%{gst_branch}/libgstxvimagesink.so
%endif

%{_libdir}/libgstapp*.so.*
%{_libdir}/libgstaudio*.so.*
%{_libdir}/libgstallocators*.so.*
%{_libdir}/libgstfft*.so.*

%define _libgirrepo_dir %{_libdir}/girepository-%{gst_branch}

%{_libgirrepo_dir}/GstApp-%{gst_branch}.typelib
%{_libgirrepo_dir}/GstAudio-%{gst_branch}.typelib
%{_libgirrepo_dir}/GstAllocators-%{gst_branch}.typelib
%{_libgirrepo_dir}/GstFft-%{gst_branch}.typelib
%{_libgirrepo_dir}/GstRiff-%{gst_branch}.typelib
%{_libgirrepo_dir}/GstRtp-%{gst_branch}.typelib
%{_libgirrepo_dir}/GstRtsp-%{gst_branch}.typelib
%{_libgirrepo_dir}/GstSdp-%{gst_branch}.typelib
%{_libgirrepo_dir}/GstTag-%{gst_branch}.typelib
%{_libgirrepo_dir}/GstVideo-%{gst_branch}.typelib
%{_libgirrepo_dir}/GstPbutils-%{gst_branch}.typelib

%{_libdir}/libgstpbutils*.so.*
%{_libdir}/libgstriff*.so.*
%{_libdir}/libgstrtp*.so.*
%{_libdir}/libgstrtsp*.so.*
%{_libdir}/libgstsdp*.so.*
%{_libdir}/libgsttag*.so.*
%{_libdir}/libgstvideo*.so.*
%dir %{_datadir}/gst-plugins-base/
%dir %{_datadir}/gst-plugins-base/%{gst_branch}/
%{_datadir}/gst-plugins-base/%{gst_branch}/license-translations.dict

%files devel
%manifest %{name}.manifest
%defattr(-, root, root)
%{_includedir}/gstreamer-%{gst_branch}/*
%{_libdir}/*.so
%{_libdir}/pkgconfig/*.pc
%{_datadir}/gir-1.0/*.gir
