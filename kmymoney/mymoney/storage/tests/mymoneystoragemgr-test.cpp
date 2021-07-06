/*
    SPDX-FileCopyrightText: 2009-2012 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "mymoneystoragemgr-test.h"
#include <iostream>
#include <QList>
#include <QTest>

#include "mymoneystoragemgr_p.h"

#include "mymoneytestutils.h"
#include "mymoneymoney.h"
#include "mymoneyfile.h"
#include "mymoneyinstitution.h"
#include "mymoneyaccount.h"
#include "mymoneysecurity.h"
#include "mymoneytag.h"
#include "mymoneypayee.h"
#include "mymoneyschedule.h"
#include "mymoneyschedule_p.h"
#include "mymoneyreport.h"
#include "mymoneysplit.h"
#include "mymoneysplit_p.h"
#include "mymoneytransaction.h"
#include "mymoneybudget.h"
#include "mymoneyprice.h"
#include "onlinejob.h"

#include "onlinetasks/dummy/tasks/dummytask.h"

#include "mymoneyenums.h"

using namespace eMyMoney;

QTEST_GUILESS_MAIN(MyMoneyStorageMgrTest)

void MyMoneyStorageMgrTest::init()
{
    m = new MyMoneyStorageMgr;
    MyMoneyFile* file = MyMoneyFile::instance();
    file->attachStorage(m);
    m->startTransaction();
}

void MyMoneyStorageMgrTest::cleanup()
{
    m->commitTransaction();
    MyMoneyFile* file = MyMoneyFile::instance();
    file->detachStorage(m);
    delete m;
}

void MyMoneyStorageMgrTest::testEmptyConstructor()
{
    MyMoneyPayee user = m->user();

    QVERIFY(user.name().isEmpty());
    QVERIFY(user.address().isEmpty());
    QVERIFY(user.city().isEmpty());
    QVERIFY(user.state().isEmpty());
    QVERIFY(user.postcode().isEmpty());
    QVERIFY(user.telephone().isEmpty());
    QVERIFY(user.email().isEmpty());
    QCOMPARE(m->d_func()->m_nextInstitutionID, 0ul);
    QCOMPARE(m->d_func()->m_nextAccountID, 0ul);
    QCOMPARE(m->d_func()->m_nextTransactionID, 0ul);
    QCOMPARE(m->d_func()->m_nextPayeeID, 0ul);
    QCOMPARE(m->d_func()->m_nextScheduleID, 0ul);
    QCOMPARE(m->d_func()->m_nextReportID, 0ul);
    QCOMPARE(m->d_func()->m_institutionList.count(), 0);
    QCOMPARE(m->d_func()->m_accountList.count(), 5);
    QCOMPARE(m->d_func()->m_transactionList.count(), 0);
    QCOMPARE(m->d_func()->m_transactionKeys.count(), 0);
    QCOMPARE(m->d_func()->m_payeeList.count(), 0);
    QCOMPARE(m->d_func()->m_tagList.count(), 0);
    QCOMPARE(m->d_func()->m_scheduleList.count(), 0);

    QCOMPARE(m->d_func()->m_dirty, false);
    QCOMPARE(m->creationDate(), QDate::currentDate());

    QCOMPARE(m->liability().name(), QLatin1String("Liability"));
    QCOMPARE(m->asset().name(), QLatin1String("Asset"));
    QCOMPARE(m->expense().name(), QLatin1String("Expense"));
    QCOMPARE(m->income().name(), QLatin1String("Income"));
    QCOMPARE(m->equity().name(), QLatin1String("Equity"));
}

void MyMoneyStorageMgrTest::testSetFunctions()
{
    MyMoneyPayee user = m->user();

    m->d_func()->m_dirty = false;
    user.setName("Name");
    m->setUser(user);
    m->commitTransaction();
    m->startTransaction();
    QCOMPARE(m->dirty(), true);
    m->d_func()->m_dirty = false;
    user.setAddress("Street");
    m->setUser(user);
    m->commitTransaction();
    m->startTransaction();
    QCOMPARE(m->dirty(), true);
    m->d_func()->m_dirty = false;
    user.setCity("Town");
    m->setUser(user);
    m->commitTransaction();
    m->startTransaction();
    QCOMPARE(m->dirty(), true);
    m->d_func()->m_dirty = false;
    user.setState("County");
    m->setUser(user);
    m->commitTransaction();
    m->startTransaction();
    QCOMPARE(m->dirty(), true);
    m->d_func()->m_dirty = false;
    user.setPostcode("Postcode");
    m->setUser(user);
    m->commitTransaction();
    m->startTransaction();
    QCOMPARE(m->dirty(), true);
    m->d_func()->m_dirty = false;
    user.setTelephone("Telephone");
    m->setUser(user);
    m->commitTransaction();
    m->startTransaction();
    QCOMPARE(m->dirty(), true);
    m->d_func()->m_dirty = false;
    user.setEmail("Email");
    m->setUser(user);
    m->commitTransaction();
    m->startTransaction();
    QCOMPARE(m->dirty(), true);
    m->d_func()->m_dirty = false;
    m->setValue("key", "value");
    m->commitTransaction();
    m->startTransaction();
    QCOMPARE(m->dirty(), true);

    user = m->user();
    QCOMPARE(user.name(), QLatin1String("Name"));
    QCOMPARE(user.address(), QLatin1String("Street"));
    QCOMPARE(user.city(), QLatin1String("Town"));
    QCOMPARE(user.state(), QLatin1String("County"));
    QCOMPARE(user.postcode(), QLatin1String("Postcode"));
    QCOMPARE(user.telephone(), QLatin1String("Telephone"));
    QCOMPARE(user.email(), QLatin1String("Email"));
    QCOMPARE(m->value("key"), QLatin1String("value"));

    m->d_func()->m_dirty = false;
    m->deletePair("key");
    m->commitTransaction();
    m->startTransaction();
    QCOMPARE(m->dirty(), true);
}

void MyMoneyStorageMgrTest::testSupportFunctions()
{
    QCOMPARE(m->d_func()->nextInstitutionID(), QLatin1String("I000001"));
    QCOMPARE(m->d_func()->m_nextInstitutionID, 1ul);
    QCOMPARE(m->d_func()->nextAccountID(), QLatin1String("A000001"));
    QCOMPARE(m->d_func()->m_nextAccountID, 1ul);
    QCOMPARE(m->d_func()->nextTransactionID(), QLatin1String("T000000000000000001"));
    QCOMPARE(m->d_func()->m_nextTransactionID, 1ul);
    QCOMPARE(m->d_func()->nextPayeeID(), QLatin1String("P000001"));
    QCOMPARE(m->d_func()->m_nextPayeeID, 1ul);
    QCOMPARE(m->d_func()->nextTagID(), QLatin1String("G000001"));
    QCOMPARE(m->d_func()->m_nextTagID, 1ul);
    QCOMPARE(m->d_func()->nextScheduleID(), QLatin1String("SCH000001"));
    QCOMPARE(m->d_func()->m_nextScheduleID, 1ul);
    QCOMPARE(m->d_func()->nextReportID(), QLatin1String("R000001"));
    QCOMPARE(m->d_func()->m_nextReportID, 1ul);
    QCOMPARE(m->d_func()->nextOnlineJobID(), QLatin1String("O000001"));
    QCOMPARE(m->d_func()->m_nextOnlineJobID, 1ul);
}

void MyMoneyStorageMgrTest::testIsStandardAccount()
{
    QCOMPARE(m->isStandardAccount(MyMoneyAccount::stdAccName(eMyMoney::Account::Standard::Liability)), true);
    QCOMPARE(m->isStandardAccount(MyMoneyAccount::stdAccName(eMyMoney::Account::Standard::Asset)), true);
    QCOMPARE(m->isStandardAccount(MyMoneyAccount::stdAccName(eMyMoney::Account::Standard::Expense)), true);
    QCOMPARE(m->isStandardAccount(MyMoneyAccount::stdAccName(eMyMoney::Account::Standard::Income)), true);
    QCOMPARE(m->isStandardAccount("A0002"), false);
}

void MyMoneyStorageMgrTest::testNewAccount()
{
    MyMoneyAccount a;

    a.setName("AccountName");
    a.setNumber("AccountNumber");

    m->addAccount(a);
    m->commitTransaction();
    m->startTransaction();

    QCOMPARE(m->d_func()->m_nextAccountID, 1ul);
    QCOMPARE(m->dirty(), true);
    QCOMPARE(m->d_func()->m_accountList.count(), 6);
    QCOMPARE(m->d_func()->m_accountList["A000001"].name(), QLatin1String("AccountName"));
}

void MyMoneyStorageMgrTest::testAccount()
{
    testNewAccount();
    m->d_func()->m_dirty = false;

    MyMoneyAccount a;

    // make sure that an invalid ID causes an exception
    try {
        a = m->account("Unknown ID");
        QFAIL("Exception expected");
    } catch (const MyMoneyException &) {
    }
    m->commitTransaction();
    m->startTransaction();
    QCOMPARE(m->dirty(), false);

    // now make sure, that a real ID works
    try {
        a = m->account("A000001");
        m->commitTransaction();
        m->startTransaction();
        QCOMPARE(a.name(), QLatin1String("AccountName"));
        QCOMPARE(a.id(), QLatin1String("A000001"));
        QCOMPARE(m->dirty(), true);
    } catch (const MyMoneyException &) {
        QFAIL("Unexpected exception");
    }
}

void MyMoneyStorageMgrTest::testAddNewAccount()
{
    testNewAccount();

    MyMoneyAccount a, b;
    b.setName("Account2");
    b.setNumber("Acc2");
    m->addAccount(b);
    m->commitTransaction();
    m->startTransaction();

    m->d_func()->m_dirty = false;

    QCOMPARE(m->d_func()->m_nextAccountID, 2ul);
    QCOMPARE(m->d_func()->m_accountList.count(), 7);

    // try to add account to undefined account
    try {
        MyMoneyAccount c("UnknownID", b);
        m->addAccount(c, a);
        QFAIL("Exception expected");
    } catch (const MyMoneyException &) {
    }
    m->commitTransaction();
    m->startTransaction();

    QCOMPARE(m->dirty(), false);
    // now try to add account 1 as sub-account to account 2
    a = m->account("A000001");
    try {
        QCOMPARE(m->d_func()->m_accountList[MyMoneyAccount::stdAccName(eMyMoney::Account::Standard::Asset)].accountList().count(), 0);
        m->addAccount(b, a);
        m->commitTransaction();
        m->startTransaction();
        QCOMPARE(m->d_func()->m_accountList["A000002"].accountList()[0], QLatin1String("A000001"));
        QCOMPARE(m->d_func()->m_accountList["A000002"].accountList().count(), 1);
        QCOMPARE(m->d_func()->m_accountList[MyMoneyAccount::stdAccName(eMyMoney::Account::Standard::Asset)].accountList().count(), 0);
        QCOMPARE(m->dirty(), true);
    } catch (const MyMoneyException &) {
        QFAIL("Unexpected exception");
    }
}

void MyMoneyStorageMgrTest::testAddInstitution()
{
    MyMoneyInstitution i;

    i.setName("Inst Name");

    m->addInstitution(i);
    QCOMPARE(m->d_func()->m_institutionList.count(), 1);
    QCOMPARE(m->d_func()->m_nextInstitutionID, 1ul);
    QCOMPARE(m->d_func()->m_institutionList["I000001"].name(), QLatin1String("Inst Name"));
}

void MyMoneyStorageMgrTest::testInstitution()
{
    testAddInstitution();
    MyMoneyInstitution i;

    m->d_func()->m_dirty = false;

    // try to find unknown institution
    try {
        i = m->institution("Unknown ID");
        QFAIL("Exception expected");
    } catch (const MyMoneyException &) {
    }

    QCOMPARE(m->dirty(), false);

    // now try to find real institution
    try {
        i = m->institution("I000001");
        QCOMPARE(i.name(), QLatin1String("Inst Name"));
        QCOMPARE(m->dirty(), false);
    } catch (const MyMoneyException &) {
        QFAIL("Unexpected exception");
    }
}

void MyMoneyStorageMgrTest::testAccount2Institution()
{
    testAddInstitution();
    testAddNewAccount();

    MyMoneyInstitution i;
    MyMoneyAccount a, b;

    try {
        i = m->institution("I000001");
        a = m->account("A000001");
    } catch (const MyMoneyException &) {
        QFAIL("Unexpected exception");
    }

    m->d_func()->m_dirty = false;

    // try to add to a false institution
    MyMoneyInstitution fake("Unknown ID", i);
    a.setInstitutionId(fake.id());
    try {
        m->modifyAccount(a);
        QFAIL("Exception expected");
    } catch (const MyMoneyException &) {
    }
    m->commitTransaction();
    m->startTransaction();

    QCOMPARE(m->dirty(), false);
    // now try to do it with a real institution
    try {
        QCOMPARE(i.accountList().count(), 0);
        a.setInstitutionId(i.id());
        m->modifyAccount(a);
        m->commitTransaction();
        m->startTransaction();
        QCOMPARE(m->dirty(), true);
        QCOMPARE(a.institutionId(), i.id());
        b = m->account("A000001");
        QCOMPARE(b.institutionId(), i.id());
        QCOMPARE(i.accountList().count(), 0);
    } catch (const MyMoneyException &) {
        QFAIL("Unexpected exception");
    }
}

void MyMoneyStorageMgrTest::testModifyAccount()
{
    testAccount2Institution();

    // test the OK case
    MyMoneyAccount a = m->account("A000001");
    a.setName("New account name");
    m->d_func()->m_dirty = false;
    try {
        m->modifyAccount(a);
        m->commitTransaction();
        m->startTransaction();
        MyMoneyAccount b = m->account("A000001");
        QCOMPARE(b.parentAccountId(), a.parentAccountId());
        QCOMPARE(b.name(), QLatin1String("New account name"));
        QCOMPARE(m->dirty(), true);
    } catch (const MyMoneyException &) {
        QFAIL("Unexpected exception");
    }

    // modify institution to unknown id
    MyMoneyAccount c("Unknown ID", a);
    m->d_func()->m_dirty = false;
    try {
        m->modifyAccount(c);
        QFAIL("Exception expected");
    } catch (const MyMoneyException &) {
    }

    // use different account type
    MyMoneyAccount d;
    d.setAccountType(Account::Type::CreditCard);
    MyMoneyAccount f("A000001", d);
    try {
        m->modifyAccount(f);
        QFAIL("Exception expected");
    } catch (const MyMoneyException &) {
    }

    // use different parent
    a.setParentAccountId("A000002");
    try {
        m->modifyAccount(c);
        QFAIL("Exception expected");
    } catch (const MyMoneyException &) {
    }
}

void MyMoneyStorageMgrTest::testModifyInstitution()
{
    testAddInstitution();
    MyMoneyInstitution i = m->institution("I000001");

    m->d_func()->m_dirty = false;
    i.setName("New inst name");
    try {
        m->modifyInstitution(i);
        m->commitTransaction();
        m->startTransaction();
        QCOMPARE(m->dirty(), true);
        i = m->institution("I000001");
        QCOMPARE(i.name(), QLatin1String("New inst name"));

    } catch (const MyMoneyException &) {
        QFAIL("Unexpected exception");
    }

    // try to modify an institution that does not exist
    MyMoneyInstitution f("Unknown ID", i);
    try {
        m->modifyInstitution(f);
        QFAIL("Exception expected");
    } catch (const MyMoneyException &) {
    }
}

void MyMoneyStorageMgrTest::testReparentAccount()
{
    // this one adds some accounts to the database
    MyMoneyAccount ex1;
    ex1.setAccountType(Account::Type::Expense);
    MyMoneyAccount ex2;
    ex2.setAccountType(Account::Type::Expense);
    MyMoneyAccount ex3;
    ex3.setAccountType(Account::Type::Expense);
    MyMoneyAccount ex4;
    ex4.setAccountType(Account::Type::Expense);
    MyMoneyAccount in;
    in.setAccountType(Account::Type::Income);
    MyMoneyAccount ch;
    ch.setAccountType(Account::Type::Checkings);

    ex1.setName("Sales Tax");
    ex2.setName("Sales Tax 16%");
    ex3.setName("Sales Tax 7%");
    ex4.setName("Grosseries");

    in.setName("Salary");
    ch.setName("My checkings account");

    try {
        m->addAccount(ex1);
        m->addAccount(ex2);
        m->addAccount(ex3);
        m->addAccount(ex4);
        m->addAccount(in);
        m->addAccount(ch);

        QCOMPARE(ex1.id(), QLatin1String("A000001"));
        QCOMPARE(ex2.id(), QLatin1String("A000002"));
        QCOMPARE(ex3.id(), QLatin1String("A000003"));
        QCOMPARE(ex4.id(), QLatin1String("A000004"));
        QCOMPARE(in.id(), QLatin1String("A000005"));
        QCOMPARE(ch.id(), QLatin1String("A000006"));

        MyMoneyAccount parent = m->expense();

        m->addAccount(parent, ex1);
        m->addAccount(ex1, ex2);
        m->addAccount(parent, ex3);
        m->addAccount(parent, ex4);

        parent = m->income();
        m->addAccount(parent, in);

        parent = m->asset();
        m->addAccount(parent, ch);

        QCOMPARE(m->expense().accountCount(), 3);
        QCOMPARE(m->account(ex1.id()).accountCount(), 1);
        QCOMPARE(ex3.parentAccountId(), MyMoneyAccount::stdAccName(eMyMoney::Account::Standard::Expense));

        m->reparentAccount(ex3, ex1);
        QCOMPARE(m->expense().accountCount(), 2);
        QCOMPARE(m->account(ex1.id()).accountCount(), 2);
        QCOMPARE(ex3.parentAccountId(), ex1.id());
    } catch (const MyMoneyException &e) {
        std::cout << std::endl << e.what() << std::endl;
        QFAIL("Unexpected exception");
    }
}

void MyMoneyStorageMgrTest::testAddTransactions()
{
    testReparentAccount();

    MyMoneyAccount ch;
    MyMoneyTransaction t1, t2;
    MyMoneySplit s;

    try {
        // I made some money, great
        s.setAccountId("A000006"); // Checkings
        s.setShares(MyMoneyMoney(100000, 100));
        s.setValue(MyMoneyMoney(100000, 100));
        QVERIFY(s.id().isEmpty());
        t1.addSplit(s);

        s.d_func()->setId(QString()); // enable re-usage of split variable
        s.setAccountId("A000005"); // Salary
        s.setShares(MyMoneyMoney(-100000, 100));
        s.setValue(MyMoneyMoney(-100000, 100));
        QVERIFY(s.id().isEmpty());
        t1.addSplit(s);

        t1.setPostDate(QDate(2002, 5, 10));
    } catch (const MyMoneyException &e) {
        unexpectedException(e);
    }

    m->d_func()->m_dirty = false;
    try {
        m->addTransaction(t1);
        m->commitTransaction();
        m->startTransaction();
        QCOMPARE(m->dirty(), true);
        QCOMPARE(t1.id(), QLatin1String("T000000000000000001"));
        QCOMPARE(t1.splitCount(), 2u);
        QCOMPARE(m->transactionCount(QString()), 1u);
    } catch (const MyMoneyException &e) {
        unexpectedException(e);
    }

    try {
        // I spent some money, not so great
        s.d_func()->setId(QString()); // enable re-usage of split variable
        s.setAccountId("A000004"); // Grosseries
        s.setShares(MyMoneyMoney(10000, 100));
        s.setValue(MyMoneyMoney(10000, 100));
        QVERIFY(s.id().isEmpty());
        t2.addSplit(s);

        s.d_func()->setId(QString()); // enable re-usage of split variable
        s.setAccountId("A000002"); // 16% sales tax
        s.setShares(MyMoneyMoney(1200, 100));
        s.setValue(MyMoneyMoney(1200, 100));
        QVERIFY(s.id().isEmpty());
        t2.addSplit(s);

        s.d_func()->setId(QString()); // enable re-usage of split variable
        s.setAccountId("A000003"); // 7% sales tax
        s.setShares(MyMoneyMoney(400, 100));
        s.setValue(MyMoneyMoney(400, 100));
        QVERIFY(s.id().isEmpty());
        t2.addSplit(s);

        s.d_func()->setId(QString()); // enable re-usage of split variable
        s.setAccountId("A000006"); // Checkings account
        s.setShares(MyMoneyMoney(-11600, 100));
        s.setValue(MyMoneyMoney(-11600, 100));
        QVERIFY(s.id().isEmpty());
        t2.addSplit(s);

        t2.setPostDate(QDate(2002, 5, 9));
    } catch (const MyMoneyException &e) {
        unexpectedException(e);
    }
    m->d_func()->m_dirty = false;
    try {
        m->addTransaction(t2);
        m->commitTransaction();
        m->startTransaction();
        QCOMPARE(m->dirty(), true);
        QCOMPARE(t2.id(), QLatin1String("T000000000000000002"));
        QCOMPARE(t2.splitCount(), 4u);
        QCOMPARE(m->transactionCount(QString()), 2u);

        QMap<QString, QString>::ConstIterator it_k;
        QMap<QString, MyMoneyTransaction>::ConstIterator it_t;
        it_k = m->d_func()->m_transactionKeys.begin();
        it_t = m->d_func()->m_transactionList.begin();

        QCOMPARE((*it_k), QLatin1String("2002-05-10-T000000000000000001"));
        QCOMPARE((*it_t).id(), QLatin1String("T000000000000000002"));
        ++it_k;
        ++it_t;
        QCOMPARE((*it_k), QLatin1String("2002-05-09-T000000000000000002"));
        QCOMPARE((*it_t).id(), QLatin1String("T000000000000000001"));
        ++it_k;
        ++it_t;
        QCOMPARE(it_k, m->d_func()->m_transactionKeys.end());
        QCOMPARE(it_t, m->d_func()->m_transactionList.end());

        ch = m->account("A000006");

        // check that the account's transaction list is updated
        QList<MyMoneyTransaction> list;
        MyMoneyTransactionFilter filter("A000006");
        list = m->transactionList(filter);
        QCOMPARE(list.size(), 2);

        QList<MyMoneyTransaction>::ConstIterator it;
        it = list.constBegin();
        QCOMPARE((*it).id(), QLatin1String("T000000000000000002"));
        ++it;
        QCOMPARE((*it).id(), QLatin1String("T000000000000000001"));
        ++it;
        QCOMPARE(it, list.constEnd());
    } catch (const MyMoneyException &e) {
        unexpectedException(e);
    }
}

void MyMoneyStorageMgrTest::testTransactionCount()
{
    testAddTransactions();
    QCOMPARE(m->transactionCount("A000001"), 0u);
    QCOMPARE(m->transactionCount("A000002"), 1u);
    QCOMPARE(m->transactionCount("A000003"), 1u);
    QCOMPARE(m->transactionCount("A000004"), 1u);
    QCOMPARE(m->transactionCount("A000005"), 1u);
    QCOMPARE(m->transactionCount("A000006"), 2u);
}

void MyMoneyStorageMgrTest::testBalance()
{
    testAddTransactions();

    QVERIFY(m->balance("A000001", QDate()).isZero());
    QCOMPARE(m->balance("A000002", QDate()),  MyMoneyMoney(1200, 100));
    QCOMPARE(m->balance("A000003", QDate()),  MyMoneyMoney(400, 100));
    QCOMPARE(m->totalBalance("A000001", QDate()),  MyMoneyMoney(1600, 100));
    QCOMPARE(m->balance("A000006", QDate(2002, 5, 9)),  MyMoneyMoney(-11600, 100));
    QCOMPARE(m->balance("A000005", QDate(2002, 5, 10)),  MyMoneyMoney(-100000, 100));
    QCOMPARE(m->balance("A000006", QDate(2002, 5, 10)),  MyMoneyMoney(88400, 100));
}

void MyMoneyStorageMgrTest::testModifyTransaction()
{
    testAddTransactions();

    MyMoneyTransaction t = m->transaction("T000000000000000002");
    MyMoneySplit s;
    MyMoneyAccount ch;

    // just modify simple stuff (splits)
    QCOMPARE(t.splitCount(), 4u);

    s = t.splits()[0];
    s.setShares(MyMoneyMoney(11000, 100));
    s.setValue(MyMoneyMoney(11000, 100));
    t.modifySplit(s);

    QCOMPARE(t.splitCount(), 4u);
    s = t.splits()[3];
    s.setShares(MyMoneyMoney(-12600, 100));
    s.setValue(MyMoneyMoney(-12600, 100));
    t.modifySplit(s);

    try {
        QCOMPARE(m->balance("A000004", QDate()),  MyMoneyMoney(10000, 100));
        QCOMPARE(m->balance("A000006", QDate()),  MyMoneyMoney(100000 - 11600, 100));
        QCOMPARE(m->totalBalance("A000001", QDate()),  MyMoneyMoney(1600, 100));
        m->modifyTransaction(t);
        QCOMPARE(m->balance("A000004", QDate()),  MyMoneyMoney(11000, 100));
        QCOMPARE(m->balance("A000006", QDate()),  MyMoneyMoney(100000 - 12600, 100));
        QCOMPARE(m->totalBalance("A000001", QDate()),  MyMoneyMoney(1600, 100));
    } catch (const MyMoneyException &) {
        QFAIL("Unexpected exception");
    }

    // now modify the date
    t.setPostDate(QDate(2002, 5, 11));
    try {
        m->modifyTransaction(t);
        QCOMPARE(m->balance("A000004", QDate()),  MyMoneyMoney(11000, 100));
        QCOMPARE(m->balance("A000006", QDate()),  MyMoneyMoney(100000 - 12600, 100));
        QCOMPARE(m->totalBalance("A000001", QDate()),  MyMoneyMoney(1600, 100));

        QMap<QString, QString>::ConstIterator it_k;
        QMap<QString, MyMoneyTransaction>::ConstIterator it_t;
        it_k = m->d_func()->m_transactionKeys.begin();
        it_t = m->d_func()->m_transactionList.begin();
        QCOMPARE((*it_k), QLatin1String("2002-05-10-T000000000000000001"));
        QCOMPARE((*it_t).id(), QLatin1String("T000000000000000001"));
        ++it_k;
        ++it_t;
        QCOMPARE((*it_k), QLatin1String("2002-05-11-T000000000000000002"));
        QCOMPARE((*it_t).id(), QLatin1String("T000000000000000002"));
        ++it_k;
        ++it_t;
        QCOMPARE(it_k, m->d_func()->m_transactionKeys.end());
        QCOMPARE(it_t, m->d_func()->m_transactionList.end());

        ch = m->account("A000006");

        // check that the account's transaction list is updated
        QList<MyMoneyTransaction> list;
        MyMoneyTransactionFilter filter("A000006");
        list = m->transactionList(filter);
        QCOMPARE(list.size(), 2);

        QList<MyMoneyTransaction>::ConstIterator it;
        it = list.constBegin();
        QCOMPARE((*it).id(), QLatin1String("T000000000000000001"));
        ++it;
        QCOMPARE((*it).id(), QLatin1String("T000000000000000002"));
        ++it;
        QCOMPARE(it, list.constEnd());
    } catch (const MyMoneyException &) {
        QFAIL("Unexpected exception");
    }
}


void MyMoneyStorageMgrTest::testRemoveUnusedAccount()
{
    testAccount2Institution();

    MyMoneyAccount a = m->account("A000001");
    MyMoneyInstitution i = m->institution("I000001");

    m->d_func()->m_dirty = false;
    // make sure, we cannot remove the standard account groups
    try {
        m->removeAccount(m->liability());
        QFAIL("Exception expected");
    } catch (const MyMoneyException &) {
    }

    try {
        m->removeAccount(m->asset());
        QFAIL("Exception expected");
    } catch (const MyMoneyException &) {
    }

    try {
        m->removeAccount(m->expense());
        QFAIL("Exception expected");
    } catch (const MyMoneyException &) {
    }

    try {
        m->removeAccount(m->income());
        QFAIL("Exception expected");
    } catch (const MyMoneyException &) {
    }

    // try to remove the account still attached to the institution
    try {
        m->removeAccount(a);
        QFAIL("Exception expected");
    } catch (const MyMoneyException &) {
    }

    // now really remove an account

    try {
        QCOMPARE(i.accountCount(), 0u);
        QCOMPARE(m->accountCount(), 7u);

        a.setInstitutionId(QString());
        m->modifyAccount(a);
        m->removeAccount(a);
        m->commitTransaction();
        m->startTransaction();
        QCOMPARE(m->accountCount(), 6u);
        QCOMPARE(m->dirty(), true);
        i = m->institution("I000001");
        QCOMPARE(i.accountCount(), 0u);
    } catch (const MyMoneyException &) {
        QFAIL("Unexpected exception");
    }
}

void MyMoneyStorageMgrTest::testRemoveUsedAccount()
{
    testAddTransactions();

    MyMoneyAccount a = m->account("A000006");

    try {
        m->removeAccount(a);
        QFAIL("Exception expected");
    } catch (const MyMoneyException &) {
    }
}

void MyMoneyStorageMgrTest::testRemoveInstitution()
{
    testModifyInstitution();
    testReparentAccount();

    MyMoneyInstitution i;
    MyMoneyAccount a;

    // assign the checkings account to the institution
    try {
        i = m->institution("I000001");
        a = m->account("A000006");
        a.setInstitutionId(i.id());
        m->modifyAccount(a);
        QCOMPARE(i.accountCount(), 0u);
    } catch (const MyMoneyException &) {
        QFAIL("Unexpected exception");
    }

    m->d_func()->m_dirty = false;
    // now remove the institution and see if the account survived ;-)
    try {
        m->removeInstitution(i);
        a.setInstitutionId(QString());
        m->modifyAccount(a);
        m->commitTransaction();
        m->startTransaction();
        a = m->account("A000006");
        QCOMPARE(m->dirty(), true);
        QVERIFY(a.institutionId().isEmpty());
        QCOMPARE(m->institutionCount(), 0u);
    } catch (const MyMoneyException &) {
        QFAIL("Unexpected exception");
    }
}

void MyMoneyStorageMgrTest::testRemoveTransaction()
{
    testAddTransactions();

    MyMoneyTransaction t = m->transaction("T000000000000000002");

    m->d_func()->m_dirty = false;
    try {
        m->removeTransaction(t);
        m->commitTransaction();
        m->startTransaction();
        QCOMPARE(m->dirty(), true);
        QCOMPARE(m->transactionCount(QString()), 1u);
        /* removed with MyMoneyAccount::Transaction
          QCOMPARE(m->account("A000006").transactionCount(QString()), 1);
        */
    } catch (const MyMoneyException &) {
        QFAIL("Unexpected exception");
    }
}

void MyMoneyStorageMgrTest::testTransactionList()
{
    testAddTransactions();

    QList<MyMoneyTransaction> list;
    MyMoneyTransactionFilter filter("A000006");
    list = m->transactionList(filter);
    QCOMPARE(list.count(), 2);
    QCOMPARE(list.at(0).id(), QLatin1String("T000000000000000002"));
    QCOMPARE(list.at(1).id(), QLatin1String("T000000000000000001"));

    filter.clear();
    filter.addAccount(QString("A000003"));
    list = m->transactionList(filter);
    QCOMPARE(list.count(), 1);
    QCOMPARE(list.at(0).id(), QLatin1String("T000000000000000002"));

    filter.clear();
    list = m->transactionList(filter);
    QCOMPARE(list.count(), 2);
    QCOMPARE(list.at(0).id(), QLatin1String("T000000000000000002"));
    QCOMPARE(list.at(1).id(), QLatin1String("T000000000000000001"));
}

void MyMoneyStorageMgrTest::testAddPayee()
{
    MyMoneyPayee p;

    p.setName("THB");
    m->d_func()->m_dirty = false;
    try {
        QCOMPARE(m->d_func()->m_nextPayeeID, 0ul);
        m->addPayee(p);
        m->commitTransaction();
        m->startTransaction();
        QCOMPARE(m->dirty(), true);
        QCOMPARE(m->d_func()->m_nextPayeeID, 1ul);
    } catch (const MyMoneyException &) {
        QFAIL("Unexpected exception");
    }

}

void MyMoneyStorageMgrTest::testSetAccountName()
{
    try {
        m->setAccountName(MyMoneyAccount::stdAccName(eMyMoney::Account::Standard::Liability), "Verbindlichkeiten");
    } catch (const MyMoneyException &) {
        QFAIL("Unexpected exception");
    }
    try {
        m->setAccountName(MyMoneyAccount::stdAccName(eMyMoney::Account::Standard::Asset), QString("Vermögen"));
    } catch (const MyMoneyException &) {
        QFAIL("Unexpected exception");
    }
    try {
        m->setAccountName(MyMoneyAccount::stdAccName(eMyMoney::Account::Standard::Expense), "Ausgaben");
    } catch (const MyMoneyException &) {
        QFAIL("Unexpected exception");
    }
    try {
        m->setAccountName(MyMoneyAccount::stdAccName(eMyMoney::Account::Standard::Income), "Einnahmen");
    } catch (const MyMoneyException &) {
        QFAIL("Unexpected exception");
    }

    QCOMPARE(m->liability().name(), QLatin1String("Verbindlichkeiten"));
    QCOMPARE(m->asset().name(), QString("Vermögen"));
    QCOMPARE(m->expense().name(), QLatin1String("Ausgaben"));
    QCOMPARE(m->income().name(), QLatin1String("Einnahmen"));

    try {
        m->setAccountName("A000001", "New account name");
        QFAIL("Exception expected");
    } catch (const MyMoneyException &) {
    }
}

void MyMoneyStorageMgrTest::testModifyPayee()
{
    MyMoneyPayee p;

    testAddPayee();

    p = m->payee("P000001");
    p.setName("New name");
    m->d_func()->m_dirty = false;
    try {
        m->modifyPayee(p);
        m->commitTransaction();
        m->startTransaction();
        p = m->payee("P000001");
        QCOMPARE(p.name(), QLatin1String("New name"));
        QCOMPARE(m->dirty(), true);
    } catch (const MyMoneyException &) {
        QFAIL("Unexpected exception");
    }
}

void MyMoneyStorageMgrTest::testRemovePayee()
{
    testAddPayee();
    m->d_func()->m_dirty = false;

    // check that we can remove an unreferenced payee
    MyMoneyPayee p = m->payee("P000001");
    try {
        QCOMPARE(m->d_func()->m_payeeList.count(), 1);
        m->removePayee(p);
        m->commitTransaction();
        m->startTransaction();
        QCOMPARE(m->d_func()->m_payeeList.count(), 0);
        QCOMPARE(m->dirty(), true);
    } catch (const MyMoneyException &) {
        QFAIL("Unexpected exception");
    }

    // add transaction
    testAddTransactions();

    MyMoneyTransaction tr = m->transaction("T000000000000000001");
    MyMoneySplit sp;
    sp = tr.splits()[0];
    sp.setPayeeId("P000001");
    tr.modifySplit(sp);

    // check that we cannot add a transaction referencing
    // an unknown payee
    try {
        m->modifyTransaction(tr);
        QFAIL("Expected exception");
    } catch (const MyMoneyException &) {
    }

    m->d_func()->m_nextPayeeID = 0;  // reset here, so that the
    // testAddPayee will not fail
    testAddPayee();

    // check that it works when the payee exists
    try {
        m->modifyTransaction(tr);
    } catch (const MyMoneyException &) {
        QFAIL("Unexpected exception");
    }

    m->d_func()->m_dirty = false;

    // now check, that we cannot remove the payee
    try {
        m->removePayee(p);
        QFAIL("Expected exception");
    } catch (const MyMoneyException &) {
    }
    QCOMPARE(m->d_func()->m_payeeList.count(), 1);
}

void MyMoneyStorageMgrTest::testAddTag()
{
    MyMoneyTag ta;

    ta.setName("THB");
    m->d_func()->m_dirty = false;
    try {
        QCOMPARE(m->d_func()->m_nextTagID, 0ul);
        m->addTag(ta);
        m->commitTransaction();
        m->startTransaction();
        QCOMPARE(m->dirty(), true);
        QCOMPARE(m->d_func()->m_nextTagID, 1ul);
    } catch (const MyMoneyException &) {
        QFAIL("Unexpected exception");
    }
}

void MyMoneyStorageMgrTest::testModifyTag()
{
    MyMoneyTag ta;

    testAddTag();

    ta = m->tag("G000001");
    ta.setName("New name");
    m->d_func()->m_dirty = false;
    try {
        m->modifyTag(ta);
        m->commitTransaction();
        m->startTransaction();
        ta = m->tag("G000001");
        QCOMPARE(ta.name(), QLatin1String("New name"));
        QCOMPARE(m->dirty(), true);
    } catch (const MyMoneyException &) {
        QFAIL("Unexpected exception");
    }
}

void MyMoneyStorageMgrTest::testRemoveTag()
{
    testAddTag();
    m->d_func()->m_dirty = false;

    // check that we can remove an unreferenced tag
    MyMoneyTag ta = m->tag("G000001");
    try {
        QCOMPARE(m->d_func()->m_tagList.count(), 1);
        m->removeTag(ta);
        m->commitTransaction();
        m->startTransaction();
        QCOMPARE(m->d_func()->m_tagList.count(), 0);
        QCOMPARE(m->dirty(), true);
    } catch (const MyMoneyException &) {
        QFAIL("Unexpected exception");
    }

    // add transaction
    testAddTransactions();

    MyMoneyTransaction tr = m->transaction("T000000000000000001");
    MyMoneySplit sp;
    sp = tr.splits()[0];
    QList<QString> tagIdList;
    tagIdList << "G000001";
    sp.setTagIdList(tagIdList);
    tr.modifySplit(sp);

    // check that we cannot add a transaction referencing
    // an unknown tag
    try {
        m->modifyTransaction(tr);
        QFAIL("Expected exception");
    } catch (const MyMoneyException &) {
    }

    m->d_func()->m_nextTagID = 0;  // reset here, so that the
    // testAddTag will not fail
    testAddTag();

    // check that it works when the tag exists
    try {
        m->modifyTransaction(tr);
    } catch (const MyMoneyException &) {
        QFAIL("Unexpected exception");
    }

    m->d_func()->m_dirty = false;

    // now check, that we cannot remove the tag
    try {
        m->removeTag(ta);
        QFAIL("Expected exception");
    } catch (const MyMoneyException &) {
    }
    QCOMPARE(m->d_func()->m_tagList.count(), 1);
}

void MyMoneyStorageMgrTest::testRemoveAccountFromTree()
{
    MyMoneyAccount a, b, c;
    a.setName("Acc A");
    b.setName("Acc B");
    c.setName("Acc C");

    // build a tree A -> B -> C, remove B and see if A -> C
    // remains in the storage manager

    try {
        m->addAccount(a);
        m->addAccount(b);
        m->addAccount(c);
        m->reparentAccount(b, a);
        m->reparentAccount(c, b);

        QCOMPARE(a.accountList().count(), 1);
        QCOMPARE(m->account(a.accountList()[0]).name(), QLatin1String("Acc B"));

        QCOMPARE(b.accountList().count(), 1);
        QCOMPARE(m->account(b.accountList()[0]).name(), QLatin1String("Acc C"));

        QCOMPARE(c.accountList().count(), 0);

        m->removeAccount(b);

        // reload account info from titutionIDtorage
        a = m->account(a.id());
        c = m->account(c.id());

        try {
            b = m->account(b.id());
            QFAIL("Exception expected");
        } catch (const MyMoneyException &) {
        }
        QCOMPARE(a.accountList().count(), 1);
        QCOMPARE(m->account(a.accountList()[0]).name(), QLatin1String("Acc C"));

        QCOMPARE(c.accountList().count(), 0);

    } catch (const MyMoneyException &e) {
        unexpectedException(e);
    }
}

void MyMoneyStorageMgrTest::testPayeeName()
{
    testAddPayee();

    MyMoneyPayee p;
    QString name("THB");

    // OK case
    try {
        p = m->payeeByName(name);
        QCOMPARE(p.name(), QLatin1String("THB"));
        QCOMPARE(p.id(), QLatin1String("P000001"));
    } catch (const MyMoneyException &e) {
        unexpectedException(e);
    }

    // Not OK case
    name = "Thb";
    try {
        p = m->payeeByName(name);
        QFAIL("Exception expected");
    } catch (const MyMoneyException &) {
    }
}

void MyMoneyStorageMgrTest::testTagName()
{
    testAddTag();

    MyMoneyTag ta;
    QString name("THB");

    // OK case
    try {
        ta = m->tagByName(name);
        QCOMPARE(ta.name(), QLatin1String("THB"));
        QCOMPARE(ta.id(), QLatin1String("G000001"));
    } catch (const MyMoneyException &e) {
        unexpectedException(e);
    }

    // Not OK case
    name = "Thb";
    try {
        ta = m->tagByName(name);
        QFAIL("Exception expected");
    } catch (const MyMoneyException &) {
    }
}

// disabled because of no real world use case
//void MyMoneyStorageMgrTest::testAssignment()
//{
//  testAddTransactions();

//  MyMoneyPayee user;
//  user.setName("Thomas");
//  m->setUser(user);

//  MyMoneyStorageMgr test = *m;
//  testEquality(&test);
//}

void MyMoneyStorageMgrTest::testEquality(const MyMoneyStorageMgr *t)
{
    QCOMPARE(m->user().name(), t->user().name());
    QCOMPARE(m->user().address(), t->user().address());
    QCOMPARE(m->user().city(), t->user().city());
    QCOMPARE(m->user().state(), t->user().state());
    QCOMPARE(m->user().postcode(), t->user().postcode());
    QCOMPARE(m->user().telephone(), t->user().telephone());
    QCOMPARE(m->user().email(), t->user().email());
    QCOMPARE(m->d_func()->m_nextInstitutionID, t->d_func()->m_nextInstitutionID);
    QCOMPARE(m->d_func()->m_nextAccountID, t->d_func()->m_nextAccountID);
    QCOMPARE(m->d_func()->m_nextTransactionID, t->d_func()->m_nextTransactionID);
    QCOMPARE(m->d_func()->m_nextPayeeID, t->d_func()->m_nextPayeeID);
    QCOMPARE(m->d_func()->m_nextTagID, t->d_func()->m_nextTagID);
    QCOMPARE(m->d_func()->m_nextScheduleID, t->d_func()->m_nextScheduleID);
    QCOMPARE(m->dirty(), t->dirty());
    QCOMPARE(m->creationDate(), t->creationDate());
    QCOMPARE(m->lastModificationDate(), t->lastModificationDate());

    /*
     * make sure, that the keys and values are the same
     * on the left and the right side
     */
    QCOMPARE(m->d_func()->m_payeeList.keys(), t->d_func()->m_payeeList.keys());
    QCOMPARE(m->d_func()->m_payeeList.values(), t->d_func()->m_payeeList.values());
    QCOMPARE(m->d_func()->m_tagList.keys(), t->d_func()->m_tagList.keys());
    QCOMPARE(m->d_func()->m_tagList.values(), t->d_func()->m_tagList.values());
    QCOMPARE(m->d_func()->m_transactionKeys.keys(), t->d_func()->m_transactionKeys.keys());
    QCOMPARE(m->d_func()->m_transactionKeys.values(), t->d_func()->m_transactionKeys.values());
    QCOMPARE(m->d_func()->m_institutionList.keys(), t->d_func()->m_institutionList.keys());
    QCOMPARE(m->d_func()->m_institutionList.values(), t->d_func()->m_institutionList.values());
    QCOMPARE(m->d_func()->m_accountList.keys(), t->d_func()->m_accountList.keys());
    QCOMPARE(m->d_func()->m_accountList.values(), t->d_func()->m_accountList.values());
    QCOMPARE(m->d_func()->m_transactionList.keys(), t->d_func()->m_transactionList.keys());
    QCOMPARE(m->d_func()->m_transactionList.values(), t->d_func()->m_transactionList.values());

// QCOMPARE(m->d_func()->m_scheduleList.keys(), t->m_scheduleList.keys());
// QCOMPARE(m->d_func()->m_scheduleList.values(), t->m_scheduleList.values());
}

// disabled because of no real world use case
//void MyMoneyStorageMgrTest::testDuplicate()
//{
//  const MyMoneyStorageMgr* t;

//  testModifyTransaction();

//  t = m->duplicate();
//  testEquality(t);
//  delete t;
//}

void MyMoneyStorageMgrTest::testAddSchedule()
{
    /* Note addSchedule() now calls validate as it should
     * so we need an account id.  Later this will
     * be checked to make sure its a valid account id.  The
     * tests currently fail because no splits are defined
     * for the schedules transaction.
    */


    try {
        QCOMPARE(m->d_func()->m_scheduleList.count(), 0);
        MyMoneyTransaction t1;
        MyMoneySplit s1, s2;
        s1.setAccountId("A000001");
        t1.addSplit(s1);
        s2.setAccountId("A000002");
        t1.addSplit(s2);
        MyMoneyAccount a1("A000001", MyMoneyAccount());
        MyMoneyAccount a2("A000002", MyMoneyAccount());
        m->addAccount(a1);
        m->addAccount(a2);

        MyMoneySchedule schedule("Sched-Name",
                                 Schedule::Type::Deposit,
                                 Schedule::Occurrence::Daily, 1,
                                 Schedule::PaymentType::ManualDeposit,
                                 QDate(),
                                 QDate(),
                                 true,
                                 false);
        t1.setPostDate(QDate(2003, 7, 10));
        schedule.setTransaction(t1);
        m->addSchedule(schedule);

        QCOMPARE(m->d_func()->m_scheduleList.count(), 1);
        QCOMPARE(schedule.id(), QLatin1String("SCH000001"));
        QCOMPARE(m->d_func()->m_scheduleList["SCH000001"].id(), QLatin1String("SCH000001"));
    } catch (const MyMoneyException &) {
        QFAIL("Unexpected exception");
    }

    try {
        MyMoneySchedule schedule("Sched-Name",
                                 Schedule::Type::Deposit,
                                 Schedule::Occurrence::Daily, 1,
                                 Schedule::PaymentType::ManualDeposit,
                                 QDate(),
                                 QDate(),
                                 true,
                                 false);
        m->addSchedule(schedule);
        QFAIL("Exception expected");
    } catch (const MyMoneyException &) {
    }
}

void MyMoneyStorageMgrTest::testSchedule()
{
    testAddSchedule();
    MyMoneySchedule sched;

    sched = m->schedule("SCH000001");
    QCOMPARE(sched.name(), QLatin1String("Sched-Name"));
    QCOMPARE(sched.id(), QLatin1String("SCH000001"));

    try {
        m->schedule("SCH000002");
        QFAIL("Exception expected");
    } catch (const MyMoneyException &) {
    }
}

void MyMoneyStorageMgrTest::testModifySchedule()
{
    testAddSchedule();
    MyMoneySchedule sched;

    sched = m->schedule("SCH000001");
    sched.d_func()->setId("SCH000002");
    try {
        m->modifySchedule(sched);
        QFAIL("Exception expected");
    } catch (const MyMoneyException &) {
    }

    sched = m->schedule("SCH000001");
    sched.setName("New Sched-Name");
    try {
        m->modifySchedule(sched);
        QCOMPARE(m->d_func()->m_scheduleList.count(), 1);
        QCOMPARE(m->d_func()->m_scheduleList["SCH000001"].name(), QLatin1String("New Sched-Name"));

    } catch (const MyMoneyException &) {
        QFAIL("Unexpected exception");
    }

}

void MyMoneyStorageMgrTest::testRemoveSchedule()
{
    testAddSchedule();
    m->commitTransaction();
    m->startTransaction();
    MyMoneySchedule sched;

    sched = m->schedule("SCH000001");
    sched.d_func()->setId("SCH000002");
    try {
        m->removeSchedule(sched);
        m->commitTransaction();
        QFAIL("Exception expected");
    } catch (const MyMoneyException &) {
        m->rollbackTransaction();
    }
    m->startTransaction();

    sched = m->schedule("SCH000001");
    try {
        m->removeSchedule(sched);
        m->commitTransaction();
        QCOMPARE(m->d_func()->m_scheduleList.count(), 0);

    } catch (const MyMoneyException &) {
        m->rollbackTransaction();
        QFAIL("Unexpected exception");
    }
    m->startTransaction();
}

void MyMoneyStorageMgrTest::testScheduleList()
{
    QDate testDate = QDate::currentDate();
    QDate notOverdue = testDate.addDays(2);
    QDate overdue = testDate.addDays(-2);


    MyMoneyTransaction t1;
    MyMoneySplit s1, s2;
    s1.setAccountId("A000001");
    t1.addSplit(s1);
    s2.setAccountId("A000002");
    t1.addSplit(s2);
    MyMoneyAccount a1("A000001", MyMoneyAccount());
    MyMoneyAccount a2("A000002", MyMoneyAccount());
    m->addAccount(a1);
    m->addAccount(a2);
    MyMoneySchedule schedule1("Schedule 1",
                              Schedule::Type::Bill,
                              Schedule::Occurrence::Once, 1,
                              Schedule::PaymentType::DirectDebit,
                              QDate(),
                              QDate(),
                              false,
                              false);
    t1.setPostDate(notOverdue);
    schedule1.setTransaction(t1);
    schedule1.setLastPayment(notOverdue);

    MyMoneyTransaction t2;
    MyMoneySplit s3, s4;
    s3.setAccountId("A000001");
    t2.addSplit(s3);
    s4.setAccountId("A000003");
    t2.addSplit(s4);
    MyMoneyAccount a3("A000003", MyMoneyAccount());
    m->addAccount(a3);
    MyMoneySchedule schedule2("Schedule 2",
                              Schedule::Type::Deposit,
                              Schedule::Occurrence::Daily, 1,
                              Schedule::PaymentType::DirectDeposit,
                              QDate(),
                              QDate(),
                              false,
                              false);
    t2.setPostDate(notOverdue.addDays(1));
    schedule2.setTransaction(t2);
    schedule2.setLastPayment(notOverdue.addDays(1));

    MyMoneyTransaction t3;
    MyMoneySplit s5, s6;
    s5.setAccountId("A000005");
    t3.addSplit(s5);
    s6.setAccountId("A000006");
    t3.addSplit(s6);
    MyMoneyAccount a5("A000005", MyMoneyAccount());
    MyMoneyAccount a6("A000006", MyMoneyAccount());
    MyMoneyAccount a7("A000007", MyMoneyAccount());
    m->addAccount(a5);
    m->addAccount(a6);
    m->addAccount(a7);
    MyMoneySchedule schedule3("Schedule 3",
                              Schedule::Type::Transfer,
                              Schedule::Occurrence::Weekly, 1,
                              Schedule::PaymentType::Other,
                              QDate(),
                              QDate(),
                              false,
                              false);
    t3.setPostDate(notOverdue.addDays(2));
    schedule3.setTransaction(t3);
    schedule3.setLastPayment(notOverdue.addDays(2));

    MyMoneyTransaction t4;
    MyMoneySplit s7, s8;
    s7.setAccountId("A000005");
    t4.addSplit(s7);
    s8.setAccountId("A000006");
    t4.addSplit(s8);
    MyMoneySchedule schedule4("Schedule 4",
                              Schedule::Type::Bill,
                              Schedule::Occurrence::Weekly, 1,
                              Schedule::PaymentType::WriteChecque,
                              QDate(),
                              notOverdue.addDays(31),
                              false,
                              false);
    t4.setPostDate(overdue.addDays(-7));
    schedule4.setTransaction(t4);

    try {
        m->addSchedule(schedule1);
        m->addSchedule(schedule2);
        m->addSchedule(schedule3);
        m->addSchedule(schedule4);
    } catch (const MyMoneyException &e) {
        qDebug("Error: %s", e.what());
        QFAIL("Unexpected exception");
    }

    QList<MyMoneySchedule> list;

    // no filter
    list = m->scheduleList(QString(), Schedule::Type::Any, Schedule::Occurrence::Any, Schedule::PaymentType::Any,
                           QDate(), QDate(), false);
    QCOMPARE(list.count(), 4);

    // filter by type
    list = m->scheduleList("", Schedule::Type::Bill, Schedule::Occurrence::Any, Schedule::PaymentType::Any,
                           QDate(), QDate(), false);
    QCOMPARE(list.count(), 2);
    QCOMPARE(list[0].name(), QLatin1String("Schedule 1"));
    QCOMPARE(list[1].name(), QLatin1String("Schedule 4"));

    // filter by occurrence
    list = m->scheduleList("", Schedule::Type::Any,
                           Schedule::Occurrence::Daily, Schedule::PaymentType::Any,
                           QDate(), QDate(), false);
    QCOMPARE(list.count(), 1);
    QCOMPARE(list[0].name(), QLatin1String("Schedule 2"));

    // filter by payment type
    list = m->scheduleList("", Schedule::Type::Any,
                           Schedule::Occurrence::Any,
                           Schedule::PaymentType::DirectDeposit,
                           QDate(), QDate(), false);
    QCOMPARE(list.count(), 1);
    QCOMPARE(list[0].name(), QLatin1String("Schedule 2"));

    // filter by account
    list = m->scheduleList("A01", Schedule::Type::Any, Schedule::Occurrence::Any, Schedule::PaymentType::Any,
                           QDate(), QDate(), false);
    QCOMPARE(list.count(), 0);
    list = m->scheduleList("A000001", Schedule::Type::Any, Schedule::Occurrence::Any, Schedule::PaymentType::Any,
                           QDate(), QDate(), false);
    QCOMPARE(list.count(), 2);
    list = m->scheduleList("A000002", Schedule::Type::Any, Schedule::Occurrence::Any, Schedule::PaymentType::Any,
                           QDate(), QDate(), false);
    QCOMPARE(list.count(), 1);

    // filter by start date
    list = m->scheduleList("", Schedule::Type::Any,
                           Schedule::Occurrence::Any,
                           Schedule::PaymentType::Any,
                           notOverdue.addDays(31),
                           QDate(), false);
    QCOMPARE(list.count(), 3);
    QCOMPARE(list[0].name(), QLatin1String("Schedule 2"));
    QCOMPARE(list[1].name(), QLatin1String("Schedule 3"));
    QCOMPARE(list[2].name(), QLatin1String("Schedule 4"));

    // filter by end date
    list = m->scheduleList("", Schedule::Type::Any,
                           Schedule::Occurrence::Any,
                           Schedule::PaymentType::Any,
                           QDate(),
                           notOverdue.addDays(1),
                           false);
    QCOMPARE(list.count(), 3);
    QCOMPARE(list[0].name(), QLatin1String("Schedule 1"));
    QCOMPARE(list[1].name(), QLatin1String("Schedule 2"));
    QCOMPARE(list[2].name(), QLatin1String("Schedule 4"));

    // filter by start and end date
    list = m->scheduleList("", Schedule::Type::Any,
                           Schedule::Occurrence::Any,
                           Schedule::PaymentType::Any,
                           notOverdue.addDays(-1),
                           notOverdue.addDays(1),
                           false);
    QCOMPARE(list.count(), 2);
    QCOMPARE(list[0].name(), QLatin1String("Schedule 1"));
    QCOMPARE(list[1].name(), QLatin1String("Schedule 2"));

    // filter by overdue status
    list = m->scheduleList("", Schedule::Type::Any,
                           Schedule::Occurrence::Any,
                           Schedule::PaymentType::Any,
                           QDate(),
                           QDate(),
                           true);
    QCOMPARE(list.count(), 1);
    QCOMPARE(list[0].name(), QLatin1String("Schedule 4"));
}

void MyMoneyStorageMgrTest::testAddCurrency()
{
    MyMoneySecurity curr("EUR", "Euro", "?", 100, 100);
    QCOMPARE(m->d_func()->m_currencyList.count(), 0);
    m->d_func()->m_dirty = false;
    try {
        m->addCurrency(curr);
        m->commitTransaction();
        m->startTransaction();
        QCOMPARE(m->d_func()->m_currencyList.count(), 1);
        QCOMPARE(m->d_func()->m_currencyList["EUR"].name(), QLatin1String("Euro"));
        QCOMPARE(m->dirty(), true);
    } catch (const MyMoneyException &) {
        QFAIL("Unexpected exception");
    }

    m->d_func()->m_dirty = false;
    try {
        m->addCurrency(curr);
        QFAIL("Expected exception missing");
    } catch (const MyMoneyException &) {
        m->commitTransaction();
        m->startTransaction();
        QCOMPARE(m->dirty(), false);
    }
}

void MyMoneyStorageMgrTest::testModifyCurrency()
{
    MyMoneySecurity curr("EUR", "Euro", "?", 100, 100);
    testAddCurrency();
    m->d_func()->m_dirty = false;
    curr.setName("EURO");
    try {
        m->modifyCurrency(curr);
        m->commitTransaction();
        m->startTransaction();
        QCOMPARE(m->d_func()->m_currencyList.count(), 1);
        QCOMPARE(m->d_func()->m_currencyList["EUR"].name(), QLatin1String("EURO"));
        QCOMPARE(m->dirty(), true);
    } catch (const MyMoneyException &) {
        QFAIL("Unexpected exception");
    }

    m->d_func()->m_dirty = false;

    MyMoneySecurity unknownCurr("DEM", "Deutsche Mark", "DM", 100, 100);
    try {
        m->modifyCurrency(unknownCurr);
        QFAIL("Expected exception missing");
    } catch (const MyMoneyException &) {
        m->commitTransaction();
        m->startTransaction();
        QCOMPARE(m->dirty(), false);
    }
}

void MyMoneyStorageMgrTest::testRemoveCurrency()
{
    MyMoneySecurity curr("EUR", "Euro", "?", 100, 100);
    testAddCurrency();
    m->d_func()->m_dirty = false;
    try {
        m->removeCurrency(curr);
        m->commitTransaction();
        m->startTransaction();
        QCOMPARE(m->d_func()->m_currencyList.count(), 0);
        QCOMPARE(m->dirty(), true);
    } catch (const MyMoneyException &) {
        QFAIL("Unexpected exception");
    }

    m->d_func()->m_dirty = false;

    MyMoneySecurity unknownCurr("DEM", "Deutsche Mark", "DM", 100, 100);
    try {
        m->removeCurrency(unknownCurr);
        QFAIL("Expected exception missing");
    } catch (const MyMoneyException &) {
        m->commitTransaction();
        m->startTransaction();
        QCOMPARE(m->dirty(), false);
    }
}

void MyMoneyStorageMgrTest::testCurrency()
{
    MyMoneySecurity curr("EUR", "Euro", "?", 100, 100);
    MyMoneySecurity newCurr;
    testAddCurrency();
    m->d_func()->m_dirty = false;
    try {
        newCurr = m->currency("EUR");
        m->commitTransaction();
        m->startTransaction();
        QCOMPARE(m->dirty(), false);
        QCOMPARE(newCurr.id(), curr.id());
        QCOMPARE(newCurr.name(), curr.name());
    } catch (const MyMoneyException &) {
        QFAIL("Unexpected exception");
    }

    try {
        m->currency("DEM");
        QFAIL("Expected exception missing");
    } catch (const MyMoneyException &) {
        m->commitTransaction();
        m->startTransaction();
        QCOMPARE(m->dirty(), false);
    }
}

void MyMoneyStorageMgrTest::testCurrencyList()
{
    QCOMPARE(m->currencyList().count(), 0);

    testAddCurrency();
    QCOMPARE(m->currencyList().count(), 1);

    MyMoneySecurity unknownCurr("DEM", "Deutsche Mark", "DM", 100, 100);
    try {
        m->addCurrency(unknownCurr);
        m->d_func()->m_dirty = false;
        QCOMPARE(m->d_func()->m_currencyList.count(), 2);
        QCOMPARE(m->currencyList().count(), 2);
        QCOMPARE(m->dirty(), false);
    } catch (const MyMoneyException &) {
        QFAIL("Unexpected exception");
    }
}

void MyMoneyStorageMgrTest::testAccountList()
{
    QList<MyMoneyAccount> accounts;
    m->accountList(accounts);
    QCOMPARE(accounts.count(), 0);
    testAddNewAccount();
    accounts.clear();
    m->accountList(accounts);
    QCOMPARE(accounts.count(), 2);

    MyMoneyAccount a = m->account("A000001");
    MyMoneyAccount b = m->account("A000002");
    m->reparentAccount(b, a);
    accounts.clear();
    m->accountList(accounts);
    QCOMPARE(accounts.count(), 2);
}

void MyMoneyStorageMgrTest::testLoaderFunctions()
{
    // we don't need the transaction started by setup() here
    m->rollbackTransaction();

    // account loader
    QMap<QString, MyMoneyAccount> amap;
    MyMoneyAccount acc("A0000176", MyMoneyAccount());
    amap[acc.id()] = acc;
    m->loadAccounts(amap);
    QCOMPARE(m->d_func()->m_accountList.values(), amap.values());
    QCOMPARE(m->d_func()->m_accountList.keys(), amap.keys());
    QCOMPARE(m->d_func()->m_nextAccountID, 176ul);

    // transaction loader
    QMap<QString, MyMoneyTransaction> tmap;
    MyMoneyTransaction t("T000000108", MyMoneyTransaction());
    tmap[t.id()] = t;
    m->loadTransactions(tmap);
    QCOMPARE(m->d_func()->m_transactionList.values(), tmap.values());
    QCOMPARE(m->d_func()->m_transactionList.keys(), tmap.keys());
    QCOMPARE(m->d_func()->m_nextTransactionID, 108ul);

    // institution loader
    QMap<QString, MyMoneyInstitution> imap;
    MyMoneyInstitution inst("I000028", MyMoneyInstitution());
    imap[inst.id()] = inst;
    m->loadInstitutions(imap);
    QCOMPARE(m->d_func()->m_institutionList.values(), imap.values());
    QCOMPARE(m->d_func()->m_institutionList.keys(), imap.keys());
    QCOMPARE(m->d_func()->m_nextInstitutionID, 28ul);

    // payee loader
    QMap<QString, MyMoneyPayee> pmap;
    MyMoneyPayee p("P1234", MyMoneyPayee());
    pmap[p.id()] = p;
    m->loadPayees(pmap);
    QCOMPARE(m->d_func()->m_payeeList.values(), pmap.values());
    QCOMPARE(m->d_func()->m_payeeList.keys(), pmap.keys());
    QCOMPARE(m->d_func()->m_nextPayeeID, 1234ul);

    // tag loader
    QMap<QString, MyMoneyTag> tamap;
    MyMoneyTag ta("G1234", MyMoneyTag());
    tamap[ta.id()] = ta;
    m->loadTags(tamap);
    QCOMPARE(m->d_func()->m_tagList.values(), tamap.values());
    QCOMPARE(m->d_func()->m_tagList.keys(), tamap.keys());
    QCOMPARE(m->d_func()->m_nextTagID, 1234ul);

    // security loader
    QMap<QString, MyMoneySecurity> smap;
    MyMoneySecurity s("E54321", MyMoneySecurity());
    smap[s.id()] = s;
    m->loadSecurities(smap);
    QCOMPARE(m->d_func()->m_securitiesList.values(), smap.values());
    QCOMPARE(m->d_func()->m_securitiesList.keys(), smap.keys());
    QCOMPARE(m->d_func()->m_nextSecurityID, 54321ul);

    // schedule loader
    QMap<QString, MyMoneySchedule> schmap;
    MyMoneySchedule sch("SCH6789", MyMoneySchedule());
    schmap[sch.id()] = sch;
    m->loadSchedules(schmap);
    QCOMPARE(m->d_func()->m_scheduleList.values(), schmap.values());
    QCOMPARE(m->d_func()->m_scheduleList.keys(), schmap.keys());
    QCOMPARE(m->d_func()->m_nextScheduleID, 6789ul);

    // report loader
    QMap<QString, MyMoneyReport> rmap;
    MyMoneyReport r("R1298", MyMoneyReport());
    rmap[r.id()] = r;
    m->loadReports(rmap);
    QCOMPARE(m->d_func()->m_reportList.values(), rmap.values());
    QCOMPARE(m->d_func()->m_reportList.keys(), rmap.keys());
    QCOMPARE(m->d_func()->m_nextReportID, 1298ul);

    // budget loader
    QMap<QString, MyMoneyBudget> bmap;
    MyMoneyBudget b("B89765", MyMoneyBudget());
    bmap[b.id()] = b;
    m->loadBudgets(bmap);
    QCOMPARE(m->d_func()->m_budgetList.values(), bmap.values());
    QCOMPARE(m->d_func()->m_budgetList.keys(), bmap.keys());
    QCOMPARE(m->d_func()->m_nextBudgetID, 89765ul);

    // restart a transaction so that teardown() is happy
    m->startTransaction();
}

void MyMoneyStorageMgrTest::testAddOnlineJob()
{
    // Add a onlineJob
    onlineJob job(new dummyTask());

    m->addOnlineJob(job);
    QCOMPARE(job.id(), QString("O000001"));

    m->commitTransaction();
    m->startTransaction();

    QCOMPARE(m->d_func()->m_nextOnlineJobID, 1ul);
    QCOMPARE(m->dirty(), true);
    QCOMPARE(m->d_func()->m_onlineJobList.count(), 1);
    QVERIFY(! m->d_func()->m_onlineJobList["O000001"].isNull());

}
