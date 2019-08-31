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

#include <QScopedPointer>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ledgerproxymodel.h"

class LedgerView;
class MyMoneyAccount;
class LedgerAccountFilterPrivate;

class LedgerAccountFilter : public LedgerProxyModel
{
  Q_OBJECT

public:
  explicit LedgerAccountFilter(LedgerView* parent = nullptr);
  ~LedgerAccountFilter() override;

  void setShowBalanceInverted(bool inverted = true);

  void setAccount(const MyMoneyAccount& acc);

  void setShowEntryForNewTransaction(bool show);

public Q_SLOTS:
  /**
   * This method finshes the initial installation
   */
  void setupBottomHalf();

  void recalculateBalances();

  void recalculateBalancesOnIdle();

private:
  QScopedPointer<LedgerAccountFilterPrivate>  d_ptr;
  Q_DECLARE_PRIVATE(LedgerAccountFilter)
};

#endif // LEDGERACCOUNTFILTER_H

