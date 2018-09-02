/*
 * Copyright 2006       Ace Jones <acejones@users.sourceforge.net>
 * Copyright 2006       Darren Gould <darren_gould@gmx.de>
 * Copyright 2017       Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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
