Name:       cert-checker
Summary:    OCSP checking on application installation
Version:    0.1.1
Release:    1
Group:      Security/Certificate Management
License:    Apache-2.0 and BSL-1.0
Source0:    %{name}-%{version}.tar.gz
BuildRequires: cmake
BuildRequires: zip
BuildRequires: gettext-tools
BuildRequires: pkgconfig(capi-appfw-application)
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
Requires: security-config
Requires: lib%{name}-common = %{version}-%{release}

%description
Check OCSP validation at app install/uninstall time.

%global SBIN_DIR         /sbin
%global TZ_SYS_DB        %{?TZ_SYS_DB:%TZ_SYS_DB}%{!?TZ_SYS_DB:/opt/dbspace}
%global TZ_SYS_ROOT      %{?TZ_SYS_ROOT:%TZ_SYS_ROOT}%{!?TZ_SYS_ROOT:/root}
%global TZ_SYS_RO_SHARE  %{?TZ_SYS_RO_SHARE:%TZ_SYS_RO_SHARE}%{!?TZ_SYS_RO_SHARE:/usr/share}
%global TZ_SYS_BIN       %{?TZ_SYS_BIN:%TZ_SYS_BIN}%{!?TZ_SYS_BIN:/usr/bin}

%global DB_INST_DIR      %{TZ_SYS_DB}/%{name}

# service macro
%global service_name     %{name}
%global service_stream   /tmp/.%{service_name}.socket
%global service_user     security_fw
%global service_group    security_fw

# common lib package
%package -n lib%{name}-common
Summary:    Common Library package for %{name}
License:    Apache-2.0
Group:      Security/Libraries
Requires:   %{SBIN_DIR}/ldconfig

%description -n lib%{name}-common
cert-checker common library package.

# client lib package
%package -n lib%{name}-client
Summary:    Client Library package for %{name}
License:    Apache-2.0
Group:      Security/Libraries
Requires:   %{SBIN_DIR}/ldconfig
Requires:   lib%{name}-common = %{version}-%{release}

%description -n lib%{name}-client
cert-checker client library package.

# devel package
%package devel
Summary: Development files for %{name}
License: Apache-2.0
Group:   Security/Development
BuildRequires: pkgconfig(capi-base-common)
Requires:      %{name} = %{version}-%{release}

%description devel
cert-checker development files.

# test package
%package -n %{name}-tests
Summary:    Internal test for cert-checker
License:    Apache-2.0 and BSL-1.0
Group:      Security/Testing
Requires:   boost-test
Requires:   %{name} = %{version}-%{release}
Requires:   lib%{name}-common = %{version}-%{release}
Requires:   lib%{name}-client = %{version}-%{release}

%description -n %{name}-tests
Internal test for cert-checker implementation.

%prep
%setup -q

%build
export CFLAGS="$CFLAGS"
export CXXFLAGS="$CXXFLAGS"
export FFLAGS="$FFLAGS"
export LDFLAGS+="-Wl,--rpath=%{_libdir} "

%cmake . \
    -DVERSION=%{version} \
    -DCMAKE_BUILD_TYPE=%{?build_type:%build_type}%{!?build_type:RELEASE} \
    -DMAKE_VERBOSE_MAKEFILE=ON \
    -DSERVICE_NAME=%{service_name} \
    -DSERVICE_STREAM=%{service_stream} \
    -DSERVICE_USER=%{service_user} \
    -DSERVICE_GROUP=%{service_group} \
    -DINCLUDE_INSTALL_DIR:PATH=%{_includedir} \
    -DTEST_APP_SIGNATURES_DIR="%{TZ_SYS_ROOT}/cert-checker-test" \
    -DSYSTEMD_UNIT_DIR=%{_unitdir} \
    -DBIN_DIR=%{TZ_SYS_BIN} \
    -DDB_INSTALL_DIR=%{DB_INST_DIR}

make %{?jobs:-j%jobs}

%install
%make_install
%find_lang %{name}
%install_service sockets.target.wants %{name}.socket


%clean
rm -rf %{buildroot}

%post
systemctl daemon-reload

# install
if [ $1 = 1 ]; then
    systemctl start %{name}.socket
fi
# upgrade / reinstall
if [ $1 = 2 ]; then
    systemctl stop %{name}.service
    systemctl restart %{name}.socket
fi

chsmack -a System %{DB_INST_DIR}
chsmack -a System %{DB_INST_DIR}/.%{name}.db

%preun
# uninstall
if [ $1 = 0 ]; then
    systemctl stop %{name}.service
    systemctl stop %{name}.socket
fi

%postun
if [ $1 = 0 ]; then
    systemctl daemon-reload
fi

%post -n lib%{name}-common -p %{SBIN_DIR}/ldconfig
%postun -n lib%{name}-common -p %{SBIN_DIR}/ldconfig

%files -f %{name}.lang
%manifest %{name}.manifest
%license LICENSE
%dir %attr(0700,%{service_user},%{service_group}) %{DB_INST_DIR}
%config(noreplace) %attr(0600,%{service_user},%{service_group}) %{DB_INST_DIR}/.%{name}.db
%{_unitdir}/%{name}.service
%{_unitdir}/%{name}.socket
%{_unitdir}/sockets.target.wants/%{name}.socket
%{TZ_SYS_BIN}/%{name}
%{TZ_SYS_BIN}/%{name}-popup

%files -n lib%{name}-common
%defattr(-,root,root,-)
%manifest %{name}-common.manifest
%license LICENSE
%{_libdir}/lib%{name}-common.so.*

%files -n lib%{name}-client
%defattr(-,root,root,-)
%manifest %{name}-client.manifest
%license LICENSE
%{_libdir}/lib%{name}-client.so.*

%files devel
%defattr(-,root,root,-)
%{_libdir}/pkgconfig/%{name}.pc
%{_libdir}/lib%{name}-common.so
%{_libdir}/lib%{name}-client.so
%{_includedir}/cchecker/ocsp.h

%files -n %{name}-tests
%defattr(-,%{service_user},%{service_group},-)
%license LICENSE LICENSE.BSL-1.0
%{TZ_SYS_BIN}/%{name}-tests
%{TZ_SYS_BIN}/%{name}-tests-logic
%{TZ_SYS_BIN}/%{name}-tests-client
%{TZ_SYS_BIN}/%{name}-popup-test
%{DB_INST_DIR}/.%{name}-test.db
%{TZ_SYS_ROOT}/%{name}-test/*/*.xml
