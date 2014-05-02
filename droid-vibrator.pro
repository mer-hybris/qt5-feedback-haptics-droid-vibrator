TEMPLATE = lib

QT += core feedback
CONFIG += qt plugin hide_symbols

TARGET = $$qtLibraryTarget(qtfeedback_droid-vibrator)
PLUGIN_TYPE = feedback

HEADERS += qfeedback.h
SOURCES += qfeedback.cpp

CONFIG += link_pkgconfig
PKGCONFIG += android-headers libhardware libvibrator
DEFINES += 'DROID_VIBRATOR_SETTINGS=\'\"$$[QT_INSTALL_PLUGINS]/feedback/droid-vibrator.ini\"\''

settings.files = droid-vibrator.ini
settings.path = $$[QT_INSTALL_PLUGINS]/feedback
INSTALLS += settings

target.path = $$[QT_INSTALL_PLUGINS]/feedback
INSTALLS += target

plugindescription.files = droid-vibrator.json
plugindescription.path = $$[QT_INSTALL_PLUGINS]/feedback/
INSTALLS += plugindescription

# also enable profile detection. libprofile-qt5 is a bit broken, work around it here.
QT += dbus
QMAKE_CXXFLAGS += -I/usr/include/profile-qt5
QMAKE_LFLAGS += -lprofile-qt5
