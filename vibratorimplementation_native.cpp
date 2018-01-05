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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstdio>

#define DEFAULT_NATIVE_FILE_PATH "/sys/class/timed_output/vibrator/enable"

struct VibratorImplementationPrivate {
    void vibrator_write(uint32_t value);
    int fd;
};

VibratorImplementation::VibratorImplementation()
    : d_ptr(new VibratorImplementationPrivate())
{
    d_ptr->fd = -1;
}

VibratorImplementation::~VibratorImplementation()
{
    if (d_ptr->fd > 0) {
        close(d_ptr->fd);
    }

    delete d_ptr;
}

bool VibratorImplementation::init()
{
    if (d_ptr->fd > 0) {
        return true;
    }

    if ((d_ptr->fd = open(DEFAULT_NATIVE_FILE_PATH, O_WRONLY)) < 0) {
        return false;
    }

    return true;
}

void VibratorImplementation::on(uint32_t duration_ms)
{
    d_ptr->vibrator_write(duration_ms);
}

void VibratorImplementation::off()
{
    d_ptr->vibrator_write(0);
}

void VibratorImplementationPrivate::vibrator_write(uint32_t value)
{
    char value_str[12]; /* fits UINT32_MAX value with newline */
    int length;

    if (fd < 0) {
        return;
    }

    length = snprintf(value_str, sizeof(value_str), "%u\n", value);
    write(fd, value_str, length);
}
