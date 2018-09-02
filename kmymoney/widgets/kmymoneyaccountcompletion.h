/*
 * Copyright 2004-2017  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2017       Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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

#ifndef KMYMONEYACCOUNTCOMPLETION_H
#define KMYMONEYACCOUNTCOMPLETION_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneycompletion.h"

namespace eMyMoney { namespace Account { enum class Type; } }

class KMyMoneyAccountSelector;

/**
  * @author Thomas Baumgart
  */
class KMyMoneyAccountCompletion : public KMyMoneyCompletion
{
  Q_OBJECT
  Q_DISABLE_COPY(KMyMoneyAccountCompletion)

public:

  explicit KMyMoneyAccountCompletion(QWidget* parent = nullptr);
  ~KMyMoneyAccountCompletion() override;

  QStringList accountList(const QList<eMyMoney::Account::Type>& list) const;
  QStringList accountList() const;

  /**
    * reimplemented from KMyMoneyCompletion
    */
  KMyMoneyAccountSelector* selector() const;

public Q_SLOTS:
  void slotMakeCompletion(const QString& txt);
};

#endif
