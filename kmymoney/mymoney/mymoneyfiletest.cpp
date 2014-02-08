/***************************************************************************
                          mymoneyfiletest.cpp
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

#include "mymoneyfiletest.h"
#include <iostream>

#include <memory>
#include <unistd.h>
#include <QFile>
#include <QDataStream>
#include <QList>
#include <QtTest/QtTest>

#include "autotest.h"

QTEST_MAIN(MyMoneyFileTest)

void MyMoneyFileTest::objectAdded(MyMoneyFile::notificationObjectT type, const MyMoneyObject * const obj)
{
  Q_UNUSED(type);
  m_objectsAdded += obj->id();
}

void MyMoneyFileTest::objectRemoved(MyMoneyFile::notificationObjectT type, const QString& id)
{
  Q_UNUSED(type);
  m_objectsRemoved += id;
}

void MyMoneyFileTest::objectModified(MyMoneyFile::notificationObjectT type, const MyMoneyObject * const obj)
{
  Q_UNUSED(type);
  m_objectsModified += obj->id();
}

void MyMoneyFileTest::clearObjectLists(void)
{
  m_objectsAdded.clear();
  m_objectsModified.clear();
  m_objectsRemoved.clear();
  m_balanceChanged.clear();
  m_valueChanged.clear();
}

void MyMoneyFileTest::balanceChanged(const MyMoneyAccount& account)
{
  m_balanceChanged += account.id();
}

void MyMoneyFileTest::valueChanged(const MyMoneyAccount& account)
{
  m_valueChanged += account.id();
}

// this method will be called once at the beginning of the test
void MyMoneyFileTest::initTestCase()
{
  m = MyMoneyFile::instance();

  connect(m, SIGNAL(objectAdded(MyMoneyFile::notificationObjectT,MyMoneyObject*const)), this, SLOT(objectAdded(MyMoneyFile::notificationObjectT,MyMoneyObject*const)));
  connect(m, SIGNAL(objectRemoved(MyMoneyFile::notificationObjectT,QString)), this, SLOT(objectRemoved(MyMoneyFile::notificationObjectT,QString)));
  connect(m, SIGNAL(objectModified(MyMoneyFile::notificationObjectT,MyMoneyObject*const)), this, SLOT(objectModified(MyMoneyFile::notificationObjectT,MyMoneyObject*const)));
  connect(m, SIGNAL(balanceChanged(MyMoneyAccount)), this, SLOT(balanceChanged(MyMoneyAccount)));
  connect(m, SIGNAL(valueChanged(MyMoneyAccount)), this, SLOT(valueChanged(MyMoneyAccount)));
}

// this method will be called before each testfunction
void MyMoneyFileTest::init()
{
  storage = new MyMoneySeqAccessMgr;
  m->attachStorage(storage);
  clearObjectLists();
}

// this method will be called after each testfunction
void MyMoneyFileTest::cleanup()
{
  m->detachStorage(storage);
  delete storage;
}

void MyMoneyFileTest::testEmptyConstructor()
{
  MyMoneyPayee user = m->user();

  QVERIFY(user.name().isEmpty());
  QVERIFY(user.address().isEmpty());
  QVERIFY(user.city().isEmpty());
  QVERIFY(user.state().isEmpty());
  QVERIFY(user.postcode().isEmpty());
  QVERIFY(user.telephone().isEmpty());
  QVERIFY(user.email().isEmpty());

  QVERIFY(m->institutionCount() == 0);
  QVERIFY(m->dirty() == false);
  QVERIFY(m->accountCount() == 5);
}

void MyMoneyFileTest::testAddOneInstitution()
{
  MyMoneyInstitution institution;

  institution.setName("institution1");
  institution.setTown("town");
  institution.setStreet("street");
  institution.setPostcode("postcode");
  institution.setTelephone("telephone");
  institution.setManager("manager");
  institution.setSortcode("sortcode");

  // MyMoneyInstitution institution_file("", institution);
  MyMoneyInstitution institution_id("I000002", institution);
  MyMoneyInstitution institution_noname(institution);
  institution_noname.setName(QString());

  QString id;

  QVERIFY(m->institutionCount() == 0);
  storage->m_dirty = false;

  clearObjectLists();
  MyMoneyFileTransaction ft;
  try {
    m->addInstitution(institution);
    ft.commit();
    QVERIFY(institution.id() == "I000001");
    QVERIFY(m->institutionCount() == 1);
    QVERIFY(m->dirty() == true);

    QVERIFY(m_objectsRemoved.count() == 0);
    QVERIFY(m_objectsAdded.count() == 1);
    QVERIFY(m_objectsModified.count() == 0);
    QVERIFY(m_balanceChanged.count() == 0);
    QVERIFY(m_valueChanged.count() == 0);
    QVERIFY(m_objectsAdded[0] == QLatin1String("I000001"));
  } catch (MyMoneyException *e) {
    QFAIL("Unexpected exception");
    delete e;
  }

  clearObjectLists();
  ft.restart();
  try {
    m->addInstitution(institution_id);
    QFAIL("Missing expected exception");
  } catch (MyMoneyException *e) {
    ft.commit();
    QVERIFY(m->institutionCount() == 1);
    delete e;
  }

  ft.restart();
  try {
    m->addInstitution(institution_noname);
    QFAIL("Missing expected exception");
  } catch (MyMoneyException *e) {
    ft.commit();
    QVERIFY(m->institutionCount() == 1);
    delete e;
  }
  QVERIFY(m_objectsRemoved.count() == 0);
  QVERIFY(m_objectsAdded.count() == 0);
  QVERIFY(m_objectsModified.count() == 0);
  QVERIFY(m_balanceChanged.count() == 0);
  QVERIFY(m_valueChanged.count() == 0);
}

void MyMoneyFileTest::testAddTwoInstitutions()
{
  testAddOneInstitution();
  MyMoneyInstitution institution;
  institution.setName("institution2");
  institution.setTown("town");
  institution.setStreet("street");
  institution.setPostcode("postcode");
  institution.setTelephone("telephone");
  institution.setManager("manager");
  institution.setSortcode("sortcode");

  QString id;

  storage->m_dirty = false;
  MyMoneyFileTransaction ft;
  try {
    m->addInstitution(institution);
    ft.commit();

    QVERIFY(institution.id() == "I000002");
    QVERIFY(m->institutionCount() == 2);
    QVERIFY(m->dirty() == true);
  } catch (MyMoneyException *e) {
    QFAIL("Unexpected exception");
    delete e;
  }

  storage->m_dirty = false;

  try {
    institution = m->institution("I000001");
    QVERIFY(institution.id() == "I000001");
    QVERIFY(m->institutionCount() == 2);
    QVERIFY(m->dirty() == false);

    institution = m->institution("I000002");
    QVERIFY(institution.id() == "I000002");
    QVERIFY(m->institutionCount() == 2);
    QVERIFY(m->dirty() == false);
  } catch (MyMoneyException *e) {
    QFAIL("Unexpected exception");
    delete e;
  }
}

void MyMoneyFileTest::testRemoveInstitution()
{
  testAddTwoInstitutions();

  MyMoneyInstitution i;

  QVERIFY(m->institutionCount() == 2);

  i = m->institution("I000001");
  QVERIFY(i.id() == "I000001");
  QVERIFY(i.accountCount() == 0);

  clearObjectLists();

  storage->m_dirty = false;
  MyMoneyFileTransaction ft;
  try {
    m->removeInstitution(i);
    QVERIFY(m_objectsRemoved.count() == 0);
    ft.commit();
    QVERIFY(m->institutionCount() == 1);
    QVERIFY(m->dirty() == true);
    QVERIFY(m_objectsRemoved.count() == 1);
    QVERIFY(m_objectsAdded.count() == 0);
    QVERIFY(m_objectsModified.count() == 0);
    QVERIFY(m_balanceChanged.count() == 0);
    QVERIFY(m_valueChanged.count() == 0);
    QVERIFY(m_objectsRemoved[0] == QLatin1String("I000001"));
  } catch (MyMoneyException *e) {
    QFAIL("Unexpected exception");
    delete e;
  }

  storage->m_dirty = false;

  try {
    m->institution("I000001");
    QFAIL("Missing expected exception");
  } catch (MyMoneyException *e) {
    QVERIFY(m->institutionCount() == 1);
    QVERIFY(m->dirty() == false);
    delete e;
  }

  clearObjectLists();
  ft.restart();
  try {
    m->removeInstitution(i);
    QFAIL("Missing expected exception");
  } catch (MyMoneyException *e) {
    ft.commit();
    QVERIFY(m->institutionCount() == 1);
    QVERIFY(m->dirty() == false);
    QVERIFY(m_objectsRemoved.count() == 0);
    delete e;
  }
}

void MyMoneyFileTest::testInstitutionRetrieval()
{

  testAddOneInstitution();

  storage->m_dirty = false;

  MyMoneyInstitution institution;

  QVERIFY(m->institutionCount() == 1);

  try {
    institution = m->institution("I000001");
    QVERIFY(institution.id() == "I000001");
    QVERIFY(m->institutionCount() == 1);
  } catch (MyMoneyException *e) {
    QFAIL("Unexpected exception");
    delete e;
  }

  try {
    institution = m->institution("I000002");
    QFAIL("Missing expected exception");
  } catch (MyMoneyException *e) {
    QVERIFY(m->institutionCount() == 1);
    delete e;
  }

  QVERIFY(m->dirty() == false);
}

void MyMoneyFileTest::testInstitutionListRetrieval()
{
  QList<MyMoneyInstitution> list;

  storage->m_dirty = false;
  list = m->institutionList();
  QVERIFY(m->dirty() == false);
  QVERIFY(list.count() == 0);

  testAddTwoInstitutions();

  storage->m_dirty = false;
  list = m->institutionList();
  QVERIFY(m->dirty() == false);
  QVERIFY(list.count() == 2);

  QList<MyMoneyInstitution>::ConstIterator it;
  it = list.constBegin();

  QVERIFY((*it).name() == "institution1");
  ++it;
  QVERIFY((*it).name() == "institution2");
  ++it;
  QVERIFY(it == list.constEnd());
}

void MyMoneyFileTest::testInstitutionModify()
{
  testAddTwoInstitutions();
  MyMoneyInstitution institution;

  institution = m->institution("I000001");
  institution.setStreet("new street");
  institution.setTown("new town");
  institution.setPostcode("new postcode");
  institution.setTelephone("new telephone");
  institution.setManager("new manager");
  institution.setName("new name");
  institution.setSortcode("new sortcode");

  storage->m_dirty = false;

  clearObjectLists();
  MyMoneyFileTransaction ft;
  try {
    m->modifyInstitution(institution);
    ft.commit();
    QVERIFY(institution.id() == "I000001");
    QVERIFY(m->institutionCount() == 2);
    QVERIFY(m->dirty() == true);

    QVERIFY(m_objectsRemoved.count() == 0);
    QVERIFY(m_objectsAdded.count() == 0);
    QVERIFY(m_objectsModified.count() == 1);
    QVERIFY(m_balanceChanged.count() == 0);
    QVERIFY(m_valueChanged.count() == 0);
    QVERIFY(m_objectsModified[0] == QLatin1String("I000001"));
  } catch (MyMoneyException *e) {
    delete e;
    QFAIL("Unexpected exception");
  }

  MyMoneyInstitution newInstitution;
  newInstitution = m->institution("I000001");

  QVERIFY(newInstitution.id() == "I000001");
  QVERIFY(newInstitution.street() == "new street");
  QVERIFY(newInstitution.town() == "new town");
  QVERIFY(newInstitution.postcode() == "new postcode");
  QVERIFY(newInstitution.telephone() == "new telephone");
  QVERIFY(newInstitution.manager() == "new manager");
  QVERIFY(newInstitution.name() == "new name");
  QVERIFY(newInstitution.sortcode() == "new sortcode");

  storage->m_dirty = false;

  ft.restart();
  MyMoneyInstitution failInstitution2("I000003", newInstitution);
  try {
    m->modifyInstitution(failInstitution2);
    QFAIL("Exception expected");
  } catch (MyMoneyException *e) {
    ft.commit();
    delete e;
    QVERIFY(failInstitution2.id() == "I000003");
    QVERIFY(m->institutionCount() == 2);
    QVERIFY(m->dirty() == false);
  }
}

void MyMoneyFileTest::testSetFunctions()
{
  MyMoneyPayee user = m->user();

  QVERIFY(user.name().isEmpty());
  QVERIFY(user.address().isEmpty());
  QVERIFY(user.city().isEmpty());
  QVERIFY(user.state().isEmpty());
  QVERIFY(user.postcode().isEmpty());
  QVERIFY(user.telephone().isEmpty());
  QVERIFY(user.email().isEmpty());

  MyMoneyFileTransaction ft;
  storage->m_dirty = false;
  user.setName("Name");
  m->setUser(user);
  QVERIFY(m->dirty() == true);
  storage->m_dirty = false;
  user.setAddress("Street");
  m->setUser(user);
  QVERIFY(m->dirty() == true);
  storage->m_dirty = false;
  user.setCity("Town");
  m->setUser(user);
  QVERIFY(m->dirty() == true);
  storage->m_dirty = false;
  user.setState("County");
  m->setUser(user);
  QVERIFY(m->dirty() == true);
  storage->m_dirty = false;
  user.setPostcode("Postcode");
  m->setUser(user);
  QVERIFY(m->dirty() == true);
  storage->m_dirty = false;
  user.setTelephone("Telephone");
  m->setUser(user);
  QVERIFY(m->dirty() == true);
  storage->m_dirty = false;
  user.setEmail("Email");
  m->setUser(user);
  QVERIFY(m->dirty() == true);
  storage->m_dirty = false;

  ft.commit();
  user = m->user();
  QVERIFY(user.name() == "Name");
  QVERIFY(user.address() == "Street");
  QVERIFY(user.city() == "Town");
  QVERIFY(user.state() == "County");
  QVERIFY(user.postcode() == "Postcode");
  QVERIFY(user.telephone() == "Telephone");
  QVERIFY(user.email() == "Email");
}

void MyMoneyFileTest::testAddAccounts()
{
  testAddTwoInstitutions();
  MyMoneyAccount  a, b, c;
  a.setAccountType(MyMoneyAccount::Checkings);
  b.setAccountType(MyMoneyAccount::Checkings);

  MyMoneyInstitution institution;

  storage->m_dirty = false;

  QVERIFY(m->accountCount() == 5);

  institution = m->institution("I000001");
  QVERIFY(institution.id() == "I000001");

  a.setName("Account1");
  a.setInstitutionId(institution.id());
  a.setCurrencyId("EUR");

  clearObjectLists();
  MyMoneyFileTransaction ft;
  try {
    MyMoneyAccount parent = m->asset();
    m->addAccount(a, parent);
    ft.commit();
    QVERIFY(m->accountCount() == 6);
    QVERIFY(a.parentAccountId() == "AStd::Asset");
    QVERIFY(a.id() == "A000001");
    QVERIFY(a.institutionId() == "I000001");
    QVERIFY(a.currencyId() == "EUR");
    QVERIFY(m->dirty() == true);
    QVERIFY(m->asset().accountList().count() == 1);
    QVERIFY(m->asset().accountList()[0] == "A000001");

    institution = m->institution("I000001");
    QVERIFY(institution.accountCount() == 1);
    QVERIFY(institution.accountList()[0] == "A000001");

    QVERIFY(m_objectsRemoved.count() == 0);
    QVERIFY(m_objectsAdded.count() == 1);
    QVERIFY(m_objectsModified.count() == 2);
    QVERIFY(m_balanceChanged.count() == 0);
    QVERIFY(m_valueChanged.count() == 0);
    QVERIFY(m_objectsAdded.contains(QLatin1String("A000001")));
    QVERIFY(m_objectsModified.contains(QLatin1String("I000001")));
    QVERIFY(m_objectsModified.contains(QLatin1String("AStd::Asset")));

  } catch (MyMoneyException *e) {
    delete e;
    QFAIL("Unexpected exception!");
  }

  // try to add this account again, should not work
  ft.restart();
  try {
    MyMoneyAccount parent = m->asset();
    m->addAccount(a, parent);
    QFAIL("Expecting exception!");
  } catch (MyMoneyException *e) {
    ft.commit();
    delete e;
  }

  // check that we can modify the local object and
  // reload it from the file
  a.setName("AccountX");
  a = m->account("A000001");
  QVERIFY(a.name() == "Account1");

  storage->m_dirty = false;

  // check if we can get the same info to a different object
  c = m->account("A000001");
  QVERIFY(c.accountType() == MyMoneyAccount::Checkings);
  QVERIFY(c.id() == "A000001");
  QVERIFY(c.name() == "Account1");
  QVERIFY(c.institutionId() == "I000001");

  QVERIFY(m->dirty() == false);

  // add a second account
  institution = m->institution("I000002");
  b.setName("Account2");
  b.setInstitutionId(institution.id());
  b.setCurrencyId("EUR");
  clearObjectLists();
  ft.restart();
  try {
    MyMoneyAccount parent = m->asset();
    m->addAccount(b, parent);
    ft.commit();
    QVERIFY(m->dirty() == true);
    QVERIFY(b.id() == "A000002");
    QVERIFY(b.currencyId() == "EUR");
    QVERIFY(b.parentAccountId() == "AStd::Asset");
    QVERIFY(m->accountCount() == 7);

    institution = m->institution("I000001");
    QVERIFY(institution.accountCount() == 1);
    QVERIFY(institution.accountList()[0] == "A000001");

    institution = m->institution("I000002");
    QVERIFY(institution.accountCount() == 1);
    QVERIFY(institution.accountList()[0] == "A000002");

    QVERIFY(m->asset().accountList().count() == 2);
    QVERIFY(m->asset().accountList()[0] == "A000001");
    QVERIFY(m->asset().accountList()[1] == "A000002");

    QVERIFY(m_objectsRemoved.count() == 0);
    QVERIFY(m_objectsAdded.count() == 1);
    QVERIFY(m_objectsModified.count() == 2);
    QVERIFY(m_balanceChanged.count() == 0);
    QVERIFY(m_valueChanged.count() == 0);
    QVERIFY(m_objectsAdded.contains(QLatin1String("A000002")));
    QVERIFY(m_objectsModified.contains(QLatin1String("I000002")));
    QVERIFY(m_objectsModified.contains(QLatin1String("AStd::Asset")));

  } catch (MyMoneyException *e) {
    delete e;
    QFAIL("Unexpected exception!");
  }

  MyMoneyAccount p;

  p = m->account("A000002");
  QVERIFY(p.accountType() == MyMoneyAccount::Checkings);
  QVERIFY(p.id() == "A000002");
  QVERIFY(p.name() == "Account2");
  QVERIFY(p.institutionId() == "I000002");
  QVERIFY(p.currencyId() == "EUR");
}

void MyMoneyFileTest::testModifyAccount()
{
  testAddAccounts();
  storage->m_dirty = false;

  MyMoneyAccount p = m->account("A000001");
  MyMoneyInstitution institution;

  QVERIFY(p.accountType() == MyMoneyAccount::Checkings);
  QVERIFY(p.name() == "Account1");

  p.setName("New account name");
  MyMoneyFileTransaction ft;
  clearObjectLists();
  try {
    m->modifyAccount(p);
    ft.commit();

    QVERIFY(m->dirty() == true);
    QVERIFY(m->accountCount() == 7);
    QVERIFY(p.accountType() == MyMoneyAccount::Checkings);
    QVERIFY(p.name() == "New account name");

    QVERIFY(m_objectsRemoved.count() == 0);
    QVERIFY(m_objectsAdded.count() == 0);
    QVERIFY(m_objectsModified.count() == 1);
    QVERIFY(m_balanceChanged.count() == 0);
    QVERIFY(m_valueChanged.count() == 0);
    QVERIFY(m_objectsModified.contains(QLatin1String("A000001")));

  } catch (MyMoneyException *e) {
    delete e;
    QFAIL("Unexpected exception!");
  }
  storage->m_dirty = false;

  // try to move account to new institution
  p.setInstitutionId("I000002");
  ft.restart();
  clearObjectLists();
  try {
    m->modifyAccount(p);
    ft.commit();

    QVERIFY(m->dirty() == true);
    QVERIFY(m->accountCount() == 7);
    QVERIFY(p.accountType() == MyMoneyAccount::Checkings);
    QVERIFY(p.name() == "New account name");
    QVERIFY(p.institutionId() == "I000002");

    institution = m->institution("I000001");
    QVERIFY(institution.accountCount() == 0);

    institution = m->institution("I000002");
    QVERIFY(institution.accountCount() == 2);
    QVERIFY(institution.accountList()[0] == "A000002");
    QVERIFY(institution.accountList()[1] == "A000001");

    QVERIFY(m_objectsRemoved.count() == 0);
    QVERIFY(m_objectsAdded.count() == 0);
    QVERIFY(m_objectsModified.count() == 3);
    QVERIFY(m_balanceChanged.count() == 0);
    QVERIFY(m_valueChanged.count() == 0);
    QVERIFY(m_objectsModified.contains(QLatin1String("A000001")));
    QVERIFY(m_objectsModified.contains(QLatin1String("I000001")));
    QVERIFY(m_objectsModified.contains(QLatin1String("I000002")));

  } catch (MyMoneyException *e) {
    delete e;
    QFAIL("Unexpected exception!");
  }
  storage->m_dirty = false;

  // try to change to an account type that is allowed
  p.setAccountType(MyMoneyAccount::Savings);
  ft.restart();
  try {
    m->modifyAccount(p);
    ft.commit();

    QVERIFY(m->dirty() == true);
    QVERIFY(m->accountCount() == 7);
    QVERIFY(p.accountType() == MyMoneyAccount::Savings);
    QVERIFY(p.name() == "New account name");

  } catch (MyMoneyException *e) {
    delete e;
    QFAIL("Unexpected exception!");
  }
  storage->m_dirty = false;

  // try to change to an account type that is not allowed
  p.setAccountType(MyMoneyAccount::CreditCard);
  ft.restart();
  try {
    m->modifyAccount(p);
    QFAIL("Expecting exception!");
  } catch (MyMoneyException *e) {
    ft.commit();
    delete e;
  }
  storage->m_dirty = false;

  // try to fool engine a bit
  p.setParentAccountId("A000001");
  ft.restart();
  try {
    m->modifyAccount(p);
    QFAIL("Expecting exception!");
  } catch (MyMoneyException *e) {
    ft.commit();
    delete e;
  }
}

void MyMoneyFileTest::testReparentAccount()
{
  testAddAccounts();
  storage->m_dirty = false;

  MyMoneyAccount p = m->account("A000001");
  MyMoneyAccount q = m->account("A000002");
  MyMoneyAccount o = m->account(p.parentAccountId());

  // make A000001 a child of A000002
  clearObjectLists();
  MyMoneyFileTransaction ft;
  try {
    QVERIFY(p.parentAccountId() != q.id());
    QVERIFY(o.accountCount() == 2);
    QVERIFY(q.accountCount() == 0);
    m->reparentAccount(p, q);
    ft.commit();
    QVERIFY(m->dirty() == true);
    QVERIFY(p.parentAccountId() == q.id());
    QVERIFY(q.accountCount() == 1);
    QVERIFY(q.id() == "A000002");
    QVERIFY(p.id() == "A000001");
    QVERIFY(q.accountList()[0] == p.id());

    QVERIFY(m_objectsRemoved.count() == 0);
    QVERIFY(m_objectsAdded.count() == 0);
    QVERIFY(m_objectsModified.count() == 3);
    QVERIFY(m_balanceChanged.count() == 0);
    QVERIFY(m_valueChanged.count() == 0);
    QVERIFY(m_objectsModified.contains(QLatin1String("A000001")));
    QVERIFY(m_objectsModified.contains(QLatin1String("A000002")));
    QVERIFY(m_objectsModified.contains(QLatin1String("AStd::Asset")));

    o = m->account(o.id());
    QVERIFY(o.accountCount() == 1);
  } catch (MyMoneyException *e) {
    delete e;
    QFAIL("Unexpected exception!");
  }
}

void MyMoneyFileTest::testRemoveStdAccount(const MyMoneyAccount& acc)
{
  QString txt("Exception expected while removing account ");
  txt += acc.id();
  MyMoneyFileTransaction ft;
  try {
    m->removeAccount(acc);
    QFAIL(qPrintable(txt));
  } catch (MyMoneyException *e) {
    ft.commit();
    delete e;
  }
}

void MyMoneyFileTest::testRemoveAccount()
{
  MyMoneyInstitution institution;

  testAddAccounts();
  QVERIFY(m->accountCount() == 7);
  storage->m_dirty = false;

  QString id;
  MyMoneyAccount p = m->account("A000001");

  clearObjectLists();

  MyMoneyFileTransaction ft;
  try {
    MyMoneyAccount q("Ainvalid", p);
    m->removeAccount(q);
    QFAIL("Exception expected!");
  } catch (MyMoneyException *e) {
    ft.commit();
    delete e;
  }

  ft.restart();
  try {
    QVERIFY(m_objectsRemoved.count() == 0);
    QVERIFY(m_objectsAdded.count() == 0);
    QVERIFY(m_objectsModified.count() == 0);

    m->removeAccount(p);
    ft.commit();
    QVERIFY(m->dirty() == true);
    QVERIFY(m->accountCount() == 6);
    institution = m->institution("I000001");
    QVERIFY(institution.accountCount() == 0);
    QVERIFY(m->asset().accountList().count() == 1);

    QVERIFY(m_objectsRemoved.count() == 1);
    QVERIFY(m_objectsModified.count() == 2);
    QVERIFY(m_objectsAdded.count() == 0);
    QVERIFY(m_balanceChanged.count() == 0);
    QVERIFY(m_valueChanged.count() == 0);

    QVERIFY(m_objectsRemoved.contains(QLatin1String("A000001")));
    QVERIFY(m_objectsModified.contains(QLatin1String("I000001")));
    QVERIFY(m_objectsModified.contains(QLatin1String("AStd::Asset")));

    institution = m->institution("I000002");
    QVERIFY(institution.accountCount() == 1);

  } catch (MyMoneyException *e) {
    delete e;
    QFAIL("Unexpected exception!");
  }

  // Check that the standard account-groups cannot be removed
  testRemoveStdAccount(m->liability());
  testRemoveStdAccount(m->asset());
  testRemoveStdAccount(m->expense());
  testRemoveStdAccount(m->income());
}

void MyMoneyFileTest::testRemoveAccountTree()
{
  testReparentAccount();
  MyMoneyAccount a = m->account("A000002");

  clearObjectLists();
  MyMoneyFileTransaction ft;
  // remove the account
  try {
    m->removeAccount(a);
    ft.commit();

    QVERIFY(m_objectsRemoved.count() == 1);
    QVERIFY(m_objectsModified.count() == 3);
    QVERIFY(m_objectsAdded.count() == 0);
    QVERIFY(m_balanceChanged.count() == 0);
    QVERIFY(m_valueChanged.count() == 0);

    QVERIFY(m_objectsRemoved.contains(QLatin1String("A000002")));
    QVERIFY(m_objectsModified.contains(QLatin1String("A000001")));
    QVERIFY(m_objectsModified.contains(QLatin1String("I000002")));
    QVERIFY(m_objectsModified.contains(QLatin1String("AStd::Asset")));

  } catch (MyMoneyException *e) {
    delete e;
    QFAIL("Unexpected exception!");
  }
  QVERIFY(m->accountCount() == 6);

  // make sure it's gone
  try {
    m->account("A000002");
    QFAIL("Exception expected!");
  } catch (MyMoneyException *e) {
    delete e;
  }

  // make sure that children are re-parented to parent account
  try {
    a = m->account("A000001");
    QVERIFY(a.parentAccountId() == m->asset().id());
  } catch (MyMoneyException *e) {
    delete e;
    QFAIL("Unexpected exception!");
  }
}

void MyMoneyFileTest::testAccountListRetrieval()
{
  QList<MyMoneyAccount> list;

  storage->m_dirty = false;
  m->accountList(list);
  QVERIFY(m->dirty() == false);
  QVERIFY(list.count() == 0);

  testAddAccounts();

  storage->m_dirty = false;
  list.clear();
  m->accountList(list);
  QVERIFY(m->dirty() == false);
  QVERIFY(list.count() == 2);

  QVERIFY(list[0].accountType() == MyMoneyAccount::Checkings);
  QVERIFY(list[1].accountType() == MyMoneyAccount::Checkings);
}

void MyMoneyFileTest::testAddTransaction()
{
  testAddAccounts();
  MyMoneyTransaction t, p;

  MyMoneyAccount exp1;
  exp1.setAccountType(MyMoneyAccount::Expense);
  exp1.setName("Expense1");
  MyMoneyAccount exp2;
  exp2.setAccountType(MyMoneyAccount::Expense);
  exp2.setName("Expense2");

  MyMoneyFileTransaction ft;
  try {
    MyMoneyAccount parent = m->expense();
    m->addAccount(exp1, parent);
    m->addAccount(exp2, parent);
    ft.commit();
  } catch (MyMoneyException *e) {
    delete e;
    QFAIL("Unexpected exception!");
  }

  // fake the last modified flag to check that the
  // date is updated when we add the transaction
  MyMoneyAccount a = m->account("A000001");
  a.setLastModified(QDate(1, 2, 3));
  ft.restart();
  try {
    m->modifyAccount(a);
    ft.commit();
  } catch (MyMoneyException *e) {
    delete e;
    QFAIL("Unexpected exception!");
  }
  ft.restart();

  QVERIFY(m->accountCount() == 9);
  a = m->account("A000001");
  QVERIFY(a.lastModified() == QDate(1, 2, 3));

  // construct a transaction and add it to the pool
  t.setPostDate(QDate(2002, 2, 1));
  t.setMemo("Memotext");

  MyMoneySplit split1;
  MyMoneySplit split2;

  split1.setAccountId("A000001");
  split1.setShares(MyMoneyMoney(-1000, 100));
  split1.setValue(MyMoneyMoney(-1000, 100));
  split2.setAccountId("A000003");
  split2.setValue(MyMoneyMoney(1000, 100));
  split2.setShares(MyMoneyMoney(1000, 100));
  try {
    t.addSplit(split1);
    t.addSplit(split2);
  } catch (MyMoneyException *e) {
    delete e;
    QFAIL("Unexpected exception!");
  }

  /*
          // FIXME: we don't have a payee and a number field right now
          // guess we should have a number field per split, don't know
          // about the payee
          t.setMethod(MyMoneyCheckingTransaction::Withdrawal);
          t.setPayee("Thomas Baumgart");
          t.setNumber("1234");
          t.setState(MyMoneyCheckingTransaction::Cleared);
  */
  storage->m_dirty = false;

  ft.restart();
  clearObjectLists();
  try {
    m->addTransaction(t);
    ft.commit();
    QVERIFY(m_objectsRemoved.count() == 0);
    QVERIFY(m_objectsModified.count() == 0);
    QVERIFY(m_objectsAdded.count() == 0);
    QVERIFY(m_balanceChanged.count() == 2);
    QVERIFY(m_balanceChanged.count("A000001") == 1);
    QVERIFY(m_balanceChanged.count("A000003") == 1);
    QVERIFY(m_valueChanged.count() == 0);
  } catch (MyMoneyException *e) {
    delete e;
    QFAIL("Unexpected exception!");
  }
  ft.restart();
  clearObjectLists();

  QVERIFY(t.id() == "T000000000000000001");
  QVERIFY(t.postDate() == QDate(2002, 2, 1));
  QVERIFY(t.entryDate() == QDate::currentDate());
  QVERIFY(m->dirty() == true);

  // check the balance of the accounts
  a = m->account("A000001");
  QVERIFY(a.lastModified() == QDate::currentDate());
  QVERIFY(a.balance() == MyMoneyMoney(-1000, 100));

  MyMoneyAccount b = m->account("A000003");
  QVERIFY(b.lastModified() == QDate::currentDate());
  QVERIFY(b.balance() == MyMoneyMoney(1000, 100));

  storage->m_dirty = false;

  // locate transaction in MyMoneyFile via id

  try {
    p = m->transaction("T000000000000000001");
    QVERIFY(m_balanceChanged.count() == 0);
    QVERIFY(m_valueChanged.count() == 0);
    QVERIFY(p.splitCount() == 2);
    QVERIFY(p.memo() == "Memotext");
    QVERIFY(p.splits()[0].accountId() == "A000001");
    QVERIFY(p.splits()[1].accountId() == "A000003");
  } catch (MyMoneyException *e) {
    delete e;
    QFAIL("Unexpected exception!");
  }

  // check if it's in the account(s) as well

  try {
    p = m->transaction("A000001", 0);
    QVERIFY(m_balanceChanged.count() == 0);
    QVERIFY(m_valueChanged.count() == 0);
    QVERIFY(p.id() == "T000000000000000001");
    QVERIFY(p.splitCount() == 2);
    QVERIFY(p.memo() == "Memotext");
    QVERIFY(p.splits()[0].accountId() == "A000001");
    QVERIFY(p.splits()[1].accountId() == "A000003");
  } catch (MyMoneyException *e) {
    delete e;
    QFAIL("Unexpected exception!");
  }

  try {
    p = m->transaction("A000003", 0);
    QVERIFY(m_balanceChanged.count() == 0);
    QVERIFY(m_valueChanged.count() == 0);
    QVERIFY(p.id() == "T000000000000000001");
    QVERIFY(p.splitCount() == 2);
    QVERIFY(p.memo() == "Memotext");
    QVERIFY(p.splits()[0].accountId() == "A000001");
    QVERIFY(p.splits()[1].accountId() == "A000003");
  } catch (MyMoneyException *e) {
    delete e;
    QFAIL("Unexpected exception!");
  }
}

void MyMoneyFileTest::testIsStandardAccount()
{
  QVERIFY(m->isStandardAccount(m->liability().id()) == true);
  QVERIFY(m->isStandardAccount(m->asset().id()) == true);
  QVERIFY(m->isStandardAccount(m->expense().id()) == true);
  QVERIFY(m->isStandardAccount(m->income().id()) == true);
  QVERIFY(m->isStandardAccount("A00001") == false);
}

void MyMoneyFileTest::testHasActiveSplits()
{
  testAddTransaction();

  QVERIFY(m->hasActiveSplits("A000001") == true);
  QVERIFY(m->hasActiveSplits("A000002") == false);
}

void MyMoneyFileTest::testModifyTransactionSimple()
{
  // this will test that we can modify the basic attributes
  // of a transaction
  testAddTransaction();

  MyMoneyTransaction t = m->transaction("T000000000000000001");
  t.setMemo("New Memotext");
  storage->m_dirty = false;

  MyMoneyFileTransaction ft;
  clearObjectLists();
  try {
    m->modifyTransaction(t);
    ft.commit();
    QVERIFY(m_objectsRemoved.count() == 0);
    QVERIFY(m_objectsModified.count() == 0);
    QVERIFY(m_objectsAdded.count() == 0);
    QVERIFY(m_balanceChanged.count() == 2);
    QVERIFY(m_balanceChanged.count(QLatin1String("A000001")) == 1);
    QVERIFY(m_balanceChanged.count(QLatin1String("A000003")) == 1);
    QVERIFY(m_valueChanged.count() == 0);
    t = m->transaction("T000000000000000001");
    QVERIFY(t.memo() == "New Memotext");
    QVERIFY(m->dirty() == true);

  } catch (MyMoneyException *e) {
    delete e;
    QFAIL("Unexpected exception!");
  }
}

void MyMoneyFileTest::testModifyTransactionNewPostDate()
{
  // this will test that we can modify the basic attributes
  // of a transaction
  testAddTransaction();

  MyMoneyTransaction t = m->transaction("T000000000000000001");
  t.setPostDate(QDate(2004, 2, 1));
  storage->m_dirty = false;

  MyMoneyFileTransaction ft;
  clearObjectLists();
  try {
    m->modifyTransaction(t);
    ft.commit();
    t = m->transaction("T000000000000000001");
    QVERIFY(t.postDate() == QDate(2004, 2, 1));
    t = m->transaction("A000001", 0);
    QVERIFY(t.id() == "T000000000000000001");
    QVERIFY(m->dirty() == true);

    QVERIFY(m_objectsRemoved.count() == 0);
    QVERIFY(m_objectsModified.count() == 0);
    QVERIFY(m_objectsAdded.count() == 0);
    QVERIFY(m_balanceChanged.count() == 2);
    QVERIFY(m_balanceChanged.count(QLatin1String("A000001")) == 1);
    QVERIFY(m_balanceChanged.count(QLatin1String("A000003")) == 1);
    QVERIFY(m_valueChanged.count() == 0);
  } catch (MyMoneyException *e) {
    delete e;
    QFAIL("Unexpected exception!");
  }
}

void MyMoneyFileTest::testModifyTransactionNewAccount()
{
  // this will test that we can modify the basic attributes
  // of a transaction
  testAddTransaction();

  MyMoneyTransaction t = m->transaction("T000000000000000001");
  MyMoneySplit s;
  s = t.splits()[0];
  s.setAccountId("A000002");
  t.modifySplit(s);

  storage->m_dirty = false;
  MyMoneyFileTransaction ft;
  clearObjectLists();
  try {
    MyMoneyTransactionFilter f1("A000001");
    MyMoneyTransactionFilter f2("A000002");
    MyMoneyTransactionFilter f3("A000003");
    QVERIFY(m->transactionList(f1).count() == 1);
    QVERIFY(m->transactionList(f2).count() == 0);
    QVERIFY(m->transactionList(f3).count() == 1);

    m->modifyTransaction(t);
    ft.commit();
    t = m->transaction("T000000000000000001");
    QVERIFY(t.postDate() == QDate(2002, 2, 1));
    t = m->transaction("A000002", 0);
    QVERIFY(m->dirty() == true);
    QVERIFY(m->transactionList(f1).count() == 0);
    QVERIFY(m->transactionList(f2).count() == 1);
    QVERIFY(m->transactionList(f3).count() == 1);

    QVERIFY(m_objectsRemoved.count() == 0);
    QVERIFY(m_objectsModified.count() == 0);
    QVERIFY(m_objectsAdded.count() == 0);
    QVERIFY(m_balanceChanged.count() == 3);
    QVERIFY(m_balanceChanged.count(QLatin1String("A000001")) == 1);
    QVERIFY(m_balanceChanged.count(QLatin1String("A000002")) == 1);
    QVERIFY(m_balanceChanged.count(QLatin1String("A000003")) == 1);
    QVERIFY(m_valueChanged.count() == 0);


  } catch (MyMoneyException *e) {
    delete e;
    QFAIL("Unexpected exception!");
  }
}

void MyMoneyFileTest::testRemoveTransaction()
{
  testModifyTransactionNewPostDate();

  MyMoneyTransaction t;
  t = m->transaction("T000000000000000001");

  storage->m_dirty = false;
  MyMoneyFileTransaction ft;
  clearObjectLists();
  try {
    m->removeTransaction(t);
    ft.commit();
    QVERIFY(m->dirty() == true);
    QVERIFY(m->transactionCount() == 0);
    MyMoneyTransactionFilter f1("A000001");
    MyMoneyTransactionFilter f2("A000002");
    MyMoneyTransactionFilter f3("A000003");
    QVERIFY(m->transactionList(f1).count() == 0);
    QVERIFY(m->transactionList(f2).count() == 0);
    QVERIFY(m->transactionList(f3).count() == 0);

    QVERIFY(m_objectsRemoved.count() == 0);
    QVERIFY(m_objectsModified.count() == 0);
    QVERIFY(m_objectsAdded.count() == 0);
    QVERIFY(m_balanceChanged.count() == 2);
    QVERIFY(m_balanceChanged.count(QLatin1String("A000001")) == 1);
    QVERIFY(m_balanceChanged.count(QLatin1String("A000003")) == 1);
    QVERIFY(m_valueChanged.count() == 0);

  } catch (MyMoneyException *e) {
    delete e;
    QFAIL("Unexpected exception!");
  }
}

/*
 * This function is currently not implemented. It's kind of tricky
 * because it modifies a lot of objects in a single call. This might
 * be a problem for the undo/redo stuff. That's why I left it out in
 * the first run. We migh add it, if we need it.
 * /
void testMoveSplits() {
        testModifyTransactionNewPostDate();

        QVERIFY(m->account("A000001").transactionCount() == 1);
        QVERIFY(m->account("A000002").transactionCount() == 0);
        QVERIFY(m->account("A000003").transactionCount() == 1);

        try {
                m->moveSplits("A000001", "A000002");
                QVERIFY(m->account("A000001").transactionCount() == 0);
                QVERIFY(m->account("A000002").transactionCount() == 1);
                QVERIFY(m->account("A000003").transactionCount() == 1);
        } catch(MyMoneyException *e) {
                delete e;
                QFAIL("Unexpected exception!");
        }
}
*/

void MyMoneyFileTest::testBalanceTotal()
{
  testAddTransaction();
  MyMoneyTransaction t;

  // construct a transaction and add it to the pool
  t.setPostDate(QDate(2002, 2, 1));
  t.setMemo("Memotext");

  MyMoneySplit split1;
  MyMoneySplit split2;

  MyMoneyFileTransaction ft;
  try {
    split1.setAccountId("A000002");
    split1.setShares(MyMoneyMoney(-1000, 100));
    split1.setValue(MyMoneyMoney(-1000, 100));
    split2.setAccountId("A000004");
    split2.setValue(MyMoneyMoney(1000, 100));
    split2.setShares(MyMoneyMoney(1000, 100));
    t.addSplit(split1);
    t.addSplit(split2);
    m->addTransaction(t);
    ft.commit();
    ft.restart();
    QVERIFY(t.id() == "T000000000000000002");
    QVERIFY(m->totalBalance("A000001") == MyMoneyMoney(-1000, 100));
    QVERIFY(m->totalBalance("A000002") == MyMoneyMoney(-1000, 100));

    MyMoneyAccount p = m->account("A000001");
    MyMoneyAccount q = m->account("A000002");
    m->reparentAccount(p, q);
    ft.commit();
    // check totalBalance() and balance() with combinations of parameters
    QVERIFY(m->totalBalance("A000001") == MyMoneyMoney(-1000, 100));
    QVERIFY(m->totalBalance("A000002") == MyMoneyMoney(-2000, 100));
    QVERIFY(m->totalBalance("A000002", QDate(2002, 1, 15)).isZero());

    QVERIFY(m->balance("A000001") == MyMoneyMoney(-1000, 100));
    QVERIFY(m->balance("A000002") == MyMoneyMoney(-1000, 100));
    // Date of a transaction
    QVERIFY(m->balance("A000001", QDate(2002, 2, 1)) == MyMoneyMoney(-1000, 100));
    QVERIFY(m->balance("A000002", QDate(2002, 2, 1)) == MyMoneyMoney(-1000, 100));
    // Date after last transaction
    QVERIFY(m->balance("A000001", QDate(2002, 2, 1)) == MyMoneyMoney(-1000, 100));
    QVERIFY(m->balance("A000002", QDate(2002, 2, 1)) == MyMoneyMoney(-1000, 100));
    // Date before first transaction
    QVERIFY(m->balance("A000001", QDate(2002, 1, 15)).isZero());
    QVERIFY(m->balance("A000002", QDate(2002, 1, 15)).isZero());

  } catch (MyMoneyException *e) {
    delete e;
    QFAIL("Unexpected exception!");
  }

  // Now check for exceptions
  try {
    // Account not found for balance()
    QVERIFY(m->balance("A000005").isZero());
    QFAIL("Exception expected");
  } catch (MyMoneyException *e) {
    delete e;
  }

  try {
    // Account not found for totalBalance()
    QVERIFY(m->totalBalance("A000005").isZero());
    QFAIL("Exception expected");
  } catch (MyMoneyException *e) {
    delete e;
  }

}

void MyMoneyFileTest::testSetAccountName()
{
  MyMoneyFileTransaction ft;
  clearObjectLists();
  try {
    m->setAccountName(STD_ACC_LIABILITY, "Verbindlichkeiten");
    ft.commit();
    QVERIFY(m_objectsRemoved.count() == 0);
    QVERIFY(m_objectsModified.count() == 1);
    QVERIFY(m_objectsAdded.count() == 0);
    QVERIFY(m_balanceChanged.count() == 0);
    QVERIFY(m_valueChanged.count() == 0);
    QVERIFY(m_objectsModified.contains(QLatin1String("AStd::Liability")));

  } catch (MyMoneyException *e) {
    delete e;
    QFAIL("Unexpected exception");
  }
  ft.restart();
  clearObjectLists();
  try {
    m->setAccountName(STD_ACC_ASSET, "Vermögen");
    ft.commit();
    QVERIFY(m_objectsRemoved.count() == 0);
    QVERIFY(m_objectsModified.count() == 1);
    QVERIFY(m_objectsAdded.count() == 0);
    QVERIFY(m_balanceChanged.count() == 0);
    QVERIFY(m_valueChanged.count() == 0);
    QVERIFY(m_objectsModified.contains(QLatin1String("AStd::Asset")));

  } catch (MyMoneyException *e) {
    delete e;
    QFAIL("Unexpected exception");
  }
  ft.restart();
  clearObjectLists();
  try {
    m->setAccountName(STD_ACC_EXPENSE, "Ausgaben");
    ft.commit();
    QVERIFY(m_objectsRemoved.count() == 0);
    QVERIFY(m_objectsModified.count() == 1);
    QVERIFY(m_objectsAdded.count() == 0);
    QVERIFY(m_balanceChanged.count() == 0);
    QVERIFY(m_valueChanged.count() == 0);
    QVERIFY(m_objectsModified.contains(QLatin1String("AStd::Expense")));

  } catch (MyMoneyException *e) {
    delete e;
    QFAIL("Unexpected exception");
  }
  ft.restart();
  clearObjectLists();
  try {
    m->setAccountName(STD_ACC_INCOME, "Einnahmen");
    ft.commit();
    QVERIFY(m_objectsRemoved.count() == 0);
    QVERIFY(m_objectsModified.count() == 1);
    QVERIFY(m_objectsAdded.count() == 0);
    QVERIFY(m_balanceChanged.count() == 0);
    QVERIFY(m_valueChanged.count() == 0);
    QVERIFY(m_objectsModified.contains(QLatin1String("AStd::Income")));

  } catch (MyMoneyException *e) {
    delete e;
    QFAIL("Unexpected exception");
  }
  ft.restart();

  QVERIFY(m->liability().name() == "Verbindlichkeiten");
  QVERIFY(m->asset().name() == "Vermögen");
  QVERIFY(m->expense().name() == "Ausgaben");
  QVERIFY(m->income().name() == "Einnahmen");

  try {
    m->setAccountName("A000001", "New account name");
    ft.commit();
    QFAIL("Exception expected");
  } catch (MyMoneyException *e) {
    delete e;
  }
}

void MyMoneyFileTest::testAddPayee()
{
  MyMoneyPayee p;

  p.setName("THB");
  QVERIFY(m->dirty() == false);
  MyMoneyFileTransaction ft;
  try {
    m->addPayee(p);
    ft.commit();
    QVERIFY(m->dirty() == true);
    QVERIFY(p.id() == "P000001");

    QVERIFY(m_objectsRemoved.count() == 0);
    QVERIFY(m_objectsModified.count() == 0);
    QVERIFY(m_objectsAdded.count() == 1);
    QVERIFY(m_balanceChanged.count() == 0);
    QVERIFY(m_valueChanged.count() == 0);

    QVERIFY(m_objectsAdded.contains(QLatin1String("P000001")));
  } catch (MyMoneyException *e) {
    delete e;
    QFAIL("Unexpected exception");
  }
}

void MyMoneyFileTest::testModifyPayee()
{
  MyMoneyPayee p;

  testAddPayee();
  clearObjectLists();

  p = m->payee("P000001");
  p.setName("New name");
  MyMoneyFileTransaction ft;
  try {
    m->modifyPayee(p);
    ft.commit();
    p = m->payee("P000001");
    QVERIFY(p.name() == "New name");

    QVERIFY(m_objectsRemoved.count() == 0);
    QVERIFY(m_objectsModified.count() == 1);
    QVERIFY(m_objectsAdded.count() == 0);
    QVERIFY(m_balanceChanged.count() == 0);
    QVERIFY(m_valueChanged.count() == 0);

    QVERIFY(m_objectsModified.contains(QLatin1String("P000001")));
  } catch (MyMoneyException *e) {
    delete e;
    QFAIL("Unexpected exception");
  }
}

void MyMoneyFileTest::testRemovePayee()
{
  MyMoneyPayee p;

  testAddPayee();
  clearObjectLists();
  QVERIFY(m->payeeList().count() == 1);

  p = m->payee("P000001");
  MyMoneyFileTransaction ft;
  try {
    m->removePayee(p);
    ft.commit();
    QVERIFY(m->payeeList().count() == 0);

    QVERIFY(m_objectsRemoved.count() == 1);
    QVERIFY(m_objectsModified.count() == 0);
    QVERIFY(m_objectsAdded.count() == 0);
    QVERIFY(m_balanceChanged.count() == 0);
    QVERIFY(m_valueChanged.count() == 0);

    QVERIFY(m_objectsRemoved.contains(QLatin1String("P000001")));

  } catch (MyMoneyException *e) {
    delete e;
    QFAIL("Unexpected exception");
  }
}

void MyMoneyFileTest::testAddTransactionStd()
{
  testAddAccounts();
  MyMoneyTransaction t, p;
  MyMoneyAccount a;

  a = m->account("A000001");

  // construct a transaction and add it to the pool
  t.setPostDate(QDate(2002, 2, 1));
  t.setMemo("Memotext");

  MyMoneySplit split1;
  MyMoneySplit split2;

  split1.setAccountId("A000001");
  split1.setShares(MyMoneyMoney(-1000, 100));
  split1.setValue(MyMoneyMoney(-1000, 100));
  split2.setAccountId(STD_ACC_EXPENSE);
  split2.setValue(MyMoneyMoney(1000, 100));
  split2.setShares(MyMoneyMoney(1000, 100));
  try {
    t.addSplit(split1);
    t.addSplit(split2);
  } catch (MyMoneyException *e) {
    delete e;
    QFAIL("Unexpected exception!");
  }

  /*
          // FIXME: we don't have a payee and a number field right now
          // guess we should have a number field per split, don't know
          // about the payee
          t.setMethod(MyMoneyCheckingTransaction::Withdrawal);
          t.setPayee("Thomas Baumgart");
          t.setNumber("1234");
          t.setState(MyMoneyCheckingTransaction::Cleared);
  */
  storage->m_dirty = false;

  MyMoneyFileTransaction ft;
  try {
    m->addTransaction(t);
    ft.commit();
    QFAIL("Missing expected exception!");
  } catch (MyMoneyException *e) {
    delete e;
  }

  QVERIFY(m->dirty() == false);
}

void MyMoneyFileTest::testAttachStorage()
{
  IMyMoneyStorage *store = new MyMoneySeqAccessMgr;
  MyMoneyFile *file = new MyMoneyFile;

  QVERIFY(file->storageAttached() == false);
  try {
    file->attachStorage(store);
    QVERIFY(file->storageAttached() == true);
  } catch (MyMoneyException *e) {
    delete e;
    QFAIL("Unexpected exception!");
  }

  try {
    file->attachStorage(store);
    QFAIL("Exception expected!");
  } catch (MyMoneyException *e) {
    delete e;
  }

  try {
    file->attachStorage(0);
    QFAIL("Exception expected!");
  } catch (MyMoneyException *e) {
    delete e;
  }

  try {
    file->detachStorage(store);
    QVERIFY(file->storageAttached() == false);
  } catch (MyMoneyException *e) {
    delete e;
    QFAIL("Unexpected exception!");
  }

  delete store;
  delete file;
}


void MyMoneyFileTest::testAccount2Category()
{
  testReparentAccount();
  QVERIFY(m->accountToCategory("A000001") == "Account2:Account1");
  QVERIFY(m->accountToCategory("A000002") == "Account2");
}

void MyMoneyFileTest::testCategory2Account()
{
  testAddTransaction();
  MyMoneyAccount a = m->account("A000003");
  MyMoneyAccount b = m->account("A000004");

  MyMoneyFileTransaction ft;
  try {
    m->reparentAccount(b, a);
    ft.commit();
    QVERIFY(m->categoryToAccount("Expense1") == "A000003");
    QVERIFY(m->categoryToAccount("Expense1:Expense2") == "A000004");
    QVERIFY(m->categoryToAccount("Acc2").isEmpty());
  } catch (MyMoneyException *e) {
    unexpectedException(e);
  }
}

void MyMoneyFileTest::testAttachedStorage()
{
  QVERIFY(m->storageAttached() == true);
  QVERIFY(m->storage() != 0);
  IMyMoneyStorage *p = m->storage();
  m->detachStorage(p);
  QVERIFY(m->storageAttached() == false);
  QVERIFY(m->storage() == 0);
  m->attachStorage(p);
  QVERIFY(m->storageAttached() == true);
  QVERIFY(m->storage() != 0);
}

void MyMoneyFileTest::testHasAccount()
{
  testAddAccounts();

  MyMoneyAccount a, b;
  a.setAccountType(MyMoneyAccount::Checkings);
  a.setName("Account3");
  b = m->account("A000001");
  MyMoneyFileTransaction ft;
  try {
    m->addAccount(a, b);
    ft.commit();
    QVERIFY(m->accountCount() == 8);
    QVERIFY(a.parentAccountId() == "A000001");
    QVERIFY(m->hasAccount("A000001", "Account3") == true);
    QVERIFY(m->hasAccount("A000001", "Account2") == false);
    QVERIFY(m->hasAccount("A000002", "Account3") == false);
  } catch (MyMoneyException *e) {
    unexpectedException(e);
  }
}

void MyMoneyFileTest::testAddEquityAccount()
{
  MyMoneyAccount i;
  i.setName("Investment");
  i.setAccountType(MyMoneyAccount::Investment);

  MyMoneyFileTransaction ft;
  try {
    MyMoneyAccount parent = m->asset();
    m->addAccount(i, parent);
    ft.commit();
  } catch (MyMoneyException *e) {
    unexpectedException(e);
  }
  // keep a copy for later use
  m_inv = i;

  // make sure, that only equity accounts can be children to it
  MyMoneyAccount a;
  a.setName("Testaccount");
  QList<MyMoneyAccount::accountTypeE> list;
  list << MyMoneyAccount::Checkings;
  list << MyMoneyAccount::Savings;
  list << MyMoneyAccount::Cash;
  list << MyMoneyAccount::CreditCard;
  list << MyMoneyAccount::Loan;
  list << MyMoneyAccount::CertificateDep;
  list << MyMoneyAccount::Investment;
  list << MyMoneyAccount::MoneyMarket;
  list << MyMoneyAccount::Asset;
  list << MyMoneyAccount::Liability;
  list << MyMoneyAccount::Currency;
  list << MyMoneyAccount::Income;
  list << MyMoneyAccount::Expense;
  list << MyMoneyAccount::AssetLoan;

  QList<MyMoneyAccount::accountTypeE>::Iterator it;
  for (it = list.begin(); it != list.end(); ++it) {
    a.setAccountType(*it);
    ft.restart();
    try {
      char    msg[100];
      m->addAccount(a, i);
      sprintf(msg, "Can add non-equity type %d to investment", *it);
      QFAIL(msg);
    } catch (MyMoneyException *e) {
      ft.commit();
      delete e;
    }
  }
  ft.restart();
  try {
    a.setName("Teststock");
    a.setAccountType(MyMoneyAccount::Stock);
    m->addAccount(a, i);
    ft.commit();
  } catch (MyMoneyException *e) {
    unexpectedException(e);
  }
}

void MyMoneyFileTest::testReparentEquity()
{
  testAddEquityAccount();
  testAddEquityAccount();
  MyMoneyAccount parent;

  // check the bad cases
  QList<MyMoneyAccount::accountTypeE> list;
  list << MyMoneyAccount::Checkings;
  list << MyMoneyAccount::Savings;
  list << MyMoneyAccount::Cash;
  list << MyMoneyAccount::CertificateDep;
  list << MyMoneyAccount::MoneyMarket;
  list << MyMoneyAccount::Asset;
  list << MyMoneyAccount::AssetLoan;
  list << MyMoneyAccount::Currency;
  parent = m->asset();
  testReparentEquity(list, parent);

  list.clear();
  list << MyMoneyAccount::CreditCard;
  list << MyMoneyAccount::Loan;
  list << MyMoneyAccount::Liability;
  parent = m->liability();
  testReparentEquity(list, parent);

  list.clear();
  list << MyMoneyAccount::Income;
  parent = m->income();
  testReparentEquity(list, parent);

  list.clear();
  list << MyMoneyAccount::Expense;
  parent = m->expense();
  testReparentEquity(list, parent);

  // now check the good case
  MyMoneyAccount stock = m->account("A000002");
  MyMoneyAccount inv = m->account(m_inv.id());
  MyMoneyFileTransaction ft;
  try {
    m->reparentAccount(stock, inv);
    ft.commit();
  } catch (MyMoneyException *e) {
    unexpectedException(e);
  }
}

void MyMoneyFileTest::testReparentEquity(QList<MyMoneyAccount::accountTypeE>& list, MyMoneyAccount& parent)
{
  MyMoneyAccount a;
  MyMoneyAccount stock = m->account("A000002");

  QList<MyMoneyAccount::accountTypeE>::Iterator it;
  MyMoneyFileTransaction ft;
  for (it = list.begin(); it != list.end(); ++it) {
    a.setName(QString("Testaccount %1").arg(*it));
    a.setAccountType(*it);
    try {
      m->addAccount(a, parent);
      char    msg[100];
      m->reparentAccount(stock, a);
      sprintf(msg, "Can reparent stock to non-investment type %d account", *it);
      QFAIL(msg);
    } catch (MyMoneyException *e) {
      ft.commit();
      delete e;
    }
    ft.restart();
  }
}

void MyMoneyFileTest::testBaseCurrency(void)
{
  MyMoneySecurity base("EUR", "Euro", QChar(0x20ac));
  MyMoneySecurity ref;

  // make sure, no base currency is set
  try {
    ref = m->baseCurrency();
    QVERIFY(ref.id().isEmpty());
  } catch (MyMoneyException *e) {
    unexpectedException(e);
  }

  // make sure, we cannot assign an unknown currency
  try {
    m->setBaseCurrency(base);
    QFAIL("Missing expected exception");
  } catch (MyMoneyException *e) {
    delete e;
  }

  MyMoneyFileTransaction ft;
  // add the currency and try again
  try {
    m->addCurrency(base);
    m->setBaseCurrency(base);
    ft.commit();
  } catch (MyMoneyException *e) {
    unexpectedException(e);
  }
  ft.restart();

  // make sure, the base currency is set
  try {
    ref = m->baseCurrency();
    QVERIFY(ref.id() == "EUR");
    QVERIFY(ref.name() == "Euro");
    QVERIFY(ref.tradingSymbol() == QChar(0x20ac));
  } catch (MyMoneyException *e) {
    unexpectedException(e);
  }

  // check if it gets reset when attaching a new storage
  m->detachStorage(storage);

  MyMoneySeqAccessMgr* newStorage = new MyMoneySeqAccessMgr;
  m->attachStorage(newStorage);

  ref = m->baseCurrency();
  QVERIFY(ref.id().isEmpty());

  m->detachStorage(newStorage);
  delete newStorage;

  m->attachStorage(storage);
  ref = m->baseCurrency();
  QVERIFY(ref.id() == "EUR");
  QVERIFY(ref.name() == "Euro");
  QVERIFY(ref.tradingSymbol() == QChar(0x20ac));
}

void MyMoneyFileTest::testOpeningBalanceNoBase(void)
{
  MyMoneyAccount openingAcc;
  MyMoneySecurity base;

  try {
    base = m->baseCurrency();
    openingAcc = m->openingBalanceAccount(base);
    QFAIL("Missing expected exception");
  } catch (MyMoneyException *e) {
    delete e;
  }
}

void MyMoneyFileTest::testOpeningBalance(void)
{
  MyMoneyAccount openingAcc;
  MyMoneySecurity second("USD", "US Dollar", "$");
  testBaseCurrency();

  try {
    openingAcc = m->openingBalanceAccount(m->baseCurrency());
    QVERIFY(openingAcc.parentAccountId() == m->equity().id());
    QVERIFY(openingAcc.name() == MyMoneyFile::OpeningBalancesPrefix);
    QVERIFY(openingAcc.openingDate() == QDate::currentDate());
  } catch (MyMoneyException *e) {
    unexpectedException(e);
  }

  // add a second currency
  MyMoneyFileTransaction ft;
  try {
    m->addCurrency(second);
    ft.commit();
  } catch (MyMoneyException *e) {
    unexpectedException(e);
  }

  QString refName = QString("%1 (%2)").arg(MyMoneyFile::OpeningBalancesPrefix).arg("USD");
  try {
    openingAcc = m->openingBalanceAccount(second);
    QVERIFY(openingAcc.parentAccountId() == m->equity().id());
    QVERIFY(openingAcc.name() == refName);
    QVERIFY(openingAcc.openingDate() == QDate::currentDate());
  } catch (MyMoneyException *e) {
    unexpectedException(e);
  }
}

void MyMoneyFileTest::testModifyStdAccount()
{
  QVERIFY(m->asset().currencyId().isEmpty());
  QVERIFY(m->asset().name() == "Asset");
  testBaseCurrency();
  QVERIFY(m->asset().currencyId().isEmpty());
  QVERIFY(!m->baseCurrency().id().isEmpty());

  MyMoneyFileTransaction ft;
  try {
    MyMoneyAccount acc = m->asset();
    acc.setName("Anlagen");
    acc.setCurrencyId(m->baseCurrency().id());
    m->modifyAccount(acc);
    ft.commit();

    QVERIFY(m->asset().name() == "Anlagen");
    QVERIFY(m->asset().currencyId() == m->baseCurrency().id());
  } catch (MyMoneyException *e) {
    unexpectedException(e);
  }

  ft.restart();
  try {
    MyMoneyAccount acc = m->asset();
    acc.setNumber("Test");
    m->modifyAccount(acc);
    QFAIL("Missing expected exception");
  } catch (MyMoneyException *e) {
    ft.rollback();
    delete e;
  }

}

void MyMoneyFileTest::testAddPrice()
{
  testAddAccounts();
  testBaseCurrency();
  MyMoneyAccount p;

  MyMoneyFileTransaction ft;
  try {
    p = m->account("A000002");
    p.setCurrencyId("RON");
    m->modifyAccount(p);
    ft.commit();

    QVERIFY(m->account("A000002").currencyId() == "RON");
  } catch (MyMoneyException *e) {
    unexpectedException(e);
  }

  clearObjectLists();
  ft.restart();
  MyMoneyPrice price("EUR", "RON", QDate::currentDate(), MyMoneyMoney(4.1), "Test source");
  m->addPrice(price);
  ft.commit();
  QVERIFY(m_balanceChanged.count() == 0);
  QVERIFY(m_valueChanged.count() == 1);
  QVERIFY(m_valueChanged.count("A000002") == 1);

  clearObjectLists();
  ft.restart();
  MyMoneyPrice priceReciprocal("RON", "EUR", QDate::currentDate(), MyMoneyMoney(1 / 4.1), "Test source reciprocal price");
  m->addPrice(priceReciprocal);
  ft.commit();
  QVERIFY(m_balanceChanged.count() == 0);
  QVERIFY(m_valueChanged.count() == 1);
  QVERIFY(m_valueChanged.count("A000002") == 1);
}

void MyMoneyFileTest::testRemovePrice()
{
  testAddPrice();
  clearObjectLists();
  MyMoneyFileTransaction ft;
  MyMoneyPrice price("EUR", "RON", QDate::currentDate(), MyMoneyMoney(4.1), "Test source");
  m->removePrice(price);
  ft.commit();
  QVERIFY(m_balanceChanged.count() == 0);
  QVERIFY(m_valueChanged.count() == 1);
  QVERIFY(m_valueChanged.count("A000002") == 1);
}

void MyMoneyFileTest::testGetPrice()
{
  testAddPrice();
  // the price for the current date is found
  QVERIFY(m->price("EUR", "RON", QDate::currentDate()).isValid());
  // the price for the current date is returned when asking for the next day with exact date set to false
  {
    const MyMoneyPrice &price = m->price("EUR", "RON", QDate::currentDate().addDays(1), false);
    QVERIFY(price.isValid() && price.date() == QDate::currentDate());
  }
  // no price is returned while asking for the next day with exact date set to true
  QVERIFY(!m->price("EUR", "RON", QDate::currentDate().addDays(1), true).isValid());

  // no price is returned while asking for the previous day with exact date set to true/false because all prices are newer
  QVERIFY(!m->price("EUR", "RON", QDate::currentDate().addDays(-1), false).isValid());
  QVERIFY(!m->price("EUR", "RON", QDate::currentDate().addDays(-1), true).isValid());

  // add two more prices
  MyMoneyFileTransaction ft;
  m->addPrice(MyMoneyPrice("EUR", "RON", QDate::currentDate().addDays(3), MyMoneyMoney(4.1), "Test source"));
  m->addPrice(MyMoneyPrice("EUR", "RON", QDate::currentDate().addDays(5), MyMoneyMoney(4.1), "Test source"));
  ft.commit();
  clearObjectLists();

  // extra tests for the exactDate=false behavior
  {
    const MyMoneyPrice &price = m->price("EUR", "RON", QDate::currentDate().addDays(2), false);
    QVERIFY(price.isValid() && price.date() == QDate::currentDate());
  }
  {
    const MyMoneyPrice &price = m->price("EUR", "RON", QDate::currentDate().addDays(3), false);
    QVERIFY(price.isValid() && price.date() == QDate::currentDate().addDays(3));
  }
  {
    const MyMoneyPrice &price = m->price("EUR", "RON", QDate::currentDate().addDays(4), false);
    QVERIFY(price.isValid() && price.date() == QDate::currentDate().addDays(3));
  }
  {
    const MyMoneyPrice &price = m->price("EUR", "RON", QDate::currentDate().addDays(5), false);
    QVERIFY(price.isValid() && price.date() == QDate::currentDate().addDays(5));
  }
  {
    const MyMoneyPrice &price = m->price("EUR", "RON", QDate::currentDate().addDays(6), false);
    QVERIFY(price.isValid() && price.date() == QDate::currentDate().addDays(5));
  }
}

void MyMoneyFileTest::testAddAccountMissingCurrency()
{
  testAddTwoInstitutions();
  MyMoneySecurity base("EUR", "Euro", QChar(0x20ac));
  MyMoneyAccount  a;
  a.setAccountType(MyMoneyAccount::Checkings);

  MyMoneyInstitution institution;

  storage->m_dirty = false;

  QVERIFY(m->accountCount() == 5);

  institution = m->institution("I000001");
  QVERIFY(institution.id() == "I000001");

  a.setName("Account1");
  a.setInstitutionId(institution.id());

  clearObjectLists();
  MyMoneyFileTransaction ft;
  try {
    m->addCurrency(base);
    m->setBaseCurrency(base);
    MyMoneyAccount parent = m->asset();
    m->addAccount(a, parent);
    ft.commit();
    QVERIFY(m->account("A000001").currencyId() == "EUR");
  } catch (MyMoneyException *e) {
    unexpectedException(e);
  }
}

void MyMoneyFileTest::testAddTransactionToClosedAccount()
{
  QSKIP("Test not implemented yet", SkipAll);
}

void MyMoneyFileTest::testRemoveTransactionFromClosedAccount()
{
  QSKIP("Test not implemented yet", SkipAll);
}

void MyMoneyFileTest::testModifyTransactionInClosedAccount()
{
  QSKIP("Test not implemented yet", SkipAll);
}

void MyMoneyFileTest::testStorageId()
{
  QString id;

  // make sure id will be setup if it does not exist
  MyMoneyFileTransaction ft;
  try {
    m->setValue("kmm-id", "");
    ft.commit();
  } catch (MyMoneyException *e) {
    unexpectedException(e);
  }

  try {
    // check for a new id
    id = m->storageId();
    QVERIFY(!id.isEmpty());
    // check that it is the same if we ask again
    QVERIFY(id == m->storageId());
  } catch (MyMoneyException *e) {
    unexpectedException(e);
  }
}

void MyMoneyFileTest::testHasMatchingOnlineBalance_emptyAccountWithoutImportedBalance()
{
  AddOneAccount();

  MyMoneyAccount a = m->account("A000001");

  QVERIFY(m->hasMatchingOnlineBalance(a) == false);
}

void MyMoneyFileTest::testHasMatchingOnlineBalance_emptyAccountWithEqualImportedBalance()
{
  AddOneAccount();

  MyMoneyAccount a = m->account("A000001");

  a.setValue("lastImportedTransactionDate", QDate(2011, 12, 1).toString(Qt::ISODate));
  a.setValue("lastStatementBalance", MyMoneyMoney(0, 1).toString());

  MyMoneyFileTransaction ft;
  m->modifyAccount(a);
  ft.commit();

  QVERIFY(m->hasMatchingOnlineBalance(a) == true);
}

void MyMoneyFileTest::testHasMatchingOnlineBalance_emptyAccountWithUnequalImportedBalance()
{
  AddOneAccount();

  MyMoneyAccount a = m->account("A000001");

  a.setValue("lastImportedTransactionDate", QDate(2011, 12, 1).toString(Qt::ISODate));
  a.setValue("lastStatementBalance", MyMoneyMoney(1, 1).toString());

  MyMoneyFileTransaction ft;
  m->modifyAccount(a);
  ft.commit();

  QVERIFY(m->hasMatchingOnlineBalance(a) == false);
}

void MyMoneyFileTest::testHasNewerTransaction_withoutAnyTransaction_afterLastImportedTransaction()
{
  AddOneAccount();

  MyMoneyAccount a = m->account("A000001");

  QDate dateOfLastTransactionImport(2011, 12, 1);

  // There are no transactions at all:
  QVERIFY(m->hasNewerTransaction(a.id(), dateOfLastTransactionImport) == false);
}

void MyMoneyFileTest::testHasNewerTransaction_withoutNewerTransaction_afterLastImportedTransaction()
{

  AddOneAccount();

  QString accId("A000001");
  QDate dateOfLastTransactionImport(2011, 12, 1);

  MyMoneyFileTransaction ft;
  MyMoneyTransaction t;

  // construct a transaction at the day of the last transaction import and add it to the pool
  t.setPostDate(dateOfLastTransactionImport);

  MyMoneySplit split1;

  split1.setAccountId(accId);
  split1.setShares(MyMoneyMoney(-1000, 100));
  split1.setValue(MyMoneyMoney(-1000, 100));
  t.addSplit(split1);

  ft.restart();
  m->addTransaction(t);
  ft.commit();

  QVERIFY(m->hasNewerTransaction(accId, dateOfLastTransactionImport) == false);
}

void MyMoneyFileTest::testHasNewerTransaction_withNewerTransaction_afterLastImportedTransaction()
{

  AddOneAccount();

  QString accId("A000001");
  QDate dateOfLastTransactionImport(2011, 12, 1);
  QDate dateOfDayAfterLastTransactionImport(dateOfLastTransactionImport.addDays(1));

  MyMoneyFileTransaction ft;
  MyMoneyTransaction t;

  // construct a transaction a day after the last transaction import and add it to the pool
  t.setPostDate(dateOfDayAfterLastTransactionImport);

  MyMoneySplit split1;

  split1.setAccountId(accId);
  split1.setShares(MyMoneyMoney(-1000, 100));
  split1.setValue(MyMoneyMoney(-1000, 100));
  t.addSplit(split1);

  ft.restart();
  m->addTransaction(t);
  ft.commit();

  QVERIFY(m->hasNewerTransaction(accId, dateOfLastTransactionImport) == true);
}

void MyMoneyFileTest::AddOneAccount()
{
  QString accountId = "A000001";
  MyMoneyAccount  a;
  a.setAccountType(MyMoneyAccount::Checkings);

  storage->m_dirty = false;

  QVERIFY(m->accountCount() == 5);

  a.setName("Account1");
  a.setCurrencyId("EUR");

  clearObjectLists();
  MyMoneyFileTransaction ft;
  try {
    MyMoneyAccount parent = m->asset();
    m->addAccount(a, parent);
    ft.commit();
    QVERIFY(m->accountCount() == 6);
    QVERIFY(a.parentAccountId() == "AStd::Asset");
    QVERIFY(a.id() == accountId);
    QVERIFY(a.currencyId() == "EUR");
    QVERIFY(m->dirty() == true);
    QVERIFY(m->asset().accountList().count() == 1);
    QVERIFY(m->asset().accountList()[0] == accountId);

    QVERIFY(m_objectsRemoved.count() == 0);
    QVERIFY(m_objectsAdded.count() == 1);
    QVERIFY(m_objectsModified.count() == 1);
    QVERIFY(m_balanceChanged.count() == 0);
    QVERIFY(m_valueChanged.count() == 0);
    QVERIFY(m_objectsAdded.contains(accountId.toLatin1()));
    QVERIFY(m_objectsModified.contains(QLatin1String("AStd::Asset")));

  } catch (MyMoneyException *e) {
    delete e;
    QFAIL("Unexpected exception!");
  }
}

void MyMoneyFileTest::testCountTransactionsWithSpecificReconciliationState_noTransactions()
{
  AddOneAccount();
  QString accountId = "A000001";

  QVERIFY(m->countTransactionsWithSpecificReconciliationState(accountId, MyMoneyTransactionFilter::notReconciled) == 0);
}

void MyMoneyFileTest::testCountTransactionsWithSpecificReconciliationState_transactionWithWantedReconcileState()
{
  AddOneAccount();
  QString accountId = "A000001";

  // construct split & transaction
  MyMoneySplit split;
  split.setAccountId(accountId);
  split.setShares(MyMoneyMoney(-1000, 100));
  split.setValue(MyMoneyMoney(-1000, 100));

  MyMoneyTransaction transaction;
  transaction.setPostDate(QDate(2013, 1, 1));
  transaction.addSplit(split);

  // add transaction
  MyMoneyFileTransaction ft;
  m->addTransaction(transaction);
  ft.commit();

  QVERIFY(m->countTransactionsWithSpecificReconciliationState(accountId, MyMoneyTransactionFilter::notReconciled) == 1);
}

void MyMoneyFileTest::testCountTransactionsWithSpecificReconciliationState_transactionWithUnwantedReconcileState()
{
  AddOneAccount();
  QString accountId = "A000001";

  // construct split & transaction
  MyMoneySplit split;
  split.setAccountId(accountId);
  split.setShares(MyMoneyMoney(-1000, 100));
  split.setValue(MyMoneyMoney(-1000, 100));
  split.setReconcileFlag(MyMoneySplit::Reconciled);

  MyMoneyTransaction transaction;
  transaction.setPostDate(QDate(2013, 1, 1));
  transaction.addSplit(split);

  // add transaction
  MyMoneyFileTransaction ft;
  m->addTransaction(transaction);
  ft.commit();

  QVERIFY(m->countTransactionsWithSpecificReconciliationState(accountId, MyMoneyTransactionFilter::notReconciled) == 0);
}

#include "mymoneyfiletest.moc"

