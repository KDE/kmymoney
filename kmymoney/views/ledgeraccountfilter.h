/*
 * Copyright 2019       Thomas Baumgart <tbaumgart@kde.org>
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


#ifndef LEDGERACCOUNTFILTER_H
#define LEDGERACCOUNTFILTER_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ledgerfilterbase.h"

class MyMoneyAccount;
class LedgerAccountFilterPrivate;

class LedgerAccountFilter : public LedgerFilterBase
{
  Q_OBJECT

public:
  explicit LedgerAccountFilter(QObject* parent, QVector<QAbstractItemModel*> specialJournalModels);
  ~LedgerAccountFilter() override;

  void setShowBalanceInverted(bool inverted = true);

  void setAccount(const MyMoneyAccount& acc);

public Q_SLOTS:
  void recalculateBalancesOnIdle(const QString& accountId);

protected Q_SLOTS:
  void recalculateBalances();

private:
  Q_DECLARE_PRIVATE_D(LedgerFilterBase::d_ptr, LedgerAccountFilter)
};

#endif // LEDGERACCOUNTFILTER_H

