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

    QVariant data(const QModelIndex& index, int role) const override;

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

    // Overridden to suppress sorting on this model
    void doSort() override;

private:
    Q_DECLARE_PRIVATE_D(LedgerFilterBase::d_ptr, LedgerAccountFilter)
};

#endif // LEDGERACCOUNTFILTER_H

