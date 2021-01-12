Name: qt5-feedback-haptics-droid-vibrator
Version: 0.0.8
Release: 1
Summary: Haptic feedback plugin using Droid Vibrator from libhybris
License: LGPLv2
URL: https://github.com/mer-hybris/qt5-feedback-haptics-droid-vibrator
Source0: %{name}-%{version}.tar.bz2
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig
Requires:	libprofile-qt5 >= 0.34.0.8
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5Gui)
BuildRequires:  pkgconfig(Qt5DBus)
BuildRequires:  pkgconfig(Qt5Feedback)
BuildRequires:  pkgconfig(android-headers)
BuildRequires:  pkgconfig(libhardware)
BuildRequires:  pkgconfig(libvibrator)
BuildRequires:  libprofile-qt5-devel
Conflicts:      qt5-feedback-haptics-native-vibrator

%description
%{summary}.

%files
%defattr(-,root,root,-)
%{_libdir}/qt5/plugins/feedback/libqtfeedback_droid-vibrator.so
%{_libdir}/qt5/plugins/feedback/droid-vibrator.json

%prep
%setup -q -n %{name}-%{version}

%build
%qmake5
make

%install
%qmake5_install

%post -p /sbin/ldconfig
%postun -p /sbin/ldconfig
