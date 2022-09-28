/*
    SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef LEDGERFILTERBASE_P_H
#define LEDGERFILTERBASE_P_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QConcatenateTablesProxyModel>
#include <QDate>
#include <QSet>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ledgersortproxymodel_p.h"
#include "mymoneyfile.h"

class LedgerFilterBasePrivate : public LedgerSortProxyModelPrivate
{
public:
    LedgerFilterBasePrivate(LedgerFilterBase* qq)
        : LedgerSortProxyModelPrivate(qq)
        , concatModel(nullptr)
        , accountType(eMyMoney::Account::Type::Asset)
        , showValuesInverted(false)
    {
    }

    QString modelType(const QAbstractItemModel* model) const
    {
        if (isAccountsModel(model))
            return QLatin1String("AccountsModel");
        if (isSpecialDatesModel(model))
            return QLatin1String("SpecialDatesModel");
        if (isReconciliationModel(model))
            return QLatin1String("ReconciliationModel");
        if (isJournalModel(model))
            return QLatin1String("JournalModel");
        if (isSchedulesJournalModel(model))
            return QLatin1String("SchedulesJournalModel");
        return QLatin1String("unknown model");
    }

    QConcatenateTablesProxyModel* concatModel;
    eMyMoney::Account::Type accountType;
    QStringList filterIds;
    bool showValuesInverted;
    QSet<QAbstractItemModel*> sourceModels;
};

#endif
