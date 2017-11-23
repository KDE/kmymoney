/***************************************************************************
                          mymoneyobjecttest.cpp
                          -------------------
    copyright            : (C) 2005 by Thomas Baumgart
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

#include "mymoneyobject-test.h"

#include <QtTest>

#include "mymoneyexception.h"
#include "mymoneyaccount.h"

class TestMyMoneyObject : public MyMoneyObject
{
public:
  TestMyMoneyObject() : MyMoneyObject() {}
  TestMyMoneyObject(const QDomElement& node, const bool forceId = true) :
      MyMoneyObject(node, forceId) {}
  virtual bool hasReferenceTo(const QString&) const {
    return false;
  }
  virtual void writeXML(QDomDocument&, QDomElement&) const {}
};

QTEST_GUILESS_MAIN(MyMoneyObjectTest)

void MyMoneyObjectTest::testEmptyConstructor()
{
  MyMoneyAccount a;
  QVERIFY(a.id().isEmpty());
}

void MyMoneyObjectTest::testConstructor()
{
  MyMoneyAccount a(QString("thb"), MyMoneyAccount());

  QVERIFY(!a.id().isEmpty());
  QVERIFY(a.id() == QString("thb"));
}

void MyMoneyObjectTest::testClearId()
{
  MyMoneyAccount a(QString("thb"), MyMoneyAccount());

  QVERIFY(!a.id().isEmpty());
  a.clearId();
  QVERIFY(a.id().isEmpty());
}

void MyMoneyObjectTest::testCopyConstructor()
{
  MyMoneyAccount a(QString("thb"), MyMoneyAccount());
  MyMoneyAccount b(a);

  QVERIFY(a.MyMoneyObject::operator==(b));
}

void MyMoneyObjectTest::testAssignmentConstructor()
{
  MyMoneyAccount a(QString("thb"), MyMoneyAccount());
  MyMoneyAccount b = a;

  QVERIFY(a.MyMoneyObject::operator==(b));
}

void MyMoneyObjectTest::testEquality()
{
  MyMoneyAccount a(QString("thb"), MyMoneyAccount());
  MyMoneyAccount b(QString("thb"), MyMoneyAccount());
  MyMoneyAccount c(QString("ace"), MyMoneyAccount());

  QVERIFY(a.MyMoneyObject::operator==(b));
  QVERIFY(!(a.MyMoneyObject::operator==(c)));
}

void MyMoneyObjectTest::testReadXML()
{
  TestMyMoneyObject t;

  QString ref_ok = QString(
                     "<!DOCTYPE TEST>\n"
                     "<TRANSACTION-CONTAINER>\n"
                     " <MYMONEYOBJECT id=\"T000000000000000001\" >\n"
                     " </MYMONEYOBJECT>\n"
                     "</TRANSACTION-CONTAINER>\n"
                   );

  QString ref_false1 = QString(
                         "<!DOCTYPE TEST>\n"
                         "<TRANSACTION-CONTAINER>\n"
                         " <MYMONEYOBJECT id=\"\" >\n"
                         " </MYMONEYOBJECT>\n"
                         "</TRANSACTION-CONTAINER>\n"
                       );

  QString ref_false2 = QString(
                         "<!DOCTYPE TEST>\n"
                         "<TRANSACTION-CONTAINER>\n"
                         " <MYMONEYOBJECT >\n"
                         " </MYMONEYOBJECT>\n"
                         "</TRANSACTION-CONTAINER>\n"
                       );

  QDomDocument doc;
  QDomElement node;

  // id="" but required
  doc.setContent(ref_false1);
  node = doc.documentElement().firstChild().toElement();

  try {
    t = TestMyMoneyObject(node);
    QFAIL("Missing expected exception");
  } catch (const MyMoneyException &) {
  }

  // id attribute missing but required
  doc.setContent(ref_false2);
  node = doc.documentElement().firstChild().toElement();

  try {
    t = TestMyMoneyObject(node);
    QFAIL("Missing expected exception");
  } catch (const MyMoneyException &) {
  }

  // id present
  doc.setContent(ref_ok);
  node = doc.documentElement().firstChild().toElement();

  try {
    t = TestMyMoneyObject(node);
    QVERIFY(t.id() == "T000000000000000001");
  } catch (const MyMoneyException &) {
    QFAIL("Unexpected exception");
  }

  // id="" but not required
  doc.setContent(ref_false1);
  node = doc.documentElement().firstChild().toElement();

  try {
    t = TestMyMoneyObject(node, false);
    QVERIFY(t.id().isEmpty());
  } catch (const MyMoneyException &) {
    QFAIL("Unexpected exception");
  }
}
