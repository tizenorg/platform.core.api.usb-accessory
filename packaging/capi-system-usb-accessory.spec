Name:           capi-system-usb-accessory
Version:        0.0.9
Release:        1
VCS:            platform/core/api/usb-accessory#accepted/tizen/20130520.100842-0-g0fb5aab8f1efb2d0fa391a0c1e2938b80aab2d45-dirty
License:        Apache-2.0
Summary:        A USB accessory library in TIZEN C API
Group:          System/API
Source0:        %{name}-%{version}.tar.gz
Source1:        capi-system-usb-accessory.manifest
BuildRequires:  cmake
BuildRequires:  pkgconfig(aul)
BuildRequires:  pkgconfig(capi-base-common)
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(vconf)
%description
A USB accessory library in TIZEN C API.

%package devel
Summary:        A accessory library in TIZEN C API (Development)
Group:          Development/System
Requires:       %{name} = %{version}

%description devel
%devel_desc

%prep
%setup -q


%build
cp %{SOURCE1} .
MAJORVER=`echo %{version} | awk 'BEGIN {FS="."}{print $1}'`
%cmake . -DFULLVER=%{version} -DMAJORVER=${MAJORVER}

make %{?_smp_mflags}

%install
%make_install

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%license LICENSE.Apache_v2
%manifest capi-system-usb-accessory.manifest
%{_libdir}/libcapi-system-usb-accessory.so.*

%files devel
%{_includedir}/system/usb_accessory.h
%{_libdir}/pkgconfig/*.pc
%{_libdir}/libcapi-system-usb-accessory.so
