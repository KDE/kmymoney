/*
    SPDX-FileCopyrightText: 2010-2014 Cristian Oneț <onet.cristian@gmail.com>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ACCOUNTSPROXYMODELPRIVATE_H
#define ACCOUNTSPROXYMODELPRIVATE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QSet>
class QComboBox;

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyenums.h"

class AccountsProxyModelPrivate
{
    Q_DISABLE_COPY(AccountsProxyModelPrivate)

public:
    AccountsProxyModelPrivate()
        : m_filterComboBox(nullptr)
        , m_hideClosedAccounts(true)
        , m_hideEquityAccounts(true)
        , m_hideZeroBalanceEquityAccounts(false)
        , m_hideUnusedIncomeExpenseAccounts(false)
        , m_haveHiddenUnusedIncomeExpenseAccounts(false)
        , m_hideFavoriteAccounts(true)
        , m_hideAllEntries(false)
        , m_canSelectClosedAccounts(false)
        , m_state(AccountsProxyModel::State::Any)
    {
    }

    virtual ~AccountsProxyModelPrivate()
    {
    }

    QList<eMyMoney::Account::Type> m_typeList;
    QString m_notSelectableId;
    QComboBox* m_filterComboBox;
    bool m_hideClosedAccounts;
    bool m_hideEquityAccounts;
    bool m_hideZeroBalanceEquityAccounts;
    bool m_hideUnusedIncomeExpenseAccounts;
    bool m_haveHiddenUnusedIncomeExpenseAccounts;
    bool m_hideFavoriteAccounts;
    bool m_hideAllEntries;
    bool m_canSelectClosedAccounts;
    QSet<eMyMoney::Account::Type> m_selectableAccountTypes;
    AccountsProxyModel::State m_state;
};

#endif // ACCOUNTSPROXYMODEL_P_H
