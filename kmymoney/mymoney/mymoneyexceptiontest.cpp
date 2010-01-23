
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

#include "mymoneyexceptiontest.h"

MyMoneyExceptionTest::MyMoneyExceptionTest()
{
}


void MyMoneyExceptionTest::setUp()
{
}

void MyMoneyExceptionTest::tearDown()
{
}

void MyMoneyExceptionTest::testDefaultConstructor()
{
  MyMoneyException *e = new MYMONEYEXCEPTION("Message");
  CPPUNIT_ASSERT(e->what() == "Message");
  CPPUNIT_ASSERT(e->line() == __LINE__ -2);
  CPPUNIT_ASSERT(e->file() == __FILE__);
  delete e;
}

void MyMoneyExceptionTest::testConstructor()
{
  MyMoneyException *e = new MyMoneyException("New message",
      "Joe's file", 1234);
  CPPUNIT_ASSERT(e->what() == "New message");
  CPPUNIT_ASSERT(e->line() == 1234);
  CPPUNIT_ASSERT(e->file() == "Joe's file");
  delete e;
}

