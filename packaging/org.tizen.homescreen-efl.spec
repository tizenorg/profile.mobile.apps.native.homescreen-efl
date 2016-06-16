Name:       org.tizen.homescreen-efl
#VCS_FROM:   profile/mobile/apps/native/homescreen-efl#cd7e34b7a1ad7194a9a7c3e55a90fd67652461c0
#RS_Ver:    20160616_1 
Summary:    homescreen-efl
Version:    1.0.0
Release:    1
Group:      Applications/Core Applications
License:    Flora-1.1
Source0:    %{name}-%{version}.tar.gz

ExcludeArch:  aarch64 x86_64
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
#VCS_FROM:   profile/mobile/apps/native/homescreen-efl#cd7e34b7a1ad7194a9a7c3e55a90fd67652461c0
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
