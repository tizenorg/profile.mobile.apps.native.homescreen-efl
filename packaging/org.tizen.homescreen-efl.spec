Name:       org.tizen.homescreen-efl
#VCS_FROM:   profile/mobile/apps/native/homescreen-efl#dd8fc18cf763e9020486037098b0c47980831561
#RS_Ver:    20160518_1 
Summary:    N/A
Version:    1.0.0
Release:    1
Group:      N/A
License:    N/A
Source0:    %{name}-%{version}.tar.gz

BuildRequires:  pkgconfig(libtzplatform-config)
Requires(post):  /usr/bin/tpk-backend

%define internal_name org.tizen.homescreen-efl
%define preload_tpk_path %{TZ_SYS_RO_APP}/.preload-tpk 

%ifarch i386 i486 i586 i686 x86_64
%define target i386
%else
%ifarch arm armv7l aarch64
%define target arm
%else
%define target noarch
%endif
%endif

%description
profile/mobile/apps/native/homescreen-efl#dd8fc18cf763e9020486037098b0c47980831561
This is a container package which have preload TPK files

%prep
%setup -q

%build

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/%{preload_tpk_path}
install %{internal_name}-%{version}-%{target}.tpk %{buildroot}/%{preload_tpk_path}/

%post

%files
%defattr(-,root,root,-)
%{preload_tpk_path}/*
