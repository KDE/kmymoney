/*
    SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef LEDGERFILTERBASE_P_H
#define LEDGERFILTERBASE_P_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QSet>

// ----------------------------------------------------------------------------
// KDE Includes

#include "kconcatenaterowsproxymodel.h"

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyfile.h"

class LedgerFilterBasePrivate {
public:
    LedgerFilterBasePrivate(LedgerFilterBase* qq)
        : q(qq)
        , concatModel(nullptr)
        , accountType(eMyMoney::Account::Type::Asset)
        , showValuesInverted(false)
    {
    }

    inline bool isAccountsModel(const QAbstractItemModel* model) const
    {
        return (model == static_cast<void*>(MyMoneyFile::instance()->accountsModel()));
    }

    inline bool isSpecialDatesModel(const QAbstractItemModel* model) const
    {
        return (model == static_cast<void*>(MyMoneyFile::instance()->specialDatesModel()));
    }

    inline bool isReconciliationModel(const QAbstractItemModel* model) const
    {
        return (model == static_cast<void*>(MyMoneyFile::instance()->reconciliationModel()));
    }

    inline bool isJournalModel(const QAbstractItemModel* model) const
    {
        return (model == static_cast<void*>(MyMoneyFile::instance()->journalModel()));
    }

    inline bool isSchedulesJournalModel(const QAbstractItemModel* model) const
    {
        return (model == static_cast<void*>(MyMoneyFile::instance()->schedulesJournalModel()));
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

    LedgerFilterBase*           q;
    KConcatenateRowsProxyModel* concatModel;          // Qt5.13+ use QConcatenateTablesProxyModel
    eMyMoney::Account::Type     accountType;
    QStringList                 filterIds;
    bool                        showValuesInverted;
    QSet<QAbstractItemModel*>   sourceModels;
};

#endif
