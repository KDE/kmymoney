/***************************************************************************
                          pivotgridtest.cpp
                          -------------------
    copyright            : (C) 2002-2005 by Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
                           Ace Jones <ace.j@hotpop.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "pivotgrid-test.h"

#include <QtTest/QtTest>

#include "reportstestcommon.h"

#include "pivotgrid.h"
#include "mymoneyinstitution.h"
#include "mymoneysecurity.h"

using namespace reports;
using namespace test;

QTEST_GUILESS_MAIN(PivotGridTest)

void PivotGridTest::init()
{
  storage = new MyMoneySeqAccessMgr;
  file = MyMoneyFile::instance();
  file->attachStorage(storage);

  MyMoneyFileTransaction ft;
  file->addCurrency(MyMoneySecurity("CAD", "Canadian Dollar",        "C$"));
  file->addCurrency(MyMoneySecurity("USD", "US Dollar",              "$"));
  file->addCurrency(MyMoneySecurity("JPY", "Japanese Yen",           QChar(0x00A5), 1));
  file->addCurrency(MyMoneySecurity("GBP", "British Pound",           "#"));
  file->setBaseCurrency(file->currency("USD"));

  MyMoneyPayee payeeTest("Test Payee");
  file->addPayee(payeeTest);
  MyMoneyPayee payeeTest2("Thomas Baumgart");
  file->addPayee(payeeTest2);

  acAsset = (MyMoneyFile::instance()->asset().id());
  acLiability = (MyMoneyFile::instance()->liability().id());
  acExpense = (MyMoneyFile::instance()->expense().id());
  acIncome = (MyMoneyFile::instance()->income().id());
  acChecking = makeAccount(QString("Checking Account"), eMyMoney::Account::Checkings, moCheckingOpen, QDate(2004, 5, 15), acAsset);
  acCredit = makeAccount(QString("Credit Card"), eMyMoney::Account::CreditCard, moCreditOpen, QDate(2004, 7, 15), acLiability);
  acSolo = makeAccount(QString("Solo"), eMyMoney::Account::Expense, MyMoneyMoney(), QDate(2004, 1, 11), acExpense);
  acParent = makeAccount(QString("Parent"), eMyMoney::Account::Expense, MyMoneyMoney(), QDate(2004, 1, 11), acExpense);
  acChild = makeAccount(QString("Child"), eMyMoney::Account::Expense, MyMoneyMoney(), QDate(2004, 2, 11), acParent);
  acForeign = makeAccount(QString("Foreign"), eMyMoney::Account::Expense, MyMoneyMoney(), QDate(2004, 1, 11), acExpense);

  acSecondChild = makeAccount(QString("Second Child"), eMyMoney::Account::Expense, MyMoneyMoney(), QDate(2004, 2, 11), acParent);
  acGrandChild1 = makeAccount(QString("Grand Child 1"), eMyMoney::Account::Expense, MyMoneyMoney(), QDate(2004, 2, 11), acChild);
  acGrandChild2 = makeAccount(QString("Grand Child 2"), eMyMoney::Account::Expense, MyMoneyMoney(), QDate(2004, 2, 11), acChild);

  MyMoneyInstitution i("Bank of the World", "", "", "", "", "", "");
  file->addInstitution(i);
  inBank = i.id();
  ft.commit();
}

void PivotGridTest::cleanup()
{
  file->detachStorage(storage);
  delete storage;
}

void PivotGridTest::testCellAddValue()
{
  PivotCell a;
  QVERIFY(a == MyMoneyMoney());
  QVERIFY(a.m_stockSplit == MyMoneyMoney::ONE);
  QVERIFY(a.m_postSplit == MyMoneyMoney());
  QVERIFY(a.formatMoney("", 2) == MyMoneyMoney().formatMoney("", 2));

  PivotCell b(MyMoneyMoney(13, 10));
  QVERIFY(b == MyMoneyMoney(13, 10));
  QVERIFY(b.m_stockSplit == MyMoneyMoney::ONE);
  QVERIFY(b.m_postSplit == MyMoneyMoney());
  QVERIFY(b.formatMoney("", 2) == MyMoneyMoney(13, 10).formatMoney("", 2));

  PivotCell s(b);
  QVERIFY(s == MyMoneyMoney(13, 10));
  QVERIFY(s.m_stockSplit == MyMoneyMoney::ONE);
  QVERIFY(s.m_postSplit == MyMoneyMoney());
  QVERIFY(s.formatMoney("", 2) == MyMoneyMoney(13, 10).formatMoney("", 2));

  s = PivotCell::stockSplit(MyMoneyMoney(1, 2));
  QVERIFY(s == MyMoneyMoney());
  QVERIFY(s.m_stockSplit == MyMoneyMoney(1, 2));
  QVERIFY(s.m_postSplit == MyMoneyMoney());
  QVERIFY(s.formatMoney("", 2) == MyMoneyMoney().formatMoney("", 2));

  a += MyMoneyMoney::ONE;
  a += MyMoneyMoney(2, 1);
  QVERIFY(a == MyMoneyMoney(3, 1));
  QVERIFY(a.m_stockSplit == MyMoneyMoney::ONE);
  QVERIFY(a.m_postSplit == MyMoneyMoney());
  QVERIFY(a.formatMoney("", 2) == MyMoneyMoney(3, 1).formatMoney("", 2));

  a += s;
  QVERIFY(a == MyMoneyMoney(3, 1));
  QVERIFY(a.m_stockSplit == MyMoneyMoney(1, 2));
  QVERIFY(a.m_postSplit == MyMoneyMoney());
  QVERIFY(a.formatMoney("", 2) == MyMoneyMoney(15, 10).formatMoney("", 2));

  a += MyMoneyMoney(3, 1);
  a += MyMoneyMoney(3, 1);
  QVERIFY(a == MyMoneyMoney(3, 1));
  QVERIFY(a.m_stockSplit == MyMoneyMoney(1, 2));
  QVERIFY(a.m_postSplit == MyMoneyMoney(6, 1));
  QVERIFY(a.formatMoney("", 2) == MyMoneyMoney(75, 10).formatMoney("", 2));
}

void PivotGridTest::testCellAddCell()
{
  PivotCell a, b;

  a += MyMoneyMoney(3, 1);
  a += PivotCell::stockSplit(MyMoneyMoney(2, 1));
  a += MyMoneyMoney(4, 1);

  QVERIFY(a == MyMoneyMoney(3, 1));
  QVERIFY(a.m_stockSplit == MyMoneyMoney(2, 1));
  QVERIFY(a.m_postSplit == MyMoneyMoney(4, 1));
  QVERIFY(a.formatMoney("", 2) == MyMoneyMoney(10, 1).formatMoney("", 2));

  b += MyMoneyMoney(4, 1);
  b += PivotCell::stockSplit(MyMoneyMoney(4, 1));
  b += MyMoneyMoney(16, 1);

  QVERIFY(b == MyMoneyMoney(4, 1));
  QVERIFY(b.m_stockSplit == MyMoneyMoney(4, 1));
  QVERIFY(b.m_postSplit == MyMoneyMoney(16, 1));
  QVERIFY(b.formatMoney("", 2) == MyMoneyMoney(32, 1).formatMoney("", 2));

  a += b;

  QVERIFY(a == MyMoneyMoney(3, 1));
  QVERIFY(a.m_stockSplit == MyMoneyMoney(8, 1));
  QVERIFY(a.m_postSplit == MyMoneyMoney(48, 1));
  QVERIFY(a.formatMoney("", 2) == MyMoneyMoney(72, 1).formatMoney("", 2));
}

void PivotGridTest::testCellRunningSum()
{
  PivotCell a;
  MyMoneyMoney runningSum(12, 10);

  a += MyMoneyMoney(3, 1);
  a += PivotCell::stockSplit(MyMoneyMoney(125, 100));
  a += MyMoneyMoney(134, 10);

  QVERIFY(a.m_stockSplit != MyMoneyMoney::ONE);
  QVERIFY(a.m_postSplit != MyMoneyMoney());

  runningSum = a.calculateRunningSum(runningSum);

  QVERIFY(runningSum == MyMoneyMoney(1865, 100));
  QVERIFY(a.formatMoney("", 2) == MyMoneyMoney(1865, 100).formatMoney("", 2));
  QVERIFY(a.m_stockSplit == MyMoneyMoney::ONE);
  QVERIFY(a.m_postSplit == MyMoneyMoney());
}
