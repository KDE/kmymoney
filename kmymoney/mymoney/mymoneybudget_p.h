/*
 * SPDX-FileCopyrightText: 2006 Ace Jones <acejones@users.sourceforge.net>
 * SPDX-FileCopyrightText: 2006 Darren Gould <darren_gould@gmx.de>
 * SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef MYMONEYBUDGET_P_H
#define MYMONEYBUDGET_P_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QDate>
#include <QHash>
#include <QMap>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyobject_p.h"
#include "mymoneymoney.h"
#include "mymoneyenums.h"

class MyMoneyBudgetPrivate : public MyMoneyObjectPrivate
{
public:
  /**
    * The user-assigned name of the Budget
    */
  QString m_name;

  /**
    * The user-assigned year of the Budget
    */
  QDate m_start;

  /**
    * Map the budgeted accounts
    *
    * Each account Id is stored against the AccountGroup information
    */
  QMap<QString, MyMoneyBudget::AccountGroup> m_accounts;
};

#endif
