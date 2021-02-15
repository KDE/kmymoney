/*
    SPDX-FileCopyrightText: 2007-2010 Alvaro Soliverez <asoliverez@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "mymoneyforecast-test.h"

#include <iostream>
#include <QList>
#include <QtTest>

#include "mymoneybudget.h"

#include "mymoneyexception.h"

#include "mymoneystoragedump.h"
#include "tests/testutilities.h"

#include "mymoneyinstitution.h"
#include "mymoneysecurity.h"
#include "mymoneymoney.h"
#include "mymoneysplit.h"
#include "mymoneyschedule.h"
#include "mymoneypayee.h"
#include "mymoneyenums.h"

using namespace eMyMoney;
using namespace test;

QTEST_GUILESS_MAIN(MyMoneyForecastTest)

MyMoneyForecastTest::MyMoneyForecastTest() :
  m(nullptr),
  storage(nullptr),
  file(nullptr),
  moT1(57, 1),
  moT2(63, 1),
  moT3(84, 1),
  moT4(62, 1),
  moT5(104, 1)
{
}

void MyMoneyForecastTest::init()
{

  //all this has been taken from pivottabletest.cpp, by Thomas Baumgart and Ace Jones

  file = MyMoneyFile::instance();
  file->unload();

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
  payeeTest2.setName("Alvaro Soliverez");
  file->addPayee(payeeTest2);

  acAsset = (MyMoneyFile::instance()->asset().id());
  acLiability = (MyMoneyFile::instance()->liability().id());
  acExpense = (MyMoneyFile::instance()->expense().id());
  acIncome = (MyMoneyFile::instance()->income().id());
  acChecking = makeAccount(QString("Checking Account"), Account::Type::Checkings, moCheckingOpen, QDate(2004, 5, 15), acAsset, "USD");
  acCredit = makeAccount(QString("Credit Card"), Account::Type::CreditCard, moCreditOpen, QDate(2004, 7, 15), acLiability, "USD");
  acSolo = makeAccount(QString("Solo"), Account::Type::Expense, MyMoneyMoney(), QDate(2004, 1, 11), acExpense, "USD");
  acParent = makeAccount(QString("Parent"), Account::Type::Expense, MyMoneyMoney(), QDate(2004, 1, 11), acExpense, "USD");
  acChild = makeAccount(QString("Child"), Account::Type::Expense, MyMoneyMoney(), QDate(2004, 2, 11), acParent, "USD");
  acForeign = makeAccount(QString("Foreign"), Account::Type::Expense, MyMoneyMoney(), QDate(2004, 1, 11), acExpense, "USD");
  acInvestment = makeAccount("Investment", Account::Type::Investment, moZero, QDate(2004, 1, 1), acAsset, "USD");

  acSecondChild = makeAccount(QString("Second Child"), Account::Type::Expense, MyMoneyMoney(), QDate(2004, 2, 11), acParent, "USD");
  acGrandChild1 = makeAccount(QString("Grand Child 1"), Account::Type::Expense, MyMoneyMoney(), QDate(2004, 2, 11), acChild, "USD");
  acGrandChild2 = makeAccount(QString("Grand Child 2"), Account::Type::Expense, MyMoneyMoney(), QDate(2004, 2, 11), acChild, "USD");

  //this account added to have an account to test opening date calculations
  acCash = makeAccount(QString("Cash"), Account::Type::Cash, moCreditOpen, QDate::currentDate().addDays(-2), acAsset, "USD");


  MyMoneyInstitution i("Bank of the World", "", "", "", "", "", "");
  file->addInstitution(i);
  inBank = i.id();
  ft.commit();

}

void MyMoneyForecastTest::cleanup()
{
}

void MyMoneyForecastTest::testEmptyConstructor()
{
  MyMoneyForecast a;
  MyMoneyAccount b;

  QVERIFY(a.forecastBalance(b, QDate::currentDate()).isZero());
  QVERIFY(!a.isForecastAccount(b));
  QVERIFY(a.forecastBalance(b, QDate::currentDate()) == MyMoneyMoney());
  QVERIFY(a.daysToMinimumBalance(b) == -1);
  QVERIFY(a.daysToZeroBalance(b) == -2);
  QVERIFY(a.forecastDays() == 90);
  QVERIFY(a.accountsCycle() == 30);
  QVERIFY(a.forecastCycles() == 3);
  QVERIFY(a.historyStartDate() == QDate::currentDate().addDays(-3*30));
  QVERIFY(a.historyEndDate() == QDate::currentDate().addDays(-1));
  QVERIFY(a.historyDays() == 30 * 3);
}


void MyMoneyForecastTest::testDoForecastInit()
{
  MyMoneyForecast a;

  a.doForecast();
  /*
  //check the illegal argument validation
  try {
    KMyMoneySettings::setForecastDays(-10);
    a.doForecast();
  }
  catch (const MyMoneyException &e)
  {
    QFAIL("Unexpected exception");
  }
  try {
    KMyMoneySettings::setForecastAccountCycle(-20);
      a.doForecast();
    }
    catch (const MyMoneyException &e) {
      QFAIL("Unexpected exception");
  }
  try {
    KMyMoneySettings::setForecastCycles(-10);
    a.doForecast();
  }
  catch (const MyMoneyException &e) {
    QFAIL("Unexpected exception");
  }

  try {
    KMyMoneySettings::setForecastAccountCycle(0);
    a.doForecast();
  }
  catch (const MyMoneyException &e) {
    QFAIL("Unexpected exception");
  }
  try {
    KMyMoneySettings::setForecastDays(0);
    KMyMoneySettings::setForecastCycles(0);
    KMyMoneySettings::setForecastAccountCycle(0);
    a.doForecast();
  }
  catch (const MyMoneyException &e) {
    QVERIFY("Unexpected exception");
  }*/
}

//test that it forecasts correctly with transactions in the period of forecast
void MyMoneyForecastTest::testDoForecast()
{
  //set up environment
  MyMoneyForecast a;

  MyMoneyAccount a_checking = file->account(acChecking);
  MyMoneyAccount a_credit = file->account(acCredit);

  //test empty forecast
  a.doForecast(); //this is just to check nothing goes wrong if forecast is run against an empty template

  //setup some transactions
  TransactionHelper t1(QDate::currentDate().addDays(-1), MyMoneySplit::actionName(eMyMoney::Split::Action::Withdrawal), this->moT1, acChecking, acSolo);
  TransactionHelper t2(QDate::currentDate().addDays(-1), MyMoneySplit::actionName(eMyMoney::Split::Action::Deposit), -(this->moT2), acCredit, acParent);
  TransactionHelper t3(QDate::currentDate().addDays(-1), MyMoneySplit::actionName(eMyMoney::Split::Action::Transfer), this->moT1, acCredit, acChecking);

  a.setForecastMethod(1);
  a.setForecastDays(3);
  a.setAccountsCycle(1);
  a.setForecastCycles(1);
  a.setBeginForecastDay(0);
  a.setHistoryMethod(0); //moving average
  a.doForecast();

  //checking didn't have balance variations, so the forecast should be equal to the current balance
  MyMoneyMoney b_checking = file->balance(a_checking.id(), QDate::currentDate());

  QVERIFY(a.forecastBalance(a_checking, QDate::currentDate().addDays(1)) == b_checking);
  QVERIFY(a.forecastBalance(a_checking, QDate::currentDate().addDays(2)) == b_checking);
  QVERIFY(a.forecastBalance(a_checking, QDate::currentDate().addDays(3)) == b_checking);
  QVERIFY(a.forecastBalance(a_checking, QDate::currentDate()) == b_checking);
  //credit had a variation so the forecast should be different for each day
  MyMoneyMoney b_credit = file->balance(a_credit.id(), QDate::currentDate());

  QVERIFY(a.forecastBalance(a_credit, 0) == b_credit);
  QVERIFY(a.forecastBalance(a_credit, QDate::currentDate().addDays(1)) == (b_credit + (moT2 - moT1)));
  QVERIFY(a.forecastBalance(a_credit, QDate::currentDate().addDays(2)) == (b_credit + ((moT2 - moT1)*2)));
  QVERIFY(a.forecastBalance(a_credit, QDate::currentDate().addDays(3)) == b_credit + ((moT2 - moT1)*3));

  a.setHistoryMethod(1); //weighted moving average
  a.doForecast();

  QVERIFY(a.forecastBalance(a_credit, 0) == b_credit);
  QVERIFY(a.forecastBalance(a_credit, QDate::currentDate().addDays(1)) == (b_credit + (moT2 - moT1)));
  QVERIFY(a.forecastBalance(a_credit, QDate::currentDate().addDays(2)) == (b_credit + ((moT2 - moT1)*2)));
  QVERIFY(a.forecastBalance(a_credit, QDate::currentDate().addDays(3)) == b_credit + ((moT2 - moT1)*3));

  //insert transactions outside the forecast period. The calculation should be the same.
  TransactionHelper t4(QDate::currentDate().addDays(-2), MyMoneySplit::actionName(eMyMoney::Split::Action::Deposit), -moT2, acCredit, acParent);
  TransactionHelper t5(QDate::currentDate().addDays(-10), MyMoneySplit::actionName(eMyMoney::Split::Action::Deposit), -moT2, acCredit, acParent);
  TransactionHelper t6(QDate::currentDate().addDays(-3), MyMoneySplit::actionName(eMyMoney::Split::Action::Deposit), -moT2, acCredit, acParent);

  a.setForecastMethod(1);
  a.setForecastDays(3);
  a.setAccountsCycle(1);
  a.setForecastCycles(1);
  a.setBeginForecastDay(0);
  a.setHistoryMethod(0); //moving average
  a.doForecast();
  //check forecast
  b_credit = file->balance(a_credit.id(), QDate::currentDate());
  MyMoneyMoney b_credit_1_exp = (b_credit + ((moT2 - moT1)));
  MyMoneyMoney b_credit_2 = a.forecastBalance(a_credit, QDate::currentDate().addDays(2));
  MyMoneyMoney b_credit_2_exp = (b_credit + ((moT2 - moT1) * 2));
  QVERIFY(a.forecastBalance(a_credit, QDate::currentDate()) == file->balance(a_credit.id(), QDate::currentDate()));
  QVERIFY(a.forecastBalance(a_credit, QDate::currentDate().addDays(1)) == b_credit + (moT2 - moT1));
  QVERIFY(a.forecastBalance(a_credit, QDate::currentDate().addDays(2)) == b_credit + ((moT2 - moT1)*2));
  QVERIFY(a.forecastBalance(a_credit, QDate::currentDate().addDays(3)) == b_credit + ((moT2 - moT1)*3));

  //test weighted moving average
  a.setForecastMethod(1);
  a.setForecastDays(3);
  a.setAccountsCycle(1);
  a.setForecastCycles(3);
  a.setBeginForecastDay(0);
  a.setHistoryMethod(1);
  a.doForecast();

  QVERIFY(a.forecastBalance(a_credit, 0) == b_credit);
  QVERIFY(a.forecastBalance(a_credit, QDate::currentDate().addDays(1)) == (b_credit + (((moT2 - moT1)*3 + moT2*2 + moT2) / MyMoneyMoney(6, 1))));

}

void MyMoneyForecastTest::testGetForecastAccountList()
{
  MyMoneyForecast a;
  MyMoneyAccount a_checking = file->account(acChecking);
  MyMoneyAccount a_parent = file->account(acParent);
  QList<MyMoneyAccount> b;

  b = a.forecastAccountList();
  //check that it contains asset account, but not expense accounts
  QVERIFY(b.contains(a_checking));
  QVERIFY(!b.contains(a_parent));

}

void MyMoneyForecastTest::testCalculateAccountTrend()
{
  //set up environment
  TransactionHelper t1(QDate::currentDate().addDays(-3), MyMoneySplit::actionName(eMyMoney::Split::Action::Deposit), -moT2, acChecking, acSolo);
  MyMoneyAccount a_checking = file->account(acChecking);

  //test invalid arguments

  try {
    MyMoneyForecast::calculateAccountTrend(a_checking, 0);
  } catch (const MyMoneyException &e) {
    QVERIFY(QString::fromLatin1(e.what()).startsWith(QLatin1String("Illegal arguments when calling calculateAccountTrend. trendDays must be higher than 0")));
  }
  try {
    MyMoneyForecast::calculateAccountTrend(a_checking, -10);
  } catch (const MyMoneyException &e) {
    QVERIFY(QString::fromLatin1(e.what()).startsWith(QLatin1String("Illegal arguments when calling calculateAccountTrend. trendDays must be higher than 0")));
  }

  //test that it calculates correctly
  QVERIFY(MyMoneyForecast::calculateAccountTrend(a_checking , 3) == moT2 / MyMoneyMoney(3, 1));

  //test that it works for all kind of accounts
  MyMoneyAccount a_solo = file->account(acSolo);
  MyMoneyMoney soloTrend = MyMoneyForecast::calculateAccountTrend(a_solo, 3);
  MyMoneyMoney soloTrendExp = -moT2 / MyMoneyMoney(3, 1);
  QVERIFY(MyMoneyForecast::calculateAccountTrend(a_solo, 3) == -moT2 / MyMoneyMoney(3, 1));

  //test that it does not take into account the transactions of the opening date of the account
  MyMoneyAccount a_cash = file->account(acCash);
  TransactionHelper t2(QDate::currentDate().addDays(-2), MyMoneySplit::actionName(eMyMoney::Split::Action::Deposit), moT2, acCash, acParent);
  TransactionHelper t3(QDate::currentDate().addDays(-1), MyMoneySplit::actionName(eMyMoney::Split::Action::Deposit), moT1, acCash, acParent);
  QVERIFY(MyMoneyForecast::calculateAccountTrend(a_cash, 3) == -moT1);

}

void MyMoneyForecastTest::testGetForecastBalance()
{
  //set up environment
  MyMoneyForecast a;

  TransactionHelper t1(QDate::currentDate().addDays(-1), MyMoneySplit::actionName(eMyMoney::Split::Action::Withdrawal), this->moT1, acChecking, acSolo);
  TransactionHelper t2(QDate::currentDate().addDays(-1), MyMoneySplit::actionName(eMyMoney::Split::Action::Deposit), -(this->moT2), acCredit, acParent);
  TransactionHelper t3(QDate::currentDate().addDays(-1), MyMoneySplit::actionName(eMyMoney::Split::Action::Transfer), this->moT1, acCredit, acChecking);

  a.setForecastMethod(1);
  a.setForecastDays(3);
  a.setAccountsCycle(1);
  a.setForecastCycles(1);
  a.setHistoryMethod(0);
  a.doForecast();

  MyMoneyAccount a_checking = file->account(acChecking);
  MyMoneyAccount a_credit = file->account(acCredit);

  //test invalid arguments
  QVERIFY(a.forecastBalance(a_checking, QDate::currentDate().addDays(-1)) == MyMoneyMoney());
  QVERIFY(a.forecastBalance(a_checking, QDate::currentDate().addDays(-10)) == MyMoneyMoney());
  QVERIFY(a.forecastBalance(a_checking, -1) == MyMoneyMoney());
  QVERIFY(a.forecastBalance(a_checking, -100) == MyMoneyMoney());

  //test a date outside the forecast days
  QVERIFY(a.forecastBalance(a_checking, QDate::currentDate().addDays(4)) == MyMoneyMoney());
  QVERIFY(a.forecastBalance(a_checking, 4) == MyMoneyMoney());
  QVERIFY(a.forecastBalance(a_checking, QDate::currentDate().addDays(10)) == MyMoneyMoney());
  QVERIFY(a.forecastBalance(a_checking, 10) == MyMoneyMoney());

  //test it returns valid results
  MyMoneyMoney b_credit = file->balance(a_credit.id(), QDate::currentDate());
  QVERIFY(a.forecastBalance(a_credit, QDate::currentDate()) == file->balance(a_credit.id(), QDate::currentDate()));
  QVERIFY(a.forecastBalance(a_credit, QDate::currentDate().addDays(1)) == b_credit + (moT2 - moT1));
  QVERIFY(a.forecastBalance(a_credit, QDate::currentDate().addDays(2)) == b_credit + ((moT2 - moT1)*2));
  QVERIFY(a.forecastBalance(a_credit, QDate::currentDate().addDays(3)) == b_credit + ((moT2 - moT1)*3));
}

void MyMoneyForecastTest::testIsForecastAccount()
{
  MyMoneyForecast a;

  MyMoneyAccount a_checking = file->account(acChecking);
  MyMoneyAccount a_solo = file->account(acSolo);
  MyMoneyAccount a_investment = file->account(acInvestment);

  //test an invalid account
  QVERIFY(a.isForecastAccount(a_solo) == false);
  QVERIFY(a.isForecastAccount(a_investment) == true);

  //test a valid account
  QVERIFY(a.isForecastAccount(a_checking) == true);

}

void MyMoneyForecastTest::testDoFutureScheduledForecast()
{
  //set up future transactions
  MyMoneyForecast a;

  MyMoneyAccount a_cash = file->account(acCash);
  TransactionHelper t1(QDate::currentDate().addDays(1), MyMoneySplit::actionName(eMyMoney::Split::Action::Deposit), -moT1, acCash, acParent);
  TransactionHelper t2(QDate::currentDate().addDays(2), MyMoneySplit::actionName(eMyMoney::Split::Action::Deposit), -moT2, acCash, acParent);
  TransactionHelper t3(QDate::currentDate().addDays(3), MyMoneySplit::actionName(eMyMoney::Split::Action::Deposit), -moT3, acCash, acParent);
  TransactionHelper t4(QDate::currentDate().addDays(10), MyMoneySplit::actionName(eMyMoney::Split::Action::Deposit), -moT4, acCash, acParent);

  a.setForecastMethod(0);
  a.setForecastDays(3);
  a.setAccountsCycle(1);
  a.setForecastCycles(1);
  a.doForecast();

  MyMoneyMoney b_cash = file->balance(a_cash.id(), QDate::currentDate());

  //test valid results
  QVERIFY(a.forecastBalance(a_cash, QDate::currentDate()) == b_cash);
  QVERIFY(a.forecastBalance(a_cash, QDate::currentDate().addDays(1)) == b_cash + moT1);
  QVERIFY(a.forecastBalance(a_cash, QDate::currentDate().addDays(2)) == b_cash + moT1 + moT2);
  QVERIFY(a.forecastBalance(a_cash, QDate::currentDate().addDays(3)) == b_cash + moT1 + moT2 + moT3);
}

void MyMoneyForecastTest::testScheduleForecast()
{
  //set up schedule environment for testing
  MyMoneyAccount a_cash = file->account(acCash);
  MyMoneyAccount a_parent = file->account(acParent);

  MyMoneyFileTransaction ft;
  MyMoneySchedule sch("A Name",
                      Schedule::Type::Bill,
                      Schedule::Occurrence::Weekly, 1,
                      Schedule::PaymentType::DirectDebit,
                      QDate::currentDate().addDays(1),
                      QDate(),
                      true,
                      true);

  MyMoneyTransaction t;
  t.setPostDate(QDate::currentDate().addDays(1));
  t.setEntryDate(QDate::currentDate().addDays(1));
  //t.setId("T000000000000000001");
  t.setBankID("BID");
  t.setMemo("Wohnung:Miete");
  t.setCommodity("USD");
  t.setValue("key", "value");

  MyMoneySplit s;
  s.setPayeeId("P000001");
  s.setShares(moT2);
  s.setValue(moT2);
  s.setAccountId(a_parent.id());
  s.setBankID("SPID1");
  s.setReconcileFlag(eMyMoney::Split::State::Reconciled);
  t.addSplit(s);

  s.setPayeeId("P000001");
  s.setShares(-moT2);
  s.setValue(-moT2);
  s.setAccountId(a_cash.id());
  s.setBankID("SPID2");
  s.setReconcileFlag(eMyMoney::Split::State::Cleared);
  s.clearId();
  t.addSplit(s);

  sch.setTransaction(t);

  file->addSchedule(sch);
  ft.commit();

  MyMoneyFileTransaction ft3;
  MyMoneySchedule sch3("A Name1",
                       Schedule::Type::Bill,
                       Schedule::Occurrence::Weekly, 1,
                       Schedule::PaymentType::DirectDebit,
                       QDate::currentDate().addDays(5),
                       QDate(),
                       true,
                       true);

  //sch.setLastPayment(QDate::currentDate());
  //sch.recordPayment(QDate::currentDate().addDays(1));
  //sch.setId("SCH0001");

  MyMoneyTransaction t3;
  t3.setPostDate(QDate::currentDate().addDays(5));
  t3.setEntryDate(QDate::currentDate().addDays(5));
  //t.setId("T000000000000000001");
  t3.setBankID("BID");
  t3.setMemo("Wohnung:Miete");
  t3.setCommodity("USD");
  t3.setValue("key", "value");

  MyMoneySplit s3;
  s3.setPayeeId("P000001");
  s3.setShares(moT2);
  s3.setValue(moT2);
  s3.setAccountId(a_parent.id());
  s3.setBankID("SPID1");
  s3.setReconcileFlag(eMyMoney::Split::State::Reconciled);
  t3.addSplit(s3);

  s3.setPayeeId("P000001");
  s3.setShares(-moT2);
  s3.setValue(-moT2);
  s3.setAccountId(a_cash.id());
  s3.setBankID("SPID2");
  s3.setReconcileFlag(eMyMoney::Split::State::Cleared);
  s3.clearId();
  t3.addSplit(s3);

  sch3.setTransaction(t3);

  file->addSchedule(sch3);
  ft3.commit();


  MyMoneyFileTransaction ft2;
  MyMoneySchedule sch2("A Name2",
                       Schedule::Type::Bill,
                       Schedule::Occurrence::Weekly, 1,
                       Schedule::PaymentType::DirectDebit,
                       QDate::currentDate().addDays(2),
                       QDate(),
                       true,
                       true);

  //sch.setLastPayment(QDate::currentDate());
  //sch.recordPayment(QDate::currentDate().addDays(1));
  //sch.setId("SCH0001");

  MyMoneyTransaction t2;
  t2.setPostDate(QDate::currentDate().addDays(2));
  t2.setEntryDate(QDate::currentDate().addDays(2));
  //t.setId("T000000000000000001");
  t2.setBankID("BID");
  t2.setMemo("Wohnung:Miete");
  t2.setCommodity("USD");
  t2.setValue("key", "value");

  MyMoneySplit s2;
  s2.setPayeeId("P000001");
  s2.setShares(moT1);
  s2.setValue(moT1);
  s2.setAccountId(a_parent.id());
  s2.setBankID("SPID1");
  s2.setReconcileFlag(eMyMoney::Split::State::Reconciled);
  t2.addSplit(s2);

  s2.setPayeeId("P000001");
  s2.setShares(-moT1);
  s2.setValue(-moT1);
  s2.setAccountId(a_cash.id());
  s2.setBankID("SPID2");
  s2.setReconcileFlag(eMyMoney::Split::State::Cleared);
  s2.clearId();
  t2.addSplit(s2);

  sch2.setTransaction(t2);

  file->addSchedule(sch2);

  ft2.commit();

  //run forecast
  MyMoneyForecast a;
  a.setForecastMethod(0);
  a.setForecastDays(3);
  a.setAccountsCycle(1);
  a.setForecastCycles(1);
  a.doForecast();

  //check result for single schedule
  MyMoneyMoney b_cash = file->balance(a_cash.id(), QDate::currentDate());
  MyMoneyMoney b_cash1 = a.forecastBalance(a_cash, QDate::currentDate().addDays(1));

  //test valid results
  QVERIFY(a.forecastBalance(a_cash, QDate::currentDate()) == b_cash);
  QVERIFY(a.forecastBalance(a_cash, QDate::currentDate().addDays(1)) == b_cash - moT2);
  QVERIFY(a.forecastBalance(a_cash, QDate::currentDate().addDays(2)) == b_cash - moT2 - moT1);
}


void MyMoneyForecastTest::testDaysToMinimumBalance()
{
  //setup environment
  MyMoneyForecast a;

  MyMoneyAccount a_cash = file->account(acCash);
  MyMoneyAccount a_credit = file->account(acCredit);
  MyMoneyAccount a_parent = file->account(acParent);
  a_cash.setValue("minBalanceAbsolute", "50");
  a_credit.setValue("minBalanceAbsolute", "50");
  TransactionHelper t1(QDate::currentDate().addDays(-1), MyMoneySplit::actionName(eMyMoney::Split::Action::Deposit), -moT1, acCash, acParent);
  TransactionHelper t2(QDate::currentDate().addDays(2), MyMoneySplit::actionName(eMyMoney::Split::Action::Deposit), moT2, acCash, acParent);
  TransactionHelper t3(QDate::currentDate().addDays(-1), MyMoneySplit::actionName(eMyMoney::Split::Action::Withdrawal), -moT1, acCredit, acParent);
  TransactionHelper t4(QDate::currentDate().addDays(4), MyMoneySplit::actionName(eMyMoney::Split::Action::Withdrawal), moT5, acCredit, acParent);

  a.setForecastMethod(0);
  a.setForecastDays(3);
  a.setAccountsCycle(1);
  a.setForecastCycles(1);
  a.setBeginForecastDay(0);
  a.doForecast();

  //test invalid arguments
  MyMoneyAccount nullAcc;
  QVERIFY(a.daysToMinimumBalance(nullAcc) == -1);

  //test when not a forecast account
  QVERIFY(a.daysToMinimumBalance(a_parent) == -1);

  //test it warns when inside the forecast period
  QVERIFY(a.daysToMinimumBalance(a_cash) == 2);

  //test it does not warn when it will be outside of the forecast period
  QVERIFY(a.daysToMinimumBalance(a_credit) == -1);
}
void MyMoneyForecastTest::testDaysToZeroBalance()
{
  //set up environment
  MyMoneyAccount a_Solo = file->account(acSolo);
  MyMoneyAccount a_Cash = file->account(acCash);
  MyMoneyAccount a_Credit = file->account(acCredit);

  //MyMoneyFileTransaction ft;
  TransactionHelper t1(QDate::currentDate().addDays(2), MyMoneySplit::actionName(eMyMoney::Split::Action::Withdrawal), -moT1, acChecking, acSolo);
  TransactionHelper t2(QDate::currentDate().addDays(2), MyMoneySplit::actionName(eMyMoney::Split::Action::Transfer), (moT5), acCash, acCredit);
  TransactionHelper t3(QDate::currentDate().addDays(2), MyMoneySplit::actionName(eMyMoney::Split::Action::Withdrawal), (moT5*100), acCredit, acParent);
  //ft.commit();

  MyMoneyForecast a;
  a.setForecastMethod(0);
  a.setForecastDays(30);
  a.setAccountsCycle(1);
  a.setForecastCycles(3);
  a.doForecast();

  //test invalid arguments
  MyMoneyAccount nullAcc;
  try {
    auto days = a.daysToZeroBalance(nullAcc);
    Q_UNUSED(days)
  } catch (const MyMoneyException &) {
    QFAIL("Unexpected exception");
  }

  //test when not a forecast account
  MyMoneyAccount a_solo = file->account(acSolo);
  auto iSolo = a.daysToZeroBalance(a_Solo);

  QVERIFY(iSolo == -2);

  //test it warns when inside the forecast period

  MyMoneyMoney fCash = a.forecastBalance(a_Cash, QDate::currentDate().addDays(2));

  QVERIFY(a.daysToZeroBalance(a_Cash) == 2);

  //test it does not warn when it will be outside of the forecast period

}

void MyMoneyForecastTest::testSkipOpeningDate()
{
  //set up environment
  MyMoneyForecast a;

  TransactionHelper t1(QDate::currentDate().addDays(-2), MyMoneySplit::actionName(eMyMoney::Split::Action::Withdrawal), this->moT1, acCash, acSolo);
  TransactionHelper t2(QDate::currentDate().addDays(-1), MyMoneySplit::actionName(eMyMoney::Split::Action::Withdrawal), this->moT2, acCash, acSolo);

  a.setForecastMethod(1);
  a.setForecastDays(3);
  a.setAccountsCycle(2);
  a.setForecastCycles(1);
  a.setHistoryMethod(0);
  a.doForecast();

  MyMoneyAccount a_cash = file->account(acCash);

  //test it has no variation because it skipped the variation of the opening date
  MyMoneyMoney b_cash = file->balance(a_cash.id(), QDate::currentDate());
  QVERIFY(a.skipOpeningDate() == true);
  QVERIFY(a.forecastBalance(a_cash, QDate::currentDate()) == b_cash);
  QVERIFY(a.forecastBalance(a_cash, QDate::currentDate().addDays(1)) == b_cash);
  QVERIFY(a.forecastBalance(a_cash, QDate::currentDate().addDays(2)) == b_cash - moT2);
  QVERIFY(a.forecastBalance(a_cash, QDate::currentDate().addDays(3)) == b_cash - moT2);
}

void MyMoneyForecastTest::testAccountMinimumBalanceDateList()
{

  //set up environment
  MyMoneyForecast a;

  TransactionHelper t1(QDate::currentDate().addDays(-2), MyMoneySplit::actionName(eMyMoney::Split::Action::Withdrawal), this->moT1, acCash, acSolo);
  TransactionHelper t2(QDate::currentDate().addDays(-1), MyMoneySplit::actionName(eMyMoney::Split::Action::Withdrawal), this->moT2, acCash, acSolo);

  a.setForecastMethod(1);
  a.setForecastDays(6);
  a.setAccountsCycle(2);
  a.setForecastCycles(3);
  a.setHistoryMethod(0);
  a.setBeginForecastDay(QDate::currentDate().addDays(1).day());
  a.doForecast();

  MyMoneyAccount a_cash = file->account(acCash);

  //test
  QList<QDate> dateList;
  dateList = a.accountMinimumBalanceDateList(a_cash);

  QList<QDate>::iterator it = dateList.begin();

  QDate minDate = *it;

  QVERIFY(minDate == QDate::currentDate().addDays(2));
  it++;
  minDate = *it;
  QVERIFY(minDate == QDate::currentDate().addDays(4));
  it++;
  minDate = *it;
  QVERIFY(minDate == QDate::currentDate().addDays(6));

}

void MyMoneyForecastTest::testAccountMaximumBalanceDateList()
{
  //set up environment
  MyMoneyForecast a;

  TransactionHelper t1(QDate::currentDate().addDays(-2), MyMoneySplit::actionName(eMyMoney::Split::Action::Withdrawal), this->moT1, acCash, acSolo);
  TransactionHelper t2(QDate::currentDate().addDays(-1), MyMoneySplit::actionName(eMyMoney::Split::Action::Withdrawal), this->moT2, acCash, acSolo);

  a.setForecastMethod(1);
  a.setForecastDays(6);
  a.setAccountsCycle(2);
  a.setForecastCycles(3);
  a.setHistoryMethod(0);
  a.setBeginForecastDay(QDate::currentDate().addDays(1).day());
  a.doForecast();

  MyMoneyAccount a_cash = file->account(acCash);

  //test
  QList<QDate> dateList;
  dateList = a.accountMaximumBalanceDateList(a_cash);

  QList<QDate>::iterator it = dateList.begin();

  QDate maxDate = *it;

  QVERIFY(maxDate == QDate::currentDate().addDays(1));
  it++;
  maxDate = *it;
  QVERIFY(maxDate == QDate::currentDate().addDays(3));
  it++;
  maxDate = *it;
  QVERIFY(maxDate == QDate::currentDate().addDays(5));


}

void MyMoneyForecastTest::testAccountAverageBalance()
{
  //set up environment
  MyMoneyForecast a;

  TransactionHelper t1(QDate::currentDate().addDays(-2), MyMoneySplit::actionName(eMyMoney::Split::Action::Withdrawal), this->moT1, acCash, acSolo);
  TransactionHelper t2(QDate::currentDate().addDays(-1), MyMoneySplit::actionName(eMyMoney::Split::Action::Withdrawal), this->moT2, acCash, acSolo);

  a.setForecastMethod(1);
  a.setForecastDays(3);
  a.setAccountsCycle(2);
  a.setForecastCycles(1);
  a.setBeginForecastDay(0);
  a.doForecast();

  MyMoneyAccount a_cash = file->account(acCash);

  //test
  MyMoneyMoney b_cash1 = a.forecastBalance(a_cash, QDate::currentDate().addDays(1));
  MyMoneyMoney b_cash2 = a.forecastBalance(a_cash, QDate::currentDate().addDays(2));
  MyMoneyMoney b_cash3 = a.forecastBalance(a_cash, QDate::currentDate().addDays(3));

  MyMoneyMoney average = (b_cash1 + b_cash2 + b_cash3) / MyMoneyMoney(3, 1);


  QVERIFY(a.accountAverageBalance(a_cash) == average);
}

void MyMoneyForecastTest::testBeginForecastDate()
{
  //set up environment
  MyMoneyForecast a;
  QDate beginDate;
  qint64 beginDay;

  a.setForecastMethod(1);
  a.setForecastDays(90);
  a.setAccountsCycle(14);
  a.setForecastCycles(3);
  a.setBeginForecastDay(0);
  a.doForecast();

  //test when using old method without begin day
  QVERIFY(QDate::currentDate() == a.beginForecastDate());

  //setup begin to last day of month
  a.setBeginForecastDay(31);
  beginDay = a.beginForecastDay();
  a.doForecast();

  //test
  if (QDate::currentDate().day() < beginDay) {
    if (QDate::currentDate().daysInMonth() < beginDay)
      beginDay = QDate::currentDate().daysInMonth();

    beginDate = QDate(QDate::currentDate().year(), QDate::currentDate().month(), beginDay);

    QVERIFY(beginDate == a.beginForecastDate());
  }

  //setup begin day to same date
  a.setBeginForecastDay(QDate::currentDate().day());
  beginDay = a.beginForecastDay();
  a.doForecast();

  QVERIFY(QDate::currentDate() == a.beginForecastDate());

  //setup to first day of month with small interval
  a.setBeginForecastDay(1);
  a.setAccountsCycle(1);
  beginDay = a.beginForecastDay();
  a.doForecast();

  //test
  if (QDate::currentDate() == a.beginForecastDate()) {
    QVERIFY(QDate::currentDate() == a.beginForecastDate());
  } else {
    beginDay = ((((QDate::currentDate().day() - beginDay) / a.accountsCycle()) + 1) * a.accountsCycle()) + beginDay;
    if (beginDay > QDate::currentDate().daysInMonth())
      beginDay = QDate::currentDate().daysInMonth();
    beginDate = QDate(QDate::currentDate().year(), QDate::currentDate().month(), beginDay);
    if (QDate::currentDate().day() == QDate::currentDate().daysInMonth()) {
      std::cout << std::endl << "testBeginForecastDate(): test of first day of month with small interval skipped because it is the last day of month" << std::endl;
    } else {
      QVERIFY(beginDate == a.beginForecastDate());
    }
  }

  //setup to test when current date plus cycle equals begin day
  a.setAccountsCycle(14);
  beginDay = QDate::currentDate().addDays(14).day();
  a.setBeginForecastDay(beginDay);
  beginDate = QDate::currentDate().addDays(14);
  a.doForecast();

  //test
  QVERIFY(beginDate == a.beginForecastDate());

  //setup to test when the begin day will be next month
  a.setBeginForecastDay(1);
  a.setAccountsCycle(40);
  a.doForecast();

  beginDate = QDate(QDate::currentDate().addMonths(1).year(), QDate::currentDate().addMonths(1).month(), 1);

  //test
  if (QDate::currentDate().day() > 1) {
    QVERIFY(beginDate == a.beginForecastDate());
  } else {
    //test is not valid if today is 1st of month
    std::cout << std::endl << "testBeginForecastDate(): test of first day of month skipped because current day is 1st of month" << std::endl;
  }
}

void MyMoneyForecastTest::testHistoryDays()
{
  MyMoneyForecast a;

  QVERIFY(a.historyStartDate() == QDate::currentDate().addDays(-a.forecastCycles()*a.accountsCycle()));
  QVERIFY(a.historyEndDate() == QDate::currentDate().addDays(-1));
  QVERIFY(a.historyDays() == a.forecastCycles()*a.accountsCycle());

  a.setForecastMethod(1);
  a.setForecastDays(90);
  a.setAccountsCycle(14);
  a.setForecastCycles(3);
  a.setBeginForecastDay(0);
  a.doForecast();

  QVERIFY(a.historyStartDate() == QDate::currentDate().addDays(-14*3));
  QVERIFY(a.historyDays() == (14*3));
  QVERIFY(a.historyEndDate() == (QDate::currentDate().addDays(-1)));
}

void MyMoneyForecastTest::testCreateBudget()
{
  //set up environment
  MyMoneyForecast a;
  MyMoneyForecast b;
  MyMoneyBudget budget;

  TransactionHelper t1(QDate(2005, 1, 3), MyMoneySplit::actionName(eMyMoney::Split::Action::Withdrawal), this->moT1, acCash, acSolo);
  TransactionHelper t2(QDate(2005, 1, 15), MyMoneySplit::actionName(eMyMoney::Split::Action::Withdrawal), this->moT2, acCash, acParent);
  TransactionHelper t3(QDate(2005, 1, 30), MyMoneySplit::actionName(eMyMoney::Split::Action::Withdrawal), this->moT3, acCash, acSolo);
  TransactionHelper t4(QDate(2006, 1, 25), MyMoneySplit::actionName(eMyMoney::Split::Action::Withdrawal), this->moT4, acCash, acParent);
  TransactionHelper t5(QDate(2005, 4, 3), MyMoneySplit::actionName(eMyMoney::Split::Action::Withdrawal), this->moT1, acCash, acSolo);
  TransactionHelper t6(QDate(2006, 5, 15), MyMoneySplit::actionName(eMyMoney::Split::Action::Withdrawal), this->moT2, acCash, acParent);
  TransactionHelper t7(QDate(2005, 8, 3), MyMoneySplit::actionName(eMyMoney::Split::Action::Withdrawal), this->moT3, acCash, acSolo);
  TransactionHelper t8(QDate(2006, 9, 15), MyMoneySplit::actionName(eMyMoney::Split::Action::Withdrawal), this->moT4, acCash, acParent);

  a.setHistoryMethod(0);
  a.setForecastMethod(1);
  a.createBudget(budget, QDate(2005, 1, 1), QDate(2006, 12, 31), QDate(2007, 1, 1), QDate(2007, 12, 31), true);

  //test
  MyMoneyAccount a_solo = file->account(acSolo);
  MyMoneyAccount a_parent = file->account(acParent);

  //test it has no variation because it skipped the variation of the opening date
  QVERIFY(a.forecastBalance(a_solo, QDate(2007, 1, 1)) == ((moT1 + moT3) / MyMoneyMoney(2, 1)));
  QVERIFY(a.forecastBalance(a_parent, QDate(2007, 1, 1)) == ((moT2 + moT4) / MyMoneyMoney(2, 1)));
  QVERIFY(a.forecastBalance(a_solo, QDate(2007, 4, 1)) == ((moT1) / MyMoneyMoney(2, 1)));
  QVERIFY(a.forecastBalance(a_parent, QDate(2007, 5, 1)) == ((moT2) / MyMoneyMoney(2, 1)));
  QVERIFY(a.forecastBalance(a_solo, QDate(2007, 8, 1)) == ((moT3) / MyMoneyMoney(2, 1)));
  QVERIFY(a.forecastBalance(a_parent, QDate(2007, 9, 1)) == ((moT4) / MyMoneyMoney(2, 1)));
  //test the budget object returned by the method
  QVERIFY(budget.account(a_parent.id()).period(QDate(2007, 9, 1)).amount() == ((moT4) / MyMoneyMoney(2, 1)));

  //setup test for a length lower than a year
  b.setForecastMethod(1);
  b.setHistoryMethod(0);
  b.createBudget(budget, QDate(2005, 1, 1), QDate(2005, 6, 30), QDate(2007, 1, 1), QDate(2007, 6, 30), true);

  //test
  QVERIFY(b.forecastBalance(a_solo, QDate(2007, 1, 1)) == (moT1 + moT3));
  QVERIFY(b.forecastBalance(a_parent, QDate(2007, 1, 1)) == (moT2));
  QVERIFY(b.forecastBalance(a_solo, QDate(2007, 4, 1)) == (moT1));
  QVERIFY(b.forecastBalance(a_parent, QDate(2007, 5, 1)) == (MyMoneyMoney()));

  //set up schedule environment for testing
  MyMoneyAccount a_cash = file->account(acCash);

  MyMoneyFileTransaction ft;
  MyMoneySchedule sch("A Name",
                      Schedule::Type::Bill,
                      Schedule::Occurrence::Monthly, 1,
                      Schedule::PaymentType::DirectDebit,
                      QDate::currentDate(),
                      QDate(),
                      true,
                      true);

  MyMoneyTransaction t10;
  t10.setPostDate(QDate::currentDate().addMonths(1));
  t10.setEntryDate(QDate::currentDate().addMonths(1));
  //t.setId("T000000000000000001");
  t10.setBankID("BID");
  t10.setMemo("Wohnung:Miete");
  t10.setCommodity("USD");
  t10.setValue("key", "value");

  MyMoneySplit s;
  s.setPayeeId("P000001");
  s.setShares(moT2);
  s.setValue(moT2);
  s.setAccountId(a_parent.id());
  s.setBankID("SPID1");
  s.setReconcileFlag(eMyMoney::Split::State::Reconciled);
  t10.addSplit(s);

  s.setPayeeId("P000001");
  s.setShares(-moT2);
  s.setValue(-moT2);
  s.setAccountId(a_cash.id());
  s.setBankID("SPID2");
  s.setReconcileFlag(eMyMoney::Split::State::Cleared);
  s.clearId();
  t10.addSplit(s);

  sch.setTransaction(t10);

  file->addSchedule(sch);
  ft.commit();

  //run forecast
  MyMoneyForecast c;
  c.setForecastMethod(0);
  c.setForecastCycles(1);
  c.createBudget(budget, QDate::currentDate().addYears(-2), QDate::currentDate().addYears(-1), QDate::currentDate().addMonths(-2), QDate::currentDate().addMonths(6), true);

  MyMoneyMoney c_parent = c.forecastBalance(a_parent, QDate(QDate::currentDate().addMonths(1).year(), QDate::currentDate().addMonths(1).month(), 1));

  //test valid results
  QCOMPARE(c.forecastBalance(a_parent, QDate(QDate::currentDate().addMonths(1).year(), QDate::currentDate().addMonths(1).month(), 1)).formatMoney(4), moT2.formatMoney(4));
}

void MyMoneyForecastTest::testLinearRegression()
{
  //set up environment
  MyMoneyForecast a;

  MyMoneyAccount a_checking = file->account(acChecking);
  MyMoneyAccount a_credit = file->account(acCredit);

  //setup some transactions
  TransactionHelper t1(QDate::currentDate().addDays(-1), MyMoneySplit::actionName(eMyMoney::Split::Action::Withdrawal), this->moT1, acChecking, acSolo);
  TransactionHelper t2(QDate::currentDate().addDays(-1), MyMoneySplit::actionName(eMyMoney::Split::Action::Deposit), -(this->moT2), acCredit, acParent);
  TransactionHelper t3(QDate::currentDate().addDays(-1), MyMoneySplit::actionName(eMyMoney::Split::Action::Transfer), this->moT1, acCredit, acChecking);

//TODO Add tests specific for linear regression


}
