Name:       cert-checker
Summary:    OCSP checking on application installation
Version:    0.1.1
Release:    1
Group:      Security/Certificate Management
License:    Apache-2.0 and BSL-1.0
Source0:    %{name}-%{version}.tar.gz
BuildRequires: cmake
BuildRequires: zip
BuildRequires: gettext
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

%global sbin_dir         %{_sbindir}
%global bin_dir          %{?TZ_SYS_BIN:%TZ_SYS_BIN}%{!?TZ_SYS_BIN:%_bindir}
%global root_dir         %{?TZ_SYS_ROOT:%TZ_SYS_ROOT}%{!?TZ_SYS_ROOT:/root}
%global ro_data_dir      %{?TZ_SYS_RO_SHARE:%TZ_SYS_RO_SHARE}%{!?TZ_SYS_RO_SHARE:%_datadir}
%global db_dir           %{?TZ_SYS_DB:%TZ_SYS_DB}%{!?TZ_SYS_DB:/opt/dbspace}

# service macro
%global service_name     %{name}
%global service_stream   /tmp/.%{service_name}.socket
%global service_user     security_fw
%global service_group    security_fw

# popup macro
%global popup_stream     /tmp/.%{service_name}-popup.socket
%global popup_env        /run/tizen-system-env
%global popup_unitdir    %{_unitdir_user}

%package -n lib%{name}-common
Summary:    Common Library package for %{name}
License:    Apache-2.0
Group:      Security/Libraries
Requires(post):   %{sbin_dir}/ldconfig
Requires(postun): %{sbin_dir}/ldconfig

%description -n lib%{name}-common
cert-checker common library package.

%package -n lib%{name}-client
Summary:    Client Library package for %{name}
License:    Apache-2.0
Group:      Security/Libraries
Requires:   lib%{name}-common = %{version}-%{release}
Requires(post):   %{sbin_dir}/ldconfig
Requires(postun): %{sbin_dir}/ldconfig

%description -n lib%{name}-client
cert-checker client library package.

%package devel
Summary: Development files for %{name}
License: Apache-2.0
Group:   Security/Development
BuildRequires: pkgconfig(capi-base-common)
Requires:      %{name} = %{version}-%{release}

%description devel
cert-checker development files.

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
    -DTEST_APP_SIGNATURES_DIR="%{root_dir}/cert-checker-test" \
    -DSYSTEMD_UNIT_DIR=%{_unitdir} \
    -DBIN_DIR=%{bin_dir} \
    -DDB_INSTALL_DIR=%{db_dir} \
    -DPOPUP_ENV_PATH=%{popup_env} \
    -DPOPUP_STREAM=%{popup_stream} \
    -DPOPUP_SYSTEMD_UNIT_DIR=%{popup_unitdir}

make %{?_smp_mflags}

%install
%make_install
%find_lang %{name}

%install_service sockets.target.wants %{name}.socket
mkdir -p %{buildroot}%{popup_unitdir}/sockets.target.wants
ln -s ../%{name}-popup.socket %{buildroot}%{popup_unitdir}/sockets.target.wants/%{name}-popup.socket

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

%post -n lib%{name}-common -p %{sbin_dir}/ldconfig
%post -n lib%{name}-client -p %{sbin_dir}/ldconfig
%postun -n lib%{name}-common -p %{sbin_dir}/ldconfig
%postun -n lib%{name}-client -p %{sbin_dir}/ldconfig

%files -f %{name}.lang
%manifest %{name}.manifest
%license LICENSE
%attr(0600,%{service_user},%{service_group}) %{db_dir}/.%{name}.db
%{bin_dir}/%{name}
%{bin_dir}/%{name}-popup
%{_unitdir}/%{name}.service
%{_unitdir}/%{name}.socket
%{_unitdir}/sockets.target.wants/%{name}.socket
%{popup_unitdir}/%{name}-popup.service
%{popup_unitdir}/%{name}-popup.socket
%{popup_unitdir}/sockets.target.wants/%{name}-popup.socket

%files -n lib%{name}-common
%manifest %{name}-common.manifest
%license LICENSE
%{_libdir}/lib%{name}-common.so.*

%files -n lib%{name}-client
%manifest %{name}-client.manifest
%license LICENSE
%{_libdir}/lib%{name}-client.so.*

%files devel
%{_libdir}/pkgconfig/%{name}.pc
%{_libdir}/lib%{name}-common.so
%{_libdir}/lib%{name}-client.so
%{_includedir}/cchecker/ocsp.h

%files -n %{name}-tests
%manifest %{name}-tests.manifest
%license LICENSE LICENSE.BSL-1.0
%defattr(-,%{service_user},%{service_group},-)
%{bin_dir}/%{name}-tests
%{bin_dir}/%{name}-tests-logic
%{bin_dir}/%{name}-tests-client
%{bin_dir}/%{name}-popup-test
%{db_dir}/.%{name}-test.db
%{root_dir}/%{name}-test/*/*.xml
