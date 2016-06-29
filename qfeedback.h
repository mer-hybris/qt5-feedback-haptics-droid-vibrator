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
#ifndef QFEEDBACK_DROID_VIBRATOR_H
#define QFEEDBACK_DROID_VIBRATOR_H

#include <QObject>
#include <QTimerEvent>
#include <qfeedbackplugininterfaces.h>

#include <profile.h>


QT_BEGIN_HEADER

class QFeedbackDroidVibrator : public QObject, public QFeedbackHapticsInterface, public QFeedbackThemeInterface {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QtFeedbackPlugin" FILE "droid-vibrator.json")
    Q_INTERFACES(QFeedbackHapticsInterface)
    Q_INTERFACES(QFeedbackThemeInterface)

public:
    QFeedbackDroidVibrator(QObject *parent = 0);
    ~QFeedbackDroidVibrator();

    virtual bool play(QFeedbackEffect::Effect) Q_DECL_OVERRIDE;
    virtual QFeedbackInterface::PluginPriority pluginPriority() Q_DECL_OVERRIDE;

    virtual QList<QFeedbackActuator*> actuators() Q_DECL_OVERRIDE;
    virtual void setActuatorProperty(const QFeedbackActuator &, ActuatorProperty, const QVariant &) Q_DECL_OVERRIDE;
    virtual QVariant actuatorProperty(const QFeedbackActuator &, ActuatorProperty) Q_DECL_OVERRIDE;
    virtual bool isActuatorCapabilitySupported(const QFeedbackActuator &, QFeedbackActuator::Capability) Q_DECL_OVERRIDE;
    virtual void updateEffectProperty(const QFeedbackHapticsEffect *, EffectProperty) Q_DECL_OVERRIDE;
    virtual void setEffectState(const QFeedbackHapticsEffect *, QFeedbackEffect::State) Q_DECL_OVERRIDE;
    virtual QFeedbackEffect::State effectState(const QFeedbackHapticsEffect *) Q_DECL_OVERRIDE;

protected:
    void timerEvent(QTimerEvent *event);

private slots:
    void deviceProfileSettingsChanged();

private:
    void stopCustomEffect(const QFeedbackHapticsEffect *effect);
    void startCustomEffect(const QFeedbackHapticsEffect *effect);

private:
    // profile change detection (normal / silent / airplane etc)
    Profile m_profile;
    bool m_profileEnablesVibra;
    int m_profileTouchscreenVibraLevel;
    int m_durations[QFeedbackEffect::NumberOfEffects];

    QFeedbackActuator *m_actuator;
    QFeedbackHapticsEffect *m_activeEffect;
    bool m_actuatorEnabled;
    int m_stateChangeTimerId;
};

QT_END_HEADER

#endif // QFEEDBACK_DROID_VIBRATOR_H
