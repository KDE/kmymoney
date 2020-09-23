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

#ifndef SELECTEDTRANSACTIONS_H
#define SELECTEDTRANSACTIONS_H

#include "kmm_oldregister_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QList>
#include <QMetaType>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "selectedtransaction.h"

namespace KMyMoneyRegister
{
  class Register;

  class KMM_OLDREGISTER_EXPORT SelectedTransactions: public QList<SelectedTransaction>
  {
  public:
    // TODO: find out how to move this ctor out of header
    SelectedTransactions() {} // krazy:exclude=inline
    explicit  SelectedTransactions(const Register* r);

    /**
   * @return the highest warnLevel of all transactions in the list
   */
    SelectedTransaction::warnLevel_t warnLevel() const;

    bool canModify() const;
    bool canDuplicate() const;
  };

} // namespace

Q_DECLARE_METATYPE(KMyMoneyRegister::SelectedTransactions)

#endif

