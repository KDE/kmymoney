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

#include "mymoneytransaction.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>
#include <QList>
#include <QStringList>
#include <QDate>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneystoragenames.h"
#include "mymoneyutils.h"
#include "mymoneymoney.h"
#include "mymoneysplit.h"

using namespace MyMoneyStorageNodes;

class MyMoneyTransactionPrivate {

public:
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

MyMoneyTransaction::MyMoneyTransaction() :
    MyMoneyObject(),
    d_ptr(new MyMoneyTransactionPrivate)
{
  Q_D(MyMoneyTransaction);
  d->m_nextSplitID = 1;
  d->m_entryDate = QDate();
  d->m_postDate = QDate();
}

MyMoneyTransaction::MyMoneyTransaction(const QDomElement& node, const bool forceId) :
    MyMoneyObject(node, forceId),
    d_ptr(new MyMoneyTransactionPrivate)
{
  Q_D(MyMoneyTransaction);
  if (nodeNames[nnTransaction] != node.tagName())
    throw MYMONEYEXCEPTION("Node was not TRANSACTION");

  d->m_nextSplitID = 1;

  d->m_postDate = stringToDate(node.attribute(getAttrName(Attribute::PostDate)));
  d->m_entryDate = stringToDate(node.attribute(getAttrName(Attribute::EntryDate)));
  d->m_bankID = QStringEmpty(node.attribute(getAttrName(Attribute::BankID)));
  d->m_memo = QStringEmpty(node.attribute(getAttrName(Attribute::Memo)));
  d->m_commodity = QStringEmpty(node.attribute(getAttrName(Attribute::Commodity)));

  QDomNode child = node.firstChild();
  while (!child.isNull() && child.isElement()) {
    QDomElement c = child.toElement();
    if (c.tagName() == getElName(Element::Splits)) {

      // Process any split information found inside the transaction entry.
      QDomNodeList nodeList = c.elementsByTagName(getElName(Element::Split));
      for (int i = 0; i < nodeList.count(); ++i) {
        MyMoneySplit s(nodeList.item(i).toElement());
        if (!d->m_bankID.isEmpty())
          s.setBankID(d->m_bankID);
        if (!s.accountId().isEmpty())
          addSplit(s);
        else
          qDebug("Dropped split because it did not have an account id");
      }

    } else if (c.tagName() == nodeNames[nnKeyValuePairs]) {
      MyMoneyKeyValueContainer kvp(c);
      setPairs(kvp.pairs());
    }

    child = child.nextSibling();
  }
  d->m_bankID.clear();
}

MyMoneyTransaction::MyMoneyTransaction(const MyMoneyTransaction& other) :
  MyMoneyObject(other.id()),
  MyMoneyKeyValueContainer(other),
  d_ptr(new MyMoneyTransactionPrivate(*other.d_func()))
{
}

MyMoneyTransaction::MyMoneyTransaction(const QString& id, const MyMoneyTransaction& other) :
  MyMoneyObject(id),
  MyMoneyKeyValueContainer(other),
  d_ptr(new MyMoneyTransactionPrivate(*other.d_func()))
{
  Q_D(MyMoneyTransaction);
  if (d->m_entryDate == QDate())
    d->m_entryDate = QDate::currentDate();

  foreach (auto split, d->m_splits)
    split.setTransactionId(id);
}

MyMoneyTransaction::~MyMoneyTransaction()
{
  Q_D(MyMoneyTransaction);
  delete d;
}

QDate MyMoneyTransaction::entryDate() const
{
  Q_D(const MyMoneyTransaction);
  return d->m_entryDate;
}

void MyMoneyTransaction::setEntryDate(const QDate& date)
{
  Q_D(MyMoneyTransaction);
  d->m_entryDate = date;
}

QDate MyMoneyTransaction::postDate() const
{
  Q_D(const MyMoneyTransaction);
  return d->m_postDate;
}

void MyMoneyTransaction::setPostDate(const QDate& date)
{
  Q_D(MyMoneyTransaction);
  d->m_postDate = date;
}

QString MyMoneyTransaction::memo() const
{
  Q_D(const MyMoneyTransaction);
  return d->m_memo;
}

void MyMoneyTransaction::setMemo(const QString& memo)
{
  Q_D(MyMoneyTransaction);
  d->m_memo = memo;
}

const QList<MyMoneySplit>& MyMoneyTransaction::splits() const
{
  Q_D(const MyMoneyTransaction);
  return d->m_splits;
}

QList<MyMoneySplit>& MyMoneyTransaction::splits()
{
  Q_D(MyMoneyTransaction);
  return d->m_splits;
}
uint MyMoneyTransaction::splitCount() const
{
  Q_D(const MyMoneyTransaction);
  return d->m_splits.count();
}

QString MyMoneyTransaction::commodity() const
{
  Q_D(const MyMoneyTransaction);
  return d->m_commodity;
}

void MyMoneyTransaction::setCommodity(const QString& commodityId)
{
  Q_D(MyMoneyTransaction);
  d->m_commodity = commodityId;
}

QString MyMoneyTransaction::bankID() const
{
  Q_D(const MyMoneyTransaction);
  return d->m_bankID;
}

void MyMoneyTransaction::setBankID(const QString& bankID)
{
  Q_D(MyMoneyTransaction);
  d->m_bankID = bankID;
}

bool MyMoneyTransaction::operator == (const MyMoneyTransaction& right) const
{
  Q_D(const MyMoneyTransaction);
  auto d2 = static_cast<const MyMoneyTransactionPrivate *>(right.d_func());
  return (MyMoneyObject::operator==(right) &&
          MyMoneyKeyValueContainer::operator==(right) &&
          (d->m_commodity == d2->m_commodity) &&
          ((d->m_memo.length() == 0 && d2->m_memo.length() == 0) || (d->m_memo == d2->m_memo)) &&
          (d->m_splits == d2->m_splits) &&
          (d->m_entryDate == d2->m_entryDate) &&
          (d->m_postDate == d2->m_postDate));
}

bool MyMoneyTransaction::operator != (const MyMoneyTransaction& r) const
{
  return !(*this == r);
}

bool MyMoneyTransaction::operator< (const MyMoneyTransaction& r) const
{
  return postDate() < r.postDate();
}

bool MyMoneyTransaction::operator<= (const MyMoneyTransaction& r) const
{
  return postDate() <= r.postDate();
}

bool MyMoneyTransaction::operator> (const MyMoneyTransaction& r) const
{
  return postDate() > r.postDate();
}

bool MyMoneyTransaction::accountReferenced(const QString& id) const
{
  Q_D(const MyMoneyTransaction);

  foreach (const auto split, d->m_splits) {
    if (split.accountId() == id)
      return true;
  }
  return false;
}

void MyMoneyTransaction::addSplit(MyMoneySplit &split)
{
  if (!split.id().isEmpty())
    throw MYMONEYEXCEPTION("Cannot add split with assigned id (" + split.id() + ')');

  if (split.accountId().isEmpty())
    throw MYMONEYEXCEPTION("Cannot add split that does not contain an account reference");

  MyMoneySplit newSplit(nextSplitID(), split);
  split = newSplit;
  split.setTransactionId(id());
  Q_D(MyMoneyTransaction);
  d->m_splits.append(split);
}

void MyMoneyTransaction::modifySplit(const MyMoneySplit& split)
{
// This is the other version which allows having more splits referencing
// the same account.
  if (split.accountId().isEmpty())
    throw MYMONEYEXCEPTION("Cannot modify split that does not contain an account reference");

  Q_D(MyMoneyTransaction);
  QList<MyMoneySplit>::Iterator it;
  for (it = d->m_splits.begin(); it != d->m_splits.end(); ++it) {
    if (split.id() == (*it).id()) {
      *it = split;
      return;
    }
  }
  throw MYMONEYEXCEPTION(QString("Invalid split id '%1'").arg(split.id()));
}

void MyMoneyTransaction::removeSplit(const MyMoneySplit& split)
{
  QList<MyMoneySplit>::Iterator it;
  auto removed = false;

  Q_D(MyMoneyTransaction);
  for (it = d->m_splits.begin(); it != d->m_splits.end(); ++it) {
    if (split.id() == (*it).id()) {
      d->m_splits.erase(it);
      removed = true;
      break;
    }
  }
  if (!removed)
    throw MYMONEYEXCEPTION(QString("Invalid split id '%1'").arg(split.id()));
}

void MyMoneyTransaction::removeSplits()
{
  Q_D(MyMoneyTransaction);
  d->m_splits.clear();
  d->m_nextSplitID = 1;
}

MyMoneySplit MyMoneyTransaction::splitByPayee(const QString& payeeId) const
{
  Q_D(const MyMoneyTransaction);
  foreach (const auto split, d->m_splits) {
    if (split.payeeId() == payeeId)
      return split;
  }
  throw MYMONEYEXCEPTION(QString("Split not found for payee '%1'").arg(QString(payeeId)));
}

MyMoneySplit MyMoneyTransaction::splitByAccount(const QString& accountId, const bool match) const
{
  Q_D(const MyMoneyTransaction);
  foreach (const auto split, d->m_splits) {
    if ((match == true && split.accountId() == accountId) ||
        (match == false && split.accountId() != accountId))
      return split;
  }
  throw MYMONEYEXCEPTION(QString("Split not found for account %1%2").arg(match ? "" : "!").arg(QString(accountId)));
}

MyMoneySplit MyMoneyTransaction::splitByAccount(const QStringList& accountIds, const bool match) const
{
  Q_D(const MyMoneyTransaction);
  foreach (const auto split, d->m_splits) {
    if ((match == true && accountIds.contains(split.accountId())) ||
        (match == false && !accountIds.contains(split.accountId())))
      return split;
  }
  throw MYMONEYEXCEPTION(QString("Split not found for account  %1%1...%2").arg(match ? "" : "!").arg(accountIds.front(), accountIds.back()));
}

MyMoneySplit MyMoneyTransaction::splitById(const QString& splitId) const
{
  Q_D(const MyMoneyTransaction);
  foreach (const auto split, d->m_splits) {
    if (split.id() == splitId)
      return split;
  }
  throw MYMONEYEXCEPTION(QString("Split not found for id '%1'").arg(QString(splitId)));
}

QString MyMoneyTransaction::nextSplitID()
{
  Q_D(MyMoneyTransaction);
  QString id;
  id = 'S' + id.setNum(d->m_nextSplitID++).rightJustified(SPLIT_ID_SIZE, '0');
  return id;
}

QString MyMoneyTransaction::firstSplitID()
{
  QString id;
  id = 'S' + id.setNum(1).rightJustified(SPLIT_ID_SIZE, '0');
  return id;
}

MyMoneyMoney MyMoneyTransaction::splitSum() const
{
  MyMoneyMoney result;

  Q_D(const MyMoneyTransaction);
  foreach (const auto split, d->m_splits)
    result += split.value();
  return result;
}

bool MyMoneyTransaction::isLoanPayment() const
{
  try {

    Q_D(const MyMoneyTransaction);
    foreach (const auto split, d->m_splits) {
      if (split.isAmortizationSplit())
        return true;
    }
  } catch (const MyMoneyException &) {
  }
  return false;
}

MyMoneySplit MyMoneyTransaction::amortizationSplit() const
{
  static MyMoneySplit nullSplit;

  Q_D(const MyMoneyTransaction);
  foreach (const auto split, d->m_splits) {
    if (split.isAmortizationSplit() && split.isAutoCalc())
      return split;
  }
  return nullSplit;
}

MyMoneySplit MyMoneyTransaction::interestSplit() const
{
  static MyMoneySplit nullSplit;

  Q_D(const MyMoneyTransaction);
  foreach (const auto split, d->m_splits) {
    if (split.isInterestSplit() && split.isAutoCalc())
      return split;
  }
  return nullSplit;
}

unsigned long MyMoneyTransaction::hash(const QString& txt, unsigned long h)
{
  unsigned long g;

  for (int i = 0; i < txt.length(); ++i) {
    unsigned short uc = txt[i].unicode();
    for (unsigned j = 0; j < 2; ++j) {
      unsigned char c = uc & 0xff;
      // if either the cell or the row of the Unicode char is 0, stop processing
      if (!c)
        break;
      h = (h << 4) + c;
      if ((g = (h & 0xf0000000))) {
        h = h ^(g >> 24);
        h = h ^ g;
      }
      uc >>= 8;
    }
  }
  return h;
}

bool MyMoneyTransaction::isStockSplit() const
{
  Q_D(const MyMoneyTransaction);
  return (d->m_splits.count() == 1 && d->m_splits[0].action() == MyMoneySplit::ActionSplitShares);
}

bool MyMoneyTransaction::isImported() const
{
  return value("Imported").toLower() == QString("true");
}

void MyMoneyTransaction::setImported(bool state)
{
  if (state)
    setValue("Imported", "true");
  else
    deletePair("Imported");
}

void MyMoneyTransaction::writeXML(QDomDocument& document, QDomElement& parent) const
{
  Q_D(const MyMoneyTransaction);
  auto el = document.createElement(nodeNames[nnTransaction]);

  writeBaseXML(document, el);
  el.setAttribute(getAttrName(Attribute::PostDate), dateToString(d->m_postDate));
  el.setAttribute(getAttrName(Attribute::Memo), d->m_memo);
  el.setAttribute(getAttrName(Attribute::EntryDate), dateToString(d->m_entryDate));
  el.setAttribute(getAttrName(Attribute::Commodity), d->m_commodity);

  QDomElement splits = document.createElement(getElName(Element::Splits));
  QList<MyMoneySplit>::ConstIterator it;
  for (it = d->m_splits.begin(); it != d->m_splits.end(); ++it) {
    (*it).writeXML(document, splits);
  }
  el.appendChild(splits);

  MyMoneyKeyValueContainer::writeXML(document, el);

  parent.appendChild(el);
}

bool MyMoneyTransaction::hasReferenceTo(const QString& id) const
{
  Q_D(const MyMoneyTransaction);
  QList<MyMoneySplit>::const_iterator it;
  bool rc = (id == d->m_commodity);
  for (it = d->m_splits.begin(); rc == false && it != d->m_splits.end(); ++it) {
    rc = (*it).hasReferenceTo(id);
  }
  return rc;
}

bool MyMoneyTransaction::hasAutoCalcSplit() const
{
  Q_D(const MyMoneyTransaction);

  foreach (const auto split, d->m_splits)
    if (split.isAutoCalc())
      return true;
  return false;
}

QString MyMoneyTransaction::accountSignature(bool includeSplitCount) const
{
  Q_D(const MyMoneyTransaction);
  QMap<QString, int> accountList;
  QList<MyMoneySplit>::const_iterator it_s;
  for (it_s = d->m_splits.constBegin(); it_s != d->m_splits.constEnd(); ++it_s) {
    accountList[(*it_s).accountId()] += 1;
  }

  QMap<QString, int>::const_iterator it_a;
  QString rc;
  for (it_a = accountList.constBegin(); it_a != accountList.constEnd(); ++it_a) {
    if (it_a != accountList.constBegin())
      rc += '-';
    rc += it_a.key();
    if (includeSplitCount)
      rc += QString("*%1").arg(*it_a);
  }
  return rc;
}

QString MyMoneyTransaction::uniqueSortKey() const
{
  QString year, month, day, key;
  const QDate& postdate = postDate();
  year = year.setNum(postdate.year()).rightJustified(YEAR_SIZE, '0');
  month = month.setNum(postdate.month()).rightJustified(MONTH_SIZE, '0');
  day = day.setNum(postdate.day()).rightJustified(DAY_SIZE, '0');
  key = QString(QLatin1String("%1-%2-%3-%4")).arg(year, month, day, m_id);
  return key;
}

bool MyMoneyTransaction::replaceId(const QString& newId, const QString& oldId)
{
  auto changed = false;
  QList<MyMoneySplit>::Iterator it;

  Q_D(MyMoneyTransaction);
  for (it = d->m_splits.begin(); it != d->m_splits.end(); ++it) {
    changed |= (*it).replaceId(newId, oldId);
  }
  return changed;
}

QString MyMoneyTransaction::getElName(const Element el)
{
  static const QHash<Element, QString> elNames = {
    {Element::Split,  QStringLiteral("SPLIT")},
    {Element::Splits, QStringLiteral("SPLITS")}
  };
  return elNames[el];
}

QString MyMoneyTransaction::getAttrName(const Attribute attr)
{
  static const QHash<Attribute, QString> attrNames = {
    {Attribute::Name,       QStringLiteral("name")},
    {Attribute::Type,       QStringLiteral("type")},
    {Attribute::PostDate,   QStringLiteral("postdate")},
    {Attribute::Memo,       QStringLiteral("memo")},
    {Attribute::EntryDate,  QStringLiteral("entrydate")},
    {Attribute::Commodity,  QStringLiteral("commodity")},
    {Attribute::BankID,     QStringLiteral("bankid")},
  };
  return attrNames[attr];
}
