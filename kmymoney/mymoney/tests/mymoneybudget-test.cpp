/*
    SPDX-FileCopyrightText: 2010-2011 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "mymoneybudget-test.h"

#include <QtTest>

#define KMM_MYMONEY_UNIT_TESTABLE friend class MyMoneyBudgetTest;

#include "mymoneymoney.h"
#include "mymoneybudget.h"
#include "mymoneybudget_p.h"

QTEST_GUILESS_MAIN(MyMoneyBudgetTest)

void MyMoneyBudgetTest::init()
{
}

void MyMoneyBudgetTest::cleanup()
{
}

void MyMoneyBudgetTest::addMonthlyToMonthly()
{
  MyMoneyBudget::AccountGroup a0, a1;
  a0.setBudgetLevel(eMyMoney::Budget::Level::Monthly);
  a1.setBudgetLevel(eMyMoney::Budget::Level::Monthly);
  MyMoneyBudget::PeriodGroup period;
  period.setStartDate(QDate(2010, 1, 1));
  period.setAmount(MyMoneyMoney(100, 1));
  a0.addPeriod(QDate(2010, 1, 1), period);
  a1.addPeriod(QDate(2010, 1, 1), period);

  a0 += a1;

  QVERIFY(a0.budgetLevel() == eMyMoney::Budget::Level::Monthly);
  QVERIFY(a1.budgetLevel() == eMyMoney::Budget::Level::Monthly);
  QVERIFY(a0.getPeriods().count() == 1);
  QVERIFY(a1.getPeriods().count() == 1);
  QVERIFY(a0.balance() == MyMoneyMoney(200, 1));
  QVERIFY(a1.balance() == MyMoneyMoney(100, 1));
}

void MyMoneyBudgetTest::addMonthlyToYearly()
{
  MyMoneyBudget::AccountGroup a0, a1;
  a0.setBudgetLevel(eMyMoney::Budget::Level::Yearly);
  a1.setBudgetLevel(eMyMoney::Budget::Level::Monthly);
  MyMoneyBudget::PeriodGroup period;
  period.setStartDate(QDate(2010, 1, 1));
  period.setAmount(MyMoneyMoney(100, 1));
  a0.addPeriod(QDate(2010, 1, 1), period);
  a1.addPeriod(QDate(2010, 1, 1), period);

  QVERIFY(a0.totalBalance() == MyMoneyMoney(100, 1));
  QVERIFY(a1.totalBalance() == MyMoneyMoney(1200, 1));

  a0 += a1;

  QVERIFY(a0.budgetLevel() == eMyMoney::Budget::Level::Yearly);
  QVERIFY(a1.budgetLevel() == eMyMoney::Budget::Level::Monthly);
  QVERIFY(a0.getPeriods().count() == 1);
  QVERIFY(a1.getPeriods().count() == 1);
  QVERIFY(a0.totalBalance() == MyMoneyMoney(1300, 1));
  QVERIFY(a1.totalBalance() == MyMoneyMoney(1200, 1));
}

void MyMoneyBudgetTest::addMonthlyToMonthByMonth()
{
  MyMoneyBudget::AccountGroup a0, a1;
  a0.setBudgetLevel(eMyMoney::Budget::Level::MonthByMonth);
  a1.setBudgetLevel(eMyMoney::Budget::Level::Monthly);
  MyMoneyBudget::PeriodGroup period;
  period.setStartDate(QDate(2010, 1, 1));
  period.setAmount(MyMoneyMoney(100, 1));
  a1.addPeriod(QDate(2010, 1, 1), period);
  QDate date(2010, 1, 1);
  for (int i = 0; i < 12; ++i) {
    period.setAmount(MyMoneyMoney((i + 1) * 100, 1));
    a0.addPeriod(date, period);
    date = date.addMonths(1);
    period.setStartDate(date);
  }
  QVERIFY(a0.getPeriods().count() == 12);
  QVERIFY(a0.totalBalance() == MyMoneyMoney(7800, 1));
  QVERIFY(a1.totalBalance() == MyMoneyMoney(1200, 1));

  a0 += a1;

  QVERIFY(a0.budgetLevel() == eMyMoney::Budget::Level::MonthByMonth);
  QVERIFY(a1.budgetLevel() == eMyMoney::Budget::Level::Monthly);
  QVERIFY(a0.getPeriods().count() == 12);
  QVERIFY(a1.getPeriods().count() == 1);
  QVERIFY(a0.totalBalance() == MyMoneyMoney(9000, 1));
  QVERIFY(a1.totalBalance() == MyMoneyMoney(1200, 1));
}

void MyMoneyBudgetTest::addYearlyToMonthly()
{
  MyMoneyBudget::AccountGroup a0, a1;
  a0.setBudgetLevel(eMyMoney::Budget::Level::Yearly);
  a1.setBudgetLevel(eMyMoney::Budget::Level::Monthly);
  MyMoneyBudget::PeriodGroup period;
  period.setStartDate(QDate(2010, 1, 1));
  period.setAmount(MyMoneyMoney(100, 1));
  a0.addPeriod(QDate(2010, 1, 1), period);
  a1.addPeriod(QDate(2010, 1, 1), period);

  QVERIFY(a0.totalBalance() == MyMoneyMoney(100, 1));
  QVERIFY(a1.totalBalance() == MyMoneyMoney(1200, 1));

  a1 += a0;

  QVERIFY(a0.budgetLevel() == eMyMoney::Budget::Level::Yearly);
  QVERIFY(a1.budgetLevel() == eMyMoney::Budget::Level::Monthly);
  QVERIFY(a0.getPeriods().count() == 1);
  QVERIFY(a1.getPeriods().count() == 1);
  QVERIFY(a0.totalBalance() == MyMoneyMoney(100, 1));
  QVERIFY(a1.totalBalance() == MyMoneyMoney(1300, 1));
}

void MyMoneyBudgetTest::addYearlyToYearly()
{
  MyMoneyBudget::AccountGroup a0, a1;
  a0.setBudgetLevel(eMyMoney::Budget::Level::Yearly);
  a1.setBudgetLevel(eMyMoney::Budget::Level::Yearly);
  MyMoneyBudget::PeriodGroup period;
  period.setStartDate(QDate(2010, 1, 1));
  period.setAmount(MyMoneyMoney(100, 1));
  a0.addPeriod(QDate(2010, 1, 1), period);
  a1.addPeriod(QDate(2010, 1, 1), period);

  a0 += a1;

  QVERIFY(a0.budgetLevel() == eMyMoney::Budget::Level::Yearly);
  QVERIFY(a1.budgetLevel() == eMyMoney::Budget::Level::Yearly);
  QVERIFY(a0.getPeriods().count() == 1);
  QVERIFY(a1.getPeriods().count() == 1);
  QVERIFY(a0.balance() == MyMoneyMoney(200, 1));
  QVERIFY(a1.balance() == MyMoneyMoney(100, 1));
}

void MyMoneyBudgetTest::addYearlyToMonthByMonth()
{
  MyMoneyBudget::AccountGroup a0, a1;
  a0.setBudgetLevel(eMyMoney::Budget::Level::MonthByMonth);
  a1.setBudgetLevel(eMyMoney::Budget::Level::Yearly);
  MyMoneyBudget::PeriodGroup period;
  period.setStartDate(QDate(2010, 1, 1));
  period.setAmount(MyMoneyMoney(100, 1));
  a1.addPeriod(QDate(2010, 1, 1), period);
  QDate date(2010, 1, 1);
  for (int i = 0; i < 12; ++i) {
    period.setAmount(MyMoneyMoney((i + 1) * 100, 1));
    a0.addPeriod(date, period);
    date = date.addMonths(1);
    period.setStartDate(date);
  }
  QVERIFY(a0.getPeriods().count() == 12);
  QVERIFY(a0.totalBalance() == MyMoneyMoney(7800, 1));
  QVERIFY(a1.totalBalance() == MyMoneyMoney(100, 1));

  a0 += a1;

  QVERIFY(a0.budgetLevel() == eMyMoney::Budget::Level::MonthByMonth);
  QVERIFY(a1.budgetLevel() == eMyMoney::Budget::Level::Yearly);
  QVERIFY(a0.getPeriods().count() == 12);
  QVERIFY(a1.getPeriods().count() == 1);
  QVERIFY(a0.totalBalance() == MyMoneyMoney(7900, 1));
  QVERIFY(a1.totalBalance() == MyMoneyMoney(100, 1));
}

void MyMoneyBudgetTest::addMonthByMonthToMonthly()
{
  MyMoneyBudget::AccountGroup a0, a1;
  a0.setBudgetLevel(eMyMoney::Budget::Level::MonthByMonth);
  a1.setBudgetLevel(eMyMoney::Budget::Level::Monthly);
  MyMoneyBudget::PeriodGroup period;
  period.setStartDate(QDate(2010, 1, 1));
  period.setAmount(MyMoneyMoney(100, 1));
  a1.addPeriod(QDate(2010, 1, 1), period);
  QDate date(2010, 1, 1);
  for (int i = 0; i < 12; ++i) {
    period.setAmount(MyMoneyMoney((i + 1) * 100, 1));
    a0.addPeriod(date, period);
    date = date.addMonths(1);
    period.setStartDate(date);
  }
  QVERIFY(a0.getPeriods().count() == 12);
  QVERIFY(a0.totalBalance() == MyMoneyMoney(7800, 1));
  QVERIFY(a1.totalBalance() == MyMoneyMoney(1200, 1));

  a1 += a0;

  QVERIFY(a0.budgetLevel() == eMyMoney::Budget::Level::MonthByMonth);
  QVERIFY(a1.budgetLevel() == eMyMoney::Budget::Level::MonthByMonth);
  QVERIFY(a0.getPeriods().count() == 12);
  QVERIFY(a1.getPeriods().count() == 12);
  QVERIFY(a0.totalBalance() == MyMoneyMoney(7800, 1));
  QVERIFY(a1.totalBalance() == MyMoneyMoney(9000, 1));
}

void MyMoneyBudgetTest::addMonthByMonthToYearly()
{
  MyMoneyBudget::AccountGroup a0, a1;
  a0.setBudgetLevel(eMyMoney::Budget::Level::MonthByMonth);
  a1.setBudgetLevel(eMyMoney::Budget::Level::Yearly);
  MyMoneyBudget::PeriodGroup period;
  period.setStartDate(QDate(2010, 1, 1));
  period.setAmount(MyMoneyMoney(100, 1));
  a1.addPeriod(QDate(2010, 1, 1), period);
  QDate date(2010, 1, 1);
  for (int i = 0; i < 12; ++i) {
    period.setAmount(MyMoneyMoney((i + 1) * 100, 1));
    a0.addPeriod(date, period);
    date = date.addMonths(1);
    period.setStartDate(date);
  }
  QVERIFY(a0.getPeriods().count() == 12);
  QVERIFY(a0.totalBalance() == MyMoneyMoney(7800, 1));
  QVERIFY(a1.totalBalance() == MyMoneyMoney(100, 1));

  a1 += a0;

  QVERIFY(a0.budgetLevel() == eMyMoney::Budget::Level::MonthByMonth);
  QVERIFY(a1.budgetLevel() == eMyMoney::Budget::Level::MonthByMonth);
  QVERIFY(a0.getPeriods().count() == 12);
  QVERIFY(a1.getPeriods().count() == 12);
  QVERIFY(a0.totalBalance() == MyMoneyMoney(7800, 1));
  QVERIFY(a1.totalBalance() == MyMoneyMoney(7900, 1));
}

void MyMoneyBudgetTest::addMonthByMonthToMonthByMonth()
{
  MyMoneyBudget::AccountGroup a0, a1;
  a0.setBudgetLevel(eMyMoney::Budget::Level::MonthByMonth);
  a1.setBudgetLevel(eMyMoney::Budget::Level::MonthByMonth);
  MyMoneyBudget::PeriodGroup period;
  QDate date(2010, 1, 1);
  for (int i = 0; i < 12; ++i) {
    period.setStartDate(date);
    period.setAmount(MyMoneyMoney((i + 1) * 100, 1));
    a0.addPeriod(date, period);
    period.setAmount(MyMoneyMoney((i + 1) * 200, 1));
    a1.addPeriod(date, period);
    date = date.addMonths(1);
  }
  QVERIFY(a0.getPeriods().count() == 12);
  QVERIFY(a1.getPeriods().count() == 12);
  QVERIFY(a0.totalBalance() == MyMoneyMoney(7800, 1));
  QVERIFY(a1.totalBalance() == MyMoneyMoney(15600, 1));

  a0 += a1;

  QVERIFY(a0.budgetLevel() == eMyMoney::Budget::Level::MonthByMonth);
  QVERIFY(a1.budgetLevel() == eMyMoney::Budget::Level::MonthByMonth);
  QVERIFY(a0.getPeriods().count() == 12);
  QVERIFY(a1.getPeriods().count() == 12);
  QVERIFY(a0.totalBalance() == MyMoneyMoney(23400, 1));
  QVERIFY(a1.totalBalance() == MyMoneyMoney(15600, 1));
}
