/*
 * Copyright 2000-2002  Michael Edwardes <mte@users.sourceforge.net>
 * Copyright 2001       Felix Rodriguez <frodriguez@users.sourceforge.net>
 * Copyright 2002-2003  Kevin Tambascio <ktambascio@users.sourceforge.net>
 * Copyright 2006-2017  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2006       Ace Jones <acejones@users.sourceforge.net>
 * Copyright 2017-2018  Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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

#ifndef MYMONEYACCOUNT_P_H
#define MYMONEYACCOUNT_P_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>
#include <QDate>
#include <QMap>

// ----------------------------------------------------------------------------
// KDE Includes


// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyobject_p.h"
#include "mymoneykeyvaluecontainer.h"
#include "mymoneymoney.h"
#include "mymoneyenums.h"

using namespace eMyMoney;

class MyMoneyAccountPrivate : public MyMoneyObjectPrivate
{
public:

  MyMoneyAccountPrivate() :
    m_accountType(Account::Type::Unknown),
    m_fraction(-1)
  {
  }

  /**
    * This member variable identifies the type of account
    */
  eMyMoney::Account::Type m_accountType;

  /**
    * This member variable keeps the ID of the MyMoneyInstitution object
    * that this object belongs to.
    */
  QString m_institution;

  /**
    * This member variable keeps the name of the account
    * It is solely for documentation purposes and it's contents is not
    * used otherwise by the mymoney-engine.
    */
  QString m_name;

  /**
    * This member variable keeps the account number at the institution
    * It is solely for documentation purposes and it's contents is not
    * used otherwise by the mymoney-engine.
    */
  QString m_number;

  /**
    * This member variable is a description of the account.
    * It is solely for documentation purposes and it's contents is not
    * used otherwise by the mymoney-engine.
    */
  QString m_description;

  /**
    * This member variable keeps the date when the account
    * was last modified.
    */
  QDate m_lastModified;

  /**
    * This member variable keeps the date when the
    * account was created as an object in a MyMoneyFile
    */
  QDate m_openingDate;

  /**
    * This member variable keeps the date of the last
    * reconciliation of this account
    */
  QDate m_lastReconciliationDate;

  /**
    * This member holds the ID's of all sub-ordinate accounts
    */
  QStringList m_accountList;

  /**
    * This member contains the ID of the parent account
    */
  QString m_parentAccount;

  /**
    * This member contains the ID of the currency associated with this account
    */
  QString m_currencyId;

  /**
    * This member holds the balance of all transactions stored in the journal
    * for this account.
    */
  MyMoneyMoney    m_balance;

  /**
    * This member variable keeps the set of kvp's needed to establish
    * online banking sessions to this account.
    */
  MyMoneyKeyValueContainer m_onlineBankingSettings;

  /**
    * This member keeps the fraction for the account. It is filled by MyMoneyFile
    * when set to -1. See also @sa fraction(const MyMoneySecurity&).
    */
  int             m_fraction;

  /**
    * This member keeps the reconciliation history
    */
  QMap<QDate, MyMoneyMoney> m_reconciliationHistory;

};

#endif
