Name:       cert-checker
Summary:    OCSP checking on application installation
Version:    0.0.4
Release:    1
Group:      System/Security
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz
Source1:    %{name}.manifest
BuildRequires: cmake
BuildRequires: zip
BuildRequires: gettext-tools
BuildRequires: pkgconfig(capi-appfw-application)
BuildRequires: pkgconfig(cert-svc-vcore)
BuildRequires: pkgconfig(db-util)
BuildRequires: pkgconfig(dbus-1)
BuildRequires: pkgconfig(dbus-glib-1)
BuildRequires: pkgconfig(elementary)
BuildRequires: pkgconfig(glib-2.0)
BuildRequires: pkgconfig(icu-i18n)
BuildRequires: pkgconfig(key-manager)
BuildRequires: pkgconfig(libsystemd-journal)
BuildRequires: pkgconfig(libtzplatform-config)
BuildRequires: pkgconfig(sqlite3)
BuildRequires: pkgconfig(pkgmgr)
BuildRequires: pkgconfig(pkgmgr-info)
BuildRequires: boost-devel

%global TZ_SYS_DB        %{?TZ_SYS_DB:%TZ_SYS_DB}%{!?TZ_SYS_DB:/opt/dbspace}
%global TZ_SYS_ROOT      %{?TZ_SYS_ROOT:%TZ_SYS_ROOT}%{!?TZ_SYS_ROOT:/root}
%global TZ_SYS_RO_SHARE  %{?TZ_SYS_RO_SHARE:%TZ_SYS_RO_SHARE}%{!?TZ_SYS_RO_SHARE:/usr/share}
%global TZ_SYS_BIN       %{?TZ_SYS_BIN:%TZ_SYS_BIN}%{!?TZ_SYS_BIN:/usr/bin}

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
        -DTEST_APP_SIGNATURES_DIR="%{TZ_SYS_ROOT}/cert-checker-test" \
        -DSYSTEMD_UNIT_DIR=%{_unitdir}

make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/%{TZ_SYS_RO_SHARE}/license
cp LICENSE %{buildroot}/%{TZ_SYS_RO_SHARE}/license/%{name}
%make_install
cp -a %{SOURCE1} %{buildroot}%{TZ_SYS_RO_SHARE}/
%find_lang %{name}

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
chsmack -a System %{TZ_SYS_DB}/.cert-checker.db

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


%files -f %{name}.lang
%{TZ_SYS_BIN}/cert-checker
%{TZ_SYS_BIN}/cert-checker-popup
%manifest %{TZ_SYS_RO_SHARE}/%{name}.manifest
%{TZ_SYS_RO_SHARE}/license/%{name}
%config(noreplace) %attr(0600,root,root) %{TZ_SYS_DB}/.cert-checker.db
%{_unitdir}/cert-checker.service
%{_unitdir}/multi-user.target.wants/cert-checker.service

%files -n cert-checker-tests
%defattr(-,root,root,-)
%{TZ_SYS_BIN}/cert-checker-tests
%{TZ_SYS_BIN}/cert-checker-tests-logic
%{TZ_SYS_BIN}/cert-checker-popup-test
%{TZ_SYS_DB}/.cert-checker-test.db
%{TZ_SYS_ROOT}/cert-checker-test/*/*.xml
