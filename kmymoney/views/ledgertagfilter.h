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


#ifndef LEDGERTAGFILTER_H
#define LEDGERTAGFILTER_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ledgerfilterbase.h"

class MyMoneyAccount;
class LedgerTagFilterPrivate;
class LedgerTagFilter : public LedgerFilterBase
{
  Q_OBJECT

public:
  explicit LedgerTagFilter(QObject* parent, QVector<QAbstractItemModel*> specialJournalModels);
  ~LedgerTagFilter() override;

  void setShowBalanceInverted(bool inverted = true);

  void setTagIdList (const QStringList& payeeIds);

public Q_SLOTS:
  void recalculateBalances();

  void recalculateBalancesOnIdle(const QString& accountId);

protected:
  /**
   * This method is overridden and checks if the splits payeeId is
   * contained in the list provided by setPayeeIdList() and
   * references an asset or liability account.
   */
  bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

private:
  Q_DECLARE_PRIVATE_D(LedgerFilterBase::d_ptr, LedgerTagFilter);
};

#endif // LEDGERTAGFILTER_H

