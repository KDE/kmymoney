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


#ifndef LEDGERPAYEEFILTER_H
#define LEDGERPAYEEFILTER_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QScopedPointer>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ledgerfilterbase.h"

class LedgerView;
class MyMoneyAccount;
class LedgerPayeeFilterPrivate;
class LedgerPayeeFilter : public LedgerFilterBase
{
  Q_OBJECT

public:
  explicit LedgerPayeeFilter(LedgerView* parent = nullptr);
  ~LedgerPayeeFilter() override;

  void setShowBalanceInverted(bool inverted = true);

  void setPayeeIdList(const QStringList& payeeIds);

public Q_SLOTS:
  /**
   * This method finshes the initial installation
   */
  void setupBottomHalf();

  void recalculateBalances();

  void recalculateBalancesOnIdle();

protected:
  /**
   * This method is overridden and checks if the splits payeeId is
   * contained in the list provided by setPayeeIdList() and
   * references an asset or liability account.
   */
  bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

private:
  Q_DECLARE_PRIVATE_D(LedgerFilterBase::d_ptr, LedgerPayeeFilter);
};

#endif // LEDGERPAYEEFILTER_H

