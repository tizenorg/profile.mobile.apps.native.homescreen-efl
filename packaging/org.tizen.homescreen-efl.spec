Name:    org.tizen.homescreen-efl
Summary: Homescreen-Efl
Version: 0.0.1
Release: 0.1
Group:   Applications/Core Applications
License: Flora-1.1
Source0: %{name}-%{version}.tar.gz

%if "%{?tizen_profile_name}" == "wearable"
ExcludeArch: %{arm} %ix86 x86_64
%endif

%if "%{?tizen_profile_name}" == "tv"
ExcludeArch: %{arm} %ix86 x86_64
%endif

BuildRequires:  cmake
BuildRequires:  edje-tools
BuildRequires:  gettext-tools
BuildRequires:  coreutils
BuildRequires:  hash-signer
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
BuildRequires:  pkgconfig(widget_service)
BuildRequires:  pkgconfig(widget_viewer_evas)
BuildRequires:  pkgconfig(shortcut)
BuildRequires:  pkgconfig(capi-appfw-preference)
BuildRequires:  pkgconfig(libtzplatform-config)

%description
Tizen Homescreen Reference Implementation

%prep
%setup -q

%build

%define _pkg_dir %{TZ_SYS_RO_APP}/%{name}
%define _pkg_shared_dir %{_pkg_dir}/shared
%define _pkg_data_dir %{_pkg_dir}/data
%define _sys_icons_dir %{_pkg_shared_dir}/res
%define _sys_packages_dir %{TZ_SYS_RO_PACKAGES}
%define _sys_license_dir %{TZ_SYS_SHARE}/license

cd CMake
cmake . -DINSTALL_PREFIX=%{_pkg_dir} \
	-DSYS_ICONS_DIR=%{_sys_icons_dir} \
	-DSYS_PACKAGES_DIR=%{_sys_packages_dir}
make %{?jobs:-j%jobs}
cd -

%install
cd CMake
%make_install
cd -

%define tizen_sign 1
%define tizen_sign_base %{_pkg_dir}
%define tizen_sign_level public
%define tizen_author_sign 1
%define tizen_dist_sign 1
mkdir -p %{buildroot}%{_sys_license_dir}
cp LICENSE %{buildroot}%{_sys_license_dir}/homescreen-efl
%find_lang homescreen-efl

%files -f homescreen-efl.lang
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_pkg_dir}/bin/homescreen-efl
%{_pkg_dir}/res/edje/*.edj
%{_pkg_dir}/res/images/*.png
%{_pkg_dir}/res/images/livebox/*.png
%{_sys_icons_dir}/homescreen-efl.png
%{_sys_packages_dir}/%{name}.xml
%{_sys_license_dir}/homescreen-efl
%{_pkg_dir}/author-signature.xml
%{_pkg_dir}/signature1.xml
