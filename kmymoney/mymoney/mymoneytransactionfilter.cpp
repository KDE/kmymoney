/***************************************************************************
                          mymoneytransactionfilter.cpp  -  description
                             -------------------
    begin                : Fri Aug 22 2003
    copyright            : (C) 2003 by Thomas Baumgart
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
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

#include "mymoneytransactionfilter.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDate>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneymoney.h"
#include "mymoneyfile.h"
#include "mymoneyaccount.h"
#include "mymoneypayee.h"
#include "mymoneytag.h"
#include "mymoneytransaction.h"
#include "mymoneysplit.h"
#include "mymoneyenums.h"

class MyMoneyTransactionFilterPrivate {

public:
  MyMoneyTransactionFilter::FilterSet m_filterSet;
  bool                m_reportAllSplits;
  bool                m_considerCategory;

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
  QList<MyMoneySplit> m_matchingSplits;

};

MyMoneyTransactionFilter::MyMoneyTransactionFilter() :
  d_ptr(new MyMoneyTransactionFilterPrivate)
{
  Q_D(MyMoneyTransactionFilter);
  d->m_filterSet.allFilter = 0;
  d->m_reportAllSplits = true;
  d->m_considerCategory = true;
  d->m_invertText = false;
}

MyMoneyTransactionFilter::MyMoneyTransactionFilter(const QString& id) :
  d_ptr(new MyMoneyTransactionFilterPrivate)
{
  Q_D(MyMoneyTransactionFilter);
  d->m_filterSet.allFilter = 0;
  d->m_reportAllSplits = false;
  d->m_considerCategory = false;
  d->m_invertText = false;

  addAccount(id);
  // addCategory(id);
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
  d->m_matchingSplits.clear();
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
  QStringList::ConstIterator it;

  d->m_filterSet.singleFilter.accountFilter = 1;
  for (it = ids.begin(); it != ids.end(); ++it)
    addAccount(*it);
}

void MyMoneyTransactionFilter::addAccount(const QString& id)
{
  Q_D(MyMoneyTransactionFilter);
  if (!d->m_accounts.isEmpty() && !id.isEmpty()) {
    if (d->m_accounts.find(id) != d->m_accounts.end())
      return;
  }
  d->m_filterSet.singleFilter.accountFilter = 1;
  if (!id.isEmpty())
    d->m_accounts.insert(id, "");
}

void MyMoneyTransactionFilter::addCategory(const QStringList& ids)
{
  Q_D(MyMoneyTransactionFilter);
  QStringList::ConstIterator it;

  d->m_filterSet.singleFilter.categoryFilter = 1;
  for (it = ids.begin(); it != ids.end(); ++it)
    addCategory(*it);
}

void MyMoneyTransactionFilter::addCategory(const QString& id)
{
  Q_D(MyMoneyTransactionFilter);
  if (!d->m_categories.isEmpty() && !id.isEmpty()) {
    if (d->m_categories.end() != d->m_categories.find(id))
      return;
  }
  d->m_filterSet.singleFilter.categoryFilter = 1;
  if (!id.isEmpty())
    d->m_categories.insert(id, "");
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
  if (from > to) {
    MyMoneyMoney tmp = d->m_fromAmount;
    d->m_fromAmount = d->m_toAmount;
    d->m_toAmount = tmp;
  }
}

void MyMoneyTransactionFilter::addPayee(const QString& id)
{
  Q_D(MyMoneyTransactionFilter);
  if (!d->m_payees.isEmpty() && !id.isEmpty()) {
    if (d->m_payees.find(id) != d->m_payees.end())
      return;
  }
  d->m_filterSet.singleFilter.payeeFilter = 1;
  if (!id.isEmpty())
    d->m_payees.insert(id, "");
}

void MyMoneyTransactionFilter::addTag(const QString& id)
{
  Q_D(MyMoneyTransactionFilter);
  if (!d->m_tags.isEmpty() && !id.isEmpty()) {
    if (d->m_tags.find(id) != d->m_tags.end())
      return;
  }
  d->m_filterSet.singleFilter.tagFilter = 1;
  if (!id.isEmpty())
    d->m_tags.insert(id, "");
}

void MyMoneyTransactionFilter::addType(const int type)
{
  Q_D(MyMoneyTransactionFilter);
  if (!d->m_types.isEmpty()) {
    if (d->m_types.find(type) != d->m_types.end())
      return;
  }
  d->m_filterSet.singleFilter.typeFilter = 1;
  d->m_types.insert(type, "");
}

void MyMoneyTransactionFilter::addState(const int state)
{
  Q_D(MyMoneyTransactionFilter);
  if (!d->m_states.isEmpty()) {
    if (d->m_states.find(state) != d->m_states.end())
      return;
  }
  d->m_filterSet.singleFilter.stateFilter = 1;
  d->m_states.insert(state, "");
}

void MyMoneyTransactionFilter::addValidity(const int type)
{
  Q_D(MyMoneyTransactionFilter);
  if (!d->m_validity.isEmpty()) {
    if (d->m_validity.find(type) != d->m_validity.end())
      return;
  }
  d->m_filterSet.singleFilter.validityFilter = 1;
  d->m_validity.insert(type, "");
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

QList<MyMoneySplit> MyMoneyTransactionFilter::matchingSplits() const
{
  Q_D(const MyMoneyTransactionFilter);
  return d->m_matchingSplits;
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

bool MyMoneyTransactionFilter::matchText(const MyMoneySplit * const sp) const
{
  Q_D(const MyMoneyTransactionFilter);
  // check if the text is contained in one of the fields
  // memo, value, number, payee, tag, account, date
  if (d->m_filterSet.singleFilter.textFilter) {
    MyMoneyFile* file = MyMoneyFile::instance();
    const MyMoneyAccount& acc = file->account(sp->accountId());
    const MyMoneySecurity& sec = file->security(acc.currencyId());
    if (sp->memo().contains(d->m_text)
        || sp->shares().formatMoney(acc.fraction(sec)).contains(d->m_text)
        || sp->value().formatMoney(acc.fraction(sec)).contains(d->m_text)
        || sp->number().contains(d->m_text)
        || (d->m_text.pattern() ==  sp->transactionId()))
      return !d->m_invertText;

    if (acc.name().contains(d->m_text))
      return !d->m_invertText;

    if (!sp->payeeId().isEmpty()) {
      const MyMoneyPayee& payee = file->payee(sp->payeeId());
      if (payee.name().contains(d->m_text))
        return !d->m_invertText;
    }

    if (!sp->tagIdList().isEmpty()) {
      QList<QString>::ConstIterator it_s;
      QList<QString> t = sp->tagIdList();
      for (it_s = t.constBegin(); it_s != t.constEnd(); ++it_s) {
        const MyMoneyTag& tag = file->tag((*it_s));
        if (tag.name().contains(d->m_text))
          return !d->m_invertText;
      }
    }

    return d->m_invertText;
  }
  return true;
}

bool MyMoneyTransactionFilter::matchAmount(const MyMoneySplit * const sp) const
{
  Q_D(const MyMoneyTransactionFilter);
  if (d->m_filterSet.singleFilter.amountFilter) {
    if (((sp->value().abs() < d->m_fromAmount) || sp->value().abs() > d->m_toAmount)
        && ((sp->shares().abs() < d->m_fromAmount) || sp->shares().abs() > d->m_toAmount))
      return false;
  }

  return true;
}

bool MyMoneyTransactionFilter::match(const MyMoneySplit * const sp) const
{
  return matchText(sp) && matchAmount(sp);
}

bool MyMoneyTransactionFilter::match(const MyMoneyTransaction& transaction)
{
  Q_D(MyMoneyTransactionFilter);
  auto file = MyMoneyFile::instance();

  d->m_matchingSplits.clear();

  // qDebug("T: %s", transaction.id().data());
  // if no filter is set, we can safely return a match
  // if we should report all splits, then we collect them
  if (!d->m_filterSet.allFilter) {
    if (d->m_reportAllSplits) {
      d->m_matchingSplits = transaction.splits();
    }
    return true;
  }

  // perform checks on the MyMoneyTransaction object first

  // check the date range
  if (d->m_filterSet.singleFilter.dateFilter) {
    if (d->m_fromDate != QDate()) {
      if (transaction.postDate() < d->m_fromDate)
        return false;
    }

    if (d->m_toDate != QDate()) {
      if (transaction.postDate() > d->m_toDate)
        return false;
    }
  }

  // construct a local list of pointers to all splits and
  // remove the ones that do not match account and/or categories.

  QList<const MyMoneySplit*> matchingSplits;
  //QList<MyMoneySplit> matchingSplits;
  foreach (const MyMoneySplit& s, transaction.splits()) {
    matchingSplits.append(&s);
  }

  bool categoryMatched = !d->m_filterSet.singleFilter.categoryFilter;
  bool accountMatched = !d->m_filterSet.singleFilter.accountFilter;
  bool isTransfer = true;

  // check the transaction's validity
  if (d->m_filterSet.singleFilter.validityFilter) {
    if (d->m_validity.count() > 0) {
      if (d->m_validity.end() == d->m_validity.find((int)validTransaction(transaction)))
        return false;
    }
  }

  QMutableListIterator<const MyMoneySplit*> sp(matchingSplits);

  if (d->m_filterSet.singleFilter.accountFilter == 1
      || d->m_filterSet.singleFilter.categoryFilter == 1) {
    for (sp.toFront(); sp.hasNext();) {
      bool removeSplit = true;
      const MyMoneySplit* & s = sp.next();
      const MyMoneyAccount& acc = file->account(s->accountId());
      if (d->m_considerCategory) {
        switch (acc.accountGroup()) {
          case eMyMoney::Account::Type::Income:
          case eMyMoney::Account::Type::Expense:
            isTransfer = false;
            // check if the split references one of the categories in the list
            if (d->m_filterSet.singleFilter.categoryFilter) {
              if (d->m_categories.count() > 0) {
                if (d->m_categories.end() != d->m_categories.find(s->accountId())) {
                  categoryMatched = true;
                  removeSplit = false;
                }
              } else {
                // we're looking for transactions with 'no' categories
                return false;
              }
            }
            break;

          default:
            // check if the split references one of the accounts in the list
            if (d->m_filterSet.singleFilter.accountFilter) {
              if (d->m_accounts.count() > 0) {
                if (d->m_accounts.end() != d->m_accounts.find(s->accountId())) {
                  accountMatched = true;
                  removeSplit = false;
                }
              }
            } else
              removeSplit = false;

            break;
        }

      } else {
        if (d->m_filterSet.singleFilter.accountFilter) {
          if (d->m_accounts.count() > 0) {
            if (d->m_accounts.end() != d->m_accounts.find(s->accountId())) {
              accountMatched = true;
              removeSplit = false;
            }
          }
        } else
          removeSplit = false;
      }

      if (removeSplit) {
        // qDebug(" S: %s", (*it).id().data());
        sp.remove();
      }
    }
  }

  // check if we're looking for transactions without assigned category
  if (!categoryMatched && transaction.splitCount() == 1 && d->m_categories.count() == 0) {
    categoryMatched = true;
  }

  // if there's no category filter and the category did not
  // match, then we still want to see this transaction if it's
  // a transfer
  if (!categoryMatched && !d->m_filterSet.singleFilter.categoryFilter)
    categoryMatched = isTransfer;

  if (matchingSplits.count() == 0
      || !(accountMatched && categoryMatched))
    return false;

  FilterSet filterSet = d->m_filterSet;
  filterSet.singleFilter.dateFilter =
    filterSet.singleFilter.accountFilter =
      filterSet.singleFilter.categoryFilter = 0;

  // check if we still have something to do
  if (filterSet.allFilter != 0) {
    for (sp.toFront(); sp.hasNext();) {
      bool removeSplit = true;
      const MyMoneySplit* & s = sp.next();
      removeSplit = !(matchAmount(s) && matchText(s));

      const MyMoneyAccount& acc = file->account(s->accountId());

      // Determine if this account is a category or an account
      bool isCategory = false;
      switch (acc.accountGroup()) {
        case eMyMoney::Account::Type::Income:
        case eMyMoney::Account::Type::Expense:
          isCategory = true;
        default:
          break;
      }

      if (!isCategory && !removeSplit) {
        // check the payee list
        if (!removeSplit && d->m_filterSet.singleFilter.payeeFilter) {
          if (d->m_payees.count() > 0) {
            if (s->payeeId().isEmpty() || d->m_payees.end() == d->m_payees.find(s->payeeId()))
              removeSplit = true;
          } else if (!s->payeeId().isEmpty())
            removeSplit = true;
        }

        // check the tag list
        if (!removeSplit && d->m_filterSet.singleFilter.tagFilter) {
          if (d->m_tags.count() > 0) {
            if (s->tagIdList().isEmpty())
              removeSplit = true;
            else {
              bool found = false;
              for (int i = 0; i < s->tagIdList().size(); i++) {
                if (d->m_tags.end() != d->m_tags.find(s->tagIdList()[i])) {
                  found = true;
                  break;
                }
              }
              if (!found) {
                removeSplit = true;
              }
            }
          } else if (!s->tagIdList().isEmpty())
            removeSplit = true;
        }

        // check the type list
        if (!removeSplit && d->m_filterSet.singleFilter.typeFilter) {
          if (d->m_types.count() > 0) {
            if (d->m_types.end() == d->m_types.find(splitType(transaction, *s)))
              removeSplit = true;
          }
        }

        // check the state list
        if (!removeSplit && d->m_filterSet.singleFilter.stateFilter) {
          if (d->m_states.count() > 0) {
            if (d->m_states.end() == d->m_states.find(splitState(*s)))
              removeSplit = true;
          }
        }

        if (!removeSplit && d->m_filterSet.singleFilter.nrFilter) {
          if (!d->m_fromNr.isEmpty()) {
            if (s->number() < d->m_fromNr)
              removeSplit = true;
          }
          if (!d->m_toNr.isEmpty()) {
            if (s->number() > d->m_toNr)
              removeSplit = true;
          }
        }
      } else if (d->m_filterSet.singleFilter.payeeFilter
                 || d->m_filterSet.singleFilter.tagFilter
                 || d->m_filterSet.singleFilter.typeFilter
                 || d->m_filterSet.singleFilter.stateFilter
                 || d->m_filterSet.singleFilter.nrFilter)
        removeSplit = true;

      if (removeSplit) {
        // qDebug(" S: %s", (*it).id().data());
        sp.remove();
      }
    }
  }

  if (d->m_reportAllSplits == false && matchingSplits.count() != 0) {
    d->m_matchingSplits.append(transaction.splits()[0]);
  } else {
    foreach (const MyMoneySplit* s, matchingSplits) {
      d->m_matchingSplits.append(*s);
    }
  }
  // all filters passed, I guess we have a match
  // qDebug("  C: %d", m_matchingSplits.count());
  return matchingSplits.count() != 0;
}

int MyMoneyTransactionFilter::splitState(const MyMoneySplit& split) const
{
  int rc = (int)eMyMoney::TransactionFilter::State::NotReconciled;

  switch (split.reconcileFlag()) {
    default:
    case eMyMoney::Split::State::NotReconciled:
      break;;

    case eMyMoney::Split::State::Cleared:
      rc = (int)eMyMoney::TransactionFilter::State::Cleared;
      break;

    case eMyMoney::Split::State::Reconciled:
      rc = (int)eMyMoney::TransactionFilter::State::Reconciled;
      break;

    case eMyMoney::Split::State::Frozen:
      rc = (int)eMyMoney::TransactionFilter::State::Frozen;
      break;
  }
  return rc;
}

int MyMoneyTransactionFilter::splitType(const MyMoneyTransaction& t, const MyMoneySplit& split) const
{
  auto file = MyMoneyFile::instance();
  MyMoneyAccount a, b;
  a = file->account(split.accountId());
  if ((a.accountGroup() == eMyMoney::Account::Type::Income
       || a.accountGroup() == eMyMoney::Account::Type::Expense))
    return (int)eMyMoney::TransactionFilter::Type::All;

  if (t.splitCount() == 2) {
    QString ida, idb;
    if (t.splits().size() > 0)
      ida = t.splits()[0].accountId();
    if (t.splits().size() > 1)
      idb = t.splits()[1].accountId();

    a = file->account(ida);
    b = file->account(idb);
    if ((a.accountGroup() != eMyMoney::Account::Type::Expense
         && a.accountGroup() != eMyMoney::Account::Type::Income)
        && (b.accountGroup() != eMyMoney::Account::Type::Expense
            && b.accountGroup() != eMyMoney::Account::Type::Income))
      return (int)eMyMoney::TransactionFilter::Type::Transfers;
  }

  if (split.value().isPositive())
    return (int)eMyMoney::TransactionFilter::Type::Deposits;

  return (int)eMyMoney::TransactionFilter::Type::Payments;
}

eMyMoney::TransactionFilter::Validity MyMoneyTransactionFilter::validTransaction(const MyMoneyTransaction& t) const
{
  QList<MyMoneySplit>::ConstIterator it_s;
  MyMoneyMoney val;

  for (it_s = t.splits().begin(); it_s != t.splits().end(); ++it_s) {
    val += (*it_s).value();
  }
  return (val == MyMoneyMoney()) ? eMyMoney::TransactionFilter::Validity::Valid : eMyMoney::TransactionFilter::Validity::Invalid;
}

bool MyMoneyTransactionFilter::includesCategory(const QString& cat) const
{
  Q_D(const MyMoneyTransactionFilter);
  return (! d->m_filterSet.singleFilter.categoryFilter) || d->m_categories.end() != d->m_categories.find(cat);
}

bool MyMoneyTransactionFilter::includesAccount(const QString& acc) const
{
  Q_D(const MyMoneyTransactionFilter);
  return (! d->m_filterSet.singleFilter.accountFilter) || d->m_accounts.end() != d->m_accounts.find(acc);
}

bool MyMoneyTransactionFilter::includesPayee(const QString& pye) const
{
  Q_D(const MyMoneyTransactionFilter);
  return (! d->m_filterSet.singleFilter.payeeFilter) || d->m_payees.end() != d->m_payees.find(pye);
}

bool MyMoneyTransactionFilter::includesTag(const QString& tag) const
{
  Q_D(const MyMoneyTransactionFilter);
  return (! d->m_filterSet.singleFilter.tagFilter) || d->m_tags.end() != d->m_tags.find(tag);
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
