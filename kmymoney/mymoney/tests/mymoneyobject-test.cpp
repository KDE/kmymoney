/*
    SPDX-FileCopyrightText: 2005-2011 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
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
