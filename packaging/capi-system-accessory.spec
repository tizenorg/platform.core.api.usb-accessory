Name:       capi-system-usb-accessory
Summary:    A usb accessory library in TIZEN C API
Version: 0.0.1
Release:    3
Group:      TO_BE/FILLED_IN
License:    TO BE FILLED IN
Source0:    %{name}-%{version}.tar.gz
BuildRequires:  cmake
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(capi-base-common)
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(vconf)
BuildRequires:  pkgconfig(aul)
#BuildRequires:  pkgconfig(ecore-x)
#BuildRequires:  pkgconfig(elementary)
#BuildRequires:  pkgconfig(appcore-efl)

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
MAJORVER=`echo %{version} | awk 'BEGIN {FS="."}{print $1}'`
cmake . -DCMAKE_INSTALL_PREFIX=/usr -DFULLVER=%{version} -DMAJORVER=${MAJORVER}

make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install

#%post -p /sbin/ldconfig
%post
/sbin/ldconfig
#chown 5000:5000 /opt/apps/acc_test/bin/acc_test

#%postun -p /sbin/ldconfig
%postun
/sbin/ldconfig


%files
%{_libdir}/libcapi-system-usb-accessory.so.*
#%attr(644,root,root) /opt/share/applications/acc_test.desktop
#%attr(555,root,root) /opt/apps/acc_test/bin/acc_test

%files devel
%{_includedir}/system/usb_accessory.h
%{_libdir}/pkgconfig/*.pc
%{_libdir}/libcapi-system-usb-accessory.so


