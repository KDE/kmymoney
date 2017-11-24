/***************************************************************************
                          mymoneyaccount.cpp
                          -------------------
    copyright            : (C) 2000 by Michael Edwardes <mte@users.sourceforge.net>
                           (C) 2002 by Thomas Baumgart <ipwizard@users.sourceforge.net>
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

#ifndef MYMONEYACCOUNT_P_H
#define MYMONEYACCOUNT_P_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>
#include <QDate>

// ----------------------------------------------------------------------------
// KDE Includes


// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneykeyvaluecontainer.h"
#include "mymoneymoney.h"
#include "mymoneyenums.h"

using namespace eMyMoney;

namespace eMyMoney
{
  namespace Account
  {
    enum class Element { SubAccount,
                         SubAccounts,
                         OnlineBanking
                       };
    uint qHash(const Element key, uint seed) { return ::qHash(static_cast<uint>(key), seed); }

    enum class Attribute { ID = 0 ,
                           Name,
                           Type,
                           ParentAccount,
                           LastReconciled,
                           LastModified,
                           Institution,
                           Opened,
                           Number,
                           Description,
                           Currency,
                           OpeningBalance,
                           IBAN,
                           BIC,
                           // insert new entries above this line
                           LastAttribute
                         };
    uint qHash(const Attribute key, uint seed) { return ::qHash(static_cast<uint>(key), seed); }
  }
}

class MyMoneyAccountPrivate {

public:

  MyMoneyAccountPrivate() :
    m_accountType(Account::Type::Unknown),
    m_fraction(-1)
  {
  }

  static QString getElName(const Account::Element el)
  {
    static const QMap<Account::Element, QString> elNames = {
      {Account::Element::SubAccount,     QStringLiteral("SUBACCOUNT")},
      {Account::Element::SubAccounts,    QStringLiteral("SUBACCOUNTS")},
      {Account::Element::OnlineBanking,  QStringLiteral("ONLINEBANKING")}
    };
    return elNames[el];
  }

  static QString getAttrName(const Account::Attribute attr)
  {
    static const QHash<Account::Attribute, QString> attrNames = {
      {Account::Attribute::ID,             QStringLiteral("id")},
      {Account::Attribute::Name,           QStringLiteral("name")},
      {Account::Attribute::Type,           QStringLiteral("type")},
      {Account::Attribute::ParentAccount,  QStringLiteral("parentaccount")},
      {Account::Attribute::LastReconciled, QStringLiteral("lastreconciled")},
      {Account::Attribute::LastModified,   QStringLiteral("lastmodified")},
      {Account::Attribute::Institution,    QStringLiteral("institution")},
      {Account::Attribute::Opened,         QStringLiteral("opened")},
      {Account::Attribute::Number,         QStringLiteral("number")},
      {Account::Attribute::Type,           QStringLiteral("type")},
      {Account::Attribute::Description,    QStringLiteral("description")},
      {Account::Attribute::Currency,       QStringLiteral("currency")},
      {Account::Attribute::OpeningBalance, QStringLiteral("openingbalance")},
      {Account::Attribute::IBAN,           QStringLiteral("iban")},
      {Account::Attribute::BIC,            QStringLiteral("bic")},
    };
    return attrNames[attr];
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
