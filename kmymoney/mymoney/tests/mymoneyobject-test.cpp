/*
 * Copyright 2005-2011  Thomas Baumgart <tbaumgart@kde.org>
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

#include "mymoneyobject-test.h"

#include <QtTest>
#include <QDomDocument>
#include <QDomElement>

#include "mymoneyobject_p.h"
#include "mymoneyexception.h"
#include "mymoneyaccount.h"

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
