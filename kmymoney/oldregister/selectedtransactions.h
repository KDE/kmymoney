/*
 * SPDX-FileCopyrightText: 2006-2018 Thomas Baumgart <tbaumgart@kde.org>
 * SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
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

