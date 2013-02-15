Name:       capi-system-usb-accessory
Summary:    A usb accessory library in TIZEN C API
Version:    0.0.6
Release:    1
Group:      framework-api
License:    APLv2
Source0:    %{name}-%{version}.tar.gz
Source1:    capi-system-usb-accessory.manifest
BuildRequires:  cmake
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(capi-base-common)
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(vconf)
BuildRequires:  pkgconfig(aul)

Requires(post): /sbin/ldconfig  
Requires(postun): /sbin/ldconfig

%description


%package devel
Summary:  A accessory library in TIZEN C API (Development)
Group:    TO_BE/FILLED_IN
Requires: %{name} = %{version}-%{release}

%description devel



%prep
%setup -q


%build
cp %{SOURCE1} .
MAJORVER=`echo %{version} | awk 'BEGIN {FS="."}{print $1}'`
cmake . -DCMAKE_INSTALL_PREFIX=/usr -DFULLVER=%{version} -DMAJORVER=${MAJORVER}

make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install

%post
/sbin/ldconfig

%postun
/sbin/ldconfig


%files
%manifest capi-system-usb-accessory.manifest
%{_libdir}/libcapi-system-usb-accessory.so.*

%files devel
%{_includedir}/system/usb_accessory.h
%{_libdir}/pkgconfig/*.pc
%{_libdir}/libcapi-system-usb-accessory.so
