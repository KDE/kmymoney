/*
    SPDX-FileCopyrightText: 2017, 2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "menuenums.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QHash>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

QHash<eMenu::Action, QAction *> pActions;
QHash<eMenu::Menu, QMenu *> pMenus;


