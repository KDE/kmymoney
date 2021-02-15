/*
    SPDX-FileCopyrightText: 2002-2010 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2006 Ace Jones <acejones@users.sourceforge.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "mymoneyinstitution-test.h"

#include <QtTest>
#include <QDomDocument>
#include <QDomElement>

#define KMM_MYMONEY_UNIT_TESTABLE friend class MyMoneyInstitutionTest;

#include "mymoneyexception.h"
#include "mymoneyinstitution.h"
#include "mymoneyinstitution_p.h"

QTEST_GUILESS_MAIN(MyMoneyInstitutionTest)

void MyMoneyInstitutionTest::init()
{
  m = new MyMoneyInstitution();
  n = new MyMoneyInstitution("name", "town", "street", "postcode",
                             "telephone", "manager", "sortcode");
}

void MyMoneyInstitutionTest::cleanup()
{
  delete m;
  delete n;
}

void MyMoneyInstitutionTest::testEmptyConstructor()
{
  QVERIFY(m->id().isEmpty());
  QVERIFY(m->street().isEmpty());
  QVERIFY(m->town().isEmpty());
  QVERIFY(m->postcode().isEmpty());
  QVERIFY(m->telephone().isEmpty());
  QVERIFY(m->manager().isEmpty());

  QVERIFY(m->accountCount() == 0);
}

void MyMoneyInstitutionTest::testSetFunctions()
{
  m->setStreet("street");
  m->setTown("town");
  m->setPostcode("postcode");
  m->setTelephone("telephone");
  m->setManager("manager");
  m->setName("name");

  QVERIFY(m->id().isEmpty());
  QVERIFY(m->street() == "street");
  QVERIFY(m->town() == "town");
  QVERIFY(m->postcode() == "postcode");
  QVERIFY(m->telephone() == "telephone");
  QVERIFY(m->manager() == "manager");
  QVERIFY(m->name() == "name");
}

void MyMoneyInstitutionTest::testNonemptyConstructor()
{
  QVERIFY(n->id().isEmpty());
  QVERIFY(n->street() == "street");
  QVERIFY(n->town() == "town");
  QVERIFY(n->postcode() == "postcode");
  QVERIFY(n->telephone() == "telephone");
  QVERIFY(n->manager() == "manager");
  QVERIFY(n->name() == "name");
  QVERIFY(n->sortcode() == "sortcode");
}

void MyMoneyInstitutionTest::testCopyConstructor()
{
  QScopedPointer<MyMoneyInstitution> n1 (new MyMoneyInstitution("GUID1", *n));
  MyMoneyInstitution n2(*n1);

  QVERIFY(*n1 == n2);
}

void MyMoneyInstitutionTest::testMyMoneyFileConstructor()
{
  QScopedPointer<MyMoneyInstitution> t (new MyMoneyInstitution("GUID", *n));

  QVERIFY(t->id() == "GUID");

  QVERIFY(t->street() == "street");
  QVERIFY(t->town() == "town");
  QVERIFY(t->postcode() == "postcode");
  QVERIFY(t->telephone() == "telephone");
  QVERIFY(t->manager() == "manager");
  QVERIFY(t->name() == "name");
  QVERIFY(t->sortcode() == "sortcode");
}

void MyMoneyInstitutionTest::testEquality()
{
  MyMoneyInstitution t("name", "town", "street", "postcode",
                       "telephone", "manager", "sortcode");

  QVERIFY(t == *n);
  t.setStreet("x");
  QVERIFY(!(t == *n));
  t.setStreet("street");
  QVERIFY(t == *n);
  t.setName("x");
  QVERIFY(!(t == *n));
  t.setName("name");
  QVERIFY(t == *n);
  t.setTown("x");
  QVERIFY(!(t == *n));
  t.setTown("town");
  QVERIFY(t == *n);
  t.setPostcode("x");
  QVERIFY(!(t == *n));
  t.setPostcode("postcode");
  QVERIFY(t == *n);
  t.setTelephone("x");
  QVERIFY(!(t == *n));
  t.setTelephone("telephone");
  QVERIFY(t == *n);
  t.setManager("x");
  QVERIFY(!(t == *n));
  t.setManager("manager");
  QVERIFY(t == *n);

  QScopedPointer<MyMoneyInstitution> n1 (new MyMoneyInstitution("GUID1", *n));
  QScopedPointer<MyMoneyInstitution> n2 (new MyMoneyInstitution("GUID1", *n));

  n1->addAccountId("A000001");
  n2->addAccountId("A000001");

  QVERIFY(*n1 == *n2);
}

void MyMoneyInstitutionTest::testInequality()
{
  QScopedPointer<MyMoneyInstitution> n1 (new MyMoneyInstitution("GUID0", *n));
  QScopedPointer<MyMoneyInstitution> n2 (new MyMoneyInstitution("GUID1", *n));
  QScopedPointer<MyMoneyInstitution> n3 (new MyMoneyInstitution("GUID2", *n));
  QScopedPointer<MyMoneyInstitution> n4 (new MyMoneyInstitution("GUID2", *n));

  QVERIFY(!(*n1 == *n2));
  QVERIFY(!(*n1 == *n3));
  QVERIFY(*n3 == *n4);

  n3->addAccountId("A000001");
  n4->addAccountId("A000002");
  QVERIFY(!(*n3 == *n4));
}

void MyMoneyInstitutionTest::testAccountIDList()
{
  MyMoneyInstitution institution;
  QStringList list;
  QString id;

  // list must be empty
  list = institution.accountList();
  QVERIFY(list.count() == 0);

  // add one account
  institution.addAccountId("A000002");
  list = institution.accountList();
  QVERIFY(list.count() == 1);
  QVERIFY(list.contains("A000002") == 1);

  // adding same account shouldn't make a difference
  institution.addAccountId("A000002");
  list = institution.accountList();
  QVERIFY(list.count() == 1);
  QVERIFY(list.contains("A000002") == 1);

  // now add another account
  institution.addAccountId("A000001");
  list = institution.accountList();
  QVERIFY(list.count() == 2);
  QVERIFY(list.contains("A000002") == 1);
  QVERIFY(list.contains("A000001") == 1);

  id = institution.removeAccountId("A000001");
  QVERIFY(id == "A000001");
  list = institution.accountList();
  QVERIFY(list.count() == 1);
  QVERIFY(list.contains("A000002") == 1);

}
