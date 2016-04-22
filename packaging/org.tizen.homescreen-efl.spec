Name:       org.tizen.homescreen-efl
#VCS_FROM:   profile/mobile/apps/native/homescreen-efl#d245e469d8a339bda12d4baa6bb8fa7ee9934741
#RS_Ver:    20160422_1 
Summary:    Homescreen-Efl
Version:    0.0.1
Release:    1
Group:      Applications/Core Applications
License:    Flora-1.1
Source0:    %{name}-%{version}.tar.gz

BuildRequires:  pkgconfig(libtzplatform-config)
Requires(post):  /usr/bin/tpk-backend

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
This is a container package which have preload TPK files

%prep
%setup -q

%build

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/%{preload_tpk_path}
install %{name}-%{version}-%{target}.tpk %{buildroot}/%{preload_tpk_path}/

%post

%files
%defattr(-,root,root,-)
%{preload_tpk_path}/*
