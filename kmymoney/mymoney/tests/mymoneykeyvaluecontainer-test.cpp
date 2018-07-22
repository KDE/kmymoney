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
