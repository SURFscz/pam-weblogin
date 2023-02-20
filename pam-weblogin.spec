Name:		    pam-weblogin
%{!?pwl_version: %define pwl_version 0      }
%{!?pwl_distro:  %define pwl_distro unknown }
Version:	    %{pwl_version}
Release:	    %{pwl_distro}
Summary:	    PAM WebLogin plugin
Group:          System Environment/Base
License:        Apache License 2.0
URL:            https://github.com/SURFscz/pam-weblogin
Source0:        %{name}-%{version}.source.tar.xz
BuildRequires:	make gcc libcurl-devel pam-devel
Requires:	    libcurl pam

%description
Allows a user to login via ssh to a server by logging in on a web page instead of (or in addition to) to usual ssh authentication process.

%prep
%autosetup

%build
make
pandoc README.md -t plain > README

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/etc/pam.d/
mkdir -p %{buildroot}/usr/share/doc/pam-weblogin/
make install DESTDIR=%{buildroot}

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root,0750)
/usr/local/lib/security/*.so
/etc/pam.d/weblogin
/etc/pam-weblogin.conf

%doc README

%license LICENSE

%changelog
# let's skip this for now