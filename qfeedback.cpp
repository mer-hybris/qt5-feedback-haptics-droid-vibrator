/****************************************************************************
**
** Copyright (C) 2014 Jolla Ltd.
** Contact: Thomas Perl <thomas.perl@jolla.com>
**
** This file is part of the QtFeedback Droid Vibrator Plugin.
** libprofile integration based on qt-mobility-haptics-ffmemless.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/
#include "qfeedback.h"
#include "qfeedbackeffect.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QtPlugin>

#include <QtCore/QFile>
#include <QtCore/QSettings>

#include <hardware_legacy/vibrator.h>

// Uncomment the following line for debug info
//#define DROID_VIBRATOR_DEBUG

#define DROID_VIBRATOR_TAG "qtfeedback-droid-vibrator: "

#if defined(DROID_VIBRATOR_DEBUG)
#  define PLUGIN_DEBUG(fmt, ...) qDebug(DROID_VIBRATOR_TAG fmt, ##__VA_ARGS__)
#else
#  define PLUGIN_DEBUG(fmt, ...)
#endif


QFeedbackDroidVibrator::QFeedbackDroidVibrator(QObject *parent)
    : QObject(parent)
    , QFeedbackThemeInterface()
    , m_profile(this)
    , m_profileEnablesVibra(false)
    , m_profileTouchscreenVibraLevel(0)
    , m_durations()
{
    PLUGIN_DEBUG("Initializing plugin");

    // Defaults
    m_durations[QFeedbackEffect::Press] = 20;
    m_durations[QFeedbackEffect::Release] = 18;
    m_durations[QFeedbackEffect::PressWeak] = 10;
    m_durations[QFeedbackEffect::ReleaseWeak] = 7;
    m_durations[QFeedbackEffect::PressStrong] = 30;
    m_durations[QFeedbackEffect::ReleaseStrong] = 25;
    m_durations[QFeedbackEffect::DragStart] = 5;
    m_durations[QFeedbackEffect::DragDropInZone] = 15;
    m_durations[QFeedbackEffect::DragDropOutOfZone] = 0;
    m_durations[QFeedbackEffect::DragCrossBoundary] = 10;
    m_durations[QFeedbackEffect::Appear] = 0;
    m_durations[QFeedbackEffect::Disappear] = 0;
    m_durations[QFeedbackEffect::Move] = 0;

    if (QFile(DROID_VIBRATOR_SETTINGS).exists()) {
        QSettings settings(DROID_VIBRATOR_SETTINGS, QSettings::IniFormat, this);

#define READ_SETTING(x) \
        m_durations[QFeedbackEffect::x] = \
        settings.value(#x, m_durations[QFeedbackEffect::x]).value<int>(); \
        PLUGIN_DEBUG("Reading setting for %s: %d", #x, m_durations[QFeedbackEffect::x]);

        READ_SETTING(Press)
        READ_SETTING(Release)
        READ_SETTING(PressWeak)
        READ_SETTING(ReleaseWeak)
        READ_SETTING(PressStrong)
        READ_SETTING(ReleaseStrong)
        READ_SETTING(DragStart)
        READ_SETTING(DragDropInZone)
        READ_SETTING(DragDropOutOfZone)
        READ_SETTING(DragCrossBoundary)
        READ_SETTING(Appear)
        READ_SETTING(Disappear)
        READ_SETTING(Move)

#undef READ_SETTING
    } else {
        PLUGIN_DEBUG("Not loading settings (%s does not exist)", DROID_VIBRATOR_SETTINGS);
    }

    connect(&m_profile, SIGNAL(activeProfileChanged(QString)),
            this, SLOT(deviceProfileSettingsChanged()));
    connect(&m_profile, SIGNAL(vibrationChanged(QString, bool)),
            this, SLOT(deviceProfileSettingsChanged()));
    connect(&m_profile, SIGNAL(touchscreenVibrationLevelChanged(QString, int)),
            this, SLOT(deviceProfileSettingsChanged()));

    deviceProfileSettingsChanged();
}

QFeedbackDroidVibrator::~QFeedbackDroidVibrator()
{
    PLUGIN_DEBUG("Deinitializing plugin");
}

void QFeedbackDroidVibrator::deviceProfileSettingsChanged()
{
    QString profile = m_profile.activeProfile();
    m_profileEnablesVibra = m_profile.isVibrationEnabled(profile);
    m_profileTouchscreenVibraLevel = m_profile.touchscreenVibrationLevel(profile);

    PLUGIN_DEBUG("Profile settings changed: enabled=%s, level=%d",
            m_profileEnablesVibra ? "true" : "false",
            m_profileTouchscreenVibraLevel);
}

bool QFeedbackDroidVibrator::play(QFeedbackEffect::Effect effect)
{
    // If vibra is disabled, disable effects
    if (Q_UNLIKELY(!m_profileEnablesVibra)) {
        PLUGIN_DEBUG("Not playing effect (vibra disabled)");
        return false;
    }

    // For weak vibration feedback, disable if vibra level is low
    if (Q_UNLIKELY(m_profileTouchscreenVibraLevel == 0)) {
        switch (effect) {
            case QFeedbackEffect::PressWeak:
            case QFeedbackEffect::ReleaseWeak:
                PLUGIN_DEBUG("Not playing effect (vibra level setting)");
                return false;
            default:
                break;
        }
    }

    switch (effect) {
        case QFeedbackEffect::Press:
        case QFeedbackEffect::Release:
        case QFeedbackEffect::PressWeak:
        case QFeedbackEffect::ReleaseWeak:
        case QFeedbackEffect::PressStrong:
        case QFeedbackEffect::ReleaseStrong:
        case QFeedbackEffect::DragStart:
        case QFeedbackEffect::DragDropInZone:
        case QFeedbackEffect::DragDropOutOfZone:
        case QFeedbackEffect::DragCrossBoundary:
        case QFeedbackEffect::Appear:
        case QFeedbackEffect::Disappear:
        case QFeedbackEffect::Move:
            PLUGIN_DEBUG("Playing effect #%d (%d ms)", effect, m_durations[effect]);
            vibrator_on(m_durations[effect]);
            return true;
        default:
            PLUGIN_DEBUG("Unknown or undefined effect #%d", effect);
            break;
    }

    return false;
}

QFeedbackInterface::PluginPriority QFeedbackDroidVibrator::pluginPriority()
{
    return QFeedbackInterface::PluginLowPriority;
}
