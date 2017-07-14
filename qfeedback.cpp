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
#include <QtCore/QLoggingCategory>

Q_LOGGING_CATEGORY(qtFeedbackDroidVibrator, "qt.Feedback.DroidVibrator")

QFeedbackDroidVibrator::QFeedbackDroidVibrator(QObject *parent)
    : QObject(parent)
    , QFeedbackHapticsInterface()
    , QFeedbackThemeInterface()
    , m_profile(this)
    , m_profileEnablesVibra(false)
    , m_profileTouchscreenVibraLevel(0)
    , m_durations()
    , m_actuator(createFeedbackActuator(this, 2))
    , m_activeEffect(Q_NULLPTR)
    , m_actuatorEnabled(true)
#if ANDROID_VERSION_MAJOR >= 7
    , m_dev(NULL)
#endif
{
    qCDebug(qtFeedbackDroidVibrator) << "Initializing plugin";

#if (ANDROID_VERSION_MAJOR >= 7)
    struct hw_module_t *hwmod;

    hw_get_module(VIBRATOR_HARDWARE_MODULE_ID, (const hw_module_t **)(&hwmod));
    Q_ASSERT(hwmod != NULL);

    if (vibrator_open(hwmod, &m_dev) < 0) {
        qCCritical(qtFeedbackDroidVibrator) << "Unable to open vibrator device";
    }
#endif

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
        qCDebug(qtFeedbackDroidVibrator) << "Reading setting for" << #x << ":" << m_durations[QFeedbackEffect::x];

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
        qCDebug(qtFeedbackDroidVibrator) << "Not loading settings (" << DROID_VIBRATOR_SETTINGS << "does not exist)";
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
    qCDebug(qtFeedbackDroidVibrator) << "Deinitializing plugin";
}

void QFeedbackDroidVibrator::deviceProfileSettingsChanged()
{
    QString profile = m_profile.activeProfile();
    m_profileEnablesVibra = m_profile.isVibrationEnabled(profile);
    m_profileTouchscreenVibraLevel = m_profile.touchscreenVibrationLevel(profile);

    qCDebug(qtFeedbackDroidVibrator) << "Profile settings changed: enabled:"
                                     << m_profileEnablesVibra
                                     << "level:" << m_profileTouchscreenVibraLevel;
}

bool QFeedbackDroidVibrator::play(QFeedbackEffect::Effect effect)
{
    // If vibra is disabled, disable effects
    if (Q_UNLIKELY(!m_profileEnablesVibra)) {
        qCDebug(qtFeedbackDroidVibrator) << "Not playing effect (vibra disabled)";
        return false;
    }

    // For weak vibration feedback, disable if vibra level is low
    if (Q_UNLIKELY(m_profileTouchscreenVibraLevel == 0)) {
        switch (effect) {
            case QFeedbackEffect::PressWeak:
            case QFeedbackEffect::ReleaseWeak:
                qCDebug(qtFeedbackDroidVibrator) << "Not playing effect (vibra level setting)";
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
            qCDebug(qtFeedbackDroidVibrator) << "Playing effect #" << effect << "(" << m_durations[effect] << "ms)";
#if ANDROID_VERSION_MAJOR >= 7
            if (m_dev)
                m_dev->vibrator_on(m_dev, m_durations[effect]);
#else
            vibrator_on(m_durations[effect]);
#endif
            return true;
        default:
            qCDebug(qtFeedbackDroidVibrator) << "Unknown or undefined effect #" << effect;
            break;
    }

    return false;
}

QFeedbackInterface::PluginPriority QFeedbackDroidVibrator::pluginPriority()
{
    return QFeedbackInterface::PluginLowPriority;
}

QList<QFeedbackActuator*> QFeedbackDroidVibrator::actuators()
{
    return QList<QFeedbackActuator*>() << m_actuator;
}

void QFeedbackDroidVibrator::setActuatorProperty(const QFeedbackActuator &, ActuatorProperty prop, const QVariant &value)
{
    switch (prop) {
        case Enabled: {
            bool old = m_actuatorEnabled;
            m_actuatorEnabled = value.toBool();
            if (old != m_actuatorEnabled && !m_actuatorEnabled && m_activeEffect) {
                setEffectState(m_activeEffect, QFeedbackEffect::Stopped);
                m_activeEffect = 0;
            }
            break;
        }
        default: break;
    }
}

QVariant QFeedbackDroidVibrator::actuatorProperty(const QFeedbackActuator &, ActuatorProperty prop)
{
    switch (prop) {
        case Name:    return QLatin1String("DROID_VIBRATOR");
        case State:   return QFeedbackActuator::Ready;
        case Enabled: return m_actuatorEnabled;
        default:      return QVariant();
    }
}

bool QFeedbackDroidVibrator::isActuatorCapabilitySupported(const QFeedbackActuator &, QFeedbackActuator::Capability)
{
    return false; // we don't support envelope or periodicity (since we don't support changing intensity level).
}

void QFeedbackDroidVibrator::updateEffectProperty(const QFeedbackHapticsEffect *effect, QFeedbackHapticsInterface::EffectProperty prop)
{
    if (!m_actuatorEnabled)
        return;

    if (m_activeEffect != effect)
        return;

    if (prop == QFeedbackHapticsInterface::Duration) {
        qCDebug(qtFeedbackDroidVibrator) << "Playing custom effect due to property update (" << effect->duration() << "ms)";
        setEffectState(effect, QFeedbackEffect::Running);
    }
}

void QFeedbackDroidVibrator::setEffectState(const QFeedbackHapticsEffect *effect, QFeedbackEffect::State state)
{
    if (!m_actuatorEnabled)
        return;

    switch (state) {
        case QFeedbackEffect::Running: startCustomEffect(effect); break;
        case QFeedbackEffect::Stopped: stopCustomEffect(effect); break;
        case QFeedbackEffect::Paused:  // not supported
        case QFeedbackEffect::Loading: // not supported
        default: break;
    }
}

QFeedbackEffect::State QFeedbackDroidVibrator::effectState(const QFeedbackHapticsEffect *effect)
{
    if (m_activeEffect == effect)
        return QFeedbackEffect::Running;
    return QFeedbackEffect::Stopped;
}

void QFeedbackDroidVibrator::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_stateChangeTimerId) {
        // the specified duration has elapsed, mark the effect as stopped.
        stopCustomEffect(m_activeEffect);
    }
}

void QFeedbackDroidVibrator::startCustomEffect(const QFeedbackHapticsEffect *effect)
{
    if ((m_activeEffect == effect || !m_activeEffect) && effect->duration() > 0) {
        m_activeEffect = const_cast<QFeedbackHapticsEffect*>(effect);
        m_stateChangeTimerId = QObject::startTimer(m_activeEffect->duration());
        qCDebug(qtFeedbackDroidVibrator) << "Playing custom effect due to state change (" << m_activeEffect->duration() << "ms)";
#if ANDROID_VERSION_MAJOR >= 7
        if (m_dev)
            m_dev->vibrator_on(m_dev, m_activeEffect->duration());
#else
        vibrator_on(m_activeEffect->duration());
#endif

    }
}

void QFeedbackDroidVibrator::stopCustomEffect(const QFeedbackHapticsEffect *effect)
{
    if (m_activeEffect == effect) {
        qCDebug(qtFeedbackDroidVibrator) << "Stopping custom effect due to state change";
#if ANDROID_VERSION_MAJOR >= 7
        if (m_dev)
            m_dev->vibrator_off(m_dev);
#else
        vibrator_off();
#endif
        killTimer(m_stateChangeTimerId);
        m_activeEffect = 0;
        m_stateChangeTimerId = 0;
    }
}
