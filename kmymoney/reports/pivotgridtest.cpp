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

#include "pivotgridtest.h"

#include "reportstestcommon.h"

#define private public
#include "pivotgrid.h"
#undef private

using namespace reports;
using namespace test;

PivotGridTest::PivotGridTest()
{
}

void PivotGridTest::setUp ()
{
  storage = new MyMoneySeqAccessMgr;
  file = MyMoneyFile::instance();
  file->attachStorage(storage);

  MyMoneyFileTransaction ft;
  file->addCurrency(MyMoneySecurity("CAD", "Canadian Dollar",        "C$"));
  file->addCurrency(MyMoneySecurity("USD", "US Dollar",              "$"));
  file->addCurrency(MyMoneySecurity("JPY", "Japanese Yen",           QChar(0x00A5), 100, 1));
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
  acChecking = makeAccount(QString("Checking Account"),MyMoneyAccount::Checkings,moCheckingOpen,QDate(2004,5,15),acAsset);
  acCredit = makeAccount(QString("Credit Card"),MyMoneyAccount::CreditCard,moCreditOpen,QDate(2004,7,15),acLiability);
  acSolo = makeAccount(QString("Solo"),MyMoneyAccount::Expense,0,QDate(2004,1,11),acExpense);
  acParent = makeAccount(QString("Parent"),MyMoneyAccount::Expense,0,QDate(2004,1,11),acExpense);
  acChild = makeAccount(QString("Child"),MyMoneyAccount::Expense,0,QDate(2004,2,11),acParent);
  acForeign = makeAccount(QString("Foreign"),MyMoneyAccount::Expense,0,QDate(2004,1,11),acExpense);

  acSecondChild = makeAccount(QString("Second Child"),MyMoneyAccount::Expense,0,QDate(2004,2,11),acParent);
  acGrandChild1 = makeAccount(QString("Grand Child 1"),MyMoneyAccount::Expense,0,QDate(2004,2,11),acChild);
  acGrandChild2 = makeAccount(QString("Grand Child 2"),MyMoneyAccount::Expense,0,QDate(2004,2,11),acChild);

  MyMoneyInstitution i("Bank of the World","","","","","","");
  file->addInstitution(i);
  inBank = i.id();
  ft.commit();
}

void PivotGridTest::tearDown ()
{
  file->detachStorage(storage);
  delete storage;
}

void PivotGridTest::testCellAddValue(void)
{
  PivotCell a;
  CPPUNIT_ASSERT(a == MyMoneyMoney(0,1));
  CPPUNIT_ASSERT(a.m_stockSplit == MyMoneyMoney(1,1));
  CPPUNIT_ASSERT(a.m_postSplit == MyMoneyMoney(0,1));
  CPPUNIT_ASSERT(a.formatMoney("", 2) == MyMoneyMoney(0,1).formatMoney("", 2));

  PivotCell b(MyMoneyMoney(13,10));
  CPPUNIT_ASSERT(b == MyMoneyMoney(13,10));
  CPPUNIT_ASSERT(b.m_stockSplit == MyMoneyMoney(1,1));
  CPPUNIT_ASSERT(b.m_postSplit == MyMoneyMoney(0,1));
  CPPUNIT_ASSERT(b.formatMoney("", 2) == MyMoneyMoney(13,10).formatMoney("", 2));

  PivotCell s(b);
  CPPUNIT_ASSERT(s == MyMoneyMoney(13,10));
  CPPUNIT_ASSERT(s.m_stockSplit == MyMoneyMoney(1,1));
  CPPUNIT_ASSERT(s.m_postSplit == MyMoneyMoney(0,1));
  CPPUNIT_ASSERT(s.formatMoney("", 2) == MyMoneyMoney(13,10).formatMoney("", 2));

  s = PivotCell::stockSplit(MyMoneyMoney(1,2));
  CPPUNIT_ASSERT(s == MyMoneyMoney(0,1));
  CPPUNIT_ASSERT(s.m_stockSplit == MyMoneyMoney(1,2));
  CPPUNIT_ASSERT(s.m_postSplit == MyMoneyMoney(0,1));
  CPPUNIT_ASSERT(s.formatMoney("", 2) == MyMoneyMoney(0,1).formatMoney("", 2));
  
  a += MyMoneyMoney(1,1);
  a += MyMoneyMoney(2,1);
  CPPUNIT_ASSERT(a == MyMoneyMoney(3,1));
  CPPUNIT_ASSERT(a.m_stockSplit == MyMoneyMoney(1,1));
  CPPUNIT_ASSERT(a.m_postSplit == MyMoneyMoney(0,1));
  CPPUNIT_ASSERT(a.formatMoney("", 2) == MyMoneyMoney(3,1).formatMoney("", 2));

  a += s;
  CPPUNIT_ASSERT(a == MyMoneyMoney(3,1));
  CPPUNIT_ASSERT(a.m_stockSplit == MyMoneyMoney(1,2));
  CPPUNIT_ASSERT(a.m_postSplit == MyMoneyMoney(0,1));
  CPPUNIT_ASSERT(a.formatMoney("", 2) == MyMoneyMoney(15,10).formatMoney("", 2));

  a += MyMoneyMoney(3,1);
  a += MyMoneyMoney(3,1);
  CPPUNIT_ASSERT(a == MyMoneyMoney(3,1));
  CPPUNIT_ASSERT(a.m_stockSplit == MyMoneyMoney(1,2));
  CPPUNIT_ASSERT(a.m_postSplit == MyMoneyMoney(6,1));
  CPPUNIT_ASSERT(a.formatMoney("", 2) == MyMoneyMoney(75,10).formatMoney("", 2));
}

void PivotGridTest::testCellAddCell(void)
{
  PivotCell a,b;

  a += MyMoneyMoney(3,1);
  a += PivotCell::stockSplit(MyMoneyMoney(2,1));
  a += MyMoneyMoney(4,1);

  CPPUNIT_ASSERT(a == MyMoneyMoney(3,1));
  CPPUNIT_ASSERT(a.m_stockSplit == MyMoneyMoney(2,1));
  CPPUNIT_ASSERT(a.m_postSplit == MyMoneyMoney(4,1));
  CPPUNIT_ASSERT(a.formatMoney("", 2) == MyMoneyMoney(10,1).formatMoney("", 2));

  b += MyMoneyMoney(4,1);
  b += PivotCell::stockSplit(MyMoneyMoney(4,1));
  b += MyMoneyMoney(16,1);
  
  CPPUNIT_ASSERT(b == MyMoneyMoney(4,1));
  CPPUNIT_ASSERT(b.m_stockSplit == MyMoneyMoney(4,1));
  CPPUNIT_ASSERT(b.m_postSplit == MyMoneyMoney(16,1));
  CPPUNIT_ASSERT(b.formatMoney("", 2) == MyMoneyMoney(32,1).formatMoney("", 2));

  a += b;

  CPPUNIT_ASSERT(a == MyMoneyMoney(3,1));
  CPPUNIT_ASSERT(a.m_stockSplit == MyMoneyMoney(8,1));
  CPPUNIT_ASSERT(a.m_postSplit == MyMoneyMoney(48,1));
  CPPUNIT_ASSERT(a.formatMoney("", 2) == MyMoneyMoney(72,1).formatMoney("", 2));
}

void PivotGridTest::testCellRunningSum(void)
{
  PivotCell a;
  MyMoneyMoney runningSum(12,10);

  a += MyMoneyMoney(3,1);
  a += PivotCell::stockSplit(MyMoneyMoney(125,100));
  a += MyMoneyMoney(134,10);

  CPPUNIT_ASSERT(a.m_stockSplit != MyMoneyMoney(1,1));
  CPPUNIT_ASSERT(a.m_postSplit != MyMoneyMoney(0,1));

  runningSum = a.calculateRunningSum(runningSum);

  CPPUNIT_ASSERT(runningSum == MyMoneyMoney(1865,100));
  CPPUNIT_ASSERT(a.formatMoney("", 2) == MyMoneyMoney(1865,100).formatMoney("", 2));
  CPPUNIT_ASSERT(a.m_stockSplit == MyMoneyMoney(1,1));
  CPPUNIT_ASSERT(a.m_postSplit == MyMoneyMoney(0,1));
}

// vim:cin:si:ai:et:ts=2:sw=2:
