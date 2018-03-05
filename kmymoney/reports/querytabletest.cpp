/***************************************************************************
                          querytabletest.cpp
                          -------------------
    copyright            : (C) 2002 by Thomas Baumgart
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

#include "querytabletest.h"

#include <QFile>

#include <kaboutdata.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kdebug.h>
#include <kdeversion.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <qtest_kde.h>

#include "reportstestcommon.h"
#include "querytable.h"
#include "mymoneyaccount.h"
#include "mymoneysecurity.h"
#include "mymoneyprice.h"
#include "mymoneystoragedump.h"
#include "mymoneyreport.h"
#include "mymoneystatement.h"
#include "mymoneystoragexml.h"

using namespace reports;
using namespace test;

QTEST_KDEMAIN_CORE_WITH_COMPONENTNAME(QueryTableTest, "kmymoney")

void QueryTableTest::init()
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
  acTax = makeAccount(QString("Tax"), MyMoneyAccount::Expense, MyMoneyMoney(), QDate(2005, 1, 11), acExpense, "", true);

  MyMoneyInstitution i("Bank of the World", "", "", "", "", "", "");
  file->addInstitution(i);
  inBank = i.id();
  ft.commit();
}

void QueryTableTest::cleanup()
{
  file->detachStorage(storage);
  delete storage;
}

void QueryTableTest::testQueryBasics()
{
  try {
    TransactionHelper t1q1(QDate(2004, 1, 1), MyMoneySplit::ActionWithdrawal, moSolo, acChecking, acSolo);
    TransactionHelper t2q1(QDate(2004, 2, 1), MyMoneySplit::ActionWithdrawal, moParent1, acCredit, acParent);
    TransactionHelper t3q1(QDate(2004, 3, 1), MyMoneySplit::ActionWithdrawal, moParent2, acCredit, acParent);
    TransactionHelper t4y1(QDate(2004, 11, 7), MyMoneySplit::ActionWithdrawal, moChild, acCredit, acChild);

    TransactionHelper t1q2(QDate(2004, 4, 1), MyMoneySplit::ActionWithdrawal, moSolo, acChecking, acSolo);
    TransactionHelper t2q2(QDate(2004, 5, 1), MyMoneySplit::ActionWithdrawal, moParent1, acCredit, acParent);
    TransactionHelper t3q2(QDate(2004, 6, 1), MyMoneySplit::ActionWithdrawal, moParent2, acCredit, acParent);
    TransactionHelper t4q2(QDate(2004, 11, 7), MyMoneySplit::ActionWithdrawal, moChild, acCredit, acChild);

    TransactionHelper t1y2(QDate(2005, 1, 1), MyMoneySplit::ActionWithdrawal, moSolo, acChecking, acSolo);
    TransactionHelper t2y2(QDate(2005, 5, 1), MyMoneySplit::ActionWithdrawal, moParent1, acCredit, acParent);
    TransactionHelper t3y2(QDate(2005, 9, 1), MyMoneySplit::ActionWithdrawal, moParent2, acCredit, acParent);
    TransactionHelper t4y2(QDate(2004, 11, 7), MyMoneySplit::ActionWithdrawal, moChild, acCredit, acChild);

    unsigned cols;

    MyMoneyReport filter;
    filter.setRowType(MyMoneyReport::eCategory);
    cols = MyMoneyReport::QueryColumns::Number | MyMoneyReport::QueryColumns::Payee | MyMoneyReport::QueryColumns::Account;
    filter.setQueryColumns(static_cast<MyMoneyReport::QueryColumns::Type>(cols));   //
    filter.setName("Transactions by Category");
    XMLandback(filter);
    QueryTable qtbl_1(filter);

    writeTabletoHTML(qtbl_1, "Transactions by Category.html");

    QList<ListTable::TableRow> rows = qtbl_1.rows();

    QVERIFY(rows.count() == 12);
    QVERIFY(rows[0]["categorytype"] == "Expense");
    QVERIFY(rows[0]["category"] == "Parent");
    QVERIFY(rows[0]["postdate"] == "2004-02-01");
    QVERIFY(rows[11]["categorytype"] == "Expense");
    QVERIFY(rows[11]["category"] == "Solo");
    QVERIFY(rows[11]["postdate"] == "2005-01-01");

    QString html = qtbl_1.renderBody();
    QVERIFY(searchHTML(html, i18nc("Total balance", "Total") + " Parent") == -(moParent1 + moParent2) * 3);
    QVERIFY(searchHTML(html, i18nc("Total balance", "Total") + " Parent: Child") == -(moChild) * 3);
    QVERIFY(searchHTML(html, i18nc("Total balance", "Total") + " Solo") == -(moSolo) * 3);
    QVERIFY(searchHTML(html, i18nc("Total balance", "Total") + " Expense") == -(moParent1 + moParent2 + moSolo + moChild) * 3);
    QVERIFY(searchHTML(html, i18nc("Grand total balance", "Grand Total")) == -(moParent1 + moParent2 + moSolo + moChild) * 3 + moCheckingOpen + moCreditOpen);
    filter.setRowType(MyMoneyReport::eTopCategory);
    cols = MyMoneyReport::QueryColumns::Number | MyMoneyReport::QueryColumns::Payee | MyMoneyReport::QueryColumns::Account;
    filter.setQueryColumns(static_cast<MyMoneyReport::QueryColumns::Type>(cols));   //
    filter.setName("Transactions by Top Category");
    XMLandback(filter);
    QueryTable qtbl_2(filter);

    writeTabletoHTML(qtbl_2, "Transactions by Top Category.html");

    rows = qtbl_2.rows();

    QVERIFY(rows.count() == 12);
    QVERIFY(rows[0]["categorytype"] == "Expense");
    QVERIFY(rows[0]["topcategory"] == "Parent");
    QVERIFY(rows[0]["postdate"] == "2004-02-01");
    QVERIFY(rows[8]["categorytype"] == "Expense");
    QVERIFY(rows[8]["topcategory"] == "Parent");
    QVERIFY(rows[8]["postdate"] == "2005-09-01");
    QVERIFY(rows[11]["categorytype"] == "Expense");
    QVERIFY(rows[11]["topcategory"] == "Solo");
    QVERIFY(rows[11]["postdate"] == "2005-01-01");

    html = qtbl_2.renderBody();
    QVERIFY(searchHTML(html, i18nc("Total balance", "Total") + " Parent") == -(moParent1 + moParent2 + moChild) * 3);
    QVERIFY(searchHTML(html, i18nc("Total balance", "Total") + " Solo") == -(moSolo) * 3);
    QVERIFY(searchHTML(html, i18nc("Total balance", "Total") + " Expense") == -(moParent1 + moParent2 + moSolo + moChild) * 3);
    QVERIFY(searchHTML(html, i18nc("Grand total balance", "Grand Total")) == -(moParent1 + moParent2 + moSolo + moChild) * 3 + moCheckingOpen + moCreditOpen);

    filter.setRowType(MyMoneyReport::eAccount);
    filter.setName("Transactions by Account");
    cols = MyMoneyReport::QueryColumns::Number | MyMoneyReport::QueryColumns::Payee | MyMoneyReport::QueryColumns::Category;
    filter.setQueryColumns(static_cast<MyMoneyReport::QueryColumns::Type>(cols));   //
    XMLandback(filter);
    QueryTable qtbl_3(filter);

    writeTabletoHTML(qtbl_3, "Transactions by Account.html");

    rows = qtbl_3.rows();

#if 1
    QVERIFY(rows.count() == 16);
    QVERIFY(rows[1]["account"] == "Checking Account");
    QVERIFY(rows[1]["category"] == "Solo");
    QVERIFY(rows[1]["postdate"] == "2004-01-01");
    QVERIFY(rows[14]["account"] == "Credit Card");
    QVERIFY(rows[14]["category"] == "Parent");
    QVERIFY(rows[14]["postdate"] == "2005-09-01");
#else
    QVERIFY(rows.count() == 12);
    QVERIFY(rows[0]["account"] == "Checking Account");
    QVERIFY(rows[0]["category"] == "Solo");
    QVERIFY(rows[0]["postdate"] == "2004-01-01");
    QVERIFY(rows[11]["account"] == "Credit Card");
    QVERIFY(rows[11]["category"] == "Parent");
    QVERIFY(rows[11]["postdate"] == "2005-09-01");
#endif

    html = qtbl_3.renderBody();
    QVERIFY(searchHTML(html, i18nc("Total balance for checking account", "Total") + " Checking Account") == -(moSolo) * 3 + moCheckingOpen);
    QVERIFY(searchHTML(html, i18nc("Total balance for credit card", "Total") + " Credit Card") == -(moParent1 + moParent2 + moChild) * 3 + moCreditOpen);
    QVERIFY(searchHTML(html, i18nc("Grand total balance", "Grand Total")) == -(moParent1 + moParent2 + moSolo + moChild) * 3 + moCheckingOpen + moCreditOpen);

    filter.setRowType(MyMoneyReport::ePayee);
    filter.setName("Transactions by Payee");
    cols = MyMoneyReport::QueryColumns::Number | MyMoneyReport::QueryColumns::Memo | MyMoneyReport::QueryColumns::Category;
    filter.setQueryColumns(static_cast<MyMoneyReport::QueryColumns::Type>(cols));   //
    XMLandback(filter);
    QueryTable qtbl_4(filter);

    writeTabletoHTML(qtbl_4, "Transactions by Payee.html");

    rows = qtbl_4.rows();

    QVERIFY(rows.count() == 12);
    QVERIFY(rows[0]["payee"] == "Test Payee");
    QVERIFY(rows[0]["category"] == "Solo");
    QVERIFY(rows[0]["postdate"] == "2004-01-01");
    QVERIFY(rows[8]["payee"] == "Test Payee");
    QVERIFY(rows[8]["category"] == "Parent: Child");
    QVERIFY(rows[8]["postdate"] == "2004-11-07");
    QVERIFY(rows[11]["payee"] == "Test Payee");
    QVERIFY(rows[11]["category"] == "Parent");
    QVERIFY(rows[11]["postdate"] == "2005-09-01");

    html = qtbl_4.renderBody();
    QVERIFY(searchHTML(html, i18nc("Total balance", "Total") + " Test Payee") == -(moParent1 + moParent2 + moSolo + moChild) * 3);
    QVERIFY(searchHTML(html, i18nc("Grand total balance", "Grand Total")) == -(moParent1 + moParent2 + moSolo + moChild) * 3 + moCheckingOpen + moCreditOpen);

    filter.setRowType(MyMoneyReport::eMonth);
    filter.setName("Transactions by Month");
    cols = MyMoneyReport::QueryColumns::Number | MyMoneyReport::QueryColumns::Payee | MyMoneyReport::QueryColumns::Category;
    filter.setQueryColumns(static_cast<MyMoneyReport::QueryColumns::Type>(cols));   //
    XMLandback(filter);
    QueryTable qtbl_5(filter);

    writeTabletoHTML(qtbl_5, "Transactions by Month.html");

    rows = qtbl_5.rows();

    QVERIFY(rows.count() == 12);
    QVERIFY(rows[0]["payee"] == "Test Payee");
    QVERIFY(rows[0]["category"] == "Solo");
    QVERIFY(rows[0]["postdate"] == "2004-01-01");
    QVERIFY(rows[8]["payee"] == "Test Payee");
    QVERIFY(rows[8]["category"] == "Parent: Child");
    QVERIFY(rows[8]["postdate"] == "2004-11-07");
    QVERIFY(rows[11]["payee"] == "Test Payee");
    QVERIFY(rows[11]["category"] == "Parent");
    QVERIFY(rows[11]["postdate"] == "2005-09-01");

    html = qtbl_5.renderBody();
    QVERIFY(searchHTML(html, i18nc("Total balance", "Total") + " Month of 2004-01-01") == -moSolo);
    QVERIFY(searchHTML(html, i18nc("Total balance", "Total") + " Month of 2004-11-01") == -(moChild) * 3);
    QVERIFY(searchHTML(html, i18nc("Total balance", "Total") + " Month of 2004-05-01") == -moParent1 + moCheckingOpen);
    QVERIFY(searchHTML(html, i18nc("Grand total balance", "Grand Total")) == -(moParent1 + moParent2 + moSolo + moChild) * 3 + moCheckingOpen + moCreditOpen);

    filter.setRowType(MyMoneyReport::eWeek);
    filter.setName("Transactions by Week");
    cols = MyMoneyReport::QueryColumns::Number | MyMoneyReport::QueryColumns::Payee | MyMoneyReport::QueryColumns::Category;
    filter.setQueryColumns(static_cast<MyMoneyReport::QueryColumns::Type>(cols));   //
    XMLandback(filter);
    QueryTable qtbl_6(filter);

    writeTabletoHTML(qtbl_6, "Transactions by Week.html");

    rows = qtbl_6.rows();

    QVERIFY(rows.count() == 12);
    QVERIFY(rows[0]["payee"] == "Test Payee");
    QVERIFY(rows[0]["category"] == "Solo");
    QVERIFY(rows[0]["postdate"] == "2004-01-01");
    QVERIFY(rows[11]["payee"] == "Test Payee");
    QVERIFY(rows[11]["category"] == "Parent");
    QVERIFY(rows[11]["postdate"] == "2005-09-01");

    html = qtbl_6.renderBody();
    QVERIFY(searchHTML(html, i18nc("Total balance", "Total") + " Week of 2003-12-29") == -moSolo);
    QVERIFY(searchHTML(html, i18nc("Total balance", "Total") + " Week of 2004-11-01") == -(moChild) * 3);
    QVERIFY(searchHTML(html, i18nc("Total balance", "Total") + " Week of 2005-08-29") == -moParent2);
    QVERIFY(searchHTML(html, i18nc("Grand total balance", "Grand Total")) == -(moParent1 + moParent2 + moSolo + moChild) * 3 + moCheckingOpen + moCreditOpen);
  } catch (const MyMoneyException &e) {
    QFAIL(qPrintable(e.what()));
  }

  // Test querytable::TableRow::operator> and operator==

  QueryTable::TableRow low;
  low["first"] = 'A';
  low["second"] = 'B';
  low["third"] = 'C';

  QueryTable::TableRow high;
  high["first"] = 'A';
  high["second"] = 'C';
  high["third"] = 'B';

  QueryTable::TableRow::setSortCriteria("first,second,third");
  QVERIFY(low < high);
  QVERIFY(low <= high);
  QVERIFY(high > low);
  QVERIFY(high <= high);
  QVERIFY(high == high);
}

void QueryTableTest::testCashFlowAnalysis()
{
  //
  // Test IRR calculations
  //

  CashFlowList list;

  list += CashFlowListItem(QDate(2004, 5, 3), MyMoneyMoney(1000.0));
  list += CashFlowListItem(QDate(2004, 5, 20), MyMoneyMoney(59.0));
  list += CashFlowListItem(QDate(2004, 6, 3), MyMoneyMoney(14.0));
  list += CashFlowListItem(QDate(2004, 6, 24), MyMoneyMoney(92.0));
  list += CashFlowListItem(QDate(2004, 7, 6), MyMoneyMoney(63.0));
  list += CashFlowListItem(QDate(2004, 7, 25), MyMoneyMoney(15.0));
  list += CashFlowListItem(QDate(2004, 8, 5), MyMoneyMoney(92.0));
  list += CashFlowListItem(QDate(2004, 9, 2), MyMoneyMoney(18.0));
  list += CashFlowListItem(QDate(2004, 9, 21), MyMoneyMoney(5.0));
  list += CashFlowListItem(QDate(2004, 10, 16), MyMoneyMoney(-2037.0));

  MyMoneyMoney IRR(list.IRR(), 1000);

  QVERIFY(IRR == MyMoneyMoney(1676, 1000));

  list.pop_back();
  list += CashFlowListItem(QDate(2004, 10, 16), MyMoneyMoney(-1358.0));

  IRR = MyMoneyMoney(list.IRR(), 1000);

  QVERIFY(IRR.isZero());
}

void QueryTableTest::testAccountQuery()
{
  try {
    QString htmlcontext = QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\">\n<html><head><link rel=\"stylesheet\" type=\"text/css\" href=\"html/kmymoney.css\"></head><body>\n%1\n</body></html>\n");

    //
    // No transactions, opening balances only
    //

    MyMoneyReport filter;
    filter.setRowType(MyMoneyReport::eInstitution);
    filter.setName("Accounts by Institution (No transactions)");
    XMLandback(filter);
    QueryTable qtbl_1(filter);

    writeTabletoHTML(qtbl_1, "Accounts by Institution (No transactions).html");

    QList<ListTable::TableRow> rows = qtbl_1.rows();

    QVERIFY(rows.count() == 2);
    QVERIFY(rows[0]["account"] == "Checking Account");
    QVERIFY(MyMoneyMoney(rows[0]["value"]) == moCheckingOpen);
    QVERIFY(rows[0]["equitytype"].isEmpty());
    QVERIFY(rows[1]["account"] == "Credit Card");
    QVERIFY(MyMoneyMoney(rows[1]["value"]) == moCreditOpen);
    QVERIFY(rows[1]["equitytype"].isEmpty());

    QString html = qtbl_1.renderBody();
    QVERIFY(searchHTML(html, i18nc("Total balance", "Total") + " None") == moCheckingOpen + moCreditOpen);
    QVERIFY(searchHTML(html, i18nc("Grand total balance", "Grand Total")) == moCheckingOpen + moCreditOpen);

    //
    // Adding in transactions
    //

    TransactionHelper t1q1(QDate(2004, 1, 1), MyMoneySplit::ActionWithdrawal, moSolo, acChecking, acSolo);
    TransactionHelper t2q1(QDate(2004, 2, 1), MyMoneySplit::ActionWithdrawal, moParent1, acCredit, acParent);
    TransactionHelper t3q1(QDate(2004, 3, 1), MyMoneySplit::ActionWithdrawal, moParent2, acCredit, acParent);
    TransactionHelper t4y1(QDate(2004, 11, 7), MyMoneySplit::ActionWithdrawal, moChild, acCredit, acChild);

    TransactionHelper t1q2(QDate(2004, 4, 1), MyMoneySplit::ActionWithdrawal, moSolo, acChecking, acSolo);
    TransactionHelper t2q2(QDate(2004, 5, 1), MyMoneySplit::ActionWithdrawal, moParent1, acCredit, acParent);
    TransactionHelper t3q2(QDate(2004, 6, 1), MyMoneySplit::ActionWithdrawal, moParent2, acCredit, acParent);
    TransactionHelper t4q2(QDate(2004, 11, 7), MyMoneySplit::ActionWithdrawal, moChild, acCredit, acChild);

    TransactionHelper t1y2(QDate(2005, 1, 1), MyMoneySplit::ActionWithdrawal, moSolo, acChecking, acSolo);
    TransactionHelper t2y2(QDate(2005, 5, 1), MyMoneySplit::ActionWithdrawal, moParent1, acCredit, acParent);
    TransactionHelper t3y2(QDate(2005, 9, 1), MyMoneySplit::ActionWithdrawal, moParent2, acCredit, acParent);
    TransactionHelper t4y2(QDate(2004, 11, 7), MyMoneySplit::ActionWithdrawal, moChild, acCredit, acChild);

    filter.setRowType(MyMoneyReport::eInstitution);
    filter.setName("Accounts by Institution (With Transactions)");
    XMLandback(filter);
    QueryTable qtbl_2(filter);

    rows = qtbl_2.rows();

    QVERIFY(rows.count() == 2);
    QVERIFY(rows[0]["account"] == "Checking Account");
    QVERIFY(MyMoneyMoney(rows[0]["value"]) == (moCheckingOpen - moSolo*3));
    QVERIFY(rows[1]["account"] == "Credit Card");
    QVERIFY(MyMoneyMoney(rows[1]["value"]) == (moCreditOpen - (moParent1 + moParent2 + moChild) * 3));

    html = qtbl_2.renderBody();
    QVERIFY(searchHTML(html, i18n("Grand Total")) == moCheckingOpen + moCreditOpen - (moParent1 + moParent2 + moSolo + moChild) * 3);

    //
    // Account TYPES
    //

    filter.setRowType(MyMoneyReport::eAccountType);
    filter.setName("Accounts by Type");
    XMLandback(filter);
    QueryTable qtbl_3(filter);

    rows = qtbl_3.rows();

    QVERIFY(rows.count() == 2);
    QVERIFY(rows[0]["account"] == "Checking Account");
    QVERIFY(MyMoneyMoney(rows[0]["value"]) == (moCheckingOpen - moSolo*3));
    QVERIFY(rows[1]["account"] == "Credit Card");
    QVERIFY(MyMoneyMoney(rows[1]["value"]) == (moCreditOpen - (moParent1 + moParent2 + moChild) * 3));

    html = qtbl_3.renderBody();
    QVERIFY(searchHTML(html, i18nc("Total balance", "Total") + ' ' + i18n("Checking")) == moCheckingOpen - moSolo*3);
    QVERIFY(searchHTML(html, i18nc("Total balance", "Total") + ' ' + i18n("Credit Card")) == moCreditOpen - (moParent1 + moParent2 + moChild) * 3);
    QVERIFY(searchHTML(html, i18nc("Grand total balance", "Grand Total")) == moCheckingOpen + moCreditOpen - (moParent1 + moParent2 + moSolo + moChild) * 3);
  } catch (const MyMoneyException &e) {
    QFAIL(qPrintable(e.what()));
  }
}

void QueryTableTest::testInvestment()
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
    acInterest = makeAccount("Interest", MyMoneyAccount::Income, moZero, QDate(2004, 1, 1), acIncome);

    // Transactions
    //                         Date             Action                                               Shares                Price   Stock     Asset       Income
    InvTransactionHelper s1b1(QDate(2004, 2, 1), MyMoneySplit::ActionBuyShares,        MyMoneyMoney(1000.00), MyMoneyMoney(100.00), acStock1, acChecking, QString());
    InvTransactionHelper s1b2(QDate(2004, 3, 1), MyMoneySplit::ActionBuyShares,        MyMoneyMoney(1000.00), MyMoneyMoney(110.00), acStock1, acChecking, QString());
    InvTransactionHelper s1s1(QDate(2004, 4, 1), MyMoneySplit::ActionBuyShares,        MyMoneyMoney(-200.00), MyMoneyMoney(120.00), acStock1, acChecking, QString());
    InvTransactionHelper s1s2(QDate(2004, 5, 1), MyMoneySplit::ActionBuyShares,        MyMoneyMoney(-200.00), MyMoneyMoney(100.00), acStock1, acChecking, QString());
    InvTransactionHelper s1r1(QDate(2004, 6, 1), MyMoneySplit::ActionReinvestDividend, MyMoneyMoney(50.00), MyMoneyMoney(100.00), acStock1, QString(), acDividends);
    InvTransactionHelper s1r2(QDate(2004, 7, 1), MyMoneySplit::ActionReinvestDividend, MyMoneyMoney(50.00), MyMoneyMoney(80.00), acStock1, QString(), acDividends);
    InvTransactionHelper s1c1(QDate(2004, 8, 1), MyMoneySplit::ActionDividend,         MyMoneyMoney(10.00), MyMoneyMoney(100.00), acStock1, acChecking, acDividends);
    InvTransactionHelper s1c2(QDate(2004, 9, 1), MyMoneySplit::ActionDividend,         MyMoneyMoney(10.00), MyMoneyMoney(120.00), acStock1, acChecking, acDividends);
    InvTransactionHelper s1y1(QDate(2004, 9, 15), MyMoneySplit::ActionYield,           MyMoneyMoney(10.00), MyMoneyMoney(110.00), acStock1, acChecking, acInterest);

    makeEquityPrice(eqStock1, QDate(2004, 10, 1), MyMoneyMoney(100.00));

    //
    // Investment Transactions Report
    //

    MyMoneyReport invtran_r(
      MyMoneyReport::eTopAccount,
      MyMoneyReport::QueryColumns::Action | MyMoneyReport::QueryColumns::Shares | MyMoneyReport::QueryColumns::Price,
      MyMoneyTransactionFilter::userDefined,
      MyMoneyReport::eDetailAll,
      i18n("Investment Transactions"),
      i18n("Test Report")
    );
    invtran_r.setDateFilter(QDate(2004, 1, 1), QDate(2004, 12, 31));
    invtran_r.setInvestmentsOnly(true);
    XMLandback(invtran_r);
    QueryTable invtran(invtran_r);

#if 1
    writeTabletoHTML(invtran, "investment_transactions_test.html");

    QList<ListTable::TableRow> rows = invtran.rows();

    QVERIFY(rows.count() == 17);
    QVERIFY(MyMoneyMoney(rows[1]["value"]) == MyMoneyMoney(100000.00));
    QVERIFY(MyMoneyMoney(rows[2]["value"]) == MyMoneyMoney(110000.00));
    QVERIFY(MyMoneyMoney(rows[3]["value"]) == MyMoneyMoney(-24000.00));
    QVERIFY(MyMoneyMoney(rows[4]["value"]) == MyMoneyMoney(-20000.00));
    QVERIFY(MyMoneyMoney(rows[5]["value"]) == MyMoneyMoney(5000.00));
    QVERIFY(MyMoneyMoney(rows[6]["value"]) == MyMoneyMoney(4000.00));
    // need to fix these... fundamentally different from the original test
    //QVERIFY(MyMoneyMoney(invtran.m_rows[8]["value"])==MyMoneyMoney( -1000.00));
    //QVERIFY(MyMoneyMoney(invtran.m_rows[11]["value"])==MyMoneyMoney( -1200.00));
    //QVERIFY(MyMoneyMoney(invtran.m_rows[14]["value"])==MyMoneyMoney( -1100.00));

    QVERIFY(MyMoneyMoney(rows[1]["price"]) == MyMoneyMoney(100.00));
    QVERIFY(MyMoneyMoney(rows[3]["price"]) == MyMoneyMoney(120.00));
    QVERIFY(MyMoneyMoney(rows[5]["price"]) == MyMoneyMoney(100.00));
    QVERIFY(MyMoneyMoney(rows[7]["price"]) == MyMoneyMoney(100.00));
    QVERIFY(MyMoneyMoney(rows[10]["price"]) == MyMoneyMoney(120.00));

    QVERIFY(MyMoneyMoney(rows[2]["shares"]) == MyMoneyMoney(1000.00));
    QVERIFY(MyMoneyMoney(rows[4]["shares"]) == MyMoneyMoney(-200.00));
    QVERIFY(MyMoneyMoney(rows[6]["shares"]) == MyMoneyMoney(50.00));
    QVERIFY(MyMoneyMoney(rows[8]["shares"]) == MyMoneyMoney(0.00));
    QVERIFY(MyMoneyMoney(rows[11]["shares"]) == MyMoneyMoney(0.00));

    QVERIFY(rows[1]["action"] == "Buy");
    QVERIFY(rows[3]["action"] == "Sell");
    QVERIFY(rows[5]["action"] == "Reinvest");
    QVERIFY(rows[7]["action"] == "Dividend");
    QVERIFY(rows[13]["action"] == "Yield");
#else
    QVERIFY(rows.count() == 9);
    QVERIFY(MyMoneyMoney(rows[0]["value"]) == MyMoneyMoney(100000.00));
    QVERIFY(MyMoneyMoney(rows[1]["value"]) == MyMoneyMoney(110000.00));
    QVERIFY(MyMoneyMoney(rows[2]["value"]) == MyMoneyMoney(-24000.00));
    QVERIFY(MyMoneyMoney(rows[3]["value"]) == MyMoneyMoney(-20000.00));
    QVERIFY(MyMoneyMoney(rows[4]["value"]) == MyMoneyMoney(5000.00));
    QVERIFY(MyMoneyMoney(rows[5]["value"]) == MyMoneyMoney(4000.00));
    QVERIFY(MyMoneyMoney(rows[6]["value"]) == MyMoneyMoney(-1000.00));
    QVERIFY(MyMoneyMoney(rows[7]["value"]) == MyMoneyMoney(-1200.00));
    QVERIFY(MyMoneyMoney(rows[8]["value"]) == MyMoneyMoney(-1100.00));

    QVERIFY(MyMoneyMoney(rows[0]["price"]) == MyMoneyMoney(100.00));
    QVERIFY(MyMoneyMoney(rows[2]["price"]) == MyMoneyMoney(120.00));
    QVERIFY(MyMoneyMoney(rows[4]["price"]) == MyMoneyMoney(100.00));
    QVERIFY(MyMoneyMoney(rows[6]["price"]) == MyMoneyMoney(0.00));
    QVERIFY(MyMoneyMoney(rows[8]["price"]) == MyMoneyMoney(0.00));

    QVERIFY(MyMoneyMoney(rows[1]["shares"]) == MyMoneyMoney(1000.00));
    QVERIFY(MyMoneyMoney(rows[3]["shares"]) == MyMoneyMoney(-200.00));
    QVERIFY(MyMoneyMoney(rows[5]["shares"]) == MyMoneyMoney(50.00));
    QVERIFY(MyMoneyMoney(rows[7]["shares"]) == MyMoneyMoney(0.00));
    QVERIFY(MyMoneyMoney(rows[8]["shares"]) == MyMoneyMoney(0.00));

    QVERIFY(rows[0]["action"] == "Buy");
    QVERIFY(rows[2]["action"] == "Sell");
    QVERIFY(rows[4]["action"] == "Reinvest");
    QVERIFY(rows[6]["action"] == "Dividend");
    QVERIFY(rows[8]["action"] == "Yield");
#endif

    QString html = invtran.renderBody();
#if 1
    // i think this is the correct amount. different treatment of dividend and yield
    QVERIFY(searchHTML(html, i18n("Total Stock 1")) == MyMoneyMoney(175000.00));
    QVERIFY(searchHTML(html, i18n("Grand Total")) == MyMoneyMoney(175000.00));
#else
    QVERIFY(searchHTML(html, i18n("Total Stock 1")) == MyMoneyMoney(171700.00));
    QVERIFY(searchHTML(html, i18n("Grand Total")) == MyMoneyMoney(171700.00));
#endif

    //
    // Investment Performance Report
    //

    MyMoneyReport invhold_r(
      MyMoneyReport::eAccountByTopAccount,
      MyMoneyReport::QueryColumns::Performance,
      MyMoneyTransactionFilter::userDefined,
      MyMoneyReport::eDetailAll,
      i18n("Investment Performance by Account"),
      i18n("Test Report")
    );
    invhold_r.setDateFilter(QDate(2004, 1, 1), QDate(2004, 10, 1));
    invhold_r.setInvestmentsOnly(true);
    XMLandback(invhold_r);
    QueryTable invhold(invhold_r);

    writeTabletoHTML(invhold, "Investment Performance by Account.html");

    rows = invhold.rows();

    QVERIFY(rows.count() == 1);
    QVERIFY(MyMoneyMoney(rows[0]["return"]) == MyMoneyMoney("669/10000"));
    QVERIFY(MyMoneyMoney(rows[0]["buys"]) == MyMoneyMoney(210000.00));
    QVERIFY(MyMoneyMoney(rows[0]["sells"]) == MyMoneyMoney(-44000.00));
    QVERIFY(MyMoneyMoney(rows[0]["reinvestincome"]) == MyMoneyMoney(9000.00));
    QVERIFY(MyMoneyMoney(rows[0]["cashincome"]) == MyMoneyMoney(3300.00));
    QVERIFY(MyMoneyMoney(rows[0]["shares"]) == MyMoneyMoney(1700.00));
    QVERIFY(MyMoneyMoney(rows[0]["price"]) == MyMoneyMoney(100.00));

    html = invhold.renderBody();
    QVERIFY(searchHTML(html, i18n("Grand Total")) == MyMoneyMoney(170000.00));

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
//this is to prevent me from making mistakes again when modifying balances - asoliverez
//this case tests only the opening and ending balance of the accounts
void QueryTableTest::testBalanceColumn()
{
  try {
    TransactionHelper t1q1(QDate(2004, 1, 1), MyMoneySplit::ActionWithdrawal, moSolo, acChecking, acSolo);
    TransactionHelper t2q1(QDate(2004, 2, 1), MyMoneySplit::ActionWithdrawal, moParent1, acCredit, acParent);
    TransactionHelper t3q1(QDate(2004, 3, 1), MyMoneySplit::ActionWithdrawal, moParent2, acCredit, acParent);
    TransactionHelper t4y1(QDate(2004, 11, 7), MyMoneySplit::ActionWithdrawal, moChild, acCredit, acChild);

    TransactionHelper t1q2(QDate(2004, 4, 1), MyMoneySplit::ActionWithdrawal, moSolo, acChecking, acSolo);
    TransactionHelper t2q2(QDate(2004, 5, 1), MyMoneySplit::ActionWithdrawal, moParent1, acCredit, acParent);
    TransactionHelper t3q2(QDate(2004, 6, 1), MyMoneySplit::ActionWithdrawal, moParent2, acCredit, acParent);
    TransactionHelper t4q2(QDate(2004, 11, 7), MyMoneySplit::ActionWithdrawal, moChild, acCredit, acChild);

    TransactionHelper t1y2(QDate(2005, 1, 1), MyMoneySplit::ActionWithdrawal, moSolo, acChecking, acSolo);
    TransactionHelper t2y2(QDate(2005, 5, 1), MyMoneySplit::ActionWithdrawal, moParent1, acCredit, acParent);
    TransactionHelper t3y2(QDate(2005, 9, 1), MyMoneySplit::ActionWithdrawal, moParent2, acCredit, acParent);
    TransactionHelper t4y2(QDate(2004, 11, 7), MyMoneySplit::ActionWithdrawal, moChild, acCredit, acChild);

    unsigned cols;

    MyMoneyReport filter;

    filter.setRowType(MyMoneyReport::eAccount);
    filter.setName("Transactions by Account");
    cols = MyMoneyReport::QueryColumns::Number | MyMoneyReport::QueryColumns::Payee | MyMoneyReport::QueryColumns::Category | MyMoneyReport::QueryColumns::Balance;
    filter.setQueryColumns(static_cast<MyMoneyReport::QueryColumns::Type>(cols));   //
    XMLandback(filter);
    QueryTable qtbl_3(filter);

    writeTabletoHTML(qtbl_3, "Transactions by Account.html");

    QString html = qtbl_3.renderBody();

    QList<ListTable::TableRow> rows = qtbl_3.rows();

    QVERIFY(rows.count() == 16);

    //this is to make sure that the dates of closing and opening balances and the balance numbers are ok
    QString openingDate = KGlobal::locale()->formatDate(QDate(2004, 1, 1), KLocale::ShortDate);
    QString closingDate = KGlobal::locale()->formatDate(QDate(2005, 9, 1), KLocale::ShortDate);
    QVERIFY(html.indexOf(openingDate + "</td><td class=\"left\"></td><td class=\"left\">" + i18n("Opening Balance")) > 0);
    QVERIFY(html.indexOf(closingDate + "</td><td class=\"left\"></td><td class=\"left\">" + i18n("Closing Balance") + "</td><td class=\"left\"></td><td class=\"value\"></td><td>&nbsp;-702.36</td></tr>") > 0);
    QVERIFY(html.indexOf(closingDate + "</td><td class=\"left\"></td><td class=\"left\">" + i18n("Closing Balance") + "</td><td class=\"left\"></td><td class=\"value\"></td><td>&nbsp;-705.69</td></tr>") > 0);

  } catch (const MyMoneyException &e) {
    QFAIL(qPrintable(e.what()));
  }

}

void QueryTableTest::testBalanceColumnWithMultipleCurrencies()
{
  try {

    MyMoneyMoney moJpyOpening(0.0, 1);
    MyMoneyMoney moJpyPrice(0.010, 100);
    MyMoneyMoney moJpyPrice2(0.011, 100);
    MyMoneyMoney moJpyPrice3(0.024, 100);
    MyMoneyMoney moTransaction(100, 1);
    MyMoneyMoney moJpyTransaction(100, 1);

    QString acJpyChecking = makeAccount(QString("Japanese Checking"), MyMoneyAccount::Checkings, moJpyOpening, QDate(2003, 11, 15), acAsset, "JPY");

    makePrice("JPY", QDate(2004, 1, 1), MyMoneyMoney(moJpyPrice));
    makePrice("JPY", QDate(2004, 5, 1), MyMoneyMoney(moJpyPrice2));
    makePrice("JPY", QDate(2004, 6, 30), MyMoneyMoney(moJpyPrice3));

    QDate openingDate(2004, 2, 20);
    QDate intermediateDate(2004, 5, 20);
    QDate closingDate(2004, 7, 20);

    TransactionHelper t1(openingDate,      MyMoneySplit::ActionTransfer,   MyMoneyMoney(moJpyTransaction), acJpyChecking, acChecking, "JPY");
    TransactionHelper t4(openingDate,      MyMoneySplit::ActionDeposit,    MyMoneyMoney(moTransaction),    acCredit,      acChecking);
    TransactionHelper t2(intermediateDate, MyMoneySplit::ActionTransfer,   MyMoneyMoney(moJpyTransaction), acJpyChecking, acChecking, "JPY");
    TransactionHelper t5(intermediateDate, MyMoneySplit::ActionDeposit,    MyMoneyMoney(moTransaction),    acCredit,      acChecking);
    TransactionHelper t3(closingDate,      MyMoneySplit::ActionTransfer,   MyMoneyMoney(moJpyTransaction), acJpyChecking, acChecking, "JPY");
    TransactionHelper t6(closingDate,      MyMoneySplit::ActionDeposit,    MyMoneyMoney(moTransaction),    acCredit,      acChecking);
    // test that an income/expense transaction that involves a currency exchange is properly reported
    TransactionHelper t7(intermediateDate, MyMoneySplit::ActionWithdrawal, MyMoneyMoney(moJpyTransaction), acJpyChecking, acSolo, "JPY");

    unsigned cols;

    MyMoneyReport filter;

    filter.setRowType(MyMoneyReport::eAccount);
    filter.setName("Transactions by Account");
    cols = MyMoneyReport::QueryColumns::Number | MyMoneyReport::QueryColumns::Payee | MyMoneyReport::QueryColumns::Category | MyMoneyReport::QueryColumns::Balance;
    filter.setQueryColumns(static_cast<MyMoneyReport::QueryColumns::Type>(cols));
    // don't convert values to the default currency
    filter.setConvertCurrency(false);
    XMLandback(filter);
    QueryTable qtbl_3(filter);

    writeTabletoHTML(qtbl_3, "Transactions by Account (multiple currencies).html");

    QString html = qtbl_3.renderBody();

    QList<ListTable::TableRow> rows = qtbl_3.rows();

    QVERIFY(rows.count() == 19);

    //this is to make sure that the dates of closing and opening balances and the balance numbers are ok
    QString openingDateString = KGlobal::locale()->formatDate(openingDate, KLocale::ShortDate);
    QString intermediateDateString = KGlobal::locale()->formatDate(intermediateDate, KLocale::ShortDate);
    QString closingDateString = KGlobal::locale()->formatDate(closingDate, KLocale::ShortDate);
    // check the opening and closing balances
    QVERIFY(html.indexOf(openingDateString + "</td><td class=\"left\"></td><td class=\"left\">" + i18n("Opening Balance") + "</td><td class=\"left\"></td><td class=\"value\"></td><td>&nbsp;0.00</td></tr>") > 0);
    QVERIFY(html.indexOf(closingDateString + "</td><td class=\"left\"></td><td class=\"left\">" + i18n("Closing Balance") + "</td><td class=\"left\"></td><td class=\"value\"></td><td>&nbsp;304.00</td></tr>") > 0);
    QVERIFY(html.indexOf(closingDateString + "</td><td class=\"left\"></td><td class=\"left\">" + i18n("Closing Balance") + "</td><td class=\"left\"></td><td class=\"value\"></td><td>&nbsp;-300.00</td></tr>") > 0);
    QVERIFY(html.indexOf(closingDateString + "</td><td class=\"left\"></td><td class=\"left\">" + i18n("Closing Balance") + "</td><td class=\"left\"></td><td class=\"value\"></td><td>JPY&nbsp;-400.00</td></tr>") > 0);

    // after a transfer of 100 JPY the balance should be 1.00 - price is 0.010 (precision of 2)
    QVERIFY(html.indexOf("<a href=ledger?id=A000001&tid=T000000000000000001>" + openingDateString + "</a></td><td class=\"left\"></td><td class=\"left\">Test Payee</td><td class=\"left\">Transfer from Japanese Checking</td><td class=\"value\">&nbsp;1.00</td><td>&nbsp;1.00</td></tr>") > 0);

    // after a transfer of 100 the balance should be 101.00
    QVERIFY(html.indexOf("<a href=ledger?id=A000001&tid=T000000000000000002>" + openingDateString + "</a></td><td class=\"left\"></td><td class=\"left\">Test Payee</td><td class=\"left\">Transfer from Credit Card</td><td class=\"value\">&nbsp;100.00</td><td>&nbsp;101.00</td></tr>") > 0);

    // after a transfer of 100 JPY the balance should be 102.00 - price is 0.011 (precision of 2)
    QVERIFY(html.indexOf("<a href=ledger?id=A000001&tid=T000000000000000003>" + intermediateDateString + "</a></td><td class=\"left\"></td><td class=\"left\">Test Payee</td><td class=\"left\">Transfer from Japanese Checking</td><td class=\"value\">&nbsp;1.00</td><td>&nbsp;102.00</td></tr>") > 0);

    // after a transfer of 100 the balance should be 202.00
    QVERIFY(html.indexOf("<a href=ledger?id=A000001&tid=T000000000000000004>" + intermediateDateString + "</a></td><td class=\"left\"></td><td class=\"left\">Test Payee</td><td class=\"left\">Transfer from Credit Card</td><td class=\"value\">&nbsp;100.00</td><td>&nbsp;202.00</td></tr>") > 0);

    // after a transfer of 100 JPY the balance should be 204.00 - price is 0.024 (precision of 2)
    QVERIFY(html.indexOf("<a href=ledger?id=A000001&tid=T000000000000000005>" + closingDateString + "</a></td><td class=\"left\"></td><td class=\"left\">Test Payee</td><td class=\"left\">Transfer from Japanese Checking</td><td class=\"value\">&nbsp;2.00</td><td>&nbsp;204.00</td></tr>") > 0);

    // after a transfer of 100 the balance should be 304.00
    QVERIFY(html.indexOf("<a href=ledger?id=A000001&tid=T000000000000000006>" + closingDateString + "</a></td><td class=\"left\"></td><td class=\"left\">Test Payee</td><td class=\"left\">Transfer from Credit Card</td><td class=\"value\">&nbsp;100.00</td><td>&nbsp;304.00</td></tr>") > 0);

    // a 100.00 JPY withdrawal should be displayed as such even if the expense account uses another currency
    QVERIFY(html.indexOf("<a href=ledger?id=A000008&tid=T000000000000000007>" + intermediateDateString + "</a></td><td class=\"left\"></td><td class=\"left\">Test Payee</td><td class=\"left\">Solo</td><td class=\"value\">JPY&nbsp;-100.00</td><td>JPY&nbsp;-300.00</td></tr>") > 0);

    // now run the same report again but this time convert all values to the base currency and make sure the values are correct
    filter.setConvertCurrency(true);
    XMLandback(filter);
    QueryTable qtbl_4(filter);

    writeTabletoHTML(qtbl_4, "Transactions by Account (multiple currencies converted to base).html");

    html = qtbl_4.renderBody();

    rows = qtbl_4.rows();

    QVERIFY(rows.count() == 19);

    // check the opening and closing balances
    QVERIFY(html.indexOf(openingDateString + "</td><td class=\"left\"></td><td class=\"left\">" + i18n("Opening Balance") + "</td><td class=\"left\"></td><td class=\"value\"></td><td>&nbsp;0.00</td></tr>") > 0);
    QVERIFY(html.indexOf(closingDateString + "</td><td class=\"left\"></td><td class=\"left\">" + i18n("Closing Balance") + "</td><td class=\"left\"></td><td class=\"value\"></td><td>&nbsp;304.00</td></tr>") > 0);
    QVERIFY(html.indexOf(closingDateString + "</td><td class=\"left\"></td><td class=\"left\">" + i18n("Closing Balance") + "</td><td class=\"left\"></td><td class=\"value\"></td><td>&nbsp;-300.00</td></tr>") > 0);
    // although the balance should be -5.00 it's -8.00 because the foreign currency balance is converted using the closing date price (0.024)
    QVERIFY(html.indexOf(closingDateString + "</td><td class=\"left\"></td><td class=\"left\">" + i18n("Closing Balance") + "</td><td class=\"left\"></td><td class=\"value\"></td><td>&nbsp;-8.00</td></tr>") > 0);

    // a 100.00 JPY transfer should be displayed as -1.00 when converted to the base currency using the opening date price
    QVERIFY(html.indexOf("<a href=ledger?id=A000008&tid=T000000000000000001>" + openingDateString + "</a></td><td class=\"left\"></td><td class=\"left\">Test Payee</td><td class=\"left\">Transfer to Checking Account</td><td class=\"value\">&nbsp;-1.00</td><td>&nbsp;-1.00</td></tr>") > 0);

    // a 100.00 JPY transfer should be displayed as -1.00 when converted to the base currency using the intermediate date price
    QVERIFY(html.indexOf("<a href=ledger?id=A000008&tid=T000000000000000003>" + intermediateDateString + "</a></td><td class=\"left\"></td><td class=\"left\">Test Payee</td><td class=\"left\">Transfer to Checking Account</td><td class=\"value\">&nbsp;-1.00</td><td>&nbsp;-2.00</td></tr>") > 0);

    // a 100.00 JPY withdrawal should be displayed as -1.00 when converted to the base currency using the intermediate date price
    QVERIFY(html.indexOf("<a href=ledger?id=A000008&tid=T000000000000000007>" + intermediateDateString + "</a></td><td class=\"left\"></td><td class=\"left\">Test Payee</td><td class=\"left\">Solo</td><td class=\"value\">&nbsp;-1.00</td><td>&nbsp;-3.00</td></tr>") > 0);

    // a 100.00 JPY transfer should be displayed as -2.00 when converted to the base currency using the closing date price (notice the balance is -5.00)
    QVERIFY(html.indexOf("<a href=ledger?id=A000008&tid=T000000000000000005>" + closingDateString + "</a></td><td class=\"left\"></td><td class=\"left\">Test Payee</td><td class=\"left\">Transfer to Checking Account</td><td class=\"value\">&nbsp;-2.00</td><td>&nbsp;-5.00</td></tr>") > 0);

  } catch (const MyMoneyException &e) {
    QFAIL(qPrintable(e.what()));
  }
}

void QueryTableTest::testTaxReport()
{
  try {
    TransactionHelper t1q1(QDate(2004, 1, 1), MyMoneySplit::ActionWithdrawal, moSolo, acChecking, acSolo);
    TransactionHelper t2q1(QDate(2004, 2, 1), MyMoneySplit::ActionWithdrawal, moParent1, acChecking, acTax);

    unsigned cols;
    MyMoneyReport filter;

    filter.setRowType(MyMoneyReport::eCategory);
    filter.setName("Tax Transactions");
    cols = MyMoneyReport::QueryColumns::Number | MyMoneyReport::QueryColumns::Payee | MyMoneyReport::QueryColumns::Account;
    filter.setQueryColumns(static_cast<MyMoneyReport::QueryColumns::Type>(cols));
    filter.setTax(true);

    XMLandback(filter);
    QueryTable qtbl_3(filter);

    writeTabletoHTML(qtbl_3, "Tax Transactions.html");

    QList<ListTable::TableRow> rows = qtbl_3.rows();

    QString html = qtbl_3.renderBody();
    QVERIFY(rows.count() == 1);
  } catch (const MyMoneyException &e) {
    QFAIL(qPrintable(e.what()));
  }
}
