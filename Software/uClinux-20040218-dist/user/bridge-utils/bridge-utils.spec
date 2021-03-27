Summary: Utilities for configuring the linux ethernet bridge.
Name: bridge-utils
Version: 0.9.6
Release: 1
Copyright: GPL
Group: System Environment/Base
Source0: http://bridge.sourceforge.net/bridge-utils/bridge-utils-%{PACKAGE_VERSION}.tar.gz
BuildRoot: /var/tmp/%{name}-%{version}-root

%description
This package contains utilities for configuring the linux ethernet
bridge. The linux ethernet bridge can be used for connecting multiple
ethernet devices together. The connecting is fully transparent: hosts
connected to one ethernet device see hosts connected to the other
ethernet devices directly.

Install bridge-utils if you want to use the linux ethernet bridge.

%package -n bridge-utils-devel
Summary: Utilities for configuring the linux ethernet bridge.
Group: Development/Libraries

%description -n bridge-utils-devel
The bridge-utils-devel package contains the header and object files
necessary for developing programs which use 'libbridge.a', the
interface to the linux kernel ethernet bridge. If you are developing
programs which need to configure the linux ethernet bridge, your
system needs to have these standard header and object files available
in order to create the executables.

Install bridge-utils-devel if you are going to develop programs which
will use the linux ethernet bridge interface library.

%prep
%setup -n bridge-utils

%build
./configure
make

%install
[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf $RPM_BUILD_ROOT
mkdir -p ${RPM_BUILD_ROOT}
mkdir -p ${RPM_BUILD_ROOT}%{_sbindir}
mkdir -p ${RPM_BUILD_ROOT}%{_includedir}
mkdir -p ${RPM_BUILD_ROOT}%{_libdir}
mkdir -p ${RPM_BUILD_ROOT}%{_mandir}/man8
install -m 0755 brctl/brctl ${RPM_BUILD_ROOT}/%{_sbindir}
gzip doc/brctl.8
install -m 0644  doc/brctl.8.gz ${RPM_BUILD_ROOT}%{_mandir}/man8
install -m 0644 libbridge/libbridge.h ${RPM_BUILD_ROOT}%{_includedir}
install -m 0644 libbridge/libbridge.a ${RPM_BUILD_ROOT}%{_libdir}/

%clean
[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf $RPM_BUILD_ROOT

%files
%defattr (-,root,root)
%doc AUTHORS COPYING doc/FAQ doc/HOWTO doc/RPM-GPG-KEY
%{_sbindir}/brctl
%{_mandir}/man8/brctl.8.gz

%files -n bridge-utils-devel
%defattr (-,root,root)
%{_includedir}/libbridge.h
%{_libdir}/libbridge.a

%changelog
* Wed Nov 07 2001 Matthew Galgoci <mgalgoci@redhat.com>
- initial cleanup of spec file from net release
