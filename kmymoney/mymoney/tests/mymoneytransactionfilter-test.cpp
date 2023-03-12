/*
    SPDX-FileCopyrightText: 2018 Ralf Habacker <ralf.habacker@freenet.de>
    SPDX-FileCopyrightText: 2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "mymoneytransactionfilter-test.h"

#include <QRegularExpression>
#include <QTest>

#include "mymoneyaccount.h"
#include "mymoneyenums.h"
#include "mymoneyexception.h"
#include "mymoneyfile.h"
#include "mymoneypayee.h"
#include "mymoneysecurity.h"
#include "mymoneysplit.h"
#include "mymoneytag.h"
#include "mymoneytransaction.h"
#include "mymoneytransactionfilter.h"

// uses helper functions from reports tests
#include "tests/testutilities.h"
using namespace test;


QTEST_GUILESS_MAIN(MyMoneyTransactionFilterTest)


MyMoneyTransactionFilterTest::MyMoneyTransactionFilterTest::MyMoneyTransactionFilterTest()
    : file(nullptr)
{
}

void MyMoneyTransactionFilterTest::init()
{
    file = MyMoneyFile::instance();

    MyMoneyFileTransaction ft;
    file->addCurrency(MyMoneySecurity("USD", "US Dollar", "$"));
    file->setBaseCurrency(file->currency("USD"));

    MyMoneyPayee payeeTest("Payee 10.2");
    file->addPayee(payeeTest);
    payeeId = payeeTest.id();

    MyMoneyTag tag("Tag 10.2");
    file->addTag(tag);
    tagIdList << tag.id();

    const QString asset = MyMoneyFile::instance()->asset().id();
    const QString expense = (MyMoneyFile::instance()->expense().id());
    const QString income = (MyMoneyFile::instance()->income().id());
    acCheckingId = makeAccount(QLatin1String("Account 10.2"), eMyMoney::Account::Type::Checkings, MyMoneyMoney(0.0), QDate(2004, 1, 1), asset);
    acExpenseId = makeAccount(QLatin1String("Expense"), eMyMoney::Account::Type::Expense, MyMoneyMoney(), QDate(2004, 1, 11), expense);
    acIncomeId = makeAccount(QLatin1String("Expense"), eMyMoney::Account::Type::Expense, MyMoneyMoney(), QDate(2004, 1, 11), income);

    ft.commit();
}

void MyMoneyTransactionFilterTest::cleanup()
{
}

void MyMoneyTransactionFilterTest::testMatchAmount()
{
    MyMoneySplit split;
    split.setShares(MyMoneyMoney(123.20));

    MyMoneyTransactionFilter filter;
    QCOMPARE(filter.matchAmount(split), true);

    filter.setAmountFilter(MyMoneyMoney("123.0"), MyMoneyMoney("124.0"));
    QCOMPARE(filter.matchAmount(split), true);
    filter.setAmountFilter(MyMoneyMoney("120.0"), MyMoneyMoney("123.0"));
    QCOMPARE(filter.matchAmount(split), false);
}

void MyMoneyTransactionFilterTest::testMatchText()
{
    MyMoneySplit split;
    MyMoneyTransactionFilter filter;
    MyMoneyAccount account = file->account(acCheckingId);

    // no filter
    QCOMPARE(filter.matchText(split, account), true);
    filter.setTextFilter(QRegularExpression("10.2"), false, false);
    MyMoneyTransactionFilter filterInvert;
    filterInvert.setTextFilter(QRegularExpression("10.2"), false, true);
    MyMoneyTransactionFilter filterNotFound;
    filterNotFound.setTextFilter(QRegularExpression("10.5"), false, false);

    // memo
    split.setMemo("10.2");
    QCOMPARE(filter.matchText(split, account), true);
    QCOMPARE(filterInvert.matchText(split, account), false);
    QCOMPARE(filterNotFound.matchText(split, account), false);
    split.setMemo(QString());
    // payee
    split.setPayeeId(payeeId);
    QCOMPARE(filter.matchText(split, account), true);
    QCOMPARE(filterInvert.matchText(split, account), false);
    QCOMPARE(filterNotFound.matchText(split, account), false);
    split.setPayeeId(QString());
    // tag
    split.setTagIdList(tagIdList);
    QCOMPARE(filter.matchText(split, account), true);
    QCOMPARE(filterInvert.matchText(split, account), false);
    QCOMPARE(filterNotFound.matchText(split, account), false);
    split.setTagIdList(QStringList());
    // value
    split.setValue(MyMoneyMoney("10.2"));
    QCOMPARE(filter.matchText(split, account), true);
    QCOMPARE(filterInvert.matchText(split, account), false);
    QCOMPARE(filterNotFound.matchText(split, account), false);
    split.setValue(MyMoneyMoney());
    // number
    split.setNumber("10.2");
    QCOMPARE(filter.matchText(split, account), true);
    QCOMPARE(filterInvert.matchText(split, account), false);
    QCOMPARE(filterNotFound.matchText(split, account), false);
    split.setNumber("0.0");
    // transaction id
    split.setTransactionId("10.2");
    QCOMPARE(filter.matchText(split, account), true);
    QCOMPARE(filterInvert.matchText(split, account), false);
    QCOMPARE(filterNotFound.matchText(split, account), false);
    split.setTransactionId("0.0");
    // account
    split.setAccountId(acCheckingId);
    QCOMPARE(filter.matchText(split, account), true);
    QCOMPARE(filterInvert.matchText(split, account), false);
    QCOMPARE(filterNotFound.matchText(split, account), false);
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
    QCOMPARE(filter.matchingSplits(transaction).size(), 2);

    filter.setReportAllSplits(false);
    QVERIFY(filter.match(transaction));
    QCOMPARE(filter.matchingSplits(transaction).size(), 1);
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
    QCOMPARE(filter.matchingSplits(transaction).size(), 1);

    filter.setReportAllSplits(false);
    filter.setConsiderCategory(false);
    QVERIFY(filter.match(transaction));
    QCOMPARE(filter.matchingSplits(transaction).size(), 1);

    filter.setReportAllSplits(false);
    filter.setConsiderCategory(true);
    QVERIFY(filter.match(transaction));
    QCOMPARE(filter.matchingSplits(transaction).size(), 1);

    filter.setReportAllSplits(true);
    filter.setConsiderCategory(true);
    QVERIFY(filter.match(transaction));
    QCOMPARE(filter.matchingSplits(transaction).size(), 1);
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
    QCOMPARE(filter.matchingSplits(transaction).size(), 2);

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
    QCOMPARE(filter.matchingSplits(transaction).size(), 2);

    filter.setReportAllSplits(false);
    QVERIFY(filter.match(transaction));
    QCOMPARE(filter.matchingSplits(transaction).size(), 1);

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
    QCOMPARE(filter.matchingSplits(transaction).size(), 4);

    filter.setNumberFilter("1", "");
    QVERIFY(filter.match(transaction));
    QCOMPARE(filter.matchingSplits(transaction).size(), 4);

    filter.setNumberFilter("", "4");
    QVERIFY(filter.match(transaction));
    QCOMPARE(filter.matchingSplits(transaction).size(), 4);

    filter.setNumberFilter("1", "4");
    QVERIFY(filter.match(transaction));
    QCOMPARE(filter.matchingSplits(transaction).size(), 4);

    filter.setNumberFilter("1", "2");
    QVERIFY(filter.match(transaction));
    QCOMPARE(filter.matchingSplits(transaction).size(), 2);

    // do not return all matching splits
    filter.setReportAllSplits(false);

    filter.setNumberFilter("1", "4");
    QVERIFY(filter.match(transaction));
    QCOMPARE(filter.matchingSplits(transaction).size(), 1);

    filter.setNumberFilter("1", "2");
    QVERIFY(filter.match(transaction));
    QCOMPARE(filter.matchingSplits(transaction).size(), 1);
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
    QCOMPARE(filter.matchingSplits(transaction).size(), 1);

    filter.setReportAllSplits(false);
    QVERIFY(filter.match(transaction));
    QCOMPARE(filter.matchingSplits(transaction).size(), 1);

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
    QCOMPARE(filter.matchingSplits(transaction2).size(), 0);

    qDebug() << "payee on categories could not be tested";
}

void MyMoneyTransactionFilterTest::testMatchTransactionState()
{
    MyMoneySplit split;
    split.setAccountId(acCheckingId);
    split.setShares(MyMoneyMoney(123.00));
    split.setReconcileFlag(eMyMoney::Split::State::NotReconciled);

    MyMoneySplit split2;
    split2.setAccountId(acCheckingId);
    split2.setShares(MyMoneyMoney(1.00));
    split2.setReconcileFlag(eMyMoney::Split::State::Cleared);

    MyMoneySplit split3;
    split3.setAccountId(acCheckingId);
    split3.setShares(MyMoneyMoney(100.00));
    split3.setReconcileFlag(eMyMoney::Split::State::Reconciled);

    MyMoneySplit split4;
    split4.setAccountId(acCheckingId);
    split4.setShares(MyMoneyMoney(22.00));
    split4.setReconcileFlag(eMyMoney::Split::State::Frozen);

    MyMoneyTransaction transaction;
    transaction.setPostDate(QDate(2014, 1, 2));
    transaction.addSplit(split);
    transaction.addSplit(split2);
    transaction.addSplit(split3);
    transaction.addSplit(split4);

    MyMoneyTransactionFilter filter;
    filter.setReportAllSplits(true);
    QVERIFY(filter.match(transaction));
    QCOMPARE(filter.matchingSplits(transaction).size(), 4);

    // all states
    filter.addState((int)eMyMoney::TransactionFilter::State::NotReconciled);
    filter.addState((int)eMyMoney::TransactionFilter::State::Cleared);
    filter.addState((int)eMyMoney::TransactionFilter::State::Reconciled);
    filter.addState((int)eMyMoney::TransactionFilter::State::Frozen);
    QVERIFY(filter.match(transaction));
    QCOMPARE(filter.matchingSplits(transaction).size(), 4);

    // single state
    filter.clear();
    filter.addState((int)eMyMoney::TransactionFilter::State::NotReconciled);
    QVERIFY(filter.match(transaction));
    QCOMPARE(filter.matchingSplits(transaction).size(), 1);
    filter.clear();

    filter.addState((int)eMyMoney::TransactionFilter::State::Cleared);
    QVERIFY(filter.match(transaction));
    QCOMPARE(filter.matchingSplits(transaction).size(), 1);
    filter.clear();

    filter.addState((int)eMyMoney::TransactionFilter::State::Reconciled);
    QVERIFY(filter.match(transaction));
    QCOMPARE(filter.matchingSplits(transaction).size(), 1);
    filter.clear();

    filter.addState((int)eMyMoney::TransactionFilter::State::Frozen);
    QVERIFY(filter.match(transaction));
    QCOMPARE(filter.matchingSplits(transaction).size(), 1);

    // check no category support
    MyMoneySplit split5;
    split5.setAccountId(acCheckingId);
    split5.setShares(MyMoneyMoney(22.00));
    split5.setReconcileFlag(eMyMoney::Split::State::Frozen);

    MyMoneyTransaction transaction2;
    transaction.setPostDate(QDate(2014, 1, 2));
    transaction.addSplit(split5);

    filter.clear();
    filter.setReportAllSplits(true);
    filter.addState((int)eMyMoney::TransactionFilter::State::Frozen);
    QVERIFY(!filter.match(transaction2));
    QCOMPARE(filter.matchingSplits(transaction2).size(), 0);

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
    QCOMPARE(filter.matchingSplits(transaction).size(), 2);

    filter.setReportAllSplits(false);
    QVERIFY(filter.match(transaction));
    QCOMPARE(filter.matchingSplits(transaction).size(), 1);

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
    QCOMPARE(filter.matchingSplits(transaction2).size(), 0);

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
    QCOMPARE(filter.matchingSplits(transaction).size(), 2);

    filter.addType((int)eMyMoney::TransactionFilter::State::All);
    qDebug() << "MyMoneyTransactionFilter::allTypes could not be tested";
    qDebug() << "because type filter does not work with categories";

    QVERIFY(!filter.match(transaction));
    QCOMPARE(filter.matchingSplits(transaction).size(), 0);

    // ! alltypes
    MyMoneySplit split3;
    split3.setAccountId(acCheckingId);
    split3.setValue(MyMoneyMoney(-123.00));

    MyMoneyTransaction transaction2;
    transaction2.addSplit(split3);

    QVERIFY(!filter.match(transaction2));
    QCOMPARE(filter.matchingSplits(transaction2).size(), 0);
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
    QCOMPARE(filter.matchingSplits(transaction).size(), 1);

    // deposits
    filter.addType((int)eMyMoney::TransactionFilter::Type::Deposits);
    QVERIFY(filter.match(transaction));
    QCOMPARE(filter.matchingSplits(transaction).size(), 1);

    // no deposits
    MyMoneySplit split2;
    split2.setAccountId(acCheckingId);
    split2.setValue(MyMoneyMoney(-123.00));

    MyMoneyTransaction transaction2;
    transaction2.setPostDate(QDate(2014, 1, 2));
    transaction2.addSplit(split2);

    QVERIFY(!filter.match(transaction2));
    QCOMPARE(filter.matchingSplits(transaction2).size(), 0);
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
    QCOMPARE(filter.matchingSplits(transaction).size(), 1);

    // valid payments
    filter.addType((int)eMyMoney::TransactionFilter::Type::Payments);
    QVERIFY(filter.match(transaction));
    QCOMPARE(filter.matchingSplits(transaction).size(), 1);

    // no payments
    // check number of splits != 2
    MyMoneySplit split2;
    split2.setAccountId(acCheckingId);
    split2.setValue(MyMoneyMoney(-123.00));
    transaction.addSplit(split2);

    QVERIFY(!filter.match(transaction));
    QCOMPARE(filter.matchingSplits(transaction).size(), 0);

    // split value is not positive
    MyMoneySplit split3;
    split3.setAccountId(acCheckingId);
    split3.setValue(MyMoneyMoney(123.00));
    transaction.addSplit(split3);

    QVERIFY(filter.match(transaction));
    QCOMPARE(filter.matchingSplits(transaction).size(), 2);

    // account group != MyMoneyAccount::Income && account group != MyMoneyAccount::Expense
    MyMoneySplit split4;
    split4.setAccountId(acExpenseId);
    split4.setValue(MyMoneyMoney(-124.00));
    transaction.addSplit(split4);

    QVERIFY(filter.match(transaction));
    QCOMPARE(filter.matchingSplits(transaction).size(), 2);
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
    QCOMPARE(filter.matchingSplits(transaction).size(), 2);

    filter.addType((int)eMyMoney::TransactionFilter::Type::Transfers);

    QVERIFY(filter.match(transaction));
    QCOMPARE(filter.matchingSplits(transaction).size(), 2);

    // transfers - invalid number of counts
    transaction.addSplit(split3);
    QVERIFY(!filter.match(transaction));
    QCOMPARE(filter.matchingSplits(transaction).size(), 0);

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
    QCOMPARE(filter.matchingSplits(transaction2).size(), 0);
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
    filter.addValidity((int)eMyMoney::TransactionFilter::Validity::Valid);
    filter.setReportAllSplits(true);

    QVERIFY(filter.match(transaction));
    QCOMPARE(filter.matchingSplits(transaction).size(), 2);

    filter.setReportAllSplits(false);
    QVERIFY(filter.match(transaction));
    QCOMPARE(filter.matchingSplits(transaction).size(), 1);

    // check invalid transaction
    filter.clear();
    filter.addValidity((int)eMyMoney::TransactionFilter::Validity::Invalid);
    filter.setReportAllSplits(true);

    QVERIFY(!filter.match(transaction));
    QCOMPARE(filter.matchingSplits(transaction).size(), 0);

    // add split to make transaction invalid
    MyMoneySplit split3;
    split3.setAccountId(acExpenseId);
    split3.setValue(MyMoneyMoney(-10.00));
    transaction.addSplit(split3);

    filter.setReportAllSplits(true);
    QVERIFY(filter.match(transaction));
    QCOMPARE(filter.matchingSplits(transaction).size(), 3);

    filter.clear();
    filter.addValidity((int)eMyMoney::TransactionFilter::Validity::Valid);

    QVERIFY(!filter.match(transaction));
    QCOMPARE(filter.matchingSplits(transaction).size(), 0);
}
