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

namespace Budget
{
  enum class Element { Budget = 0,
                       Account,
                       Period
                     };
  uint qHash(const Element key, uint seed) { return ::qHash(static_cast<uint>(key), seed); }

  enum class Attribute { ID = 0,
                         Name,
                         Start,
                         Version,
                         BudgetLevel,
                         BudgetSubAccounts,
                         Amount,
                         // insert new entries above this line
                         LastAttribute
                       };
  uint qHash(const Attribute key, uint seed) { return ::qHash(static_cast<uint>(key), seed); }
}

class MyMoneyBudgetPrivate : public MyMoneyObjectPrivate
{
public:
  static QString getElName(const Budget::Element el)
  {
    static const QMap<Budget::Element, QString> elNames {
      {Budget::Element::Budget,   "BUDGET"},
      {Budget::Element::Account,  "ACCOUNT"},
      {Budget::Element::Period,   "PERIOD"}
    };
    return elNames[el];
  }

  static QString getAttrName(const Budget::Attribute attr)
  {
    static const QHash<Budget::Attribute, QString> attrNames {
      {Budget::Attribute::ID,                 QStringLiteral("id")},
      {Budget::Attribute::Name,               QStringLiteral("name")},
      {Budget::Attribute::Start,              QStringLiteral("start")},
      {Budget::Attribute::Version,            QStringLiteral("version")},
      {Budget::Attribute::BudgetLevel,        QStringLiteral("budgetlevel")},
      {Budget::Attribute::BudgetSubAccounts,  QStringLiteral("budgetsubaccounts")},
      {Budget::Attribute::Amount,             QStringLiteral("amount")}
    };
    return attrNames[attr];
  }

  static QHash<eMyMoney::Budget::Level, QString> budgetLevelLUT()
  {
    static const QHash<eMyMoney::Budget::Level, QString> lut {
      {eMyMoney::Budget::Level::None,         QStringLiteral("none")},
      {eMyMoney::Budget::Level::Monthly,      QStringLiteral("monthly")},
      {eMyMoney::Budget::Level::MonthByMonth, QStringLiteral("monthbymonth")},
      {eMyMoney::Budget::Level::Yearly,       QStringLiteral("yearly")},
      {eMyMoney::Budget::Level::Max,          QStringLiteral("invalid")},
    };
    return lut;
  }

  static QString budgetNames(eMyMoney::Budget::Level textID)
  {
    return budgetLevelLUT().value(textID);
  }

  static eMyMoney::Budget::Level stringToBudgetLevel(const QString &text)
  {
    return budgetLevelLUT().key(text, eMyMoney::Budget::Level::Max);
  }

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
