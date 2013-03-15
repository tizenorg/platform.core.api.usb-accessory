Name:       acc_test
Summary:    A test program for usb accessory
Version:    0.0.2
Release:    1
Group:      TO_BE/FILLED_IN
License:    TO BE FILLED IN
Source0:    %{name}-%{version}.tar.gz
Source1:    acc_test.manifest
BuildRequires:  cmake
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(capi-system-usb-accessory)
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(ecore-x)
BuildRequires:  pkgconfig(elementary)
BuildRequires:  pkgconfig(appcore-efl)

%description

%prep
%setup -q


%build
MAJORVER=`echo %{version} | awk 'BEGIN {FS="."}{print $1}'`
cmake . -DCMAKE_INSTALL_PREFIX=/usr -DFULLVER=%{version} -DMAJORVER=${MAJORVER}

make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}

cp %{SOURCE1} .

%make_install

%post
chown 5000:5000 /opt/usr/apps/acc_test/bin/acc_test


%files
%manifest acc_test.manifest
%attr(644,root,root) /opt/share/packages/acc_test.xml
%attr(555,root,root) /opt/usr/apps/acc_test/bin/acc_test
