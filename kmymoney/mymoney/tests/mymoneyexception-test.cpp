/*
 * SPDX-FileCopyrightText: 2002-2018 Thomas Baumgart <tbaumgart@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "mymoneyexception-test.h"
#include "mymoneyexception.h"

#include <QtTest>

QTEST_GUILESS_MAIN(MyMoneyExceptionTest)

void MyMoneyExceptionTest::init()
{
}

void MyMoneyExceptionTest::cleanup()
{
}

void MyMoneyExceptionTest::testDefaultConstructor()
{
  const MyMoneyException &e = MYMONEYEXCEPTION_CSTRING("Message");
  QVERIFY(QString::fromLatin1(e.what()).startsWith(QLatin1String("Message")));
  QVERIFY(QString::fromLatin1(e.what()).contains(QString::number(__LINE__ - 2)));
  QVERIFY(QString::fromLatin1(e.what()).contains(QString::fromLatin1(__FILE__)));
}

void MyMoneyExceptionTest::testCatching()
{
  try {
    throw MYMONEYEXCEPTION_CSTRING("Message");
  } catch(const MyMoneyException& e) {
    QVERIFY(QString::fromLatin1(e.what()).startsWith(QLatin1String("Message")));
    QVERIFY(QString::fromLatin1(e.what()).contains(QString::number(__LINE__ - 3)));
    QVERIFY(QString::fromLatin1(e.what()).contains(QString::fromLatin1(__FILE__)));
  } catch(...) {
    QFAIL("Catching MyMoneyException does not work properly");
  }
}
