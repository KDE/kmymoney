/***************************************************************************
                          mymoneykeyvaluecontainertest.cpp
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

#include "mymoneykeyvaluecontainertest.h"
#include <mymoneyexception.h>

MyMoneyKeyValueContainerTest::MyMoneyKeyValueContainerTest()
{
}


void MyMoneyKeyValueContainerTest::setUp()
{
  m = new MyMoneyKeyValueContainer;
}

void MyMoneyKeyValueContainerTest::tearDown()
{
  delete m;
}

void MyMoneyKeyValueContainerTest::testEmptyConstructor()
{
  CPPUNIT_ASSERT(m->m_kvp.count() == 0);
}

void MyMoneyKeyValueContainerTest::testRetrieveValue()
{
  // load a value into the container
  m->m_kvp["Key"] = "Value";
  // make sure it's there
  CPPUNIT_ASSERT(m->m_kvp.count() == 1);
  CPPUNIT_ASSERT(m->m_kvp["Key"] == "Value");
  // now check that the access function works
  CPPUNIT_ASSERT(m->value("Key") == "Value");
  CPPUNIT_ASSERT(m->value("key").isEmpty());
}

void MyMoneyKeyValueContainerTest::testSetValue()
{
  m->setValue("Key", "Value");
  CPPUNIT_ASSERT(m->m_kvp.count() == 1);
  CPPUNIT_ASSERT(m->m_kvp["Key"] == "Value");
}

void MyMoneyKeyValueContainerTest::testDeletePair()
{
  m->setValue("Key", "Value");
  m->setValue("key", "value");
  CPPUNIT_ASSERT(m->m_kvp.count() == 2);
  m->deletePair("Key");
  CPPUNIT_ASSERT(m->m_kvp.count() == 1);
  CPPUNIT_ASSERT(m->value("Key").isEmpty());
  CPPUNIT_ASSERT(m->value("key") == "value");
}

void MyMoneyKeyValueContainerTest::testClear()
{
  m->setValue("Key", "Value");
  m->setValue("key", "value");
  CPPUNIT_ASSERT(m->m_kvp.count() == 2);
  m->clear();
  CPPUNIT_ASSERT(m->m_kvp.count() == 0);
}

void MyMoneyKeyValueContainerTest::testRetrieveList()
{
  QMap<QString, QString> copy;

  copy = m->pairs();
  CPPUNIT_ASSERT(copy.count() == 0);
  m->setValue("Key", "Value");
  m->setValue("key", "value");
  copy = m->pairs();
  CPPUNIT_ASSERT(copy.count() == 2);
  CPPUNIT_ASSERT(copy["Key"] == "Value");
  CPPUNIT_ASSERT(copy["key"] == "value");
}

void MyMoneyKeyValueContainerTest::testLoadList()
{
  m->setValue("Key", "Value");
  m->setValue("key", "value");

  CPPUNIT_ASSERT(m->m_kvp.count() == 2);
  CPPUNIT_ASSERT(m->m_kvp["Key"] == "Value");
  CPPUNIT_ASSERT(m->m_kvp["key"] == "value");
}

void MyMoneyKeyValueContainerTest::testWriteXML()
{
  m->setValue("Key", "Value");
  m->setValue("key", "value");

  QDomDocument doc("TEST");
  QDomElement el = doc.createElement("KVP-CONTAINER");
  doc.appendChild(el);
  m->writeXML(doc, el);

  QString ref(
    "<!DOCTYPE TEST>\n"
    "<KVP-CONTAINER>\n"
    " <KEYVALUEPAIRS>\n"
    "  <PAIR key=\"Key\" value=\"Value\" />\n"
    "  <PAIR key=\"key\" value=\"value\" />\n"
    " </KEYVALUEPAIRS>\n"
    "</KVP-CONTAINER>\n");

#if QT_VERSION >= QT_VERSION_CHECK(4,6,0)
  ref.replace(QString(" />\n"), QString("/>\n"));
  ref.replace(QString(" >\n"), QString(">\n"));
#endif

  CPPUNIT_ASSERT(doc.toString() == ref);
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
    MyMoneyKeyValueContainer k(QDomNode());
  } catch (MyMoneyException *e) {
    CPPUNIT_FAIL("Unexpected exception");
    delete e;
  }

  try {
    MyMoneyKeyValueContainer k(node);
    CPPUNIT_FAIL("Missing expected exception");
  } catch (MyMoneyException *e) {
    delete e;
  }

  doc.setContent(ref_ok);
  node = doc.documentElement().firstChild().toElement();
  try {
    MyMoneyKeyValueContainer k(node);
    CPPUNIT_ASSERT(k.m_kvp.count() == 2);
    CPPUNIT_ASSERT(k.value("key") == "Value");
    CPPUNIT_ASSERT(k.value("Key") == "value");
  } catch (MyMoneyException *e) {
    delete e;
    CPPUNIT_FAIL("Unexpected exception");
  }
}

void MyMoneyKeyValueContainerTest::testArrayRead()
{
  MyMoneyKeyValueContainer kvp;
  const MyMoneyKeyValueContainer& ckvp = kvp;
  CPPUNIT_ASSERT(kvp.pairs().count() == 0);
  CPPUNIT_ASSERT(ckvp["Key"].isEmpty());
  CPPUNIT_ASSERT(kvp.pairs().count() == 0);
  kvp.setValue("Key", "Value");
  CPPUNIT_ASSERT(kvp["Key"] == "Value");
}

void MyMoneyKeyValueContainerTest::testArrayWrite()
{
  MyMoneyKeyValueContainer kvp;
  kvp["Key"] = "Value";
  CPPUNIT_ASSERT(kvp.pairs().count() == 1);
  CPPUNIT_ASSERT(kvp.value("Key") == "Value");
}

