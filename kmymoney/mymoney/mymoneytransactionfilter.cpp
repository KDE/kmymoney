/*
 * Copyright 2003-2019  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2004       Ace Jones <acejones@users.sourceforge.net>
 * Copyright 2008-2010  Alvaro Soliverez <asoliverez@gmail.com>
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

#include "mymoneytransactionfilter.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDate>
#include <QDebug>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneymoney.h"
#include "mymoneyfile.h"
#include "mymoneyaccount.h"
#include "mymoneysecurity.h"
#include "mymoneypayee.h"
#include "mymoneytag.h"
#include "mymoneytransaction.h"
#include "mymoneysplit.h"
#include "mymoneyenums.h"

class MyMoneyTransactionFilterPrivate {

public:
  MyMoneyTransactionFilterPrivate()
    : m_reportAllSplits(false)
    , m_considerCategory(false)
    , m_matchOnly(false)
    , m_matchingSplitsCount(0)
    , m_invertText(false)
  {
    m_filterSet.allFilter = 0;
  }

  MyMoneyTransactionFilter::FilterSet m_filterSet;
  bool                m_reportAllSplits;
  bool                m_considerCategory;
  bool                m_matchOnly;

  uint                m_matchingSplitsCount;

  QRegExp             m_text;
  bool                m_invertText;
  QHash<QString, QString>    m_accounts;
  QHash<QString, QString>    m_payees;
  QHash<QString, QString>    m_tags;
  QHash<QString, QString>    m_categories;
  QHash<int, QString>      m_states;
  QHash<int, QString>      m_types;
  QHash<int, QString>      m_validity;
  QString             m_fromNr, m_toNr;
  QDate               m_fromDate, m_toDate;
  MyMoneyMoney        m_fromAmount, m_toAmount;
};

MyMoneyTransactionFilter::MyMoneyTransactionFilter() :
  d_ptr(new MyMoneyTransactionFilterPrivate)
{
  Q_D(MyMoneyTransactionFilter);
  d->m_reportAllSplits = true;
  d->m_considerCategory = true;
}

MyMoneyTransactionFilter::MyMoneyTransactionFilter(const QString& id) :
  d_ptr(new MyMoneyTransactionFilterPrivate)
{
  addAccount(id);
}

MyMoneyTransactionFilter::MyMoneyTransactionFilter(const MyMoneyTransactionFilter& other) :
  d_ptr(new MyMoneyTransactionFilterPrivate(*other.d_func()))
{
}

MyMoneyTransactionFilter::~MyMoneyTransactionFilter()
{
  Q_D(MyMoneyTransactionFilter);
  delete d;
}

void MyMoneyTransactionFilter::clear()
{
  Q_D(MyMoneyTransactionFilter);
  d->m_filterSet.allFilter = 0;
  d->m_invertText = false;
  d->m_accounts.clear();
  d->m_categories.clear();
  d->m_payees.clear();
  d->m_tags.clear();
  d->m_types.clear();
  d->m_states.clear();
  d->m_validity.clear();
  d->m_fromDate = QDate();
  d->m_toDate = QDate();
}

void MyMoneyTransactionFilter::clearAccountFilter()
{
  Q_D(MyMoneyTransactionFilter);
  d->m_filterSet.singleFilter.accountFilter = 0;
  d->m_accounts.clear();
}

void MyMoneyTransactionFilter::setTextFilter(const QRegExp& text, bool invert)
{
  Q_D(MyMoneyTransactionFilter);
  d->m_filterSet.singleFilter.textFilter = 1;
  d->m_invertText = invert;
  d->m_text = text;
}

void MyMoneyTransactionFilter::addAccount(const QStringList& ids)
{
  Q_D(MyMoneyTransactionFilter);

  d->m_filterSet.singleFilter.accountFilter = 1;
  for (const auto& id : ids)
    addAccount(id);
}

void MyMoneyTransactionFilter::addAccount(const QString& id)
{
  Q_D(MyMoneyTransactionFilter);
  if (!d->m_accounts.isEmpty() && !id.isEmpty() &&
      d->m_accounts.contains(id))
    return;

  d->m_filterSet.singleFilter.accountFilter = 1;
  if (!id.isEmpty())
    d->m_accounts.insert(id, QString());
}

void MyMoneyTransactionFilter::addCategory(const QStringList& ids)
{
  Q_D(MyMoneyTransactionFilter);

  d->m_filterSet.singleFilter.categoryFilter = 1;
  for (const auto& id : ids)
    addCategory(id);
}

void MyMoneyTransactionFilter::addCategory(const QString& id)
{
  Q_D(MyMoneyTransactionFilter);
  if (!d->m_categories.isEmpty() && !id.isEmpty() &&
      d->m_categories.contains(id))
    return;

  d->m_filterSet.singleFilter.categoryFilter = 1;
  if (!id.isEmpty())
    d->m_categories.insert(id, QString());
}

void MyMoneyTransactionFilter::setDateFilter(const QDate& from, const QDate& to)
{
  Q_D(MyMoneyTransactionFilter);
  d->m_filterSet.singleFilter.dateFilter = from.isValid() | to.isValid();
  d->m_fromDate = from;
  d->m_toDate = to;
}

void MyMoneyTransactionFilter::setAmountFilter(const MyMoneyMoney& from, const MyMoneyMoney& to)
{
  Q_D(MyMoneyTransactionFilter);
  d->m_filterSet.singleFilter.amountFilter = 1;
  d->m_fromAmount = from.abs();
  d->m_toAmount = to.abs();

  // make sure that the user does not try to fool us  ;-)
  if (from > to)
    std::swap(d->m_fromAmount, d->m_toAmount);
}

void MyMoneyTransactionFilter::addPayee(const QString& id)
{
  Q_D(MyMoneyTransactionFilter);
  if (!d->m_payees.isEmpty() && !id.isEmpty() &&
      d->m_payees.contains(id))
    return;

  d->m_filterSet.singleFilter.payeeFilter = 1;
  if (!id.isEmpty())
    d->m_payees.insert(id, QString());
}

void MyMoneyTransactionFilter::addTag(const QString& id)
{
  Q_D(MyMoneyTransactionFilter);
  if (!d->m_tags.isEmpty() && !id.isEmpty() &&
      d->m_tags.contains(id))
    return;

  d->m_filterSet.singleFilter.tagFilter = 1;
  if (!id.isEmpty())
    d->m_tags.insert(id, QString());
}

void MyMoneyTransactionFilter::addType(const int type)
{
  Q_D(MyMoneyTransactionFilter);
  if (!d->m_types.isEmpty() &&
      d->m_types.contains(type))
    return;

  d->m_filterSet.singleFilter.typeFilter = 1;
  d->m_types.insert(type, QString());
}

void MyMoneyTransactionFilter::addState(const int state)
{
  Q_D(MyMoneyTransactionFilter);
  if (!d->m_states.isEmpty() &&
      d->m_states.contains(state))
    return;

  d->m_filterSet.singleFilter.stateFilter = 1;
  d->m_states.insert(state, QString());
}

void MyMoneyTransactionFilter::addValidity(const int type)
{
  Q_D(MyMoneyTransactionFilter);
  if (!d->m_validity.isEmpty() &&
      d->m_validity.contains(type))
    return;

  d->m_filterSet.singleFilter.validityFilter = 1;
  d->m_validity.insert(type, QString());
}

void MyMoneyTransactionFilter::setNumberFilter(const QString& from, const QString& to)
{
  Q_D(MyMoneyTransactionFilter);
  d->m_filterSet.singleFilter.nrFilter = 1;
  d->m_fromNr = from;
  d->m_toNr = to;
}

void MyMoneyTransactionFilter::setReportAllSplits(const bool report)
{
  Q_D(MyMoneyTransactionFilter);
  d->m_reportAllSplits = report;
}

void MyMoneyTransactionFilter::setConsiderCategory(const bool check)
{
  Q_D(MyMoneyTransactionFilter);
  d->m_considerCategory = check;
}

uint MyMoneyTransactionFilter::matchingSplitsCount(const MyMoneyTransaction& transaction)
{
  Q_D(MyMoneyTransactionFilter);
  d->m_matchOnly = true;
  matchingSplits(transaction);
  d->m_matchOnly = false;
  return d->m_matchingSplitsCount;
}

QVector<MyMoneySplit> MyMoneyTransactionFilter::matchingSplits(const MyMoneyTransaction& transaction)
{
  Q_D(MyMoneyTransactionFilter);

  QVector<MyMoneySplit> matchingSplits;
  const auto file = MyMoneyFile::instance();

  // qDebug("T: %s", transaction.id().data());
  // if no filter is set, we can safely return a match
  // if we should report all splits, then we collect them
  if (!d->m_filterSet.allFilter && d->m_reportAllSplits) {
    d->m_matchingSplitsCount = transaction.splitCount();
    if (!d->m_matchOnly)
      matchingSplits = QVector<MyMoneySplit>::fromList(transaction.splits());
    return matchingSplits;
  }

  d->m_matchingSplitsCount = 0;
  const auto filter = d->m_filterSet.singleFilter;

  // perform checks on the MyMoneyTransaction object first

  // check the date range
  if (filter.dateFilter) {
    if ((d->m_fromDate != QDate() &&
         transaction.postDate() < d->m_fromDate) ||
        (d->m_toDate != QDate() &&
         transaction.postDate() > d->m_toDate)) {
      return matchingSplits;
    }
  }

  auto categoryMatched = !filter.categoryFilter;
  auto accountMatched = !filter.accountFilter;
  auto isTransfer = true;

  // check the transaction's validity
  if (filter.validityFilter) {
    if (!d->m_validity.isEmpty() &&
        !d->m_validity.contains((int)validTransaction(transaction)))
      return matchingSplits;
  }

  // if d->m_reportAllSplits == false..
  // ...then we don't need splits...
  // ...but we need to know if there were any found
  auto isMatchingSplitsEmpty = true;

  auto extendedFilter = d->m_filterSet;
  extendedFilter.singleFilter.dateFilter =
  extendedFilter.singleFilter.accountFilter =
      extendedFilter.singleFilter.categoryFilter = 0;

  if (filter.accountFilter ||
      filter.categoryFilter ||
      extendedFilter.allFilter) {
    const auto& splits = transaction.splits();
    for (const auto& s : splits) {

      if (filter.accountFilter ||
          filter.categoryFilter) {
        auto removeSplit = true;
        if (d->m_considerCategory) {
          switch (file->account(s.accountId()).accountGroup()) {
            case eMyMoney::Account::Type::Income:
            case eMyMoney::Account::Type::Expense:
              isTransfer = false;
              // check if the split references one of the categories in the list
              if (filter.categoryFilter) {
                if (d->m_categories.isEmpty()) {
                  // we're looking for transactions with 'no' categories
                  d->m_matchingSplitsCount = 0;
                  matchingSplits.clear();
                  return matchingSplits;
                } else if (d->m_categories.contains(s.accountId())) {
                  categoryMatched = true;
                  removeSplit = false;
                }
              }
              break;

            default:
              // check if the split references one of the accounts in the list
              if (!filter.accountFilter) {
                removeSplit = false;
              } else if (!d->m_accounts.isEmpty() &&
                         d->m_accounts.contains(s.accountId())) {
                accountMatched = true;
                removeSplit = false;
              }

              break;
          }

        } else {
          if (!filter.accountFilter) {
            removeSplit = false;
          } else if (!d->m_accounts.isEmpty() &&
                     d->m_accounts.contains(s.accountId())) {
            accountMatched = true;
            removeSplit = false;
          }
        }

        if (removeSplit)
          continue;
      }

      // check if less frequent filters are active
      if (extendedFilter.allFilter) {
        const auto acc = file->account(s.accountId());
        if (!(matchAmount(s) && matchText(s, acc)))
          continue;

        // Determine if this account is a category or an account
        auto isCategory = false;
        switch (acc.accountGroup()) {
          case eMyMoney::Account::Type::Income:
          case eMyMoney::Account::Type::Expense:
            isCategory = true;
          default:
            break;
        }

        if (!isCategory) {
          // check the payee list
          if (filter.payeeFilter) {
            if (!d->m_payees.isEmpty()) {
              if (s.payeeId().isEmpty() || !d->m_payees.contains(s.payeeId()))
                continue;
            } else if (!s.payeeId().isEmpty())
              continue;
          }

          // check the tag list
          if (filter.tagFilter) {
            const auto tags = s.tagIdList();
            if (!d->m_tags.isEmpty()) {
              if (tags.isEmpty()) {
                continue;
              } else {
                auto found = false;
                for (const auto& tag : tags) {
                  if (d->m_tags.contains(tag)) {
                    found = true;
                    break;
                  }
                }

                if (!found)
                  continue;
              }
            } else if (!tags.isEmpty())
              continue;
          }

          // check the type list
          if (filter.typeFilter &&
              !d->m_types.isEmpty() &&
              !d->m_types.contains(splitType(transaction, s, acc)))
            continue;

          // check the state list
          if (filter.stateFilter &&
              !d->m_states.isEmpty() &&
              !d->m_states.contains(splitState(s)))
            continue;

          if (filter.nrFilter &&
              ((!d->m_fromNr.isEmpty() && s.number() < d->m_fromNr) ||
               (!d->m_toNr.isEmpty() && s.number() > d->m_toNr)))
            continue;

        } else if (filter.payeeFilter
                   || filter.tagFilter
                   || filter.typeFilter
                   || filter.stateFilter
                   || filter.nrFilter) {
          continue;
        }
      }
      if (d->m_reportAllSplits)
        matchingSplits.append(s);

      isMatchingSplitsEmpty = false;
    }
  } else if (d->m_reportAllSplits) {
    const auto& splits = transaction.splits();
    for (const auto& s : splits)
      matchingSplits.append(s);
    d->m_matchingSplitsCount = matchingSplits.count();
    return matchingSplits;
  } else if (transaction.splitCount() > 0) {
    isMatchingSplitsEmpty = false;
  }

  // check if we're looking for transactions without assigned category
  if (!categoryMatched && transaction.splitCount() == 1 && d->m_categories.isEmpty())
    categoryMatched = true;

  // if there's no category filter and the category did not
  // match, then we still want to see this transaction if it's
  // a transfer
  if (!categoryMatched && !filter.categoryFilter)
    categoryMatched = isTransfer;

  if (isMatchingSplitsEmpty || !(accountMatched && categoryMatched)) {
    d->m_matchingSplitsCount = 0;
    return matchingSplits;
  }

  if (!d->m_reportAllSplits && !isMatchingSplitsEmpty) {
    d->m_matchingSplitsCount = 1;
    if (!d->m_matchOnly)
      matchingSplits.append(transaction.firstSplit());
  } else {
    d->m_matchingSplitsCount = matchingSplits.count();
  }

  // all filters passed, I guess we have a match
  // qDebug("  C: %d", m_matchingSplits.count());
  return matchingSplits;
}

QDate MyMoneyTransactionFilter::fromDate() const
{
  Q_D(const MyMoneyTransactionFilter);
  return d->m_fromDate;
}

QDate MyMoneyTransactionFilter::toDate() const
{
  Q_D(const MyMoneyTransactionFilter);
  return d->m_toDate;
}

bool MyMoneyTransactionFilter::matchText(const MyMoneySplit& s, const MyMoneyAccount& acc) const
{
  Q_D(const MyMoneyTransactionFilter);
  // check if the text is contained in one of the fields
  // memo, value, number, payee, tag, account
  if (d->m_filterSet.singleFilter.textFilter) {
    const auto file = MyMoneyFile::instance();
    const auto sec = file->security(acc.currencyId());
    if (s.memo().contains(d->m_text) ||
        s.shares().formatMoney(acc.fraction(sec)).contains(d->m_text) ||
        s.value().formatMoney(acc.fraction(sec)).contains(d->m_text) ||
        s.number().contains(d->m_text) ||
        (d->m_text.pattern().compare(s.transactionId())) == 0)
      return !d->m_invertText;

    if (acc.name().contains(d->m_text))
      return !d->m_invertText;

    if (!s.payeeId().isEmpty() && file->payee(s.payeeId()).name().contains(d->m_text))
      return !d->m_invertText;

    for (const auto& tag : s.tagIdList())
      if (file->tag(tag).name().contains(d->m_text))
        return !d->m_invertText;

    return d->m_invertText;
  }
  return true;
}

bool MyMoneyTransactionFilter::matchAmount(const MyMoneySplit& s) const
{
  Q_D(const MyMoneyTransactionFilter);
  if (d->m_filterSet.singleFilter.amountFilter) {
    const auto value  = s.value().abs();
    const auto shares = s.shares().abs();
    if ((value  < d->m_fromAmount || value  > d->m_toAmount) &&
        (shares < d->m_fromAmount || shares > d->m_toAmount))
      return false;
  }

  return true;
}

bool MyMoneyTransactionFilter::match(const MyMoneySplit& s) const
{
  const auto& acc = MyMoneyFile::instance()->account(s.accountId());
  return matchText(s, acc) && matchAmount(s);
}

bool MyMoneyTransactionFilter::match(const MyMoneyTransaction& transaction)
{
  Q_D(MyMoneyTransactionFilter);
  d->m_matchOnly = true;
  matchingSplits(transaction);
  d->m_matchOnly = false;
  return d->m_matchingSplitsCount > 0;
}

int MyMoneyTransactionFilter::splitState(const MyMoneySplit& split) const
{
  switch (split.reconcileFlag()) {
    default:
    case eMyMoney::Split::State::NotReconciled:
      return (int)eMyMoney::TransactionFilter::State::NotReconciled;
    case eMyMoney::Split::State::Cleared:
      return (int)eMyMoney::TransactionFilter::State::Cleared;
    case eMyMoney::Split::State::Reconciled:
      return (int)eMyMoney::TransactionFilter::State::Reconciled;
    case eMyMoney::Split::State::Frozen:
      return (int)eMyMoney::TransactionFilter::State::Frozen;
  }
}

int MyMoneyTransactionFilter::splitType(const MyMoneyTransaction& t, const MyMoneySplit& split, const MyMoneyAccount& acc) const
{
  qDebug() << "SplitType";
  if (acc.isIncomeExpense())
    return (int)eMyMoney::TransactionFilter::Type::All;

  if (t.splitCount() == 2) {
    const auto& splits = t.splits();
    const auto file = MyMoneyFile::instance();
    const auto& a = splits.at(0).id().compare(split.id()) == 0 ? acc : file->account(splits.at(0).accountId());
    const auto& b = splits.at(1).id().compare(split.id()) == 0 ? acc : file->account(splits.at(1).accountId());
    qDebug() << "first split: " << splits.at(0).accountId() << "second split: " << splits.at(1).accountId();

    if (!a.isIncomeExpense() && !b.isIncomeExpense())
      return (int)eMyMoney::TransactionFilter::Type::Transfers;
  }

  if (split.value().isPositive())
    return (int)eMyMoney::TransactionFilter::Type::Deposits;

  return (int)eMyMoney::TransactionFilter::Type::Payments;
}

eMyMoney::TransactionFilter::Validity MyMoneyTransactionFilter::validTransaction(const MyMoneyTransaction& t) const
{
  MyMoneyMoney val;

  for (const auto& split : t.splits())
    val += split.value();

  return (val == MyMoneyMoney()) ? eMyMoney::TransactionFilter::Validity::Valid : eMyMoney::TransactionFilter::Validity::Invalid;
}

bool MyMoneyTransactionFilter::includesCategory(const QString& cat) const
{
  Q_D(const MyMoneyTransactionFilter);
  return !d->m_filterSet.singleFilter.categoryFilter || d->m_categories.contains(cat);
}

bool MyMoneyTransactionFilter::includesAccount(const QString& acc) const
{
  Q_D(const MyMoneyTransactionFilter);
  return !d->m_filterSet.singleFilter.accountFilter || d->m_accounts.contains(acc);
}

bool MyMoneyTransactionFilter::includesPayee(const QString& pye) const
{
  Q_D(const MyMoneyTransactionFilter);
  return !d->m_filterSet.singleFilter.payeeFilter || d->m_payees.contains(pye);
}

bool MyMoneyTransactionFilter::includesTag(const QString& tag) const
{
  Q_D(const MyMoneyTransactionFilter);
  return !d->m_filterSet.singleFilter.tagFilter || d->m_tags.contains(tag);
}

bool MyMoneyTransactionFilter::dateFilter(QDate& from, QDate& to) const
{
  Q_D(const MyMoneyTransactionFilter);
  from = d->m_fromDate;
  to = d->m_toDate;
  return d->m_filterSet.singleFilter.dateFilter == 1;
}

bool MyMoneyTransactionFilter::amountFilter(MyMoneyMoney& from, MyMoneyMoney& to) const
{
  Q_D(const MyMoneyTransactionFilter);
  from = d->m_fromAmount;
  to = d->m_toAmount;
  return d->m_filterSet.singleFilter.amountFilter == 1;
}

bool MyMoneyTransactionFilter::numberFilter(QString& from, QString& to) const
{
  Q_D(const MyMoneyTransactionFilter);
  from = d->m_fromNr;
  to = d->m_toNr;
  return d->m_filterSet.singleFilter.nrFilter == 1;
}

bool MyMoneyTransactionFilter::payees(QStringList& list) const
{
  Q_D(const MyMoneyTransactionFilter);
  auto result = d->m_filterSet.singleFilter.payeeFilter;

  if (result) {
    QHashIterator<QString, QString> it_payee(d->m_payees);
    while (it_payee.hasNext()) {
      it_payee.next();
      list += it_payee.key();
    }
  }
  return result;
}

bool MyMoneyTransactionFilter::tags(QStringList& list) const
{
  Q_D(const MyMoneyTransactionFilter);
  auto result = d->m_filterSet.singleFilter.tagFilter;

  if (result) {
    QHashIterator<QString, QString> it_tag(d->m_tags);
    while (it_tag.hasNext()) {
      it_tag.next();
      list += it_tag.key();
    }
  }
  return result;
}

bool MyMoneyTransactionFilter::accounts(QStringList& list) const
{
  Q_D(const MyMoneyTransactionFilter);
  auto result = d->m_filterSet.singleFilter.accountFilter;

  if (result) {
    QHashIterator<QString, QString> it_account(d->m_accounts);
    while (it_account.hasNext()) {
      it_account.next();
      QString account = it_account.key();
      list += account;
    }
  }
  return result;
}

bool MyMoneyTransactionFilter::categories(QStringList& list) const
{
  Q_D(const MyMoneyTransactionFilter);
  auto result = d->m_filterSet.singleFilter.categoryFilter;

  if (result) {
    QHashIterator<QString, QString> it_category(d->m_categories);
    while (it_category.hasNext()) {
      it_category.next();
      list += it_category.key();
    }
  }
  return result;
}

bool MyMoneyTransactionFilter::types(QList<int>& list) const
{
  Q_D(const MyMoneyTransactionFilter);
  auto result = d->m_filterSet.singleFilter.typeFilter;

  if (result) {
    QHashIterator<int, QString> it_type(d->m_types);
    while (it_type.hasNext()) {
      it_type.next();
      list += it_type.key();
    }
  }
  return result;
}

bool MyMoneyTransactionFilter::states(QList<int>& list) const
{
  Q_D(const MyMoneyTransactionFilter);
  auto result = d->m_filterSet.singleFilter.stateFilter;

  if (result) {
    QHashIterator<int, QString> it_state(d->m_states);
    while (it_state.hasNext()) {
      it_state.next();
      list += it_state.key();
    }
  }
  return result;
}

bool MyMoneyTransactionFilter::validities(QList<int>& list) const
{
  Q_D(const MyMoneyTransactionFilter);
  auto result = d->m_filterSet.singleFilter.validityFilter;

  if (result) {
    QHashIterator<int, QString> it_validity(d->m_validity);
    while (it_validity.hasNext()) {
      it_validity.next();
      list += it_validity.key();
    }
  }
  return result;
}

bool MyMoneyTransactionFilter::firstType(int&i) const
{
  Q_D(const MyMoneyTransactionFilter);
  auto result = d->m_filterSet.singleFilter.typeFilter;

  if (result) {
    QHashIterator<int, QString> it_type(d->m_types);
    if (it_type.hasNext()) {
      it_type.next();
      i = it_type.key();
    }
  }
  return result;
}

bool MyMoneyTransactionFilter::firstState(int&i) const
{
  Q_D(const MyMoneyTransactionFilter);
  auto result = d->m_filterSet.singleFilter.stateFilter;

  if (result) {
    QHashIterator<int, QString> it_state(d->m_states);
    if (it_state.hasNext()) {
      it_state.next();
      i = it_state.key();
    }
  }
  return result;
}

bool MyMoneyTransactionFilter::firstValidity(int&i) const
{
  Q_D(const MyMoneyTransactionFilter);
  auto result = d->m_filterSet.singleFilter.validityFilter;

  if (result) {
    QHashIterator<int, QString> it_validity(d->m_validity);
    if (it_validity.hasNext()) {
      it_validity.next();
      i = it_validity.key();
    }
  }
  return result;
}

bool MyMoneyTransactionFilter::textFilter(QRegExp& exp) const
{
  Q_D(const MyMoneyTransactionFilter);
  exp = d->m_text;
  return d->m_filterSet.singleFilter.textFilter == 1;
}

bool MyMoneyTransactionFilter::isInvertingText() const
{
  Q_D(const MyMoneyTransactionFilter);
  return d->m_invertText;
}

void MyMoneyTransactionFilter::setDateFilter(eMyMoney::TransactionFilter::Date range)
{
  QDate from, to;
  if (translateDateRange(range, from, to))
    setDateFilter(from, to);
}

static int fiscalYearStartMonth = 1;
static int fiscalYearStartDay = 1;

void MyMoneyTransactionFilter::setFiscalYearStart(int firstMonth, int firstDay)
{
  fiscalYearStartMonth = firstMonth;
  fiscalYearStartDay = firstDay;
}

bool MyMoneyTransactionFilter::translateDateRange(eMyMoney::TransactionFilter::Date id, QDate& start, QDate& end)
{
  bool rc = true;
  int yr = QDate::currentDate().year();
  int mon = QDate::currentDate().month();

  switch (id) {
    case eMyMoney::TransactionFilter::Date::All:
      start = QDate();
      end = QDate();
      break;
    case eMyMoney::TransactionFilter::Date::AsOfToday:
      start = QDate();
      end =  QDate::currentDate();
      break;
    case eMyMoney::TransactionFilter::Date::CurrentMonth:
      start = QDate(yr, mon, 1);
      end = QDate(yr, mon, 1).addMonths(1).addDays(-1);
      break;
    case eMyMoney::TransactionFilter::Date::CurrentYear:
      start = QDate(yr, 1, 1);
      end = QDate(yr, 12, 31);
      break;
    case eMyMoney::TransactionFilter::Date::MonthToDate:
      start = QDate(yr, mon, 1);
      end = QDate::currentDate();
      break;
    case eMyMoney::TransactionFilter::Date::YearToDate:
      start = QDate(yr, 1, 1);
      end = QDate::currentDate();
      break;
    case eMyMoney::TransactionFilter::Date::YearToMonth:
      start = QDate(yr, 1, 1);
      end = QDate(yr, mon, 1).addDays(-1);
      break;
    case eMyMoney::TransactionFilter::Date::LastMonth:
      start = QDate(yr, mon, 1).addMonths(-1);
      end = QDate(yr, mon, 1).addDays(-1);
      break;
    case eMyMoney::TransactionFilter::Date::LastYear:
      start = QDate(yr, 1, 1).addYears(-1);
      end = QDate(yr, 12, 31).addYears(-1);
      break;
    case eMyMoney::TransactionFilter::Date::Last7Days:
      start = QDate::currentDate().addDays(-7);
      end = QDate::currentDate();
      break;
    case eMyMoney::TransactionFilter::Date::Last30Days:
      start = QDate::currentDate().addDays(-30);
      end = QDate::currentDate();
      break;
    case eMyMoney::TransactionFilter::Date::Last3Months:
      start = QDate::currentDate().addMonths(-3);
      end = QDate::currentDate();
      break;
    case eMyMoney::TransactionFilter::Date::Last6Months:
      start = QDate::currentDate().addMonths(-6);
      end = QDate::currentDate();
      break;
    case eMyMoney::TransactionFilter::Date::Last11Months:
      start = QDate(yr, mon, 1).addMonths(-12);
      end = QDate(yr, mon, 1).addDays(-1);
      break;
    case eMyMoney::TransactionFilter::Date::Last12Months:
      start = QDate::currentDate().addMonths(-12);
      end = QDate::currentDate();
      break;
    case eMyMoney::TransactionFilter::Date::Next7Days:
      start = QDate::currentDate();
      end = QDate::currentDate().addDays(7);
      break;
    case eMyMoney::TransactionFilter::Date::Next30Days:
      start = QDate::currentDate();
      end = QDate::currentDate().addDays(30);
      break;
    case eMyMoney::TransactionFilter::Date::Next3Months:
      start = QDate::currentDate();
      end = QDate::currentDate().addMonths(3);
      break;
    case eMyMoney::TransactionFilter::Date::Next6Months:
      start = QDate::currentDate();
      end = QDate::currentDate().addMonths(6);
      break;
    case eMyMoney::TransactionFilter::Date::Next12Months:
      start = QDate::currentDate();
      end = QDate::currentDate().addMonths(12);
      break;
    case eMyMoney::TransactionFilter::Date::Next18Months:
      start = QDate::currentDate();
      end = QDate::currentDate().addMonths(18);
      break;
    case eMyMoney::TransactionFilter::Date::UserDefined:
      start = QDate();
      end = QDate();
      break;
    case eMyMoney::TransactionFilter::Date::Last3ToNext3Months:
      start = QDate::currentDate().addMonths(-3);
      end = QDate::currentDate().addMonths(3);
      break;
    case eMyMoney::TransactionFilter::Date::CurrentQuarter:
      start = QDate(yr, mon - ((mon - 1) % 3), 1);
      end = start.addMonths(3).addDays(-1);
      break;
    case eMyMoney::TransactionFilter::Date::LastQuarter:
      start = QDate(yr, mon - ((mon - 1) % 3), 1).addMonths(-3);
      end = start.addMonths(3).addDays(-1);
      break;
    case eMyMoney::TransactionFilter::Date::NextQuarter:
      start = QDate(yr, mon - ((mon - 1) % 3), 1).addMonths(3);
      end = start.addMonths(3).addDays(-1);
      break;
    case eMyMoney::TransactionFilter::Date::CurrentFiscalYear:
      start = QDate(QDate::currentDate().year(), fiscalYearStartMonth, fiscalYearStartDay);
      if (QDate::currentDate() < start)
        start = start.addYears(-1);
      end = start.addYears(1).addDays(-1);
      break;
    case eMyMoney::TransactionFilter::Date::LastFiscalYear:
      start = QDate(QDate::currentDate().year(), fiscalYearStartMonth, fiscalYearStartDay);
      if (QDate::currentDate() < start)
        start = start.addYears(-1);
      start = start.addYears(-1);
      end = start.addYears(1).addDays(-1);
      break;
    case eMyMoney::TransactionFilter::Date::Today:
      start = QDate::currentDate();
      end =  QDate::currentDate();
      break;
    default:
      qWarning("Unknown date identifier %d in MyMoneyTransactionFilter::translateDateRange()", (int)id);
      rc = false;
      break;
  }
  return rc;
}

MyMoneyTransactionFilter::FilterSet MyMoneyTransactionFilter::filterSet() const
{
  Q_D(const MyMoneyTransactionFilter);
  return d->m_filterSet;
}

void MyMoneyTransactionFilter::removeReference(const QString& id)
{
  Q_D(MyMoneyTransactionFilter);
  if (d->m_accounts.end() != d->m_accounts.find(id)) {
    qDebug("%s", qPrintable(QString("Remove account '%1' from report").arg(id)));
    d->m_accounts.take(id);
  } else if (d->m_categories.end() != d->m_categories.find(id)) {
    qDebug("%s", qPrintable(QString("Remove category '%1' from report").arg(id)));
    d->m_categories.remove(id);
  } else if (d->m_payees.end() != d->m_payees.find(id)) {
    qDebug("%s", qPrintable(QString("Remove payee '%1' from report").arg(id)));
    d->m_payees.remove(id);
  } else if (d->m_tags.end() != d->m_tags.find(id)) {
    qDebug("%s", qPrintable(QString("Remove tag '%1' from report").arg(id)));
    d->m_tags.remove(id);
  }
}
