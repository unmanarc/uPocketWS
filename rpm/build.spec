# Supported build targets:
#   - RHEL / CentOS / Rocky / AlmaLinux 7 (cmake3 + openssl11 from EPEL)
#   - RHEL / Rocky / AlmaLinux 8 (dnf4, openssl 1.1)
#   - RHEL / Rocky / AlmaLinux 9 (dnf4, openssl 3)
#   - RHEL / Rocky / AlmaLinux 10 (dnf5, openssl 3, gcc 14)
#   - Fedora 33 and newer
%define name uPocketWS
%define version 3.0.0
%define build_timestamp %{lua: print(os.date("%Y%m%d"))}

Name:           %{name}
Version:        %{version}
Release:        %{build_timestamp}.git%{?dist}
Summary:        Unmanarc's Pocket Web Server
License:        AGPL
URL:            https://github.com/unmanarc/uPocketWS
Source0:        https://github.com/unmanarc/uPocketWS/archive/main.tar.gz#/%{name}-%{version}-%{build_timestamp}.tar.gz
Group:          Applications/Internet

%define cmake cmake

%if 0%{?rhel} == 6
%define cmake cmake3
%endif

%if 0%{?rhel} == 7
%define cmake cmake3
%endif

# RHEL 8, 9 and 10 (COPR does not need the debuginfo subpackage).
%if 0%{?rhel} >= 8
%define debug_package %{nil}
%endif

%if 0%{?fedora} >= 33
%define debug_package %{nil}
%endif

BuildRequires: libMantids30-devel >= 3.0.0
BuildRequires:  %{cmake} zlib-devel boost-devel jsoncpp-devel gcc-c++
%if 0%{?rhel} == 7
BuildRequires:  openssl11-devel
%else
BuildRequires:  openssl-devel
%endif

Requires: libMantids30 >= 3.0.0
Requires: zlib boost-regex jsoncpp
%if 0%{?rhel} == 7
Requires:       openssl11
%else
Requires:       openssl
%endif

%description
This package contains a very efficient and simple web server that can be called from the command line

%prep
%autosetup -n %{name}-main

%build

%if 0%{?rhel} == 7
%{cmake} -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON -DCMAKE_INSTALL_PREFIX:PATH=/usr -DCMAKE_BUILD_TYPE=MinSizeRel -DWITH_SSL_SUPPORT=ON -DSSLRHEL7=ON
%else
%{cmake} -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON -DCMAKE_INSTALL_PREFIX:PATH=/usr -DCMAKE_BUILD_TYPE=MinSizeRel -DWITH_SSL_SUPPORT=ON
%endif

make %{?_smp_mflags}

# %clean is deprecated since rpm 4.15; RHEL 8/9/10 and current Fedora drop it.
%if 0%{?rhel} && 0%{?rhel} < 8
%clean
rm -rf $RPM_BUILD_ROOT
%endif

%install
rm -rf $RPM_BUILD_ROOT

%if 0%{?fedora} >= 33
ln -s . %{_host}
%endif

# RHEL 9+ and modern Fedora use %{_vpath_builddir}/redhat-linux-build with %cmake.
%if 0%{?rhel} >= 9
ln -s . %{_host}
ln -s . redhat-linux-build
%endif

%if 0%{?fedora} >= 35
ln -s . redhat-linux-build
%endif

%if "%{_host}" == "powerpc64le-redhat-linux-gnu"
ln -s . ppc64le-redhat-linux-gnu
%endif

%if "%{_host}" == "s390x-ibm-linux-gnu"
ln -s . s390x-redhat-linux-gnu
%endif

%if "%{cmake}" == "cmake3"
%cmake3_install
%else
%cmake_install
%endif

%files
%doc README.md BUILD.md INSTALL.md CONTRIBUTING.md LICENSE
%{_bindir}/uPocketWS

%changelog
* Mon Nov 02 2026 Aaron Mizrachi <aaron@unmanarc.com> - 3.0.0-1
- Bump to 3.0.0; track the main branch tarball instead of master.
- Ship README.md and the AGPL-3.0 LICENSE as %doc.
- Add RHEL 10 (dnf5, gcc 14, openssl 3) to the supported build targets:
  debuginfo suppression now applies to every RHEL >= 8 and the deprecated
  %clean section is only emitted for RHEL < 8.

