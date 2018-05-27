/*
 * Copyright 2002-2013  Thomas Baumgart <tbaumgart@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "mymoneykeyvaluecontainer-test.h"

#include <QtTest>
#include <QDomDocument>
#include <QDomElement>

#define KMM_MYMONEY_UNIT_TESTABLE friend class MyMoneyKeyValueContainerTest;

#include "mymoneyexception.h"
#include "mymoneykeyvaluecontainer.h"
#include "mymoneykeyvaluecontainer_p.h"

QTEST_GUILESS_MAIN(MyMoneyKeyValueContainerTest)

void MyMoneyKeyValueContainerTest::init()
{
  m = new MyMoneyKeyValueContainer;
}

void MyMoneyKeyValueContainerTest::cleanup()
{
  delete m;
}

void MyMoneyKeyValueContainerTest::testEmptyConstructor()
{
  QVERIFY(m->d_func()->m_kvp.count() == 0);
}

void MyMoneyKeyValueContainerTest::testRetrieveValue()
{
  // load a value into the container
  m->d_func()->m_kvp["Key"] = "Value";
  // make sure it's there
  QVERIFY(m->d_func()->m_kvp.count() == 1);
  QVERIFY(m->d_func()->m_kvp["Key"] == "Value");
  // now check that the access function works
  QVERIFY(m->value("Key") == "Value");
  QVERIFY(m->value("key").isEmpty());
}

void MyMoneyKeyValueContainerTest::testSetValue()
{
  m->setValue("Key", "Value");
  QVERIFY(m->d_func()->m_kvp.count() == 1);
  QVERIFY(m->d_func()->m_kvp["Key"] == "Value");
}

void MyMoneyKeyValueContainerTest::testDeletePair()
{
  m->setValue("Key", "Value");
  m->setValue("key", "value");
  QVERIFY(m->d_func()->m_kvp.count() == 2);
  m->deletePair("Key");
  QVERIFY(m->d_func()->m_kvp.count() == 1);
  QVERIFY(m->value("Key").isEmpty());
  QVERIFY(m->value("key") == "value");
}

void MyMoneyKeyValueContainerTest::testClear()
{
  m->setValue("Key", "Value");
  m->setValue("key", "value");
  QVERIFY(m->d_func()->m_kvp.count() == 2);
  m->clear();
  QVERIFY(m->d_func()->m_kvp.count() == 0);
}

void MyMoneyKeyValueContainerTest::testRetrieveList()
{
  QMap<QString, QString> copy;

  copy = m->d_func()->m_kvp;
  QVERIFY(copy.count() == 0);
  m->setValue("Key", "Value");
  m->setValue("key", "value");
  copy = m->d_func()->m_kvp;
  QVERIFY(copy.count() == 2);
  QVERIFY(copy["Key"] == "Value");
  QVERIFY(copy["key"] == "value");
}

void MyMoneyKeyValueContainerTest::testLoadList()
{
  m->setValue("Key", "Value");
  m->setValue("key", "value");

  QVERIFY(m->d_func()->m_kvp.count() == 2);
  QVERIFY(m->d_func()->m_kvp["Key"] == "Value");
  QVERIFY(m->d_func()->m_kvp["key"] == "value");
}

void MyMoneyKeyValueContainerTest::testWriteXML()
{
  m->setValue("Key", "Value");
  m->setValue("key", "value");

  QDomDocument doc("TEST");
  QDomElement el = doc.createElement("KVP-CONTAINER");
  doc.appendChild(el);
  m->writeXML(doc, el);

  QCOMPARE(doc.doctype().name(), QLatin1String("TEST"));
  QDomElement kvpContainer = doc.documentElement();
  QCOMPARE(kvpContainer.tagName(), QLatin1String("KVP-CONTAINER"));
  QCOMPARE(kvpContainer.childNodes().size(), 1);

  QVERIFY(kvpContainer.childNodes().at(0).isElement());
  QDomElement keyValuePairs = kvpContainer.childNodes().at(0).toElement();
  QCOMPARE(keyValuePairs.tagName(), QLatin1String("KEYVALUEPAIRS"));
  QCOMPARE(keyValuePairs.childNodes().size(), 2);

  QVERIFY(keyValuePairs.childNodes().at(0).isElement());
  QDomElement keyValuePair1 = keyValuePairs.childNodes().at(0).toElement();
  QCOMPARE(keyValuePair1.tagName(), QLatin1String("PAIR"));
  QCOMPARE(keyValuePair1.attribute("key"), QLatin1String("Key"));
  QCOMPARE(keyValuePair1.attribute("value"), QLatin1String("Value"));
  QCOMPARE(keyValuePair1.childNodes().size(), 0);

  QVERIFY(keyValuePairs.childNodes().at(1).isElement());
  QDomElement keyValuePair2 = keyValuePairs.childNodes().at(1).toElement();
  QCOMPARE(keyValuePair2.tagName(), QLatin1String("PAIR"));
  QCOMPARE(keyValuePair2.attribute("key"), QLatin1String("key"));
  QCOMPARE(keyValuePair2.attribute("value"), QLatin1String("value"));
  QCOMPARE(keyValuePair2.childNodes().size(), 0);
}

void MyMoneyKeyValueContainerTest::testReadXML()
{
  m->setValue("Key", "Value");
  m->setValue("key", "value");

  QString ref_ok(
    "<!DOCTYPE TEST>\n"
    "<KVP-CONTAINER>\n"
    " <KEYVALUEPAIRS>\n"
    "  <PAIR key=\"key\" value=\"Value\" />\n"
    "  <PAIR key=\"Key\" value=\"value\" />\n"
    " </KEYVALUEPAIRS>\n"
    "</KVP-CONTAINER>\n");

  QString ref_false(
    "<!DOCTYPE TEST>\n"
    "<KVP-CONTAINER>\n"
    " <KEYVALUE-PAIRS>\n"
    "  <PAIR key=\"key\" value=\"Value\" />\n"
    "  <PAIR key=\"Key\" value=\"value\" />\n"
    " </KEYVALUE-PAIRS>\n"
    "</KVP-CONTAINER>\n");


  QDomDocument doc;
  QDomElement node;
  doc.setContent(ref_false);
  node = doc.documentElement().firstChild().toElement();

  // make sure, an empty node does not trigger an exception
  try {
    QDomElement e;
    MyMoneyKeyValueContainer k(e);
  } catch (const MyMoneyException &) {
    QFAIL("Unexpected exception");
  }

  try {
    MyMoneyKeyValueContainer k(node);
    QFAIL("Missing expected exception");
  } catch (const MyMoneyException &) {
  }

  doc.setContent(ref_ok);
  node = doc.documentElement().firstChild().toElement();
  try {
    MyMoneyKeyValueContainer k(node);
    QVERIFY(k.d_func()->m_kvp.count() == 2);
    QVERIFY(k.value("key") == "Value");
    QVERIFY(k.value("Key") == "value");
  } catch (const MyMoneyException &) {
    QFAIL("Unexpected exception");
  }
}

void MyMoneyKeyValueContainerTest::testArrayRead()
{
  MyMoneyKeyValueContainer kvp;
  const MyMoneyKeyValueContainer& ckvp = kvp;
  QVERIFY(kvp.pairs().count() == 0);
  QVERIFY(ckvp["Key"].isEmpty());
  QVERIFY(kvp.pairs().count() == 0);
  kvp.setValue("Key", "Value");
  QVERIFY(kvp["Key"] == "Value");
}

void MyMoneyKeyValueContainerTest::testArrayWrite()
{
  MyMoneyKeyValueContainer kvp;
  kvp["Key"] = "Value";
  QVERIFY(kvp.pairs().count() == 1);
  QVERIFY(kvp.value("Key") == "Value");
}

void MyMoneyKeyValueContainerTest::testElementNames()
{
  for (auto i = (int)KVC::Element::Pair; i <= (int)KVC::Element::Pair; ++i) {
    auto isEmpty = MyMoneyKeyValueContainerPrivate::getElName(static_cast<KVC::Element>(i)).isEmpty();
    if (isEmpty)
      qWarning() << "Empty element's name " << i;
    QVERIFY(!isEmpty);
  }
}

void MyMoneyKeyValueContainerTest::testAttributeNames()
{
  for (auto i = (int)KVC::Attribute::Key; i < (int)KVC::Attribute::LastAttribute; ++i) {
    auto isEmpty = MyMoneyKeyValueContainerPrivate::getAttrName(static_cast<KVC::Attribute>(i)).isEmpty();
    if (isEmpty)
      qWarning() << "Empty attribute's name " << i;
    QVERIFY(!isEmpty);
  }
}
