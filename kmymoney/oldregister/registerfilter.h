/*
 * Copyright 2006-2018  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2017-2018  Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef REGISTERFILTER_H
#define REGISTERFILTER_H

#include "kmm_oldregister_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ledgerfilter.h"

namespace eWidgets { namespace eRegister { enum class ItemState; } }

namespace KMyMoneyRegister
{
  /**
  * Used to filter items from the register.
  */
  struct KMM_OLDREGISTER_EXPORT RegisterFilter {
    explicit RegisterFilter(const QString &t, LedgerFilter::State s);

    LedgerFilter::State  state;
    QString text;
  };

} // namespace

#endif
