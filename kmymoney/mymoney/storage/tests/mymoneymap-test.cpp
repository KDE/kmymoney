/*
 * Copyright 2007-2011  Thomas Baumgart <tbaumgart@kde.org>
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

#include "mymoneymap-test.h"
#include <iostream>
#include <QtTest>

QTEST_GUILESS_MAIN(MyMoneyMapTest)

void MyMoneyMapTest::init()
{
  m = new MyMoneyMap<QString, QString>;
}

void MyMoneyMapTest::cleanup()
{
  delete m;
}

void MyMoneyMapTest::testArrayOperator()
{
  QSKIP("Test not implemented yet", SkipAll);
}

void MyMoneyMapTest::testModifyKey()
{
  // setup
  m->startTransaction();
  m->insert("a", "a");
  m->commitTransaction();
  QVERIFY((*m)["a"] == "a");

  // commit
  m->startTransaction();
  m->modify("a", "b");
  QVERIFY((*m)["a"] == "b");
  m->commitTransaction();
  QVERIFY((*m)["a"] == "b");

  // rollback
  m->startTransaction();
  m->modify("a", "c");
  QVERIFY((*m)["a"] == "c");
  m->rollbackTransaction();
  QVERIFY((*m)["a"] == "b");
}

void MyMoneyMapTest::testModifyKeyTwice()
{
  m->startTransaction();
  m->insert("a", "a");
  QVERIFY((*m)["a"] == "a");
  m->insert("b", "b");
  QVERIFY((*m)["a"] == "a");
  QVERIFY((*m)["b"] == "b");

  m->modify("a", "b");
  QVERIFY((*m)["a"] == "b");

  m->rollbackTransaction();

  QVERIFY(m->count() == 0);

  m->startTransaction();
  m->insert("a", "a");
  m->insert("b", "b");
  m->commitTransaction();
  QVERIFY((*m)["a"] == "a");
  QVERIFY((*m)["b"] == "b");

  m->startTransaction();
  m->modify("a", "b");
  m->modify("a", "c");
  QVERIFY((*m)["a"] == "c");

  m->rollbackTransaction();
  QVERIFY((*m)["a"] == "a");
}
