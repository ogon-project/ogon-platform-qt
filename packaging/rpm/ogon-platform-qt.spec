#
# This file and all modifications and additions to the pristine
# package are under the same license as the package itself.
#

# norootforbuild

Name:            ogon-platform-qt
Version:         1.0.0
Release:         1
Summary:	       A qt platform for ogon
Group:           Productivity/Networking/RDP
License:         Apache-2.0
Url:             http://ogon-project.com
BuildRequires:   cmake >= 2.8.12
BuildRequires:   pkg-config
BuildRequires:   freerdp2-devel
BuildRequires:   ogon-devel
BuildRequires:   glib2-devel
BuildRequires:   protobuf-devel
BuildRequires:   openssl-devel
BuildRequires:   libogon-message1-devel
BuildRequires:   libxcb-devel
BuildRequires:   libqt5-qtbase-common-devel
BuildRequires:   libqt5-qtbase-private-headers-devel
BuildRequires:   libxkbcommon-devel
BuildRequires:   xorg-x11-proto-devel
BuildRequires:   libqt5-qttools-devel
BuildRequires:   libXcursor-devel
Source:          ogon-platform-qt.tar.xz
BuildRoot:      %{_tmppath}/%{name}-%{version}-build

%description
The ogon Remote Desktop Services provide graphical remote access to
desktop sessions and applications using the Remote Desktop Protocol
(RDP) and supports most modern RDP protocol extensions, bitmap
compression codecs and device redirections. ogon is build on the
FreeRDP library and is compatible with virtually any existing Remote
Desktop Client.

This package contains the qt backend for ogon. Which is a qt platform
module.

%package devel 
Summary: Develoment files for the ogon qt backend
Group:   Development/Productivity
Requires: ogon-platform-qt = %{version}
%description devel
The ogon Remote Desktop Services provide graphical remote access to
desktop sessions and applications using the Remote Desktop Protocol
(RDP) and supports most modern RDP protocol extensions, bitmap
compression codecs and device redirections. ogon is build on the
FreeRDP library and is compatible with virtually any existing Remote
Desktop Client.

This package contains development files and libraries for the qt backend.

%prep
%setup -q

%build
qmake-qt5
make %{?_smp_mflags}


%install
#export NO_BRP_STRIP_DEBUG=true
#export NO_DEBUGINFO_STRIP_DEBUG=true
#%%define __debug_install_post %{nil}
make INSTALL_ROOT=$RPM_BUILD_ROOT install
rm -rf $RPM_BUILD_ROOT/%{_bindir}/modeswitcher
rm -rf $RPM_BUILD_ROOT/%{_bindir}/simple_greeter

%clean
rm -rf $RPM_BUILD_ROOT

%post   devel -p /sbin/ldconfig
%postun devel -p /sbin/ldconfig

%files
%defattr(-,root,root,0755)
%dir %{_datarootdir}/qt5/translations/
%{_datarootdir}/qt5/translations/*.qm
%{_libdir}/qt5/plugins/platforms/libogon.so

# devel
%files devel
%defattr(-,root,root,0755)
%{_libdir}/cmake/qogon/
%{_libdir}/qt5/plugins/designer/libqogonwidgets_designer.so
%{_libdir}/libqogonwidgets.so
%{_libdir}/pkgconfig/qogon.pc
%{_includedir}/qt5/qogon

%changelog
* Wed Jun 08 2016 - bernhard.miklautz@thincast.com
- Initial version
