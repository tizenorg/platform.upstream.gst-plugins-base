Name:       gst-plugins-base0.10
Summary:    GStreamer streaming media framework base plug-ins
Version:    0.10.36
Release:    30
Group:      Applications/Multimedia
License:    LGPLv2+
Source0:    %{name}-%{version}.tar.gz
#Patch0:     Samsung-feature-bugs.patch
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig
BuildRequires:  pkgconfig(ogg)
BuildRequires:  pkgconfig(theora)
BuildRequires:  pkgconfig(vorbis)
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(libxml-2.0)
BuildRequires:  pkgconfig(alsa)
BuildRequires:  pkgconfig(gstreamer-0.10)
BuildRequires:  pkgconfig(gstreamer-base-0.10)
BuildRequires:  pkgconfig(xv)
BuildRequires:  pkgconfig(pango)
BuildRequires:  pkgconfig(xfixes)
BuildRequires:  pkgconfig(dri2proto)
BuildRequires:  pkgconfig(libdri2)
BuildRequires:  pkgconfig(libtbm)
BuildRequires:  pkgconfig(mm-ta)
BuildRequires:  intltool


%description
A well-groomed and well-maintained collection of GStreamer plug-ins and elements, 
spanning the range of possible types of elements one would want to write for GStreamer.



%package devel
Summary:    Development tools for GStreamer base plugins
Group:      Development/Libraries
Requires:   %{name} = %{version}-%{release}

%description devel
Separate sub-package for development based on gstreamer base plugins. 


%package tools
Summary:    Gstreamer base plugins utilities
Group:      Development/Tools
Requires:   %{name} = %{version}-%{release}

%description tools
Separate sub-package contaning helper applications of gstreamer base plugins.



%prep
%setup -q 

#%patch0 -p1

%build
%autogen --noconfigure

export CFLAGS+=" -Wall -g -fPIC\
 -DGST_EXT_XV_ENHANCEMENT\
 -DGST_EXT_LINK_FIMCCONVERT\
 -DGST_EXT_TYPEFIND_ENHANCEMENT\
 -DGST_EXT_MIME_TYPES"

%configure --prefix=/usr\
 --disable-static\
 --disable-nls\
 --with-html-dir=/tmp/dump\
 --disable-examples\
 --disable-audiorate\
 --disable-gdp\
 --disable-cdparanoia\
 --disable-gnome_vfs\
 --disable-libvisual\
 --disable-freetypetest\
 --disable-rpath\
 --disable-valgrind\
 --disable-gcov\
 --disable-gtk-doc\
 --disable-debug\
 --with-audioresample-format=int

make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install


rm -rf %{buildroot}/tmp/dump

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig


%files
%manifest gst-plugins-base.manifest
%defattr(-,root,root,-)
#%doc COPYING 
# libraries
%{_libdir}/libgstinterfaces-0.10.so.*
%{_libdir}/libgstaudio-0.10.so.*
%exclude %{_libdir}/libgstcdda-0.10.so.*
%{_libdir}/libgstfft-0.10.so.*
%{_libdir}/libgstriff-0.10.so.*
%{_libdir}/libgsttag-0.10.so.*
%{_libdir}/libgstnetbuffer-0.10.so.*
%{_libdir}/libgstrtp-0.10.so.*
%{_libdir}/libgstvideo-0.10.so.*
%{_libdir}/libgstpbutils-0.10.so.*
%{_libdir}/libgstrtsp-0.10.so.*
%{_libdir}/libgstsdp-0.10.so.*
%{_libdir}/libgstapp-0.10.so.*
# base plugins without external dependencies
%{_libdir}/gstreamer-0.10/libgstadder.so
%{_libdir}/gstreamer-0.10/libgstaudioconvert.so
%{_libdir}/gstreamer-0.10/libgstaudiotestsrc.so
%{_libdir}/gstreamer-0.10/libgstffmpegcolorspace.so
%{_libdir}/gstreamer-0.10/libgstdecodebin.so
%{_libdir}/gstreamer-0.10/libgstdecodebin2.so
%{_libdir}/gstreamer-0.10/libgstplaybin.so
%{_libdir}/gstreamer-0.10/libgsttypefindfunctions.so
%{_libdir}/gstreamer-0.10/libgstvideotestsrc.so
%{_libdir}/gstreamer-0.10/libgstsubparse.so
%{_libdir}/gstreamer-0.10/libgstvolume.so
%{_libdir}/gstreamer-0.10/libgstvideorate.so
%{_libdir}/gstreamer-0.10/libgstvideoscale.so
%{_libdir}/gstreamer-0.10/libgsttcp.so
%{_libdir}/gstreamer-0.10/libgstaudioresample.so
%{_libdir}/gstreamer-0.10/libgstapp.so
%{_libdir}/gstreamer-0.10/libgstxvimagesink.so
%exclude %{_libdir}/gstreamer-0.10/libgstencodebin.so
# base plugins with dependencies
%{_libdir}/gstreamer-0.10/libgstalsa.so
%{_libdir}/gstreamer-0.10/libgstogg.so
%{_libdir}/gstreamer-0.10/libgsttheora.so
%{_libdir}/gstreamer-0.10/libgstvorbis.so
%{_libdir}/gstreamer-0.10/libgstximagesink.so
%{_libdir}/gstreamer-0.10/libgstpango.so
%{_libdir}/gstreamer-0.10/libgstgio.so
# data
%{_datadir}/gst-plugins-base/license-translations.dict


%files devel
%defattr(-,root,root,-)
%dir %{_includedir}/gstreamer-0.10/gst/app
%{_includedir}/gstreamer-0.10/gst/tag/xmpwriter.h
%{_includedir}/gstreamer-0.10/gst/app/gstappbuffer.h
%{_includedir}/gstreamer-0.10/gst/app/gstappsink.h
%{_includedir}/gstreamer-0.10/gst/app/gstappsrc.h
%dir %{_includedir}/gstreamer-0.10/gst/audio
%{_includedir}/gstreamer-0.10/gst/audio/audio.h
%{_includedir}/gstreamer-0.10/gst/audio/audio-enumtypes.h
%{_includedir}/gstreamer-0.10/gst/audio/gstaudioclock.h
%{_includedir}/gstreamer-0.10/gst/audio/gstaudiodecoder.h
%{_includedir}/gstreamer-0.10/gst/audio/gstaudioencoder.h
%{_includedir}/gstreamer-0.10/gst/audio/gstaudiofilter.h
%{_includedir}/gstreamer-0.10/gst/audio/gstaudioiec61937.h
%{_includedir}/gstreamer-0.10/gst/audio/gstaudiosink.h
%{_includedir}/gstreamer-0.10/gst/audio/gstaudiosrc.h
%{_includedir}/gstreamer-0.10/gst/audio/gstbaseaudiosink.h
%{_includedir}/gstreamer-0.10/gst/audio/gstbaseaudiosrc.h
%{_includedir}/gstreamer-0.10/gst/audio/gstringbuffer.h
%{_includedir}/gstreamer-0.10/gst/audio/mixerutils.h
%{_includedir}/gstreamer-0.10/gst/audio/multichannel.h
%exclude %dir %{_includedir}/gstreamer-0.10/gst/cdda
%exclude %{_includedir}/gstreamer-0.10/gst/cdda/gstcddabasesrc.h
%dir %{_includedir}/gstreamer-0.10/gst/floatcast
%{_includedir}/gstreamer-0.10/gst/floatcast/floatcast.h
%dir %{_includedir}/gstreamer-0.10/gst/fft
%{_includedir}/gstreamer-0.10/gst/fft/gstfft*.h
%dir %{_includedir}/gstreamer-0.10/gst/interfaces
%{_includedir}/gstreamer-0.10/gst/interfaces/cameracontrol.h
%{_includedir}/gstreamer-0.10/gst/interfaces/cameracontrolchannel.h
%{_includedir}/gstreamer-0.10/gst/interfaces/colorbalance.h
%{_includedir}/gstreamer-0.10/gst/interfaces/colorbalancechannel.h
%{_includedir}/gstreamer-0.10/gst/interfaces/interfaces-enumtypes.h
%{_includedir}/gstreamer-0.10/gst/interfaces/mixer.h
%{_includedir}/gstreamer-0.10/gst/interfaces/mixeroptions.h
%{_includedir}/gstreamer-0.10/gst/interfaces/mixertrack.h
%{_includedir}/gstreamer-0.10/gst/interfaces/navigation.h
%{_includedir}/gstreamer-0.10/gst/interfaces/propertyprobe.h
%{_includedir}/gstreamer-0.10/gst/interfaces/tuner.h
%{_includedir}/gstreamer-0.10/gst/interfaces/tunerchannel.h
%{_includedir}/gstreamer-0.10/gst/interfaces/tunernorm.h
%{_includedir}/gstreamer-0.10/gst/interfaces/videoorientation.h
%{_includedir}/gstreamer-0.10/gst/interfaces/xoverlay.h
%{_includedir}/gstreamer-0.10/gst/interfaces/streamvolume.h
%dir %{_includedir}/gstreamer-0.10/gst/netbuffer
%{_includedir}/gstreamer-0.10/gst/netbuffer/gstnetbuffer.h
%dir %{_includedir}/gstreamer-0.10/gst/pbutils
%{_includedir}/gstreamer-0.10/gst/pbutils/codec-utils.h
%{_includedir}/gstreamer-0.10/gst/pbutils/descriptions.h
%{_includedir}/gstreamer-0.10/gst/pbutils/gstdiscoverer.h
%{_includedir}/gstreamer-0.10/gst/pbutils/gstpluginsbaseversion.h
%{_includedir}/gstreamer-0.10/gst/pbutils/install-plugins.h
%{_includedir}/gstreamer-0.10/gst/pbutils/missing-plugins.h
%{_includedir}/gstreamer-0.10/gst/pbutils/pbutils.h
%{_includedir}/gstreamer-0.10/gst/pbutils/pbutils-enumtypes.h
%{_includedir}/gstreamer-0.10/gst/pbutils/encoding-profile.h
%{_includedir}/gstreamer-0.10/gst/pbutils/encoding-target.h

%dir %{_includedir}/gstreamer-0.10/gst/riff
%{_includedir}/gstreamer-0.10/gst/riff/riff-ids.h
%{_includedir}/gstreamer-0.10/gst/riff/riff-media.h
%{_includedir}/gstreamer-0.10/gst/riff/riff-read.h
%dir %{_includedir}/gstreamer-0.10/gst/rtp
%{_includedir}/gstreamer-0.10/gst/rtp/gstbasertpaudiopayload.h
%{_includedir}/gstreamer-0.10/gst/rtp/gstbasertpdepayload.h
%{_includedir}/gstreamer-0.10/gst/rtp/gstbasertppayload.h
%{_includedir}/gstreamer-0.10/gst/rtp/gstrtcpbuffer.h
%{_includedir}/gstreamer-0.10/gst/rtp/gstrtpbuffer.h
%{_includedir}/gstreamer-0.10/gst/rtp/gstrtppayloads.h
%dir %{_includedir}/gstreamer-0.10/gst/rtsp
%{_includedir}/gstreamer-0.10/gst/rtsp/gstrtsp-enumtypes.h
%{_includedir}/gstreamer-0.10/gst/rtsp/gstrtspbase64.h
%{_includedir}/gstreamer-0.10/gst/rtsp/gstrtspconnection.h
%{_includedir}/gstreamer-0.10/gst/rtsp/gstrtspdefs.h
%{_includedir}/gstreamer-0.10/gst/rtsp/gstrtspextension.h
%{_includedir}/gstreamer-0.10/gst/rtsp/gstrtspmessage.h
%{_includedir}/gstreamer-0.10/gst/rtsp/gstrtsprange.h
%{_includedir}/gstreamer-0.10/gst/rtsp/gstrtsptransport.h
%{_includedir}/gstreamer-0.10/gst/rtsp/gstrtspurl.h
%dir %{_includedir}/gstreamer-0.10/gst/sdp/
%{_includedir}/gstreamer-0.10/gst/sdp/gstsdp.h
%{_includedir}/gstreamer-0.10/gst/sdp/gstsdpmessage.h
%dir %{_includedir}/gstreamer-0.10/gst/tag
%{_includedir}/gstreamer-0.10/gst/tag/tag.h
%{_includedir}/gstreamer-0.10/gst/tag/gsttagdemux.h
%{_includedir}/gstreamer-0.10/gst/tag/gsttagmux.h
%dir %{_includedir}/gstreamer-0.10/gst/video
%{_includedir}/gstreamer-0.10/gst/video/gstvideofilter.h
%{_includedir}/gstreamer-0.10/gst/video/gstvideosink.h
%{_includedir}/gstreamer-0.10/gst/video/video.h
%{_includedir}/gstreamer-0.10/gst/video/video-enumtypes.h
%{_includedir}/gstreamer-0.10/gst/video/video-overlay-composition.h
%{_libdir}/libgstaudio-0.10.so
%{_libdir}/libgstinterfaces-0.10.so
%{_libdir}/libgstnetbuffer-0.10.so
%{_libdir}/libgstriff-0.10.so
%{_libdir}/libgstrtp-0.10.so
%{_libdir}/libgsttag-0.10.so
%{_libdir}/libgstvideo-0.10.so
%exclude %{_libdir}/libgstcdda-0.10.so
%{_libdir}/libgstpbutils-0.10.so
%{_libdir}/libgstrtsp-0.10.so
%{_libdir}/libgstsdp-0.10.so
%{_libdir}/libgstfft-0.10.so
%{_libdir}/libgstapp-0.10.so
# pkg-config files
%{_libdir}/pkgconfig/*.pc

%files tools
%manifest gst-plugins-base-tools.manifest
%defattr(-,root,root,-)
# helper programs
%{_bindir}/gst-discoverer-0.10
%exclude %{_bindir}/gst-visualise-0.10
%exclude %{_mandir}/man1/gst-visualise-0.10*

