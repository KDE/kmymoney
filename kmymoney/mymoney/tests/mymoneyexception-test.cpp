
/***************************************************************************
                          mymoneyexceptiontest.cpp
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

#include "mymoneyexception-test.h"

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
  QVERIFY(QString::fromLatin1(e.what()).startsWith("Message"));
  QVERIFY(QString::fromLatin1(e.what()).contains(QString::number(__LINE__ - 2)));
  QVERIFY(QString::fromLatin1(e.what()).contains(QString::fromLatin1(__FILE__)));
}

void MyMoneyExceptionTest::testCatching()
{
  try {
    throw MYMONEYEXCEPTION_CSTRING("Message");
  } catch(const MyMoneyException& e) {
    QVERIFY(QString::fromLatin1(e.what()).startsWith("Message"));
    QVERIFY(QString::fromLatin1(e.what()).contains(QString::number(__LINE__ - 3)));
    QVERIFY(QString::fromLatin1(e.what()).contains(QString::fromLatin1(__FILE__)));
  } catch(...) {
    QFAIL("Catching MyMoneyException does not work properly");
  }
}
