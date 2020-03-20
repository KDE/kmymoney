/*
 * Copyright 2000-2002  Michael Edwardes <mte@users.sourceforge.net>
 * Copyright 2001-2017  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2001       Felix Rodriguez <frodriguez@users.sourceforge.net>
 * Copyright 2003       Kevin Tambascio <ktambascio@users.sourceforge.net>
 * Copyright 2004-2005  Ace Jones <acejones@users.sourceforge.net>
 * Copyright 2006       Darren Gould <darren_gould@gmx.de>
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

#include "mymoneytransaction.h"
#include "mymoneytransaction_p.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QStringList>
#include <QMap>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyutils.h"
#include "mymoneymoney.h"
#include "mymoneyexception.h"
#include "mymoneyenums.h"

MyMoneyTransaction::MyMoneyTransaction() :
    MyMoneyObject(*new MyMoneyTransactionPrivate)
{
  Q_D(MyMoneyTransaction);
  d->m_nextSplitID = 1;
  d->m_entryDate = QDate();
  d->m_postDate = QDate();
}

MyMoneyTransaction::MyMoneyTransaction(const QString &id) :
    MyMoneyObject(*new MyMoneyTransactionPrivate, id)
{
  Q_D(MyMoneyTransaction);
  d->m_nextSplitID = 1;
  d->m_entryDate = QDate();
  d->m_postDate = QDate();
}

MyMoneyTransaction::MyMoneyTransaction(const MyMoneyTransaction& other) :
  MyMoneyObject(*new MyMoneyTransactionPrivate(*other.d_func()), other.id()),
  MyMoneyKeyValueContainer(other)
{
}

MyMoneyTransaction::MyMoneyTransaction(const QString& id, const MyMoneyTransaction& other) :
  MyMoneyObject(*new MyMoneyTransactionPrivate(*other.d_func()), id),
  MyMoneyKeyValueContainer(other)
{
  Q_D(MyMoneyTransaction);
  if (d->m_entryDate == QDate())
    d->m_entryDate = QDate::currentDate();

  foreach (auto split, d->m_splits)
    split.setTransactionId(id);
}

MyMoneyTransaction::~MyMoneyTransaction()
{
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

QList<MyMoneySplit> MyMoneyTransaction::splits() const
{
  Q_D(const MyMoneyTransaction);
  return d->m_splits;
}

QList<MyMoneySplit>& MyMoneyTransaction::splits()
{
  Q_D(MyMoneyTransaction);
  return d->m_splits;
}

MyMoneySplit MyMoneyTransaction::firstSplit() const
{
  Q_D(const MyMoneyTransaction);
  return d->m_splits.first();
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
    throw MYMONEYEXCEPTION(QString::fromLatin1("Cannot add split with assigned id '%1'").arg(split.id()));

  if (split.accountId().isEmpty())
    throw MYMONEYEXCEPTION(QString::fromLatin1("Cannot add split that does not contain an account reference"));

  Q_D(MyMoneyTransaction);
  MyMoneySplit newSplit(d->nextSplitID(), split);
  split = newSplit;
  split.setTransactionId(id());
  d->m_splits.append(split);
}

void MyMoneyTransaction::modifySplit(const MyMoneySplit& split)
{
// This is the other version which allows having more splits referencing
// the same account.
  if (split.accountId().isEmpty())
    throw MYMONEYEXCEPTION_CSTRING("Cannot modify split that does not contain an account reference");

  Q_D(MyMoneyTransaction);
  for (auto& it_split : d->m_splits) {
    if (split.id() == it_split.id()) {
      it_split = split;
      return;
    }
  }
  throw MYMONEYEXCEPTION(QString::fromLatin1("Invalid split id '%1'").arg(split.id()));
}

void MyMoneyTransaction::removeSplit(const MyMoneySplit& split)
{
  Q_D(MyMoneyTransaction);
  for (auto end = d->m_splits.size(), i = 0; i < end; ++i) {
    if (split.id() == d->m_splits.at(i).id()) {
      d->m_splits.removeAt(i);
      return;
    }
  }

  throw MYMONEYEXCEPTION(QString::fromLatin1("Invalid split id '%1'").arg(split.id()));
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
  throw MYMONEYEXCEPTION(QString::fromLatin1("Split not found for payee '%1'").arg(QString(payeeId)));
}

MyMoneySplit MyMoneyTransaction::splitByAccount(const QString& accountId, const bool match) const
{
  Q_D(const MyMoneyTransaction);
  foreach (const auto split, d->m_splits) {
    if ((match == true && split.accountId() == accountId) ||
        (match == false && split.accountId() != accountId))
      return split;
  }
  throw MYMONEYEXCEPTION(QString::fromLatin1("Split not found for account %1%2").arg(match ? "" : "!", accountId));
}

MyMoneySplit MyMoneyTransaction::splitByAccount(const QStringList& accountIds, const bool match) const
{
  Q_D(const MyMoneyTransaction);
  foreach (const auto split, d->m_splits) {
    if ((match == true && accountIds.contains(split.accountId())) ||
        (match == false && !accountIds.contains(split.accountId())))
      return split;
  }
  throw MYMONEYEXCEPTION(QString::fromLatin1("Split not found for account  %1%2...%3").arg(match ? "" : "!", accountIds.front(), accountIds.back()));
}

MyMoneySplit MyMoneyTransaction::splitById(const QString& splitId) const
{
  Q_D(const MyMoneyTransaction);
  foreach (const auto split, d->m_splits) {
    if (split.id() == splitId)
      return split;
  }
  throw MYMONEYEXCEPTION(QString::fromLatin1("Split not found for id '%1'").arg(QString(splitId)));
}

QString MyMoneyTransaction::firstSplitID()
{
  QString id;
  id = 'S' + id.setNum(1).rightJustified(MyMoneyTransactionPrivate::SPLIT_ID_SIZE, '0');
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

void MyMoneyTransaction::reverse()
{
  Q_D(MyMoneyTransaction);
  for (MyMoneySplit& split : d->m_splits)
      split.negateValue();
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
  return (d->m_splits.count() == 1 && d->m_splits.first().action() == MyMoneySplit::actionName(eMyMoney::Split::Action::SplitShares));
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

bool MyMoneyTransaction::hasReferenceTo(const QString& id) const
{
  Q_D(const MyMoneyTransaction);
  if (id == d->m_commodity)
    return true;

  foreach (const auto split, d->m_splits) {
    if (split.hasReferenceTo(id))
      return true;
  }
  return false;
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
  foreach (const auto split, d->m_splits)
    accountList[split.accountId()] += 1;

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
  Q_D(const MyMoneyTransaction);
  QString year, month, day, key;
  const auto postdate = postDate();
  year = year.setNum(postdate.year()).rightJustified(MyMoneyTransactionPrivate::YEAR_SIZE, '0');
  month = month.setNum(postdate.month()).rightJustified(MyMoneyTransactionPrivate::MONTH_SIZE, '0');
  day = day.setNum(postdate.day()).rightJustified(MyMoneyTransactionPrivate::DAY_SIZE, '0');
  key = QString::fromLatin1("%1-%2-%3-%4").arg(year, month, day, d->m_id);
  return key;
}

bool MyMoneyTransaction::replaceId(const QString& newId, const QString& oldId)
{
  auto changed = false;
  Q_D(MyMoneyTransaction);
  for (MyMoneySplit& split : d->m_splits)
    changed |= split.replaceId(newId, oldId);

  return changed;
}
