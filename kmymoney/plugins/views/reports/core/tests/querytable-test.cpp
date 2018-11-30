/*
 * Copyright 2005-2018  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2005-2006  Ace Jones <acejones@users.sourceforge.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "querytable-test.h"

#include <QFile>
#include <QTest>

#include <KLocalizedString>

#include "tests/testutilities.h"
#include "cashflowlist.h"
#include "querytable.h"
#include "mymoneyinstitution.h"
#include "mymoneyaccount.h"
#include "mymoneysecurity.h"
#include "mymoneyprice.h"
#include "mymoneystoragedump.h"
#include "mymoneyreport.h"
#include "mymoneysplit.h"
#include "mymoneypayee.h"
#include "mymoneystatement.h"
#include "mymoneyexception.h"
#include "kmymoneysettings.h"

using namespace reports;
using namespace test;

QTEST_GUILESS_MAIN(QueryTableTest)

void writeTabletoHTML(const QueryTable& table, const QString& _filename = QString())
{
  static unsigned filenumber = 1;
  QString filename = _filename;
  if (filename.isEmpty()) {
    filename = QString::fromLatin1("report-%1.html").arg(filenumber, 2, 10,QLatin1Char('0'));
    ++filenumber;
  }

  QFile g(filename);
  g.open(QIODevice::WriteOnly);
  QTextStream(&g) << table.renderHTML();
  g.close();
}

void writeTabletoCSV(const QueryTable& table, const QString& _filename = QString())
{
  static unsigned filenumber = 1;
  QString filename = _filename;
  if (filename.isEmpty()) {
    filename = QString::fromLatin1("report-%1.csv").arg(filenumber, 2, 10,QLatin1Char('0'));
    ++filenumber;
  }

  QFile g(filename);
  g.open(QIODevice::WriteOnly);
  QTextStream(&g) << table.renderCSV();
  g.close();
}

void QueryTableTest::setup()
{
}

void QueryTableTest::init()
{
  storage = new MyMoneyStorageMgr;
  file = MyMoneyFile::instance();
  file->attachStorage(storage);
  MyMoneyFileTransaction ft;
  file->addCurrency(MyMoneySecurity("CAD", "Canadian Dollar",        "C$"));
  file->addCurrency(MyMoneySecurity("USD", "US Dollar",              "$"));
  file->addCurrency(MyMoneySecurity("JPY", "Japanese Yen",           QChar(0x00A5), 1));
  file->addCurrency(MyMoneySecurity("GBP", "British Pound",           "#"));
  file->setBaseCurrency(file->currency("USD"));

  MyMoneyPayee payeeTest;
  payeeTest.setName("Test Payee");
  file->addPayee(payeeTest);
  MyMoneyPayee payeeTest2;
  payeeTest2.setName("Thomas Baumgart");
  file->addPayee(payeeTest2);

  acAsset = (MyMoneyFile::instance()->asset().id());
  acLiability = (MyMoneyFile::instance()->liability().id());
  acExpense = (MyMoneyFile::instance()->expense().id());
  acIncome = (MyMoneyFile::instance()->income().id());
  acChecking = makeAccount(QString("Checking Account"), eMyMoney::Account::Type::Checkings, moCheckingOpen, QDate(2004, 5, 15), acAsset);
  acCredit = makeAccount(QString("Credit Card"), eMyMoney::Account::Type::CreditCard, moCreditOpen, QDate(2004, 7, 15), acLiability);
  acSolo = makeAccount(QString("Solo"), eMyMoney::Account::Type::Expense, MyMoneyMoney(), QDate(2004, 1, 11), acExpense);
  acParent = makeAccount(QString("Parent"), eMyMoney::Account::Type::Expense, MyMoneyMoney(), QDate(2004, 1, 11), acExpense);
  acChild = makeAccount(QString("Child"), eMyMoney::Account::Type::Expense, MyMoneyMoney(), QDate(2004, 2, 11), acParent);
  acForeign = makeAccount(QString("Foreign"), eMyMoney::Account::Type::Expense, MyMoneyMoney(), QDate(2004, 1, 11), acExpense);
  acTax = makeAccount(QString("Tax"), eMyMoney::Account::Type::Expense, MyMoneyMoney(), QDate(2005, 1, 11), acExpense, "", true);

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
    TransactionHelper t1q1(QDate(2004, 1, 1), MyMoneySplit::actionName(eMyMoney::Split::Action::Withdrawal), moSolo, acChecking, acSolo);
    TransactionHelper t2q1(QDate(2004, 2, 1), MyMoneySplit::actionName(eMyMoney::Split::Action::Withdrawal), moParent1, acCredit, acParent);
    TransactionHelper t3q1(QDate(2004, 3, 1), MyMoneySplit::actionName(eMyMoney::Split::Action::Withdrawal), moParent2, acCredit, acParent);
    TransactionHelper t4y1(QDate(2004, 11, 7), MyMoneySplit::actionName(eMyMoney::Split::Action::Withdrawal), moChild, acCredit, acChild);

    TransactionHelper t1q2(QDate(2004, 4, 1), MyMoneySplit::actionName(eMyMoney::Split::Action::Withdrawal), moSolo, acChecking, acSolo);
    TransactionHelper t2q2(QDate(2004, 5, 1), MyMoneySplit::actionName(eMyMoney::Split::Action::Withdrawal), moParent1, acCredit, acParent);
    TransactionHelper t3q2(QDate(2004, 6, 1), MyMoneySplit::actionName(eMyMoney::Split::Action::Withdrawal), moParent2, acCredit, acParent);
    TransactionHelper t4q2(QDate(2004, 11, 7), MyMoneySplit::actionName(eMyMoney::Split::Action::Withdrawal), moChild, acCredit, acChild);

    TransactionHelper t1y2(QDate(2005, 1, 1), MyMoneySplit::actionName(eMyMoney::Split::Action::Withdrawal), moSolo, acChecking, acSolo);
    TransactionHelper t2y2(QDate(2005, 5, 1), MyMoneySplit::actionName(eMyMoney::Split::Action::Withdrawal), moParent1, acCredit, acParent);
    TransactionHelper t3y2(QDate(2005, 9, 1), MyMoneySplit::actionName(eMyMoney::Split::Action::Withdrawal), moParent2, acCredit, acParent);
    TransactionHelper t4y2(QDate(2004, 11, 7), MyMoneySplit::actionName(eMyMoney::Split::Action::Withdrawal), moChild, acCredit, acChild);

    unsigned cols;

    MyMoneyReport filter;
    filter.setRowType(eMyMoney::Report::RowType::Category);
    cols = eMyMoney::Report::QueryColumn::Number | eMyMoney::Report::QueryColumn::Payee | eMyMoney::Report::QueryColumn::Account;
    filter.setQueryColumns(static_cast<eMyMoney::Report::QueryColumn>(cols));   //
    filter.setName("Transactions by Category");
    XMLandback(filter);
    QueryTable qtbl_1(filter);

    writeTabletoHTML(qtbl_1, "Transactions by Category.html");

    QList<ListTable::TableRow> rows = qtbl_1.rows();

    QVERIFY(rows.count() == 19);
    QVERIFY(rows[0][ListTable::ctCategoryType] == "Expense");
    QVERIFY(rows[0][ListTable::ctCategory] == "Parent");
    QVERIFY(rows[0][ListTable::ctPostDate] == "2004-02-01");
    QVERIFY(rows[14][ListTable::ctCategoryType] == "Expense");
    QVERIFY(rows[14][ListTable::ctCategory] == "Solo");
    QVERIFY(rows[14][ListTable::ctPostDate] == "2005-01-01");

    QVERIFY(MyMoneyMoney(rows[6][ListTable::ctValue]) == -(moParent1 + moParent2) * 3);
    QVERIFY(MyMoneyMoney(rows[10][ListTable::ctValue]) == -(moChild) * 3);
    QVERIFY(MyMoneyMoney(rows[16][ListTable::ctValue]) == -(moSolo) * 3);
    QVERIFY(MyMoneyMoney(rows[17][ListTable::ctValue]) == -(moParent1 + moParent2 + moSolo + moChild) * 3);
    QVERIFY(MyMoneyMoney(rows[18][ListTable::ctValue]) == -(moParent1 + moParent2 + moSolo + moChild) * 3 + moCheckingOpen + moCreditOpen);

    filter.setRowType(eMyMoney::Report::RowType::TopCategory);
    cols = eMyMoney::Report::QueryColumn::Number | eMyMoney::Report::QueryColumn::Payee | eMyMoney::Report::QueryColumn::Account;
    filter.setQueryColumns(static_cast<eMyMoney::Report::QueryColumn>(cols));   //
    filter.setName("Transactions by Top Category");
    XMLandback(filter);
    QueryTable qtbl_2(filter);

    writeTabletoHTML(qtbl_2, "Transactions by Top Category.html");

    rows = qtbl_2.rows();

    QVERIFY(rows.count() == 16);
    QVERIFY(rows[0][ListTable::ctCategoryType] == "Expense");
    QVERIFY(rows[0][ListTable::ctTopCategory] == "Parent");
    QVERIFY(rows[0][ListTable::ctPostDate] == "2004-02-01");
    QVERIFY(rows[8][ListTable::ctCategoryType] == "Expense");
    QVERIFY(rows[8][ListTable::ctTopCategory] == "Parent");
    QVERIFY(rows[8][ListTable::ctPostDate] == "2005-09-01");
    QVERIFY(rows[12][ListTable::ctCategoryType] == "Expense");
    QVERIFY(rows[12][ListTable::ctTopCategory] == "Solo");
    QVERIFY(rows[12][ListTable::ctPostDate] == "2005-01-01");

    QVERIFY(MyMoneyMoney(rows[9][ListTable::ctValue]) == -(moParent1 + moParent2 + moChild) * 3);
    QVERIFY(MyMoneyMoney(rows[13][ListTable::ctValue]) == -(moSolo) * 3);
    QVERIFY(MyMoneyMoney(rows[14][ListTable::ctValue]) == -(moParent1 + moParent2 + moSolo + moChild) * 3);
    QVERIFY(MyMoneyMoney(rows[15][ListTable::ctValue]) == -(moParent1 + moParent2 + moSolo + moChild) * 3 + moCheckingOpen + moCreditOpen);

    filter.setRowType(eMyMoney::Report::RowType::Account);
    filter.setName("Transactions by Account");
    cols = eMyMoney::Report::QueryColumn::Number | eMyMoney::Report::QueryColumn::Payee | eMyMoney::Report::QueryColumn::Category;
    filter.setQueryColumns(static_cast<eMyMoney::Report::QueryColumn>(cols));   //
    XMLandback(filter);
    QueryTable qtbl_3(filter);

    writeTabletoHTML(qtbl_3, "Transactions by Account.html");

    rows = qtbl_3.rows();

#if 1
    QVERIFY(rows.count() == 19);
    QVERIFY(rows[1][ListTable::ctAccount] == "Checking Account");
    QVERIFY(rows[1][ListTable::ctCategory] == "Solo");
    QVERIFY(rows[1][ListTable::ctPostDate] == "2004-01-01");
    QVERIFY(rows[15][ListTable::ctAccount] == "Credit Card");
    QVERIFY(rows[15][ListTable::ctCategory] == "Parent");
    QVERIFY(rows[15][ListTable::ctPostDate] == "2005-09-01");
#else
    QVERIFY(rows.count() == 12);
    QVERIFY(rows[0][ListTable::ctAccount] == "Checking Account");
    QVERIFY(rows[0][ListTable::ctCategory] == "Solo");
    QVERIFY(rows[0][ListTable::ctPostDate] == "2004-01-01");
    QVERIFY(rows[11][ListTable::ctAccount] == "Credit Card");
    QVERIFY(rows[11][ListTable::ctCategory] == "Parent");
    QVERIFY(rows[11][ListTable::ctPostDate] == "2005-09-01");
#endif

    QVERIFY(MyMoneyMoney(rows[5][ListTable::ctValue]) == -(moSolo) * 3 + moCheckingOpen);
    QVERIFY(MyMoneyMoney(rows[17][ListTable::ctValue]) == -(moParent1 + moParent2 + moChild) * 3 + moCreditOpen);
    QVERIFY(MyMoneyMoney(rows[18][ListTable::ctValue]) == -(moParent1 + moParent2 + moSolo + moChild) * 3 + moCheckingOpen + moCreditOpen);

    filter.setRowType(eMyMoney::Report::RowType::Payee);
    filter.setName("Transactions by Payee");
    cols = eMyMoney::Report::QueryColumn::Number | eMyMoney::Report::QueryColumn::Memo | eMyMoney::Report::QueryColumn::Category;
    filter.setQueryColumns(static_cast<eMyMoney::Report::QueryColumn>(cols));   //
    XMLandback(filter);
    QueryTable qtbl_4(filter);

    writeTabletoHTML(qtbl_4, "Transactions by Payee.html");

    rows = qtbl_4.rows();

    QVERIFY(rows.count() == 14);
    QVERIFY(rows[0][ListTable::ctPayee] == "Test Payee");
    QVERIFY(rows[0][ListTable::ctCategory] == "Solo");
    QVERIFY(rows[0][ListTable::ctPostDate] == "2004-01-01");
    QVERIFY(rows[7][ListTable::ctPayee] == "Test Payee");
    QVERIFY(rows[7][ListTable::ctCategory] == "Parent: Child");
    QVERIFY(rows[7][ListTable::ctPostDate] == "2004-11-07");
    QVERIFY(rows[11][ListTable::ctPayee] == "Test Payee");
    QVERIFY(rows[11][ListTable::ctCategory] == "Parent");
    QVERIFY(rows[11][ListTable::ctPostDate] == "2005-09-01");

    QVERIFY(MyMoneyMoney(rows[12][ListTable::ctValue]) == -(moParent1 + moParent2 + moSolo + moChild) * 3);
    QVERIFY(MyMoneyMoney(rows[13][ListTable::ctValue]) == -(moParent1 + moParent2 + moSolo + moChild) * 3 + moCheckingOpen + moCreditOpen);

    filter.setRowType(eMyMoney::Report::RowType::Month);
    filter.setName("Transactions by Month");
    cols = eMyMoney::Report::QueryColumn::Number | eMyMoney::Report::QueryColumn::Payee | eMyMoney::Report::QueryColumn::Category;
    filter.setQueryColumns(static_cast<eMyMoney::Report::QueryColumn>(cols));   //
    XMLandback(filter);
    QueryTable qtbl_5(filter);

    writeTabletoHTML(qtbl_5, "Transactions by Month.html");

    rows = qtbl_5.rows();

    QVERIFY(rows.count() == 23);
    QVERIFY(rows[0][ListTable::ctPayee] == "Test Payee");
    QVERIFY(rows[0][ListTable::ctCategory] == "Solo");
    QVERIFY(rows[0][ListTable::ctPostDate] == "2004-01-01");
    QVERIFY(rows[12][ListTable::ctPayee] == "Test Payee");
    QVERIFY(rows[12][ListTable::ctCategory] == "Parent: Child");
    QVERIFY(rows[12][ListTable::ctPostDate] == "2004-11-07");
    QVERIFY(rows[20][ListTable::ctPayee] == "Test Payee");
    QVERIFY(rows[20][ListTable::ctCategory] == "Parent");
    QVERIFY(rows[20][ListTable::ctPostDate] == "2005-09-01");

    QVERIFY(MyMoneyMoney(rows[1][ListTable::ctValue]) == -moSolo);
    QVERIFY(MyMoneyMoney(rows[15][ListTable::ctValue]) == -(moChild) * 3);
    QVERIFY(MyMoneyMoney(rows[9][ListTable::ctValue]) == -moParent1 + moCheckingOpen);
    QVERIFY(MyMoneyMoney(rows[22][ListTable::ctValue]) == -(moParent1 + moParent2 + moSolo + moChild) * 3 + moCheckingOpen + moCreditOpen);

    filter.setRowType(eMyMoney::Report::RowType::Week);
    filter.setName("Transactions by Week");
    cols = eMyMoney::Report::QueryColumn::Number | eMyMoney::Report::QueryColumn::Payee | eMyMoney::Report::QueryColumn::Category;
    filter.setQueryColumns(static_cast<eMyMoney::Report::QueryColumn>(cols));   //
    XMLandback(filter);
    QueryTable qtbl_6(filter);

    writeTabletoHTML(qtbl_6, "Transactions by Week.html");

    rows = qtbl_6.rows();

    QVERIFY(rows.count() == 23);
    QVERIFY(rows[0][ListTable::ctPayee] == "Test Payee");
    QVERIFY(rows[0][ListTable::ctCategory] == "Solo");
    QVERIFY(rows[0][ListTable::ctPostDate] == "2004-01-01");
    QVERIFY(rows[20][ListTable::ctPayee] == "Test Payee");
    QVERIFY(rows[20][ListTable::ctCategory] == "Parent");
    QVERIFY(rows[20][ListTable::ctPostDate] == "2005-09-01");

    QVERIFY(MyMoneyMoney(rows[1][ListTable::ctValue]) == -moSolo);
    QVERIFY(MyMoneyMoney(rows[15][ListTable::ctValue]) == -(moChild) * 3);
    QVERIFY(MyMoneyMoney(rows[21][ListTable::ctValue]) == -moParent2);
    QVERIFY(MyMoneyMoney(rows[22][ListTable::ctValue]) == -(moParent1 + moParent2 + moSolo + moChild) * 3 + moCheckingOpen + moCreditOpen);
  } catch (const MyMoneyException &e) {
    QFAIL(e.what());
  }

  // Test querytable::TableRow::operator> and operator==

  QueryTable::TableRow low;
  low[ListTable::ctPrice] = 'A';
  low[ListTable::ctLastPrice] = 'B';
  low[ListTable::ctBuyPrice] = 'C';

  QueryTable::TableRow high;
  high[ListTable::ctPrice] = 'A';
  high[ListTable::ctLastPrice] = 'C';
  high[ListTable::ctBuyPrice] = 'B';

  QueryTable::TableRow::setSortCriteria({ListTable::ctPrice, ListTable::ctLastPrice, ListTable::ctBuyPrice});
  QVERIFY(low < high);
  QVERIFY(low <= high);
  QVERIFY(high > low);
  QVERIFY(high <= high);
  QVERIFY(high == high);
}

void QueryTableTest::testCashFlowAnalysis()
{
  //
  // Test XIRR calculations
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

  MyMoneyMoney XIRR(list.XIRR(), 1000);
  QVERIFY(XIRR == MyMoneyMoney(1676, 1000));

  list.pop_back();
  list += CashFlowListItem(QDate(2004, 10, 16), MyMoneyMoney(-1358.0));

  XIRR = MyMoneyMoney(list.XIRR(), 1000);
  QVERIFY(XIRR.isZero());

  // two entries
  list.clear();
  list += CashFlowListItem(QDate(2005, 9, 22), MyMoneyMoney(-3472.57));
  list += CashFlowListItem(QDate(2009, 3, 18), MyMoneyMoney(6051.36));

  XIRR = MyMoneyMoney(list.XIRR(), 1000);
  QCOMPARE(XIRR.toDouble(), MyMoneyMoney(173, 1000).toDouble());

  // check ignoring zero values
  list += CashFlowListItem(QDate(1998, 11, 1), MyMoneyMoney(0.0));
  list += CashFlowListItem(QDate(2017, 8, 11), MyMoneyMoney(0.0));

  XIRR = MyMoneyMoney(list.XIRR(), 1000);
  QCOMPARE(XIRR.toDouble(), MyMoneyMoney(173, 1000).toDouble());

  list.pop_back();
  // former implementation crashed
  list += CashFlowListItem(QDate(2014, 8, 11), MyMoneyMoney(0.0));

  XIRR = MyMoneyMoney(list.XIRR(), 1000);
  QCOMPARE(XIRR.toDouble(), MyMoneyMoney(173, 1000).toDouble());

  // different ordering
  list.clear();
  list += CashFlowListItem(QDate(2004, 3, 18), MyMoneyMoney(6051.36));
  list += CashFlowListItem(QDate(2005, 9, 22), MyMoneyMoney(-3472.57));

  XIRR = MyMoneyMoney(list.XIRR(), 1000);
  QCOMPARE(XIRR.toDouble(), MyMoneyMoney(-307, 1000).toDouble());


  // not enough entries
  list.pop_back();

  bool result = false;
  try {
    XIRR = MyMoneyMoney(list.XIRR(), 1000);
  } catch (MyMoneyException &e) {
    result  = true;
  }
  QVERIFY(result);
}

void QueryTableTest::testAccountQuery()
{
  try {
    QString htmlcontext = QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\">\n<html><head><link rel=\"stylesheet\" type=\"text/css\" href=\"html/kmymoney.css\"></head><body>\n%1\n</body></html>\n");

    //
    // No transactions, opening balances only
    //

    MyMoneyReport filter;
    filter.setRowType(eMyMoney::Report::RowType::Institution);
    filter.setName("Accounts by Institution (No transactions)");
    XMLandback(filter);
    QueryTable qtbl_1(filter);

    writeTabletoHTML(qtbl_1, "Accounts by Institution (No transactions).html");

    QList<ListTable::TableRow> rows = qtbl_1.rows();

    QVERIFY(rows.count() == 6);
    QVERIFY(rows[0][ListTable::ctAccount] == "Checking Account");
    QVERIFY(MyMoneyMoney(rows[0][ListTable::ctValue]) == moCheckingOpen);
    QVERIFY(rows[0][ListTable::ctEquityType].isEmpty());
    QVERIFY(rows[2][ListTable::ctAccount] == "Credit Card");
    QVERIFY(MyMoneyMoney(rows[1][ListTable::ctValue]) == moCreditOpen);
    QVERIFY(rows[2][ListTable::ctEquityType].isEmpty());

    QVERIFY(MyMoneyMoney(rows[4][ListTable::ctValue]) == moCheckingOpen + moCreditOpen);
    QVERIFY(MyMoneyMoney(rows[5][ListTable::ctValue]) == moCheckingOpen + moCreditOpen);

    //
    // Adding in transactions
    //

    TransactionHelper t1q1(QDate(2004, 1, 1), MyMoneySplit::actionName(eMyMoney::Split::Action::Withdrawal), moSolo, acChecking, acSolo);
    TransactionHelper t2q1(QDate(2004, 2, 1), MyMoneySplit::actionName(eMyMoney::Split::Action::Withdrawal), moParent1, acCredit, acParent);
    TransactionHelper t3q1(QDate(2004, 3, 1), MyMoneySplit::actionName(eMyMoney::Split::Action::Withdrawal), moParent2, acCredit, acParent);
    TransactionHelper t4y1(QDate(2004, 11, 7), MyMoneySplit::actionName(eMyMoney::Split::Action::Withdrawal), moChild, acCredit, acChild);

    TransactionHelper t1q2(QDate(2004, 4, 1), MyMoneySplit::actionName(eMyMoney::Split::Action::Withdrawal), moSolo, acChecking, acSolo);
    TransactionHelper t2q2(QDate(2004, 5, 1), MyMoneySplit::actionName(eMyMoney::Split::Action::Withdrawal), moParent1, acCredit, acParent);
    TransactionHelper t3q2(QDate(2004, 6, 1), MyMoneySplit::actionName(eMyMoney::Split::Action::Withdrawal), moParent2, acCredit, acParent);
    TransactionHelper t4q2(QDate(2004, 11, 7), MyMoneySplit::actionName(eMyMoney::Split::Action::Withdrawal), moChild, acCredit, acChild);

    TransactionHelper t1y2(QDate(2005, 1, 1), MyMoneySplit::actionName(eMyMoney::Split::Action::Withdrawal), moSolo, acChecking, acSolo);
    TransactionHelper t2y2(QDate(2005, 5, 1), MyMoneySplit::actionName(eMyMoney::Split::Action::Withdrawal), moParent1, acCredit, acParent);
    TransactionHelper t3y2(QDate(2005, 9, 1), MyMoneySplit::actionName(eMyMoney::Split::Action::Withdrawal), moParent2, acCredit, acParent);
    TransactionHelper t4y2(QDate(2004, 11, 7), MyMoneySplit::actionName(eMyMoney::Split::Action::Withdrawal), moChild, acCredit, acChild);

    filter.setRowType(eMyMoney::Report::RowType::Institution);
    filter.setName("Accounts by Institution (With Transactions)");
    XMLandback(filter);
    QueryTable qtbl_2(filter);

    rows = qtbl_2.rows();

    QVERIFY(rows.count() == 6);
    QVERIFY(rows[0][ListTable::ctAccount] == "Checking Account");
    QVERIFY(MyMoneyMoney(rows[0][ListTable::ctValue]) == (moCheckingOpen - moSolo*3));
    QVERIFY(rows[2][ListTable::ctAccount] == "Credit Card");
    QVERIFY(MyMoneyMoney(rows[2][ListTable::ctValue]) == (moCreditOpen - (moParent1 + moParent2 + moChild) * 3));

    QVERIFY(MyMoneyMoney(rows[5][ListTable::ctValue]) == moCheckingOpen + moCreditOpen - (moParent1 + moParent2 + moSolo + moChild) * 3);

    //
    // Account TYPES
    //

    filter.setRowType(eMyMoney::Report::RowType::AccountType);
    filter.setName("Accounts by Type");
    XMLandback(filter);
    QueryTable qtbl_3(filter);

    rows = qtbl_3.rows();

    QVERIFY(rows.count() == 5);
    QVERIFY(rows[0][ListTable::ctAccount] == "Checking Account");
    QVERIFY(MyMoneyMoney(rows[0][ListTable::ctValue]) == (moCheckingOpen - moSolo * 3));
    QVERIFY(rows[2][ListTable::ctAccount] == "Credit Card");
    QVERIFY(MyMoneyMoney(rows[2][ListTable::ctValue]) == (moCreditOpen - (moParent1 + moParent2 + moChild) * 3));

    QVERIFY(MyMoneyMoney(rows[1][ListTable::ctValue]) == moCheckingOpen - moSolo * 3);
    QVERIFY(MyMoneyMoney(rows[3][ListTable::ctValue]) == moCreditOpen - (moParent1 + moParent2 + moChild) * 3);
    QVERIFY(MyMoneyMoney(rows[4][ListTable::ctValue]) == moCheckingOpen + moCreditOpen - (moParent1 + moParent2 + moSolo + moChild) * 3);
  } catch (const MyMoneyException &e) {
    QFAIL(e.what());
  }
}

void QueryTableTest::testInvestment()
{
  try {
    // Equities
    eqStock1 = makeEquity("Stock1", "STK1");
    eqStock2 = makeEquity("Stock2", "STK2");
    eqStock3 = makeEquity("Stock3", "STK3");
    eqStock4 = makeEquity("Stock4", "STK4");

    // Accounts
    acInvestment = makeAccount("Investment", eMyMoney::Account::Type::Investment, moZero, QDate(2003, 11, 1), acAsset);
    acStock1 = makeAccount("Stock 1", eMyMoney::Account::Type::Stock, moZero, QDate(2004, 1, 1), acInvestment, eqStock1);
    acStock2 = makeAccount("Stock 2", eMyMoney::Account::Type::Stock, moZero, QDate(2004, 1, 1), acInvestment, eqStock2);
    acStock3 = makeAccount("Stock 3", eMyMoney::Account::Type::Stock, moZero, QDate(2003, 11, 1), acInvestment, eqStock3);
    acStock4 = makeAccount("Stock 4", eMyMoney::Account::Type::Stock, moZero, QDate(2004, 1, 1), acInvestment, eqStock4);
    acDividends = makeAccount("Dividends", eMyMoney::Account::Type::Income, moZero, QDate(2004, 1, 1), acIncome);
    acInterest = makeAccount("Interest", eMyMoney::Account::Type::Income, moZero, QDate(2004, 1, 1), acIncome);
    acFees = makeAccount("Fees", eMyMoney::Account::Type::Expense, moZero, QDate(2003, 11, 1), acExpense);

    // Transactions
    //                         Date             Action                                               Shares                Price   Stock     Asset       Income
    InvTransactionHelper s1b1(QDate(2003, 12, 1), MyMoneySplit::actionName(eMyMoney::Split::Action::BuyShares),        MyMoneyMoney(1000.00), MyMoneyMoney(100.00), acStock3, acChecking, QString());
    InvTransactionHelper s1b2(QDate(2004, 1, 30), MyMoneySplit::actionName(eMyMoney::Split::Action::BuyShares),        MyMoneyMoney(500.00), MyMoneyMoney(100.00), acStock4, acChecking, acFees, MyMoneyMoney(100.00));
    InvTransactionHelper s1b3(QDate(2004, 1, 30), MyMoneySplit::actionName(eMyMoney::Split::Action::BuyShares),        MyMoneyMoney(500.00), MyMoneyMoney(90.00), acStock4, acChecking, acFees, MyMoneyMoney(100.00));
    InvTransactionHelper s1b4(QDate(2004, 2, 1), MyMoneySplit::actionName(eMyMoney::Split::Action::BuyShares),        MyMoneyMoney(1000.00), MyMoneyMoney(100.00), acStock1, acChecking, QString());
    InvTransactionHelper s1b5(QDate(2004, 3, 1), MyMoneySplit::actionName(eMyMoney::Split::Action::BuyShares),        MyMoneyMoney(1000.00), MyMoneyMoney(110.00), acStock1, acChecking, QString());
    InvTransactionHelper s1s1(QDate(2004, 4, 1), MyMoneySplit::actionName(eMyMoney::Split::Action::BuyShares),        MyMoneyMoney(-200.00), MyMoneyMoney(120.00), acStock1, acChecking, QString());
    InvTransactionHelper s1s2(QDate(2004, 5, 1), MyMoneySplit::actionName(eMyMoney::Split::Action::BuyShares),        MyMoneyMoney(-200.00), MyMoneyMoney(100.00), acStock1, acChecking, QString());
    InvTransactionHelper s1s3(QDate(2004, 5, 30), MyMoneySplit::actionName(eMyMoney::Split::Action::BuyShares),        MyMoneyMoney(-1000.00), MyMoneyMoney(120.00), acStock4, acChecking, acFees, MyMoneyMoney(200.00));
    InvTransactionHelper s1r1(QDate(2004, 6, 1), MyMoneySplit::actionName(eMyMoney::Split::Action::ReinvestDividend), MyMoneyMoney(50.00), MyMoneyMoney(100.00), acStock1, QString(), acDividends);
    InvTransactionHelper s1r2(QDate(2004, 7, 1), MyMoneySplit::actionName(eMyMoney::Split::Action::ReinvestDividend), MyMoneyMoney(50.00), MyMoneyMoney(80.00), acStock1, QString(), acDividends);
    InvTransactionHelper s1c1(QDate(2004, 8, 1), MyMoneySplit::actionName(eMyMoney::Split::Action::Dividend),         MyMoneyMoney(10.00), MyMoneyMoney(100.00), acStock1, acChecking, acDividends);
    InvTransactionHelper s1c2(QDate(2004, 9, 1), MyMoneySplit::actionName(eMyMoney::Split::Action::Dividend),         MyMoneyMoney(10.00), MyMoneyMoney(120.00), acStock1, acChecking, acDividends);
    InvTransactionHelper s1y1(QDate(2004, 9, 15), MyMoneySplit::actionName(eMyMoney::Split::Action::Yield),           MyMoneyMoney(10.00), MyMoneyMoney(110.00), acStock1, acChecking, acInterest);

    makeEquityPrice(eqStock1, QDate(2004, 10, 1), MyMoneyMoney(100.00));
    makeEquityPrice(eqStock3, QDate(2004, 10, 1), MyMoneyMoney(110.00));
    makeEquityPrice(eqStock4, QDate(2004, 10, 1), MyMoneyMoney(110.00));

    //
    // Investment Transactions Report
    //

    MyMoneyReport invtran_r(
      eMyMoney::Report::RowType::TopAccount,
      eMyMoney::Report::QueryColumn::Action | eMyMoney::Report::QueryColumn::Shares | eMyMoney::Report::QueryColumn::Price,
      eMyMoney::TransactionFilter::Date::UserDefined,
      eMyMoney::Report::DetailLevel::All,
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

    QVERIFY(rows.count() == 32);
    QVERIFY(MyMoneyMoney(rows[1][ListTable::ctValue]) == MyMoneyMoney(-100000.00));
    QVERIFY(MyMoneyMoney(rows[2][ListTable::ctValue]) == MyMoneyMoney(-110000.00));
    QVERIFY(MyMoneyMoney(rows[3][ListTable::ctValue]) == MyMoneyMoney(24000.00));
    QVERIFY(MyMoneyMoney(rows[4][ListTable::ctValue]) == MyMoneyMoney(20000.00));
    QVERIFY(MyMoneyMoney(rows[5][ListTable::ctValue]) == MyMoneyMoney(5000.00));
    QVERIFY(MyMoneyMoney(rows[6][ListTable::ctValue]) == MyMoneyMoney(4000.00));
    QVERIFY(MyMoneyMoney(rows[19][ListTable::ctValue]) == MyMoneyMoney(-50100.00));
    QVERIFY(MyMoneyMoney(rows[22][ListTable::ctValue]) == MyMoneyMoney(-45100.00));
    // need to fix these... fundamentally different from the original test
    //QVERIFY(MyMoneyMoney(invtran.m_rows[8][ListTable::ctValue])==MyMoneyMoney( -1000.00));
    //QVERIFY(MyMoneyMoney(invtran.m_rows[11][ListTable::ctValue])==MyMoneyMoney( -1200.00));
    //QVERIFY(MyMoneyMoney(invtran.m_rows[14][ListTable::ctValue])==MyMoneyMoney( -1100.00));

    QVERIFY(MyMoneyMoney(rows[1][ListTable::ctPrice]) == MyMoneyMoney(100.00));
    QVERIFY(MyMoneyMoney(rows[3][ListTable::ctPrice]) == MyMoneyMoney(120.00));
    QVERIFY(MyMoneyMoney(rows[5][ListTable::ctPrice]) == MyMoneyMoney(100.00));
    QVERIFY(MyMoneyMoney(rows[7][ListTable::ctPrice]) == MyMoneyMoney());
    QVERIFY(MyMoneyMoney(rows[10][ListTable::ctPrice]) == MyMoneyMoney());
    QVERIFY(MyMoneyMoney(rows[19][ListTable::ctPrice]) == MyMoneyMoney(100.00));
    QVERIFY(MyMoneyMoney(rows[22][ListTable::ctPrice]) == MyMoneyMoney(90.00));

    QVERIFY(MyMoneyMoney(rows[2][ListTable::ctShares]) == MyMoneyMoney(1000.00));
    QVERIFY(MyMoneyMoney(rows[4][ListTable::ctShares]) == MyMoneyMoney(-200.00));
    QVERIFY(MyMoneyMoney(rows[6][ListTable::ctShares]) == MyMoneyMoney(50.00));
    QVERIFY(MyMoneyMoney(rows[8][ListTable::ctShares]) == MyMoneyMoney(0.00));
    QVERIFY(MyMoneyMoney(rows[11][ListTable::ctShares]) == MyMoneyMoney(0.00));
    QVERIFY(MyMoneyMoney(rows[19][ListTable::ctShares]) == MyMoneyMoney(500.00));
    QVERIFY(MyMoneyMoney(rows[22][ListTable::ctShares]) == MyMoneyMoney(500.00));

    QVERIFY(rows[1][ListTable::ctAction] == "Buy");
    QVERIFY(rows[3][ListTable::ctAction] == "Sell");
    QVERIFY(rows[5][ListTable::ctAction] == "Reinvest");
    QVERIFY(rows[7][ListTable::ctAction] == "Dividend");
    QVERIFY(rows[13][ListTable::ctAction] == "Yield");
    QVERIFY(rows[19][ListTable::ctAction] == "Buy");
    QVERIFY(rows[22][ListTable::ctAction] == "Buy");
#else
    QVERIFY(rows.count() == 9);
    QVERIFY(MyMoneyMoney(rows[0][ListTable::ctValue]) == MyMoneyMoney(100000.00));
    QVERIFY(MyMoneyMoney(rows[1][ListTable::ctValue]) == MyMoneyMoney(110000.00));
    QVERIFY(MyMoneyMoney(rows[2][ListTable::ctValue]) == MyMoneyMoney(-24000.00));
    QVERIFY(MyMoneyMoney(rows[3][ListTable::ctValue]) == MyMoneyMoney(-20000.00));
    QVERIFY(MyMoneyMoney(rows[4][ListTable::ctValue]) == MyMoneyMoney(5000.00));
    QVERIFY(MyMoneyMoney(rows[5][ListTable::ctValue]) == MyMoneyMoney(4000.00));
    QVERIFY(MyMoneyMoney(rows[6][ListTable::ctValue]) == MyMoneyMoney(-1000.00));
    QVERIFY(MyMoneyMoney(rows[7][ListTable::ctValue]) == MyMoneyMoney(-1200.00));
    QVERIFY(MyMoneyMoney(rows[8][ListTable::ctValue]) == MyMoneyMoney(-1100.00));

    QVERIFY(MyMoneyMoney(rows[0][ListTable::ctPrice]) == MyMoneyMoney(100.00));
    QVERIFY(MyMoneyMoney(rows[2][ListTable::ctPrice]) == MyMoneyMoney(120.00));
    QVERIFY(MyMoneyMoney(rows[4][ListTable::ctPrice]) == MyMoneyMoney(100.00));
    QVERIFY(MyMoneyMoney(rows[6][ListTable::ctPrice]) == MyMoneyMoney(0.00));
    QVERIFY(MyMoneyMoney(rows[8][ListTable::ctPrice]) == MyMoneyMoney(0.00));

    QVERIFY(MyMoneyMoney(rows[1][ListTable::ctShares]) == MyMoneyMoney(1000.00));
    QVERIFY(MyMoneyMoney(rows[3][ListTable::ctShares]) == MyMoneyMoney(-200.00));
    QVERIFY(MyMoneyMoney(rows[5][ListTable::ctShares]) == MyMoneyMoney(50.00));
    QVERIFY(MyMoneyMoney(rows[7][ListTable::ctShares]) == MyMoneyMoney(0.00));
    QVERIFY(MyMoneyMoney(rows[8][ListTable::ctShares]) == MyMoneyMoney(0.00));

    QVERIFY(rows[0][ListTable::ctAction] == "Buy");
    QVERIFY(rows[2][ListTable::ctAction] == "Sell");
    QVERIFY(rows[4][ListTable::ctAction] == "Reinvest");
    QVERIFY(rows[6][ListTable::ctAction] == "Dividend");
    QVERIFY(rows[8][ListTable::ctAction] == "Yield");
#endif

#if 1
    // i think this is the correct amount. different treatment of dividend and yield
    QVERIFY(MyMoneyMoney(rows[17][ListTable::ctValue]) == MyMoneyMoney(-153700.00));
    QVERIFY(MyMoneyMoney(rows[29][ListTable::ctValue]) == MyMoneyMoney(24600.00));
    QVERIFY(MyMoneyMoney(rows[31][ListTable::ctValue]) == MyMoneyMoney(-129100.00));
#else
    QVERIFY(searchHTML(html, i18n("Total Stock 1")) == MyMoneyMoney(171700.00));
    QVERIFY(searchHTML(html, i18n("Grand Total")) == MyMoneyMoney(171700.00));
#endif

    //
    // Investment Performance Report
    //

    MyMoneyReport invhold_r(
      eMyMoney::Report::RowType::AccountByTopAccount,
      eMyMoney::Report::QueryColumn::Performance,
      eMyMoney::TransactionFilter::Date::UserDefined,
      eMyMoney::Report::DetailLevel::All,
      i18n("Investment Performance by Account"),
      i18n("Test Report")
    );
    invhold_r.setDateFilter(QDate(2004, 1, 1), QDate(2004, 10, 1));
    invhold_r.setInvestmentsOnly(true);
    XMLandback(invhold_r);
    QueryTable invhold(invhold_r);

    writeTabletoHTML(invhold, "Investment Performance by Account.html");

    rows = invhold.rows();

    QVERIFY(rows.count() == 5);
    QVERIFY(MyMoneyMoney(rows[0][ListTable::ctReturn]) == MyMoneyMoney("669/10000"));
    QVERIFY(MyMoneyMoney(rows[0][ListTable::ctReturnInvestment]) == MyMoneyMoney("-39/5000"));
    QVERIFY(MyMoneyMoney(rows[0][ListTable::ctBuys]) == MyMoneyMoney(-210000.00));
    QVERIFY(MyMoneyMoney(rows[0][ListTable::ctSells]) == MyMoneyMoney(44000.00));
    QVERIFY(MyMoneyMoney(rows[0][ListTable::ctReinvestIncome]) == MyMoneyMoney(9000.00));
    QVERIFY(MyMoneyMoney(rows[0][ListTable::ctCashIncome]) == MyMoneyMoney(3300.00));

    QVERIFY(MyMoneyMoney(rows[1][ListTable::ctReturn]) == MyMoneyMoney("1349/10000"));
    QVERIFY(MyMoneyMoney(rows[1][ListTable::ctReturnInvestment]) == MyMoneyMoney("1/10"));
    QVERIFY(MyMoneyMoney(rows[1][ListTable::ctStartingBalance]) == MyMoneyMoney(100000.00)); // this should stay non-zero to check if investment performance is calculated at non-zero starting balance

    QVERIFY(MyMoneyMoney(rows[2][ListTable::ctReturn]) == MyMoneyMoney("2501/2500"));
    QVERIFY(MyMoneyMoney(rows[2][ListTable::ctReturnInvestment]) == MyMoneyMoney("323/1250"));
    QVERIFY(MyMoneyMoney(rows[2][ListTable::ctBuys]) == MyMoneyMoney(-95200.00));
    QVERIFY(MyMoneyMoney(rows[2][ListTable::ctSells]) == MyMoneyMoney(119800.00));
    QVERIFY(MyMoneyMoney(rows[2][ListTable::ctEndingBalance]) == MyMoneyMoney(0.00)); // this should stay zero to check if investment performance is calculated at zero ending balance

    QVERIFY(MyMoneyMoney(rows[4][ListTable::ctEndingBalance]) == MyMoneyMoney(280000.00));

#if 0
    // Dump file & reports
    QFile g("investmentkmy.xml");
    g.open(QIODevice::WriteOnly);
    MyMoneyStorageXML xml;
    IMyMoneyOperationsFormat& interface = xml;
    interface.writeFile(&g, dynamic_cast<IMyMoneySerialization*>(MyMoneyFile::instance()->storage()));
    g.close();

    invtran.dump("invtran.html", "<html><head></head><body>%1</body></html>");
    invhold.dump("invhold.html", "<html><head></head><body>%1</body></html>");
#endif

  } catch (const MyMoneyException &e) {
    QFAIL(e.what());
  }
}

// prevents bug #312135
void QueryTableTest::testSplitShares()
{
  try {
    MyMoneyMoney firstSharesPurchase(16);
    MyMoneyMoney splitFactor(2);
    MyMoneyMoney secondSharesPurchase(1);
    MyMoneyMoney sharesAtTheEnd = firstSharesPurchase / splitFactor + secondSharesPurchase;
    MyMoneyMoney priceBeforeSplit(74.99, 100);
    MyMoneyMoney priceAfterSplit = splitFactor * priceBeforeSplit;

    // Equities
    eqStock1 = makeEquity("Stock1", "STK1");

    // Accounts
    acInvestment = makeAccount("Investment", eMyMoney::Account::Type::Investment, moZero, QDate(2017, 8, 1), acAsset);
    acStock1 =     makeAccount("Stock 1",    eMyMoney::Account::Type::Stock,      moZero, QDate(2017, 8, 1), acInvestment, eqStock1);

    // Transactions
    //                        Date               Action                           Shares                Price             Stock     Asset       Income
    InvTransactionHelper s1b1(QDate(2017, 8, 1), MyMoneySplit::actionName(eMyMoney::Split::Action::BuyShares),   firstSharesPurchase,  priceBeforeSplit, acStock1, acChecking, QString());
    InvTransactionHelper s1s1(QDate(2017, 8, 2), MyMoneySplit::actionName(eMyMoney::Split::Action::SplitShares), splitFactor,          MyMoneyMoney(),   acStock1, QString(),  QString());
    InvTransactionHelper s1b2(QDate(2017, 8, 3), MyMoneySplit::actionName(eMyMoney::Split::Action::BuyShares),   secondSharesPurchase, priceAfterSplit,  acStock1, acChecking, QString());

    //
    // Investment Performance Report
    //

    MyMoneyReport invhold_r(
      eMyMoney::Report::RowType::AccountByTopAccount,
      eMyMoney::Report::QueryColumn::Performance,
      eMyMoney::TransactionFilter::Date::UserDefined,
      eMyMoney::Report::DetailLevel::All,
      i18n("Investment Performance by Account"),
      i18n("Test Report")
    );
    invhold_r.setDateFilter(QDate(2017, 8, 1), QDate(2017, 8, 3));
    invhold_r.setInvestmentsOnly(true);
    XMLandback(invhold_r);
    QueryTable invhold(invhold_r);

    writeTabletoHTML(invhold, "Investment Performance by Account (with stock split).html");

    const auto rows = invhold.rows();

    QVERIFY(rows.count() == 3);
    QVERIFY(MyMoneyMoney(rows[0][ListTable::ctBuys]) == sharesAtTheEnd * priceAfterSplit * MyMoneyMoney(-1));

  } catch (const MyMoneyException &e) {
    QFAIL(e.what());
  }
}

// prevents bug #118159
void QueryTableTest::testConversionRate()
{
  try {
    MyMoneyMoney firsConversionRate(1.1800, 10000);
    MyMoneyMoney secondConversionRate(1.1567, 10000);
    MyMoneyMoney amountWithdrawn(100);

    const auto acCadChecking = makeAccount(QString("Canadian Checking"), eMyMoney::Account::Type::Checkings, moZero, QDate(2017, 8, 1), acAsset, "CAD");

    makePrice("CAD", QDate(2017, 8, 1), firsConversionRate);
    makePrice("CAD", QDate(2017, 8, 2), secondConversionRate);

    TransactionHelper t1(QDate(2017, 8, 1), MyMoneySplit::actionName(eMyMoney::Split::Action::Withdrawal), amountWithdrawn, acCadChecking, acSolo, "CAD");

    MyMoneyReport filter;
    filter.setRowType(eMyMoney::Report::RowType::Account);
    filter.setDateFilter(QDate(2017, 8, 1), QDate(2017, 8, 2));
    filter.setName("Transactions by Account");
    auto cols = eMyMoney::Report::QueryColumn::Number | eMyMoney::Report::QueryColumn::Payee | eMyMoney::Report::QueryColumn::Category | eMyMoney::Report::QueryColumn::Balance;
    filter.setQueryColumns(static_cast<eMyMoney::Report::QueryColumn>(cols));   //
    XMLandback(filter);
    QueryTable qtbl(filter);

    writeTabletoHTML(qtbl, "Transactions by Account (conversion rate).html");

    const auto rows = qtbl.rows();

    QVERIFY(rows.count() == 5);
    QVERIFY(MyMoneyMoney(rows[1][ListTable::ctValue]) == amountWithdrawn * firsConversionRate * MyMoneyMoney(-1));
    QVERIFY(MyMoneyMoney(rows[1][ListTable::ctPrice]) == firsConversionRate);
    QVERIFY(MyMoneyMoney(rows[2][ListTable::ctBalance]) == amountWithdrawn * secondConversionRate * MyMoneyMoney(-1));
    QVERIFY(MyMoneyMoney(rows[2][ListTable::ctPrice]) == secondConversionRate);

  } catch (const MyMoneyException &e) {
    QFAIL(e.what());
  }

}

//this is to prevent me from making mistakes again when modifying balances - asoliverez
//this case tests only the opening and ending balance of the accounts
void QueryTableTest::testBalanceColumn()
{
  try {
    TransactionHelper t1q1(QDate(2004, 1, 1), MyMoneySplit::actionName(eMyMoney::Split::Action::Withdrawal), moSolo, acChecking, acSolo);
    TransactionHelper t2q1(QDate(2004, 2, 1), MyMoneySplit::actionName(eMyMoney::Split::Action::Withdrawal), moParent1, acCredit, acParent);
    TransactionHelper t3q1(QDate(2004, 3, 1), MyMoneySplit::actionName(eMyMoney::Split::Action::Withdrawal), moParent2, acCredit, acParent);
    TransactionHelper t4y1(QDate(2004, 11, 7), MyMoneySplit::actionName(eMyMoney::Split::Action::Withdrawal), moChild, acCredit, acChild);

    TransactionHelper t1q2(QDate(2004, 4, 1), MyMoneySplit::actionName(eMyMoney::Split::Action::Withdrawal), moSolo, acChecking, acSolo);
    TransactionHelper t2q2(QDate(2004, 5, 1), MyMoneySplit::actionName(eMyMoney::Split::Action::Withdrawal), moParent1, acCredit, acParent);
    TransactionHelper t3q2(QDate(2004, 6, 1), MyMoneySplit::actionName(eMyMoney::Split::Action::Withdrawal), moParent2, acCredit, acParent);
    TransactionHelper t4q2(QDate(2004, 11, 7), MyMoneySplit::actionName(eMyMoney::Split::Action::Withdrawal), moChild, acCredit, acChild);

    TransactionHelper t1y2(QDate(2005, 1, 1), MyMoneySplit::actionName(eMyMoney::Split::Action::Withdrawal), moSolo, acChecking, acSolo);
    TransactionHelper t2y2(QDate(2005, 5, 1), MyMoneySplit::actionName(eMyMoney::Split::Action::Withdrawal), moParent1, acCredit, acParent);
    TransactionHelper t3y2(QDate(2005, 9, 1), MyMoneySplit::actionName(eMyMoney::Split::Action::Withdrawal), moParent2, acCredit, acParent);
    TransactionHelper t4y2(QDate(2004, 11, 7), MyMoneySplit::actionName(eMyMoney::Split::Action::Withdrawal), moChild, acCredit, acChild);

    unsigned cols;

    MyMoneyReport filter;

    filter.setRowType(eMyMoney::Report::RowType::Account);
    filter.setName("Transactions by Account");
    cols = eMyMoney::Report::QueryColumn::Number | eMyMoney::Report::QueryColumn::Payee | eMyMoney::Report::QueryColumn::Category | eMyMoney::Report::QueryColumn::Balance;
    filter.setQueryColumns(static_cast<eMyMoney::Report::QueryColumn>(cols));   //
    XMLandback(filter);
    QueryTable qtbl_3(filter);

    writeTabletoHTML(qtbl_3, "Transactions by Account.html");

    QString html = qtbl_3.renderHTML();

    QList<ListTable::TableRow> rows = qtbl_3.rows();

    QVERIFY(rows.count() == 19);

    //this is to make sure that the dates of closing and opening balances and the balance numbers are ok
    QString openingDate = QLocale().toString(QDate(2004, 1, 1), QLocale::ShortFormat);
    QString closingDate = QLocale().toString(QDate(2005, 9, 1), QLocale::ShortFormat);
    QVERIFY(html.indexOf(openingDate + "</td><td class=\"left0\"></td><td class=\"left0\">" + i18n("Opening Balance")) > 0);
    QVERIFY(html.indexOf(closingDate + "</td><td class=\"left0\"></td><td class=\"left0\">" + i18n("Closing Balance") + "</td><td class=\"left0\"></td><td class=\"value\"></td><td>&nbsp;-702.36</td></tr>") > 0);
    QVERIFY(html.indexOf(closingDate + "</td><td class=\"left0\"></td><td class=\"left0\">" + i18n("Closing Balance") + "</td><td class=\"left0\"></td><td class=\"value\"></td><td>&nbsp;-705.69</td></tr>") > 0);

  } catch (const MyMoneyException &e) {
    QFAIL(e.what());
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

    QString acJpyChecking = makeAccount(QString("Japanese Checking"), eMyMoney::Account::Type::Checkings, moJpyOpening, QDate(2003, 11, 15), acAsset, "JPY");

    makePrice("JPY", QDate(2004, 1, 1), MyMoneyMoney(moJpyPrice));
    makePrice("JPY", QDate(2004, 5, 1), MyMoneyMoney(moJpyPrice2));
    makePrice("JPY", QDate(2004, 6, 30), MyMoneyMoney(moJpyPrice3));

    QDate openingDate(2004, 2, 20);
    QDate intermediateDate(2004, 5, 20);
    QDate closingDate(2004, 7, 20);

    TransactionHelper t1(openingDate,      MyMoneySplit::actionName(eMyMoney::Split::Action::Transfer),   MyMoneyMoney(moJpyTransaction), acJpyChecking, acChecking, "JPY");
    TransactionHelper t4(openingDate,      MyMoneySplit::actionName(eMyMoney::Split::Action::Deposit),    MyMoneyMoney(moTransaction),    acCredit,      acChecking);
    TransactionHelper t2(intermediateDate, MyMoneySplit::actionName(eMyMoney::Split::Action::Transfer),   MyMoneyMoney(moJpyTransaction), acJpyChecking, acChecking, "JPY");
    TransactionHelper t5(intermediateDate, MyMoneySplit::actionName(eMyMoney::Split::Action::Deposit),    MyMoneyMoney(moTransaction),    acCredit,      acChecking);
    TransactionHelper t3(closingDate,      MyMoneySplit::actionName(eMyMoney::Split::Action::Transfer),   MyMoneyMoney(moJpyTransaction), acJpyChecking, acChecking, "JPY");
    TransactionHelper t6(closingDate,      MyMoneySplit::actionName(eMyMoney::Split::Action::Deposit),    MyMoneyMoney(moTransaction),    acCredit,      acChecking);
    // test that an income/expense transaction that involves a currency exchange is properly reported
    TransactionHelper t7(intermediateDate, MyMoneySplit::actionName(eMyMoney::Split::Action::Withdrawal), MyMoneyMoney(moJpyTransaction), acJpyChecking, acSolo, "JPY");

    unsigned cols;

    MyMoneyReport filter;

    filter.setRowType(eMyMoney::Report::RowType::Account);
    filter.setName("Transactions by Account");
    cols = eMyMoney::Report::QueryColumn::Number | eMyMoney::Report::QueryColumn::Payee | eMyMoney::Report::QueryColumn::Category | eMyMoney::Report::QueryColumn::Balance;
    filter.setQueryColumns(static_cast<eMyMoney::Report::QueryColumn>(cols));
    // don't convert values to the default currency
    filter.setConvertCurrency(false);
    XMLandback(filter);
    QueryTable qtbl_3(filter);

    writeTabletoHTML(qtbl_3, "Transactions by Account (multiple currencies).html");

    QString html = qtbl_3.renderHTML();

    QList<ListTable::TableRow> rows = qtbl_3.rows();

    QVERIFY(rows.count() == 24);

    //this is to make sure that the dates of closing and opening balances and the balance numbers are ok
    QString openingDateString = QLocale().toString(openingDate, QLocale::ShortFormat);
    QString intermediateDateString = QLocale().toString(intermediateDate, QLocale::ShortFormat);
    QString closingDateString = QLocale().toString(closingDate, QLocale::ShortFormat);
    // check the opening and closing balances
    QVERIFY(html.indexOf(openingDateString + "</td><td class=\"left0\"></td><td class=\"left0\">" + i18n("Opening Balance") + "</td><td class=\"left0\"></td><td class=\"value\"></td><td>USD&nbsp;0.00</td></tr>") > 0);
    QVERIFY(html.indexOf(closingDateString + "</td><td class=\"left0\"></td><td class=\"left0\">" + i18n("Closing Balance") + "</td><td class=\"left0\"></td><td class=\"value\"></td><td>USD&nbsp;304.00</td></tr>") > 0);
    QVERIFY(html.indexOf(closingDateString + "</td><td class=\"left0\"></td><td class=\"left0\">" + i18n("Closing Balance") + "</td><td class=\"left0\"></td><td class=\"value\"></td><td>USD&nbsp;-300.00</td></tr>") > 0);
    QVERIFY(html.indexOf(closingDateString + "</td><td class=\"left0\"></td><td class=\"left0\">" + i18n("Closing Balance") + "</td><td class=\"left0\"></td><td class=\"value\"></td><td>JPY&nbsp;-400.00</td></tr>") > 0);

    // after a transfer of 100 JPY the balance should be 1.00 - price is 0.010 (precision of 2)
    QVERIFY(html.indexOf("<a href=ledger?id=A000001&tid=T000000000000000001>" + openingDateString + "</a></td><td class=\"left0\"></td><td class=\"left0\">Test Payee</td><td class=\"left0\">Transfer from Japanese Checking</td><td class=\"value\">USD&nbsp;1.00</td><td>USD&nbsp;1.00</td></tr>") > 0);

    // after a transfer of 100 the balance should be 101.00
    QVERIFY(html.indexOf("<a href=ledger?id=A000001&tid=T000000000000000002>" + openingDateString + "</a></td><td class=\"left0\"></td><td class=\"left0\">Test Payee</td><td class=\"left0\">Transfer from Credit Card</td><td class=\"value\">USD&nbsp;100.00</td><td>USD&nbsp;101.00</td></tr>") > 0);

    // after a transfer of 100 JPY the balance should be 102.00 - price is 0.011 (precision of 2)
    QVERIFY(html.indexOf("<a href=ledger?id=A000001&tid=T000000000000000003>" + intermediateDateString + "</a></td><td class=\"left0\"></td><td class=\"left0\">Test Payee</td><td class=\"left0\">Transfer from Japanese Checking</td><td class=\"value\">USD&nbsp;1.00</td><td>USD&nbsp;102.00</td></tr>") > 0);

    // after a transfer of 100 the balance should be 202.00
    QVERIFY(html.indexOf("<a href=ledger?id=A000001&tid=T000000000000000004>" + intermediateDateString + "</a></td><td class=\"left0\"></td><td class=\"left0\">Test Payee</td><td class=\"left0\">Transfer from Credit Card</td><td class=\"value\">USD&nbsp;100.00</td><td>USD&nbsp;202.00</td></tr>") > 0);

    // after a transfer of 100 JPY the balance should be 204.00 - price is 0.024 (precision of 2)
    QVERIFY(html.indexOf("<a href=ledger?id=A000001&tid=T000000000000000005>" + closingDateString + "</a></td><td class=\"left0\"></td><td class=\"left0\">Test Payee</td><td class=\"left0\">Transfer from Japanese Checking</td><td class=\"value\">USD&nbsp;2.00</td><td>USD&nbsp;204.00</td></tr>") > 0);

    // after a transfer of 100 the balance should be 304.00
    QVERIFY(html.indexOf("<a href=ledger?id=A000001&tid=T000000000000000006>" + closingDateString + "</a></td><td class=\"left0\"></td><td class=\"left0\">Test Payee</td><td class=\"left0\">Transfer from Credit Card</td><td class=\"value\">USD&nbsp;100.00</td><td>USD&nbsp;304.00</td></tr>") > 0);

    // a 100.00 JPY withdrawal should be displayed as such even if the expense account uses another currency
    QVERIFY(html.indexOf("<a href=ledger?id=A000008&tid=T000000000000000007>" + intermediateDateString + "</a></td><td class=\"left0\"></td><td class=\"left0\">Test Payee</td><td class=\"left0\">Solo</td><td class=\"value\">JPY&nbsp;-100.00</td><td>JPY&nbsp;-300.00</td></tr>") > 0);

    // now run the same report again but this time convert all values to the base currency and make sure the values are correct
    filter.setConvertCurrency(true);
    XMLandback(filter);
    QueryTable qtbl_4(filter);

    writeTabletoHTML(qtbl_4, "Transactions by Account (multiple currencies converted to base).html");

    html = qtbl_4.renderHTML();

    rows = qtbl_4.rows();

    QVERIFY(rows.count() == 23);

    // check the opening and closing balances
    QVERIFY(html.indexOf(openingDateString + "</td><td class=\"left0\"></td><td class=\"left0\">" + i18n("Opening Balance") + "</td><td class=\"left0\"></td><td class=\"value\"></td><td>&nbsp;0.00</td></tr>") > 0);
    QVERIFY(html.indexOf(closingDateString + "</td><td class=\"left0\"></td><td class=\"left0\">" + i18n("Closing Balance") + "</td><td class=\"left0\"></td><td class=\"value\"></td><td>&nbsp;304.00</td></tr>") > 0);
    QVERIFY(html.indexOf(closingDateString + "</td><td class=\"left0\"></td><td class=\"left0\">" + i18n("Closing Balance") + "</td><td class=\"left0\"></td><td class=\"value\"></td><td>&nbsp;-300.00</td></tr>") > 0);
    // although the balance should be -5.00 it's -8.00 because the foreign currency balance is converted using the closing date price (0.024)
    QVERIFY(html.indexOf(closingDateString + "</td><td class=\"left0\"></td><td class=\"left0\">" + i18n("Closing Balance") + "</td><td class=\"left0\"></td><td class=\"value\"></td><td>&nbsp;-8.00</td></tr>") > 0);

    // a 100.00 JPY transfer should be displayed as -1.00 when converted to the base currency using the opening date price
    QVERIFY(html.indexOf("<a href=ledger?id=A000008&tid=T000000000000000001>" + openingDateString + "</a></td><td class=\"left0\"></td><td class=\"left0\">Test Payee</td><td class=\"left0\">Transfer to Checking Account</td><td class=\"value\">&nbsp;-1.00</td><td>&nbsp;-1.00</td></tr>") > 0);

    // a 100.00 JPY transfer should be displayed as -1.00 when converted to the base currency using the intermediate date price
    QVERIFY(html.indexOf("<a href=ledger?id=A000008&tid=T000000000000000003>" + intermediateDateString + "</a></td><td class=\"left0\"></td><td class=\"left0\">Test Payee</td><td class=\"left0\">Transfer to Checking Account</td><td class=\"value\">&nbsp;-1.00</td><td>&nbsp;-2.00</td></tr>") > 0);

    // a 100.00 JPY transfer should be displayed as -2.00 when converted to the base currency using the closing date price (notice the balance is -5.00)
    QVERIFY(html.indexOf("<a href=ledger?id=A000008&tid=T000000000000000005>" + closingDateString + "</a></td><td class=\"left0\"></td><td class=\"left0\">Test Payee</td><td class=\"left0\">Transfer to Checking Account</td><td class=\"value\">&nbsp;-2.00</td><td>&nbsp;-5.00</td></tr>") > 0);

    // a 100.00 JPY withdrawal should be displayed as -1.00 when converted to the base currency using the intermediate date price
    QVERIFY(html.indexOf("<a href=ledger?id=A000008&tid=T000000000000000007>" + intermediateDateString + "</a></td><td class=\"left0\"></td><td class=\"left0\">Test Payee</td><td class=\"left0\">Solo</td><td class=\"value\">&nbsp;-1.00</td><td>&nbsp;-3.00</td></tr>") > 0);

  } catch (const MyMoneyException &e) {
    QFAIL(e.what());
  }
}

void QueryTableTest::testTaxReport()
{
  try {
    TransactionHelper t1q1(QDate(2004, 1, 1), MyMoneySplit::actionName(eMyMoney::Split::Action::Withdrawal), moSolo, acChecking, acSolo);
    TransactionHelper t2q1(QDate(2004, 2, 1), MyMoneySplit::actionName(eMyMoney::Split::Action::Withdrawal), moParent1, acChecking, acTax);

    unsigned cols;
    MyMoneyReport filter;

    filter.setRowType(eMyMoney::Report::RowType::Category);
    filter.setName("Tax Transactions");
    cols = eMyMoney::Report::QueryColumn::Number | eMyMoney::Report::QueryColumn::Payee | eMyMoney::Report::QueryColumn::Account;
    filter.setQueryColumns(static_cast<eMyMoney::Report::QueryColumn>(cols));
    filter.setTax(true);

    XMLandback(filter);
    QueryTable qtbl_3(filter);

    writeTabletoHTML(qtbl_3, "Tax Transactions.html");

    QList<ListTable::TableRow> rows = qtbl_3.rows();

    QString html = qtbl_3.renderHTML();
    QVERIFY(rows.count() == 5);
  } catch (const MyMoneyException &e) {
    QFAIL(e.what());
  }
}
