/*
    SPDX-FileCopyrightText: 2021 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ledgerjournalidfilter.h"
#include "ledgerfilterbase_p.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "journalmodel.h"
#include "mymoneyfile.h"

class LedgerJournalIdFilterPrivate : public LedgerFilterBasePrivate
{
public:
    explicit LedgerJournalIdFilterPrivate(LedgerJournalIdFilter* qq)
        : LedgerFilterBasePrivate(qq)
    {
    }

    ~LedgerJournalIdFilterPrivate()
    {
    }
};

LedgerJournalIdFilter::LedgerJournalIdFilter(QObject* parent, QVector<QAbstractItemModel*> specialJournalModels)
    : LedgerFilterBase(new LedgerJournalIdFilterPrivate(this), parent)
{
    Q_D(LedgerJournalIdFilter);

    setFilterRole(eMyMoney::Model::IdRole);
    setObjectName("LedgerJournalIdFilter");
    setFilterKeyColumn(0);

    d->concatModel->setObjectName("LedgerView concatModel");
    d->concatModel->addSourceModel(MyMoneyFile::instance()->journalModel());

    for (const auto model : specialJournalModels) {
        d->concatModel->addSourceModel(model);
    }

    setSourceModel(d->concatModel);
}

LedgerJournalIdFilter::~LedgerJournalIdFilter()
{
}

bool LedgerJournalIdFilter::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
    const auto leftPos = filterFixedStrings().indexOf(left.data(eMyMoney::Model::IdRole).toString());
    const auto rightPos = filterFixedStrings().indexOf(right.data(eMyMoney::Model::IdRole).toString());

    return leftPos < rightPos;
}
