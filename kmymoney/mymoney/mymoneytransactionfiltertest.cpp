/***************************************************************************
                        mymoneytransactionfiltertest.cpp
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "mymoneytransactionfiltertest.h"

#include <QtTest/QtTest>

#include "mymoneytransactionfilter.h"
#include "mymoneyfile.h"
#include "mymoneyseqaccessmgr.h"

#undef QVERIFY
#define QVERIFY(statement) \
  QTest::qVerify((statement), #statement, "", __FILE__, __LINE__)

#undef QCOMPARE
#define QCOMPARE(actual, expected) \
 QTest::qCompare(actual, expected, #actual, #expected, __FILE__, __LINE__)

using namespace std;

// copied from reports/reportstestcommon.cpp
QString makeAccount(const QString& _name, MyMoneyAccount::accountTypeE _type, MyMoneyMoney _balance, const QDate& _open, const QString& _parent, QString _currency = "", bool _taxReport = false, bool _openingBalance = false);

QString makeAccount(const QString& _name, MyMoneyAccount::accountTypeE _type, MyMoneyMoney _balance, const QDate& _open, const QString& _parent, QString _currency, bool _taxReport, bool _openingBalance)
{
  MyMoneyAccount info;
  MyMoneyFileTransaction ft;

  info.setName(_name);
  info.setAccountType(_type);
  info.setOpeningDate(_open);
  if (!_currency.isEmpty())
    info.setCurrencyId(_currency);
  else
    info.setCurrencyId(MyMoneyFile::instance()->baseCurrency().id());

  if (_taxReport)
    info.setValue("Tax", "Yes");

  if (_openingBalance)
    info.setValue("OpeningBalanceAccount", "Yes");

  MyMoneyAccount parent = MyMoneyFile::instance()->account(_parent);
  MyMoneyFile::instance()->addAccount(info, parent);
  // create the opening balance transaction if any
  if (!_balance.isZero()) {
    MyMoneySecurity sec = MyMoneyFile::instance()->currency(info.currencyId());
    MyMoneyFile::instance()->openingBalanceAccount(sec);
    MyMoneyFile::instance()->createOpeningBalanceTransaction(info, _balance);
  }
  ft.commit();

  return info.id();
}

void MyMoneyTransactionFilterTest::initTestCase()
{
  MyMoneySeqAccessMgr *storage = new MyMoneySeqAccessMgr;
  MyMoneyFile *file = MyMoneyFile::instance();
  file->attachStorage(storage);
  MyMoneyFileTransaction ft;

  file->addCurrency(MyMoneySecurity("USD", "US Dollar", "$"));
  file->setBaseCurrency(file->currency("USD"));

  MyMoneyPayee payeeTest("Payee 10.2");
  file->addPayee(payeeTest);
  payeeId = payeeTest.id();

  MyMoneyTag tag("Tag 10.2");
  file->addTag(tag);
  tagIdList << tag.id();

  QString acAsset = MyMoneyFile::instance()->asset().id();
  QString acExpense = (MyMoneyFile::instance()->expense().id());
  QString acIncome = (MyMoneyFile::instance()->income().id());
  acCheckingId = makeAccount("Account 10.2", MyMoneyAccount::Checkings, MyMoneyMoney(0.0), QDate(2004, 1, 1), acAsset);
  acExpenseId = makeAccount("Expense", MyMoneyAccount::Expense, MyMoneyMoney(), QDate(2004, 1, 11), acExpense);
  acIncomeId = makeAccount("Expense", MyMoneyAccount::Expense, MyMoneyMoney(), QDate(2004, 1, 11), acIncome);

  ft.commit();
}

void MyMoneyTransactionFilterTest::testMatchAmount()
{
  MyMoneySplit split;
  split.setShares(MyMoneyMoney(123.20));

  MyMoneyTransactionFilter filter;
  QVERIFY (filter.matchAmount(&split));

  filter.setAmountFilter(MyMoneyMoney("123.0"), MyMoneyMoney("124.0"));
  QVERIFY (filter.matchAmount(&split));
  filter.setAmountFilter(MyMoneyMoney("120.0"), MyMoneyMoney("123.0"));
  QVERIFY (!filter.matchAmount(&split));
}

void MyMoneyTransactionFilterTest::testMatchText()
{
  MyMoneySplit split;
  MyMoneyTransactionFilter filter;

  // no filter
  QVERIFY (filter.matchText(&split));

  filter.setTextFilter(QRegExp("10.2"), false);
  MyMoneyTransactionFilter filterInvert;
  filterInvert.setTextFilter(QRegExp("10.2"), true);
  MyMoneyTransactionFilter filterNotFound;
  filterNotFound.setTextFilter(QRegExp("10.5"), false);

  // memo
  split.setMemo("10.2");
  QVERIFY (filter.matchText(&split));
  QVERIFY (!filterInvert.matchText(&split));
  QVERIFY (!filterNotFound.matchText(&split));
  split.setMemo("");
  // payee
  split.setPayeeId(payeeId);
  QVERIFY (filter.matchText(&split));
  QVERIFY (!filterInvert.matchText(&split));
  QVERIFY (!filterNotFound.matchText(&split));
  split.setPayeeId("");
  // tag
  split.setTagIdList(tagIdList);
  QVERIFY (filter.matchText(&split));
  QVERIFY (!filterInvert.matchText(&split));
  QVERIFY (!filterNotFound.matchText(&split));
  split.setTagIdList(QStringList());
  // value
  split.setValue(MyMoneyMoney("10.2"));
  QVERIFY (filter.matchText(&split));
  QVERIFY (!filterInvert.matchText(&split));
  QVERIFY (!filterNotFound.matchText(&split));
  split.setValue(MyMoneyMoney("0.0"));
  // number
  split.setNumber("10.2");
  QVERIFY (filter.matchText(&split));
  QVERIFY (!filterInvert.matchText(&split));
  QVERIFY (!filterNotFound.matchText(&split));
  split.setNumber("0.0");
  // transaction id
  split.setTransactionId("10.2");
  QVERIFY (filter.matchText(&split));
  QVERIFY (!filterInvert.matchText(&split));
  QVERIFY (!filterNotFound.matchText(&split));
  split.setTransactionId("0.0");
  // account
  split.setAccountId(acCheckingId);
  QVERIFY (filter.matchText(&split));
  QVERIFY (!filterInvert.matchText(&split));
  QVERIFY (!filterNotFound.matchText(&split));
}

void MyMoneyTransactionFilterTest::testMatchSplit()
{
  qDebug() << "returns matchText() || matchAmount(), which are already tested";
}

void MyMoneyTransactionFilterTest::testMatchTransactionAll()
{
  MyMoneySplit split;
  split.setAccountId(acCheckingId);
  split.setShares(MyMoneyMoney(123.00));

  MyMoneySplit split2;
  split2.setAccountId(acExpenseId);
  split2.setShares(MyMoneyMoney(123.00));

  MyMoneyTransaction transaction;
  transaction.setPostDate(QDate(2014, 1, 2));
  transaction.addSplit(split);
  transaction.addSplit(split2);

  MyMoneyTransactionFilter filter;
  filter.setReportAllSplits(true);
  QVERIFY(filter.match(transaction));
  QCOMPARE(filter.matchingSplits().size(), 2);

  filter.setReportAllSplits(false);
  QVERIFY(filter.match(transaction));
  // FIXME: is it correct to return no splits ?
  QCOMPARE(filter.matchingSplits().size(), 0);
}

void MyMoneyTransactionFilterTest::testMatchTransactionAccount()
{
  MyMoneySplit split;
  split.setAccountId(acCheckingId);
  split.setShares(MyMoneyMoney(123.00));

  MyMoneySplit split2;
  split2.setAccountId(acExpenseId);
  split2.setShares(MyMoneyMoney(123.00));

  MyMoneyTransaction transaction;
  transaction.setPostDate(QDate(2014, 1, 2));
  transaction.addSplit(split);
  transaction.addSplit(split2);

  MyMoneyTransactionFilter filter;
  filter.addAccount(acCheckingId);
  filter.setReportAllSplits(true);
  filter.setConsiderCategory(false);
  QVERIFY(filter.match(transaction));
  QCOMPARE(filter.matchingSplits().size(), 1);

  filter.setReportAllSplits(false);
  filter.setConsiderCategory(false);
  QVERIFY(filter.match(transaction));
  QCOMPARE(filter.matchingSplits().size(), 1);

  filter.setReportAllSplits(false);
  filter.setConsiderCategory(true);
  QVERIFY(filter.match(transaction));
  QCOMPARE(filter.matchingSplits().size(), 1);

  filter.setReportAllSplits(true);
  filter.setConsiderCategory(true);
  QVERIFY(filter.match(transaction));
  QCOMPARE(filter.matchingSplits().size(), 1);
  filter.clear();
}

void MyMoneyTransactionFilterTest::testMatchTransactionCategory()
{
  MyMoneySplit split;
  split.setAccountId(acCheckingId);
  split.setShares(MyMoneyMoney(123.00));

  MyMoneySplit split2;
  split2.setAccountId(acExpenseId);
  split2.setShares(MyMoneyMoney(123.00));

  MyMoneyTransaction transaction;
  transaction.setPostDate(QDate(2014, 1, 2));
  transaction.addSplit(split);
  transaction.addSplit(split2);

  MyMoneyTransactionFilter filter;
  filter.addCategory(acExpenseId);
  filter.setReportAllSplits(true);
  filter.setConsiderCategory(true);
  QVERIFY(filter.match(transaction));
  QCOMPARE(filter.matchingSplits().size(), 2);

  filter.setConsiderCategory(false);
  QVERIFY(!filter.match(transaction));
}

void MyMoneyTransactionFilterTest::testMatchTransactionDate()
{
  MyMoneySplit split;
  split.setAccountId(acCheckingId);
  split.setShares(MyMoneyMoney(123.00));

  MyMoneySplit split2;
  split2.setAccountId(acExpenseId);
  split2.setShares(MyMoneyMoney(123.00));

  MyMoneyTransaction transaction;
  transaction.setPostDate(QDate(2014, 1, 2));
  transaction.addSplit(split);
  transaction.addSplit(split2);

  MyMoneyTransactionFilter filter;
  filter.setReportAllSplits(true);
  filter.setDateFilter(QDate(2014, 1, 1), QDate(2014, 1, 3));
  QVERIFY(filter.match(transaction));
  QCOMPARE(filter.matchingSplits().size(), 2);

  filter.setReportAllSplits(false);
  QVERIFY(filter.match(transaction));
  QCOMPARE(filter.matchingSplits().size(), 1);

  filter.setDateFilter(QDate(2014, 1, 3), QDate(2014, 1, 5));
  QVERIFY(!filter.match(transaction));
}

void setupTransactionForNumber(MyMoneyTransaction &transaction, const QString &accountId)
{
  MyMoneySplit split;
  split.setAccountId(accountId);
  split.setShares(MyMoneyMoney(123.00));
  split.setNumber("1");
  split.setMemo("1");

  MyMoneySplit split2;
  split2.setAccountId(accountId);
  split2.setShares(MyMoneyMoney(1.00));
  split2.setNumber("2");
  split2.setMemo("2");

  MyMoneySplit split3;
  split3.setAccountId(accountId);
  split3.setShares(MyMoneyMoney(100.00));
  split3.setNumber("3");
  split3.setMemo("3");

  MyMoneySplit split4;
  split4.setAccountId(accountId);
  split4.setShares(MyMoneyMoney(22.00));
  split4.setNumber("4");
  split4.setMemo("4");

  transaction.setPostDate(QDate(2014, 1, 2));
  transaction.addSplit(split);
  transaction.addSplit(split2);
  transaction.addSplit(split3);
  transaction.addSplit(split4);
}

void runtTestMatchTransactionNumber(MyMoneyTransaction &transaction, MyMoneyTransactionFilter &filter)
{
  // return all matching splits
  filter.setReportAllSplits(true);

  filter.setNumberFilter("", "");
  QVERIFY(filter.match(transaction));
  QCOMPARE(filter.matchingSplits().size(), 4);

  filter.setNumberFilter("1", "");
  QVERIFY(filter.match(transaction));
  QCOMPARE(filter.matchingSplits().size(), 4);

  filter.setNumberFilter("", "4");
  QVERIFY(filter.match(transaction));
  QCOMPARE(filter.matchingSplits().size(), 4);

  filter.setNumberFilter("1", "4");
  QVERIFY(filter.match(transaction));
  QCOMPARE(filter.matchingSplits().size(), 4);

  filter.setNumberFilter("1", "2");
  QVERIFY(filter.match(transaction));
  QCOMPARE(filter.matchingSplits().size(), 2);

  // do not return all matching splits
  filter.setReportAllSplits(false);

  filter.setNumberFilter("1", "4");
  QVERIFY(filter.match(transaction));
  QCOMPARE(filter.matchingSplits().size(), 1);

  filter.setNumberFilter("1", "2");
  QVERIFY(filter.match(transaction));
  QCOMPARE(filter.matchingSplits().size(), 1);
}

void MyMoneyTransactionFilterTest::testMatchTransactionNumber()
{
  MyMoneyTransaction transaction;
  setupTransactionForNumber(transaction, acCheckingId);

  MyMoneyTransactionFilter filter;
  runtTestMatchTransactionNumber(transaction, filter);

  transaction.clear();
  setupTransactionForNumber(transaction, acExpenseId);

  filter.clear();
  runtTestMatchTransactionNumber(transaction, filter);
}

void MyMoneyTransactionFilterTest::testMatchTransactionPayee()
{
  MyMoneySplit split;
  split.setAccountId(acCheckingId);
  split.setShares(MyMoneyMoney(123.00));
  split.setPayeeId(payeeId);

  MyMoneySplit split2;
  split2.setAccountId(acCheckingId);
  split2.setShares(MyMoneyMoney(124.00));

  MyMoneyTransaction transaction;
  transaction.setPostDate(QDate(2014, 1, 2));
  transaction.addSplit(split);
  transaction.addSplit(split2);

  MyMoneyTransactionFilter filter;
  filter.addPayee(payeeId);

  filter.setReportAllSplits(true);
  QVERIFY(filter.match(transaction));
  QCOMPARE(filter.matchingSplits().size(), 1);

  filter.setReportAllSplits(false);
  QVERIFY(filter.match(transaction));
  QCOMPARE(filter.matchingSplits().size(), 1);

  // check no category support
  MyMoneySplit split3;
  split3.setAccountId(acExpenseId);
  split3.setShares(MyMoneyMoney(120.00));
  split3.setPayeeId(payeeId);

  MyMoneyTransaction transaction2;
  transaction2.setPostDate(QDate(2014, 1, 2));
  transaction2.addSplit(split3);

  filter.setReportAllSplits(true);
  QVERIFY(!filter.match(transaction2));
  QCOMPARE(filter.matchingSplits().size(), 0);

  qDebug() << "payee on categories could not be tested";
}

void MyMoneyTransactionFilterTest::testMatchTransactionState()
{
  MyMoneySplit split;
  split.setAccountId(acCheckingId);
  split.setShares(MyMoneyMoney(123.00));
  split.setReconcileFlag(MyMoneySplit::NotReconciled);

  MyMoneySplit split2;
  split2.setAccountId(acCheckingId);
  split2.setShares(MyMoneyMoney(1.00));
  split2.setReconcileFlag(MyMoneySplit::Cleared);

  MyMoneySplit split3;
  split3.setAccountId(acCheckingId);
  split3.setShares(MyMoneyMoney(100.00));
  split3.setReconcileFlag(MyMoneySplit::Reconciled);

  MyMoneySplit split4;
  split4.setAccountId(acCheckingId);
  split4.setShares(MyMoneyMoney(22.00));
  split4.setReconcileFlag(MyMoneySplit::Frozen);

  MyMoneyTransaction transaction;
  transaction.setPostDate(QDate(2014, 1, 2));
  transaction.addSplit(split);
  transaction.addSplit(split2);
  transaction.addSplit(split3);
  transaction.addSplit(split4);

  MyMoneyTransactionFilter filter;
  filter.setReportAllSplits(true);
  QVERIFY(filter.match(transaction));
  QCOMPARE(filter.matchingSplits().size(), 4);

  // all states
  filter.addState(MyMoneyTransactionFilter::notReconciled);
  filter.addState(MyMoneyTransactionFilter::cleared);
  filter.addState(MyMoneyTransactionFilter::reconciled);
  filter.addState(MyMoneyTransactionFilter::frozen);
  QVERIFY(filter.match(transaction));
  QCOMPARE(filter.matchingSplits().size(), 4);

  // single state
  filter.clear();
  filter.addState(MyMoneyTransactionFilter::notReconciled);
  QVERIFY(filter.match(transaction));
  QCOMPARE(filter.matchingSplits().size(), 1);
  filter.clear();

  filter.addState(MyMoneyTransactionFilter::cleared);
  QVERIFY(filter.match(transaction));
  QCOMPARE(filter.matchingSplits().size(), 1);
  filter.clear();

  filter.addState(MyMoneyTransactionFilter::reconciled);
  QVERIFY(filter.match(transaction));
  QCOMPARE(filter.matchingSplits().size(), 1);
  filter.clear();

  filter.addState(MyMoneyTransactionFilter::frozen);
  QVERIFY(filter.match(transaction));
  QCOMPARE(filter.matchingSplits().size(), 1);

  // check no category support
  MyMoneySplit split5;
  split5.setAccountId(acCheckingId);
  split5.setShares(MyMoneyMoney(22.00));
  split5.setReconcileFlag(MyMoneySplit::Frozen);

  MyMoneyTransaction transaction2;
  transaction.setPostDate(QDate(2014, 1, 2));
  transaction.addSplit(split5);

  filter.clear();
  filter.setReportAllSplits(true);
  filter.addState(MyMoneyTransactionFilter::frozen);
  QVERIFY(!filter.match(transaction2));
  QCOMPARE(filter.matchingSplits().size(), 0);

  qDebug() << "states on categories could not be tested";
}

void MyMoneyTransactionFilterTest::testMatchTransactionTag()
{
  MyMoneySplit split;
  split.setAccountId(acCheckingId);
  split.setShares(MyMoneyMoney(123.00));
  split.setTagIdList(tagIdList);

  MyMoneySplit split2;
  split2.setAccountId(acExpenseId);
  split2.setShares(MyMoneyMoney(123.00));
  split2.setTagIdList(tagIdList);

  MyMoneySplit split3;
  split3.setAccountId(acCheckingId);
  split3.setShares(MyMoneyMoney(10.00));
  split3.setTagIdList(tagIdList);

  MyMoneyTransaction transaction;
  transaction.setPostDate(QDate(2014, 1, 2));
  transaction.addSplit(split);
  transaction.addSplit(split2);
  transaction.addSplit(split3);

  MyMoneyTransactionFilter filter;
  filter.addTag(tagIdList.first());
  filter.setReportAllSplits(true);
  QVERIFY(filter.match(transaction));
  // -1 because categories are not supported yet
  QCOMPARE(filter.matchingSplits().size(), 2);

  filter.setReportAllSplits(false);
  QVERIFY(filter.match(transaction));
  QCOMPARE(filter.matchingSplits().size(), 1);

  // check no category support
  MyMoneySplit split4;
  split4.setAccountId(acExpenseId);
  split4.setShares(MyMoneyMoney(123.00));
  split4.setTagIdList(tagIdList);

  MyMoneyTransaction transaction2;
  transaction2.setPostDate(QDate(2014, 1, 2));
  transaction2.addSplit(split4);

  filter.setReportAllSplits(true);
  QVERIFY(!filter.match(transaction2));
  QCOMPARE(filter.matchingSplits().size(), 0);

  qDebug() << "tags on categories could not be tested";
}

void MyMoneyTransactionFilterTest::testMatchTransactionTypeAllTypes()
{
  /*
    alltypes
     - account group == MyMoneyAccount::Income ||
     - account group == MyMoneyAccount::Expense
  */
  MyMoneySplit split;
  split.setAccountId(acExpenseId);
  split.setValue(MyMoneyMoney(123.00));

  MyMoneySplit split2;
  split2.setAccountId(acIncomeId);
  split2.setValue(MyMoneyMoney(-123.00));

  MyMoneyTransaction transaction;
  transaction.setPostDate(QDate(2014, 1, 2));
  transaction.addSplit(split);
  transaction.addSplit(split2);

  MyMoneyTransactionFilter filter;
  filter.setReportAllSplits(true);

  // all splits
  QVERIFY(filter.match(transaction));
  QCOMPARE(filter.matchingSplits().size(), 2);

  filter.addType(MyMoneyTransactionFilter::allTypes);
  qDebug() << "MyMoneyTransactionFilter::allTypes could not be tested";
  qDebug() << "because type filter does not work with categories";

  QVERIFY(!filter.match(transaction));
  QCOMPARE(filter.matchingSplits().size(), 0);

  // ! alltypes
  MyMoneySplit split3;
  split3.setAccountId(acCheckingId);
  split3.setValue(MyMoneyMoney(-123.00));

  MyMoneyTransaction transaction2;
  transaction2.addSplit(split3);

  QVERIFY(!filter.match(transaction2));
  QCOMPARE(filter.matchingSplits().size(), 0);
}

void MyMoneyTransactionFilterTest::testMatchTransactionTypeDeposits()
{
  // deposits - split value is positive
  MyMoneySplit split;
  split.setAccountId(acCheckingId);
  split.setValue(MyMoneyMoney(123.00));

  MyMoneyTransaction transaction;
  transaction.setPostDate(QDate(2014, 1, 2));
  transaction.addSplit(split);

  MyMoneyTransactionFilter filter;
  filter.setReportAllSplits(true);

  // all splits
  QVERIFY(filter.match(transaction));
  QCOMPARE(filter.matchingSplits().size(), 1);

  // deposits
  filter.addType(MyMoneyTransactionFilter::deposits);
  QVERIFY(filter.match(transaction));
  QCOMPARE(filter.matchingSplits().size(), 1);

  // no deposits
  MyMoneySplit split2;
  split2.setAccountId(acCheckingId);
  split2.setValue(MyMoneyMoney(-123.00));

  MyMoneyTransaction transaction2;
  transaction2.setPostDate(QDate(2014, 1, 2));
  transaction2.addSplit(split2);

  QVERIFY(!filter.match(transaction2));
  QCOMPARE(filter.matchingSplits().size(), 0);
}

void MyMoneyTransactionFilterTest::testMatchTransactionTypePayments()
{
  /*
    payments
    - account group != MyMoneyAccount::Income
    - account group != MyMoneyAccount::Expense
    - split value is not positive
    - number of splits != 2
  */
  MyMoneySplit split;
  split.setAccountId(acCheckingId);
  split.setValue(MyMoneyMoney(-123.00));

  MyMoneyTransaction transaction;
  transaction.setPostDate(QDate(2014, 1, 2));
  transaction.addSplit(split);

  MyMoneyTransactionFilter filter;
  filter.setReportAllSplits(true);

  // all splits
  QVERIFY(filter.match(transaction));
  QCOMPARE(filter.matchingSplits().size(), 1);

  // valid payments
  filter.addType(MyMoneyTransactionFilter::payments);
  QVERIFY(filter.match(transaction));
  QCOMPARE(filter.matchingSplits().size(), 1);

  // no payments
  // check number of splits != 2
  MyMoneySplit split2;
  split2.setAccountId(acCheckingId);
  split2.setValue(MyMoneyMoney(-123.00));
  transaction.addSplit(split2);

  QVERIFY(!filter.match(transaction));
  QCOMPARE(filter.matchingSplits().size(), 0);

  // split value is not positive
  MyMoneySplit split3;
  split3.setAccountId(acCheckingId);
  split3.setValue(MyMoneyMoney(123.00));
  transaction.addSplit(split3);

  QVERIFY(filter.match(transaction));
  QCOMPARE(filter.matchingSplits().size(), 2);

  // account group != MyMoneyAccount::Income && account group != MyMoneyAccount::Expense
  MyMoneySplit split4;
  split4.setAccountId(acExpenseId);
  split4.setValue(MyMoneyMoney(-124.00));
  transaction.addSplit(split4);

  QVERIFY(filter.match(transaction));
  QCOMPARE(filter.matchingSplits().size(), 2);
}

void MyMoneyTransactionFilterTest::testMatchTransactionTypeTransfers()
{
  /*
   check transfers
    - number of splits == 2
    - account group != MyMoneyAccount::Income
    - account group != MyMoneyAccount::Expense
  */
  MyMoneySplit split;
  split.setAccountId(acCheckingId);
  split.setValue(MyMoneyMoney(123.00));

  MyMoneySplit split2;
  split2.setAccountId(acCheckingId);
  split2.setValue(MyMoneyMoney(-123.00));

  MyMoneySplit split3;
  split3.setAccountId(acCheckingId);
  split3.setValue(MyMoneyMoney(-123.00));

  MyMoneyTransaction transaction;
  transaction.setPostDate(QDate(2014, 1, 2));
  transaction.addSplit(split);
  transaction.addSplit(split2);

  MyMoneyTransactionFilter filter;
  filter.setReportAllSplits(true);

  // all splits
  QVERIFY(filter.match(transaction));
  QCOMPARE(filter.matchingSplits().size(), 2);

  filter.addType(MyMoneyTransactionFilter::transfers);

  QVERIFY(filter.match(transaction));
  QCOMPARE(filter.matchingSplits().size(), 2);

  // transfers - invalid number of counts
  transaction.addSplit(split3);
  QVERIFY(!filter.match(transaction));
  QCOMPARE(filter.matchingSplits().size(), 0);

  // transfers - invalid account
  MyMoneySplit split4;
  split4.setAccountId(acIncomeId);
  split4.setValue(MyMoneyMoney(-123.00));

  MyMoneySplit split5;
  split5.setAccountId(acCheckingId);
  split5.setValue(MyMoneyMoney(123.00));

  MyMoneyTransaction transaction2;
  transaction2.setPostDate(QDate(2014, 1, 2));
  transaction2.addSplit(split4);
  transaction2.addSplit(split5);

  QVERIFY(!filter.match(transaction2));
  QCOMPARE(filter.matchingSplits().size(), 0);
}

void MyMoneyTransactionFilterTest::testMatchTransactionValidity()
{
    MyMoneySplit split;
    split.setAccountId(acCheckingId);
    split.setValue(MyMoneyMoney(123.00));

    MyMoneySplit split2;
    split2.setAccountId(acExpenseId);
    split2.setValue(MyMoneyMoney(-123.00));

    MyMoneyTransaction transaction;
    transaction.setPostDate(QDate(2014, 1, 2));
    transaction.addSplit(split);
    transaction.addSplit(split2);

    // check valid transaction
    MyMoneyTransactionFilter filter;
    filter.addValidity(MyMoneyTransactionFilter::valid);
    filter.setReportAllSplits(true);

    QVERIFY(filter.match(transaction));
    QCOMPARE(filter.matchingSplits().size(), 2);

    filter.setReportAllSplits(false);
    QVERIFY(filter.match(transaction));
    QCOMPARE(filter.matchingSplits().size(), 1);

    // check invalid transaction
    filter.clear();
    filter.addValidity(MyMoneyTransactionFilter::invalid);
    filter.setReportAllSplits(true);

    QVERIFY(!filter.match(transaction));
    QCOMPARE(filter.matchingSplits().size(), 0);

    // add split to make transaction invalid
    MyMoneySplit split3;
    split3.setAccountId(acExpenseId);
    split3.setValue(MyMoneyMoney(-10.00));
    transaction.addSplit(split3);

    filter.setReportAllSplits(true);
    QVERIFY(filter.match(transaction));
    QCOMPARE(filter.matchingSplits().size(), 3);

    filter.clear();
    filter.addValidity(MyMoneyTransactionFilter::valid);

    QVERIFY(!filter.match(transaction));
    QCOMPARE(filter.matchingSplits().size(), 0);
}

QTEST_MAIN(MyMoneyTransactionFilterTest)
