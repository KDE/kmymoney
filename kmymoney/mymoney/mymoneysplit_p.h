/*
 * Copyright 2002-2017  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2004       Kevin Tambascio <ktambascio@users.sourceforge.net>
 * Copyright 2005-2006  Ace Jones <acejones@users.sourceforge.net>
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

#ifndef MYMONEYSPLIT_P_H
#define MYMONEYSPLIT_P_H

#include "mymoneysplit.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>
#include <QDate>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyobject_p.h"
#include "mymoneymoney.h"
#include "mymoneyenums.h"
namespace eMyMoney
{
  namespace Split
  {
    enum class Element { Split = 0,
                         Tag,
                         Match,
                         Container,
                         KeyValuePairs
                       };
    uint qHash(const Element key, uint seed) { return ::qHash(static_cast<uint>(key), seed); }

    enum class Attribute { ID = 0,
                           BankID,
                           Account,
                           Payee,
                           Tag,
                           Number,
                           Action,
                           Value,
                           Shares,
                           Price,
                           Memo,
                           CostCenter,
                           ReconcileDate,
                           ReconcileFlag,
                           KMMatchedTx,
                           // insert new entries above this line
                           LastAttribute
                         };
    uint qHash(const Attribute key, uint seed) { return ::qHash(static_cast<uint>(key), seed); }
  }
}

using namespace eMyMoney;

class MyMoneySplitPrivate : public MyMoneyObjectPrivate
{

public:

  static QString getElName(const Split::Element el)
  {
    static const QHash<Split::Element, QString> elNames {
      {Split::Element::Split,          QStringLiteral("SPLIT")},
      {Split::Element::Tag,            QStringLiteral("TAG")},
      {Split::Element::Match,          QStringLiteral("MATCH")},
      {Split::Element::Container,      QStringLiteral("CONTAINER")},
      {Split::Element::KeyValuePairs,  QStringLiteral("KEYVALUEPAIRS")}
    };
    return elNames[el];
  }

  static QString getAttrName(const Split::Attribute attr)
  {
    static const QHash<Split::Attribute, QString> attrNames {
      {Split::Attribute::ID,             QStringLiteral("id")},
      {Split::Attribute::BankID,         QStringLiteral("bankid")},
      {Split::Attribute::Account,        QStringLiteral("account")},
      {Split::Attribute::Payee,          QStringLiteral("payee")},
      {Split::Attribute::Tag,            QStringLiteral("tag")},
      {Split::Attribute::Number,         QStringLiteral("number")},
      {Split::Attribute::Action,         QStringLiteral("action")},
      {Split::Attribute::Value,          QStringLiteral("value")},
      {Split::Attribute::Shares,         QStringLiteral("shares")},
      {Split::Attribute::Price,          QStringLiteral("price")},
      {Split::Attribute::Memo,           QStringLiteral("memo")},
      {Split::Attribute::CostCenter,     QStringLiteral("costcenter")},
      {Split::Attribute::ReconcileDate,  QStringLiteral("reconciledate")},
      {Split::Attribute::ReconcileFlag,  QStringLiteral("reconcileflag")},
      {Split::Attribute::KMMatchedTx,    QStringLiteral("kmm-matched-tx")}
    };
    return attrNames[attr];
  }

  /**
    * This member contains the ID of the payee
    */
  QString        m_payee;

  /**
    * This member contains a list of the IDs of the tags
    */
  QList<QString> m_tagList;

  /**
    * This member contains the ID of the account
    */
  QString        m_account;

  /**
   * This member contains the ID of the cost center
   */
  QString        m_costCenter;

  /**
    */
  MyMoneyMoney   m_shares;

  /**
    */
  MyMoneyMoney   m_value;

  /**
    * If the quotient of m_shares divided by m_values is not the correct price
    * because of truncation, the price can be stored in this member. For display
    * purpose and transaction edit this value can be used by the application.
    */
  MyMoneyMoney   m_price;

  QString        m_memo;

  /**
    * This member contains information about the reconciliation
    * state of the split. Possible values are
    *
    * @li NotReconciled
    * @li Cleared
    * @li Reconciled
    * @li Frozen
    *
    */
  eMyMoney::Split::State m_reconcileFlag;

  /**
    * In case the reconciliation flag is set to Reconciled or Frozen
    * this member contains the date of the reconciliation.
    */
  QDate          m_reconcileDate;

  /**
    * The m_action member is an arbitrary string, but is intended to
    * be conveniently limited to a menu of selections such as
    * "Buy", "Sell", "Interest", etc.
    */
  QString        m_action;

  /**
    * The m_number member is used to store a reference number to
    * the split supplied by the user (e.g. check number, etc.).
    */
  QString        m_number;

  /**
    * This member keeps the bank's unique ID for the split, so we can
    * avoid duplicates.  This is only used for electronic statement downloads.
    *
    * This should only be set on the split which refers to the account
    * that was downloaded.
    */
  QString        m_bankID;

  /**
    * This member keeps a backward id to the transaction that this
    * split can be found in. It is the purpose of the MyMoneyTransaction
    * object to maintain this member variable.
    */
  QString        m_transactionId;
};

#endif
