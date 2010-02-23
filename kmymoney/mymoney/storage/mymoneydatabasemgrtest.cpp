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

#include "mymoneydatabasemgrtest.h"
#include <pwd.h>
#include <iostream>

MyMoneyDatabaseMgrTest::MyMoneyDatabaseMgrTest()
    : m_dbAttached(false),
    m_canOpen(true)
{}

void MyMoneyDatabaseMgrTest::setUp()
{
  m = new MyMoneyDatabaseMgr;
}

void MyMoneyDatabaseMgrTest::tearDown()
{
  if (m_canOpen) {
    // All transactions should have already been committed.
    //m->commitTransaction();
  }
  if (MyMoneyFile::instance()->storageAttached()) {
    MyMoneyFile::instance()->detachStorage(m);
  }
  delete m;
}

void MyMoneyDatabaseMgrTest::testEmptyConstructor()
{
  MyMoneyPayee user = m->user();

  CPPUNIT_ASSERT(user.name().isEmpty());
  CPPUNIT_ASSERT(user.address().isEmpty());
  CPPUNIT_ASSERT(user.city().isEmpty());
  CPPUNIT_ASSERT(user.state().isEmpty());
  CPPUNIT_ASSERT(user.postcode().isEmpty());
  CPPUNIT_ASSERT(user.telephone().isEmpty());
  CPPUNIT_ASSERT(user.email().isEmpty());
  CPPUNIT_ASSERT(m->nextInstitutionID() == 0);
  CPPUNIT_ASSERT(m->nextAccountID() == 0);
  CPPUNIT_ASSERT(m->nextTransactionID() == 0);
  CPPUNIT_ASSERT(m->nextPayeeID() == 0);
  CPPUNIT_ASSERT(m->nextScheduleID() == 0);
  CPPUNIT_ASSERT(m->nextReportID() == 0);
  CPPUNIT_ASSERT(m->institutionList().count() == 0);

  QList<MyMoneyAccount> accList;
  m->accountList(accList);
  CPPUNIT_ASSERT(accList.count() == 0);

  MyMoneyTransactionFilter f;
  CPPUNIT_ASSERT(m->transactionList(f).count() == 0);

  CPPUNIT_ASSERT(m->payeeList().count() == 0);
  CPPUNIT_ASSERT(m->scheduleList().count() == 0);

  CPPUNIT_ASSERT(m->m_creationDate == QDate::currentDate());
}

void MyMoneyDatabaseMgrTest::testCreateDb()
{

  // Fetch the list of available drivers
  QStringList list = QSqlDatabase::drivers();
  QStringList::Iterator it = list.begin();

  if (it == list.end()) {
    m_canOpen = false;
  } else {
    struct passwd * pwd = getpwuid(geteuid());
    QString userName;
    if (pwd != 0) {
      userName = QString(pwd->pw_name);
    }

    m_url = "sql://" + userName + "@localhost/kmm_test_driver?driver="
            //"QPSQL&mode=single";
            "QSQLITE&mode=single";
            //"QMYSQL&mode=single";
    KSharedPtr <MyMoneyStorageSql> sql = m->connectToDatabase(m_url);
    CPPUNIT_ASSERT(0 != sql);
    //qDebug("Database driver is %s", qPrintable(sql->driverName()));
    // Clear the database, so there is a fresh start on each run.
    if (0 == sql->open(m_url, QIODevice::WriteOnly, true)) {
      MyMoneyFile::instance()->attachStorage(m);
      CPPUNIT_ASSERT(sql->writeFile());
      CPPUNIT_ASSERT(0 == sql->upgradeDb());
    } else {
      m_canOpen = false;
    }
  }
}

void MyMoneyDatabaseMgrTest::testAttachDb()
{
  if (!m_dbAttached) {
    testCreateDb();
    if (m_canOpen) {
      MyMoneyFile::instance()->detachStorage();
      KSharedPtr <MyMoneyStorageSql> sql = m->connectToDatabase(m_url);
      CPPUNIT_ASSERT(sql);
      int openStatus = sql->open(m_url, QIODevice::ReadWrite);
      CPPUNIT_ASSERT(0 == openStatus);
      MyMoneyFile::instance()->attachStorage(m);
      m_dbAttached = true;
    }
  }
}

void MyMoneyDatabaseMgrTest::testSetFunctions()
{
  testAttachDb();
  if (!m_canOpen) {
    std::cout << "Database test skipped because no database could be opened." << std::endl;
    return;
  }

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
  CPPUNIT_ASSERT(user.name() == "Name");
  CPPUNIT_ASSERT(user.address() == "Street");
  CPPUNIT_ASSERT(user.city() == "Town");
  CPPUNIT_ASSERT(user.state() == "County");
  CPPUNIT_ASSERT(user.postcode() == "Postcode");
  CPPUNIT_ASSERT(user.telephone() == "Telephone");
  CPPUNIT_ASSERT(user.email() == "Email");
  CPPUNIT_ASSERT(m->value("key") == "value");

  m->setDirty();
  m->deletePair("key");
  CPPUNIT_ASSERT(m->dirty() == false);
}

void MyMoneyDatabaseMgrTest::testSupportFunctions()
{
  testAttachDb();
  if (!m_canOpen) {
    std::cout << "Database test skipped because no database could be opened." << std::endl;
    return;
  }

  try {
    CPPUNIT_ASSERT(m->nextInstitutionID() == "I000001");
    CPPUNIT_ASSERT(m->nextAccountID() == "A000001");
    CPPUNIT_ASSERT(m->nextTransactionID() == "T000000000000000001");
    CPPUNIT_ASSERT(m->nextPayeeID() == "P000001");
    CPPUNIT_ASSERT(m->nextScheduleID() == "SCH000001");
    CPPUNIT_ASSERT(m->nextReportID() == "R000001");

    CPPUNIT_ASSERT(m->liability().name() == "Liability");
    CPPUNIT_ASSERT(m->asset().name() == "Asset");
    CPPUNIT_ASSERT(m->expense().name() == "Expense");
    CPPUNIT_ASSERT(m->income().name() == "Income");
    CPPUNIT_ASSERT(m->equity().name() == "Equity");
    CPPUNIT_ASSERT(m->dirty() == false);
  } catch (MyMoneyException* e) {
    unexpectedException(e);
  }
}

void MyMoneyDatabaseMgrTest::testIsStandardAccount()
{
  testAttachDb();
  if (!m_canOpen) {
    std::cout << "Database test skipped because no database could be opened." << std::endl;
    return;
  }

  CPPUNIT_ASSERT(m->isStandardAccount(STD_ACC_LIABILITY) == true);
  CPPUNIT_ASSERT(m->isStandardAccount(STD_ACC_ASSET) == true);
  CPPUNIT_ASSERT(m->isStandardAccount(STD_ACC_EXPENSE) == true);
  CPPUNIT_ASSERT(m->isStandardAccount(STD_ACC_INCOME) == true);
  CPPUNIT_ASSERT(m->isStandardAccount(STD_ACC_EQUITY) == true);
  CPPUNIT_ASSERT(m->isStandardAccount("A0002") == false);
}

void MyMoneyDatabaseMgrTest::testNewAccount()
{
  testAttachDb();
  if (!m_canOpen) {
    std::cout << "Database test skipped because no database could be opened." << std::endl;
    return;
  }

  MyMoneyAccount a;

  a.setName("AccountName");
  a.setNumber("AccountNumber");
  a.setValue("Key", "Value");

  m->addAccount(a);

  CPPUNIT_ASSERT(m->accountId() == 1);
  QList<MyMoneyAccount> accList;
  m->accountList(accList);
  CPPUNIT_ASSERT(accList.count() == 1);
  CPPUNIT_ASSERT((*(accList.begin())).name() == "AccountName");
  CPPUNIT_ASSERT((*(accList.begin())).id() == "A000001");
  CPPUNIT_ASSERT((*(accList.begin())).value("Key") == "Value");
}

void MyMoneyDatabaseMgrTest::testAccount()
{
  testNewAccount();
  if (!m_canOpen) {
    std::cout << "Database test skipped because no database could be opened." << std::endl;
    return;
  }

  m->setDirty();

  MyMoneyAccount a;

  // make sure that an invalid ID causes an exception
  try {
    a = m->account("Unknown ID");
    CPPUNIT_FAIL("Exception expected");
  } catch (MyMoneyException *e) {
    delete e;
  }
  CPPUNIT_ASSERT(m->dirty() == false);

  // now make sure, that a real ID works
  try {
    a = m->account("A000001");
    CPPUNIT_ASSERT(a.name() == "AccountName");
    CPPUNIT_ASSERT(a.id() == "A000001");
  } catch (MyMoneyException *e) {
    unexpectedException(e);
  }
}

void MyMoneyDatabaseMgrTest::testAddNewAccount()
{
  testNewAccount();
  if (!m_canOpen) {
    std::cout << "Database test skipped because no database could be opened." << std::endl;
    return;
  }

  MyMoneyAccount a, b;
  b.setName("Account2");
  b.setNumber("Acc2");
  m->addAccount(b);

  m->setDirty();

  CPPUNIT_ASSERT(m->accountId() == 2);
  QList<MyMoneyAccount> accList;
  m->accountList(accList);
  CPPUNIT_ASSERT(accList.count() == 2);

  // try to add account to undefined account
  try {
    MyMoneyAccount c("UnknownID", b);
    m->addAccount(c, a);
    CPPUNIT_FAIL("Exception expected");
  } catch (MyMoneyException *e) {
    delete e;
  }

  CPPUNIT_ASSERT(m->dirty() == false);
  // now try to add account 1 as sub-account to account 2
  try {
    a = m->account("A000001");
    CPPUNIT_ASSERT(m->asset().accountList().count() == 0);
    m->addAccount(b, a);
    MyMoneyAccount acc(m->account("A000002"));
    CPPUNIT_ASSERT(acc.accountList()[0] == "A000001");
    CPPUNIT_ASSERT(acc.accountList().count() == 1);
    CPPUNIT_ASSERT(m->asset().accountList().count() == 0);
  } catch (MyMoneyException *e) {
    unexpectedException(e);
  }
}

void MyMoneyDatabaseMgrTest::testAddInstitution()
{
  testAttachDb();

  if (!m_canOpen) {
    std::cout << "Database test skipped because no database could be opened." << std::endl;
    return;
  }

  MyMoneyInstitution i;

  i.setName("Inst Name");

  m->addInstitution(i);
  CPPUNIT_ASSERT(m->institutionList().count() == 1);
  CPPUNIT_ASSERT(m->institutionId() == 1);
  CPPUNIT_ASSERT((*(m->institutionList().begin())).name() == "Inst Name");
  CPPUNIT_ASSERT((*(m->institutionList().begin())).id() == "I000001");
}

void MyMoneyDatabaseMgrTest::testInstitution()
{
  testAddInstitution();
  if (!m_canOpen) {
    std::cout << "Database test skipped because no database could be opened." << std::endl;
    return;
  }

  MyMoneyInstitution i;

  m->setDirty();

  // try to find unknown institution
  try {
    i = m->institution("Unknown ID");
    CPPUNIT_FAIL("Exception expected");
  } catch (MyMoneyException *e) {
    delete e;
  }

  CPPUNIT_ASSERT(m->dirty() == false);

  // now try to find real institution
  try {
    i = m->institution("I000001");
    CPPUNIT_ASSERT(i.name() == "Inst Name");
    CPPUNIT_ASSERT(m->dirty() == false);
  } catch (MyMoneyException *e) {
    unexpectedException(e);
  }
}

void MyMoneyDatabaseMgrTest::testAccount2Institution()
{
  testAttachDb();

  if (!m_canOpen) {
    std::cout << "Database test skipped because no database could be opened." << std::endl;
    return;
  }

  testAddInstitution();
  testAddNewAccount();

  MyMoneyInstitution i;
  MyMoneyAccount a, b;

  try {
    i = m->institution("I000001");
    a = m->account("A000001");
  } catch (MyMoneyException *e) {
    unexpectedException(e);
  }

  m->setDirty();

  // try to add to a false institution
  MyMoneyInstitution fake("Unknown ID", i);
  a.setInstitutionId(fake.id());
  try {
    m->modifyAccount(a);
    CPPUNIT_FAIL("Exception expected");
  } catch (MyMoneyException *e) {
    delete e;
  }

  CPPUNIT_ASSERT(m->dirty() == false);
  // now try to do it with a real institution
  try {
    CPPUNIT_ASSERT(i.accountList().count() == 0);
    a.setInstitutionId(i.id());
    m->modifyAccount(a);
    CPPUNIT_ASSERT(a.institutionId() == i.id());
    b = m->account("A000001");
    CPPUNIT_ASSERT(b.institutionId() == i.id());
    CPPUNIT_ASSERT(i.accountList().count() == 0);
  } catch (MyMoneyException *e) {
    unexpectedException(e);
  }
}

void MyMoneyDatabaseMgrTest::testModifyAccount()
{
  testAttachDb();

  if (!m_canOpen) {
    std::cout << "Database test skipped because no database could be opened." << std::endl;
    return;
  }

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
    CPPUNIT_ASSERT(b.parentAccountId() == a.parentAccountId());
    CPPUNIT_ASSERT(b.name() == "New account name");
    CPPUNIT_ASSERT(b.value("Key") == "Value");
  } catch (MyMoneyException *e) {
    unexpectedException(e);
  }

  // modify institution to unknown id
  MyMoneyAccount c("Unknown ID", a);
  m->setDirty();
  try {
    m->modifyAccount(c);
    CPPUNIT_FAIL("Exception expected");
  } catch (MyMoneyException *e) {
    delete e;
  }

  // use different account type
  MyMoneyAccount d;
  d.setAccountType(MyMoneyAccount::CreditCard);
  MyMoneyAccount f("A000001", d);
  try {
    m->modifyAccount(f);
    CPPUNIT_FAIL("Exception expected");
  } catch (MyMoneyException *e) {
    delete e;
  }

  // use different parent
  a.setParentAccountId("A000002");
  try {
    m->modifyAccount(c);
    CPPUNIT_FAIL("Exception expected");
  } catch (MyMoneyException *e) {
    delete e;
  }
}

void MyMoneyDatabaseMgrTest::testModifyInstitution()
{
  testAddInstitution();
  if (!m_canOpen) {
    std::cout << "Database test skipped because no database could be opened." << std::endl;
    return;
  }

  MyMoneyInstitution i = m->institution("I000001");

  m->setDirty();
  i.setName("New inst name");
  try {
    m->modifyInstitution(i);
    i = m->institution("I000001");
    CPPUNIT_ASSERT(i.name() == "New inst name");
  } catch (MyMoneyException *e) {
    unexpectedException(e);
  }

  // try to modify an institution that does not exist
  MyMoneyInstitution f("Unknown ID", i);
  try {
    m->modifyInstitution(f);
    CPPUNIT_FAIL("Exception expected");
  } catch (MyMoneyException *e) {
    delete e;
  }
}

void MyMoneyDatabaseMgrTest::testReparentAccount()
{
  testAttachDb();

  if (!m_canOpen) {
    std::cout << "Database test skipped because no database could be opened." << std::endl;
    return;
  }

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
  ch.setValue("Key", "Value");

  try {
    m->addAccount(ex1);
    m->addAccount(ex2);
    m->addAccount(ex3);
    m->addAccount(ex4);
    m->addAccount(in);
    m->addAccount(ch);

    CPPUNIT_ASSERT(ex1.id() == "A000001");
    CPPUNIT_ASSERT(ex2.id() == "A000002");
    CPPUNIT_ASSERT(ex3.id() == "A000003");
    CPPUNIT_ASSERT(ex4.id() == "A000004");
    CPPUNIT_ASSERT(in.id() == "A000005");
    CPPUNIT_ASSERT(ch.id() == "A000006");
    CPPUNIT_ASSERT(ch.value("Key") == "Value");

    MyMoneyAccount parent = m->expense();

    m->addAccount(parent, ex1);
    m->addAccount(ex1, ex2);
    m->addAccount(parent, ex3);
    m->addAccount(parent, ex4);

    parent = m->income();
    m->addAccount(parent, in);

    parent = m->asset();
    m->addAccount(parent, ch);
    CPPUNIT_ASSERT(ch.value("Key") == "Value");

    MyMoneyFile::instance()->preloadCache();
    CPPUNIT_ASSERT(m->expense().accountCount() == 3);
    CPPUNIT_ASSERT(m->account(ex1.id()).accountCount() == 1);
    CPPUNIT_ASSERT(ex3.parentAccountId() == STD_ACC_EXPENSE);

    //for (int i = 0; i < 100; ++i) {
    m->reparentAccount(ex3, ex1);
    //}
    MyMoneyFile::instance()->preloadCache();
    CPPUNIT_ASSERT(m->expense().accountCount() == 2);
    CPPUNIT_ASSERT(m->account(ex1.id()).accountCount() == 2);
    CPPUNIT_ASSERT(ex3.parentAccountId() == ex1.id());
  } catch (MyMoneyException *e) {
    unexpectedException(e);
  }
}

void MyMoneyDatabaseMgrTest::testAddTransactions()
{
  testAttachDb();

  if (!m_canOpen) {
    std::cout << "Database test skipped because no database could be opened." << std::endl;
    return;
  }

  testReparentAccount();

  MyMoneyAccount ch;
  MyMoneyTransaction t1, t2;
  MyMoneySplit s;

  try {
    // I made some money, great
    s.setAccountId("A000006");  // Checkings
    s.setShares(MyMoneyMoney(100000));
    s.setValue(MyMoneyMoney(100000));
    CPPUNIT_ASSERT(s.id().isEmpty());
    t1.addSplit(s);

    s.setId(QString());  // enable re-usage of split variable
    s.setAccountId("A000005");  // Salary
    s.setShares(MyMoneyMoney(-100000));
    s.setValue(MyMoneyMoney(-100000));
    CPPUNIT_ASSERT(s.id().isEmpty());
    t1.addSplit(s);

    t1.setPostDate(QDate(2002, 5, 10));
  } catch (MyMoneyException *e) {
    unexpectedException(e);
  }

  m->setDirty();
  try {
    ch = m->account("A000006");
    CPPUNIT_ASSERT(ch.value("Key") == "Value");
    m->addTransaction(t1);
    CPPUNIT_ASSERT(t1.id() == "T000000000000000001");
    CPPUNIT_ASSERT(t1.splitCount() == 2);
    CPPUNIT_ASSERT(m->transactionCount() == 1);
    ch = m->account("A000006");
    CPPUNIT_ASSERT(ch.value("Key") == "Value");
  } catch (MyMoneyException *e) {
    unexpectedException(e);
  }

  try {
    // I spent some money, not so great
    s.setId(QString());  // enable re-usage of split variable
    s.setAccountId("A000004");  // Grosseries
    s.setShares(MyMoneyMoney(10000));
    s.setValue(MyMoneyMoney(10000));
    CPPUNIT_ASSERT(s.id().isEmpty());
    t2.addSplit(s);

    s.setId(QString());  // enable re-usage of split variable
    s.setAccountId("A000002");  // 16% sales tax
    s.setShares(MyMoneyMoney(1200));
    s.setValue(MyMoneyMoney(1200));
    CPPUNIT_ASSERT(s.id().isEmpty());
    t2.addSplit(s);

    s.setId(QString());  // enable re-usage of split variable
    s.setAccountId("A000003");  // 7% sales tax
    s.setShares(MyMoneyMoney(400));
    s.setValue(MyMoneyMoney(400));
    CPPUNIT_ASSERT(s.id().isEmpty());
    t2.addSplit(s);

    s.setId(QString());  // enable re-usage of split variable
    s.setAccountId("A000006");  // Checkings account
    s.setShares(MyMoneyMoney(-11600));
    s.setValue(MyMoneyMoney(-11600));
    CPPUNIT_ASSERT(s.id().isEmpty());
    t2.addSplit(s);

    t2.setPostDate(QDate(2002, 5, 9));
  } catch (MyMoneyException *e) {
    unexpectedException(e);
  }
  m->setDirty();
  try {
    m->addTransaction(t2);
    CPPUNIT_ASSERT(t2.id() == "T000000000000000002");
    CPPUNIT_ASSERT(t2.splitCount() == 4);
    CPPUNIT_ASSERT(m->transactionCount() == 2);

    //QMap<QString, QString>::ConstIterator it_k;
    MyMoneyTransactionFilter f;
    QList<MyMoneyTransaction> transactionList(m->transactionList(f));
    QList<MyMoneyTransaction>::ConstIterator it_t(transactionList.constBegin());

    //CPPUNIT_ASSERT((*it_k) == "2002-05-10-T000000000000000001");
    CPPUNIT_ASSERT((*it_t).id() == "T000000000000000002");

    //++it_k;
    ++it_t;
    //CPPUNIT_ASSERT((*it_k) == "2002-05-09-T000000000000000002");
    CPPUNIT_ASSERT((*it_t).id() == "T000000000000000001");
    //++it_k;
    ++it_t;
    //CPPUNIT_ASSERT(it_k == m->m_transactionKeys.end());
    CPPUNIT_ASSERT(it_t == transactionList.constEnd());

    ch = m->account("A000006");
    CPPUNIT_ASSERT(ch.value("Key") == "Value");

    // check that the account's transaction list is updated
    QList<MyMoneyTransaction> list;
    MyMoneyTransactionFilter filter("A000006");
    list = m->transactionList(filter);
    CPPUNIT_ASSERT(list.size() == 2);

    QList<MyMoneyTransaction>::ConstIterator it;
    it = list.constBegin();
    //CPPUNIT_ASSERT((*it).id() == "T000000000000000002");
    CPPUNIT_ASSERT((*it) == t2);
    ++it;

    CPPUNIT_ASSERT((*it) == t1);
    ++it;
    CPPUNIT_ASSERT(it == list.constEnd());

  } catch (MyMoneyException *e) {
    unexpectedException(e);
  }
}

void MyMoneyDatabaseMgrTest::testTransactionCount()
{
  testAttachDb();

  if (!m_canOpen) {
    std::cout << "Database test skipped because no database could be opened." << std::endl;
    return;
  }

  testAddTransactions();
  CPPUNIT_ASSERT(m->transactionCount("A000001") == 0);
  CPPUNIT_ASSERT(m->transactionCount("A000002") == 1);
  CPPUNIT_ASSERT(m->transactionCount("A000003") == 1);
  CPPUNIT_ASSERT(m->transactionCount("A000004") == 1);
  CPPUNIT_ASSERT(m->transactionCount("A000005") == 1);
  CPPUNIT_ASSERT(m->transactionCount("A000006") == 2);
}

void MyMoneyDatabaseMgrTest::testAddBudget()
{
  testAttachDb();

  if (!m_canOpen) {
    std::cout << "Database test skipped because no database could be opened." << std::endl;
    return;
  }

  MyMoneyBudget budget;

  budget.setName("TestBudget");
  budget.setBudgetStart(QDate::currentDate());

  m->addBudget(budget);

  CPPUNIT_ASSERT(m->budgetList().count() == 1);
  CPPUNIT_ASSERT(m->budgetId() == 1);
  MyMoneyBudget newBudget = m->budgetByName("TestBudget");

  CPPUNIT_ASSERT(budget.budgetStart() == newBudget.budgetStart());
  CPPUNIT_ASSERT(budget.name() == newBudget.name());
}

void MyMoneyDatabaseMgrTest::testCopyBudget()
{
  testAddBudget();

  if (!m_canOpen) {
    std::cout << "Database test skipped because no database could be opened." << std::endl;
    return;
  }

  try {
    MyMoneyBudget oldBudget = m->budgetByName("TestBudget");
    MyMoneyBudget newBudget = oldBudget;

    newBudget.clearId();
    newBudget.setName(QString("Copy of %1").arg(oldBudget.name()));
    m->addBudget(newBudget);

    CPPUNIT_ASSERT(m->budgetList().count() == 2);
    CPPUNIT_ASSERT(m->budgetId() == 2);

    MyMoneyBudget testBudget = m->budgetByName("TestBudget");

    CPPUNIT_ASSERT(oldBudget.budgetStart() == testBudget.budgetStart());
    CPPUNIT_ASSERT(oldBudget.name() == testBudget.name());

    testBudget = m->budgetByName("Copy of TestBudget");

    CPPUNIT_ASSERT(testBudget.budgetStart() == newBudget.budgetStart());
    CPPUNIT_ASSERT(testBudget.name() == newBudget.name());
  } catch (QString& s) {
    qDebug("Error in testCopyBudget(): %s", qPrintable(s));
    CPPUNIT_ASSERT(false);
  }
}

void MyMoneyDatabaseMgrTest::testModifyBudget()
{
  testAddBudget();

  if (!m_canOpen) {
    std::cout << "Database test skipped because no database could be opened." << std::endl;
    return;
  }

  MyMoneyBudget budget = m->budgetByName("TestBudget");

  budget.setBudgetStart(QDate::currentDate().addDays(-1));

  m->modifyBudget(budget);

  MyMoneyBudget newBudget = m->budgetByName("TestBudget");

  CPPUNIT_ASSERT(budget.id() == newBudget.id());
  CPPUNIT_ASSERT(budget.budgetStart() == newBudget.budgetStart());
  CPPUNIT_ASSERT(budget.name() == newBudget.name());
}

void MyMoneyDatabaseMgrTest::testRemoveBudget()
{
  testAddBudget();

  if (!m_canOpen) {
    std::cout << "Database test skipped because no database could be opened." << std::endl;
    return;
  }

  MyMoneyBudget budget = m->budgetByName("TestBudget");
  m->removeBudget(budget);

  try {
    budget = m->budgetByName("TestBudget");
    // exception should be thrown if budget not found.
    CPPUNIT_ASSERT(false);
  } catch (MyMoneyException *e) {
    delete e;
    CPPUNIT_ASSERT(true);
  }
}

void MyMoneyDatabaseMgrTest::testBalance()
{
  testAttachDb();

  if (!m_canOpen) {
    std::cout << "Database test skipped because no database could be opened." << std::endl;
    return;
  }

  testAddTransactions();

  try {
    CPPUNIT_ASSERT(m->balance("A000001", QDate()).isZero());
    CPPUNIT_ASSERT(m->balance("A000002", QDate()) == MyMoneyMoney(1200));
    CPPUNIT_ASSERT(m->balance("A000003", QDate()) == MyMoneyMoney(400));
    //Add a transaction to zero account A000003
    MyMoneyTransaction t1;
    MyMoneySplit s;

    s.setAccountId("A000003");
    s.setShares(MyMoneyMoney(-400));
    s.setValue(MyMoneyMoney(-400));
    CPPUNIT_ASSERT(s.id().isEmpty());
    t1.addSplit(s);

    s.setId(QString());  // enable re-usage of split variable
    s.setAccountId("A000002");
    s.setShares(MyMoneyMoney(400));
    s.setValue(MyMoneyMoney(400));
    CPPUNIT_ASSERT(s.id().isEmpty());
    t1.addSplit(s);

    t1.setPostDate(QDate(2007, 5, 10));

    m->addTransaction(t1);

    //qDebug ("Balance of A000003 is 0 = %s", m->balance("A000003", QDate()).toString().ascii());
    CPPUNIT_ASSERT(m->balance("A000003", QDate()).isZero());

    //qDebug ("Balance of A000001 is 1600 = %s", m->balance("A000001", QDate()).toString().ascii());
    CPPUNIT_ASSERT(m->totalBalance("A000001", QDate()) == MyMoneyMoney(1600));

    //qDebug ("Balance of A000006 is -11600 = %s", m->balance("A000006", QDate(2002,5,9)).toString().ascii());
    CPPUNIT_ASSERT(m->balance("A000006", QDate(2002, 5, 9)) == MyMoneyMoney(-11600));

    //qDebug ("Balance of A000005 is -100000 = %s", m->balance("A000005", QDate(2002,5,10)).toString().ascii());
    CPPUNIT_ASSERT(m->balance("A000005", QDate(2002, 5, 10)) == MyMoneyMoney(-100000));

    //qDebug ("Balance of A000006 is 88400 = %s", m->balance("A000006", QDate(2002,5,10)).toString().ascii());
    CPPUNIT_ASSERT(m->balance("A000006", QDate(2002, 5, 10)) == MyMoneyMoney(88400));
  } catch (MyMoneyException* e) {
    unexpectedException(e);
  }
}

void MyMoneyDatabaseMgrTest::testModifyTransaction()
{
  testAttachDb();

  if (!m_canOpen) {
    std::cout << "Database test skipped because no database could be opened." << std::endl;
    return;
  }

  testAddTransactions();

  MyMoneyTransaction t = m->transaction("T000000000000000002");
  MyMoneySplit s;
  MyMoneyAccount ch;

  // just modify simple stuff (splits)
  CPPUNIT_ASSERT(t.splitCount() == 4);

  ch = m->account("A000006");
  CPPUNIT_ASSERT(ch.value("Key") == "Value");

  s = t.splits()[0];
  s.setShares(MyMoneyMoney(11000));
  s.setValue(MyMoneyMoney(11000));
  t.modifySplit(s);

  CPPUNIT_ASSERT(t.splitCount() == 4);
  s = t.splits()[3];
  s.setShares(MyMoneyMoney(-12600));
  s.setValue(MyMoneyMoney(-12600));
  t.modifySplit(s);
  ch = m->account("A000006");
  CPPUNIT_ASSERT(ch.value("Key") == "Value");

  try {
    CPPUNIT_ASSERT(m->balance("A000004", QDate()) == MyMoneyMoney(10000));
    CPPUNIT_ASSERT(m->balance("A000006", QDate()) == MyMoneyMoney(100000 - 11600));
    CPPUNIT_ASSERT(m->totalBalance("A000001", QDate()) == MyMoneyMoney(1600));
    m->modifyTransaction(t);
    ch = m->account("A000006");
    CPPUNIT_ASSERT(ch.value("Key") == "Value");
    CPPUNIT_ASSERT(m->balance("A000004", QDate()) == MyMoneyMoney(11000));
    CPPUNIT_ASSERT(m->balance("A000006", QDate()) == MyMoneyMoney(100000 - 12600));
    CPPUNIT_ASSERT(m->totalBalance("A000001", QDate()) == MyMoneyMoney(1600));
  } catch (MyMoneyException *e) {
    unexpectedException(e);
  }

  // now modify the date
  t.setPostDate(QDate(2002, 5, 11));
  try {
    ch = m->account("A000006");
    CPPUNIT_ASSERT(ch.value("Key") == "Value");
    m->modifyTransaction(t);
    CPPUNIT_ASSERT(m->balance("A000004", QDate()) == MyMoneyMoney(11000));
    CPPUNIT_ASSERT(m->balance("A000006", QDate()) == MyMoneyMoney(100000 - 12600));
    CPPUNIT_ASSERT(m->totalBalance("A000001", QDate()) == MyMoneyMoney(1600));

    //QMap<QString, QString>::ConstIterator it_k;
    MyMoneyTransactionFilter f;
    QList<MyMoneyTransaction> transactionList(m->transactionList(f));
    QList<MyMoneyTransaction>::ConstIterator it_t(transactionList.constBegin());
    //it_k = m->m_transactionKeys.begin();
    //CPPUNIT_ASSERT((*it_k) == "2002-05-10-T000000000000000001");
    CPPUNIT_ASSERT((*it_t).id() == "T000000000000000001");
    //++it_k;
    ++it_t;
    //CPPUNIT_ASSERT((*it_k) == "2002-05-11-T000000000000000002");
    CPPUNIT_ASSERT((*it_t).id() == "T000000000000000002");
    //++it_k;
    ++it_t;
    //CPPUNIT_ASSERT(it_k == m->m_transactionKeys.end());
    CPPUNIT_ASSERT(it_t == transactionList.constEnd());

    ch = m->account("A000006");
    CPPUNIT_ASSERT(ch.value("Key") == "Value");

    // check that the account's transaction list is updated
    QList<MyMoneyTransaction> list;
    MyMoneyTransactionFilter filter("A000006");
    list = m->transactionList(filter);
    CPPUNIT_ASSERT(list.size() == 2);

    QList<MyMoneyTransaction>::ConstIterator it;
    it = list.constBegin();
    CPPUNIT_ASSERT((*it).id() == "T000000000000000001");
    ++it;
    CPPUNIT_ASSERT((*it).id() == "T000000000000000002");
    ++it;
    CPPUNIT_ASSERT(it == list.constEnd());

  } catch (MyMoneyException *e) {
    unexpectedException(e);
  }
}


void MyMoneyDatabaseMgrTest::testRemoveUnusedAccount()
{
  testAttachDb();

  if (!m_canOpen) {
    std::cout << "Database test skipped because no database could be opened." << std::endl;
    return;
  }

  testAccount2Institution();

  MyMoneyAccount a = m->account("A000001");
  MyMoneyInstitution i = m->institution("I000001");

  m->setDirty();
  // make sure, we cannot remove the standard account groups
  try {
    m->removeAccount(m->liability());
    CPPUNIT_FAIL("Exception expected");
  } catch (MyMoneyException *e) {
    delete e;
  }

  try {
    m->removeAccount(m->asset());
    CPPUNIT_FAIL("Exception expected");
  } catch (MyMoneyException *e) {
    delete e;
  }

  try {
    m->removeAccount(m->expense());
    CPPUNIT_FAIL("Exception expected");
  } catch (MyMoneyException *e) {
    delete e;
  }

  try {
    m->removeAccount(m->income());
    CPPUNIT_FAIL("Exception expected");
  } catch (MyMoneyException *e) {
    delete e;
  }

  // try to remove the account still attached to the institution
  try {
    m->removeAccount(a);
    CPPUNIT_FAIL("Exception expected");
  } catch (MyMoneyException *e) {
    delete e;
  }

  // now really remove an account

  try {
    MyMoneyFile::instance()->preloadCache();
    i = m->institution("I000001");

    //CPPUNIT_ASSERT(i.accountCount() == 0);
    CPPUNIT_ASSERT(i.accountCount() == 1);
    CPPUNIT_ASSERT(m->accountCount() == 7);

    a.setInstitutionId(QString());
    m->modifyAccount(a);
    m->removeAccount(a);
    CPPUNIT_ASSERT(m->accountCount() == 6);
    i = m->institution("I000001");
    CPPUNIT_ASSERT(i.accountCount() == 0);
  } catch (MyMoneyException *e) {
    unexpectedException(e);
  }
}

void MyMoneyDatabaseMgrTest::testRemoveUsedAccount()
{
  testAttachDb();

  if (!m_canOpen) {
    std::cout << "Database test skipped because no database could be opened." << std::endl;
    return;
  }

  testAddTransactions();

  MyMoneyAccount a = m->account("A000006");

  try {
    m->removeAccount(a);
    CPPUNIT_FAIL("Exception expected");
  } catch (MyMoneyException *e) {
    delete e;
  }
}

void MyMoneyDatabaseMgrTest::testRemoveInstitution()
{
  testAttachDb();

  if (!m_canOpen) {
    std::cout << "Database test skipped because no database could be opened." << std::endl;
    return;
  }

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
    CPPUNIT_ASSERT(i.accountCount() == 0);
  } catch (MyMoneyException *e) {
    unexpectedException(e);
  }

  m->setDirty();
  // now remove the institution and see if the account survived ;-)
  try {
    m->removeInstitution(i);
    a.setInstitutionId(QString());
    m->modifyAccount(a);
    a = m->account("A000006");
    CPPUNIT_ASSERT(a.institutionId().isEmpty());
    CPPUNIT_ASSERT(m->institutionCount() == 0);
  } catch (MyMoneyException *e) {
    unexpectedException(e);
  }
}

void MyMoneyDatabaseMgrTest::testRemoveTransaction()
{
  testAttachDb();

  if (!m_canOpen) {
    std::cout << "Database test skipped because no database could be opened." << std::endl;
    return;
  }

  testAddTransactions();

  MyMoneyTransaction t = m->transaction("T000000000000000002");

  m->setDirty();
  try {
    m->removeTransaction(t);
    CPPUNIT_ASSERT(m->transactionCount() == 1);
  } catch (MyMoneyException *e) {
    unexpectedException(e);
  }
}

void MyMoneyDatabaseMgrTest::testTransactionList()
{
  testAttachDb();

  if (!m_canOpen) {
    std::cout << "Database test skipped because no database could be opened." << std::endl;
    return;
  }

  testAddTransactions();

  QList<MyMoneyTransaction> list;
  MyMoneyTransactionFilter filter("A000006");
  list = m->transactionList(filter);
  CPPUNIT_ASSERT(list.count() == 2);
  CPPUNIT_ASSERT(list.at(0).id() == "T000000000000000002");
  CPPUNIT_ASSERT(list.at(1).id() == "T000000000000000001");

  filter.clear();
  filter.addAccount(QString("A000003"));
  list = m->transactionList(filter);
  CPPUNIT_ASSERT(list.count() == 1);
  CPPUNIT_ASSERT(list.at(0).id() == "T000000000000000002");

  filter.clear();
  list = m->transactionList(filter);
  CPPUNIT_ASSERT(list.count() == 2);
  CPPUNIT_ASSERT(list.at(0).id() == "T000000000000000002");
  CPPUNIT_ASSERT(list.at(1).id() == "T000000000000000001");
}

void MyMoneyDatabaseMgrTest::testAddPayee()
{
  testAttachDb();

  if (!m_canOpen) {
    std::cout << "Database test skipped because no database could be opened." << std::endl;
    return;
  }

  MyMoneyPayee p;

  p.setName("THB");
  m->setDirty();
  try {
    CPPUNIT_ASSERT(m->payeeId() == 0);
    m->addPayee(p);
    CPPUNIT_ASSERT(m->payeeId() == 1);
    MyMoneyPayee p1 = m->payeeByName("THB");
    CPPUNIT_ASSERT(p.id() == p1.id());
    CPPUNIT_ASSERT(p.name() == p1.name());
    CPPUNIT_ASSERT(p.address() == p1.address());
    CPPUNIT_ASSERT(p.city() == p1.city());
    CPPUNIT_ASSERT(p.state() == p1.state());
    CPPUNIT_ASSERT(p.postcode() == p1.postcode());
    CPPUNIT_ASSERT(p.telephone() == p1.telephone());
    CPPUNIT_ASSERT(p.email() == p1.email());
    MyMoneyPayee::payeeMatchType m, m1;
    bool ignore, ignore1;
    QStringList keys, keys1;
    m = p.matchData(ignore, keys);
    m1 = p1.matchData(ignore1, keys1);
    CPPUNIT_ASSERT(m == m1);
    CPPUNIT_ASSERT(ignore == ignore1);
    CPPUNIT_ASSERT(keys == keys1);
    CPPUNIT_ASSERT(p.reference() == p1.reference());
    CPPUNIT_ASSERT(p.defaultAccountEnabled() == p1.defaultAccountEnabled());
    CPPUNIT_ASSERT(p.defaultAccountId() == p1.defaultAccountId());

    CPPUNIT_ASSERT(p == p1);
  } catch (MyMoneyException *e) {
    unexpectedException(e);
  }

}

void MyMoneyDatabaseMgrTest::testSetAccountName()
{
  testAttachDb();

  if (!m_canOpen) {
    std::cout << "Database test skipped because no database could be opened." << std::endl;
    return;
  }

  try {
    m->setAccountName(STD_ACC_LIABILITY, "Verbindlichkeiten");
  } catch (MyMoneyException *e) {
    unexpectedException(e);
  }
  try {
    m->setAccountName(STD_ACC_ASSET, "Verm�gen");
  } catch (MyMoneyException *e) {
    unexpectedException(e);
  }
  try {
    m->setAccountName(STD_ACC_EXPENSE, "Ausgaben");
  } catch (MyMoneyException *e) {
    unexpectedException(e);
  }
  try {
    m->setAccountName(STD_ACC_INCOME, "Einnahmen");
  } catch (MyMoneyException *e) {
    unexpectedException(e);
  }

  MyMoneyFile::instance()->preloadCache();

  try {
    CPPUNIT_ASSERT(m->liability().name() == "Verbindlichkeiten");
    CPPUNIT_ASSERT(m->asset().name() == "Verm�gen");
    CPPUNIT_ASSERT(m->expense().name() == "Ausgaben");
    CPPUNIT_ASSERT(m->income().name() == "Einnahmen");
  } catch (MyMoneyException* e) {
    unexpectedException(e);
  }

  try {
    m->setAccountName("A000001", "New account name");
    CPPUNIT_FAIL("Exception expected");
  } catch (MyMoneyException *e) {
    delete e;
  }
}

void MyMoneyDatabaseMgrTest::testModifyPayee()
{
  testAttachDb();

  if (!m_canOpen) {
    std::cout << "Database test skipped because no database could be opened." << std::endl;
    return;
  }

  MyMoneyPayee p;

  testAddPayee();

  p = m->payee("P000001");
  p.setName("New name");
  m->setDirty();
  try {
    m->modifyPayee(p);
    p = m->payee("P000001");
    CPPUNIT_ASSERT(p.name() == "New name");
  } catch (MyMoneyException *e) {
    unexpectedException(e);
  }
}

void MyMoneyDatabaseMgrTest::testRemovePayee()
{
  testAttachDb();

  if (!m_canOpen) {
    std::cout << "Database test skipped because no database could be opened." << std::endl;
    return;
  }

  testAddPayee();
  m->setDirty();

  // check that we can remove an unreferenced payee
  MyMoneyPayee p = m->payee("P000001");
  try {
    CPPUNIT_ASSERT(m->payeeList().count() == 1);
    m->removePayee(p);
    CPPUNIT_ASSERT(m->payeeList().count() == 0);
  } catch (MyMoneyException *e) {
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
    CPPUNIT_FAIL("Expected exception");
  } catch (MyMoneyException *e) {
    delete e;
  }

  // reset here, so that the
  // testAddPayee will not fail
  m->loadPayeeId(0);
  testAddPayee();

  // check that it works when the payee exists
  try {
    m->modifyTransaction(tr);
  } catch (MyMoneyException *e) {
    unexpectedException(e);
  }

  m->setDirty();

  // now check, that we cannot remove the payee
  try {
    m->removePayee(p);
    CPPUNIT_FAIL("Expected exception");
  } catch (MyMoneyException *e) {
    delete e;
  }
  CPPUNIT_ASSERT(m->payeeList().count() == 1);
}


void MyMoneyDatabaseMgrTest::testRemoveAccountFromTree()
{
  testAttachDb();

  if (!m_canOpen) {
    std::cout << "Database test skipped because no database could be opened." << std::endl;
    return;
  }

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

    CPPUNIT_ASSERT(a.accountList().count() == 1);
    CPPUNIT_ASSERT(m->account(a.accountList()[0]).name() == "Acc B");

    CPPUNIT_ASSERT(b.accountList().count() == 1);
    CPPUNIT_ASSERT(m->account(b.accountList()[0]).name() == "Acc C");

    CPPUNIT_ASSERT(c.accountList().count() == 0);

    m->removeAccount(b);

    // reload account info from titutionIDtorage
    a = m->account(a.id());
    c = m->account(c.id());

    try {
      b = m->account(b.id());
      CPPUNIT_FAIL("Exception expected");
    } catch (MyMoneyException *e) {
      delete e;
    }
    CPPUNIT_ASSERT(a.accountList().count() == 1);
    CPPUNIT_ASSERT(m->account(a.accountList()[0]).name() == "Acc C");

    CPPUNIT_ASSERT(c.accountList().count() == 0);

  } catch (MyMoneyException *e) {
    unexpectedException(e);
  }
}

void MyMoneyDatabaseMgrTest::testPayeeName()
{
  testAttachDb();

  if (!m_canOpen) {
    std::cout << "Database test skipped because no database could be opened." << std::endl;
    return;
  }

  testAddPayee();

  MyMoneyPayee p;
  QString name("THB");

  // OK case
  try {
    p = m->payeeByName(name);
    CPPUNIT_ASSERT(p.name() == "THB");
    CPPUNIT_ASSERT(p.id() == "P000001");
  } catch (MyMoneyException *e) {
    unexpectedException(e);
  }

  // Not OK case
  name = "Thb";
  try {
    p = m->payeeByName(name);
    CPPUNIT_FAIL("Exception expected");
  } catch (MyMoneyException *e) {
    delete e;
  }
}

void MyMoneyDatabaseMgrTest::testAssignment()
{
  testAttachDb();

  if (!m_canOpen) {
    std::cout << "Database test skipped because no database could be opened." << std::endl;
    return;
  }

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

  if (!m_canOpen) {
    std::cout << "Database test skipped because no database could be opened." << std::endl;
    return;
  }

  CPPUNIT_ASSERT(m->user().name() == t->user().name());
  CPPUNIT_ASSERT(m->user().address() == t->user().address());
  CPPUNIT_ASSERT(m->user().city() == t->user().city());
  CPPUNIT_ASSERT(m->user().state() == t->user().state());
  CPPUNIT_ASSERT(m->user().postcode() == t->user().postcode());
  CPPUNIT_ASSERT(m->user().telephone() == t->user().telephone());
  CPPUNIT_ASSERT(m->user().email() == t->user().email());
  //CPPUNIT_ASSERT(m->nextInstitutionID() == t->nextInstitutionID());
  //CPPUNIT_ASSERT(m->nextAccountID() == t->nextAccountID());
  //CPPUNIT_ASSERT(m->m_nextTransactionID == t->m_nextTransactionID);
  //CPPUNIT_ASSERT(m->nextPayeeID() == t->nextPayeeID());
  //CPPUNIT_ASSERT(m->m_nextScheduleID == t->m_nextScheduleID);
  CPPUNIT_ASSERT(m->dirty() == t->dirty());
  CPPUNIT_ASSERT(m->m_creationDate == t->m_creationDate);
  CPPUNIT_ASSERT(m->m_lastModificationDate == t->m_lastModificationDate);

  /*
   * make sure, that the keys and values are the same
   * on the left and the right side
   */
  //CPPUNIT_ASSERT(m->payeeList().keys() == t->payeeList().keys());
  //CPPUNIT_ASSERT(m->payeeList().values() == t->payeeList().values());
  CPPUNIT_ASSERT(m->payeeList() == t->payeeList());
  //CPPUNIT_ASSERT(m->m_transactionKeys.keys() == t->m_transactionKeys.keys());
  //CPPUNIT_ASSERT(m->m_transactionKeys.values() == t->m_transactionKeys.values());
  //CPPUNIT_ASSERT(m->institutionList().keys() == t->institutionList().keys());
  //CPPUNIT_ASSERT(m->institutionList().values() == t->institutionList().values());
  //CPPUNIT_ASSERT(m->m_accountList.keys() == t->m_accountList.keys());
  //CPPUNIT_ASSERT(m->m_accountList.values() == t->m_accountList.values());
  //CPPUNIT_ASSERT(m->m_transactionList.keys() == t->m_transactionList.keys());
  //CPPUNIT_ASSERT(m->m_transactionList.values() == t->m_transactionList.values());
  //CPPUNIT_ASSERT(m->m_balanceCache.keys() == t->m_balanceCache.keys());
  //CPPUNIT_ASSERT(m->m_balanceCache.values() == t->m_balanceCache.values());

//  CPPUNIT_ASSERT(m->scheduleList().keys() == t->scheduleList().keys());
//  CPPUNIT_ASSERT(m->scheduleList().values() == t->scheduleList().values());
}

void MyMoneyDatabaseMgrTest::testDuplicate()
{
  testAttachDb();

  if (!m_canOpen) {
    std::cout << "Database test skipped because no database could be opened." << std::endl;
    return;
  }

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

  if (!m_canOpen) {
    std::cout << "Database test skipped because no database could be opened." << std::endl;
    return;
  }

  // put some accounts in the db, so the tests don't break
  testReparentAccount();

  try {
    CPPUNIT_ASSERT(m->scheduleList().count() == 0);
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

    CPPUNIT_ASSERT(m->scheduleList().count() == 1);
    CPPUNIT_ASSERT(schedule.id() == "SCH000001");
    MyMoneyFile::instance()->clearCache();
    CPPUNIT_ASSERT(m->schedule("SCH000001").id() == "SCH000001");
  } catch (MyMoneyException *e) {
    unexpectedException(e);
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
    CPPUNIT_FAIL("Exception expected");
  } catch (MyMoneyException *e) {
    delete e;
  }

  CPPUNIT_ASSERT(m->scheduleList().count() == 1);

  // now try with a bad account, so this should cause an exception
  try {
    MyMoneyTransaction t1;
    MyMoneySplit s1, s2;
    s1.setAccountId("Abadaccount1");
    t1.addSplit(s1);
    s2.setAccountId("Abadaccount2");
    //t1.addSplit(s2);
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
    CPPUNIT_FAIL("Exception expected, but not thrown");
  } catch (MyMoneyException *e) {
    delete e;
    // Exception caught as expected.
  }

  CPPUNIT_ASSERT(m->scheduleList().count() == 1);
}

void MyMoneyDatabaseMgrTest::testSchedule()
{
  testAttachDb();

  if (!m_canOpen) {
    std::cout << "Database test skipped because no database could be opened." << std::endl;
    return;
  }

  testAddSchedule();
  MyMoneySchedule sched;

  sched = m->schedule("SCH000001");
  CPPUNIT_ASSERT(sched.name() == "Sched-Name");
  CPPUNIT_ASSERT(sched.id() == "SCH000001");

  try {
    m->schedule("SCH000002");
    CPPUNIT_FAIL("Exception expected");
  } catch (MyMoneyException *e) {
    delete e;
  }
}

void MyMoneyDatabaseMgrTest::testModifySchedule()
{
  testAttachDb();

  if (!m_canOpen) {
    std::cout << "Database test skipped because no database could be opened." << std::endl;
    return;
  }

  testAddSchedule();
  MyMoneySchedule sched;

  sched = m->schedule("SCH000001");
  sched.setId("SCH000002");
  try {
    m->modifySchedule(sched);
    CPPUNIT_FAIL("Exception expected");
  } catch (MyMoneyException *e) {
    delete e;
  }

  sched = m->schedule("SCH000001");
  sched.setName("New Sched-Name");
  try {
    m->modifySchedule(sched);
    CPPUNIT_ASSERT(m->scheduleList().count() == 1);
    CPPUNIT_ASSERT((*(m->scheduleList().begin())).name() == "New Sched-Name");
    CPPUNIT_ASSERT((*(m->scheduleList().begin())).id() == "SCH000001");

  } catch (MyMoneyException *e) {
    unexpectedException(e);
  }

}

void MyMoneyDatabaseMgrTest::testRemoveSchedule()
{
  testAttachDb();

  if (!m_canOpen) {
    std::cout << "Database test skipped because no database could be opened." << std::endl;
    return;
  }

  testAddSchedule();
  MyMoneySchedule sched;

  sched = m->schedule("SCH000001");
  sched.setId("SCH000002");
  try {
    m->removeSchedule(sched);
    CPPUNIT_FAIL("Exception expected");
  } catch (MyMoneyException *e) {
    delete e;
  }

  sched = m->schedule("SCH000001");
  try {
    m->removeSchedule(sched);
    CPPUNIT_ASSERT(m->scheduleList().count() == 0);

  } catch (MyMoneyException *e) {
    unexpectedException(e);
  }
}

void MyMoneyDatabaseMgrTest::testScheduleList()
{
  testAttachDb();

  if (!m_canOpen) {
    std::cout << "Database test skipped because no database could be opened." << std::endl;
    return;
  }

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
  } catch (MyMoneyException *e) {
    unexpectedException(e);
  }

  QList<MyMoneySchedule> list;

  // no filter
  list = m->scheduleList();
  CPPUNIT_ASSERT(list.count() == 4);

  // filter by type
  list = m->scheduleList("", MyMoneySchedule::TYPE_BILL);
  CPPUNIT_ASSERT(list.count() == 2);
  CPPUNIT_ASSERT(list[0].name() == "Schedule 1");
  CPPUNIT_ASSERT(list[1].name() == "Schedule 4");

  // filter by occurrence
  list = m->scheduleList("", MyMoneySchedule::TYPE_ANY,
                         MyMoneySchedule::OCCUR_DAILY);
  CPPUNIT_ASSERT(list.count() == 1);
  CPPUNIT_ASSERT(list[0].name() == "Schedule 2");

  // filter by payment type
  list = m->scheduleList("", MyMoneySchedule::TYPE_ANY,
                         MyMoneySchedule::OCCUR_ANY,
                         MyMoneySchedule::STYPE_DIRECTDEPOSIT);
  CPPUNIT_ASSERT(list.count() == 1);
  CPPUNIT_ASSERT(list[0].name() == "Schedule 2");

  // filter by account
  list = m->scheduleList("A01");
  CPPUNIT_ASSERT(list.count() == 0);
  list = m->scheduleList("A000001");
  CPPUNIT_ASSERT(list.count() == 2);
  list = m->scheduleList("A000002");
  CPPUNIT_ASSERT(list.count() == 1);

  // filter by start date
  list = m->scheduleList("", MyMoneySchedule::TYPE_ANY,
                         MyMoneySchedule::OCCUR_ANY,
                         MyMoneySchedule::STYPE_ANY,
                         notOverdue.addDays(31));
  CPPUNIT_ASSERT(list.count() == 3);
  CPPUNIT_ASSERT(list[0].name() == "Schedule 2");
  CPPUNIT_ASSERT(list[1].name() == "Schedule 3");
  CPPUNIT_ASSERT(list[2].name() == "Schedule 4");

  // filter by end date
  list = m->scheduleList("", MyMoneySchedule::TYPE_ANY,
                         MyMoneySchedule::OCCUR_ANY,
                         MyMoneySchedule::STYPE_ANY,
                         QDate(),
                         notOverdue.addDays(1));
  CPPUNIT_ASSERT(list.count() == 3);
  CPPUNIT_ASSERT(list[0].name() == "Schedule 1");
  CPPUNIT_ASSERT(list[1].name() == "Schedule 2");
  CPPUNIT_ASSERT(list[2].name() == "Schedule 4");

  // filter by start and end date
  list = m->scheduleList("", MyMoneySchedule::TYPE_ANY,
                         MyMoneySchedule::OCCUR_ANY,
                         MyMoneySchedule::STYPE_ANY,
                         notOverdue.addDays(-1),
                         notOverdue.addDays(1));
  CPPUNIT_ASSERT(list.count() == 2);
  CPPUNIT_ASSERT(list[0].name() == "Schedule 1");
  CPPUNIT_ASSERT(list[1].name() == "Schedule 2");

  // filter by overdue status
  list = m->scheduleList("", MyMoneySchedule::TYPE_ANY,
                         MyMoneySchedule::OCCUR_ANY,
                         MyMoneySchedule::STYPE_ANY,
                         QDate(),
                         QDate(),
                         true);
  CPPUNIT_ASSERT(list.count() == 1);
  CPPUNIT_ASSERT(list[0].name() == "Schedule 4");
}

void MyMoneyDatabaseMgrTest::testAddCurrency()
{
  testAttachDb();

  if (!m_canOpen) {
    std::cout << "Database test skipped because no database could be opened." << std::endl;
    return;
  }

  MyMoneySecurity curr("EUR", "Euro", "?", 100, 100);
  CPPUNIT_ASSERT(m->currencyList().count() == 0);
  m->setDirty();
  try {
    m->addCurrency(curr);
    CPPUNIT_ASSERT(m->currencyList().count() == 1);
    CPPUNIT_ASSERT((*(m->currencyList().begin())).name() == "Euro");
    CPPUNIT_ASSERT((*(m->currencyList().begin())).id() == "EUR");
  } catch (MyMoneyException *e) {
    unexpectedException(e);
  }

  m->setDirty();
  try {
    m->addCurrency(curr);
    CPPUNIT_FAIL("Expected exception missing");
  } catch (MyMoneyException *e) {
    CPPUNIT_ASSERT(m->dirty() == false);
    delete e;
  }
}

void MyMoneyDatabaseMgrTest::testModifyCurrency()
{
  testAttachDb();

  if (!m_canOpen) {
    std::cout << "Database test skipped because no database could be opened." << std::endl;
    return;
  }

  MyMoneySecurity curr("EUR", "Euro", "?", 100, 100);
  testAddCurrency();
  m->setDirty();
  curr.setName("EURO");
  try {
    m->modifyCurrency(curr);
    CPPUNIT_ASSERT(m->currencyList().count() == 1);
    CPPUNIT_ASSERT((*(m->currencyList().begin())).name() == "EURO");
    CPPUNIT_ASSERT((*(m->currencyList().begin())).id() == "EUR");
  } catch (MyMoneyException *e) {
    unexpectedException(e);
  }

  m->setDirty();

  MyMoneySecurity unknownCurr("DEM", "Deutsche Mark", "DM", 100, 100);
  try {
    m->modifyCurrency(unknownCurr);
    CPPUNIT_FAIL("Expected exception missing");
  } catch (MyMoneyException *e) {
    CPPUNIT_ASSERT(m->dirty() == false);
    delete e;
  }
}

void MyMoneyDatabaseMgrTest::testRemoveCurrency()
{
  testAttachDb();

  if (!m_canOpen) {
    std::cout << "Database test skipped because no database could be opened." << std::endl;
    return;
  }

  MyMoneySecurity curr("EUR", "Euro", "?", 100, 100);
  testAddCurrency();
  m->setDirty();
  try {
    m->removeCurrency(curr);
    CPPUNIT_ASSERT(m->currencyList().count() == 0);
  } catch (MyMoneyException *e) {
    unexpectedException(e);
  }

  m->setDirty();

  MyMoneySecurity unknownCurr("DEM", "Deutsche Mark", "DM", 100, 100);
  try {
    m->removeCurrency(unknownCurr);
    CPPUNIT_FAIL("Expected exception missing");
  } catch (MyMoneyException *e) {
    CPPUNIT_ASSERT(m->dirty() == false);
    delete e;
  }
}

void MyMoneyDatabaseMgrTest::testCurrency()
{
  testAttachDb();

  if (!m_canOpen) {
    std::cout << "Database test skipped because no database could be opened." << std::endl;
    return;
  }

  MyMoneySecurity curr("EUR", "Euro", "?", 100, 100);
  MyMoneySecurity newCurr;
  testAddCurrency();
  m->setDirty();
  try {
    newCurr = m->currency("EUR");
    CPPUNIT_ASSERT(m->dirty() == false);
    CPPUNIT_ASSERT(newCurr.id() == curr.id());
    CPPUNIT_ASSERT(newCurr.name() == curr.name());
  } catch (MyMoneyException *e) {
    unexpectedException(e);
  }

  try {
    m->currency("DEM");
    CPPUNIT_FAIL("Expected exception missing");
  } catch (MyMoneyException *e) {
    CPPUNIT_ASSERT(m->dirty() == false);
    delete e;
  }
}

void MyMoneyDatabaseMgrTest::testCurrencyList()
{
  testAttachDb();

  if (!m_canOpen) {
    std::cout << "Database test skipped because no database could be opened." << std::endl;
    return;
  }

  CPPUNIT_ASSERT(m->currencyList().count() == 0);

  testAddCurrency();
  CPPUNIT_ASSERT(m->currencyList().count() == 1);

  MyMoneySecurity unknownCurr("DEM", "Deutsche Mark", "DM", 100, 100);
  try {
    m->addCurrency(unknownCurr);
    m->setDirty();
    CPPUNIT_ASSERT(m->currencyList().count() == 2);
    CPPUNIT_ASSERT(m->currencyList().count() == 2);
    CPPUNIT_ASSERT(m->dirty() == false);
  } catch (MyMoneyException *e) {
    unexpectedException(e);
  }
}

void MyMoneyDatabaseMgrTest::testAccountList()
{
  testAttachDb();

  if (!m_canOpen) {
    std::cout << "Database test skipped because no database could be opened." << std::endl;
    return;
  }

  QList<MyMoneyAccount> accounts;
  m->accountList(accounts);
  CPPUNIT_ASSERT(accounts.count() == 0);
  testAddNewAccount();
  accounts.clear();
  m->accountList(accounts);
  CPPUNIT_ASSERT(accounts.count() == 2);

  MyMoneyAccount a = m->account("A000001");
  MyMoneyAccount b = m->account("A000002");
  m->reparentAccount(b, a);
  accounts.clear();
  m->accountList(accounts);
  CPPUNIT_ASSERT(accounts.count() == 2);
}

