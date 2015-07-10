Name:       cert-checker
Summary:    OCSP checking on application installation
Version:    0.0.1
Release:    1
Group:      System/Security
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz
Source1:    %{name}.manifest
BuildRequires: cmake
BuildRequires: zip
BuildRequires: pkgconfig(db-util)
BuildRequires: pkgconfig(icu-i18n)
BuildRequires: pkgconfig(glib-2.0)
BuildRequires: pkgconfig(notification)
BuildRequires: pkgconfig(dbus-1)
BuildRequires: pkgconfig(dbus-glib-1)
BuildRequires: pkgconfig(cert-svc-vcore)
BuildRequires: pkgconfig(key-manager)
BuildRequires: pkgconfig(libsystemd-journal)
BuildRequires: pkgconfig(libtzplatform-config)
BuildRequires: pkgconfig(sqlite3)
BuildRequires: boost-devel

%description
Cert-checker

%package -n cert-checker-tests
Summary:    Internal test for cert-checker
Group:      Development
Requires:   boost-test
Requires:   cert-checker = %{version}-%{release}

%description -n cert-checker-tests
Internal test for cert-checker implementation.

%prep
%setup -q

%build
export CFLAGS="$CFLAGS"
export CXXFLAGS="$CXXFLAGS"
export FFLAGS="$FFLAGS"
export LDFLAGS+="-Wl,--rpath=%{_libdir} "

%cmake . -DVERSION=%{version} \
        -DDB_INSTALL_DIR=%{TZ_SYS_DB} \
        -DCMAKE_BUILD_TYPE=%{?build_type:%build_type}%{!?build_type:RELEASE} \
        -DCMAKE_VERBOSE_MAKEFILE=ON \
        -DTEST_APP_SIGNATURES_DIR="/root/cert-checker-test" \
        -DSYSTEMD_UNIT_DIR=%{_unitdir}

make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/usr/share/license
cp LICENSE %{buildroot}/usr/share/license/%{name}
%make_install
cp -a %{SOURCE1} %{buildroot}%{_datadir}/

%make_install
mkdir -p %{buildroot}%{_unitdir}/multi-user.target.wants
ln -s ../cert-checker.service %{buildroot}%{_unitdir}/multi-user.target.wants/cert-checker.service

%clean
rm -rf %{buildroot}

%post
systemctl daemon-reload
if [ $1 = 1 ]; then
    # installation
    systemctl start cert-checker.service
fi

if [ $1 = 2 ]; then
    # update
    systemctl restart cert-checker.service
fi


%preun
if [ $1 = 0 ]; then
    # unistall
    systemctl stop cert-checker.service
fi

%postun
if [ $1 = 0 ]; then
    # unistall
    systemctl daemon-reload
fi


%files
%{_bindir}/cert-checker
%manifest %{_datadir}/%{name}.manifest
%{_datadir}/license/%{name}
%config(noreplace) %attr(0600,root,root) %{TZ_SYS_DB}/.cert-checker.db
%{_unitdir}/cert-checker.service
%{_unitdir}/multi-user.target.wants/cert-checker.service

%files -n cert-checker-tests
%defattr(-,root,root,-)
%{_bindir}/cert-checker-tests
%{TZ_SYS_DB}/.cert-checker-test.db
/root/cert-checker-test/*/*.xml
