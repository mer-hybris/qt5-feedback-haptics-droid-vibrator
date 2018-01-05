/****************************************************************************
**
** Copyright (C) 2018 Jolla Ltd.
** Contact: Juho Hämäläinen <juho.hamalainen@jolla.com>
**
** This file is part of the QtFeedback Droid Vibrator Plugin.
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

#include "vibratorimplementation.h"

#include <android-version.h>
#if ANDROID_VERSION_MAJOR >= 7
#include <hardware/vibrator.h>
#else
#include <hardware_legacy/vibrator.h>
#endif

struct VibratorImplementationPrivate {
#if ANDROID_VERSION_MAJOR >= 7
    vibrator_device_t *dev;
#endif
};

VibratorImplementation::VibratorImplementation()
#if ANDROID_VERSION_MAJOR >= 7
    : d_ptr(new VibratorImplementationPrivate())
{
    d_ptr->dev = 0;
}
#else
    : d_ptr(0)
{
}
#endif

VibratorImplementation::~VibratorImplementation()
{
#if ANDROID_VERSION_MAJOR >= 7
    if (d_ptr->dev) {
        d_ptr->dev->common.close(reinterpret_cast<hw_device_t *>(d_ptr->dev));
    }

    delete d_ptr;
#endif
}

bool VibratorImplementation::init()
{
#if ANDROID_VERSION_MAJOR >= 7
    struct hw_module_t *hwmod = 0;

    hw_get_module(VIBRATOR_HARDWARE_MODULE_ID, (const hw_module_t **)(&hwmod));

    if (!hwmod || vibrator_open(hwmod, &d_ptr->dev) < 0) {
        return false;
    }
#endif

    return true;
}

void VibratorImplementation::on(uint32_t duration_ms)
{
#if ANDROID_VERSION_MAJOR >= 7
    if (d_ptr->dev) {
        d_ptr->dev->vibrator_on(d_ptr->dev, duration_ms);
    }
#else
    vibrator_on(duration_ms);
#endif
}

void VibratorImplementation::off()
{
#if ANDROID_VERSION_MAJOR >= 7
    if (d_ptr->dev) {
        d_ptr->dev->vibrator_off(d_ptr->dev);
    }
#else
    vibrator_off();
#endif
}
