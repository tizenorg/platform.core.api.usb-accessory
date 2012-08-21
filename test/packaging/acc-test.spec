Name:       acc-test
Summary:    A test program for usb accessory
Version:	0.0.1
Release:    1
Group:      TO_BE/FILLED_IN
License:    TO BE FILLED IN
Source0:    %{name}-%{version}.tar.gz
BuildRequires:  cmake
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(capi-system-usb-accessory)
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(ecore-x)
BuildRequires:  pkgconfig(elementary)
BuildRequires:  pkgconfig(appcore-efl)

%prep
%setup -q


%build
MAJORVER=`echo %{version} | awk 'BEGIN {FS="."}{print $1}'`
cmake . -DCMAKE_INSTALL_PREFIX=/usr -DFULLVER=%{version} -DMAJORVER=${MAJORVER}

make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install

%post
chown 5000:5000 /opt/apps/acc_test/bin/acc_test


%files
%attr(644,root,root) /opt/share/applications/acc_test.desktop
%attr(555,root,root) /opt/apps/acc_test/bin/acc_test
