/*
    SPDX-FileCopyrightText: 2017 Marc Hübner <mahueb55@gmail.com>
    SPDX-FileCopyrightText: 2020 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "platformtools.h"

#include <pwd.h>
#include <unistd.h>

#include <QString>

QString platformTools::osUsername()
{
    QString name;
    struct passwd* pwd = getpwuid(geteuid());
    if( pwd != nullptr) {
        name = QString::fromLatin1(pwd->pw_name);
    }
    return name;
}

uint platformTools::processId()
{
    return getpid();
}
