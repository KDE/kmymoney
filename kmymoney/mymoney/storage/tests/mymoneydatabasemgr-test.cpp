/***************************************************************************
                          mymoneydatabasemgrtest.cpp
                          -------------------
    copyright            : (C) 2008 by Fernando Vilas
    email                : fvilas@iname.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "mymoneydatabasemgr-test.h"
#include <iostream>

#include <QtTest/QtTest>

#include "mymoneytestutils.h"
#include "mymoneyfile.h"
#include "mymoneyinstitution.h"
#include "mymoneyaccount.h"
#include "mymoneysecurity.h"
#include "mymoneytag.h"
#include "mymoneypayee.h"
#include "mymoneyschedule.h"
#include "mymoneyreport.h"
#include "mymoneysplit.h"
#include "mymoneytransaction.h"
#include "mymoneybudget.h"

#include "onlinetasks/dummy/tasks/dummytask.h"
#include "misc/platformtools.h"

#include "mymoneyenums.h"

using namespace eMyMoney;

QTEST_GUILESS_MAIN(MyMoneyDatabaseMgrTest)

MyMoneyDatabaseMgrTest::MyMoneyDatabaseMgrTest()
    : m_dbAttached(false),
    m_canOpen(true),
    m_haveEmptyDataBase(false),
    m_file(this),
    m_emptyFile(this)
{
  // Open and close the temp file so that it exists
  m_file.open();
  m_file.close();
  // The same with the empty db file
  m_emptyFile.open();
  m_emptyFile.close();

  testCaseTimer.start();
}

void MyMoneyDatabaseMgrTest::init()
{
  testStepTimer.start();
  m = new MyMoneyDatabaseMgr;
  // Create file and close it to release possible read-write locks
  m_file.open();
  m_file.close();
}

void MyMoneyDatabaseMgrTest::cleanup()
{
  if (m_canOpen) {
    // All transactions should have already been committed.
    //m->commitTransaction();
  }
  if (MyMoneyFile::instance()->storageAttached()) {
    MyMoneyFile::instance()->detachStorage(m);
    m_dbAttached = false;
  }
  delete m;
  qDebug() << "teststep" << testStepTimer.elapsed() << "msec, total" << testCaseTimer.elapsed() << "msec";
}

void MyMoneyDatabaseMgrTest::testEmptyConstructor()
{
  MyMoneyPayee user = m->user();

  QVERIFY(user.name().isEmpty());
  QVERIFY(user.address().isEmpty());
  QVERIFY(user.city().isEmpty());
  QVERIFY(user.state().isEmpty());
  QVERIFY(user.postcode().isEmpty());
  QVERIFY(user.telephone().isEmpty());
  QVERIFY(user.email().isEmpty());
  QVERIFY(m->nextInstitutionID().isEmpty());
  QVERIFY(m->nextAccountID().isEmpty());
  QVERIFY(m->nextTransactionID().isEmpty());
  QVERIFY(m->nextPayeeID().isEmpty());
  QVERIFY(m->nextScheduleID().isEmpty());
  QVERIFY(m->nextReportID().isEmpty());
  QVERIFY(m->nextOnlineJobID().isEmpty());
  QCOMPARE(m->institutionList().count(), 0);

  QList<MyMoneyAccount> accList;
  m->accountList(accList);
  QCOMPARE(accList.count(), 0);

  MyMoneyTransactionFilter f;
  QCOMPARE(m->transactionList(f).count(), 0);

  QCOMPARE(m->payeeList().count(), 0);
  QCOMPARE(m->tagList().count(), 0);
  QCOMPARE(m->scheduleList().count(), 0);

  QCOMPARE(m->m_creationDate, QDate::currentDate());
}

void MyMoneyDatabaseMgrTest::setupUrl(const QString& fname)
{
  QString m_userName = platformTools::osUsername();

  QString m_mode =
    //"QPSQL&mode=single";
    //"QMYSQL&mode=single";
    "QSQLITE&mode=single";

  m_url = QUrl(QString("sql://%1@localhost/%2?driver=%3").arg(m_userName, fname, m_mode));
}

void MyMoneyDatabaseMgrTest::copyDatabaseFile(QFile& src, QFile& dest)
{
  if (src.open(QIODevice::ReadOnly)) {
    if (dest.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
      dest.write(src.readAll());
      dest.close();
    }
    src.close();
  }
}

void MyMoneyDatabaseMgrTest::testBadConnections()
{
  // Check a connection that exists but has empty tables
  setupUrl(m_file.fileName());

  try {
    QExplicitlySharedDataPointer <MyMoneyStorageSql> sql = m->connectToDatabase(m_url);
    QVERIFY(sql);
    QEXPECT_FAIL("", "Will fix when correct behaviour in this case is clear.", Continue);
    QVERIFY(sql->open(m_url, QIODevice::ReadWrite) != 0);
  } catch (const MyMoneyException &e) {
    unexpectedException(e);
  }
}

void MyMoneyDatabaseMgrTest::testCreateDb()
{
  try {
    // Fetch the list of available drivers
    QStringList list = QSqlDatabase::drivers();
    QStringList::Iterator it = list.begin();

    if (it == list.end()) {
      m_canOpen = false;
    } else {
      setupUrl(m_file.fileName());
      QExplicitlySharedDataPointer <MyMoneyStorageSql> sql = m->connectToDatabase(m_url);
      QVERIFY(0 != sql);
      //qDebug("Database driver is %s", qPrintable(sql->driverName()));
      // Clear the database, so there is a fresh start on each run.
      if (0 == sql->open(m_url, QIODevice::WriteOnly, true)) {
        MyMoneyFile::instance()->attachStorage(m);
        QVERIFY(sql->writeFile());
        sql->close();
        copyDatabaseFile(m_file, m_emptyFile);
        m_haveEmptyDataBase = true;
      } else {
        m_canOpen = false;
      }
    }
  } catch (const MyMoneyException &e) {
    unexpectedException(e);
  }
}

void MyMoneyDatabaseMgrTest::testAttachDb()
{
  if (!m_dbAttached) {
    if (!m_haveEmptyDataBase) {
      testCreateDb();
    } else {
      // preload database file with empty set
      copyDatabaseFile(m_emptyFile, m_file);
    }

    if (m_canOpen) {
      try {
        MyMoneyFile::instance()->detachStorage();
        QExplicitlySharedDataPointer <MyMoneyStorageSql> sql = m->connectToDatabase(m_url);
        QVERIFY(sql);
        int openStatus = sql->open(m_url, QIODevice::ReadWrite);
        QCOMPARE(openStatus, 0);
        MyMoneyFile::instance()->attachStorage(m);
        m_dbAttached = true;
      } catch (const MyMoneyException &e) {
        unexpectedException(e);
      }
    }
  }
}

void MyMoneyDatabaseMgrTest::testDisconnection()
{
  testAttachDb();
  if (!m_canOpen)
    QSKIP("Database test skipped because no database could be opened.", SkipAll);

  try {
    ((QSqlDatabase*)(m->m_sql.data()))->close();
    QList<MyMoneyAccount> accList;
    m->accountList(accList);
  } catch (const MyMoneyException &e) {
    unexpectedException(e);
  }
}

void MyMoneyDatabaseMgrTest::testSetFunctions()
{
  testAttachDb();
  if (!m_canOpen)
    QSKIP("Database test skipped because no database could be opened.", SkipAll);

  MyMoneyPayee user = m->user();

  user.setName("Name");
  m->setUser(user);
  user.setAddress("Street");
  m->setUser(user);
  user.setCity("Town");
  m->setUser(user);
  user.setState("County");
  m->setUser(user);
  user.setPostcode("Postcode");
  m->setUser(user);
  user.setTelephone("Telephone");
  m->setUser(user);
  user.setEmail("Email");
  m->setUser(user);
  m->setValue("key", "value");

  user = m->user();
  QVERIFY(user.name() == "Name");
  QVERIFY(user.address() == "Street");
  QVERIFY(user.city() == "Town");
  QVERIFY(user.state() == "County");
  QVERIFY(user.postcode() == "Postcode");
  QVERIFY(user.telephone() == "Telephone");
  QVERIFY(user.email() == "Email");
  QVERIFY(m->value("key") == "value");

  m->setDirty();
  m->deletePair("key");
  QVERIFY(m->dirty() == false);
}

void MyMoneyDatabaseMgrTest::testSupportFunctions()
{
  testAttachDb();
  if (!m_canOpen)
    QSKIP("Database test skipped because no database could be opened.", SkipAll);

  try {
    QCOMPARE(m->nextInstitutionID(), QLatin1String("I000001"));
    QCOMPARE(m->nextAccountID(), QLatin1String("A000001"));
    QCOMPARE(m->nextTransactionID(), QLatin1String("T000000000000000001"));
    QCOMPARE(m->nextPayeeID(), QLatin1String("P000001"));
    QCOMPARE(m->nextTagID(), QLatin1String("G000001"));
    QCOMPARE(m->nextScheduleID(), QLatin1String("SCH000001"));
    QCOMPARE(m->nextReportID(), QLatin1String("R000001"));
    QCOMPARE(m->nextOnlineJobID(), QLatin1String("O00000001"));

    QCOMPARE(m->liability().name(), QLatin1String("Liability"));
    QCOMPARE(m->asset().name(), QLatin1String("Asset"));
    QCOMPARE(m->expense().name(), QLatin1String("Expense"));
    QCOMPARE(m->income().name(), QLatin1String("Income"));
    QCOMPARE(m->equity().name(), QLatin1String("Equity"));
    QCOMPARE(m->dirty(), false);
  } catch (const MyMoneyException &e) {
    unexpectedException(e);
  }
}

void MyMoneyDatabaseMgrTest::testIsStandardAccount()
{
  testAttachDb();
  if (!m_canOpen)
    QSKIP("Database test skipped because no database could be opened.", SkipAll);

  QVERIFY(m->isStandardAccount(STD_ACC_LIABILITY) == true);
  QVERIFY(m->isStandardAccount(STD_ACC_ASSET) == true);
  QVERIFY(m->isStandardAccount(STD_ACC_EXPENSE) == true);
  QVERIFY(m->isStandardAccount(STD_ACC_INCOME) == true);
  QVERIFY(m->isStandardAccount(STD_ACC_EQUITY) == true);
  QVERIFY(m->isStandardAccount("A0002") == false);
}

void MyMoneyDatabaseMgrTest::testNewAccount()
{
  testAttachDb();
  if (!m_canOpen)
    QSKIP("Database test skipped because no database could be opened.", SkipAll);

  MyMoneyAccount a;

  a.setName("AccountName");
  a.setNumber("AccountNumber");
  a.setValue("Key", "Value");

  m->addAccount(a);

  QCOMPARE(m->accountId(), 1ul);
  QList<MyMoneyAccount> accList;
  m->accountList(accList);
  QCOMPARE(accList.count(), 1);
  QCOMPARE((*(accList.begin())).name(), QLatin1String("AccountName"));
  QCOMPARE((*(accList.begin())).id(), QLatin1String("A000001"));
  QCOMPARE((*(accList.begin())).value("Key"), QLatin1String("Value"));
}

void MyMoneyDatabaseMgrTest::testAccount()
{
  testNewAccount();
  if (!m_canOpen)
    QSKIP("Database test skipped because no database could be opened.", SkipAll);

  m->setDirty();

  MyMoneyAccount a;

  // make sure that an invalid ID causes an exception
  try {
    a = m->account("Unknown ID");
    QFAIL("Exception expected");
  } catch (const MyMoneyException &) {
  }
  QVERIFY(m->dirty() == false);

  // now make sure, that a real ID works
  try {
    a = m->account("A000001");
    QVERIFY(a.name() == "AccountName");
    QVERIFY(a.id() == "A000001");
  } catch (const MyMoneyException &e) {
    unexpectedException(e);
  }
}

void MyMoneyDatabaseMgrTest::testAddNewAccount()
{
  testNewAccount();
  if (!m_canOpen)
    QSKIP("Database test skipped because no database could be opened.", SkipAll);

  MyMoneyAccount a, b;
  b.setName("Account2");
  b.setNumber("Acc2");
  m->addAccount(b);

  m->setDirty();

  QVERIFY(m->accountId() == 2);
  QList<MyMoneyAccount> accList;
  m->accountList(accList);
  QVERIFY(accList.count() == 2);

  // try to add account to undefined account
  try {
    MyMoneyAccount c("UnknownID", b);
    m->addAccount(c, a);
    QFAIL("Exception expected");
  } catch (const MyMoneyException &) {
  }

  QVERIFY(m->dirty() == false);
  // now try to add account 1 as sub-account to account 2
  try {
    a = m->account("A000001");
    QVERIFY(m->asset().accountList().count() == 0);
    m->addAccount(b, a);
    MyMoneyAccount acc(m->account("A000002"));
    QVERIFY(acc.accountList()[0] == "A000001");
    QVERIFY(acc.accountList().count() == 1);
    QVERIFY(m->asset().accountList().count() == 0);
  } catch (const MyMoneyException &e) {
    unexpectedException(e);
  }
}

void MyMoneyDatabaseMgrTest::testAddInstitution()
{
  testAttachDb();

  if (!m_canOpen)
    QSKIP("Database test skipped because no database could be opened.", SkipAll);

  MyMoneyInstitution i;

  i.setName("Inst Name");

  m->addInstitution(i);
  QVERIFY(m->institutionList().count() == 1);
  QVERIFY(m->institutionId() == 1);
  QVERIFY((*(m->institutionList().begin())).name() == "Inst Name");
  QVERIFY((*(m->institutionList().begin())).id() == "I000001");
}

void MyMoneyDatabaseMgrTest::testInstitution()
{
  testAddInstitution();
  if (!m_canOpen)
    QSKIP("Database test skipped because no database could be opened.", SkipAll);

  MyMoneyInstitution i;

  m->setDirty();

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
  } catch (const MyMoneyException &e) {
    unexpectedException(e);
  }
}

void MyMoneyDatabaseMgrTest::testAccount2Institution()
{
  testAttachDb();

  if (!m_canOpen)
    QSKIP("Database test skipped because no database could be opened.", SkipAll);

  testAddInstitution();
  testAddNewAccount();

  MyMoneyInstitution i;
  MyMoneyAccount a, b;

  try {
    i = m->institution("I000001");
    a = m->account("A000001");
  } catch (const MyMoneyException &e) {
    unexpectedException(e);
  }

  m->setDirty();

  // try to add to a false institution
  MyMoneyInstitution fake("Unknown ID", i);
  a.setInstitutionId(fake.id());
  try {
    m->modifyAccount(a);
    QFAIL("Exception expected");
  } catch (const MyMoneyException &) {
  }

  QVERIFY(m->dirty() == false);
  // now try to do it with a real institution
  try {
    QVERIFY(i.accountList().count() == 0);
    a.setInstitutionId(i.id());
    m->modifyAccount(a);
    QVERIFY(a.institutionId() == i.id());
    b = m->account("A000001");
    QVERIFY(b.institutionId() == i.id());
    QVERIFY(i.accountList().count() == 0);
  } catch (const MyMoneyException &e) {
    unexpectedException(e);
  }
}

void MyMoneyDatabaseMgrTest::testModifyAccount()
{
  testAttachDb();

  if (!m_canOpen)
    QSKIP("Database test skipped because no database could be opened.", SkipAll);

  testAccount2Institution();

  // test the OK case
  //
  //FIXME: modify 2 accounts simultaneously to trip a write error
  MyMoneyAccount a = m->account("A000001");
  a.setName("New account name");
  m->setDirty();
  try {
    m->modifyAccount(a);
    MyMoneyAccount b = m->account("A000001");
    QVERIFY(b.parentAccountId() == a.parentAccountId());
    QVERIFY(b.name() == "New account name");
    QVERIFY(b.value("Key") == "Value");
  } catch (const MyMoneyException &e) {
    unexpectedException(e);
  }

  // modify institution to unknown id
  MyMoneyAccount c("Unknown ID", a);
  m->setDirty();
  try {
    m->modifyAccount(c);
    QFAIL("Exception expected");
  } catch (const MyMoneyException &) {
  }

  // use different account type
  MyMoneyAccount d;
  d.setAccountType(Account::CreditCard);
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

void MyMoneyDatabaseMgrTest::testModifyInstitution()
{
  testAddInstitution();
  if (!m_canOpen)
    QSKIP("Database test skipped because no database could be opened.", SkipAll);

  MyMoneyInstitution i = m->institution("I000001");

  m->setDirty();
  i.setName("New inst name");
  try {
    m->modifyInstitution(i);
    i = m->institution("I000001");
    QVERIFY(i.name() == "New inst name");
  } catch (const MyMoneyException &e) {
    unexpectedException(e);
  }

  // try to modify an institution that does not exist
  MyMoneyInstitution f("Unknown ID", i);
  try {
    m->modifyInstitution(f);
    QFAIL("Exception expected");
  } catch (const MyMoneyException &) {
  }
}

void MyMoneyDatabaseMgrTest::testReparentAccount()
{
  testAttachDb();

  if (!m_canOpen)
    QSKIP("Database test skipped because no database could be opened.", SkipAll);

  // this one adds some accounts to the database
  MyMoneyAccount ex1;
  ex1.setAccountType(Account::Expense);
  MyMoneyAccount ex2;
  ex2.setAccountType(Account::Expense);
  MyMoneyAccount ex3;
  ex3.setAccountType(Account::Expense);
  MyMoneyAccount ex4;
  ex4.setAccountType(Account::Expense);
  MyMoneyAccount in;
  in.setAccountType(Account::Income);
  MyMoneyAccount ch;
  ch.setAccountType(Account::Checkings);

  ex1.setName("Sales Tax");
  ex2.setName("Sales Tax 16%");
  ex3.setName("Sales Tax 7%");
  ex4.setName("Grosseries");

  in.setName("Salary");
  ch.setName("My checkings account");
  ch.setValue("Key", "Value");

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
    QVERIFY(ch.value("Key") == "Value");

    MyMoneyAccount parent = m->expense();

    m->addAccount(parent, ex1);
    m->addAccount(ex1, ex2);
    m->addAccount(parent, ex3);
    m->addAccount(parent, ex4);

    parent = m->income();
    m->addAccount(parent, in);

    parent = m->asset();
    m->addAccount(parent, ch);
    QVERIFY(ch.value("Key") == "Value");

    MyMoneyFile::instance()->preloadCache();
    QVERIFY(m->expense().accountCount() == 3);
    QVERIFY(m->account(ex1.id()).accountCount() == 1);
    QVERIFY(ex3.parentAccountId() == STD_ACC_EXPENSE);

    //for (int i = 0; i < 100; ++i) {
    m->reparentAccount(ex3, ex1);
    //}
    MyMoneyFile::instance()->preloadCache();
    QVERIFY(m->expense().accountCount() == 2);
    QVERIFY(m->account(ex1.id()).accountCount() == 2);
    QVERIFY(ex3.parentAccountId() == ex1.id());
  } catch (const MyMoneyException &e) {
    unexpectedException(e);
  }
}

void MyMoneyDatabaseMgrTest::testAddTransactions()
{
  testAttachDb();

  if (!m_canOpen)
    QSKIP("Database test skipped because no database could be opened.", SkipAll);

  testReparentAccount();

  MyMoneyAccount ch;
  MyMoneyTransaction t1, t2;
  MyMoneySplit s;

  try {
    // I made some money, great
    s.setAccountId("A000006");  // Checkings
    s.setShares(MyMoneyMoney(100000, 100));
    s.setValue(MyMoneyMoney(100000, 100));
    QVERIFY(s.id().isEmpty());
    t1.addSplit(s);

    s.setId(QString());  // enable re-usage of split variable
    s.setAccountId("A000005");  // Salary
    s.setShares(MyMoneyMoney(-100000, 100));
    s.setValue(MyMoneyMoney(-100000, 100));
    QVERIFY(s.id().isEmpty());
    t1.addSplit(s);

    t1.setPostDate(QDate(2002, 5, 10));
  } catch (const MyMoneyException &e) {
    unexpectedException(e);
  }

  m->setDirty();
  try {
    ch = m->account("A000006");
    QVERIFY(ch.value("Key") == "Value");
    m->addTransaction(t1);
    QVERIFY(t1.id() == "T000000000000000001");
    QVERIFY(t1.splitCount() == 2);
    QVERIFY(m->transactionCount() == 1);
    ch = m->account("A000006");
    QVERIFY(ch.value("Key") == "Value");
  } catch (const MyMoneyException &e) {
    unexpectedException(e);
  }

  try {
    // I spent some money, not so great
    s.setId(QString());  // enable re-usage of split variable
    s.setAccountId("A000004");  // Grosseries
    s.setShares(MyMoneyMoney(10000, 100));
    s.setValue(MyMoneyMoney(10000, 100));
    QVERIFY(s.id().isEmpty());
    t2.addSplit(s);

    s.setId(QString());  // enable re-usage of split variable
    s.setAccountId("A000002");  // 16% sales tax
    s.setShares(MyMoneyMoney(1200, 100));
    s.setValue(MyMoneyMoney(1200, 100));
    QVERIFY(s.id().isEmpty());
    t2.addSplit(s);

    s.setId(QString());  // enable re-usage of split variable
    s.setAccountId("A000003");  // 7% sales tax
    s.setShares(MyMoneyMoney(400, 100));
    s.setValue(MyMoneyMoney(400, 100));
    QVERIFY(s.id().isEmpty());
    t2.addSplit(s);

    s.setId(QString());  // enable re-usage of split variable
    s.setAccountId("A000006");  // Checkings account
    s.setShares(MyMoneyMoney(-11600, 100));
    s.setValue(MyMoneyMoney(-11600, 100));
    QVERIFY(s.id().isEmpty());
    t2.addSplit(s);

    t2.setPostDate(QDate(2002, 5, 9));
  } catch (const MyMoneyException &e) {
    unexpectedException(e);
  }
  m->setDirty();
  try {
    m->addTransaction(t2);
    QVERIFY(t2.id() == "T000000000000000002");
    QVERIFY(t2.splitCount() == 4);
    QVERIFY(m->transactionCount() == 2);

    //QMap<QString, QString>::ConstIterator it_k;
    MyMoneyTransactionFilter f;
    QList<MyMoneyTransaction> transactionList(m->transactionList(f));
    QList<MyMoneyTransaction>::ConstIterator it_t(transactionList.constBegin());

    QCOMPARE((*it_t).id(), QLatin1String("T000000000000000002"));

    ++it_t;
    QCOMPARE((*it_t).id(), QLatin1String("T000000000000000001"));

    ++it_t;
    QCOMPARE(it_t, transactionList.constEnd());

    ch = m->account("A000006");
    QCOMPARE(ch.value("Key"), QLatin1String("Value"));

    // check that the account's transaction list is updated
    QList<MyMoneyTransaction> list;
    MyMoneyTransactionFilter filter("A000006");
    list = m->transactionList(filter);
    QCOMPARE(list.size(), 2);

    QList<MyMoneyTransaction>::ConstIterator it;
    it = list.constBegin();
    //QVERIFY((*it).id() == "T000000000000000002");
    QCOMPARE((*it), t2);
    ++it;

    QCOMPARE((*it), t1);
    ++it;
    QCOMPARE(it, list.constEnd());

  } catch (const MyMoneyException &e) {
    unexpectedException(e);
  }
}

void MyMoneyDatabaseMgrTest::testTransactionCount()
{
  testAttachDb();

  if (!m_canOpen)
    QSKIP("Database test skipped because no database could be opened.", SkipAll);

  testAddTransactions();
  QCOMPARE(m->transactionCount("A000001"), 0u);
  QCOMPARE(m->transactionCount("A000002"), 1u);
  QCOMPARE(m->transactionCount("A000003"), 1u);
  QCOMPARE(m->transactionCount("A000004"), 1u);
  QCOMPARE(m->transactionCount("A000005"), 1u);
  QCOMPARE(m->transactionCount("A000006"), 2u);
}

void MyMoneyDatabaseMgrTest::testAddBudget()
{
  testAttachDb();

  if (!m_canOpen)
    QSKIP("Database test skipped because no database could be opened.", SkipAll);

  MyMoneyBudget budget;

  budget.setName("TestBudget");
  budget.setBudgetStart(QDate::currentDate());

  m->addBudget(budget);

  QCOMPARE(m->budgetList().count(), 1);
  QCOMPARE(m->budgetId(), 1ul);
  MyMoneyBudget newBudget = m->budgetByName("TestBudget");

  QCOMPARE(budget.budgetStart(), newBudget.budgetStart());
  QCOMPARE(budget.name(), newBudget.name());
}

void MyMoneyDatabaseMgrTest::testCopyBudget()
{
  testAddBudget();

  if (!m_canOpen)
    QSKIP("Database test skipped because no database could be opened.", SkipAll);

  try {
    MyMoneyBudget oldBudget = m->budgetByName("TestBudget");
    MyMoneyBudget newBudget = oldBudget;

    newBudget.clearId();
    newBudget.setName(QString("Copy of %1").arg(oldBudget.name()));
    m->addBudget(newBudget);

    QCOMPARE(m->budgetList().count(), 2);
    QCOMPARE(m->budgetId(), 2ul);

    MyMoneyBudget testBudget = m->budgetByName("TestBudget");

    QCOMPARE(oldBudget.budgetStart(), testBudget.budgetStart());
    QCOMPARE(oldBudget.name(), testBudget.name());

    testBudget = m->budgetByName("Copy of TestBudget");

    QCOMPARE(testBudget.budgetStart(), newBudget.budgetStart());
    QCOMPARE(testBudget.name(), newBudget.name());
  } catch (QString& s) {
    QFAIL(qPrintable(QString("Error in testCopyBudget(): %1").arg(qPrintable(s))));
  }
}

void MyMoneyDatabaseMgrTest::testModifyBudget()
{
  testAddBudget();

  if (!m_canOpen)
    QSKIP("Database test skipped because no database could be opened.", SkipAll);

  MyMoneyBudget budget = m->budgetByName("TestBudget");

  budget.setBudgetStart(QDate::currentDate().addDays(-1));

  m->modifyBudget(budget);

  MyMoneyBudget newBudget = m->budgetByName("TestBudget");

  QCOMPARE(budget.id(), newBudget.id());
  QCOMPARE(budget.budgetStart(), newBudget.budgetStart());
  QCOMPARE(budget.name(), newBudget.name());
}

void MyMoneyDatabaseMgrTest::testRemoveBudget()
{
  testAddBudget();

  if (!m_canOpen)
    QSKIP("Database test skipped because no database could be opened.", SkipAll);

  MyMoneyBudget budget = m->budgetByName("TestBudget");
  m->removeBudget(budget);

  try {
    budget = m->budgetByName("TestBudget");
    // exception should be thrown if budget not found.
    QFAIL("Missing expected exception.");
  } catch (const MyMoneyException &) {
    QVERIFY(true);
  }
}

void MyMoneyDatabaseMgrTest::testBalance()
{
  testAttachDb();

  if (!m_canOpen)
    QSKIP("Database test skipped because no database could be opened.", SkipAll);

  testAddTransactions();

  try {
    QVERIFY(m->balance("A000001", QDate()).isZero());
    QCOMPARE(m->balance("A000002", QDate()), MyMoneyMoney(1200, 100));
    QCOMPARE(m->balance("A000003", QDate()), MyMoneyMoney(400, 100));
    //Add a transaction to zero account A000003
    MyMoneyTransaction t1;
    MyMoneySplit s;

    s.setAccountId("A000003");
    s.setShares(MyMoneyMoney(-400, 100));
    s.setValue(MyMoneyMoney(-400, 100));
    QVERIFY(s.id().isEmpty());
    t1.addSplit(s);

    s.setId(QString());  // enable re-usage of split variable
    s.setAccountId("A000002");
    s.setShares(MyMoneyMoney(400, 100));
    s.setValue(MyMoneyMoney(400, 100));
    QVERIFY(s.id().isEmpty());
    t1.addSplit(s);

    t1.setPostDate(QDate(2007, 5, 10));

    m->addTransaction(t1);

    QVERIFY(m->balance("A000003", QDate()).isZero());
    QCOMPARE(m->totalBalance("A000001", QDate()), MyMoneyMoney(1600, 100));
    QCOMPARE(m->balance("A000006", QDate(2002, 5, 9)), MyMoneyMoney(-11600, 100));
    QCOMPARE(m->balance("A000005", QDate(2002, 5, 10)), MyMoneyMoney(-100000, 100));
    QCOMPARE(m->balance("A000006", QDate(2002, 5, 10)), MyMoneyMoney(88400, 100));
  } catch (const MyMoneyException &e) {
    unexpectedException(e);
  }
}

void MyMoneyDatabaseMgrTest::testModifyTransaction()
{
  testAttachDb();

  if (!m_canOpen)
    QSKIP("Database test skipped because no database could be opened.", SkipAll);

  testAddTransactions();

  MyMoneyTransaction t = m->transaction("T000000000000000002");
  MyMoneySplit s;
  MyMoneyAccount ch;

  // just modify simple stuff (splits)
  QVERIFY(t.splitCount() == 4);

  ch = m->account("A000006");
  QVERIFY(ch.value("Key") == "Value");

  s = t.splits()[0];
  s.setShares(MyMoneyMoney(11000, 100));
  s.setValue(MyMoneyMoney(11000, 100));
  t.modifySplit(s);

  QVERIFY(t.splitCount() == 4);
  s = t.splits()[3];
  s.setShares(MyMoneyMoney(-12600, 100));
  s.setValue(MyMoneyMoney(-12600, 100));
  t.modifySplit(s);
  ch = m->account("A000006");
  QVERIFY(ch.value("Key") == "Value");

  try {
    QVERIFY(m->balance("A000004", QDate()) == MyMoneyMoney(10000, 100));
    QVERIFY(m->balance("A000006", QDate()) == MyMoneyMoney(100000 - 11600, 100));
    QVERIFY(m->totalBalance("A000001", QDate()) == MyMoneyMoney(1600, 100));
    m->modifyTransaction(t);
    ch = m->account("A000006");
    QVERIFY(ch.value("Key") == "Value");
    QVERIFY(m->balance("A000004", QDate()) == MyMoneyMoney(11000, 100));
    QVERIFY(m->balance("A000006", QDate()) == MyMoneyMoney(100000 - 12600, 100));
    QVERIFY(m->totalBalance("A000001", QDate()) == MyMoneyMoney(1600, 100));
  } catch (const MyMoneyException &e) {
    unexpectedException(e);
  }

  // now modify the date
  t.setPostDate(QDate(2002, 5, 11));
  try {
    ch = m->account("A000006");
    QVERIFY(ch.value("Key") == "Value");
    m->modifyTransaction(t);
    QVERIFY(m->balance("A000004", QDate()) == MyMoneyMoney(11000, 100));
    QVERIFY(m->balance("A000006", QDate()) == MyMoneyMoney(100000 - 12600, 100));
    QVERIFY(m->totalBalance("A000001", QDate()) == MyMoneyMoney(1600, 100));

    //QMap<QString, QString>::ConstIterator it_k;
    MyMoneyTransactionFilter f;
    QList<MyMoneyTransaction> transactionList(m->transactionList(f));
    QList<MyMoneyTransaction>::ConstIterator it_t(transactionList.constBegin());
    //it_k = m->m_transactionKeys.begin();
    //QVERIFY((*it_k) == "2002-05-10-T000000000000000001");
    QVERIFY((*it_t).id() == "T000000000000000001");
    //++it_k;
    ++it_t;
    //QVERIFY((*it_k) == "2002-05-11-T000000000000000002");
    QVERIFY((*it_t).id() == "T000000000000000002");
    //++it_k;
    ++it_t;
    //QVERIFY(it_k == m->m_transactionKeys.end());
    QVERIFY(it_t == transactionList.constEnd());

    ch = m->account("A000006");
    QVERIFY(ch.value("Key") == "Value");

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

  } catch (const MyMoneyException &e) {
    unexpectedException(e);
  }

  // Create another transaction
  MyMoneyTransaction t1;
  try {
    s.setId(QString());  // enable re-usage of split variable
    s.setAccountId("A000006");  // Checkings
    s.setShares(MyMoneyMoney(10000, 100));
    s.setValue(MyMoneyMoney(10000, 100));
    QVERIFY(s.id().isEmpty());
    t1.addSplit(s);

    s.setId(QString());  // enable re-usage of split variable
    s.setAccountId("A000005");  // Salary
    s.setShares(MyMoneyMoney(-10000, 100));
    s.setValue(MyMoneyMoney(-10000, 100));
    QVERIFY(s.id().isEmpty());
    t1.addSplit(s);

    t1.setPostDate(QDate(2002, 5, 10));
  } catch (const MyMoneyException &e) {
    unexpectedException(e);
  }

  // Add it to the database
  m->addTransaction(t1);

  ch = m->account("A000005");
  QVERIFY(ch.balance() == MyMoneyMoney(-100000 - 10000, 100));
  QVERIFY(m->balance("A000005", QDate()) == MyMoneyMoney(-100000 - 10000, 100));

  ch = m->account("A000006");
  QVERIFY(ch.balance() == MyMoneyMoney(100000 - 12600 + 10000, 100));
  QVERIFY(m->balance("A000006", QDate()) == MyMoneyMoney(100000 - 12600 + 10000, 100));

  // Oops, the income was classified as Salary, but should have been
  // a refund from the grocery store.
  t1.splits()[1].setAccountId("A000004");

  m->modifyTransaction(t1);

  // Make sure the account balances got updated correctly.
  ch = m->account("A000004");
  QVERIFY(ch.balance() == MyMoneyMoney(11000 - 10000, 100));
  QVERIFY(m->balance("A000004", QDate()) == MyMoneyMoney(11000 - 10000, 100));

  ch = m->account("A000005");
  QVERIFY(m->balance("A000005", QDate()) == MyMoneyMoney(-100000, 100));
  QVERIFY(ch.balance() == MyMoneyMoney(-100000, 100));

  ch = m->account("A000006");
  QVERIFY(ch.balance() == MyMoneyMoney(100000 - 12600 + 10000, 100));
  QVERIFY(m->balance("A000006", QDate()) == MyMoneyMoney(100000 - 12600 + 10000, 100));

}


void MyMoneyDatabaseMgrTest::testRemoveUnusedAccount()
{
  testAttachDb();

  if (!m_canOpen)
    QSKIP("Database test skipped because no database could be opened.", SkipAll);

  testAccount2Institution();

  MyMoneyAccount a = m->account("A000001");
  MyMoneyInstitution i = m->institution("I000001");

  m->setDirty();
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
    MyMoneyFile::instance()->preloadCache();
    i = m->institution("I000001");

    //QVERIFY(i.accountCount() == 0);
    QVERIFY(i.accountCount() == 1);
    QVERIFY(m->accountCount() == 7);

    a.setInstitutionId(QString());
    m->modifyAccount(a);
    m->removeAccount(a);
    QVERIFY(m->accountCount() == 6);
    i = m->institution("I000001");
    QVERIFY(i.accountCount() == 0);
  } catch (const MyMoneyException &e) {
    unexpectedException(e);
  }
}

void MyMoneyDatabaseMgrTest::testRemoveUsedAccount()
{
  testAttachDb();

  if (!m_canOpen)
    QSKIP("Database test skipped because no database could be opened.", SkipAll);

  testAddTransactions();

  MyMoneyAccount a = m->account("A000006");

  try {
    m->removeAccount(a);
    QFAIL("Exception expected");
  } catch (const MyMoneyException &) {
  }
}

void MyMoneyDatabaseMgrTest::testRemoveInstitution()
{
  testAttachDb();

  if (!m_canOpen)
    QSKIP("Database test skipped because no database could be opened.", SkipAll);

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
  } catch (const MyMoneyException &e) {
    unexpectedException(e);
  }

  m->setDirty();
  // now remove the institution and see if the account survived ;-)
  try {
    m->removeInstitution(i);
    a.setInstitutionId(QString());
    m->modifyAccount(a);
    a = m->account("A000006");
    QVERIFY(a.institutionId().isEmpty());
    QVERIFY(m->institutionCount() == 0);
  } catch (const MyMoneyException &e) {
    unexpectedException(e);
  }
}

void MyMoneyDatabaseMgrTest::testRemoveTransaction()
{
  testAttachDb();

  if (!m_canOpen)
    QSKIP("Database test skipped because no database could be opened.", SkipAll);

  testAddTransactions();

  MyMoneyTransaction t = m->transaction("T000000000000000002");

  m->setDirty();
  try {
    m->removeTransaction(t);
    QVERIFY(m->transactionCount() == 1);
  } catch (const MyMoneyException &e) {
    unexpectedException(e);
  }
}

void MyMoneyDatabaseMgrTest::testTransactionList()
{
  testAttachDb();

  if (!m_canOpen)
    QSKIP("Database test skipped because no database could be opened.", SkipAll);

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

  // test the date filtering while split filtering is active but with an empty filter
  filter.clear();
  filter.addPayee(QString());
  filter.setDateFilter(QDate(2002, 5, 10), QDate(2002, 5, 10));
  list = m->transactionList(filter);
  QVERIFY(list.count() == 1);
  QVERIFY(list.at(0).id() == "T000000000000000001");

  filter.clear();
  filter.addAccount(QString());
  filter.setDateFilter(QDate(2002, 5, 9), QDate(2002, 5, 9));
  list = m->transactionList(filter);
  QVERIFY(list.count() == 1);
  QVERIFY(list.at(0).id() == "T000000000000000002");
}

void MyMoneyDatabaseMgrTest::testAddPayee()
{
  testAttachDb();

  if (!m_canOpen)
    QSKIP("Database test skipped because no database could be opened.", SkipAll);

  MyMoneyPayee p;

  p.setName("THB");
  m->setDirty();
  try {
    QVERIFY(m->payeeId() == 0);
    m->addPayee(p);
    QVERIFY(m->payeeId() == 1);
    MyMoneyPayee p1 = m->payeeByName("THB");
    QVERIFY(p.id() == p1.id());
    QVERIFY(p.name() == p1.name());
    QVERIFY(p.address() == p1.address());
    QVERIFY(p.city() == p1.city());
    QVERIFY(p.state() == p1.state());
    QVERIFY(p.postcode() == p1.postcode());
    QVERIFY(p.telephone() == p1.telephone());
    QVERIFY(p.email() == p1.email());
    MyMoneyPayee::payeeMatchType m, m1;
    bool ignore, ignore1;
    QStringList keys, keys1;
    m = p.matchData(ignore, keys);
    m1 = p1.matchData(ignore1, keys1);
    QVERIFY(m == m1);
    QVERIFY(ignore == ignore1);
    QVERIFY(keys == keys1);
    QVERIFY(p.reference() == p1.reference());
    QVERIFY(p.defaultAccountEnabled() == p1.defaultAccountEnabled());
    QVERIFY(p.defaultAccountId() == p1.defaultAccountId());

    QVERIFY(p == p1);
  } catch (const MyMoneyException &e) {
    unexpectedException(e);
  }

}

void MyMoneyDatabaseMgrTest::testSetAccountName()
{
  testAttachDb();

  if (!m_canOpen)
    QSKIP("Database test skipped because no database could be opened.", SkipAll);

  try {
    m->setAccountName(STD_ACC_LIABILITY, "Verbindlichkeiten");
  } catch (const MyMoneyException &e) {
    unexpectedException(e);
  }
  try {
    m->setAccountName(STD_ACC_ASSET, "Verm�gen");
  } catch (const MyMoneyException &e) {
    unexpectedException(e);
  }
  try {
    m->setAccountName(STD_ACC_EXPENSE, "Ausgaben");
  } catch (const MyMoneyException &e) {
    unexpectedException(e);
  }
  try {
    m->setAccountName(STD_ACC_INCOME, "Einnahmen");
  } catch (const MyMoneyException &e) {
    unexpectedException(e);
  }

  MyMoneyFile::instance()->preloadCache();

  try {
    QVERIFY(m->liability().name() == "Verbindlichkeiten");
    QVERIFY(m->asset().name() == "Verm�gen");
    QVERIFY(m->expense().name() == "Ausgaben");
    QVERIFY(m->income().name() == "Einnahmen");
  } catch (const MyMoneyException &e) {
    unexpectedException(e);
  }

  try {
    m->setAccountName("A000001", "New account name");
    QFAIL("Exception expected");
  } catch (const MyMoneyException &) {
  }
}

void MyMoneyDatabaseMgrTest::testModifyPayee()
{
  testAttachDb();

  if (!m_canOpen)
    QSKIP("Database test skipped because no database could be opened.", SkipAll);

  MyMoneyPayee p;

  testAddPayee();

  p = m->payee("P000001");
  p.setName("New name");
  m->setDirty();
  try {
    m->modifyPayee(p);
    p = m->payee("P000001");
    QVERIFY(p.name() == "New name");
  } catch (const MyMoneyException &e) {
    unexpectedException(e);
  }
}

void MyMoneyDatabaseMgrTest::testRemovePayee()
{
  testAttachDb();

  if (!m_canOpen)
    QSKIP("Database test skipped because no database could be opened.", SkipAll);

  testAddPayee();
  m->setDirty();

  // check that we can remove an unreferenced payee
  MyMoneyPayee p = m->payee("P000001");
  try {
    QVERIFY(m->payeeList().count() == 1);
    m->removePayee(p);
    QVERIFY(m->payeeList().count() == 0);
  } catch (const MyMoneyException &e) {
    unexpectedException(e);
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

  // reset here, so that the
  // testAddPayee will not fail
  m->loadPayeeId(0);
  testAddPayee();

  // check that it works when the payee exists
  try {
    m->modifyTransaction(tr);
  } catch (const MyMoneyException &e) {
    unexpectedException(e);
  }

  m->setDirty();

  // now check, that we cannot remove the payee
  try {
    m->removePayee(p);
    QFAIL("Expected exception");
  } catch (const MyMoneyException &) {
  }
  QVERIFY(m->payeeList().count() == 1);
}

void MyMoneyDatabaseMgrTest::testAddTag()
{
  testAttachDb();

  if (!m_canOpen)
    QSKIP("Database test skipped because no database could be opened.", SkipAll);

  MyMoneyTag ta;

  ta.setName("THB");
  m->setDirty();
  try {
    QVERIFY(m->tagId() == 0);
    m->addTag(ta);
    QVERIFY(m->tagId() == 1);
    MyMoneyTag ta1 = m->tagByName("THB");
    QVERIFY(ta.id() == ta1.id());
    QVERIFY(ta.name() == ta1.name());
    QVERIFY(ta.isClosed() == ta1.isClosed());
    QVERIFY(ta.tagColor().name() == ta1.tagColor().name());
    QVERIFY(ta.notes() == ta1.notes());
    QVERIFY(ta == ta1);
  } catch (const MyMoneyException &e) {
    unexpectedException(e);
  }

}

void MyMoneyDatabaseMgrTest::testModifyTag()
{
  testAttachDb();

  if (!m_canOpen)
    QSKIP("Database test skipped because no database could be opened.", SkipAll);

  MyMoneyTag ta;

  testAddTag();

  ta = m->tag("G000001");
  ta.setName("New name");
  m->setDirty();
  try {
    m->modifyTag(ta);
    ta = m->tag("G000001");
    QVERIFY(ta.name() == "New name");
  } catch (const MyMoneyException &e) {
    unexpectedException(e);
  }
}

void MyMoneyDatabaseMgrTest::testRemoveTag()
{
  testAttachDb();

  if (!m_canOpen)
    QSKIP("Database test skipped because no database could be opened.", SkipAll);

  testAddTag();
  m->setDirty();

  // check that we can remove an unreferenced tag
  MyMoneyTag ta = m->tag("G000001");
  try {
    QVERIFY(m->tagList().count() == 1);
    m->removeTag(ta);
    QVERIFY(m->tagList().count() == 0);
  } catch (const MyMoneyException &e) {
    unexpectedException(e);
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

  // reset here, so that the
  // testAddTag will not fail
  m->loadTagId(0);
  testAddTag();

  // check that it works when the tag exists
  try {
    m->modifyTransaction(tr);
  } catch (const MyMoneyException &e) {
    unexpectedException(e);
  }

  m->setDirty();

  // now check, that we cannot remove the tag
  try {
    m->removeTag(ta);
    QFAIL("Expected exception");
  } catch (const MyMoneyException &) {
  }
  QVERIFY(m->tagList().count() == 1);
}

void MyMoneyDatabaseMgrTest::testRemoveAccountFromTree()
{
  testAttachDb();

  if (!m_canOpen)
    QSKIP("Database test skipped because no database could be opened.", SkipAll);

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

void MyMoneyDatabaseMgrTest::testPayeeName()
{
  testAttachDb();

  if (!m_canOpen)
    QSKIP("Database test skipped because no database could be opened.", SkipAll);

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

void MyMoneyDatabaseMgrTest::testTagName()
{
  testAttachDb();

  if (!m_canOpen)
    QSKIP("Database test skipped because no database could be opened.", SkipAll);

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

void MyMoneyDatabaseMgrTest::testAssignment()
{
  testAttachDb();

  if (!m_canOpen)
    QSKIP("Database test skipped because no database could be opened.", SkipAll);

  testAddTransactions();

  MyMoneyPayee user;
  user.setName("Thomas");
  m->setUser(user);

  MyMoneyDatabaseMgr test = *m;
  testEquality(&test);
}

void MyMoneyDatabaseMgrTest::testEquality(const MyMoneyDatabaseMgr *t)
{
  testAttachDb();

  if (!m_canOpen)
    QSKIP("Database test skipped because no database could be opened.", SkipAll);

  QVERIFY(m->user().name() == t->user().name());
  QVERIFY(m->user().address() == t->user().address());
  QVERIFY(m->user().city() == t->user().city());
  QVERIFY(m->user().state() == t->user().state());
  QVERIFY(m->user().postcode() == t->user().postcode());
  QVERIFY(m->user().telephone() == t->user().telephone());
  QVERIFY(m->user().email() == t->user().email());
  //QVERIFY(m->nextInstitutionID() == t->nextInstitutionID());
  //QVERIFY(m->nextAccountID() == t->nextAccountID());
  //QVERIFY(m->m_nextTransactionID == t->m_nextTransactionID);
  //QVERIFY(m->nextPayeeID() == t->nextPayeeID());
  //QVERIFY(m->m_nextScheduleID == t->m_nextScheduleID);
  QVERIFY(m->dirty() == t->dirty());
  QVERIFY(m->m_creationDate == t->m_creationDate);
  QVERIFY(m->m_lastModificationDate == t->m_lastModificationDate);

  /*
   * make sure, that the keys and values are the same
   * on the left and the right side
   */
  //QVERIFY(m->payeeList().keys() == t->payeeList().keys());
  //QVERIFY(m->payeeList().values() == t->payeeList().values());
  QVERIFY(m->payeeList() == t->payeeList());
  QVERIFY(m->tagList() == t->tagList());
  //QVERIFY(m->m_transactionKeys.keys() == t->m_transactionKeys.keys());
  //QVERIFY(m->m_transactionKeys.values() == t->m_transactionKeys.values());
  //QVERIFY(m->institutionList().keys() == t->institutionList().keys());
  //QVERIFY(m->institutionList().values() == t->institutionList().values());
  //QVERIFY(m->m_accountList.keys() == t->m_accountList.keys());
  //QVERIFY(m->m_accountList.values() == t->m_accountList.values());
  //QVERIFY(m->m_transactionList.keys() == t->m_transactionList.keys());
  //QVERIFY(m->m_transactionList.values() == t->m_transactionList.values());
  //QVERIFY(m->m_balanceCache.keys() == t->m_balanceCache.keys());
  //QVERIFY(m->m_balanceCache.values() == t->m_balanceCache.values());

//  QVERIFY(m->scheduleList().keys() == t->scheduleList().keys());
//  QVERIFY(m->scheduleList().values() == t->scheduleList().values());
}

void MyMoneyDatabaseMgrTest::testDuplicate()
{
  testAttachDb();

  if (!m_canOpen)
    QSKIP("Database test skipped because no database could be opened.", SkipAll);

  const MyMoneyDatabaseMgr* t;

  testModifyTransaction();

  t = m->duplicate();
  testEquality(t);
  delete t;
}

void MyMoneyDatabaseMgrTest::testAddSchedule()
{
  /* Note addSchedule() now calls validate as it should
   * so we need an account id.  Later this will
   * be checked to make sure its a valid account id.  The
   * tests currently fail because no splits are defined
   * for the schedules transaction.
  */

  testAttachDb();

  if (!m_canOpen)
    QSKIP("Database test skipped because no database could be opened.", SkipAll);

  // put some accounts in the db, so the tests don't break
  testReparentAccount();

  try {
    QVERIFY(m->scheduleList().count() == 0);
    MyMoneyTransaction t1;
    MyMoneySplit s1, s2;
    s1.setAccountId("A000001");
    t1.addSplit(s1);
    s2.setAccountId("A000002");
    t1.addSplit(s2);
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

    QVERIFY(m->scheduleList().count() == 1);
    QVERIFY(schedule.id() == "SCH000001");
    //MyMoneyFile::instance()->clearCache(); // test passes without this, so why is it here for?
    QVERIFY(m->schedule("SCH000001").id() == "SCH000001");
  } catch (const MyMoneyException &e) {
    unexpectedException(e);
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

  QVERIFY(m->scheduleList().count() == 1);

  // now try with a bad account, so this should cause an exception
  try {
    MyMoneyTransaction t1;
    MyMoneySplit s1, s2;
    s1.setAccountId("Abadaccount1");
    t1.addSplit(s1);
    s2.setAccountId("Abadaccount2");
    //t1.addSplit(s2);
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
    QFAIL("Exception expected, but not thrown");
  } catch (const MyMoneyException &) {
    // Exception caught as expected.
  }

  QVERIFY(m->scheduleList().count() == 1);
}

void MyMoneyDatabaseMgrTest::testSchedule()
{
  testAttachDb();

  if (!m_canOpen)
    QSKIP("Database test skipped because no database could be opened.", SkipAll);

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

void MyMoneyDatabaseMgrTest::testModifySchedule()
{
  testAttachDb();

  if (!m_canOpen)
    QSKIP("Database test skipped because no database could be opened.", SkipAll);

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
    QVERIFY(m->scheduleList().count() == 1);
    QVERIFY((*(m->scheduleList().begin())).name() == "New Sched-Name");
    QVERIFY((*(m->scheduleList().begin())).id() == "SCH000001");

  } catch (const MyMoneyException &e) {
    unexpectedException(e);
  }

}

void MyMoneyDatabaseMgrTest::testRemoveSchedule()
{
  testAttachDb();

  if (!m_canOpen)
    QSKIP("Database test skipped because no database could be opened.", SkipAll);

  testAddSchedule();
  MyMoneySchedule sched;

  sched = m->schedule("SCH000001");
  sched.setId("SCH000002");
  try {
    m->removeSchedule(sched);
    QFAIL("Exception expected");
  } catch (const MyMoneyException &) {
  }

  sched = m->schedule("SCH000001");
  try {
    m->removeSchedule(sched);
    QVERIFY(m->scheduleList().count() == 0);

  } catch (const MyMoneyException &e) {
    unexpectedException(e);
  }
}

void MyMoneyDatabaseMgrTest::testScheduleList()
{
  testAttachDb();

  if (!m_canOpen)
    QSKIP("Database test skipped because no database could be opened.", SkipAll);

  // put some accounts in the db, so the tests don't break
  testReparentAccount();

  QDate  testDate = QDate::currentDate();
  QDate  notOverdue = testDate.addDays(2);
  QDate  overdue = testDate.addDays(-2);

  MyMoneyTransaction t1;
  MyMoneySplit s1, s2;
  s1.setAccountId("A000001");
  t1.addSplit(s1);
  s2.setAccountId("A000002");
  t1.addSplit(s2);
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
    unexpectedException(e);
  }

  QList<MyMoneySchedule> list;

  // no filter
  list = m->scheduleList();
  QVERIFY(list.count() == 4);

  // filter by type
  list = m->scheduleList(QString(), Schedule::Type::Bill);
  QVERIFY(list.count() == 2);
  QVERIFY(list[0].name() == "Schedule 1");
  QVERIFY(list[1].name() == "Schedule 4");

  // filter by occurrence
  list = m->scheduleList(QString(), Schedule::Type::Any,
                         Schedule::Occurrence::Daily);
  QVERIFY(list.count() == 1);
  QVERIFY(list[0].name() == "Schedule 2");

  // filter by payment type
  list = m->scheduleList(QString(), Schedule::Type::Any,
                         Schedule::Occurrence::Any,
                         Schedule::PaymentType::DirectDeposit);
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
  list = m->scheduleList(QString(), Schedule::Type::Any,
                         Schedule::Occurrence::Any,
                         Schedule::PaymentType::Any,
                         notOverdue.addDays(31),
                         QDate(),
                         false);
  QVERIFY(list.count() == 3);
  QVERIFY(list[0].name() == "Schedule 2");
  QVERIFY(list[1].name() == "Schedule 3");
  QVERIFY(list[2].name() == "Schedule 4");

  // filter by end date
  list = m->scheduleList(QString(), Schedule::Type::Any,
                         Schedule::Occurrence::Any,
                         Schedule::PaymentType::Any,
                         QDate(),
                         notOverdue.addDays(1),
                         false);
  QVERIFY(list.count() == 3);
  QVERIFY(list[0].name() == "Schedule 1");
  QVERIFY(list[1].name() == "Schedule 2");
  QVERIFY(list[2].name() == "Schedule 4");

  // filter by start and end date
  list = m->scheduleList(QString(), Schedule::Type::Any,
                         Schedule::Occurrence::Any,
                         Schedule::PaymentType::Any,
                         notOverdue.addDays(-1),
                         notOverdue.addDays(1),
                         false);
  QVERIFY(list.count() == 2);
  QVERIFY(list[0].name() == "Schedule 1");
  QVERIFY(list[1].name() == "Schedule 2");

  // filter by overdue status
  list = m->scheduleList(QString(), Schedule::Type::Any,
                         Schedule::Occurrence::Any,
                         Schedule::PaymentType::Any,
                         QDate(),
                         QDate(),
                         true);
  QVERIFY(list.count() == 1);
  QVERIFY(list[0].name() == "Schedule 4");
}

void MyMoneyDatabaseMgrTest::testAddCurrency()
{
  testAttachDb();

  if (!m_canOpen)
    QSKIP("Database test skipped because no database could be opened.", SkipAll);

  MyMoneySecurity curr("EUR", "Euro", "?", 100, 100);
  QVERIFY(m->currencyList().count() == 0);
  m->setDirty();
  try {
    m->addCurrency(curr);
    QVERIFY(m->currencyList().count() == 1);
    QVERIFY((*(m->currencyList().begin())).name() == "Euro");
    QVERIFY((*(m->currencyList().begin())).id() == "EUR");
  } catch (const MyMoneyException &e) {
    unexpectedException(e);
  }

  m->setDirty();
  try {
    m->addCurrency(curr);
    QFAIL("Expected exception missing");
  } catch (const MyMoneyException &) {
    QVERIFY(m->dirty() == false);
  }
}

void MyMoneyDatabaseMgrTest::testModifyCurrency()
{
  testAttachDb();

  if (!m_canOpen)
    QSKIP("Database test skipped because no database could be opened.", SkipAll);

  MyMoneySecurity curr("EUR", "Euro", "?", 100, 100);
  testAddCurrency();
  m->setDirty();
  curr.setName("EURO");
  try {
    m->modifyCurrency(curr);
    QVERIFY(m->currencyList().count() == 1);
    QVERIFY((*(m->currencyList().begin())).name() == "EURO");
    QVERIFY((*(m->currencyList().begin())).id() == "EUR");
  } catch (const MyMoneyException &e) {
    unexpectedException(e);
  }

  m->setDirty();

  MyMoneySecurity unknownCurr("DEM", "Deutsche Mark", "DM", 100, 100);
  try {
    m->modifyCurrency(unknownCurr);
    QFAIL("Expected exception missing");
  } catch (const MyMoneyException &) {
    QVERIFY(m->dirty() == false);
  }
}

void MyMoneyDatabaseMgrTest::testRemoveCurrency()
{
  testAttachDb();

  if (!m_canOpen)
    QSKIP("Database test skipped because no database could be opened.", SkipAll);

  MyMoneySecurity curr("EUR", "Euro", "?", 100, 100);
  testAddCurrency();
  m->setDirty();
  try {
    m->removeCurrency(curr);
    QVERIFY(m->currencyList().count() == 0);
  } catch (const MyMoneyException &e) {
    unexpectedException(e);
  }

  m->setDirty();

  MyMoneySecurity unknownCurr("DEM", "Deutsche Mark", "DM", 100, 100);
  try {
    m->removeCurrency(unknownCurr);
    QFAIL("Expected exception missing");
  } catch (const MyMoneyException &) {
    QVERIFY(m->dirty() == false);
  }
}

void MyMoneyDatabaseMgrTest::testCurrency()
{
  testAttachDb();

  if (!m_canOpen)
    QSKIP("Database test skipped because no database could be opened.", SkipAll);

  MyMoneySecurity curr("EUR", "Euro", "?", 100, 100);
  MyMoneySecurity newCurr;
  testAddCurrency();
  m->setDirty();
  try {
    newCurr = m->currency("EUR");
    QVERIFY(m->dirty() == false);
    QVERIFY(newCurr.id() == curr.id());
    QVERIFY(newCurr.name() == curr.name());
  } catch (const MyMoneyException &e) {
    unexpectedException(e);
  }

  try {
    m->currency("DEM");
    QFAIL("Expected exception missing");
  } catch (const MyMoneyException &) {
    QVERIFY(m->dirty() == false);
  }
}

void MyMoneyDatabaseMgrTest::testCurrencyList()
{
  testAttachDb();

  if (!m_canOpen)
    QSKIP("Database test skipped because no database could be opened.", SkipAll);

  QVERIFY(m->currencyList().count() == 0);

  testAddCurrency();
  QVERIFY(m->currencyList().count() == 1);

  MyMoneySecurity unknownCurr("DEM", "Deutsche Mark", "DM", 100, 100);
  try {
    m->addCurrency(unknownCurr);
    m->setDirty();
    QVERIFY(m->currencyList().count() == 2);
    QVERIFY(m->currencyList().count() == 2);
    QVERIFY(m->dirty() == false);
  } catch (const MyMoneyException &e) {
    unexpectedException(e);
  }
}

void MyMoneyDatabaseMgrTest::testAccountList()
{
  testAttachDb();

  if (!m_canOpen)
    QSKIP("Database test skipped because no database could be opened.", SkipAll);

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

void MyMoneyDatabaseMgrTest::testAddOnlineJob()
{
  testAttachDb();

  if (!m_canOpen)
    QSKIP("Database test skipped because no database could be opened.", SkipAll);

  // Add a onlineJob
  onlineJob job(new dummyTask());

  QCOMPARE(m->onlineJobList().count(), 0);
  m->setDirty();

  QSKIP("Test not fully implemented, yet.", SkipAll);

  try {
    m->addOnlineJob(job);

    QCOMPARE(m->onlineJobList().count(), 1);
    QCOMPARE((*(m->onlineJobList().begin())).id(), QLatin1String("O00000001"));

  } catch (const MyMoneyException &e) {
    unexpectedException(e);
  }

  // Try to re-add the same job. It should fail.
  m->setDirty();
  try {
    m->addOnlineJob(job);
    QFAIL("Expected exception missing");
  } catch (const MyMoneyException &) {
    QCOMPARE(m->dirty(), false);
  }
}

void MyMoneyDatabaseMgrTest::testModifyOnlineJob()
{
  testAttachDb();

  if (!m_canOpen)
    QSKIP("Database test skipped because no database could be opened.", SkipAll);

  onlineJob job(new dummyTask());
  testAddOnlineJob();
  m->setDirty();

  QSKIP("Test not fully implemented, yet.", SkipAll);

  // update online job
  try {
    m->modifyOnlineJob(job);
    QVERIFY(m->onlineJobList().count() == 1);
    //QVERIFY((*(m->onlineJobList().begin())).name() == "EURO");
    QVERIFY((*(m->onlineJobList().begin())).id() == "O00000001");
  } catch (const MyMoneyException &e) {
    unexpectedException(e);
  }

  m->setDirty();

  onlineJob unknownJob(new dummyTask());
  try {
    m->modifyOnlineJob(unknownJob);
    QFAIL("Expected exception missing");
  } catch (const MyMoneyException &) {
    QVERIFY(m->dirty() == false);
  }
}

void MyMoneyDatabaseMgrTest::testRemoveOnlineJob()
{
  testAttachDb();

  if (!m_canOpen)
    QSKIP("Database test skipped because no database could be opened.", SkipAll);

  onlineJob job(new dummyTask());
  testAddOnlineJob();
  m->setDirty();

  QSKIP("Test not fully implemented, yet.", SkipAll);

  try {
    m->removeOnlineJob(job);
    QVERIFY(m->onlineJobList().count() == 0);
  } catch (const MyMoneyException &e) {
    unexpectedException(e);
  }

  m->setDirty();

  onlineJob unknownJob(new dummyTask());
  try {
    m->removeOnlineJob(unknownJob);
    QFAIL("Expected exception missing");
  } catch (const MyMoneyException &) {
    QVERIFY(m->dirty() == false);
  }
}

void MyMoneyDatabaseMgrTest::testHighestNumberFromIdString()
{
  testAttachDb();

  if (!m_canOpen)
    QSKIP("Database test skipped because no database could be opened.", SkipAll);

  testAddTransactions();

  QCOMPARE(m->m_sql->highestNumberFromIdString(QLatin1String("kmmTransactions"), QLatin1String("id"), 1), 2ul);
  QCOMPARE(m->m_sql->highestNumberFromIdString(QLatin1String("kmmAccounts"), QLatin1String("id"), 1), 6ul);
}
