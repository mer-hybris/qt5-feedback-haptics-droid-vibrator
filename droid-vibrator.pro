TEMPLATE = lib

QT += core feedback
CONFIG += qt plugin hide_symbols

TARGET = $$qtLibraryTarget(qtfeedback_droid-vibrator)
PLUGIN_TYPE = feedback

HEADERS += qfeedback.h vibratorimplementation.h
SOURCES += qfeedback.cpp
OTHER_FILES += droid-vibrator.json

DEFINES += 'DROID_VIBRATOR_SETTINGS=\'\"$$[QT_INSTALL_PLUGINS]/feedback/droid-vibrator-device.ini\"\''

native_vibrator {
    SOURCES += vibratorimplementation_native.cpp
} else {
    CONFIG += link_pkgconfig
    PKGCONFIG += android-headers libhardware libvibrator
    SOURCES += vibratorimplementation_hal.cpp
}

target.path = $$[QT_INSTALL_PLUGINS]/feedback
INSTALLS += target

plugindescription.files = droid-vibrator.json
plugindescription.path = $$[QT_INSTALL_PLUGINS]/feedback/
INSTALLS += plugindescription

# also enable profile detection. libprofile-qt5 is a bit broken, work around it here.
QT += dbus
QMAKE_CXXFLAGS += -I/usr/include/profile-qt5
QMAKE_LFLAGS += -lprofile-qt5
