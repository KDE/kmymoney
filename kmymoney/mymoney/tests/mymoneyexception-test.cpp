
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
  const MyMoneyException &e = MYMONEYEXCEPTION("Message");
  QVERIFY(e.what() == "Message");
  QVERIFY(e.line() == __LINE__ - 2);
  QVERIFY(e.file() == __FILE__);
}

void MyMoneyExceptionTest::testConstructor()
{
  MyMoneyException e("New message", "Joe's file", 1234);
  QVERIFY(e.what() == "New message");
  QVERIFY(e.line() == 1234);
  QVERIFY(e.file() == "Joe's file");
}
