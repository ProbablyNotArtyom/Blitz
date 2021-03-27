Name:		pciutils
Version:	2.1.8
Release: 	1
Source:		ftp://atrey.karlin.mff.cuni.cz/pub/linux/pci/%{name}-%{version}.tar.gz
Copyright:	GNU GPL
Buildroot: 	/tmp/%{name}-%{version}-root
ExclusiveOS: 	Linux
Summary: 	Linux PCI Utilities
Summary(pl): 	Narz�dzia do manipulacji ustawieniami urz�dze� PCI
Group: 		Utilities/System

%description
This package contains various utilities for inspecting and
setting of devices connected to the PCI bus. Requires kernel
version 2.1.82 or newer (supporting the /proc/bus/pci interface).

%description -l pl
Pakiet zawiera narz�dzia do ustawiania i odczytywania informacji
o urz�dzeniach pod��czonych do szyny PCI w Twoim komputerze.
Wymaga kernela 2.1.82 lub nowszego (udost�pniaj�cego odpowiednie
informacje poprzez /proc/bus/pci).

%description -l de
Dieses Paket enth�lt verschiedene Programme zum Anzeigen und
Einstellen von PCI-Bus Erweiterungen.  Ben�tigt wird ein Kernel
Version 2.1.82 oder neuer (mit Unterst�tzung f�r die /proc/bus/pci
Schnittstelle).

%prep
%setup -q

%build
make OPT="$RPM_OPT_FLAGS"

%install
rm -rf $RPM_BUILD_ROOT
make install PREFIX=$RPM_BUILD_ROOT/usr ROOT=$RPM_BUILD_ROOT/ \
     MANDIR=$RPM_BUILD_ROOT/%{_mandir}

%files
%defattr(0644, root, root, 0755)
%attr(0644, root, man) %{_mandir}/man8/*
%attr(0711, root, root) /sbin/*
%config /usr/share/pci.ids
%doc README ChangeLog pciutils.lsm

%clean
rm -rf $RPM_BUILD_ROOT

%changelog
* Tue Sep 29 1998 Krzysztof G. Baranowski <kgb@knm.org.pl>
[1.07-1]
- build from non-root account against glibc-2.0
- written spec from scratch
