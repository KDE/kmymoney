/*
    SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef LEDGERFILTERBASE_P_H
#define LEDGERFILTERBASE_P_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QDate>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ledgerconcatenatemodel.h"
#include "ledgersortproxymodel_p.h"
#include "mymoneyfile.h"

class LedgerFilterBasePrivate : public LedgerSortProxyModelPrivate
{
public:
    explicit LedgerFilterBasePrivate(LedgerFilterBase* qq)
        : LedgerSortProxyModelPrivate(qq)
        , concatModel(nullptr)
        , accountType(eMyMoney::Account::Type::Asset)
        , showValuesInverted(false)
        , maintainBalances(false)
        , enableEdit(true)
    {
    }

    LedgerConcatenateModel* concatModel;
    eMyMoney::Account::Type accountType;
    QStringList filterIds;
    bool showValuesInverted;
    bool maintainBalances;
    bool enableEdit;
    KMMSet<QAbstractItemModel*> sourceModels;
    QVector<MyMoneyMoney> balances;
    QVector<int> splitMaxLineCount;
};

#endif
