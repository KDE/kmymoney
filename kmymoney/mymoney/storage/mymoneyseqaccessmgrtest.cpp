/***************************************************************************
                          mymoneyseqaccessmgrtest.cpp
                          -------------------
    copyright            : (C) 2002 by Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "mymoneyseqaccessmgrtest.h"
#include <iostream>
#include <QList>
#include <QtTest/QtTest>

#include "autotest.h"

#include "onlinetasks/dummy/tasks/dummytask.h"

QTEST_MAIN(MyMoneySeqAccessMgrTest)

void MyMoneySeqAccessMgrTest::init()
{
  m = new MyMoneySeqAccessMgr;
  MyMoneyFile* file = MyMoneyFile::instance();
  file->attachStorage(m);
  m->startTransaction();
}

void MyMoneySeqAccessMgrTest::cleanup()
{
  m->commitTransaction();
  MyMoneyFile* file = MyMoneyFile::instance();
  file->detachStorage(m);
  delete m;
}

void MyMoneySeqAccessMgrTest::testEmptyConstructor()
{
  MyMoneyPayee user = m->user();

  QVERIFY(user.name().isEmpty());
  QVERIFY(user.address().isEmpty());
  QVERIFY(user.city().isEmpty());
  QVERIFY(user.state().isEmpty());
  QVERIFY(user.postcode().isEmpty());
  QVERIFY(user.telephone().isEmpty());
  QVERIFY(user.email().isEmpty());
  QVERIFY(m->m_nextInstitutionID == 0);
  QVERIFY(m->m_nextAccountID == 0);
  QVERIFY(m->m_nextTransactionID == 0);
  QVERIFY(m->m_nextPayeeID == 0);
  QVERIFY(m->m_nextScheduleID == 0);
  QVERIFY(m->m_nextReportID == 0);
  QVERIFY(m->m_institutionList.count() == 0);
  QVERIFY(m->m_accountList.count() == 5);
  QVERIFY(m->m_transactionList.count() == 0);
  QVERIFY(m->m_transactionKeys.count() == 0);
  QVERIFY(m->m_payeeList.count() == 0);
  QVERIFY(m->m_tagList.count() == 0);
  QVERIFY(m->m_scheduleList.count() == 0);

  QVERIFY(m->m_dirty == false);
  QVERIFY(m->m_creationDate == QDate::currentDate());

  QVERIFY(m->liability().name() == "Liability");
  QVERIFY(m->asset().name() == "Asset");
  QVERIFY(m->expense().name() == "Expense");
  QVERIFY(m->income().name() == "Income");
  QVERIFY(m->equity().name() == "Equity");
}

void MyMoneySeqAccessMgrTest::testSetFunctions()
{
  MyMoneyPayee user = m->user();

  m->m_dirty = false;
  user.setName("Name");
  m->setUser(user);
  m->commitTransaction();
  m->startTransaction();
  QVERIFY(m->dirty() == true);
  m->m_dirty = false;
  user.setAddress("Street");
  m->setUser(user);
  m->commitTransaction();
  m->startTransaction();
  QVERIFY(m->dirty() == true);
  m->m_dirty = false;
  user.setCity("Town");
  m->setUser(user);
  m->commitTransaction();
  m->startTransaction();
  QVERIFY(m->dirty() == true);
  m->m_dirty = false;
  user.setState("County");
  m->setUser(user);
  m->commitTransaction();
  m->startTransaction();
  QVERIFY(m->dirty() == true);
  m->m_dirty = false;
  user.setPostcode("Postcode");
  m->setUser(user);
  m->commitTransaction();
  m->startTransaction();
  QVERIFY(m->dirty() == true);
  m->m_dirty = false;
  user.setTelephone("Telephone");
  m->setUser(user);
  m->commitTransaction();
  m->startTransaction();
  QVERIFY(m->dirty() == true);
  m->m_dirty = false;
  user.setEmail("Email");
  m->setUser(user);
  m->commitTransaction();
  m->startTransaction();
  QVERIFY(m->dirty() == true);
  m->m_dirty = false;
  m->setValue("key", "value");
  m->commitTransaction();
  m->startTransaction();
  QVERIFY(m->dirty() == true);

  user = m->user();
  QVERIFY(user.name() == "Name");
  QVERIFY(user.address() == "Street");
  QVERIFY(user.city() == "Town");
  QVERIFY(user.state() == "County");
  QVERIFY(user.postcode() == "Postcode");
  QVERIFY(user.telephone() == "Telephone");
  QVERIFY(user.email() == "Email");
  QVERIFY(m->value("key") == "value");

  m->m_dirty = false;
  m->deletePair("key");
  m->commitTransaction();
  m->startTransaction();
  QVERIFY(m->dirty() == true);
}

void MyMoneySeqAccessMgrTest::testSupportFunctions()
{
  QVERIFY(m->nextInstitutionID() == "I000001");
  QVERIFY(m->m_nextInstitutionID == 1);
  QVERIFY(m->nextAccountID() == "A000001");
  QVERIFY(m->m_nextAccountID == 1);
  QVERIFY(m->nextTransactionID() == "T000000000000000001");
  QVERIFY(m->m_nextTransactionID == 1);
  QVERIFY(m->nextPayeeID() == "P000001");
  QVERIFY(m->m_nextPayeeID == 1);
  QVERIFY(m->nextTagID() == "G000001");
  QVERIFY(m->m_nextTagID == 1);
  QVERIFY(m->nextScheduleID() == "SCH000001");
  QVERIFY(m->m_nextScheduleID == 1);
  QVERIFY(m->nextReportID() == "R000001");
  QVERIFY(m->m_nextReportID == 1);
  QVERIFY(m->nextOnlineJobID() == "O000001");
  QVERIFY(m->m_nextOnlineJobID == 1);
}

void MyMoneySeqAccessMgrTest::testIsStandardAccount()
{
  QVERIFY(m->isStandardAccount(STD_ACC_LIABILITY) == true);
  QVERIFY(m->isStandardAccount(STD_ACC_ASSET) == true);
  QVERIFY(m->isStandardAccount(STD_ACC_EXPENSE) == true);
  QVERIFY(m->isStandardAccount(STD_ACC_INCOME) == true);
  QVERIFY(m->isStandardAccount("A0002") == false);
}

void MyMoneySeqAccessMgrTest::testNewAccount()
{
  MyMoneyAccount a;

  a.setName("AccountName");
  a.setNumber("AccountNumber");

  m->addAccount(a);
  m->commitTransaction();
  m->startTransaction();

  QVERIFY(m->m_nextAccountID == 1);
  QVERIFY(m->dirty() == true);
  QVERIFY(m->m_accountList.count() == 6);
  QVERIFY(m->m_accountList["A000001"].name() == "AccountName");
}

void MyMoneySeqAccessMgrTest::testAccount()
{
  testNewAccount();
  m->m_dirty = false;

  MyMoneyAccount a;

  // make sure that an invalid ID causes an exception
  try {
    a = m->account("Unknown ID");
    QFAIL("Exception expected");
  } catch (const MyMoneyException &) {
  }
  m->commitTransaction();
  m->startTransaction();
  QVERIFY(m->dirty() == false);

  // now make sure, that a real ID works
  try {
    a = m->account("A000001");
    m->commitTransaction();
    m->startTransaction();
    QVERIFY(a.name() == "AccountName");
    QVERIFY(a.id() == "A000001");
    QVERIFY(m->dirty() == true);
  } catch (const MyMoneyException &) {
    QFAIL("Unexpected exception");
  }
}

void MyMoneySeqAccessMgrTest::testAddNewAccount()
{
  testNewAccount();

  MyMoneyAccount a, b;
  b.setName("Account2");
  b.setNumber("Acc2");
  m->addAccount(b);
  m->commitTransaction();
  m->startTransaction();

  m->m_dirty = false;

  QVERIFY(m->m_nextAccountID == 2);
  QVERIFY(m->m_accountList.count() == 7);

  // try to add account to undefined account
  try {
    MyMoneyAccount c("UnknownID", b);
    m->addAccount(c, a);
    QFAIL("Exception expected");
  } catch (const MyMoneyException &) {
  }
  m->commitTransaction();
  m->startTransaction();

  QVERIFY(m->dirty() == false);
  // now try to add account 1 as sub-account to account 2
  a = m->account("A000001");
  try {
    QVERIFY(m->m_accountList[STD_ACC_ASSET].accountList().count() == 0);
    m->addAccount(b, a);
    m->commitTransaction();
    m->startTransaction();
    QVERIFY(m->m_accountList["A000002"].accountList()[0] == "A000001");
    QVERIFY(m->m_accountList["A000002"].accountList().count() == 1);
    QVERIFY(m->m_accountList[STD_ACC_ASSET].accountList().count() == 0);
    QVERIFY(m->dirty() == true);
  } catch (const MyMoneyException &) {
    QFAIL("Unexpected exception");
  }
}

void MyMoneySeqAccessMgrTest::testAddInstitution()
{
  MyMoneyInstitution i;

  i.setName("Inst Name");

  m->addInstitution(i);
  QVERIFY(m->m_institutionList.count() == 1);
  QVERIFY(m->m_nextInstitutionID == 1);
  QVERIFY(m->m_institutionList["I000001"].name() == "Inst Name");
}

void MyMoneySeqAccessMgrTest::testInstitution()
{
  testAddInstitution();
  MyMoneyInstitution i;

  m->m_dirty = false;

  // try to find unknown institution
  try {
    i = m->institution("Unknown ID");
    QFAIL("Exception expected");
  } catch (const MyMoneyException &) {
  }

  QVERIFY(m->dirty() == false);

  // now try to find real institution
  try {
    i = m->institution("I000001");
    QVERIFY(i.name() == "Inst Name");
    QVERIFY(m->dirty() == false);
  } catch (const MyMoneyException &) {
    QFAIL("Unexpected exception");
  }
}

void MyMoneySeqAccessMgrTest::testAccount2Institution()
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

  m->m_dirty = false;

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

  QVERIFY(m->dirty() == false);
  // now try to do it with a real institution
  try {
    QVERIFY(i.accountList().count() == 0);
    a.setInstitutionId(i.id());
    m->modifyAccount(a);
    m->commitTransaction();
    m->startTransaction();
    QVERIFY(m->dirty() == true);
    QVERIFY(a.institutionId() == i.id());
    b = m->account("A000001");
    QVERIFY(b.institutionId() == i.id());
    QVERIFY(i.accountList().count() == 0);
  } catch (const MyMoneyException &) {
    QFAIL("Unexpected exception");
  }
}

void MyMoneySeqAccessMgrTest::testModifyAccount()
{
  testAccount2Institution();

  // test the OK case
  MyMoneyAccount a = m->account("A000001");
  a.setName("New account name");
  m->m_dirty = false;
  try {
    m->modifyAccount(a);
    m->commitTransaction();
    m->startTransaction();
    MyMoneyAccount b = m->account("A000001");
    QVERIFY(b.parentAccountId() == a.parentAccountId());
    QVERIFY(b.name() == "New account name");
    QVERIFY(m->dirty() == true);
  } catch (const MyMoneyException &) {
    QFAIL("Unexpected exception");
  }

  // modify institution to unknown id
  MyMoneyAccount c("Unknown ID", a);
  m->m_dirty = false;
  try {
    m->modifyAccount(c);
    QFAIL("Exception expected");
  } catch (const MyMoneyException &) {
  }

  // use different account type
  MyMoneyAccount d;
  d.setAccountType(MyMoneyAccount::CreditCard);
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

void MyMoneySeqAccessMgrTest::testModifyInstitution()
{
  testAddInstitution();
  MyMoneyInstitution i = m->institution("I000001");

  m->m_dirty = false;
  i.setName("New inst name");
  try {
    m->modifyInstitution(i);
    m->commitTransaction();
    m->startTransaction();
    QVERIFY(m->dirty() == true);
    i = m->institution("I000001");
    QVERIFY(i.name() == "New inst name");

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

void MyMoneySeqAccessMgrTest::testReparentAccount()
{
  // this one adds some accounts to the database
  MyMoneyAccount ex1;
  ex1.setAccountType(MyMoneyAccount::Expense);
  MyMoneyAccount ex2;
  ex2.setAccountType(MyMoneyAccount::Expense);
  MyMoneyAccount ex3;
  ex3.setAccountType(MyMoneyAccount::Expense);
  MyMoneyAccount ex4;
  ex4.setAccountType(MyMoneyAccount::Expense);
  MyMoneyAccount in;
  in.setAccountType(MyMoneyAccount::Income);
  MyMoneyAccount ch;
  ch.setAccountType(MyMoneyAccount::Checkings);

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

    QVERIFY(ex1.id() == "A000001");
    QVERIFY(ex2.id() == "A000002");
    QVERIFY(ex3.id() == "A000003");
    QVERIFY(ex4.id() == "A000004");
    QVERIFY(in.id() == "A000005");
    QVERIFY(ch.id() == "A000006");

    MyMoneyAccount parent = m->expense();

    m->addAccount(parent, ex1);
    m->addAccount(ex1, ex2);
    m->addAccount(parent, ex3);
    m->addAccount(parent, ex4);

    parent = m->income();
    m->addAccount(parent, in);

    parent = m->asset();
    m->addAccount(parent, ch);

    QVERIFY(m->expense().accountCount() == 3);
    QVERIFY(m->account(ex1.id()).accountCount() == 1);
    QVERIFY(ex3.parentAccountId() == STD_ACC_EXPENSE);

    m->reparentAccount(ex3, ex1);
    QVERIFY(m->expense().accountCount() == 2);
    QVERIFY(m->account(ex1.id()).accountCount() == 2);
    QVERIFY(ex3.parentAccountId() == ex1.id());
  } catch (const MyMoneyException &e) {
    std::cout << std::endl << qPrintable(e.what()) << std::endl;
    QFAIL("Unexpected exception");
  }
}

void MyMoneySeqAccessMgrTest::testAddTransactions()
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

    s.setId(QString()); // enable re-usage of split variable
    s.setAccountId("A000005"); // Salary
    s.setShares(MyMoneyMoney(-100000, 100));
    s.setValue(MyMoneyMoney(-100000, 100));
    QVERIFY(s.id().isEmpty());
    t1.addSplit(s);

    t1.setPostDate(QDate(2002, 5, 10));
  } catch (const MyMoneyException &e) {
    unexpectedException(e);
  }

  m->m_dirty = false;
  try {
    m->addTransaction(t1);
    m->commitTransaction();
    m->startTransaction();
    QVERIFY(m->dirty() == true);
    QVERIFY(t1.id() == "T000000000000000001");
    QVERIFY(t1.splitCount() == 2);
    QVERIFY(m->transactionCount() == 1);
  } catch (const MyMoneyException &e) {
    unexpectedException(e);
  }

  try {
    // I spent some money, not so great
    s.setId(QString()); // enable re-usage of split variable
    s.setAccountId("A000004"); // Grosseries
    s.setShares(MyMoneyMoney(10000, 100));
    s.setValue(MyMoneyMoney(10000, 100));
    QVERIFY(s.id().isEmpty());
    t2.addSplit(s);

    s.setId(QString()); // enable re-usage of split variable
    s.setAccountId("A000002"); // 16% sales tax
    s.setShares(MyMoneyMoney(1200, 100));
    s.setValue(MyMoneyMoney(1200, 100));
    QVERIFY(s.id().isEmpty());
    t2.addSplit(s);

    s.setId(QString()); // enable re-usage of split variable
    s.setAccountId("A000003"); // 7% sales tax
    s.setShares(MyMoneyMoney(400, 100));
    s.setValue(MyMoneyMoney(400, 100));
    QVERIFY(s.id().isEmpty());
    t2.addSplit(s);

    s.setId(QString()); // enable re-usage of split variable
    s.setAccountId("A000006"); // Checkings account
    s.setShares(MyMoneyMoney(-11600, 100));
    s.setValue(MyMoneyMoney(-11600, 100));
    QVERIFY(s.id().isEmpty());
    t2.addSplit(s);

    t2.setPostDate(QDate(2002, 5, 9));
  } catch (const MyMoneyException &e) {
    unexpectedException(e);
  }
  m->m_dirty = false;
  try {
    m->addTransaction(t2);
    m->commitTransaction();
    m->startTransaction();
    QVERIFY(m->dirty() == true);
    QVERIFY(t2.id() == "T000000000000000002");
    QVERIFY(t2.splitCount() == 4);
    QVERIFY(m->transactionCount() == 2);

    QMap<QString, QString>::ConstIterator it_k;
    QMap<QString, MyMoneyTransaction>::ConstIterator it_t;
    it_k = m->m_transactionKeys.begin();
    it_t = m->m_transactionList.begin();

    QVERIFY((*it_k) == "2002-05-10-T000000000000000001");
    QVERIFY((*it_t).id() == "T000000000000000002");
    ++it_k;
    ++it_t;
    QVERIFY((*it_k) == "2002-05-09-T000000000000000002");
    QVERIFY((*it_t).id() == "T000000000000000001");
    ++it_k;
    ++it_t;
    QVERIFY(it_k == m->m_transactionKeys.end());
    QVERIFY(it_t == m->m_transactionList.end());

    ch = m->account("A000006");

    // check that the account's transaction list is updated
    QList<MyMoneyTransaction> list;
    MyMoneyTransactionFilter filter("A000006");
    list = m->transactionList(filter);
    QVERIFY(list.size() == 2);

    QList<MyMoneyTransaction>::ConstIterator it;
    it = list.constBegin();
    QVERIFY((*it).id() == "T000000000000000002");
    ++it;
    QVERIFY((*it).id() == "T000000000000000001");
    ++it;
    QVERIFY(it == list.constEnd());
  } catch (const MyMoneyException &e) {
    unexpectedException(e);
  }
}

void MyMoneySeqAccessMgrTest::testTransactionCount()
{
  testAddTransactions();
  QVERIFY(m->transactionCount("A000001") == 0);
  QVERIFY(m->transactionCount("A000002") == 1);
  QVERIFY(m->transactionCount("A000003") == 1);
  QVERIFY(m->transactionCount("A000004") == 1);
  QVERIFY(m->transactionCount("A000005") == 1);
  QVERIFY(m->transactionCount("A000006") == 2);
}

void MyMoneySeqAccessMgrTest::testBalance()
{
  testAddTransactions();

  QVERIFY(m->balance("A000001").isZero());
  QVERIFY(m->balance("A000002") == MyMoneyMoney(1200, 100));
  QVERIFY(m->balance("A000003") == MyMoneyMoney(400, 100));
  QVERIFY(m->totalBalance("A000001") == MyMoneyMoney(1600, 100));
  QVERIFY(m->balance("A000006", QDate(2002, 5, 9)) == MyMoneyMoney(-11600, 100));
  QVERIFY(m->balance("A000005", QDate(2002, 5, 10)) == MyMoneyMoney(-100000, 100));
  QVERIFY(m->balance("A000006", QDate(2002, 5, 10)) == MyMoneyMoney(88400, 100));
}

void MyMoneySeqAccessMgrTest::testModifyTransaction()
{
  testAddTransactions();

  MyMoneyTransaction t = m->transaction("T000000000000000002");
  MyMoneySplit s;
  MyMoneyAccount ch;

  // just modify simple stuff (splits)
  QVERIFY(t.splitCount() == 4);

  s = t.splits()[0];
  s.setShares(MyMoneyMoney(11000, 100));
  s.setValue(MyMoneyMoney(11000, 100));
  t.modifySplit(s);

  QVERIFY(t.splitCount() == 4);
  s = t.splits()[3];
  s.setShares(MyMoneyMoney(-12600, 100));
  s.setValue(MyMoneyMoney(-12600, 100));
  t.modifySplit(s);

  try {
    QVERIFY(m->balance("A000004") == MyMoneyMoney(10000, 100));
    QVERIFY(m->balance("A000006") == MyMoneyMoney(100000 - 11600, 100));
    QVERIFY(m->totalBalance("A000001") == MyMoneyMoney(1600, 100));
    m->modifyTransaction(t);
    QVERIFY(m->balance("A000004") == MyMoneyMoney(11000, 100));
    QVERIFY(m->balance("A000006") == MyMoneyMoney(100000 - 12600, 100));
    QVERIFY(m->totalBalance("A000001") == MyMoneyMoney(1600, 100));
  } catch (const MyMoneyException &) {
    QFAIL("Unexpected exception");
  }

  // now modify the date
  t.setPostDate(QDate(2002, 5, 11));
  try {
    m->modifyTransaction(t);
    QVERIFY(m->balance("A000004") == MyMoneyMoney(11000, 100));
    QVERIFY(m->balance("A000006") == MyMoneyMoney(100000 - 12600, 100));
    QVERIFY(m->totalBalance("A000001") == MyMoneyMoney(1600, 100));

    QMap<QString, QString>::ConstIterator it_k;
    QMap<QString, MyMoneyTransaction>::ConstIterator it_t;
    it_k = m->m_transactionKeys.begin();
    it_t = m->m_transactionList.begin();
    QVERIFY((*it_k) == "2002-05-10-T000000000000000001");
    QVERIFY((*it_t).id() == "T000000000000000001");
    ++it_k;
    ++it_t;
    QVERIFY((*it_k) == "2002-05-11-T000000000000000002");
    QVERIFY((*it_t).id() == "T000000000000000002");
    ++it_k;
    ++it_t;
    QVERIFY(it_k == m->m_transactionKeys.end());
    QVERIFY(it_t == m->m_transactionList.end());

    ch = m->account("A000006");

    // check that the account's transaction list is updated
    QList<MyMoneyTransaction> list;
    MyMoneyTransactionFilter filter("A000006");
    list = m->transactionList(filter);
    QVERIFY(list.size() == 2);

    QList<MyMoneyTransaction>::ConstIterator it;
    it = list.constBegin();
    QVERIFY((*it).id() == "T000000000000000001");
    ++it;
    QVERIFY((*it).id() == "T000000000000000002");
    ++it;
    QVERIFY(it == list.constEnd());
  } catch (const MyMoneyException &) {
    QFAIL("Unexpected exception");
  }
}


void MyMoneySeqAccessMgrTest::testRemoveUnusedAccount()
{
  testAccount2Institution();

  MyMoneyAccount a = m->account("A000001");
  MyMoneyInstitution i = m->institution("I000001");

  m->m_dirty = false;
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
    QVERIFY(i.accountCount() == 0);
    QVERIFY(m->accountCount() == 7);

    a.setInstitutionId(QString());
    m->modifyAccount(a);
    m->removeAccount(a);
    m->commitTransaction();
    m->startTransaction();
    QVERIFY(m->accountCount() == 6);
    QVERIFY(m->dirty() == true);
    i = m->institution("I000001");
    QVERIFY(i.accountCount() == 0);
  } catch (const MyMoneyException &) {
    QFAIL("Unexpected exception");
  }
}

void MyMoneySeqAccessMgrTest::testRemoveUsedAccount()
{
  testAddTransactions();

  MyMoneyAccount a = m->account("A000006");

  try {
    m->removeAccount(a);
    QFAIL("Exception expected");
  } catch (const MyMoneyException &) {
  }
}

void MyMoneySeqAccessMgrTest::testRemoveInstitution()
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
    QVERIFY(i.accountCount() == 0);
  } catch (const MyMoneyException &) {
    QFAIL("Unexpected exception");
  }

  m->m_dirty = false;
  // now remove the institution and see if the account survived ;-)
  try {
    m->removeInstitution(i);
    a.setInstitutionId(QString());
    m->modifyAccount(a);
    m->commitTransaction();
    m->startTransaction();
    a = m->account("A000006");
    QVERIFY(m->dirty() == true);
    QVERIFY(a.institutionId().isEmpty());
    QVERIFY(m->institutionCount() == 0);
  } catch (const MyMoneyException &) {
    QFAIL("Unexpected exception");
  }
}

void MyMoneySeqAccessMgrTest::testRemoveTransaction()
{
  testAddTransactions();

  MyMoneyTransaction t = m->transaction("T000000000000000002");

  m->m_dirty = false;
  try {
    m->removeTransaction(t);
    m->commitTransaction();
    m->startTransaction();
    QVERIFY(m->dirty() == true);
    QVERIFY(m->transactionCount() == 1);
    /* removed with MyMoneyAccount::Transaction
      QVERIFY(m->account("A000006").transactionCount() == 1);
    */
  } catch (const MyMoneyException &) {
    QFAIL("Unexpected exception");
  }
}

void MyMoneySeqAccessMgrTest::testTransactionList()
{
  testAddTransactions();

  QList<MyMoneyTransaction> list;
  MyMoneyTransactionFilter filter("A000006");
  list = m->transactionList(filter);
  QVERIFY(list.count() == 2);
  QVERIFY(list.at(0).id() == "T000000000000000002");
  QVERIFY(list.at(1).id() == "T000000000000000001");

  filter.clear();
  filter.addAccount(QString("A000003"));
  list = m->transactionList(filter);
  QVERIFY(list.count() == 1);
  QVERIFY(list.at(0).id() == "T000000000000000002");

  filter.clear();
  list = m->transactionList(filter);
  QVERIFY(list.count() == 2);
  QVERIFY(list.at(0).id() == "T000000000000000002");
  QVERIFY(list.at(1).id() == "T000000000000000001");
}

void MyMoneySeqAccessMgrTest::testAddPayee()
{
  MyMoneyPayee p;

  p.setName("THB");
  m->m_dirty = false;
  try {
    QVERIFY(m->m_nextPayeeID == 0);
    m->addPayee(p);
    m->commitTransaction();
    m->startTransaction();
    QVERIFY(m->dirty() == true);
    QVERIFY(m->m_nextPayeeID == 1);
  } catch (const MyMoneyException &) {
    QFAIL("Unexpected exception");
  }

}

void MyMoneySeqAccessMgrTest::testSetAccountName()
{
  try {
    m->setAccountName(STD_ACC_LIABILITY, "Verbindlichkeiten");
  } catch (const MyMoneyException &) {
    QFAIL("Unexpected exception");
  }
  try {
    m->setAccountName(STD_ACC_ASSET, "Verm�gen");
  } catch (const MyMoneyException &) {
    QFAIL("Unexpected exception");
  }
  try {
    m->setAccountName(STD_ACC_EXPENSE, "Ausgaben");
  } catch (const MyMoneyException &) {
    QFAIL("Unexpected exception");
  }
  try {
    m->setAccountName(STD_ACC_INCOME, "Einnahmen");
  } catch (const MyMoneyException &) {
    QFAIL("Unexpected exception");
  }

  QVERIFY(m->liability().name() == "Verbindlichkeiten");
  QVERIFY(m->asset().name() == "Verm�gen");
  QVERIFY(m->expense().name() == "Ausgaben");
  QVERIFY(m->income().name() == "Einnahmen");

  try {
    m->setAccountName("A000001", "New account name");
    QFAIL("Exception expected");
  } catch (const MyMoneyException &) {
  }
}

void MyMoneySeqAccessMgrTest::testModifyPayee()
{
  MyMoneyPayee p;

  testAddPayee();

  p = m->payee("P000001");
  p.setName("New name");
  m->m_dirty = false;
  try {
    m->modifyPayee(p);
    m->commitTransaction();
    m->startTransaction();
    p = m->payee("P000001");
    QVERIFY(p.name() == "New name");
    QVERIFY(m->dirty() == true);
  } catch (const MyMoneyException &) {
    QFAIL("Unexpected exception");
  }
}

void MyMoneySeqAccessMgrTest::testRemovePayee()
{
  testAddPayee();
  m->m_dirty = false;

  // check that we can remove an unreferenced payee
  MyMoneyPayee p = m->payee("P000001");
  try {
    QVERIFY(m->m_payeeList.count() == 1);
    m->removePayee(p);
    m->commitTransaction();
    m->startTransaction();
    QVERIFY(m->m_payeeList.count() == 0);
    QVERIFY(m->dirty() == true);
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

  m->m_nextPayeeID = 0;  // reset here, so that the
  // testAddPayee will not fail
  testAddPayee();

  // check that it works when the payee exists
  try {
    m->modifyTransaction(tr);
  } catch (const MyMoneyException &) {
    QFAIL("Unexpected exception");
  }

  m->m_dirty = false;

  // now check, that we cannot remove the payee
  try {
    m->removePayee(p);
    QFAIL("Expected exception");
  } catch (const MyMoneyException &) {
  }
  QVERIFY(m->m_payeeList.count() == 1);
}

void MyMoneySeqAccessMgrTest::testAddTag()
{
  MyMoneyTag ta;

  ta.setName("THB");
  m->m_dirty = false;
  try {
    QVERIFY(m->m_nextTagID == 0);
    m->addTag(ta);
    m->commitTransaction();
    m->startTransaction();
    QVERIFY(m->dirty() == true);
    QVERIFY(m->m_nextTagID == 1);
  } catch (const MyMoneyException &) {
    QFAIL("Unexpected exception");
  }
}

void MyMoneySeqAccessMgrTest::testModifyTag()
{
  MyMoneyTag ta;

  testAddTag();

  ta = m->tag("G000001");
  ta.setName("New name");
  m->m_dirty = false;
  try {
    m->modifyTag(ta);
    m->commitTransaction();
    m->startTransaction();
    ta = m->tag("G000001");
    QVERIFY(ta.name() == "New name");
    QVERIFY(m->dirty() == true);
  } catch (const MyMoneyException &) {
    QFAIL("Unexpected exception");
  }
}

void MyMoneySeqAccessMgrTest::testRemoveTag()
{
  testAddTag();
  m->m_dirty = false;

  // check that we can remove an unreferenced tag
  MyMoneyTag ta = m->tag("G000001");
  try {
    QVERIFY(m->m_tagList.count() == 1);
    m->removeTag(ta);
    m->commitTransaction();
    m->startTransaction();
    QVERIFY(m->m_tagList.count() == 0);
    QVERIFY(m->dirty() == true);
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

  m->m_nextTagID = 0;  // reset here, so that the
  // testAddTag will not fail
  testAddTag();

  // check that it works when the tag exists
  try {
    m->modifyTransaction(tr);
  } catch (const MyMoneyException &) {
    QFAIL("Unexpected exception");
  }

  m->m_dirty = false;

  // now check, that we cannot remove the tag
  try {
    m->removeTag(ta);
    QFAIL("Expected exception");
  } catch (const MyMoneyException &) {
  }
  QVERIFY(m->m_tagList.count() == 1);
}

void MyMoneySeqAccessMgrTest::testRemoveAccountFromTree()
{
  MyMoneyAccount a, b, c;
  a.setName("Acc A");
  b.setName("Acc B");
  c.setName("Acc C");

  // build a tree A -> B -> C, remove B and see if A -> C
  // remains in the storag manager

  try {
    m->addAccount(a);
    m->addAccount(b);
    m->addAccount(c);
    m->reparentAccount(b, a);
    m->reparentAccount(c, b);

    QVERIFY(a.accountList().count() == 1);
    QVERIFY(m->account(a.accountList()[0]).name() == "Acc B");

    QVERIFY(b.accountList().count() == 1);
    QVERIFY(m->account(b.accountList()[0]).name() == "Acc C");

    QVERIFY(c.accountList().count() == 0);

    m->removeAccount(b);

    // reload account info from titutionIDtorage
    a = m->account(a.id());
    c = m->account(c.id());

    try {
      b = m->account(b.id());
      QFAIL("Exception expected");
    } catch (const MyMoneyException &) {
    }
    QVERIFY(a.accountList().count() == 1);
    QVERIFY(m->account(a.accountList()[0]).name() == "Acc C");

    QVERIFY(c.accountList().count() == 0);

  } catch (const MyMoneyException &e) {
    unexpectedException(e);
  }
}

void MyMoneySeqAccessMgrTest::testPayeeName()
{
  testAddPayee();

  MyMoneyPayee p;
  QString name("THB");

  // OK case
  try {
    p = m->payeeByName(name);
    QVERIFY(p.name() == "THB");
    QVERIFY(p.id() == "P000001");
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

void MyMoneySeqAccessMgrTest::testTagName()
{
  testAddTag();

  MyMoneyTag ta;
  QString name("THB");

  // OK case
  try {
    ta = m->tagByName(name);
    QVERIFY(ta.name() == "THB");
    QVERIFY(ta.id() == "G000001");
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

void MyMoneySeqAccessMgrTest::testAssignment()
{
  testAddTransactions();

  MyMoneyPayee user;
  user.setName("Thomas");
  m->setUser(user);

  MyMoneySeqAccessMgr test = *m;
  testEquality(&test);
}

void MyMoneySeqAccessMgrTest::testEquality(const MyMoneySeqAccessMgr *t)
{
  QVERIFY(m->user().name() == t->user().name());
  QVERIFY(m->user().address() == t->user().address());
  QVERIFY(m->user().city() == t->user().city());
  QVERIFY(m->user().state() == t->user().state());
  QVERIFY(m->user().postcode() == t->user().postcode());
  QVERIFY(m->user().telephone() == t->user().telephone());
  QVERIFY(m->user().email() == t->user().email());
  QVERIFY(m->m_nextInstitutionID == t->m_nextInstitutionID);
  QVERIFY(m->m_nextAccountID == t->m_nextAccountID);
  QVERIFY(m->m_nextTransactionID == t->m_nextTransactionID);
  QVERIFY(m->m_nextPayeeID == t->m_nextPayeeID);
  QVERIFY(m->m_nextTagID == t->m_nextTagID);
  QVERIFY(m->m_nextScheduleID == t->m_nextScheduleID);
  QVERIFY(m->m_dirty == t->m_dirty);
  QVERIFY(m->m_creationDate == t->m_creationDate);
  QVERIFY(m->m_lastModificationDate == t->m_lastModificationDate);

  /*
   * make sure, that the keys and values are the same
   * on the left and the right side
   */
  QVERIFY(m->m_payeeList.keys() == t->m_payeeList.keys());
  QVERIFY(m->m_payeeList.values() == t->m_payeeList.values());
  QVERIFY(m->m_tagList.keys() == t->m_tagList.keys());
  QVERIFY(m->m_tagList.values() == t->m_tagList.values());
  QVERIFY(m->m_transactionKeys.keys() == t->m_transactionKeys.keys());
  QVERIFY(m->m_transactionKeys.values() == t->m_transactionKeys.values());
  QVERIFY(m->m_institutionList.keys() == t->m_institutionList.keys());
  QVERIFY(m->m_institutionList.values() == t->m_institutionList.values());
  QVERIFY(m->m_accountList.keys() == t->m_accountList.keys());
  QVERIFY(m->m_accountList.values() == t->m_accountList.values());
  QVERIFY(m->m_transactionList.keys() == t->m_transactionList.keys());
  QVERIFY(m->m_transactionList.values() == t->m_transactionList.values());

// QVERIFY(m->m_scheduleList.keys() == t->m_scheduleList.keys());
// QVERIFY(m->m_scheduleList.values() == t->m_scheduleList.values());
}

void MyMoneySeqAccessMgrTest::testDuplicate()
{
  const MyMoneySeqAccessMgr* t;

  testModifyTransaction();

  t = m->duplicate();
  testEquality(t);
  delete t;
}

void MyMoneySeqAccessMgrTest::testAddSchedule()
{
  /* Note addSchedule() now calls validate as it should
   * so we need an account id.  Later this will
   * be checked to make sure its a valid account id.  The
   * tests currently fail because no splits are defined
   * for the schedules transaction.
  */


  try {
    QVERIFY(m->m_scheduleList.count() == 0);
    MyMoneyTransaction t1;
    MyMoneySplit s1, s2;
    s1.setAccountId("A000001");
    t1.addSplit(s1);
    s2.setAccountId("A000002");
    t1.addSplit(s2);
    MyMoneySchedule schedule("Sched-Name",
                             MyMoneySchedule::TYPE_DEPOSIT,
                             MyMoneySchedule::OCCUR_DAILY, 1,
                             MyMoneySchedule::STYPE_MANUALDEPOSIT,
                             QDate(),
                             QDate(),
                             true,
                             false);
    t1.setPostDate(QDate(2003, 7, 10));
    schedule.setTransaction(t1);

    m->addSchedule(schedule);

    QVERIFY(m->m_scheduleList.count() == 1);
    QVERIFY(schedule.id() == "SCH000001");
    QVERIFY(m->m_scheduleList["SCH000001"].id() == "SCH000001");
  } catch (const MyMoneyException &) {
    QFAIL("Unexpected exception");
  }

  try {
    MyMoneySchedule schedule("Sched-Name",
                             MyMoneySchedule::TYPE_DEPOSIT,
                             MyMoneySchedule::OCCUR_DAILY, 1,
                             MyMoneySchedule::STYPE_MANUALDEPOSIT,
                             QDate(),
                             QDate(),
                             true,
                             false);
    m->addSchedule(schedule);
    QFAIL("Exception expected");
  } catch (const MyMoneyException &) {
  }
}

void MyMoneySeqAccessMgrTest::testSchedule()
{
  testAddSchedule();
  MyMoneySchedule sched;

  sched = m->schedule("SCH000001");
  QVERIFY(sched.name() == "Sched-Name");
  QVERIFY(sched.id() == "SCH000001");

  try {
    m->schedule("SCH000002");
    QFAIL("Exception expected");
  } catch (const MyMoneyException &) {
  }
}

void MyMoneySeqAccessMgrTest::testModifySchedule()
{
  testAddSchedule();
  MyMoneySchedule sched;

  sched = m->schedule("SCH000001");
  sched.setId("SCH000002");
  try {
    m->modifySchedule(sched);
    QFAIL("Exception expected");
  } catch (const MyMoneyException &) {
  }

  sched = m->schedule("SCH000001");
  sched.setName("New Sched-Name");
  try {
    m->modifySchedule(sched);
    QVERIFY(m->m_scheduleList.count() == 1);
    QVERIFY(m->m_scheduleList["SCH000001"].name() == "New Sched-Name");

  } catch (const MyMoneyException &) {
    QFAIL("Unexpected exception");
  }

}

void MyMoneySeqAccessMgrTest::testRemoveSchedule()
{
  testAddSchedule();
  m->commitTransaction();
  m->startTransaction();
  MyMoneySchedule sched;

  sched = m->schedule("SCH000001");
  sched.setId("SCH000002");
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
    QVERIFY(m->m_scheduleList.count() == 0);

  } catch (const MyMoneyException &) {
    m->rollbackTransaction();
    QFAIL("Unexpected exception");
  }
  m->startTransaction();
}

void MyMoneySeqAccessMgrTest::testScheduleList()
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
  MyMoneySchedule schedule1("Schedule 1",
                            MyMoneySchedule::TYPE_BILL,
                            MyMoneySchedule::OCCUR_ONCE, 1,
                            MyMoneySchedule::STYPE_DIRECTDEBIT,
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
  MyMoneySchedule schedule2("Schedule 2",
                            MyMoneySchedule::TYPE_DEPOSIT,
                            MyMoneySchedule::OCCUR_DAILY, 1,
                            MyMoneySchedule::STYPE_DIRECTDEPOSIT,
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
  MyMoneySchedule schedule3("Schedule 3",
                            MyMoneySchedule::TYPE_TRANSFER,
                            MyMoneySchedule::OCCUR_WEEKLY, 1,
                            MyMoneySchedule::STYPE_OTHER,
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
                            MyMoneySchedule::TYPE_BILL,
                            MyMoneySchedule::OCCUR_WEEKLY, 1,
                            MyMoneySchedule::STYPE_WRITECHEQUE,
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
    qDebug("Error: %s", qPrintable(e.what()));
    QFAIL("Unexpected exception");
  }

  QList<MyMoneySchedule> list;

  // no filter
  list = m->scheduleList();
  QVERIFY(list.count() == 4);

  // filter by type
  list = m->scheduleList("", MyMoneySchedule::TYPE_BILL);
  QVERIFY(list.count() == 2);
  QVERIFY(list[0].name() == "Schedule 1");
  QVERIFY(list[1].name() == "Schedule 4");

  // filter by occurrence
  list = m->scheduleList("", MyMoneySchedule::TYPE_ANY,
                         MyMoneySchedule::OCCUR_DAILY);
  QVERIFY(list.count() == 1);
  QVERIFY(list[0].name() == "Schedule 2");

  // filter by payment type
  list = m->scheduleList("", MyMoneySchedule::TYPE_ANY,
                         MyMoneySchedule::OCCUR_ANY,
                         MyMoneySchedule::STYPE_DIRECTDEPOSIT);
  QVERIFY(list.count() == 1);
  QVERIFY(list[0].name() == "Schedule 2");

  // filter by account
  list = m->scheduleList("A01");
  QVERIFY(list.count() == 0);
  list = m->scheduleList("A000001");
  QVERIFY(list.count() == 2);
  list = m->scheduleList("A000002");
  QVERIFY(list.count() == 1);

  // filter by start date
  list = m->scheduleList("", MyMoneySchedule::TYPE_ANY,
                         MyMoneySchedule::OCCUR_ANY,
                         MyMoneySchedule::STYPE_ANY,
                         notOverdue.addDays(31));
  QVERIFY(list.count() == 3);
  QVERIFY(list[0].name() == "Schedule 2");
  QVERIFY(list[1].name() == "Schedule 3");
  QVERIFY(list[2].name() == "Schedule 4");

  // filter by end date
  list = m->scheduleList("", MyMoneySchedule::TYPE_ANY,
                         MyMoneySchedule::OCCUR_ANY,
                         MyMoneySchedule::STYPE_ANY,
                         QDate(),
                         notOverdue.addDays(1));
  QVERIFY(list.count() == 3);
  QVERIFY(list[0].name() == "Schedule 1");
  QVERIFY(list[1].name() == "Schedule 2");
  QVERIFY(list[2].name() == "Schedule 4");

  // filter by start and end date
  list = m->scheduleList("", MyMoneySchedule::TYPE_ANY,
                         MyMoneySchedule::OCCUR_ANY,
                         MyMoneySchedule::STYPE_ANY,
                         notOverdue.addDays(-1),
                         notOverdue.addDays(1));
  QVERIFY(list.count() == 2);
  QVERIFY(list[0].name() == "Schedule 1");
  QVERIFY(list[1].name() == "Schedule 2");

  // filter by overdue status
  list = m->scheduleList("", MyMoneySchedule::TYPE_ANY,
                         MyMoneySchedule::OCCUR_ANY,
                         MyMoneySchedule::STYPE_ANY,
                         QDate(),
                         QDate(),
                         true);
  QVERIFY(list.count() == 1);
  QVERIFY(list[0].name() == "Schedule 4");
}

void MyMoneySeqAccessMgrTest::testAddCurrency()
{
  MyMoneySecurity curr("EUR", "Euro", "?", 100, 100);
  QVERIFY(m->m_currencyList.count() == 0);
  m->m_dirty = false;
  try {
    m->addCurrency(curr);
    m->commitTransaction();
    m->startTransaction();
    QVERIFY(m->m_currencyList.count() == 1);
    QVERIFY(m->m_currencyList["EUR"].name() == "Euro");
    QVERIFY(m->dirty() == true);
  } catch (const MyMoneyException &) {
    QFAIL("Unexpected exception");
  }

  m->m_dirty = false;
  try {
    m->addCurrency(curr);
    QFAIL("Expected exception missing");
  } catch (const MyMoneyException &) {
    m->commitTransaction();
    m->startTransaction();
    QVERIFY(m->dirty() == false);
  }
}

void MyMoneySeqAccessMgrTest::testModifyCurrency()
{
  MyMoneySecurity curr("EUR", "Euro", "?", 100, 100);
  testAddCurrency();
  m->m_dirty = false;
  curr.setName("EURO");
  try {
    m->modifyCurrency(curr);
    m->commitTransaction();
    m->startTransaction();
    QVERIFY(m->m_currencyList.count() == 1);
    QVERIFY(m->m_currencyList["EUR"].name() == "EURO");
    QVERIFY(m->dirty() == true);
  } catch (const MyMoneyException &) {
    QFAIL("Unexpected exception");
  }

  m->m_dirty = false;

  MyMoneySecurity unknownCurr("DEM", "Deutsche Mark", "DM", 100, 100);
  try {
    m->modifyCurrency(unknownCurr);
    QFAIL("Expected exception missing");
  } catch (const MyMoneyException &) {
    m->commitTransaction();
    m->startTransaction();
    QVERIFY(m->dirty() == false);
  }
}

void MyMoneySeqAccessMgrTest::testRemoveCurrency()
{
  MyMoneySecurity curr("EUR", "Euro", "?", 100, 100);
  testAddCurrency();
  m->m_dirty = false;
  try {
    m->removeCurrency(curr);
    m->commitTransaction();
    m->startTransaction();
    QVERIFY(m->m_currencyList.count() == 0);
    QVERIFY(m->dirty() == true);
  } catch (const MyMoneyException &) {
    QFAIL("Unexpected exception");
  }

  m->m_dirty = false;

  MyMoneySecurity unknownCurr("DEM", "Deutsche Mark", "DM", 100, 100);
  try {
    m->removeCurrency(unknownCurr);
    QFAIL("Expected exception missing");
  } catch (const MyMoneyException &) {
    m->commitTransaction();
    m->startTransaction();
    QVERIFY(m->dirty() == false);
  }
}

void MyMoneySeqAccessMgrTest::testCurrency()
{
  MyMoneySecurity curr("EUR", "Euro", "?", 100, 100);
  MyMoneySecurity newCurr;
  testAddCurrency();
  m->m_dirty = false;
  try {
    newCurr = m->currency("EUR");
    m->commitTransaction();
    m->startTransaction();
    QVERIFY(m->dirty() == false);
    QVERIFY(newCurr.id() == curr.id());
    QVERIFY(newCurr.name() == curr.name());
  } catch (const MyMoneyException &) {
    QFAIL("Unexpected exception");
  }

  try {
    m->currency("DEM");
    QFAIL("Expected exception missing");
  } catch (const MyMoneyException &) {
    m->commitTransaction();
    m->startTransaction();
    QVERIFY(m->dirty() == false);
  }
}

void MyMoneySeqAccessMgrTest::testCurrencyList()
{
  QVERIFY(m->currencyList().count() == 0);

  testAddCurrency();
  QVERIFY(m->currencyList().count() == 1);

  MyMoneySecurity unknownCurr("DEM", "Deutsche Mark", "DM", 100, 100);
  try {
    m->addCurrency(unknownCurr);
    m->m_dirty = false;
    QVERIFY(m->m_currencyList.count() == 2);
    QVERIFY(m->currencyList().count() == 2);
    QVERIFY(m->dirty() == false);
  } catch (const MyMoneyException &) {
    QFAIL("Unexpected exception");
  }
}

void MyMoneySeqAccessMgrTest::testAccountList()
{
  QList<MyMoneyAccount> accounts;
  m->accountList(accounts);
  QVERIFY(accounts.count() == 0);
  testAddNewAccount();
  accounts.clear();
  m->accountList(accounts);
  QVERIFY(accounts.count() == 2);

  MyMoneyAccount a = m->account("A000001");
  MyMoneyAccount b = m->account("A000002");
  m->reparentAccount(b, a);
  accounts.clear();
  m->accountList(accounts);
  QVERIFY(accounts.count() == 2);
}

void MyMoneySeqAccessMgrTest::testLoaderFunctions()
{
  // we don't need the transaction started by setup() here
  m->rollbackTransaction();

  // account loader
  QMap<QString, MyMoneyAccount> amap;
  MyMoneyAccount acc("A0000176", MyMoneyAccount());
  amap[acc.id()] = acc;
  m->loadAccounts(amap);
  QVERIFY(m->m_accountList.values() == amap.values());
  QVERIFY(m->m_accountList.keys() == amap.keys());
  QVERIFY(m->m_nextAccountID == 176);

  // transaction loader
  QMap<QString, MyMoneyTransaction> tmap;
  MyMoneyTransaction t("T000000108", MyMoneyTransaction());
  tmap[t.id()] = t;
  m->loadTransactions(tmap);
  QVERIFY(m->m_transactionList.values() == tmap.values());
  QVERIFY(m->m_transactionList.keys() == tmap.keys());
  QVERIFY(m->m_nextTransactionID == 108);

  // institution loader
  QMap<QString, MyMoneyInstitution> imap;
  MyMoneyInstitution inst("I000028", MyMoneyInstitution());
  imap[inst.id()] = inst;
  m->loadInstitutions(imap);
  QVERIFY(m->m_institutionList.values() == imap.values());
  QVERIFY(m->m_institutionList.keys() == imap.keys());
  QVERIFY(m->m_nextInstitutionID == 28);

  // payee loader
  QMap<QString, MyMoneyPayee> pmap;
  MyMoneyPayee p("P1234", MyMoneyPayee());
  pmap[p.id()] = p;
  m->loadPayees(pmap);
  QVERIFY(m->m_payeeList.values() == pmap.values());
  QVERIFY(m->m_payeeList.keys() == pmap.keys());
  QVERIFY(m->m_nextPayeeID == 1234);

  // tag loader
  QMap<QString, MyMoneyTag> tamap;
  MyMoneyTag ta("G1234", MyMoneyTag());
  tamap[ta.id()] = ta;
  m->loadTags(tamap);
  QVERIFY(m->m_tagList.values() == tamap.values());
  QVERIFY(m->m_tagList.keys() == tamap.keys());
  QVERIFY(m->m_nextTagID == 1234);

  // security loader
  QMap<QString, MyMoneySecurity> smap;
  MyMoneySecurity s("S54321", MyMoneySecurity());
  smap[s.id()] = s;
  m->loadSecurities(smap);
  QVERIFY(m->m_securitiesList.values() == smap.values());
  QVERIFY(m->m_securitiesList.keys() == smap.keys());
  QVERIFY(m->m_nextSecurityID == 54321);

  // schedule loader
  QMap<QString, MyMoneySchedule> schmap;
  MyMoneySchedule sch("SCH6789", MyMoneySchedule());
  schmap[sch.id()] = sch;
  m->loadSchedules(schmap);
  QVERIFY(m->m_scheduleList.values() == schmap.values());
  QVERIFY(m->m_scheduleList.keys() == schmap.keys());
  QVERIFY(m->m_nextScheduleID == 6789);

  // report loader
  QMap<QString, MyMoneyReport> rmap;
  MyMoneyReport r("R1298", MyMoneyReport());
  rmap[r.id()] = r;
  m->loadReports(rmap);
  QVERIFY(m->m_reportList.values() == rmap.values());
  QVERIFY(m->m_reportList.keys() == rmap.keys());
  QVERIFY(m->m_nextReportID == 1298);

  // budget loader
  QMap<QString, MyMoneyBudget> bmap;
  MyMoneyBudget b("B89765", MyMoneyBudget());
  bmap[b.id()] = b;
  m->loadBudgets(bmap);
  QVERIFY(m->m_budgetList.values() == bmap.values());
  QVERIFY(m->m_budgetList.keys() == bmap.keys());
  QVERIFY(m->m_nextBudgetID == 89765);

  // restart a transaction so that teardown() is happy
  m->startTransaction();
}

void MyMoneySeqAccessMgrTest::testAddOnlineJob()
{
  // Add a onlineJob
  onlineJob job(new dummyTask());

  m->addOnlineJob( job );
  QCOMPARE( job.id(), QString("O000001"));

  m->commitTransaction();
  m->startTransaction();

  QVERIFY(m->m_nextOnlineJobID == 1);
  QVERIFY(m->dirty() == true);
  QVERIFY(m->m_onlineJobList.count() == 1);
  QVERIFY(! m->m_onlineJobList["O000001"].isNull());

}

#include "mymoneyseqaccessmgrtest.moc"
