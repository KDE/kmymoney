/*
    SPDX-FileCopyrightText: 2017 Marc Hübner <mahueb55@gmail.com>
    SPDX-FileCopyrightText: 2020 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "platformtools.h"

#include <lmcons.h>
#include <process.h>
#include <windows.h>

#include <QString>

QString platformTools::osUsername()
{
    QString name;
    DWORD size = UNLEN+1;
    wchar_t  wcname[UNLEN+1];
    if(GetUserNameW((LPWSTR) wcname, &size)) {
        name = QString::fromWCharArray(wcname);
    }
    return name;
}

uint platformTools::processId()
{
    return _getpid();
}
