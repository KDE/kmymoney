/***************************************************************************
 *   Copyright 2010  Cristian Onet onet.cristian@gmail.com                 *
 *   Copyright 2017-2018  Łukasz Wojniłowicz lukasz.wojnilowicz@gmail.com  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public License as        *
 *   published by the Free Software Foundation; either version 2 of        *
 *   the License or (at your option) version 3 or any later version        *
 *   accepted by the membership of KDE e.V. (or its successor approved     *
 *   by the membership of KDE e.V.), which shall act as a proxy            *
 *   defined in Section 14 of version 3 of the license.                    *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>  *
 ***************************************************************************/

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
