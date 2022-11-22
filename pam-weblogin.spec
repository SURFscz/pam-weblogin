Name:		pam-weblogin
Version:	v0.1
%define rel 1
Release:	%{rel}%{?dist}
Summary:	PAM WebLogin plugin
Group:      System Environment/Base
License:    Apache License 2.0
URL:        https://github.com/SURFscz/pam-weblogin
%global source_dir %{name}-%{version}
Source0:		%{source_dir}.tar.gz
BuildRequires:	libcurl-devel pam-devel
Requires:	libcurl pam

%description
Allows a user to login via ssh to a server by logging in on a web page instead of (or in addition to) to usual ssh authentication process.

%prep
%autosetup

%build
make

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/etc/pam.d/
make install DESTDIR=%{buildroot}

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root,0750)
/usr/local/lib/security/*.so
/etc/pam.d/weblogin
/etc/pam-weblogin.conf

%doc README.md

%license LICENSE

%changelog
# let's skip this for now