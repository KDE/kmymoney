/***************************************************************************
                          mymoneybudget.cpp
                             -------------------
    begin                : Sun July 4 2004
    copyright            : (C) 2004-2005 by Ace Jones
    email                : acejones@users.sourceforge.net
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

#include "mymoneybudget.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QMap>
#include <QDate>
#include <QDomElement>
#include <QDomDocument>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneymoney.h"

class MyMoneyBudget::PeriodGroupPrivate {

public:
  QDate         m_start;
  MyMoneyMoney  m_amount;
};

MyMoneyBudget::PeriodGroup::PeriodGroup() :
  d_ptr(new PeriodGroupPrivate)
{
}

MyMoneyBudget::PeriodGroup::PeriodGroup(const MyMoneyBudget::PeriodGroup & other) :
  d_ptr(new PeriodGroupPrivate(*other.d_func()))
{
}

MyMoneyBudget::PeriodGroup::~PeriodGroup()
{
  Q_D(PeriodGroup);
  delete d;
}

QDate MyMoneyBudget::PeriodGroup::startDate() const
{
  Q_D(const PeriodGroup);
  return d->m_start;
}

void MyMoneyBudget::PeriodGroup::setStartDate(const QDate& start)
{
  Q_D(PeriodGroup);
  d->m_start  = start;
}

MyMoneyMoney MyMoneyBudget::PeriodGroup::amount() const
{
  Q_D(const PeriodGroup);
  return d->m_amount;
}

void MyMoneyBudget::PeriodGroup::setAmount(const MyMoneyMoney& amount)
{
  Q_D(PeriodGroup);
  d->m_amount = amount;
}

bool MyMoneyBudget::PeriodGroup::operator == (const PeriodGroup& right) const
{
  Q_D(const PeriodGroup);
  auto d2 = static_cast<const PeriodGroupPrivate *>(right.d_func());
  return (d->m_start == d2->m_start && d->m_amount == d2->m_amount);
}

const QStringList MyMoneyBudget::AccountGroup::kBudgetLevelText = QString("none,monthly,monthbymonth,yearly,invalid").split(',');
const int BUDGET_VERSION = 2;

class MyMoneyBudget::AccountGroupPrivate {

public:

  AccountGroupPrivate() :
    m_budgetlevel(AccountGroup::eBudgetLevel::eNone),
    m_budgetsubaccounts(false)
  {
  }

  QString                                   m_id;
  AccountGroup::eBudgetLevel                m_budgetlevel;
  bool                                      m_budgetsubaccounts;
  QMap<QDate, MyMoneyBudget::PeriodGroup>   m_periods;

};

MyMoneyBudget::AccountGroup::AccountGroup() :
  d_ptr(new AccountGroupPrivate)
{
}

MyMoneyBudget::AccountGroup::AccountGroup(const MyMoneyBudget::AccountGroup& other) :
  d_ptr(new AccountGroupPrivate(*other.d_func()))
{
}

MyMoneyBudget::AccountGroup::~AccountGroup()
{
  Q_D(AccountGroup);
  delete d;
}

bool MyMoneyBudget::AccountGroup::isZero() const
{
  Q_D(const AccountGroup);
  return (!d->m_budgetsubaccounts && d->m_budgetlevel == eMonthly && balance().isZero());
}

void MyMoneyBudget::AccountGroup::convertToMonthly()
{
  MyMoneyBudget::PeriodGroup period;

  Q_D(AccountGroup);
  switch (d->m_budgetlevel) {
    case eYearly:
    case eMonthByMonth:
      period = d->m_periods.first();         // make him monthly
      period.setAmount(balance() / MyMoneyMoney(12, 1));
      clearPeriods();
      addPeriod(period.startDate(), period);
      break;
    default:
      break;
  }
  d->m_budgetlevel = eMonthly;
}

void MyMoneyBudget::AccountGroup::convertToYearly()
{
  MyMoneyBudget::PeriodGroup period;

  Q_D(AccountGroup);
  switch (d->m_budgetlevel) {
    case eMonthByMonth:
    case eMonthly:
      period = d->m_periods.first();         // make him monthly
      period.setAmount(totalBalance());
      clearPeriods();
      addPeriod(period.startDate(), period);
      break;
    default:
      break;
  }
  d->m_budgetlevel = eYearly;
}

void MyMoneyBudget::AccountGroup::convertToMonthByMonth()
{
  MyMoneyBudget::PeriodGroup period;
  QDate date;

  Q_D(AccountGroup);
  switch (d->m_budgetlevel) {
    case eYearly:
    case eMonthly:
      period = d->m_periods.first();
      period.setAmount(totalBalance() / MyMoneyMoney(12, 1));
      clearPeriods();
      date = period.startDate();
      for (auto i = 0; i < 12; ++i) {
        addPeriod(date, period);
        date = date.addMonths(1);
        period.setStartDate(date);
      }
      break;
    default:
      break;
  }
  d->m_budgetlevel = eMonthByMonth;
}

QString MyMoneyBudget::AccountGroup::id() const
{
  Q_D(const AccountGroup);
  return d->m_id;
}

void MyMoneyBudget::AccountGroup::setId(const QString& id)
{
  Q_D(AccountGroup);
  d->m_id = id;
}

bool MyMoneyBudget::AccountGroup::budgetSubaccounts() const
{
  Q_D(const AccountGroup);
  return d->m_budgetsubaccounts;
}

void MyMoneyBudget::AccountGroup::setBudgetSubaccounts(bool budgetsubaccounts)
{
  Q_D(AccountGroup);
  d->m_budgetsubaccounts = budgetsubaccounts;
}

MyMoneyBudget::AccountGroup::eBudgetLevel MyMoneyBudget::AccountGroup::budgetLevel() const
{
  Q_D(const AccountGroup);
  return d->m_budgetlevel;
}

void MyMoneyBudget::AccountGroup::setBudgetLevel(eBudgetLevel level)
{
  Q_D(AccountGroup);
  d->m_budgetlevel = level;
}

MyMoneyBudget::PeriodGroup MyMoneyBudget::AccountGroup::period(const QDate& date) const
{
  Q_D(const AccountGroup);
  return d->m_periods[date];
}

void MyMoneyBudget::AccountGroup::addPeriod(const QDate& date, PeriodGroup& period)
{
  Q_D(AccountGroup);
  d->m_periods[date] = period;
}

const QMap<QDate, MyMoneyBudget::PeriodGroup> MyMoneyBudget::AccountGroup::getPeriods() const
{
  Q_D(const AccountGroup);
  return d->m_periods;
}

void MyMoneyBudget::AccountGroup::clearPeriods()
{
  Q_D(AccountGroup);
  d->m_periods.clear();
}

MyMoneyMoney MyMoneyBudget::AccountGroup::balance() const
{
  Q_D(const AccountGroup);
  MyMoneyMoney balance;

  foreach (const auto period, d->m_periods)
    balance += period.amount();
  return balance;
}

MyMoneyMoney MyMoneyBudget::AccountGroup::totalBalance() const
{
  Q_D(const AccountGroup);
  auto bal = balance();
  switch (d->m_budgetlevel) {
    default:
      break;
    case eMonthly:
      bal = bal * 12;
      break;
  }
  return bal;
}

MyMoneyBudget::AccountGroup MyMoneyBudget::AccountGroup::operator += (const MyMoneyBudget::AccountGroup& right)
{
  Q_D(AccountGroup);
  auto d2 = static_cast<const AccountGroupPrivate *>(right.d_func());
  // in case the right side is empty, we're done
  if (d2->m_budgetlevel == eNone)
    return *this;

  MyMoneyBudget::AccountGroup r(right);
  auto d3 = static_cast<const AccountGroupPrivate *>(r.d_func());

  // make both operands based on the same budget level
  if (d->m_budgetlevel != d3->m_budgetlevel) {
    if (d->m_budgetlevel == eMonthly) {        // my budget is monthly
      if (d3->m_budgetlevel == eYearly) {     // his is yearly
        r.convertToMonthly();
      } else if (d3->m_budgetlevel == eMonthByMonth) { // his is month by month
        convertToMonthByMonth();
      }
    } else if (d->m_budgetlevel == eYearly) {  // my budget is yearly
      if (d3->m_budgetlevel == eMonthly) {    // his is monthly
        r.convertToYearly();
      } else if (d3->m_budgetlevel == eMonthByMonth) { // his is month by month
        convertToMonthByMonth();
      }
    } else if (d->m_budgetlevel == eMonthByMonth) {  // my budget is month by month
      r.convertToMonthByMonth();
    }
  }

  QMap<QDate, MyMoneyBudget::PeriodGroup> rPeriods = d3->m_periods;
  QMap<QDate, MyMoneyBudget::PeriodGroup>::const_iterator it_pr;

  // in case the left side is empty, we add empty periods
  // so that both budgets are identical
  if (d->m_budgetlevel == eNone) {
    it_pr = rPeriods.constBegin();
    QDate date = (*it_pr).startDate();
    while (it_pr != rPeriods.constEnd()) {
      MyMoneyBudget::PeriodGroup period = *it_pr;
      period.setAmount(MyMoneyMoney());
      addPeriod(date, period);
      date = date.addMonths(1);
      ++it_pr;
    }
    d->m_budgetlevel = d3->m_budgetlevel;
  }

  QMap<QDate, MyMoneyBudget::PeriodGroup> periods = d->m_periods;
  QMap<QDate, MyMoneyBudget::PeriodGroup>::const_iterator it_p;

  // now both budgets should be of the same type and we simply need
  // to iterate over the period list and add the values
  d->m_periods.clear();
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

bool MyMoneyBudget::AccountGroup::operator == (const AccountGroup& right) const
{
  Q_D(const AccountGroup);
  auto d2 = static_cast<const AccountGroupPrivate *>(right.d_func());
  return (d->m_id == d2->m_id
          && d->m_budgetlevel == d2->m_budgetlevel
          && d->m_budgetsubaccounts == d2->m_budgetsubaccounts
          && d->m_periods.keys() == d2->m_periods.keys()
          && d->m_periods.values() == d2->m_periods.values());
}

class MyMoneyBudgetPrivate {

public:
  /**
    * The user-assigned name of the Budget
    */
  QString m_name;

  /**
    * The user-assigned year of the Budget
    */
  QDate m_start;

  /**
    * Map the budgeted accounts
    *
    * Each account Id is stored against the AccountGroup information
    */
  QMap<QString, MyMoneyBudget::AccountGroup> m_accounts;
};

MyMoneyBudget::MyMoneyBudget() :
  d_ptr(new MyMoneyBudgetPrivate)
{
  Q_D(MyMoneyBudget);
  d->m_name = QLatin1Literal("Unconfigured Budget");
}

MyMoneyBudget::MyMoneyBudget(const QString& name) :
  d_ptr(new MyMoneyBudgetPrivate)
{
  Q_D(MyMoneyBudget);
  d->m_name = name;
}

MyMoneyBudget::MyMoneyBudget(const QDomElement& node) :
    MyMoneyObject(node),
    d_ptr(new MyMoneyBudgetPrivate)
{
  if (!read(node))
    clearId();
}

MyMoneyBudget::MyMoneyBudget(const QString& id, const MyMoneyBudget& other) :
  MyMoneyObject(id),
  d_ptr(new MyMoneyBudgetPrivate(*other.d_func()))
{
}

MyMoneyBudget::MyMoneyBudget(const MyMoneyBudget& other) :
  MyMoneyObject(other.id()),
  d_ptr(new MyMoneyBudgetPrivate(*other.d_func()))
{
}

MyMoneyBudget::~MyMoneyBudget()
{
  Q_D(MyMoneyBudget);
  delete d;
}

bool MyMoneyBudget::operator == (const MyMoneyBudget& right) const
{
  Q_D(const MyMoneyBudget);
  auto d2 = static_cast<const MyMoneyBudgetPrivate *>(right.d_func());
  return (MyMoneyObject::operator==(right) &&
          (d->m_accounts.count() == d2->m_accounts.count()) &&
          (d->m_accounts.keys() == d2->m_accounts.keys()) &&
          (d->m_accounts.values() == d2->m_accounts.values()) &&
          (d->m_name == d2->m_name) &&
          (d->m_start == d2->m_start));
}

void MyMoneyBudget::write(QDomElement& e, QDomDocument *doc) const
{
  writeBaseXML(*doc, e);

  Q_D(const MyMoneyBudget);
  e.setAttribute(getAttrName(Attribute::Name),  d->m_name);
  e.setAttribute(getAttrName(Attribute::Start), d->m_start.toString(Qt::ISODate));
  e.setAttribute(getAttrName(Attribute::Version), BUDGET_VERSION);

  QMap<QString, AccountGroup>::const_iterator it;
  for (it = d->m_accounts.begin(); it != d->m_accounts.end(); ++it) {
    // only add the account if there is a budget entered
    // or it covers some sub accounts
    if (!(*it).balance().isZero() || (*it).budgetSubaccounts()) {
      QDomElement domAccount = doc->createElement(getElName(Element::Account));
      domAccount.setAttribute(getAttrName(Attribute::ID), it.key());
      domAccount.setAttribute(getAttrName(Attribute::BudgetLevel), AccountGroup::kBudgetLevelText[it.value().budgetLevel()]);
      domAccount.setAttribute(getAttrName(Attribute::BudgetSubAccounts), it.value().budgetSubaccounts());

      const QMap<QDate, PeriodGroup> periods = it.value().getPeriods();
      QMap<QDate, PeriodGroup>::const_iterator it_per;
      for (it_per = periods.begin(); it_per != periods.end(); ++it_per) {
        if (!(*it_per).amount().isZero()) {
          QDomElement domPeriod = doc->createElement(getElName(Element::Period));

          domPeriod.setAttribute(getAttrName(Attribute::Amount), (*it_per).amount().toString());
          domPeriod.setAttribute(getAttrName(Attribute::Start), (*it_per).startDate().toString(Qt::ISODate));
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

  Q_D(MyMoneyBudget);
  auto result = false;

  if (getElName(Element::Budget) == e.tagName()) {
    result = true;
    d->m_name  = e.attribute(getAttrName(Attribute::Name));
    d->m_start = QDate::fromString(e.attribute(getAttrName(Attribute::Start)), Qt::ISODate);
    m_id    = e.attribute(getAttrName(Attribute::ID));

    QDomNode child = e.firstChild();
    while (!child.isNull() && child.isElement()) {
      QDomElement c = child.toElement();

      AccountGroup account;

      if (getElName(Element::Account) == c.tagName()) {
        if (c.hasAttribute(getAttrName(Attribute::ID)))
          account.setId(c.attribute(getAttrName(Attribute::ID)));

        if (c.hasAttribute(getAttrName(Attribute::BudgetLevel))) {
          int i = AccountGroup::kBudgetLevelText.indexOf(c.attribute(getAttrName(Attribute::BudgetLevel)));
          if (i != -1)
            account.setBudgetLevel(static_cast<AccountGroup::eBudgetLevel>(i));
        }

        if (c.hasAttribute(getAttrName(Attribute::BudgetSubAccounts)))
          account.setBudgetSubaccounts(c.attribute(getAttrName(Attribute::BudgetSubAccounts)).toUInt());
      }

      QDomNode period = c.firstChild();
      while (!period.isNull() && period.isElement()) {
        QDomElement per = period.toElement();
        PeriodGroup pGroup;

        if (getElName(Element::Period) == per.tagName() && per.hasAttribute(getAttrName(Attribute::Amount)) && per.hasAttribute(getAttrName(Attribute::Start))) {
          pGroup.setAmount(MyMoneyMoney(per.attribute(getAttrName(Attribute::Amount))));
          pGroup.setStartDate(QDate::fromString(per.attribute(getAttrName(Attribute::Start)), Qt::ISODate));
          account.addPeriod(pGroup.startDate(), pGroup);
        }

        period = period.nextSibling();
      }

      d->m_accounts[account.id()] = account;

      child = child.nextSibling();
    }
  }

  return result;
}

void MyMoneyBudget::writeXML(QDomDocument& document, QDomElement& parent) const
{
  QDomElement el = document.createElement(getElName(Element::Budget));
  write(el, &document);
  parent.appendChild(el);
}

bool MyMoneyBudget::hasReferenceTo(const QString& id) const
{
  Q_D(const MyMoneyBudget);
  // return true if we have an assignment for this id
  return (d->m_accounts.contains(id));
}

void MyMoneyBudget::removeReference(const QString& id)
{
  Q_D(MyMoneyBudget);
  if (d->m_accounts.contains(id)) {
    d->m_accounts.remove(id);
  }
}

const MyMoneyBudget::AccountGroup& MyMoneyBudget::account(const QString& id) const
{
  static AccountGroup empty;
  QMap<QString, AccountGroup>::ConstIterator it;

  Q_D(const MyMoneyBudget);
  it = d->m_accounts.constFind(id);
  if (it != d->m_accounts.constEnd())
    return it.value();
  return empty;
}

void MyMoneyBudget::setAccount(const AccountGroup& account, const QString& id)
{
  Q_D(MyMoneyBudget);
  if (account.isZero()) {
    d->m_accounts.remove(id);
  } else {
    // make sure we store a correct id
    AccountGroup acc(account);
    if (acc.id() != id)
      acc.setId(id);
    d->m_accounts[id] = acc;
  }
}

bool MyMoneyBudget::contains(const QString &id) const
{
  Q_D(const MyMoneyBudget);
  return d->m_accounts.contains(id);
}

QList<MyMoneyBudget::AccountGroup> MyMoneyBudget::getaccounts() const
{
  Q_D(const MyMoneyBudget);
  return d->m_accounts.values();
}

QString MyMoneyBudget::name() const
{
  Q_D(const MyMoneyBudget);
  return d->m_name;
}

void MyMoneyBudget::setName(const QString& name)
{
  Q_D(MyMoneyBudget);
  d->m_name = name;
}

QDate MyMoneyBudget::budgetStart() const
{
  Q_D(const MyMoneyBudget);
  return d->m_start;
}

void MyMoneyBudget::setBudgetStart(const QDate& start)
{
  Q_D(MyMoneyBudget);
  auto oldDate = QDate(d->m_start.year(), d->m_start.month(), 1);
  d->m_start = QDate(start.year(), start.month(), 1);
  if (oldDate.isValid()) {
    int adjust = ((d->m_start.year() - oldDate.year()) * 12) + (d->m_start.month() - oldDate.month());
    QMap<QString, AccountGroup>::iterator it;
    for (it = d->m_accounts.begin(); it != d->m_accounts.end(); ++it) {
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

QString MyMoneyBudget::getElName(const Element el)
{
  static const QMap<Element, QString> elNames = {
    {Element::Budget,   "BUDGET"},
    {Element::Account,  "ACCOUNT"},
    {Element::Period,   "PERIOD"}
  };
  return elNames[el];
}

QString MyMoneyBudget::getAttrName(const Attribute attr)
{
  static const QHash<Attribute, QString> attrNames = {
    {Attribute::ID,                 QStringLiteral("id")},
    {Attribute::Name,               QStringLiteral("name")},
    {Attribute::Start,              QStringLiteral("start")},
    {Attribute::Version,            QStringLiteral("version")},
    {Attribute::BudgetLevel,        QStringLiteral("budgetlevel")},
    {Attribute::BudgetSubAccounts,  QStringLiteral("budgetsubaccounts")},
    {Attribute::Amount,             QStringLiteral("amount")}
  };
  return attrNames[attr];
}
