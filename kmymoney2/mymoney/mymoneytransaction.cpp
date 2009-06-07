/***************************************************************************

                          mymoneytransaction.cpp
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes,
                               2002 by Thomas Baumgart
    email                : mte@users.sourceforge.net,
                           ipwizard@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneytransaction.h"
//Added by qt3to4:
#include <QList>

MyMoneyTransaction::MyMoneyTransaction() :
  MyMoneyObject()
{
  m_nextSplitID = 1;
  m_entryDate = QDate();
  m_postDate = QDate();
}

MyMoneyTransaction::MyMoneyTransaction(const QString id, const MyMoneyTransaction& transaction) :
  MyMoneyObject(id)
{
  *this = transaction;
  m_id = id;
  if(m_entryDate == QDate())
    m_entryDate = QDate::currentDate();

  QList<MyMoneySplit>::Iterator it;
  for(it = m_splits.begin(); it != m_splits.end(); ++it) {
    (*it).setTransactionId(id);
  }
}

MyMoneyTransaction::MyMoneyTransaction(const QDomElement& node, const bool forceId) :
  MyMoneyObject(node, forceId),
  MyMoneyKeyValueContainer(node.elementsByTagName("KEYVALUEPAIRS").item(0).toElement())
{
  if("TRANSACTION" != node.tagName())
    throw new MYMONEYEXCEPTION("Node was not TRANSACTION");

  m_nextSplitID = 1;

  m_postDate = stringToDate(node.attribute("postdate"));
  m_entryDate = stringToDate(node.attribute("entrydate"));
  m_bankID = QStringEmpty(node.attribute("bankid"));
  m_memo = QStringEmpty(node.attribute("memo"));
  m_commodity = QStringEmpty(node.attribute("commodity"));

  //  Process any split information found inside the transaction entry.
  QDomNodeList nodeList = node.elementsByTagName("SPLITS");
  if(nodeList.count() > 0) {
    nodeList = nodeList.item(0).toElement().elementsByTagName("SPLIT");
    for(int i = 0; i < nodeList.count(); ++i) {
      MyMoneySplit s(nodeList.item(i).toElement());
      if(!m_bankID.isEmpty())
        s.setBankID(m_bankID);
      if(!s.accountId().isEmpty())
        addSplit(s);
      else
        qDebug("Dropped split because it did not have an account id");
    }
  }

  m_bankID = QString();
}

MyMoneyTransaction::~MyMoneyTransaction()
{
}

bool MyMoneyTransaction::operator == (const MyMoneyTransaction& right) const
{
  return (MyMoneyObject::operator==(right) &&
      MyMoneyKeyValueContainer::operator==(right) &&
      (m_commodity == right.m_commodity) &&
      ((m_memo.length() == 0 && right.m_memo.length() == 0) || (m_memo == right.m_memo)) &&
      (m_splits == right.m_splits) &&
      (m_entryDate == right.m_entryDate) &&
      (m_postDate == right.m_postDate) );
}

bool MyMoneyTransaction::accountReferenced(const QString& id) const
{
  QList<MyMoneySplit>::ConstIterator it;

  for(it = m_splits.begin(); it != m_splits.end(); ++it) {
    if((*it).accountId() == id)
      return true;
  }
  return false;
}

void MyMoneyTransaction::addSplit(MyMoneySplit& split)
{
  if(!split.id().isEmpty())
    throw new MYMONEYEXCEPTION("Cannot add split with assigned id (" + split.id() + ")");

/*
  QValueList<MyMoneySplit>::Iterator it;

  // if the account referenced in this split is already
  // referenced in another split, we add the amount of
  // this split to the other one. All other data contained
  // in the new split will be discarded.
  for(it = m_splits.begin(); it != m_splits.end(); ++it) {
    if((*it).accountId() == split.accountId()) {
      (*it).setValue((*it).value()+split.value());
      split = (*it);
      return;
    }
  }
*/

  if(split.accountId().isEmpty())
    throw new MYMONEYEXCEPTION("Cannot add split that does not contain an account reference");

  MyMoneySplit newSplit(nextSplitID(), split);
  split = newSplit;
  split.setTransactionId(id());
  m_splits.append(split);
}

void MyMoneyTransaction::modifySplit(MyMoneySplit& split)
{
// This version of the routine allows only a single
// split to reference one account. If a second split
// is modified to reference an account already referenced
// by another split, the values will be added and the
// duplicate removed.
/*
  QValueList<MyMoneySplit>::Iterator it;
  QValueList<MyMoneySplit>::Iterator self = m_splits.end();
  QValueList<MyMoneySplit>::Iterator dup = self;
  bool duplicateAccount = false;

  for(it = m_splits.begin(); it != m_splits.end(); ++it) {
    if(split.id() == (*it).id()) {
      self = it;
    } else if(split.accountId() == (*it).accountId()) {
      (*it).setValue((*it).value() + split.value());
      dup = it;
      duplicateAccount = true;
    }
  }

  if(self == m_splits.end())
    throw new MYMONEYEXCEPTION("Invalid split id '" + split.id() + "'");

  if(duplicateAccount) {
    m_splits.remove(self);
    split = *dup;
  } else
    *self = split;
*/

// This is the other version which allows having more splits referencing
// the same account.
  if(split.accountId().isEmpty())
    throw new MYMONEYEXCEPTION("Cannot modify split that does not contain an account reference");

  QList<MyMoneySplit>::Iterator it;
  for(it = m_splits.begin(); it != m_splits.end(); ++it) {
    if(split.id() == (*it).id()) {
      *it = split;
      return;
    }
  }
  throw new MYMONEYEXCEPTION(QString("Invalid split id '%1'").arg(split.id()));
}

void MyMoneyTransaction::removeSplit(const MyMoneySplit& split)
{
  QList<MyMoneySplit>::Iterator it;

  for(it = m_splits.begin(); it != m_splits.end(); ++it) {
    if(split.id() == (*it).id()) {
      m_splits.erase(it);
      break;
    }
  }
  if(it == m_splits.end())
    throw new MYMONEYEXCEPTION(QString("Invalid split id '%1'").arg(split.id()));
}

void MyMoneyTransaction::removeSplits(void)
{
  m_splits.clear();
  m_nextSplitID = 1;
}

const MyMoneySplit& MyMoneyTransaction::splitByPayee(const QString& payeeId) const
{
  QList<MyMoneySplit>::ConstIterator it;

  for(it = m_splits.begin(); it != m_splits.end(); ++it) {
    if((*it).payeeId() == payeeId)
      return *it;
  }
  throw new MYMONEYEXCEPTION(QString("Split not found for payee '%1'").arg(QString(payeeId)));
}

const MyMoneySplit& MyMoneyTransaction::splitByAccount(const QString& accountId, const bool match) const
{
  QList<MyMoneySplit>::ConstIterator it;

  for(it = m_splits.begin(); it != m_splits.end(); ++it) {
    if(match == true && (*it).accountId() == accountId)
      return *it;
    if(match == false && (*it).accountId() != accountId)
      return *it;
  }
  throw new MYMONEYEXCEPTION(QString("Split not found for account %1%2").arg(match?"":"!").arg(QString(accountId)));
}

const MyMoneySplit& MyMoneyTransaction::splitByAccount(const QStringList& accountIds, const bool match) const
{
  QList<MyMoneySplit>::ConstIterator it;

  for(it = m_splits.begin(); it != m_splits.end(); ++it) {
    if(match == true && accountIds.contains((*it).accountId()) )
      return *it;
    if(match == false && !accountIds.contains((*it).accountId()))
      return *it;
  }
  throw new MYMONEYEXCEPTION(QString("Split not found for account  %1%1...%2").arg(match?"":"!").arg(accountIds.front(),accountIds.back()));
}

const MyMoneySplit& MyMoneyTransaction::splitById(const QString& splitId) const
{
  QList<MyMoneySplit>::ConstIterator it;

  for(it = m_splits.begin(); it != m_splits.end(); ++it) {
    if((*it).id() == splitId)
      return *it;
  }
  throw new MYMONEYEXCEPTION(QString("Split not found for id '%1'").arg(QString(splitId)));
}

const QString MyMoneyTransaction::nextSplitID()
{
  QString id;
  id = "S" + id.setNum(m_nextSplitID++).rightJustified(SPLIT_ID_SIZE, '0');
  return id;
}

const QString MyMoneyTransaction::firstSplitID()
{
  QString id;
  id = "S" + id.setNum(1).rightJustified(SPLIT_ID_SIZE, '0');
  return id;
}

const MyMoneyMoney MyMoneyTransaction::splitSum(void) const
{
  MyMoneyMoney result(0);
  QList<MyMoneySplit>::ConstIterator it;

  for(it = m_splits.begin(); it != m_splits.end(); ++it) {
    result += (*it).value();
  }
  return result;
}

void MyMoneyTransaction::setPostDate(const QDate& date) { m_postDate = date; }
void MyMoneyTransaction::setEntryDate(const QDate& date) { m_entryDate = date; }
void MyMoneyTransaction::setMemo(const QString& memo) { m_memo = memo; }

bool MyMoneyTransaction::isLoanPayment(void) const
{
  try {
    QList<MyMoneySplit>::ConstIterator it;

    for(it = m_splits.begin(); it != m_splits.end(); ++it) {
      if((*it).isAmortizationSplit())
        return true;
    }
  } catch (MyMoneyException *e) {
    delete e;
  }
  return false;
}

const MyMoneySplit& MyMoneyTransaction::amortizationSplit(void) const
{
  static MyMoneySplit nullSplit;

  QList<MyMoneySplit>::ConstIterator it;

  for(it = m_splits.begin(); it != m_splits.end(); ++it) {
    if((*it).isAmortizationSplit() && (*it).isAutoCalc())
      return *it;
  }
  return nullSplit;
}

const MyMoneySplit& MyMoneyTransaction::interestSplit(void) const
{
  static MyMoneySplit nullSplit;

  QList<MyMoneySplit>::ConstIterator it;

  for(it = m_splits.begin(); it != m_splits.end(); ++it) {
    if((*it).isInterestSplit() && (*it).isAutoCalc())
      return *it;
  }
  return nullSplit;
}

unsigned long MyMoneyTransaction::hash(const QString& txt, unsigned long h)
{
  unsigned long g;

  for(int i=0; i < txt.length(); ++i) {
    unsigned short uc = txt[i].unicode();
    for(unsigned j = 0; j < 2; ++j) {
      unsigned char c = uc & 0xff;
      // if either the cell or the row of the Unicode char is 0, stop processing
      if(!c)
        break;
      h = (h << 4) + c;
      if( (g = (h & 0xf0000000)) ) {
        h = h ^ (g >> 24);
        h = h ^ g;
      }
      uc >>= 8;
    }
  }
  return h;
}

bool MyMoneyTransaction::isStockSplit(void) const
{
  return (m_splits.count() == 1 && m_splits[0].action() == MyMoneySplit::ActionSplitShares);
}

bool MyMoneyTransaction::isImported(void) const
{
  return value("Imported").toLower() == QString("true");
}

void MyMoneyTransaction::setImported(bool state)
{
  if(state)
    setValue("Imported", "true");
  else
    deletePair("Imported");
}

bool MyMoneyTransaction::isDuplicate(const MyMoneyTransaction& r) const
{
  bool rc = true;
  if(splitCount() != r.splitCount()) {
    rc = false;
  } else {
    if(abs(m_postDate.daysTo(r.postDate())) > 3) {
      rc = false;
    } else {
      unsigned long accHash[2];
      unsigned long valHash[2];
      unsigned long numHash[2];
      for(int i = 0; i < 2; ++i)
        accHash[i] = valHash[i] = numHash[i] = 0;

      QList<MyMoneySplit>::ConstIterator it;
      for(it = splits().begin(); it != splits().end(); ++it) {
        accHash[0] += hash((*it).accountId());
        valHash[0] += hash((*it).value().formatMoney("", 4));
        numHash[0] += hash((*it).number());
      }
      for(it = r.splits().begin(); it != r.splits().end(); ++it) {
        accHash[1] += hash((*it).accountId());
        valHash[1] += hash((*it).value().formatMoney("", 4));
        numHash[1] += hash((*it).number());
      }

      if(accHash[0] != accHash[1]
      || valHash[0] != valHash[1]
      || numHash[0] != numHash[1]
      ) {
        rc = false;
      }
    }
  }

  return rc;
}

void MyMoneyTransaction::writeXML(QDomDocument& document, QDomElement& parent) const
{
  QDomElement el = document.createElement("TRANSACTION");

  writeBaseXML(document, el);

  el.setAttribute("postdate", dateToString(m_postDate));
  el.setAttribute("memo", m_memo);
  el.setAttribute("entrydate", dateToString(m_entryDate));
  el.setAttribute("commodity", m_commodity);

  QDomElement splits = document.createElement("SPLITS");
  QList<MyMoneySplit>::ConstIterator it;
  for(it = m_splits.begin(); it != m_splits.end(); ++it) {
    (*it).writeXML(document, splits);
  }
  el.appendChild(splits);

  MyMoneyKeyValueContainer::writeXML(document, el);

  parent.appendChild(el);
}

bool MyMoneyTransaction::hasReferenceTo(const QString& id) const
{
  QList<MyMoneySplit>::const_iterator it;
  bool rc = (id == m_commodity);
  for(it = m_splits.begin(); rc == false && it != m_splits.end(); ++it) {
    rc = (*it).hasReferenceTo(id);
  }
  return rc;
}

bool MyMoneyTransaction::hasAutoCalcSplit(void) const
{
  QList<MyMoneySplit>::ConstIterator it;

  for(it = m_splits.begin(); it != m_splits.end(); ++it) {
    if((*it).isAutoCalc())
      return true;
  }
  return false;
}

QString MyMoneyTransaction::accountSignature(bool includeSplitCount) const
{
  QMap<QString, int> accountList;
  QList<MyMoneySplit>::const_iterator it_s;
  for(it_s = m_splits.constBegin(); it_s != m_splits.constEnd(); ++it_s) {
    accountList[(*it_s).accountId()] += 1;
  }

  QMap<QString, int>::const_iterator it_a;
  QString rc;
  for(it_a = accountList.constBegin(); it_a != accountList.constEnd(); ++it_a) {
    if(it_a != accountList.constBegin())
      rc += "-";
    rc += it_a.key();
    if(includeSplitCount)
      rc += QString("*%1").arg(*it_a);
  }
  return rc;
}

QString MyMoneyTransaction::uniqueSortKey(void) const
{
  QString year, month, day, key;
  const QDate& postdate = postDate();
  year = year.setNum(postdate.year()).rightJustified(YEAR_SIZE, '0');
  month = month.setNum(postdate.month()).rightJustified(MONTH_SIZE, '0');
  day = day.setNum(postdate.day()).rightJustified(DAY_SIZE, '0');
  key = year + "-" + month + "-" + day + "-" + m_id;
  return key;
}
