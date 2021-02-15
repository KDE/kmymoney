/*
    SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef LEDGERPAYEEFILTER_H
#define LEDGERPAYEEFILTER_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ledgerfilterbase.h"

class MyMoneyAccount;
class LedgerPayeeFilterPrivate;
class LedgerPayeeFilter : public LedgerFilterBase
{
  Q_OBJECT

public:
  explicit LedgerPayeeFilter(QObject* parent, QVector<QAbstractItemModel*> specialJournalModels);
  ~LedgerPayeeFilter() override;

  void setShowBalanceInverted(bool inverted = true);

  void setPayeeIdList(const QStringList& payeeIds);

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
  Q_DECLARE_PRIVATE_D(LedgerFilterBase::d_ptr, LedgerPayeeFilter);
};

#endif // LEDGERPAYEEFILTER_H

