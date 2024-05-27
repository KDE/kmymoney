/*
    SPDX-FileCopyrightText: 2017 Marc Hübner <mahueb55@gmail.com>
    SPDX-FileCopyrightText: 2020 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PLATFORMTOOLS_H
#define PLATFORMTOOLS_H


#include <QtGlobal>

class QString;


namespace platformTools
{

/**
 * This function returns the os username of the user account
 * under which the application is being run.
 */
QString osUsername();

/**
 * This function returns the PID associated with the current process.
 */
uint processId();
};

#endif // PLATFORMTOOLS_H
