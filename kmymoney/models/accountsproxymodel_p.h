/*
    SPDX-FileCopyrightText: 2010-2014 Cristian Oneț <onet.cristian@gmail.com>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ACCOUNTSPROXYMODELPRIVATE_H
#define ACCOUNTSPROXYMODELPRIVATE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QList>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyenums.h"
#include "modelenums.h"

class AccountsProxyModelPrivate
{
    Q_DISABLE_COPY(AccountsProxyModelPrivate)

public:
    AccountsProxyModelPrivate() :
        m_mdlColumns(nullptr),
        m_hideClosedAccounts(true),
        m_hideEquityAccounts(true),
        m_hideUnusedIncomeExpenseAccounts(false),
        m_haveHiddenUnusedIncomeExpenseAccounts(false)
    {
    }

    virtual ~AccountsProxyModelPrivate()
    {
    }

    QList<eMyMoney::Account::Type> m_typeList;
    QList<eAccountsModel::Column> *m_mdlColumns;
    bool m_hideClosedAccounts;
    bool m_hideEquityAccounts;
    bool m_hideUnusedIncomeExpenseAccounts;
    bool m_haveHiddenUnusedIncomeExpenseAccounts;
};

#endif
