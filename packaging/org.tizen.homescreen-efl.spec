%define _optdir /opt

%define _appdir /opt/usr/apps
%define _packagedir %{_appdir}/%{_package_name}

Name:       org.tizen.homescreen-efl
Summary: Homescreen
Version: 0.0.1
Release: 0.1
Group:      TO_BE/FILLED_IN
License:    Flora License
Source0:    %{name}-%{version}.tar.gz

%if "%{?tizen_profile_name}" == "wearable"
ExcludeArch: %{arm} %ix86 x86_64
%endif

%if "%{?tizen_profile_name}" == "tv"
ExcludeArch: %{arm} %ix86 x86_64
%endif

BuildRequires:  pkgconfig(appsvc)
BuildRequires:  pkgconfig(badge)
BuildRequires:  pkgconfig(capi-appfw-application)
BuildRequires:  pkgconfig(capi-system-info)
BuildRequires:  pkgconfig(capi-appfw-package-manager)
BuildRequires:  pkgconfig(capi-appfw-app-manager)
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(elementary)
BuildRequires:  pkgconfig(shortcut)
BuildRequires:  pkgconfig(syspopup-caller)
BuildRequires:  pkgconfig(widget_viewer)
BuildRequires:  pkgconfig(widget_service)
BuildRequires:  pkgconfig(sqlite3)
BuildRequires:  pkgconfig(db-util)
BuildRequires:  pkgconfig(notification)
BuildRequires:  pkgconfig(capi-system-system-settings)
BuildRequires:  cmake
BuildRequires:  edje-tools
BuildRequires:  gettext-tools
BuildRequires:  coreutils
BuildRequires:  pkgconfig(widget_service)
BuildRequires:  pkgconfig(widget_viewer_evas)
BuildRequires:  pkgconfig(shortcut)
BuildRequires:  pkgconfig(capi-appfw-preference)

%description
Tizen Homescreen

%prep
%setup -q

%build
%if 0%{?tizen_build_binary_release_type_eng}
export CFLAGS="${CFLAGS} -DTIZEN_ENGINEER_MODE"
export CXXFLAGS="${CXXFLAGS} -DTIZEN_ENGINEER_MODE"
export FFLAGS="${FFLAGS} -DTIZEN_ENGINEER_MODE"
%endif

%cmake . -DCMAKE_INSTALL_PREFIX=%{_prefix}
CFLAGS="${CFLAGS} -Wall" LDFLAGS="${LDFLAGS} -Wl,--hash-style=both -Wl,--as-needed -fpie"
make %{?jobs:-j%jobs}

%install
%make_install
%define tizen_sign 1
%define tizen_sign_base %{_packagedir}
%define tizen_sign_level platform
%define tizen_author_sign 1
%define tizen_dist_sign 1
mkdir -p %{buildroot}%{_datadir}
mkdir -p %{buildroot}/usr/share/license
cp LICENSE %{buildroot}/usr/share/license/homescreen-efl

%post
killall -9 homescreen-efl
dlogutil -c
#/usr/bin/signing-client/hash-signer-client.sh -a -d -p platform %{_packagedir}

%files
%manifest %{name}.manifest
%defattr(-,root,root,-)

%{_optdir}/usr/apps/%{name}/bin/homescreen-efl
%{_optdir}/usr/apps/%{name}/res/edje/*.edj
%{_optdir}/usr/apps/%{name}/res/images/*.png
%{_optdir}/usr/apps/%{name}/res/images/livebox/*.png
%{_optdir}/usr/apps/%{name}/res/locale/*/*/*.mo
%{_datadir}/packages/%{name}.xml
%{_sysconfdir}/smack/accesses.d/*.efl
%dir %{_optdir}/usr/apps/%{name}/data
%attr(777, root, root) %{_optdir}/usr/apps/%{name}/data
/usr/share/license/homescreen-efl