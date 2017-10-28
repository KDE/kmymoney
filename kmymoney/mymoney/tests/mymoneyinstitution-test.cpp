/***************************************************************************
                          mymoneyinstitutiontest.cpp
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

#include "mymoneyinstitution-test.h"

#include <QtTest/QtTest>

#define KMM_MYMONEY_UNIT_TESTABLE friend class MyMoneyInstitutionTest;

#include "mymoneyexception.h"
#include "mymoneyinstitution.h"

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
  MyMoneyInstitution* n1 = new MyMoneyInstitution("GUID1", *n);
  MyMoneyInstitution n2(*n1);

  QVERIFY(*n1 == n2);

  delete n1;
}

void MyMoneyInstitutionTest::testMyMoneyFileConstructor()
{
  MyMoneyInstitution *t = new MyMoneyInstitution("GUID", *n);

  QVERIFY(t->id() == "GUID");

  QVERIFY(t->street() == "street");
  QVERIFY(t->town() == "town");
  QVERIFY(t->postcode() == "postcode");
  QVERIFY(t->telephone() == "telephone");
  QVERIFY(t->manager() == "manager");
  QVERIFY(t->name() == "name");
  QVERIFY(t->sortcode() == "sortcode");

  delete t;
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

  MyMoneyInstitution* n1 = new MyMoneyInstitution("GUID1", *n);
  MyMoneyInstitution* n2 = new MyMoneyInstitution("GUID1", *n);

  n1->addAccountId("A000001");
  n2->addAccountId("A000001");

  QVERIFY(*n1 == *n2);

  delete n1;
  delete n2;
}

void MyMoneyInstitutionTest::testInequality()
{
  MyMoneyInstitution* n1 = new MyMoneyInstitution("GUID0", *n);
  MyMoneyInstitution* n2 = new MyMoneyInstitution("GUID1", *n);
  MyMoneyInstitution* n3 = new MyMoneyInstitution("GUID2", *n);
  MyMoneyInstitution* n4 = new MyMoneyInstitution("GUID2", *n);

  QVERIFY(!(*n1 == *n2));
  QVERIFY(!(*n1 == *n3));
  QVERIFY(*n3 == *n4);

  n3->addAccountId("A000001");
  n4->addAccountId("A000002");
  QVERIFY(!(*n3 == *n4));

  delete n1;
  delete n2;
  delete n3;
  delete n4;
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

void MyMoneyInstitutionTest::testWriteXML()
{
  MyMoneyKeyValueContainer kvp;

  n->addAccountId("A000001");
  n->addAccountId("A000003");
  n->setValue(QString("key"), "value");

  QDomDocument doc("TEST");
  QDomElement el = doc.createElement("INSTITUTION-CONTAINER");
  doc.appendChild(el);

  MyMoneyInstitution i("I00001", *n);

  i.writeXML(doc, el);

  QCOMPARE(doc.doctype().name(), QLatin1String("TEST"));
  QDomElement institutionContainer = doc.documentElement();
  QVERIFY(institutionContainer.isElement());
  QCOMPARE(institutionContainer.tagName(), QLatin1String("INSTITUTION-CONTAINER"));
  QVERIFY(institutionContainer.childNodes().size() == 1);
  QVERIFY(institutionContainer.elementsByTagName("INSTITUTION").at(0).isElement());

  QDomElement institution = institutionContainer.elementsByTagName("INSTITUTION").at(0).toElement();
  QCOMPARE(institution.tagName(), QLatin1String("INSTITUTION"));
  QCOMPARE(institution.attribute("id"), QLatin1String("I00001"));
  QCOMPARE(institution.attribute("manager"), QLatin1String("manager"));
  QCOMPARE(institution.attribute("name"), QLatin1String("name"));
  QCOMPARE(institution.attribute("sortcode"), QLatin1String("sortcode"));
  QCOMPARE(institution.childNodes().size(), 3);

  QVERIFY(institution.childNodes().at(0).isElement());
  QDomElement address = institution.childNodes().at(0).toElement();
  QCOMPARE(address.tagName(), QLatin1String("ADDRESS"));
  QCOMPARE(address.attribute("street"), QLatin1String("street"));
  QCOMPARE(address.attribute("telephone"), QLatin1String("telephone"));
  QCOMPARE(address.attribute("zip"), QLatin1String("postcode"));
  QCOMPARE(address.attribute("city"), QLatin1String("town"));
  QCOMPARE(address.childNodes().size(), 0);

  QVERIFY(institution.childNodes().at(1).isElement());
  QDomElement accountIds = institution.childNodes().at(1).toElement();
  QCOMPARE(accountIds.tagName(), QLatin1String("ACCOUNTIDS"));
  QCOMPARE(accountIds.childNodes().size(), 2);

  QVERIFY(accountIds.childNodes().at(0).isElement());
  QDomElement account1 = accountIds.childNodes().at(0).toElement();
  QCOMPARE(account1.tagName(), QLatin1String("ACCOUNTID"));
  QCOMPARE(account1.attribute("id"), QLatin1String("A000001"));
  QCOMPARE(account1.childNodes().size(), 0);

  QVERIFY(accountIds.childNodes().at(1).isElement());
  QDomElement account2 = accountIds.childNodes().at(1).toElement();
  QCOMPARE(account2.tagName(), QLatin1String("ACCOUNTID"));
  QCOMPARE(account2.attribute("id"), QLatin1String("A000003"));
  QCOMPARE(account2.childNodes().size(), 0);

  QVERIFY(institution.childNodes().at(2).isElement());
  QDomElement keyValuePairs = institution.childNodes().at(2).toElement();
  QCOMPARE(keyValuePairs.tagName(), QLatin1String("KEYVALUEPAIRS"));
  QCOMPARE(keyValuePairs.childNodes().size(), 1);

  QVERIFY(keyValuePairs.childNodes().at(0).isElement());
  QDomElement keyValuePair1 = keyValuePairs.childNodes().at(0).toElement();
  QCOMPARE(keyValuePair1.tagName(), QLatin1String("PAIR"));
  QCOMPARE(keyValuePair1.attribute("key"), QLatin1String("key"));
  QCOMPARE(keyValuePair1.attribute("value"), QLatin1String("value"));
  QCOMPARE(keyValuePair1.childNodes().size(), 0);
}

void MyMoneyInstitutionTest::testReadXML()
{
  MyMoneyInstitution i;
  QString ref_ok = QString(
                     "<!DOCTYPE TEST>\n"
                     "<INSTITUTION-CONTAINER>\n"
                     " <INSTITUTION sortcode=\"sortcode\" id=\"I00001\" manager=\"manager\" name=\"name\" >\n"
                     "  <ADDRESS street=\"street\" zip=\"postcode\" city=\"town\" telephone=\"telephone\" />\n"
                     "  <ACCOUNTIDS>\n"
                     "   <ACCOUNTID id=\"A000001\" />\n"
                     "   <ACCOUNTID id=\"A000003\" />\n"
                     "  </ACCOUNTIDS>\n"
                     "  <KEYVALUEPAIRS>\n"
                     "   <PAIR key=\"key\" value=\"value\" />\n"
                     "  </KEYVALUEPAIRS>\n"
                     " </INSTITUTION>\n"
                     "</INSTITUTION-CONTAINER>\n");

  QString ref_false = QString(
                        "<!DOCTYPE TEST>\n"
                        "<INSTITUTION-CONTAINER>\n"
                        " <KINSTITUTION sortcode=\"sortcode\" id=\"I00001\" manager=\"manager\" name=\"name\" >\n"
                        "  <ADDRESS street=\"street\" zip=\"postcode\" city=\"town\" telephone=\"telephone\" />\n"
                        "  <ACCOUNTIDS>\n"
                        "   <ACCOUNTID id=\"A000001\" />\n"
                        "   <ACCOUNTID id=\"A000003\" />\n"
                        "  </ACCOUNTIDS>\n"
                        " </KINSTITUTION>\n"
                        "</INSTITUTION-CONTAINER>\n");

  QDomDocument doc;
  QDomElement node;

  doc.setContent(ref_false);
  node = doc.documentElement().firstChild().toElement();
  try {
    i = MyMoneyInstitution(node);
    QFAIL("Missing expected exception");
  } catch (const MyMoneyException &) {
  }

  i.addAccountId("TEST");

  doc.setContent(ref_ok);
  node = doc.documentElement().firstChild().toElement();
  try {
    QStringList alist;
    alist << "A000001" << "A000003";
    i = MyMoneyInstitution(node);

    QVERIFY(i.sortcode() == "sortcode");
    QVERIFY(i.id() == "I00001");
    QVERIFY(i.manager() == "manager");
    QVERIFY(i.name() == "name");
    QVERIFY(i.street() == "street");
    QVERIFY(i.postcode() == "postcode");
    QVERIFY(i.city() == "town");
    QVERIFY(i.telephone() == "telephone");
    QVERIFY(i.accountList() == alist);
    QVERIFY(i.value(QString("key")) == "value");

  } catch (const MyMoneyException &) {
    QFAIL("Unexpected exception");
  }
}

void MyMoneyInstitutionTest::testElementNames()
{  
  for (auto i = (int)MyMoneyInstitution::Element::AccountID; i <= (int)MyMoneyInstitution::Element::Address; ++i) {
    auto isEmpty = MyMoneyInstitution::getElName(static_cast<MyMoneyInstitution::Element>(i)).isEmpty();
    if (isEmpty)
      qWarning() << "Empty element's name " << i;
    QVERIFY(!isEmpty);
  }

}

void MyMoneyInstitutionTest::testAttributeNames()
{
  for (auto i = (int)MyMoneyInstitution::Attribute::ID; i < (int)MyMoneyInstitution::Attribute::LastAttribute; ++i) {
    auto isEmpty = MyMoneyInstitution::getAttrName(static_cast<MyMoneyInstitution::Attribute>(i)).isEmpty();
    if (isEmpty)
      qWarning() << "Empty attribute's name " << i;
    QVERIFY(!isEmpty);
  }
}
