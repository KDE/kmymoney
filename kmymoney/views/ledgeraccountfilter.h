/*
    SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
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
  void sortView();

protected:
  bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

private:
  Q_DECLARE_PRIVATE_D(LedgerFilterBase::d_ptr, LedgerAccountFilter)
};

#endif // LEDGERACCOUNTFILTER_H

