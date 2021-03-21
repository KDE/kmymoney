/*
    SPDX-FileCopyrightText: 2006 Ace Jones <acejones@users.sourceforge.net>
    SPDX-FileCopyrightText: 2006 Darren Gould <darren_gould@gmx.de>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "mymoneybudget.h"
#include "mymoneybudget_p.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QMap>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class MyMoneyBudget::PeriodGroupPrivate
{
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

class MyMoneyBudget::AccountGroupPrivate {

public:

    AccountGroupPrivate() :
        m_budgetlevel(eMyMoney::Budget::Level::None),
        m_budgetsubaccounts(false)
    {
    }

    QString                                   m_id;
    eMyMoney::Budget::Level                   m_budgetlevel;
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
    return (!d->m_budgetsubaccounts && d->m_budgetlevel == eMyMoney::Budget::Level::Monthly && balance().isZero());
}

void MyMoneyBudget::AccountGroup::convertToMonthly()
{
    MyMoneyBudget::PeriodGroup period;

    Q_D(AccountGroup);
    switch (d->m_budgetlevel) {
    case eMyMoney::Budget::Level::Yearly:
    case eMyMoney::Budget::Level::MonthByMonth:
        period = d->m_periods.first();         // make him monthly
        period.setAmount(balance() / MyMoneyMoney(12, 1));
        clearPeriods();
        addPeriod(period.startDate(), period);
        break;
    default:
        break;
    }
    d->m_budgetlevel = eMyMoney::Budget::Level::Monthly;
}

void MyMoneyBudget::AccountGroup::convertToYearly()
{
    MyMoneyBudget::PeriodGroup period;

    Q_D(AccountGroup);
    switch (d->m_budgetlevel) {
    case eMyMoney::Budget::Level::MonthByMonth:
    case eMyMoney::Budget::Level::Monthly:
        period = d->m_periods.first();         // make him monthly
        period.setAmount(totalBalance());
        clearPeriods();
        addPeriod(period.startDate(), period);
        break;
    default:
        break;
    }
    d->m_budgetlevel = eMyMoney::Budget::Level::Yearly;
}

void MyMoneyBudget::AccountGroup::convertToMonthByMonth()
{
    MyMoneyBudget::PeriodGroup period;
    QDate date;

    Q_D(AccountGroup);
    switch (d->m_budgetlevel) {
    case eMyMoney::Budget::Level::Yearly:
    case eMyMoney::Budget::Level::Monthly:
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
    d->m_budgetlevel = eMyMoney::Budget::Level::MonthByMonth;
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

eMyMoney::Budget::Level MyMoneyBudget::AccountGroup::budgetLevel() const
{
    Q_D(const AccountGroup);
    return d->m_budgetlevel;
}

void MyMoneyBudget::AccountGroup::setBudgetLevel(eMyMoney::Budget::Level level)
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
    case eMyMoney::Budget::Level::Monthly:
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
    if (d2->m_budgetlevel == eMyMoney::Budget::Level::None)
        return *this;

    MyMoneyBudget::AccountGroup r(right);
    auto d3 = static_cast<const AccountGroupPrivate *>(r.d_func());

    // make both operands based on the same budget level
    if (d->m_budgetlevel != d3->m_budgetlevel) {
        if (d->m_budgetlevel == eMyMoney::Budget::Level::Monthly) {        // my budget is monthly
            if (d3->m_budgetlevel == eMyMoney::Budget::Level::Yearly) {     // his is yearly
                r.convertToMonthly();
            } else if (d3->m_budgetlevel == eMyMoney::Budget::Level::MonthByMonth) { // his is month by month
                convertToMonthByMonth();
            }
        } else if (d->m_budgetlevel == eMyMoney::Budget::Level::Yearly) {  // my budget is yearly
            if (d3->m_budgetlevel == eMyMoney::Budget::Level::Monthly) {    // his is monthly
                r.convertToYearly();
            } else if (d3->m_budgetlevel == eMyMoney::Budget::Level::MonthByMonth) { // his is month by month
                convertToMonthByMonth();
            }
        } else if (d->m_budgetlevel == eMyMoney::Budget::Level::MonthByMonth) {  // my budget is month by month
            r.convertToMonthByMonth();
        }
    }

    QMap<QDate, MyMoneyBudget::PeriodGroup> rPeriods = d3->m_periods;
    QMap<QDate, MyMoneyBudget::PeriodGroup>::const_iterator it_pr;

    // in case the left side is empty, we add empty periods
    // so that both budgets are identical
    if (d->m_budgetlevel == eMyMoney::Budget::Level::None) {
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
    return (d->m_id == d2->m_id //
            && d->m_budgetlevel == d2->m_budgetlevel //
            && d->m_budgetsubaccounts == d2->m_budgetsubaccounts //
            && d->m_periods.keys() == d2->m_periods.keys() //
            && d->m_periods.values() == d2->m_periods.values());
}

MyMoneyBudget::MyMoneyBudget() :
    MyMoneyObject(*new MyMoneyBudgetPrivate)
{
    Q_D(MyMoneyBudget);
    d->m_name = QLatin1Literal("Unconfigured Budget");
}

MyMoneyBudget::MyMoneyBudget(const QString &id) :
    MyMoneyObject(*new MyMoneyBudgetPrivate, id)
{
    Q_D(MyMoneyBudget);
    d->m_name = QLatin1Literal("Unconfigured Budget");
}

MyMoneyBudget::MyMoneyBudget(const QString& id, const MyMoneyBudget& other) :
    MyMoneyObject(*new MyMoneyBudgetPrivate(*other.d_func()), id)
{
}

MyMoneyBudget::MyMoneyBudget(const MyMoneyBudget& other) :
    MyMoneyObject(*new MyMoneyBudgetPrivate(*other.d_func()), other.id())
{
}

MyMoneyBudget::~MyMoneyBudget()
{
}

bool MyMoneyBudget::operator == (const MyMoneyBudget& right) const
{
    Q_D(const MyMoneyBudget);
    auto d2 = static_cast<const MyMoneyBudgetPrivate *>(right.d_func());
    // clang-format off
    return (MyMoneyObject::operator==(right)
            && (d->m_accounts.count() == d2->m_accounts.count())
            && (d->m_accounts.keys() == d2->m_accounts.keys())
            && (d->m_accounts.values() == d2->m_accounts.values())
            && (d->m_name == d2->m_name)
            && (d->m_start == d2->m_start));
    // clang-format on
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

QMap<QString, MyMoneyBudget::AccountGroup> MyMoneyBudget::accountsMap() const
{
    Q_D(const MyMoneyBudget);
    return d->m_accounts;
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
