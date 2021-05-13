/*
    SPDX-FileCopyrightText: 2021 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef LEDGERJOURNALIDFILTER_H
#define LEDGERJOURNALIDFILTER_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ledgerfilterbase.h"

class LedgerJournalIdFilterPrivate;
class LedgerJournalIdFilter : public LedgerFilterBase
{
    Q_OBJECT

public:
    explicit LedgerJournalIdFilter(QObject* parent, QVector<QAbstractItemModel*> specialJournalModels);
    ~LedgerJournalIdFilter() override;

protected:
    bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;

private:
    Q_DECLARE_PRIVATE_D(LedgerFilterBase::d_ptr, LedgerJournalIdFilter);
};

#endif
