/*
 * Copyright 2019       Thomas Baumgart <tbaumgart@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ACCOUNTSPROXYMODEL_P_H
#define ACCOUNTSPROXYMODEL_P_H

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
