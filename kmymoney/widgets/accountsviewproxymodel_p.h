/*
 * SPDX-FileCopyrightText: 2009 Cristian One ț <onet.cristian@gmail.com>
 * SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef ACCOUNTSVIEWPROXYMODELPRIVATE_H
#define ACCOUNTSVIEWPROXYMODELPRIVATE_H

#include "accountsproxymodel_p.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QSet>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "modelenums.h"

class AccountsViewProxyModelPrivate : public AccountsProxyModelPrivate
{
  Q_DISABLE_COPY(AccountsViewProxyModelPrivate)

public:
  AccountsViewProxyModelPrivate() :
    AccountsProxyModelPrivate()
  {
  }

  virtual ~AccountsViewProxyModelPrivate() override
  {
  }

  QSet<eAccountsModel::Column> m_visColumns;
};

#endif
