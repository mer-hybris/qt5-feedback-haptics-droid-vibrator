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

#define ACTIVATE_ON     (1)
#define ACTIVATE_OFF    (0)

struct file_location {
    const char *duration;
    const char *activate;
};

static const struct file_location file_locations[] = {
    { "/sys/class/timed_output/vibrator/enable",    NULL },
    { "/sys/class/leds/vibrator/duration",          "/sys/class/leds/vibrator/activate" },
};

struct VibratorImplementationPrivate {
    void vibrator_write(int fd, uint32_t value);
    int duration_fd;
    int activate_fd;
};

VibratorImplementation::VibratorImplementation()
    : d_ptr(new VibratorImplementationPrivate())
{
    d_ptr->duration_fd = -1;
    d_ptr->activate_fd = -1;
}

VibratorImplementation::~VibratorImplementation()
{
    if (d_ptr->duration_fd >= 0)
        close(d_ptr->duration_fd);

    if (d_ptr->activate_fd >= 0)
        close(d_ptr->activate_fd);

    delete d_ptr;
}

bool VibratorImplementation::init()
{
    if (d_ptr->duration_fd >= 0)
        return true;

    for (unsigned i = 0; i < sizeof(file_locations) / sizeof(file_locations[0]); i++) {
        const char *duration_path = file_locations[i].duration;
        const char *activate_path = file_locations[i].activate;

        if ((d_ptr->duration_fd = open(duration_path, O_WRONLY)) < 0)
            continue;

        if (activate_path) {
            if ((d_ptr->activate_fd = open(activate_path, O_WRONLY)) < 0) {
                close(d_ptr->duration_fd);
                d_ptr->duration_fd = -1;
                continue;
            }
        }

        break;
    }

    if (d_ptr->duration_fd < 0)
        return false;

    return true;
}

void VibratorImplementation::on(uint32_t duration_ms)
{
    d_ptr->vibrator_write(d_ptr->duration_fd, duration_ms);
    if (d_ptr->duration_fd >= 0)
        d_ptr->vibrator_write(d_ptr->activate_fd, ACTIVATE_ON);
}

void VibratorImplementation::off()
{
    d_ptr->vibrator_write(d_ptr->duration_fd, ACTIVATE_OFF);
    if (d_ptr->activate_fd >= 0)
        d_ptr->vibrator_write(d_ptr->activate_fd, ACTIVATE_OFF);
}

void VibratorImplementationPrivate::vibrator_write(int fd, uint32_t value)
{
    char value_str[12]; /* fits UINT32_MAX value with newline */
    int length;

    if (fd < 0) {
        return;
    }

    length = snprintf(value_str, sizeof(value_str), "%u\n", value);
    write(fd, value_str, length);
}
