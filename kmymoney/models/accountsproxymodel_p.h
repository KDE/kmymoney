/*
    SPDX-FileCopyrightText: 2010-2014 Cristian One ț <onet.cristian@gmail.com>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ACCOUNTSPROXYMODELPRIVATE_H
#define ACCOUNTSPROXYMODELPRIVATE_H

// ----------------------------------------------------------------------------
// QT Includes

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
        : m_hideClosedAccounts(true)
        , m_hideEquityAccounts(true)
        , m_hideUnusedIncomeExpenseAccounts(false)
        , m_haveHiddenUnusedIncomeExpenseAccounts(false)
        , m_hideFavoriteAccounts(true)
        , m_hideAllEntries(false)
    {
    }

    virtual ~AccountsProxyModelPrivate()
    {
    }

    QList<eMyMoney::Account::Type>  m_typeList;
    QString                         m_notSelectableId;
    bool                            m_hideClosedAccounts;
    bool                            m_hideEquityAccounts;
    bool                            m_hideUnusedIncomeExpenseAccounts;
    bool                            m_haveHiddenUnusedIncomeExpenseAccounts;
    bool                            m_hideFavoriteAccounts;
    bool                            m_hideAllEntries;
};

#endif // ACCOUNTSPROXYMODEL_P_H
