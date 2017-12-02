/***************************************************************************
                          mymoneytransaction.cpp
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes <mte@users.sourceforge.net>
                               2002 by Thomas Baumgart <ipwizard@users.sourceforge.net>

***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MYMONEYTRANSACTION_P_H
#define MYMONEYTRANSACTION_P_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>
#include <QList>
#include <QDate>
#include <QHash>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyobject_p.h"
#include "mymoneysplit.h"
namespace eMyMoney
{
  namespace Transaction
  {
    enum class Element { Split = 0,
                         Splits };
    uint qHash(const Element key, uint seed) { return ::qHash(static_cast<uint>(key), seed); }

    enum class Attribute { Name = 0,
                           Type,
                           PostDate,
                           Memo,
                           EntryDate,
                           Commodity,
                           BankID,
                           // insert new entries above this line
                           LastAttribute
                         };

    uint qHash(const Attribute key, uint seed) { return ::qHash(static_cast<uint>(key), seed); }
  }
}

using namespace eMyMoney;

class MyMoneyTransactionPrivate : public MyMoneyObjectPrivate
{
public:
  static QString getElName(const Transaction::Element el)
  {
    static const QHash<Transaction::Element, QString> elNames {
      {Transaction::Element::Split,  QStringLiteral("SPLIT")},
      {Transaction::Element::Splits, QStringLiteral("SPLITS")}
    };
    return elNames[el];
  }

  static QString getAttrName(const Transaction::Attribute attr)
  {
    static const QHash<Transaction::Attribute, QString> attrNames {
      {Transaction::Attribute::Name,       QStringLiteral("name")},
      {Transaction::Attribute::Type,       QStringLiteral("type")},
      {Transaction::Attribute::PostDate,   QStringLiteral("postdate")},
      {Transaction::Attribute::Memo,       QStringLiteral("memo")},
      {Transaction::Attribute::EntryDate,  QStringLiteral("entrydate")},
      {Transaction::Attribute::Commodity,  QStringLiteral("commodity")},
      {Transaction::Attribute::BankID,     QStringLiteral("bankid")},
    };
    return attrNames[attr];
  }

  /**
    * This method returns the next id to be used for a split
    */
  QString nextSplitID()
  {
    QString id;
    id = 'S' + id.setNum(m_nextSplitID++).rightJustified(SPLIT_ID_SIZE, '0');
    return id;
  }

  static const int SPLIT_ID_SIZE = 4;
  /** constants for unique sort key */
  static const int YEAR_SIZE = 4;
  static const int MONTH_SIZE = 2;
  static const int DAY_SIZE = 2;

  /**
    * This member contains the date when the transaction was entered
    * into the engine
    */
  QDate m_entryDate;

  /**
    * This member contains the date the transaction was posted
    */
  QDate m_postDate;

  /**
    * This member keeps the memo text associated with this transaction
    */
  QString m_memo;

  /**
    * This member contains the splits for this transaction
    */
  QList<MyMoneySplit> m_splits;

  /**
    * This member keeps the unique numbers of splits within this
    * transaction. Upon creation of a MyMoneyTransaction object this
    * value will be set to 1.
    */
  uint m_nextSplitID;

  /**
    * This member keeps the base commodity (e.g. currency) for this transaction
    */
  QString  m_commodity;

  /**
    * This member keeps the bank's unique ID for the transaction, so we can
    * avoid duplicates.  This is only used for electronic statement downloads.
    *
    * Note this is now deprecated!  Bank ID's should be set on splits, not transactions.
    */
  QString m_bankID;

};

#endif
