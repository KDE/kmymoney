/*
    SPDX-FileCopyrightText: 2000 Michael Edwardes <mte@users.sourceforge.net>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KSTARTUPLOGO_H
#define KSTARTUPLOGO_H

#include <memory>

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// Project Includes


class QSplashScreen;

std::unique_ptr<QSplashScreen> createStartupLogo();

#endif
