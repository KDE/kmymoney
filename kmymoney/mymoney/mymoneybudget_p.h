/***************************************************************************
                          mymoneybudget.cpp
                             -------------------
    begin                : Sun July 4 2004
    copyright            : (C) 2004-2005 by Ace Jones
    email                : acejones@users.sourceforge.net
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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

#include "mymoneymoney.h"

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

class MyMoneyBudgetPrivate {

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
