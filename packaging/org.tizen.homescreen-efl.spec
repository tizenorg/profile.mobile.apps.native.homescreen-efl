%define _appdir /usr/apps
%define _packagedir %{_appdir}/%{_package_name}

Name:       org.tizen.homescreen-efl
Summary: Homescreen-Efl
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
%if 0%{?sec_build_binary_debug_enable}
export CFLAGS="${CFLAGS} -DTIZEN_DEBUG_ENABLE"
export CXXFLAGS="${CXXFLAGS} -DTIZEN_DEBUG_ENABLE"
export FFLAGS="${FFLAGS} -DTIZEN_DEBUG_ENABLE"
%endif

%ifarch %{arm}
CXXFLAGS+=" -D_ARCH_ARM_ -mfpu=neon"
%endif

%cmake . -DCMAKE_INSTALL_PREFIX=%{_prefix}
CFLAGS="${CFLAGS} -Wall" LDFLAGS="${LDFLAGS} -Wl,--hash-style=both -Wl,--as-needed -fpie"
make %{?jobs:-j%jobs}

%install
%make_install
%define tizen_sign 1
%define tizen_sign_base %{_packagedir}
%define tizen_sign_level public
%define tizen_author_sign 1
%define tizen_dist_sign 1
mkdir -p %{buildroot}%{_datadir}
mkdir -p %{buildroot}/usr/share/license
cp LICENSE %{buildroot}/usr/share/license/homescreen-efl

%post

%files
%manifest %{name}.manifest
%defattr(-,root,root,-)
/usr/apps/%{name}/bin/homescreen-efl
/usr/apps/%{name}/res/edje/*.edj
/usr/apps/%{name}/res/images/*.png
/usr/apps/%{name}/res/images/livebox/*.png
/usr/apps/%{name}/res/locale/*/*/*.mo
/usr/share/icons/default/small/homescreen-efl.png
%{_datadir}/packages/%{name}.xml
%dir /usr/apps/%{name}/data
%attr(777, root, root) /usr/apps/%{name}/data
/usr/share/license/homescreen-efl
