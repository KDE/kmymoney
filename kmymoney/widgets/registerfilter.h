/*
    SPDX-FileCopyrightText: 2006-2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef REGISTERFILTER_H
#define REGISTERFILTER_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

namespace eWidgets { namespace eRegister { enum class ItemState; } }

namespace KMyMoneyRegister
{
  /**
  * Used to filter items from the register.
  */
  struct RegisterFilter {
    explicit RegisterFilter(const QString &t, eWidgets::eRegister::ItemState s);

    eWidgets::eRegister::ItemState state;
    QString text;
  };

} // namespace

#endif
