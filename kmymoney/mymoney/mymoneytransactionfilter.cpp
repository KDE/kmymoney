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

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyfile.h"

MyMoneyTransactionFilter::MyMoneyTransactionFilter()
{
  m_filterSet.allFilter = 0;
  m_reportAllSplits = true;
  m_considerCategory = true;
  m_invertText = false;
}

MyMoneyTransactionFilter::MyMoneyTransactionFilter(const QString& id)
{
  m_filterSet.allFilter = 0;
  m_reportAllSplits = false;
  m_considerCategory = false;
  m_invertText = false;

  addAccount(id);
  // addCategory(id);
}

MyMoneyTransactionFilter::~MyMoneyTransactionFilter()
{
}

void MyMoneyTransactionFilter::clear()
{
  m_filterSet.allFilter = 0;
  m_invertText = false;
  m_accounts.clear();
  m_categories.clear();
  m_payees.clear();
  m_tags.clear();
  m_types.clear();
  m_states.clear();
  m_validity.clear();
  m_matchingSplits.clear();
  m_fromDate = QDate();
  m_toDate = QDate();
}

void MyMoneyTransactionFilter::clearAccountFilter()
{
  m_filterSet.singleFilter.accountFilter = 0;
  m_accounts.clear();
}

void MyMoneyTransactionFilter::setTextFilter(const QRegExp& text, bool invert)
{
  m_filterSet.singleFilter.textFilter = 1;
  m_invertText = invert;
  m_text = text;
}

void MyMoneyTransactionFilter::addAccount(const QStringList& ids)
{
  QStringList::ConstIterator it;

  m_filterSet.singleFilter.accountFilter = 1;
  for (it = ids.begin(); it != ids.end(); ++it)
    addAccount(*it);
}

void MyMoneyTransactionFilter::addAccount(const QString& id)
{
  if (!m_accounts.isEmpty() && !id.isEmpty()) {
    if (m_accounts.find(id) != m_accounts.end())
      return;
  }
  m_filterSet.singleFilter.accountFilter = 1;
  if (!id.isEmpty())
    m_accounts.insert(id, "");
}

void MyMoneyTransactionFilter::addCategory(const QStringList& ids)
{
  QStringList::ConstIterator it;

  m_filterSet.singleFilter.categoryFilter = 1;
  for (it = ids.begin(); it != ids.end(); ++it)
    addCategory(*it);
}

void MyMoneyTransactionFilter::addCategory(const QString& id)
{
  if (!m_categories.isEmpty() && !id.isEmpty()) {
    if (m_categories.end() != m_categories.find(id))
      return;
  }
  m_filterSet.singleFilter.categoryFilter = 1;
  if (!id.isEmpty())
    m_categories.insert(id, "");
}

void MyMoneyTransactionFilter::setDateFilter(const QDate& from, const QDate& to)
{
  m_filterSet.singleFilter.dateFilter = from.isValid() | to.isValid();
  m_fromDate = from;
  m_toDate = to;
}

void MyMoneyTransactionFilter::setAmountFilter(const MyMoneyMoney& from, const MyMoneyMoney& to)
{
  m_filterSet.singleFilter.amountFilter = 1;
  m_fromAmount = from.abs();
  m_toAmount = to.abs();

  // make sure that the user does not try to fool us  ;-)
  if (from > to) {
    MyMoneyMoney tmp = m_fromAmount;
    m_fromAmount = m_toAmount;
    m_toAmount = tmp;
  }
}

void MyMoneyTransactionFilter::addPayee(const QString& id)
{
  if (!m_payees.isEmpty() && !id.isEmpty()) {
    if (m_payees.find(id) != m_payees.end())
      return;
  }
  m_filterSet.singleFilter.payeeFilter = 1;
  if (!id.isEmpty())
    m_payees.insert(id, "");
}

void MyMoneyTransactionFilter::addTag(const QString& id)
{
  if (!m_tags.isEmpty() && !id.isEmpty()) {
    if (m_tags.find(id) != m_tags.end())
      return;
  }
  m_filterSet.singleFilter.tagFilter = 1;
  if (!id.isEmpty())
    m_tags.insert(id, "");
}

void MyMoneyTransactionFilter::addType(const int type)
{
  if (!m_types.isEmpty()) {
    if (m_types.find(type) != m_types.end())
      return;
  }
  m_filterSet.singleFilter.typeFilter = 1;
  m_types.insert(type, "");
}

void MyMoneyTransactionFilter::addState(const int state)
{
  if (!m_states.isEmpty()) {
    if (m_states.find(state) != m_states.end())
      return;
  }
  m_filterSet.singleFilter.stateFilter = 1;
  m_states.insert(state, "");
}

void MyMoneyTransactionFilter::addValidity(const int type)
{
  if (!m_validity.isEmpty()) {
    if (m_validity.find(type) != m_validity.end())
      return;
  }
  m_filterSet.singleFilter.validityFilter = 1;
  m_validity.insert(type, "");
}

void MyMoneyTransactionFilter::setNumberFilter(const QString& from, const QString& to)
{
  m_filterSet.singleFilter.nrFilter = 1;
  m_fromNr = from;
  m_toNr = to;
}

void MyMoneyTransactionFilter::setReportAllSplits(const bool report)
{
  m_reportAllSplits = report;
}

void MyMoneyTransactionFilter::setConsiderCategory(const bool check)
{
  m_considerCategory = check;
}

const QList<MyMoneySplit>& MyMoneyTransactionFilter::matchingSplits() const
{
  return m_matchingSplits;
}

bool MyMoneyTransactionFilter::matchText(const MyMoneySplit * const sp) const
{
  // check if the text is contained in one of the fields
  // memo, value, number, payee, tag, account
  if (m_filterSet.singleFilter.textFilter) {
    MyMoneyFile* file = MyMoneyFile::instance();
    const MyMoneyAccount& acc = file->account(sp->accountId());
    const MyMoneySecurity& sec = file->security(acc.currencyId());
    if (sp->memo().contains(m_text)
        || sp->shares().formatMoney(acc.fraction(sec)).contains(m_text)
        || sp->value().formatMoney(acc.fraction(sec)).contains(m_text)
        || sp->number().contains(m_text)
        || (m_text.pattern() ==  sp->transactionId()))
      return !m_invertText;

    if (acc.name().contains(m_text))
      return !m_invertText;

    if (!sp->payeeId().isEmpty()) {
      const MyMoneyPayee& payee = file->payee(sp->payeeId());
      if (payee.name().contains(m_text))
        return !m_invertText;
    }

    if (!sp->tagIdList().isEmpty()) {
      QList<QString>::ConstIterator it_s;
      QList<QString> t = sp->tagIdList();
      for (it_s = t.constBegin(); it_s != t.constEnd(); ++it_s) {
        const MyMoneyTag& tag = file->tag((*it_s));
        if (tag.name().contains(m_text))
          return !m_invertText;
      }
    }

    return m_invertText;
  }
  return true;
}

bool MyMoneyTransactionFilter::matchAmount(const MyMoneySplit * const sp) const
{
  if (m_filterSet.singleFilter.amountFilter) {
    if (((sp->value().abs() < m_fromAmount) || sp->value().abs() > m_toAmount)
        && ((sp->shares().abs() < m_fromAmount) || sp->shares().abs() > m_toAmount))
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
  MyMoneyFile* file = MyMoneyFile::instance();

  m_matchingSplits.clear();

  // qDebug("T: %s", transaction.id().data());
  // if no filter is set, we can safely return a match
  // if we should report all splits, then we collect them
  if (!m_filterSet.allFilter) {
    if (m_reportAllSplits) {
      m_matchingSplits = transaction.splits();
    }
    return true;
  }

  // perform checks on the MyMoneyTransaction object first

  // check the date range
  if (m_filterSet.singleFilter.dateFilter) {
    if (m_fromDate != QDate()) {
      if (transaction.postDate() < m_fromDate)
        return false;
    }

    if (m_toDate != QDate()) {
      if (transaction.postDate() > m_toDate)
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

  bool categoryMatched = !m_filterSet.singleFilter.categoryFilter;
  bool accountMatched = !m_filterSet.singleFilter.accountFilter;
  bool isTransfer = true;

  // check the transaction's validity
  if (m_filterSet.singleFilter.validityFilter) {
    if (m_validity.count() > 0) {
      if (m_validity.end() == m_validity.find(validTransaction(transaction)))
        return false;
    }
  }

  QMutableListIterator<const MyMoneySplit*> sp(matchingSplits);

  if (m_filterSet.singleFilter.accountFilter == 1
      || m_filterSet.singleFilter.categoryFilter == 1) {
    for (sp.toFront(); sp.hasNext();) {
      bool removeSplit = true;
      const MyMoneySplit* & s = sp.next();
      const MyMoneyAccount& acc = file->account(s->accountId());
      if (m_considerCategory) {
        switch (acc.accountGroup()) {
          case MyMoneyAccount::Income:
          case MyMoneyAccount::Expense:
            isTransfer = false;
            // check if the split references one of the categories in the list
            if (m_filterSet.singleFilter.categoryFilter) {
              if (m_categories.count() > 0) {
                if (m_categories.end() != m_categories.find(s->accountId())) {
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
            if (m_filterSet.singleFilter.accountFilter) {
              if (m_accounts.count() > 0) {
                if (m_accounts.end() != m_accounts.find(s->accountId())) {
                  accountMatched = true;
                  removeSplit = false;
                }
              }
            } else
              removeSplit = false;

            break;
        }

      } else {
        if (m_filterSet.singleFilter.accountFilter) {
          if (m_accounts.count() > 0) {
            if (m_accounts.end() != m_accounts.find(s->accountId())) {
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
  if (!categoryMatched && transaction.splitCount() == 1 && m_categories.count() == 0) {
    categoryMatched = true;
  }

  // if there's no category filter and the category did not
  // match, then we still want to see this transaction if it's
  // a transfer
  if (!categoryMatched && !m_filterSet.singleFilter.categoryFilter)
    categoryMatched = isTransfer;

  if (matchingSplits.count() == 0
      || !(accountMatched && categoryMatched))
    return false;

  FilterSet filterSet = m_filterSet;
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
        case MyMoneyAccount::Income:
        case MyMoneyAccount::Expense:
          isCategory = true;
        default:
          break;
      }

      if (!isCategory && !removeSplit) {
        // check the payee list
        if (!removeSplit && m_filterSet.singleFilter.payeeFilter) {
          if (m_payees.count() > 0) {
            if (s->payeeId().isEmpty() || m_payees.end() == m_payees.find(s->payeeId()))
              removeSplit = true;
          } else if (!s->payeeId().isEmpty())
            removeSplit = true;
        }

        // check the tag list
        if (!removeSplit && m_filterSet.singleFilter.tagFilter) {
          if (m_tags.count() > 0) {
            if (s->tagIdList().isEmpty())
              removeSplit = true;
            else {
              bool found = false;
              for (int i = 0; i < s->tagIdList().size(); i++) {
                if (m_tags.end() != m_tags.find(s->tagIdList()[i])) {
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
        if (!removeSplit && m_filterSet.singleFilter.typeFilter) {
          if (m_types.count() > 0) {
            if (m_types.end() == m_types.find(splitType(transaction, *s)))
              removeSplit = true;
          }
        }

        // check the state list
        if (!removeSplit && m_filterSet.singleFilter.stateFilter) {
          if (m_states.count() > 0) {
            if (m_states.end() == m_states.find(splitState(*s)))
              removeSplit = true;
          }
        }

        if (!removeSplit && m_filterSet.singleFilter.nrFilter) {
          if (!m_fromNr.isEmpty()) {
            if (s->number() < m_fromNr)
              removeSplit = true;
          }
          if (!m_toNr.isEmpty()) {
            if (s->number() > m_toNr)
              removeSplit = true;
          }
        }
      } else if (m_filterSet.singleFilter.payeeFilter
                 || m_filterSet.singleFilter.tagFilter
                 || m_filterSet.singleFilter.typeFilter
                 || m_filterSet.singleFilter.stateFilter
                 || m_filterSet.singleFilter.nrFilter)
        removeSplit = true;

      if (removeSplit) {
        // qDebug(" S: %s", (*it).id().data());
        sp.remove();
      }
    }
  }

  if (m_reportAllSplits == false && matchingSplits.count() != 0) {
    m_matchingSplits.append(transaction.splits()[0]);
  } else {
    foreach (const MyMoneySplit* s, matchingSplits) {
      m_matchingSplits.append(*s);
    }
  }
  // all filters passed, I guess we have a match
  // qDebug("  C: %d", m_matchingSplits.count());
  return matchingSplits.count() != 0;
}

int MyMoneyTransactionFilter::splitState(const MyMoneySplit& split) const
{
  int rc = notReconciled;

  switch (split.reconcileFlag()) {
    default:
    case MyMoneySplit::NotReconciled:
      break;;

    case MyMoneySplit::Cleared:
      rc = cleared;
      break;

    case MyMoneySplit::Reconciled:
      rc = reconciled;
      break;

    case MyMoneySplit::Frozen:
      rc = frozen;
      break;
  }
  return rc;
}

int MyMoneyTransactionFilter::splitType(const MyMoneyTransaction& t, const MyMoneySplit& split) const
{
  MyMoneyFile* file = MyMoneyFile::instance();
  MyMoneyAccount a, b;
  a = file->account(split.accountId());
  if ((a.accountGroup() == MyMoneyAccount::Income
       || a.accountGroup() == MyMoneyAccount::Expense))
    return allTypes;

  if (t.splitCount() == 2) {
    QString ida, idb;
    if (t.splits().size() > 0)
      ida = t.splits()[0].accountId();
    if (t.splits().size() > 1)
      idb = t.splits()[1].accountId();

    a = file->account(ida);
    b = file->account(idb);
    if ((a.accountGroup() != MyMoneyAccount::Expense
         && a.accountGroup() != MyMoneyAccount::Income)
        && (b.accountGroup() != MyMoneyAccount::Expense
            && b.accountGroup() != MyMoneyAccount::Income))
      return transfers;
  }

  if (split.value().isPositive())
    return deposits;

  return payments;
}

MyMoneyTransactionFilter::validityOptionE MyMoneyTransactionFilter::validTransaction(const MyMoneyTransaction& t) const
{
  QList<MyMoneySplit>::ConstIterator it_s;
  MyMoneyMoney val;

  for (it_s = t.splits().begin(); it_s != t.splits().end(); ++it_s) {
    val += (*it_s).value();
  }
  return (val == MyMoneyMoney()) ? valid : invalid;
}

bool MyMoneyTransactionFilter::includesCategory(const QString& cat) const
{
  return (! m_filterSet.singleFilter.categoryFilter) || m_categories.end() != m_categories.find(cat);
}

bool MyMoneyTransactionFilter::includesAccount(const QString& acc) const
{
  return (! m_filterSet.singleFilter.accountFilter) || m_accounts.end() != m_accounts.find(acc);
}

bool MyMoneyTransactionFilter::includesPayee(const QString& pye) const
{
  return (! m_filterSet.singleFilter.payeeFilter) || m_payees.end() != m_payees.find(pye);
}

bool MyMoneyTransactionFilter::includesTag(const QString& tag) const
{
  return (! m_filterSet.singleFilter.tagFilter) || m_tags.end() != m_tags.find(tag);
}

bool MyMoneyTransactionFilter::dateFilter(QDate& from, QDate& to) const
{
  from = m_fromDate;
  to = m_toDate;
  return m_filterSet.singleFilter.dateFilter == 1;
}

bool MyMoneyTransactionFilter::amountFilter(MyMoneyMoney& from, MyMoneyMoney& to) const
{
  from = m_fromAmount;
  to = m_toAmount;
  return m_filterSet.singleFilter.amountFilter == 1;
}

bool MyMoneyTransactionFilter::numberFilter(QString& from, QString& to) const
{
  from = m_fromNr;
  to = m_toNr;
  return m_filterSet.singleFilter.nrFilter == 1;
}

bool MyMoneyTransactionFilter::payees(QStringList& list) const
{
  bool result = m_filterSet.singleFilter.payeeFilter;

  if (result) {
    QHashIterator<QString, QString> it_payee(m_payees);
    while (it_payee.hasNext()) {
      it_payee.next();
      list += it_payee.key();
    }
  }
  return result;
}

bool MyMoneyTransactionFilter::tags(QStringList& list) const
{
  bool result = m_filterSet.singleFilter.tagFilter;

  if (result) {
    QHashIterator<QString, QString> it_tag(m_tags);
    while (it_tag.hasNext()) {
      it_tag.next();
      list += it_tag.key();
    }
  }
  return result;
}

bool MyMoneyTransactionFilter::accounts(QStringList& list) const
{
  bool result = m_filterSet.singleFilter.accountFilter;

  if (result) {
    QHashIterator<QString, QString> it_account(m_accounts);
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
  bool result = m_filterSet.singleFilter.categoryFilter;

  if (result) {
    QHashIterator<QString, QString> it_category(m_categories);
    while (it_category.hasNext()) {
      it_category.next();
      list += it_category.key();
    }
  }
  return result;
}

bool MyMoneyTransactionFilter::types(QList<int>& list) const
{
  bool result = m_filterSet.singleFilter.typeFilter;

  if (result) {
    QHashIterator<int, QString> it_type(m_types);
    while (it_type.hasNext()) {
      it_type.next();
      list += it_type.key();
    }
  }
  return result;
}

bool MyMoneyTransactionFilter::states(QList<int>& list) const
{
  bool result = m_filterSet.singleFilter.stateFilter;

  if (result) {
    QHashIterator<int, QString> it_state(m_states);
    while (it_state.hasNext()) {
      it_state.next();
      list += it_state.key();
    }
  }
  return result;
}

bool MyMoneyTransactionFilter::firstType(int&i) const
{
  bool result = m_filterSet.singleFilter.typeFilter;

  if (result) {
    QHashIterator<int, QString> it_type(m_types);
    if (it_type.hasNext()) {
      it_type.next();
      i = it_type.key();
    }
  }
  return result;
}

bool MyMoneyTransactionFilter::firstState(int&i) const
{
  bool result = m_filterSet.singleFilter.stateFilter;

  if (result) {
    QHashIterator<int, QString> it_state(m_states);
    if (it_state.hasNext()) {
      it_state.next();
      i = it_state.key();
    }
  }
  return result;
}

bool MyMoneyTransactionFilter::textFilter(QRegExp& exp) const
{
  exp = m_text;
  return m_filterSet.singleFilter.textFilter == 1;
}

void MyMoneyTransactionFilter::setDateFilter(dateOptionE range)
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

bool MyMoneyTransactionFilter::translateDateRange(dateOptionE id, QDate& start, QDate& end)
{
  bool rc = true;
  int yr = QDate::currentDate().year();
  int mon = QDate::currentDate().month();

  switch (id) {
    case MyMoneyTransactionFilter::allDates:
      start = QDate();
      end = QDate();
      break;
    case MyMoneyTransactionFilter::asOfToday:
      start = QDate();
      end =  QDate::currentDate();
      break;
    case MyMoneyTransactionFilter::currentMonth:
      start = QDate(yr, mon, 1);
      end = QDate(yr, mon, 1).addMonths(1).addDays(-1);
      break;
    case MyMoneyTransactionFilter::currentYear:
      start = QDate(yr, 1, 1);
      end = QDate(yr, 12, 31);
      break;
    case MyMoneyTransactionFilter::monthToDate:
      start = QDate(yr, mon, 1);
      end = QDate::currentDate();
      break;
    case MyMoneyTransactionFilter::yearToDate:
      start = QDate(yr, 1, 1);
      end = QDate::currentDate();
      break;
    case MyMoneyTransactionFilter::yearToMonth:
      start = QDate(yr, 1, 1);
      end = QDate(yr, mon, 1).addDays(-1);
      break;
    case MyMoneyTransactionFilter::lastMonth:
      start = QDate(yr, mon, 1).addMonths(-1);
      end = QDate(yr, mon, 1).addDays(-1);
      break;
    case MyMoneyTransactionFilter::lastYear:
      start = QDate(yr, 1, 1).addYears(-1);
      end = QDate(yr, 12, 31).addYears(-1);
      break;
    case MyMoneyTransactionFilter::last7Days:
      start = QDate::currentDate().addDays(-7);
      end = QDate::currentDate();
      break;
    case MyMoneyTransactionFilter::last30Days:
      start = QDate::currentDate().addDays(-30);
      end = QDate::currentDate();
      break;
    case MyMoneyTransactionFilter::last3Months:
      start = QDate::currentDate().addMonths(-3);
      end = QDate::currentDate();
      break;
    case MyMoneyTransactionFilter::last6Months:
      start = QDate::currentDate().addMonths(-6);
      end = QDate::currentDate();
      break;
    case MyMoneyTransactionFilter::last11Months:
      start = QDate(yr, mon, 1).addMonths(-12);
      end = QDate(yr, mon, 1).addDays(-1);
      break;
    case MyMoneyTransactionFilter::last12Months:
      start = QDate::currentDate().addMonths(-12);
      end = QDate::currentDate();
      break;
    case MyMoneyTransactionFilter::next7Days:
      start = QDate::currentDate();
      end = QDate::currentDate().addDays(7);
      break;
    case MyMoneyTransactionFilter::next30Days:
      start = QDate::currentDate();
      end = QDate::currentDate().addDays(30);
      break;
    case MyMoneyTransactionFilter::next3Months:
      start = QDate::currentDate();
      end = QDate::currentDate().addMonths(3);
      break;
    case MyMoneyTransactionFilter::next6Months:
      start = QDate::currentDate();
      end = QDate::currentDate().addMonths(6);
      break;
    case MyMoneyTransactionFilter::next12Months:
      start = QDate::currentDate();
      end = QDate::currentDate().addMonths(12);
      break;
    case MyMoneyTransactionFilter::next18Months:
      start = QDate::currentDate();
      end = QDate::currentDate().addMonths(18);
      break;
    case MyMoneyTransactionFilter::userDefined:
      start = QDate();
      end = QDate();
      break;
    case MyMoneyTransactionFilter::last3ToNext3Months:
      start = QDate::currentDate().addMonths(-3);
      end = QDate::currentDate().addMonths(3);
      break;
    case MyMoneyTransactionFilter::currentQuarter:
      start = QDate(yr, mon - ((mon - 1) % 3), 1);
      end = start.addMonths(3).addDays(-1);
      break;
    case MyMoneyTransactionFilter::lastQuarter:
      start = QDate(yr, mon - ((mon - 1) % 3), 1).addMonths(-3);
      end = start.addMonths(3).addDays(-1);
      break;
    case MyMoneyTransactionFilter::nextQuarter:
      start = QDate(yr, mon - ((mon - 1) % 3), 1).addMonths(3);
      end = start.addMonths(3).addDays(-1);
      break;
    case MyMoneyTransactionFilter::currentFiscalYear:
      start = QDate(QDate::currentDate().year(), fiscalYearStartMonth, fiscalYearStartDay);
      if (QDate::currentDate() < start)
        start = start.addYears(-1);
      end = start.addYears(1).addDays(-1);
      break;
    case MyMoneyTransactionFilter::lastFiscalYear:
      start = QDate(QDate::currentDate().year(), fiscalYearStartMonth, fiscalYearStartDay);
      if (QDate::currentDate() < start)
        start = start.addYears(-1);
      start = start.addYears(-1);
      end = start.addYears(1).addDays(-1);
      break;
    case MyMoneyTransactionFilter::today:
      start = QDate::currentDate();
      end =  QDate::currentDate();
      break;
    default:
      qWarning("Unknown date identifier %d in MyMoneyTransactionFilter::translateDateRange()", id);
      rc = false;
      break;
  }
  return rc;
}

void MyMoneyTransactionFilter::removeReference(const QString& id)
{
  if (m_accounts.end() != m_accounts.find(id)) {
    qDebug("%s", qPrintable(QString("Remove account '%1' from report").arg(id)));
    m_accounts.take(id);
  } else if (m_categories.end() != m_categories.find(id)) {
    qDebug("%s", qPrintable(QString("Remove category '%1' from report").arg(id)));
    m_categories.remove(id);
  } else if (m_payees.end() != m_payees.find(id)) {
    qDebug("%s", qPrintable(QString("Remove payee '%1' from report").arg(id)));
    m_payees.remove(id);
  } else if (m_tags.end() != m_tags.find(id)) {
    qDebug("%s", qPrintable(QString("Remove tag '%1' from report").arg(id)));
    m_tags.remove(id);
  }
}
