/***************************************************************************
                          mymoneybudget.cpp
                             -------------------
    begin                : Sun July 4 2004
    copyright            : (C) 2004-2005 by Ace Jones
    email                : acejones@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "mymoneybudget.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>
#include <QDomElement>
#include <QDomDocument>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

const QStringList MyMoneyBudget::AccountGroup::kBudgetLevelText = QString("none,monthly,monthbymonth,yearly,invalid").split(',');
const int BUDGET_VERSION = 2;

bool MyMoneyBudget::AccountGroup::isZero() const
{
  return (!m_budgetsubaccounts && m_budgetlevel == eMonthly && balance().isZero());
}

void MyMoneyBudget::AccountGroup::convertToMonthly()
{
  MyMoneyBudget::PeriodGroup period;

  switch (m_budgetlevel) {
    case eYearly:
    case eMonthByMonth:
      period = *(m_periods.begin());         // make him monthly
      period.setAmount(balance() / MyMoneyMoney(12, 1));
      clearPeriods();
      addPeriod(period.startDate(), period);
      break;
    default:
      break;
  }
  m_budgetlevel = eMonthly;
}

void MyMoneyBudget::AccountGroup::convertToYearly()
{
  MyMoneyBudget::PeriodGroup period;

  switch (m_budgetlevel) {
    case eMonthByMonth:
    case eMonthly:
      period = *(m_periods.begin());         // make him monthly
      period.setAmount(totalBalance());
      clearPeriods();
      addPeriod(period.startDate(), period);
      break;
    default:
      break;
  }
  m_budgetlevel = eYearly;
}

void MyMoneyBudget::AccountGroup::convertToMonthByMonth()
{
  MyMoneyBudget::PeriodGroup period;
  QDate date;

  switch (m_budgetlevel) {
    case eYearly:
    case eMonthly:
      period = *(m_periods.begin());
      period.setAmount(totalBalance() / MyMoneyMoney(12, 1));
      clearPeriods();
      date = period.startDate();
      for (int i = 0; i < 12; ++i) {
        addPeriod(date, period);
        date = date.addMonths(1);
        period.setStartDate(date);
      }
      break;
    default:
      break;
  }
  m_budgetlevel = eMonthByMonth;
}

MyMoneyBudget::AccountGroup MyMoneyBudget::AccountGroup::operator += (const MyMoneyBudget::AccountGroup & _r)
{
  // in case the right side is empty, we're done
  if (_r.m_budgetlevel == eNone)
    return *this;

  MyMoneyBudget::AccountGroup r(_r);

  // make both operands based on the same budget level
  if (m_budgetlevel != r.m_budgetlevel) {
    if (m_budgetlevel == eMonthly) {        // my budget is monthly
      if (r.m_budgetlevel == eYearly) {     // his is yearly
        r.convertToMonthly();
      } else if (r.m_budgetlevel == eMonthByMonth) { // his is month by month
        convertToMonthByMonth();
      }
    } else if (m_budgetlevel == eYearly) {  // my budget is yearly
      if (r.m_budgetlevel == eMonthly) {    // his is monthly
        r.convertToYearly();
      } else if (r.m_budgetlevel == eMonthByMonth) { // his is month by month
        convertToMonthByMonth();
      }
    } else if (m_budgetlevel == eMonthByMonth) {  // my budget is month by month
      r.convertToMonthByMonth();
    }
  }

  QMap<QDate, MyMoneyBudget::PeriodGroup> rPeriods = r.m_periods;
  QMap<QDate, MyMoneyBudget::PeriodGroup>::const_iterator it_pr;

  // in case the left side is empty, we add empty periods
  // so that both budgets are identical
  if (m_budgetlevel == eNone) {
    it_pr = rPeriods.constBegin();
    QDate date = (*it_pr).startDate();
    while (it_pr != rPeriods.constEnd()) {
      MyMoneyBudget::PeriodGroup period = *it_pr;
      period.setAmount(MyMoneyMoney());
      addPeriod(date, period);
      date = date.addMonths(1);
      ++it_pr;
    }
    m_budgetlevel = r.m_budgetlevel;
  }

  QMap<QDate, MyMoneyBudget::PeriodGroup> periods = m_periods;
  QMap<QDate, MyMoneyBudget::PeriodGroup>::const_iterator it_p;

  // now both budgets should be of the same type and we simply need
  // to iterate over the period list and add the values
  m_periods.clear();
  it_p = periods.constBegin();
  it_pr = rPeriods.constBegin();
  QDate date = (*it_p).startDate();
  while (it_p != periods.constEnd()) {
    MyMoneyBudget::PeriodGroup period = *it_p;
    if (it_pr != rPeriods.constEnd()) {
      period.setAmount(period.amount() + (*it_pr).amount());
      ++it_pr;
    }
    addPeriod(date, period);
    date = date.addMonths(1);
    ++it_p;
  }
  return *this;
}

bool MyMoneyBudget::AccountGroup::operator == (const AccountGroup &r) const
{
  return (m_id == r.m_id
          && m_budgetlevel == r.m_budgetlevel
          && m_budgetsubaccounts == r.m_budgetsubaccounts
          && m_periods.keys() == r.m_periods.keys()
          && m_periods.values() == r.m_periods.values());
}

MyMoneyBudget::MyMoneyBudget() :
    m_name("Unconfigured Budget")
{
}

MyMoneyBudget::MyMoneyBudget(const QString& _name) :
    m_name(_name)
{
}

MyMoneyBudget::MyMoneyBudget(const QDomElement& node) :
    MyMoneyObject(node)
{
  if (!read(node))
    clearId();
}

MyMoneyBudget::MyMoneyBudget(const QString& id, const MyMoneyBudget& budget)
{
  *this = budget;
  m_id = id;
}

MyMoneyBudget::~MyMoneyBudget()
{
}

bool MyMoneyBudget::operator == (const MyMoneyBudget& right) const
{
  return (MyMoneyObject::operator==(right) &&
          (m_accounts.count() == right.m_accounts.count()) &&
          (m_accounts.keys() == right.m_accounts.keys()) &&
          (m_accounts.values() == right.m_accounts.values()) &&
          (m_name == right.m_name) &&
          (m_start == right.m_start));
}

void MyMoneyBudget::write(QDomElement& e, QDomDocument *doc) const
{
  writeBaseXML(*doc, e);

  e.setAttribute("name",  m_name);
  e.setAttribute("start", m_start.toString(Qt::ISODate));
  e.setAttribute("version", BUDGET_VERSION);

  QMap<QString, AccountGroup>::const_iterator it;
  for (it = m_accounts.begin(); it != m_accounts.end(); ++it) {
    // only add the account if there is a budget entered
    // or it covers some sub accounts
    if (!(*it).balance().isZero() || (*it).budgetSubaccounts()) {
      QDomElement domAccount = doc->createElement("ACCOUNT");
      domAccount.setAttribute("id", it.key());
      domAccount.setAttribute("budgetlevel", AccountGroup::kBudgetLevelText[it.value().budgetLevel()]);
      domAccount.setAttribute("budgetsubaccounts", it.value().budgetSubaccounts());

      const QMap<QDate, PeriodGroup> periods = it.value().getPeriods();
      QMap<QDate, PeriodGroup>::const_iterator it_per;
      for (it_per = periods.begin(); it_per != periods.end(); ++it_per) {
        if (!(*it_per).amount().isZero()) {
          QDomElement domPeriod = doc->createElement("PERIOD");

          domPeriod.setAttribute("amount", (*it_per).amount().toString());
          domPeriod.setAttribute("start", (*it_per).startDate().toString(Qt::ISODate));
          domAccount.appendChild(domPeriod);
        }
      }

      e.appendChild(domAccount);
    }
  }
}

bool MyMoneyBudget::read(const QDomElement& e)
{
  // The goal of this reading method is 100% backward AND 100% forward
  // compatibility.  Any Budget ever created with any version of KMyMoney
  // should be able to be loaded by this method (as long as it's one of the
  // Budget types supported in this version, of course)

  bool result = false;

  if ("BUDGET" == e.tagName()) {
    result = true;
    m_name  = e.attribute("name");
    m_start = QDate::fromString(e.attribute("start"), Qt::ISODate);
    m_id    = e.attribute("id");

    QDomNode child = e.firstChild();
    while (!child.isNull() && child.isElement()) {
      QDomElement c = child.toElement();

      AccountGroup account;

      if ("ACCOUNT" == c.tagName()) {
        if (c.hasAttribute("id"))
          account.setId(c.attribute("id"));

        if (c.hasAttribute("budgetlevel")) {
          int i = AccountGroup::kBudgetLevelText.indexOf(c.attribute("budgetlevel"));
          if (i != -1)
            account.setBudgetLevel(static_cast<AccountGroup::eBudgetLevel>(i));
        }

        if (c.hasAttribute("budgetsubaccounts"))
          account.setBudgetSubaccounts(c.attribute("budgetsubaccounts").toUInt());
      }

      QDomNode period = c.firstChild();
      while (!period.isNull() && period.isElement()) {
        QDomElement per = period.toElement();
        PeriodGroup pGroup;

        if ("PERIOD" == per.tagName() && per.hasAttribute("amount") && per.hasAttribute("start")) {
          pGroup.setAmount(MyMoneyMoney(per.attribute("amount")));
          pGroup.setStartDate(QDate::fromString(per.attribute("start"), Qt::ISODate));
          account.addPeriod(pGroup.startDate(), pGroup);
        }

        period = period.nextSibling();
      }

      m_accounts[account.id()] = account;

      child = child.nextSibling();
    }
  }

  return result;
}

void MyMoneyBudget::writeXML(QDomDocument& document, QDomElement& parent) const
{
  QDomElement el = document.createElement("BUDGET");
  write(el, &document);
  parent.appendChild(el);
}

bool MyMoneyBudget::hasReferenceTo(const QString& id) const
{
  // return true if we have an assignment for this id
  return (m_accounts.contains(id));
}

void MyMoneyBudget::removeReference(const QString& id)
{
  if (m_accounts.contains(id)) {
    m_accounts.remove(id);
  }
}

void MyMoneyBudget::setAccount(const AccountGroup &_account, const QString &_id)
{
  if (_account.isZero()) {
    m_accounts.remove(_id);
  } else {
    // make sure we store a correct id
    AccountGroup account(_account);
    if (account.id() != _id)
      account.setId(_id);
    m_accounts[_id] = account;
  }
}

const MyMoneyBudget::AccountGroup& MyMoneyBudget::account(const QString &_id) const
{
  static AccountGroup empty;
  QMap<QString, AccountGroup>::ConstIterator it;

  it = m_accounts.constFind(_id);
  if (it != m_accounts.constEnd())
    return it.value();
  return empty;
}

void MyMoneyBudget::setBudgetStart(const QDate& _start)
{
  QDate oldDate = QDate(m_start.year(), m_start.month(), 1);
  m_start = QDate(_start.year(), _start.month(), 1);
  if (oldDate.isValid()) {
    int adjust = ((m_start.year() - oldDate.year()) * 12) + (m_start.month() - oldDate.month());
    QMap<QString, AccountGroup>::iterator it;
    for (it = m_accounts.begin(); it != m_accounts.end(); ++it) {
      const QMap<QDate, PeriodGroup> periods = (*it).getPeriods();
      QMap<QDate, PeriodGroup>::const_iterator it_per;
      (*it).clearPeriods();
      for (it_per = periods.begin(); it_per != periods.end(); ++it_per) {
        PeriodGroup pgroup = (*it_per);
        pgroup.setStartDate(pgroup.startDate().addMonths(adjust));
        (*it).addPeriod(pgroup.startDate(), pgroup);
      }
    }
  }
}
