#
# pngmeta -- Get Metadata from PNG images
#
%define version 1.10
%define name pngmeta
Summary: Display metadata information from PNG images
Name: %{name}
Version: %{version}
Release: 1
Copyright: Freely Distributable
Group: Applications/Graphics
Source: http://www.cs.ukc.ac.uk/people/staff/djb1/software/%{name}-%{version}.tar.gz
URL: http://www.cs.ukc.ac.uk/people/staff/djb1/software
Provides: pngmeta
Requires: libpng.so.2 zlib
BuildRoot: /tmp/rpmbuild_%{name}

%description
This small filter program extracts metadata from PNG images and
displays them as either HTML, SOIF, RDF/XML or simple fields and
values.

%prep
rm -rf $RPM_BUILD_ROOT
rm -rf $RPM_BUILD_DIR/%{name}-%{version}

%setup

%build
CFLAGS="$RPM_OPT_FLAGS" ./configure --prefix=/usr
make

%install
make install-strip prefix=$RPM_BUILD_ROOT/usr

%files

%doc %attr (-,root,root) NEWS
%doc %attr (-,root,root) README

%attr (-,root,root) /usr/man/man1/pngmeta.1
%attr (755,root,root) /usr/bin/pngmeta

%clean
rm -rf $RPM_BUILD_ROOT
rm -rf $RPM_BUILD_DIR/%{name}-%{version}
