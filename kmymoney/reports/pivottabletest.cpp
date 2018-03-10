/***************************************************************************
                          pivottabletest.cpp
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

#include "pivottabletest.h"

#include <QList>
#include <QFile>

#include <kdebug.h>
#include <kdeversion.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <qtest_kde.h>

// DOH, mmreport.h uses this without including it!!
#include "mymoneyaccount.h"

#include "mymoneysecurity.h"
#include "mymoneyprice.h"
#include "mymoneyreport.h"
#include "mymoneystatement.h"
#include "mymoneystoragedump.h"
#include "mymoneystoragexml.h"

#include "pivottable.h"
#include "reportstestcommon.h"

using namespace reports;
using namespace test;

QTEST_KDEMAIN_CORE_WITH_COMPONENTNAME(PivotTableTest, "kmymoney")

void PivotTableTest::init()
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
  acChecking = makeAccount(QString("Checking Account"), MyMoneyAccount::Checkings, moCheckingOpen, QDate(2004, 5, 15), acAsset);
  acCredit = makeAccount(QString("Credit Card"), MyMoneyAccount::CreditCard, moCreditOpen, QDate(2004, 7, 15), acLiability);
  acSolo = makeAccount(QString("Solo"), MyMoneyAccount::Expense, MyMoneyMoney(), QDate(2004, 1, 11), acExpense);
  acParent = makeAccount(QString("Parent"), MyMoneyAccount::Expense, MyMoneyMoney(), QDate(2004, 1, 11), acExpense);
  acChild = makeAccount(QString("Child"), MyMoneyAccount::Expense, MyMoneyMoney(), QDate(2004, 2, 11), acParent);
  acForeign = makeAccount(QString("Foreign"), MyMoneyAccount::Expense, MyMoneyMoney(), QDate(2004, 1, 11), acExpense);

  acSecondChild = makeAccount(QString("Second Child"), MyMoneyAccount::Expense, MyMoneyMoney(), QDate(2004, 2, 11), acParent);
  acGrandChild1 = makeAccount(QString("Grand Child 1"), MyMoneyAccount::Expense, MyMoneyMoney(), QDate(2004, 2, 11), acChild);
  acGrandChild2 = makeAccount(QString("Grand Child 2"), MyMoneyAccount::Expense, MyMoneyMoney(), QDate(2004, 2, 11), acChild);

  MyMoneyInstitution i("Bank of the World", "", "", "", "", "", "");
  file->addInstitution(i);
  inBank = i.id();
  ft.commit();
}

void PivotTableTest::cleanup()
{
  file->detachStorage(storage);
  delete storage;
}

void PivotTableTest::testNetWorthSingle()
{
  try {
    MyMoneyReport filter;
    filter.setRowType(MyMoneyReport::Row::AssetLiability);
    filter.setDateFilter(QDate(2004, 1, 1), QDate(2004, 7, 1).addDays(-1));
    XMLandback(filter);
    PivotTable networth_f(filter);
    writeTabletoCSV(networth_f);

    QVERIFY(networth_f.m_grid["Asset"]["Checking Account"][acChecking][eActual][5] == moCheckingOpen);
    QVERIFY(networth_f.m_grid["Asset"]["Checking Account"][acChecking][eActual][6] == moCheckingOpen);
    QVERIFY(networth_f.m_grid["Asset"]["Checking Account"].m_total[eActual][5] == moCheckingOpen);
    QVERIFY(networth_f.m_grid.m_total[eActual][0] == moZero);
    QVERIFY(networth_f.m_grid.m_total[eActual][4] == moZero);
    QVERIFY(networth_f.m_grid.m_total[eActual][5] == moCheckingOpen);
    QVERIFY(networth_f.m_grid.m_total[eActual][6] == moCheckingOpen);
  } catch (const MyMoneyException &e) {
    QFAIL(qPrintable(e.what()));
  }
}

void PivotTableTest::testNetWorthOfsetting()
{
  // Test the net worth report to make sure it picks up the opening balance for two
  // accounts opened during the period of the report, one asset & one liability.  Test
  // that it calculates the totals correctly.

  MyMoneyReport filter;
  filter.setRowType(MyMoneyReport::Row::AssetLiability);
  filter.setDateFilter(QDate(2004, 1, 1), QDate(2005, 1, 1).addDays(-1));
  XMLandback(filter);
  PivotTable networth_f(filter);
  QVERIFY(networth_f.m_grid["Liability"]["Credit Card"][acCredit][eActual][7] == -moCreditOpen);
  QVERIFY(networth_f.m_grid.m_total[eActual][0] == moZero);
  QVERIFY(networth_f.m_grid.m_total[eActual][12] == moCheckingOpen + moCreditOpen);

}

void PivotTableTest::testNetWorthOpeningPrior()
{
  // Test the net worth report to make sure it's picking up opening balances PRIOR to
  // the period of the report.

  MyMoneyReport filter;
  filter.setRowType(MyMoneyReport::Row::AssetLiability);
  filter.setDateFilter(QDate(2005, 8, 1), QDate(2005, 12, 31));
  filter.setName("Net Worth Opening Prior 1");
  XMLandback(filter);
  PivotTable networth_f(filter);
  writeTabletoCSV(networth_f);

  QVERIFY(networth_f.m_grid["Liability"]["Credit Card"].m_total[eActual][0] == -moCreditOpen);
  QVERIFY(networth_f.m_grid["Asset"]["Checking Account"].m_total[eActual][0] == moCheckingOpen);
  QVERIFY(networth_f.m_grid.m_total[eActual][0] == moCheckingOpen + moCreditOpen);
  QVERIFY(networth_f.m_grid.m_total[eActual][1] == moCheckingOpen + moCreditOpen);

  // Test the net worth report to make sure that transactions prior to the report
  // period are included in the opening balance

  TransactionHelper t1(QDate(2004, 11, 7), MyMoneySplit::ActionWithdrawal, moParent1, acCredit, acParent);
  TransactionHelper t2(QDate(2004, 11, 7), MyMoneySplit::ActionWithdrawal, moParent2, acCredit, acParent);
  TransactionHelper t3(QDate(2004, 11, 7), MyMoneySplit::ActionWithdrawal, moChild, acChecking, acChild);

  filter.setName("Net Worth Opening Prior 2");
  PivotTable networth_f2(filter);
  writeTabletoCSV(networth_f2);

  MyMoneyMoney m1 = (networth_f2.m_grid["Liability"]["Credit Card"].m_total[eActual][1]);
  MyMoneyMoney m2 = (-moCreditOpen + moParent);

  QVERIFY((networth_f2.m_grid["Liability"]["Credit Card"].m_total[eActual][1]) == (-moCreditOpen + moParent));
  QVERIFY(networth_f2.m_grid["Asset"]["Checking Account"].m_total[eActual][1] == moCheckingOpen - moChild);
  QVERIFY(networth_f2.m_grid.m_total[eActual][1] == moCheckingOpen + moCreditOpen - moChild - moParent);
}

void PivotTableTest::testNetWorthDateFilter()
{
  // Test a net worth report whose period is prior to the time any accounts are open,
  // so the report should be zero.

  MyMoneyReport filter;
  filter.setRowType(MyMoneyReport::Row::AssetLiability);
  filter.setDateFilter(QDate(2004, 1, 1), QDate(2004, 2, 1).addDays(-1));
  XMLandback(filter);
  PivotTable networth_f(filter);
  QVERIFY(networth_f.m_grid.m_total[eActual][1] == moZero);

}

void PivotTableTest::testSpendingEmpty()
{
  // test a spending report with no entries

  MyMoneyReport filter;
  filter.setRowType(MyMoneyReport::Row::ExpenseIncome);
  XMLandback(filter);
  PivotTable spending_f1(filter);
  QVERIFY(spending_f1.m_grid.m_total[eActual].m_total == moZero);

  filter.setDateFilter(QDate(2004, 9, 1), QDate(2005, 1, 1).addDays(-1));
  PivotTable spending_f2(filter);
  QVERIFY(spending_f2.m_grid.m_total[eActual].m_total == moZero);
}

void PivotTableTest::testSingleTransaction()
{
  // Test a single transaction
  TransactionHelper t(QDate(2004, 10, 31), MyMoneySplit::ActionWithdrawal, moSolo, acChecking, acSolo);

  MyMoneyReport filter;
  filter.setRowType(MyMoneyReport::Row::ExpenseIncome);
  filter.setDateFilter(QDate(2004, 9, 1), QDate(2005, 1, 1).addDays(-1));
  filter.setName("Spending with Single Transaction.html");
  XMLandback(filter);
  PivotTable spending_f(filter);
  writeTabletoHTML(spending_f, "Spending with Single Transaction.html");

  QVERIFY(spending_f.m_grid["Expense"]["Solo"][acSolo][eActual][2] == moSolo);
  QVERIFY(spending_f.m_grid["Expense"]["Solo"].m_total[eActual][2] == moSolo);
  QVERIFY(spending_f.m_grid["Expense"]["Solo"].m_total[eActual][1] == moZero);
  QVERIFY(spending_f.m_grid.m_total[eActual][2] == (-moSolo));
  QVERIFY(spending_f.m_grid.m_total[eActual].m_total == (-moSolo));

  filter.clear();
  filter.setRowType(MyMoneyReport::Row::AssetLiability);
  filter.setDateFilter(QDate(2004, 9, 1), QDate(2005, 1, 1).addDays(-1));
  XMLandback(filter);
  PivotTable networth_f(filter);
  QVERIFY(networth_f.m_grid["Asset"]["Checking Account"].m_total[eActual][2] == (moCheckingOpen - moSolo));
}

void PivotTableTest::testSubAccount()
{
  // Test a sub-account with a value, under an account with a value

  TransactionHelper t1(QDate(2004, 11, 7), MyMoneySplit::ActionWithdrawal, moParent1, acCredit, acParent);
  TransactionHelper t2(QDate(2004, 11, 7), MyMoneySplit::ActionWithdrawal, moParent2, acCredit, acParent);
  TransactionHelper t3(QDate(2004, 11, 7), MyMoneySplit::ActionWithdrawal, moChild, acCredit, acChild);

  MyMoneyReport filter;
  filter.setRowType(MyMoneyReport::Row::ExpenseIncome);
  filter.setDateFilter(QDate(2004, 9, 1), QDate(2005, 1, 1).addDays(-1));
  filter.setDetailLevel(MyMoneyReport::DetailLevel::All);
  filter.setName("Spending with Sub-Account");
  XMLandback(filter);
  PivotTable spending_f(filter);
  writeTabletoHTML(spending_f, "Spending with Sub-Account.html");

  QVERIFY(spending_f.m_grid["Expense"]["Parent"][acParent][eActual][3] == moParent);
  QVERIFY(spending_f.m_grid["Expense"]["Parent"][acChild][eActual][3] == moChild);
  QVERIFY(spending_f.m_grid["Expense"]["Parent"].m_total[eActual][3] == moParent + moChild);
  QVERIFY(spending_f.m_grid["Expense"]["Parent"].m_total[eActual][2] == moZero);
  QVERIFY(spending_f.m_grid["Expense"]["Parent"].m_total[eActual].m_total == moParent + moChild);
  QVERIFY(spending_f.m_grid.m_total[eActual][3] == (-moParent - moChild));
  QVERIFY(spending_f.m_grid.m_total[eActual].m_total == (-moParent - moChild));

  filter.clear();
  filter.setRowType(MyMoneyReport::Row::AssetLiability);
  filter.setDateFilter(QDate(2004, 9, 1), QDate(2005, 1, 1).addDays(-1));
  filter.setName("Net Worth with Sub-Account");
  XMLandback(filter);
  PivotTable networth_f(filter);
  writeTabletoHTML(networth_f, "Net Worth with Sub-Account.html");
  QVERIFY(networth_f.m_grid["Liability"]["Credit Card"].m_total[eActual][3] == moParent + moChild - moCreditOpen);
  QVERIFY(networth_f.m_grid.m_total[eActual][4] == -moParent - moChild + moCreditOpen + moCheckingOpen);

}

void PivotTableTest::testFilterIEvsIE()
{
  // Test that removing an income/spending account will remove the entry from an income/spending report
  TransactionHelper t1(QDate(2004, 10, 31), MyMoneySplit::ActionWithdrawal, moSolo, acChecking, acSolo);
  TransactionHelper t2(QDate(2004, 11, 7), MyMoneySplit::ActionWithdrawal, moParent1, acCredit, acParent);
  TransactionHelper t3(QDate(2004, 11, 7), MyMoneySplit::ActionWithdrawal, moChild, acCredit, acChild);

  MyMoneyReport filter;
  filter.setRowType(MyMoneyReport::Row::ExpenseIncome);
  filter.setDateFilter(QDate(2004, 9, 1), QDate(2005, 1, 1).addDays(-1));
  filter.addCategory(acChild);
  filter.addCategory(acSolo);
  XMLandback(filter);
  PivotTable spending_f(filter);

  QVERIFY(spending_f.m_grid["Expense"]["Parent"].m_total[eActual][3] == moChild);
  QVERIFY(spending_f.m_grid["Expense"].m_total[eActual][2] == moSolo);
  QVERIFY(spending_f.m_grid.m_total[eActual].m_total == -moSolo - moChild);

}

void PivotTableTest::testFilterALvsAL()
{
  // Test that removing an asset/liability account will remove the entry from an asset/liability report
  TransactionHelper t1(QDate(2004, 10, 31), MyMoneySplit::ActionWithdrawal, moSolo, acChecking, acSolo);
  TransactionHelper t2(QDate(2004, 11, 7), MyMoneySplit::ActionWithdrawal, moParent1, acCredit, acParent);
  TransactionHelper t3(QDate(2004, 11, 7), MyMoneySplit::ActionWithdrawal, moChild, acCredit, acChild);

  MyMoneyReport filter;
  filter.setRowType(MyMoneyReport::Row::AssetLiability);
  filter.setDateFilter(QDate(2004, 9, 1), QDate(2005, 1, 1).addDays(-1));
  filter.addAccount(acChecking);
  filter.addCategory(acChild);
  filter.addCategory(acSolo);
  XMLandback(filter);
  PivotTable networth_f(filter);
  QVERIFY(networth_f.m_grid.m_total[eActual][3] == -moSolo + moCheckingOpen);
}

void PivotTableTest::testFilterALvsIE()
{
  // Test that removing an asset/liability account will remove the entry from an income/spending report
  TransactionHelper t1(QDate(2004, 10, 31), MyMoneySplit::ActionWithdrawal, moSolo, acChecking, acSolo);
  TransactionHelper t2(QDate(2004, 11, 7), MyMoneySplit::ActionWithdrawal, moParent1, acCredit, acParent);
  TransactionHelper t3(QDate(2004, 11, 7), MyMoneySplit::ActionWithdrawal, moChild, acCredit, acChild);

  MyMoneyReport filter;
  filter.setRowType(MyMoneyReport::Row::ExpenseIncome);
  filter.setDateFilter(QDate(2004, 9, 1), QDate(2005, 1, 1).addDays(-1));
  filter.addAccount(acChecking);
  QVERIFY(file->transactionList(filter).count() == 1);

  XMLandback(filter);
  PivotTable spending_f(filter);

  QVERIFY(spending_f.m_grid["Expense"].m_total[eActual][3] == moZero);
  QVERIFY(spending_f.m_grid["Expense"].m_total[eActual][2] == moSolo);
  QVERIFY(spending_f.m_grid.m_total[eActual].m_total == -moSolo);
}

void PivotTableTest::testFilterAllvsIE()
{
  // Test that removing an asset/liability account AND an income/expense
  // category will remove the entry from an income/spending report
  TransactionHelper t1(QDate(2004, 10, 31), MyMoneySplit::ActionWithdrawal, moSolo, acChecking, acSolo);
  TransactionHelper t2(QDate(2004, 11, 7), MyMoneySplit::ActionWithdrawal, moParent1, acCredit, acParent);
  TransactionHelper t3(QDate(2004, 11, 7), MyMoneySplit::ActionWithdrawal, moChild, acCredit, acChild);

  MyMoneyReport filter;
  filter.setRowType(MyMoneyReport::Row::ExpenseIncome);
  filter.setDateFilter(QDate(2004, 9, 1), QDate(2005, 1, 1).addDays(-1));
  filter.addAccount(acCredit);
  filter.addCategory(acChild);
  PivotTable spending_f(filter);

  QVERIFY(spending_f.m_grid["Expense"].m_total[eActual][2] == moZero);
  QVERIFY(spending_f.m_grid["Expense"].m_total[eActual][3] == moChild);
  QVERIFY(spending_f.m_grid.m_total[eActual].m_total == -moChild);
}

void PivotTableTest::testFilterBasics()
{
  // Test that the filters are operating the way that the reports expect them to
  TransactionHelper t1(QDate(2004, 10, 31), MyMoneySplit::ActionWithdrawal, moSolo, acChecking, acSolo);
  TransactionHelper t2(QDate(2004, 11, 7), MyMoneySplit::ActionWithdrawal, moParent1, acCredit, acParent);
  TransactionHelper t3(QDate(2004, 11, 7), MyMoneySplit::ActionWithdrawal, moParent2, acCredit, acParent);
  TransactionHelper t4(QDate(2004, 11, 7), MyMoneySplit::ActionWithdrawal, moChild, acCredit, acChild);

  MyMoneyTransactionFilter filter;
  filter.clear();
  filter.setDateFilter(QDate(2004, 9, 1), QDate(2005, 1, 1).addDays(-1));
  filter.addCategory(acSolo);
  filter.setReportAllSplits(false);
  filter.setConsiderCategory(true);

  QVERIFY(file->transactionList(filter).count() == 1);

  filter.addCategory(acParent);

  QVERIFY(file->transactionList(filter).count() == 3);

  filter.addAccount(acChecking);

  QVERIFY(file->transactionList(filter).count() == 1);

  filter.clear();
  filter.setDateFilter(QDate(2004, 9, 1), QDate(2005, 1, 1).addDays(-1));
  filter.addCategory(acParent);
  filter.addAccount(acCredit);
  filter.setReportAllSplits(false);
  filter.setConsiderCategory(true);

  QVERIFY(file->transactionList(filter).count() == 2);
}

void PivotTableTest::testMultipleCurrencies()
{
  MyMoneyMoney moCanOpening(0.0, 100);
  MyMoneyMoney moJpyOpening(0.0, 100);
  MyMoneyMoney moCanPrice(0.75, 100);
  MyMoneyMoney moJpyPrice(0.010, 100);
  MyMoneyMoney moJpyPrice2(0.011, 100);
  MyMoneyMoney moJpyPrice3(0.014, 100);
  MyMoneyMoney moJpyPrice4(0.0395, 100);
  MyMoneyMoney moCanTransaction(100.0, 100);
  MyMoneyMoney moJpyTransaction(100.0, 100);

  QString acCanChecking = makeAccount(QString("Canadian Checking"), MyMoneyAccount::Checkings, moCanOpening, QDate(2003, 11, 15), acAsset, "CAD");
  QString acJpyChecking = makeAccount(QString("Japanese Checking"), MyMoneyAccount::Checkings, moJpyOpening, QDate(2003, 11, 15), acAsset, "JPY");
  QString acCanCash = makeAccount(QString("Canadian"), MyMoneyAccount::Expense, MyMoneyMoney(), QDate(2004, 2, 11), acForeign, "CAD");
  QString acJpyCash = makeAccount(QString("Japanese"), MyMoneyAccount::Expense, MyMoneyMoney(), QDate(2004, 2, 11), acForeign, "JPY");

  makePrice("CAD", QDate(2004, 1, 1), MyMoneyMoney(moCanPrice));
  makePrice("JPY", QDate(2004, 1, 1), MyMoneyMoney(moJpyPrice));
  makePrice("JPY", QDate(2004, 5, 1), MyMoneyMoney(moJpyPrice2));
  makePrice("JPY", QDate(2004, 6, 30), MyMoneyMoney(moJpyPrice3));
  makePrice("JPY", QDate(2004, 7, 15), MyMoneyMoney(moJpyPrice4));

  TransactionHelper t1(QDate(2004, 2, 20), MyMoneySplit::ActionWithdrawal, MyMoneyMoney(moJpyTransaction), acJpyChecking, acJpyCash, "JPY");
  TransactionHelper t2(QDate(2004, 3, 20), MyMoneySplit::ActionWithdrawal, MyMoneyMoney(moJpyTransaction), acJpyChecking, acJpyCash, "JPY");
  TransactionHelper t3(QDate(2004, 4, 20), MyMoneySplit::ActionWithdrawal, MyMoneyMoney(moJpyTransaction), acJpyChecking, acJpyCash, "JPY");
  TransactionHelper t4(QDate(2004, 2, 20), MyMoneySplit::ActionWithdrawal, MyMoneyMoney(moCanTransaction), acCanChecking, acCanCash, "CAD");
  TransactionHelper t5(QDate(2004, 3, 20), MyMoneySplit::ActionWithdrawal, MyMoneyMoney(moCanTransaction), acCanChecking, acCanCash, "CAD");
  TransactionHelper t6(QDate(2004, 4, 20), MyMoneySplit::ActionWithdrawal, MyMoneyMoney(moCanTransaction), acCanChecking, acCanCash, "CAD");

#if 0
  QFile g("multicurrencykmy.xml");
  g.open(QIODevice::WriteOnly);
  MyMoneyStorageXML xml;
  IMyMoneyStorageFormat& interface = xml;
  interface.writeFile(&g, dynamic_cast<IMyMoneySerialize*>(MyMoneyFile::instance()->storage()));
  g.close();
#endif

  MyMoneyReport filter;
  filter.setRowType(MyMoneyReport::Row::ExpenseIncome);
  filter.setDateFilter(QDate(2004, 1, 1), QDate(2005, 1, 1).addDays(-1));
  filter.setDetailLevel(MyMoneyReport::DetailLevel::All);
  filter.setConvertCurrency(true);
  filter.setName("Multiple Currency Spending Rerport (with currency conversion)");
  XMLandback(filter);

  PivotTable spending_f(filter);

  writeTabletoCSV(spending_f);

  // test single foreign currency
  QVERIFY(spending_f.m_grid["Expense"]["Foreign"][acCanCash][eActual][2] == (moCanTransaction*moCanPrice));
  QVERIFY(spending_f.m_grid["Expense"]["Foreign"][acCanCash][eActual][3] == (moCanTransaction*moCanPrice));
  QVERIFY(spending_f.m_grid["Expense"]["Foreign"][acCanCash][eActual][4] == (moCanTransaction*moCanPrice));

  // test multiple foreign currencies under a common parent
  QVERIFY(spending_f.m_grid["Expense"]["Foreign"][acJpyCash][eActual][2] == (moJpyTransaction*moJpyPrice));
  QVERIFY(spending_f.m_grid["Expense"]["Foreign"][acJpyCash][eActual][3] == (moJpyTransaction*moJpyPrice));
  QVERIFY(spending_f.m_grid["Expense"]["Foreign"][acJpyCash][eActual][4] == (moJpyTransaction*moJpyPrice));
  QVERIFY(spending_f.m_grid["Expense"]["Foreign"].m_total[eActual][2] == (moJpyTransaction*moJpyPrice + moCanTransaction*moCanPrice));
  QVERIFY(spending_f.m_grid["Expense"]["Foreign"].m_total[eActual].m_total == (moJpyTransaction*moJpyPrice + moCanTransaction*moCanPrice + moJpyTransaction*moJpyPrice + moCanTransaction*moCanPrice + moJpyTransaction*moJpyPrice + moCanTransaction*moCanPrice));

  // Test the report type where we DO NOT convert the currency
  filter.setConvertCurrency(false);
  filter.setDetailLevel(MyMoneyReport::DetailLevel::All);
  filter.setName("Multiple Currency Spending Report (WITHOUT currency conversion)");
  XMLandback(filter);
  PivotTable spending_fnc(filter);
  writeTabletoCSV(spending_fnc);

  QVERIFY(spending_fnc.m_grid["Expense"]["Foreign"][acCanCash][eActual][2] == (moCanTransaction));
  QVERIFY(spending_fnc.m_grid["Expense"]["Foreign"][acCanCash][eActual][3] == (moCanTransaction));
  QVERIFY(spending_fnc.m_grid["Expense"]["Foreign"][acCanCash][eActual][4] == (moCanTransaction));
  QVERIFY(spending_fnc.m_grid["Expense"]["Foreign"][acJpyCash][eActual][2] == (moJpyTransaction));
  QVERIFY(spending_fnc.m_grid["Expense"]["Foreign"][acJpyCash][eActual][3] == (moJpyTransaction));
  QVERIFY(spending_fnc.m_grid["Expense"]["Foreign"][acJpyCash][eActual][4] == (moJpyTransaction));

  filter.setConvertCurrency(true);
  filter.clear();
  filter.setName("Multiple currency net worth");
  filter.setRowType(MyMoneyReport::Row::AssetLiability);
  filter.setDateFilter(QDate(2004, 1, 1), QDate(2005, 1, 1).addDays(-1));
  XMLandback(filter);
  PivotTable networth_f(filter);
  writeTabletoCSV(networth_f);

  // test single foreign currency
  QVERIFY(networth_f.m_grid["Asset"]["Canadian Checking"][acCanChecking][eActual][1] == (moCanOpening*moCanPrice));
  QVERIFY(networth_f.m_grid["Asset"]["Canadian Checking"][acCanChecking][eActual][2] == ((moCanOpening - moCanTransaction)*moCanPrice));
  QVERIFY(networth_f.m_grid["Asset"]["Canadian Checking"][acCanChecking][eActual][3] == ((moCanOpening - moCanTransaction - moCanTransaction)*moCanPrice));
  QVERIFY(networth_f.m_grid["Asset"]["Canadian Checking"][acCanChecking][eActual][4] == ((moCanOpening - moCanTransaction - moCanTransaction - moCanTransaction)*moCanPrice));
  QVERIFY(networth_f.m_grid["Asset"]["Canadian Checking"][acCanChecking][eActual][12] == ((moCanOpening - moCanTransaction - moCanTransaction - moCanTransaction)*moCanPrice));

  // test Stable currency price, fluctuating account balance
  QVERIFY(networth_f.m_grid["Asset"]["Japanese Checking"][acJpyChecking][eActual][1] == (moJpyOpening*moJpyPrice));
  QVERIFY(networth_f.m_grid["Asset"]["Japanese Checking"][acJpyChecking][eActual][2] == ((moJpyOpening - moJpyTransaction)*moJpyPrice));
  QVERIFY(networth_f.m_grid["Asset"]["Japanese Checking"][acJpyChecking][eActual][3] == ((moJpyOpening - moJpyTransaction - moJpyTransaction)*moJpyPrice));
  QVERIFY(networth_f.m_grid["Asset"]["Japanese Checking"][acJpyChecking][eActual][4] == ((moJpyOpening - moJpyTransaction - moJpyTransaction - moJpyTransaction)*moJpyPrice));

  // test Fluctuating currency price, stable account balance
  QVERIFY(networth_f.m_grid["Asset"]["Japanese Checking"][acJpyChecking][eActual][5] == ((moJpyOpening - moJpyTransaction - moJpyTransaction - moJpyTransaction)*moJpyPrice2));
  QVERIFY(networth_f.m_grid["Asset"]["Japanese Checking"][acJpyChecking][eActual][6] == ((moJpyOpening - moJpyTransaction - moJpyTransaction - moJpyTransaction)*moJpyPrice3));
  QVERIFY(networth_f.m_grid["Asset"]["Japanese Checking"][acJpyChecking][eActual][7] == ((moJpyOpening - moJpyTransaction - moJpyTransaction - moJpyTransaction)*moJpyPrice4));

  // test multiple currencies totalled up
  QVERIFY(networth_f.m_grid["Asset"].m_total[eActual][4] == ((moCanOpening - moCanTransaction - moCanTransaction - moCanTransaction)*moCanPrice) + ((moJpyOpening - moJpyTransaction - moJpyTransaction - moJpyTransaction)*moJpyPrice));
  QVERIFY(networth_f.m_grid["Asset"].m_total[eActual][5] == ((moCanOpening - moCanTransaction - moCanTransaction - moCanTransaction)*moCanPrice) + ((moJpyOpening - moJpyTransaction - moJpyTransaction - moJpyTransaction)*moJpyPrice2) + moCheckingOpen);

}

void PivotTableTest::testAdvancedFilter()
{
  // test more advanced filtering capabilities

  // amount
  {
    TransactionHelper t1(QDate(2004, 11, 7), MyMoneySplit::ActionWithdrawal, moParent1, acCredit, acParent);
    TransactionHelper t2(QDate(2004, 11, 7), MyMoneySplit::ActionWithdrawal, moChild, acCredit, acChild);

    MyMoneyReport filter;
    filter.setRowType(MyMoneyReport::Row::ExpenseIncome);
    filter.setDateFilter(QDate(2004, 1, 1), QDate(2005, 1, 1).addDays(-1));
    filter.setAmountFilter(moChild, moChild);
    XMLandback(filter);
    PivotTable spending_f(filter);
    QVERIFY(spending_f.m_grid.m_total[eActual].m_total == -moChild);
  }

  // payee (specific)
  {
    TransactionHelper t1(QDate(2004, 10, 31), MyMoneySplit::ActionWithdrawal, moSolo, acChecking, acSolo);
    TransactionHelper t2(QDate(2004, 11, 7), MyMoneySplit::ActionWithdrawal, moParent1, acCredit, acParent);
    TransactionHelper t3(QDate(2004, 11, 7), MyMoneySplit::ActionWithdrawal, moChild, acCredit, acChild);
    TransactionHelper t4(QDate(2004, 11, 7), MyMoneySplit::ActionWithdrawal, moThomas, acCredit, acParent, QString(), "Thomas Baumgart");

    MyMoneyReport filter;
    filter.setRowType(MyMoneyReport::Row::ExpenseIncome);
    filter.setDateFilter(QDate(2004, 1, 1), QDate(2005, 1, 1).addDays(-1));
    filter.addPayee(MyMoneyFile::instance()->payeeByName("Thomas Baumgart").id());
    filter.setName("Spending with Payee Filter");
    XMLandback(filter);
    PivotTable spending_f(filter);
    writeTabletoHTML(spending_f, "Spending with Payee Filter.html");

    QVERIFY(spending_f.m_grid["Expense"]["Parent"][acParent][eActual][11] == moThomas);
    QVERIFY(spending_f.m_grid.m_total[eActual].m_total == -moThomas);
  }
  // payee (no payee)
  {
    TransactionHelper t1(QDate(2004, 10, 31), MyMoneySplit::ActionWithdrawal, moSolo, acChecking, acSolo);
    TransactionHelper t2(QDate(2004, 11, 7), MyMoneySplit::ActionWithdrawal, moParent1, acCredit, acParent);
    TransactionHelper t3(QDate(2004, 11, 7), MyMoneySplit::ActionWithdrawal, moChild, acCredit, acChild);
    TransactionHelper t4(QDate(2004, 11, 7), MyMoneySplit::ActionWithdrawal, moNoPayee, acCredit, acParent, QString(), QString());

    MyMoneyReport filter;
    filter.setRowType(MyMoneyReport::Row::ExpenseIncome);
    filter.setDateFilter(QDate(2004, 1, 1), QDate(2005, 1, 1).addDays(-1));
    filter.addPayee(QString());
    XMLandback(filter);
    PivotTable spending_f(filter);
    QVERIFY(spending_f.m_grid["Expense"]["Parent"][acParent][eActual][11] == moNoPayee);
    QVERIFY(spending_f.m_grid.m_total[eActual].m_total == -moNoPayee);
  }

  // text
  {
    TransactionHelper t1(QDate(2004, 10, 31), MyMoneySplit::ActionWithdrawal, moSolo, acChecking, acSolo);
    TransactionHelper t2(QDate(2004, 11, 7), MyMoneySplit::ActionWithdrawal, moParent1, acCredit, acParent);
    TransactionHelper t3(QDate(2004, 11, 7), MyMoneySplit::ActionWithdrawal, moChild, acCredit, acChild);
    TransactionHelper t4(QDate(2004, 11, 7), MyMoneySplit::ActionWithdrawal, moThomas, acCredit, acParent, QString(), "Thomas Baumgart");

    MyMoneyReport filter;
    filter.setRowType(MyMoneyReport::Row::ExpenseIncome);
    filter.setDateFilter(QDate(2004, 1, 1), QDate(2005, 1, 1).addDays(-1));
    filter.setTextFilter(QRegExp("Thomas"));
    XMLandback(filter);
    PivotTable spending_f(filter);
  }

  // type (payment, deposit, transfer)
  {
    TransactionHelper t1(QDate(2004, 1, 1), MyMoneySplit::ActionWithdrawal, moSolo, acChecking, acSolo);
    TransactionHelper t2(QDate(2004, 2, 1), MyMoneySplit::ActionDeposit, -moParent1, acCredit, acParent);
    TransactionHelper t3(QDate(2004, 11, 1), MyMoneySplit::ActionTransfer, moChild, acCredit, acChecking);

    MyMoneyReport filter;
    filter.setRowType(MyMoneyReport::Row::ExpenseIncome);
    filter.addType(MyMoneyTransactionFilter::payments);
    XMLandback(filter);
    PivotTable spending_f(filter);

    QVERIFY(spending_f.m_grid.m_total[eActual].m_total == -moSolo);

    filter.clear();
    filter.addType(MyMoneyTransactionFilter::deposits);
    XMLandback(filter);
    PivotTable spending_f2(filter);

    QVERIFY(spending_f2.m_grid.m_total[eActual].m_total == moParent1);

    filter.clear();
    filter.addType(MyMoneyTransactionFilter::transfers);
    XMLandback(filter);
    PivotTable spending_f3(filter);

    QVERIFY(spending_f3.m_grid.m_total[eActual].m_total == moZero);

    filter.setRowType(MyMoneyReport::Row::AssetLiability);
    filter.setDateFilter(QDate(2004, 1, 1), QDate(2004, 12, 31));
    XMLandback(filter);
    PivotTable networth_f4(filter);

    QVERIFY(networth_f4.m_grid["Asset"].m_total[eActual][11] == moCheckingOpen + moChild);
    QVERIFY(networth_f4.m_grid["Liability"].m_total[eActual][11] == - moCreditOpen + moChild);
    QVERIFY(networth_f4.m_grid.m_total[eActual][10] == moCheckingOpen + moCreditOpen);
    QVERIFY(networth_f4.m_grid.m_total[eActual][11] == moCheckingOpen + moCreditOpen);
  }

  // state (reconciled, cleared, not)
  {
    TransactionHelper t1(QDate(2004, 1, 1), MyMoneySplit::ActionWithdrawal, moSolo, acChecking, acSolo);
    TransactionHelper t2(QDate(2004, 2, 1), MyMoneySplit::ActionWithdrawal, moParent1, acCredit, acParent);
    TransactionHelper t3(QDate(2004, 3, 1), MyMoneySplit::ActionWithdrawal, moParent2, acCredit, acParent);
    TransactionHelper t4(QDate(2004, 4, 1), MyMoneySplit::ActionWithdrawal, moChild, acCredit, acChild);

    QList<MyMoneySplit> splits = t1.splits();
    splits[0].setReconcileFlag(MyMoneySplit::Cleared);
    splits[1].setReconcileFlag(MyMoneySplit::Cleared);
    t1.modifySplit(splits[0]);
    t1.modifySplit(splits[1]);
    t1.update();

    splits.clear();
    splits = t2.splits();
    splits[0].setReconcileFlag(MyMoneySplit::Reconciled);
    splits[1].setReconcileFlag(MyMoneySplit::Reconciled);
    t2.modifySplit(splits[0]);
    t2.modifySplit(splits[1]);
    t2.update();

    MyMoneyReport filter;
    filter.setRowType(MyMoneyReport::Row::ExpenseIncome);
    filter.setDateFilter(QDate(2004, 1, 1), QDate(2005, 1, 1).addDays(-1));
    filter.addState(MyMoneyTransactionFilter::cleared);
    XMLandback(filter);
    PivotTable spending_f(filter);

    QVERIFY(spending_f.m_grid.m_total[eActual].m_total == -moSolo);

    filter.addState(MyMoneyTransactionFilter::reconciled);
    XMLandback(filter);
    PivotTable spending_f2(filter);

    QVERIFY(spending_f2.m_grid.m_total[eActual].m_total == -moSolo - moParent1);

    filter.clear();
    filter.addState(MyMoneyTransactionFilter::notReconciled);
    XMLandback(filter);
    PivotTable spending_f3(filter);

    QVERIFY(spending_f3.m_grid.m_total[eActual].m_total == -moChild - moParent2);
  }

  // number
  {
    TransactionHelper t1(QDate(2004, 10, 31), MyMoneySplit::ActionWithdrawal, moSolo, acChecking, acSolo);
    TransactionHelper t2(QDate(2004, 11, 7), MyMoneySplit::ActionWithdrawal, moParent1, acCredit, acParent);
    TransactionHelper t3(QDate(2004, 11, 7), MyMoneySplit::ActionWithdrawal, moParent2, acCredit, acParent);
    TransactionHelper t4(QDate(2004, 11, 7), MyMoneySplit::ActionWithdrawal, moChild, acCredit, acChild);

    QList<MyMoneySplit> splits = t1.splits();
    splits[0].setNumber("1");
    splits[1].setNumber("1");
    t1.modifySplit(splits[0]);
    t1.modifySplit(splits[1]);
    t1.update();

    splits.clear();
    splits = t2.splits();
    splits[0].setNumber("2");
    splits[1].setNumber("2");
    t2.modifySplit(splits[0]);
    t2.modifySplit(splits[1]);
    t2.update();

    splits.clear();
    splits = t3.splits();
    splits[0].setNumber("3");
    splits[1].setNumber("3");
    t3.modifySplit(splits[0]);
    t3.modifySplit(splits[1]);
    t3.update();

    splits.clear();
    splits = t2.splits();
    splits[0].setNumber("4");
    splits[1].setNumber("4");
    t4.modifySplit(splits[0]);
    t4.modifySplit(splits[1]);
    t4.update();

    MyMoneyReport filter;
    filter.setRowType(MyMoneyReport::Row::ExpenseIncome);
    filter.setDateFilter(QDate(2004, 1, 1), QDate(2005, 1, 1).addDays(-1));
    filter.setNumberFilter("1", "3");
    XMLandback(filter);
    PivotTable spending_f(filter);
    QVERIFY(spending_f.m_grid.m_total[eActual].m_total == -moSolo - moParent1 - moParent2);
  }

  // blank dates
  {
    TransactionHelper t1y1(QDate(2003, 10, 1), MyMoneySplit::ActionWithdrawal, moSolo, acChecking, acSolo);
    TransactionHelper t2y1(QDate(2003, 11, 1), MyMoneySplit::ActionWithdrawal, moParent1, acCredit, acParent);
    TransactionHelper t3y1(QDate(2003, 12, 1), MyMoneySplit::ActionWithdrawal, moParent2, acCredit, acParent);

    TransactionHelper t1y2(QDate(2004, 4, 1), MyMoneySplit::ActionWithdrawal, moSolo, acChecking, acSolo);
    TransactionHelper t2y2(QDate(2004, 5, 1), MyMoneySplit::ActionWithdrawal, moParent1, acCredit, acParent);
    TransactionHelper t3y2(QDate(2004, 6, 1), MyMoneySplit::ActionWithdrawal, moParent2, acCredit, acParent);

    TransactionHelper t1y3(QDate(2005, 1, 1), MyMoneySplit::ActionWithdrawal, moSolo, acChecking, acSolo);
    TransactionHelper t2y3(QDate(2005, 5, 1), MyMoneySplit::ActionWithdrawal, moParent1, acCredit, acParent);
    TransactionHelper t3y3(QDate(2005, 9, 1), MyMoneySplit::ActionWithdrawal, moParent2, acCredit, acParent);

    MyMoneyReport filter;
    filter.setRowType(MyMoneyReport::Row::ExpenseIncome);
    filter.setDateFilter(QDate(), QDate(2004, 7, 1));
    XMLandback(filter);
    PivotTable spending_f(filter);
    QVERIFY(spending_f.m_grid.m_total[eActual].m_total == -moSolo - moParent1 - moParent2 - moSolo - moParent1 - moParent2);

    filter.clear();
    XMLandback(filter);
    PivotTable spending_f2(filter);
    QVERIFY(spending_f2.m_grid.m_total[eActual].m_total == -moSolo - moParent1 - moParent2 - moSolo - moParent1 - moParent2 - moSolo - moParent1 - moParent2);

  }

}

void PivotTableTest::testColumnType()
{
  // test column type values of other than 'month'

  TransactionHelper t1q1(QDate(2004, 1, 1), MyMoneySplit::ActionWithdrawal, moSolo, acChecking, acSolo);
  TransactionHelper t2q1(QDate(2004, 2, 1), MyMoneySplit::ActionWithdrawal, moParent1, acCredit, acParent);
  TransactionHelper t3q1(QDate(2004, 3, 1), MyMoneySplit::ActionWithdrawal, moParent2, acCredit, acParent);

  TransactionHelper t1q2(QDate(2004, 4, 1), MyMoneySplit::ActionWithdrawal, moSolo, acChecking, acSolo);
  TransactionHelper t2q2(QDate(2004, 5, 1), MyMoneySplit::ActionWithdrawal, moParent1, acCredit, acParent);
  TransactionHelper t3q2(QDate(2004, 6, 1), MyMoneySplit::ActionWithdrawal, moParent2, acCredit, acParent);

  TransactionHelper t1y2(QDate(2005, 1, 1), MyMoneySplit::ActionWithdrawal, moSolo, acChecking, acSolo);
  TransactionHelper t2y2(QDate(2005, 5, 1), MyMoneySplit::ActionWithdrawal, moParent1, acCredit, acParent);
  TransactionHelper t3y2(QDate(2005, 9, 1), MyMoneySplit::ActionWithdrawal, moParent2, acCredit, acParent);

  MyMoneyReport filter;
  filter.setRowType(MyMoneyReport::Row::ExpenseIncome);
  filter.setDateFilter(QDate(2003, 12, 31), QDate(2005, 12, 31));
  filter.setRowType(MyMoneyReport::Row::ExpenseIncome);
  filter.setColumnType(MyMoneyReport::Column::BiMonths);
  XMLandback(filter);
  PivotTable spending_b(filter);

  QVERIFY(spending_b.m_grid.m_total[eActual][1] == moZero);
  QVERIFY(spending_b.m_grid.m_total[eActual][2] == -moParent1 - moSolo);
  QVERIFY(spending_b.m_grid.m_total[eActual][3] == -moParent2 - moSolo);
  QVERIFY(spending_b.m_grid.m_total[eActual][4] == -moParent);
  QVERIFY(spending_b.m_grid.m_total[eActual][5] == moZero);
  QVERIFY(spending_b.m_grid.m_total[eActual][6] == moZero);
  QVERIFY(spending_b.m_grid.m_total[eActual][7] == moZero);
  QVERIFY(spending_b.m_grid.m_total[eActual][8] == -moSolo);
  QVERIFY(spending_b.m_grid.m_total[eActual][9] == moZero);
  QVERIFY(spending_b.m_grid.m_total[eActual][10] == -moParent1);
  QVERIFY(spending_b.m_grid.m_total[eActual][11] == moZero);
  QVERIFY(spending_b.m_grid.m_total[eActual][12] == -moParent2);
  QVERIFY(spending_b.m_grid.m_total[eActual][13] == moZero);

  filter.setColumnType(MyMoneyReport::Column::Quarters);
  XMLandback(filter);
  PivotTable spending_q(filter);

  QVERIFY(spending_q.m_grid.m_total[eActual][1] == moZero);
  QVERIFY(spending_q.m_grid.m_total[eActual][2] == -moSolo - moParent);
  QVERIFY(spending_q.m_grid.m_total[eActual][3] == -moSolo - moParent);
  QVERIFY(spending_q.m_grid.m_total[eActual][4] == moZero);
  QVERIFY(spending_q.m_grid.m_total[eActual][5] == moZero);
  QVERIFY(spending_q.m_grid.m_total[eActual][6] == -moSolo);
  QVERIFY(spending_q.m_grid.m_total[eActual][7] == -moParent1);
  QVERIFY(spending_q.m_grid.m_total[eActual][8] == -moParent2);
  QVERIFY(spending_q.m_grid.m_total[eActual][9] == moZero);

  filter.setRowType(MyMoneyReport::Row::AssetLiability);
  filter.setName("Net Worth by Quarter");
  XMLandback(filter);
  PivotTable networth_q(filter);
  writeTabletoHTML(networth_q, "Net Worth by Quarter.html");

  QVERIFY(networth_q.m_grid.m_total[eActual][1] == moZero);
  QVERIFY(networth_q.m_grid.m_total[eActual][2] == -moSolo - moParent);
  QVERIFY(networth_q.m_grid.m_total[eActual][3] == -moSolo - moParent - moSolo - moParent + moCheckingOpen);
  QVERIFY(networth_q.m_grid.m_total[eActual][4] == -moSolo - moParent - moSolo - moParent + moCheckingOpen + moCreditOpen);
  QVERIFY(networth_q.m_grid.m_total[eActual][5] == -moSolo - moParent - moSolo - moParent + moCheckingOpen + moCreditOpen);
  QVERIFY(networth_q.m_grid.m_total[eActual][6] == -moSolo - moSolo - moParent - moSolo - moParent + moCheckingOpen + moCreditOpen);
  QVERIFY(networth_q.m_grid.m_total[eActual][7] == -moParent1 - moSolo - moSolo - moParent - moSolo - moParent + moCheckingOpen + moCreditOpen);
  QVERIFY(networth_q.m_grid.m_total[eActual][8] == -moParent2 - moParent1 - moSolo - moSolo - moParent - moSolo - moParent + moCheckingOpen + moCreditOpen);
  QVERIFY(networth_q.m_grid.m_total[eActual][9] == -moParent2 - moParent1 - moSolo - moSolo - moParent - moSolo - moParent + moCheckingOpen + moCreditOpen);

  filter.setRowType(MyMoneyReport::Row::ExpenseIncome);
  filter.setColumnType(MyMoneyReport::Column::Years);
  XMLandback(filter);
  PivotTable spending_y(filter);

  QVERIFY(spending_y.m_grid.m_total[eActual][1] == moZero);
  QVERIFY(spending_y.m_grid.m_total[eActual][2] == -moSolo - moParent - moSolo - moParent);
  QVERIFY(spending_y.m_grid.m_total[eActual][3] == -moSolo - moParent);
  QVERIFY(spending_y.m_grid.m_total[eActual].m_total == -moSolo - moParent - moSolo - moParent - moSolo - moParent);

  filter.setRowType(MyMoneyReport::Row::AssetLiability);
  XMLandback(filter);
  PivotTable networth_y(filter);

  QVERIFY(networth_y.m_grid.m_total[eActual][1] == moZero);
  QVERIFY(networth_y.m_grid.m_total[eActual][2] == -moSolo - moParent - moSolo - moParent + moCheckingOpen + moCreditOpen);
  QVERIFY(networth_y.m_grid.m_total[eActual][3] == -moSolo - moParent - moSolo - moParent - moSolo - moParent + moCheckingOpen + moCreditOpen);

  // Test days-based reports

  TransactionHelper t1d1(QDate(2004, 7, 1), MyMoneySplit::ActionWithdrawal, moSolo, acChecking, acSolo);
  TransactionHelper t2d1(QDate(2004, 7, 1), MyMoneySplit::ActionWithdrawal, moParent1, acCredit, acParent);
  TransactionHelper t3d1(QDate(2004, 7, 5), MyMoneySplit::ActionWithdrawal, moParent2, acCredit, acParent);

  TransactionHelper t1d2(QDate(2004, 7, 14), MyMoneySplit::ActionWithdrawal, moSolo, acChecking, acSolo);
  TransactionHelper t2d2(QDate(2004, 7, 15), MyMoneySplit::ActionWithdrawal, moParent1, acCredit, acParent);
  TransactionHelper t3d2(QDate(2004, 7, 20), MyMoneySplit::ActionWithdrawal, moParent2, acCredit, acParent);

  TransactionHelper t1d3(QDate(2004, 8, 2), MyMoneySplit::ActionWithdrawal, moSolo, acChecking, acSolo);
  TransactionHelper t2d3(QDate(2004, 8, 3), MyMoneySplit::ActionWithdrawal, moParent1, acCredit, acParent);
  TransactionHelper t3d3(QDate(2004, 8, 4), MyMoneySplit::ActionWithdrawal, moParent2, acCredit, acParent);

  filter.setDateFilter(QDate(2004, 7, 2), QDate(2004, 7, 14));
  filter.setRowType(MyMoneyReport::Row::ExpenseIncome);
  filter.setColumnType(MyMoneyReport::Column::Months);
  filter.setColumnsAreDays(true);
  filter.setName("Spending by Days");

  XMLandback(filter);
  PivotTable spending_days(filter);
  writeTabletoHTML(spending_days, "Spending by Days.html");

  QVERIFY(spending_days.m_grid.m_total[eActual][4] == -moParent2);
  QVERIFY(spending_days.m_grid.m_total[eActual][13] == -moSolo);
  QVERIFY(spending_days.m_grid.m_total[eActual].m_total == -moSolo - moParent2);

  unsigned save_dayweekstart = KGlobal::locale()->weekStartDay();
  KGlobal::locale()->setWeekStartDay(2);

  filter.setDateFilter(QDate(2004, 7, 2), QDate(2004, 8, 1));
  filter.setRowType(MyMoneyReport::Row::ExpenseIncome);
  filter.setColumnType(static_cast<MyMoneyReport::Column::Type>(7));
  filter.setColumnsAreDays(true);
  filter.setName("Spending by Weeks");

  XMLandback(filter);
  PivotTable spending_weeks(filter);
  writeTabletoHTML(spending_weeks, "Spending by Weeks.html");

  KGlobal::locale()->setWeekStartDay(save_dayweekstart);

  QVERIFY(spending_weeks.m_grid.m_total[eActual][0] == moZero);
  QVERIFY(spending_weeks.m_grid.m_total[eActual][1] == -moParent2);
  QVERIFY(spending_weeks.m_grid.m_total[eActual][2] == moZero);
  QVERIFY(spending_weeks.m_grid.m_total[eActual][3] == -moSolo - moParent1);
  QVERIFY(spending_weeks.m_grid.m_total[eActual][4] == -moParent2);
  QVERIFY(spending_weeks.m_grid.m_total[eActual][5] == moZero);
  QVERIFY(spending_weeks.m_grid.m_total[eActual].m_total == -moSolo - moParent - moParent2);


}

void PivotTableTest::testInvestment()
{
  try {
    // Equities
    eqStock1 = makeEquity("Stock1", "STK1");
    eqStock2 = makeEquity("Stock2", "STK2");

    // Accounts
    acInvestment = makeAccount("Investment", MyMoneyAccount::Investment, moZero, QDate(2004, 1, 1), acAsset);
    acStock1 = makeAccount("Stock 1", MyMoneyAccount::Stock, moZero, QDate(2004, 1, 1), acInvestment, eqStock1);
    acStock2 = makeAccount("Stock 2", MyMoneyAccount::Stock, moZero, QDate(2004, 1, 1), acInvestment, eqStock2);
    acDividends = makeAccount("Dividends", MyMoneyAccount::Income, moZero, QDate(2004, 1, 1), acIncome);

    // Transactions
    //                         Date             Action                                              Shares                 Price   Stock     Asset       Income
    InvTransactionHelper s1b1(QDate(2004, 2, 1), MyMoneySplit::ActionBuyShares,        MyMoneyMoney(1000.00), MyMoneyMoney(100.00), acStock1, acChecking, QString());
    InvTransactionHelper s1b2(QDate(2004, 3, 1), MyMoneySplit::ActionBuyShares,        MyMoneyMoney(1000.00), MyMoneyMoney(110.00), acStock1, acChecking, QString());
    InvTransactionHelper s1s1(QDate(2004, 4, 1), MyMoneySplit::ActionBuyShares,        MyMoneyMoney(-200.00), MyMoneyMoney(120.00), acStock1, acChecking, QString());
    InvTransactionHelper s1s2(QDate(2004, 5, 1), MyMoneySplit::ActionBuyShares,        MyMoneyMoney(-200.00), MyMoneyMoney(100.00), acStock1, acChecking, QString());
    InvTransactionHelper s1r1(QDate(2004, 6, 1), MyMoneySplit::ActionReinvestDividend, MyMoneyMoney(50.00), MyMoneyMoney(100.00), acStock1, QString(), acDividends);
    InvTransactionHelper s1r2(QDate(2004, 7, 1), MyMoneySplit::ActionReinvestDividend, MyMoneyMoney(50.00), MyMoneyMoney(80.00), acStock1, QString(), acDividends);
    InvTransactionHelper s1c1(QDate(2004, 8, 1), MyMoneySplit::ActionDividend,         MyMoneyMoney(10.00), MyMoneyMoney(100.00), acStock1, acChecking, acDividends);
    InvTransactionHelper s1c2(QDate(2004, 9, 1), MyMoneySplit::ActionDividend,         MyMoneyMoney(10.00), MyMoneyMoney(120.00), acStock1, acChecking, acDividends);

    makeEquityPrice(eqStock1, QDate(2004, 10, 1), MyMoneyMoney(100.00));

    //
    // Net Worth Report (with investments)
    //

    MyMoneyReport networth_r;
    networth_r.setRowType(MyMoneyReport::Row::AssetLiability);
    networth_r.setDateFilter(QDate(2004, 1, 1), QDate(2004, 12, 31).addDays(-1));
    XMLandback(networth_r);
    PivotTable networth(networth_r);

    networth.dump("networth_i.html");

    QVERIFY(networth.m_grid["Asset"]["Investment"].m_total[eActual][1] == moZero);
    // 1000 shares @ $100.00
    QVERIFY(networth.m_grid["Asset"]["Investment"].m_total[eActual][2] == MyMoneyMoney(100000.0));
    // 2000 shares @ $110.00
    QVERIFY(networth.m_grid["Asset"]["Investment"].m_total[eActual][3] == MyMoneyMoney(220000.0));
    // 1800 shares @ $120.00
    QVERIFY(networth.m_grid["Asset"]["Investment"].m_total[eActual][4] == MyMoneyMoney(216000.0));
    // 1600 shares @ $100.00
    QVERIFY(networth.m_grid["Asset"]["Investment"].m_total[eActual][5] == MyMoneyMoney(160000.0));
    // 1650 shares @ $100.00
    QVERIFY(networth.m_grid["Asset"]["Investment"].m_total[eActual][6] == MyMoneyMoney(165000.0));
    // 1700 shares @ $ 80.00
    QVERIFY(networth.m_grid["Asset"]["Investment"].m_total[eActual][7] == MyMoneyMoney(136000.0));
    // 1700 shares @ $100.00
    QVERIFY(networth.m_grid["Asset"]["Investment"].m_total[eActual][8] == MyMoneyMoney(170000.0));
    // 1700 shares @ $120.00
    QVERIFY(networth.m_grid["Asset"]["Investment"].m_total[eActual][9] == MyMoneyMoney(204000.0));
    // 1700 shares @ $100.00
    QVERIFY(networth.m_grid["Asset"]["Investment"].m_total[eActual][10] == MyMoneyMoney(170000.0));

#if 0
    // Dump file & reports
    QFile g("investmentkmy.xml");
    g.open(QIODevice::WriteOnly);
    MyMoneyStorageXML xml;
    IMyMoneyStorageFormat& interface = xml;
    interface.writeFile(&g, dynamic_cast<IMyMoneySerialize*>(MyMoneyFile::instance()->storage()));
    g.close();

    invtran.dump("invtran.html", "<html><head></head><body>%1</body></html>");
    invhold.dump("invhold.html", "<html><head></head><body>%1</body></html>");
#endif

  } catch (const MyMoneyException &e) {
    QFAIL(qPrintable(e.what()));
  }
}

void PivotTableTest::testBudget()
{

  // 1. Budget on A, transactions on A
  {
    BudgetHelper budget;
    budget += BudgetEntryHelper(QDate(2006, 1, 1), acSolo, false, MyMoneyMoney(100.0));

    MyMoneyReport report(MyMoneyReport::Row::BudgetActual,
                         MyMoneyReport::Column::Months,
                         MyMoneyTransactionFilter::yearToDate,
                         MyMoneyReport::DetailLevel::Top,
                         "Yearly Budgeted vs. Actual", "Default Report");
    PivotTable table(report);
  }

  // 2. Budget on B, not applying to sub accounts, transactions on B and B:1
  {
    BudgetHelper budget;
    budget += BudgetEntryHelper(QDate(2006, 1, 1), acParent, false, MyMoneyMoney(100.0));
    MyMoneyReport report(MyMoneyReport::Row::BudgetActual,
                         MyMoneyReport::Column::Months,
                         MyMoneyTransactionFilter::yearToDate,
                         MyMoneyReport::DetailLevel::Top,
                         "Yearly Budgeted vs. Actual", "Default Report");
    PivotTable table(report);
  }

  //  - Both B and B:1 totals should show up
  //  - B actuals compare against B budget
  //  - B:1 actuals compare against 0

  // 3. Budget on C, applying to sub accounts, transactions on C and C:1 and C:1:a
  {
    BudgetHelper budget;
    budget += BudgetEntryHelper(QDate(2006, 1, 1), acParent, true, MyMoneyMoney(100.0));
    MyMoneyReport report(MyMoneyReport::Row::BudgetActual,
                         MyMoneyReport::Column::Months,
                         MyMoneyTransactionFilter::yearToDate,
                         MyMoneyReport::DetailLevel::Top ,
                         "Yearly Budgeted vs. Actual", "Default Report");
    PivotTable table(report);
  }

  //  - Only C totals show up, not C:1 or C:1:a totals
  //  - C + C:1 totals compare against C budget

  // 4. Budget on D, not applying to sub accounts, budget on D:1 not applying, budget on D:2 applying.  Transactions on D, D:1, D:2, D:2:a, D:2:b
  {
    BudgetHelper budget;
    budget += BudgetEntryHelper(QDate(2006, 1, 1), acParent, false, MyMoneyMoney(100.0));
    budget += BudgetEntryHelper(QDate(2006, 1, 1), acChild, false, MyMoneyMoney(100.0));
    budget += BudgetEntryHelper(QDate(2006, 1, 1), acSecondChild, true, MyMoneyMoney(100.0));
    MyMoneyReport report(MyMoneyReport::Row::BudgetActual,
                         MyMoneyReport::Column::Months,
                         MyMoneyTransactionFilter::yearToDate,
                         MyMoneyReport::DetailLevel::Top,
                         "Yearly Budgeted vs. Actual", "Default Report");
    PivotTable table(report);
  }

  //  - Totals for D, D:1, D:2 show up.  D:2:a and D:2:b do not
  //  - D actuals (only) compare against D budget
  //  - Ditto for D:1
  //  - D:2 acutals and children compare against D:2 budget

  // 5. Budget on E, no transactions on E
  {
    BudgetHelper budget;
    budget += BudgetEntryHelper(QDate(2006, 1, 1), acSolo, false, MyMoneyMoney(100.0));
    MyMoneyReport report(MyMoneyReport::Row::BudgetActual,
                         MyMoneyReport::Column::Months,
                         MyMoneyTransactionFilter::yearToDate,
                         MyMoneyReport::DetailLevel::Top,
                         "Yearly Budgeted vs. Actual", "Default Report");
    PivotTable table(report);
  }
}

void PivotTableTest::testHtmlEncoding()
{
  MyMoneyReport filter;
  filter.setRowType(MyMoneyReport::Row::AssetLiability);
  filter.setDateFilter(QDate(2004, 1, 1), QDate(2005, 1, 1).addDays(-1));
  XMLandback(filter);
  PivotTable networth_f(filter);

  QByteArray encoding = KGlobal::locale()->encoding();

  QString html = networth_f.renderHTML(0, encoding, filter.name(), false);

  QRegExp rx("*<meta * charset=" + encoding + "*>*");
  rx.setPatternSyntax(QRegExp::Wildcard);
  rx.setCaseSensitivity(Qt::CaseInsensitive);
  QVERIFY(rx.exactMatch(html));
}
