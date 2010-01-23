/***************************************************************************
                          mymoneypricetest.cpp
                          -------------------
    copyright            : (C) 2005 by Thomas Baumgart
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

#include "mymoneypricetest.h"
#include "mymoneyexception.h"

MyMoneyPriceTest::MyMoneyPriceTest()
{
}


void MyMoneyPriceTest::setUp()
{
  m = new MyMoneyPrice();
}

void MyMoneyPriceTest::tearDown()
{
  delete m;
}

void MyMoneyPriceTest::testDefaultConstructor()
{
  CPPUNIT_ASSERT(m->isValid() == false);
}

void MyMoneyPriceTest::testConstructor()
{
  MyMoneyPrice n(QString("from"), QString("to"), QDate(2005, 9, 23), MyMoneyMoney(1, 3),  QString("MySource"));

  CPPUNIT_ASSERT(n.isValid() == true);
  CPPUNIT_ASSERT(n.from() == QString("from"));
  CPPUNIT_ASSERT(n.to() == QString("to"));
  CPPUNIT_ASSERT(n.date() == QDate(2005, 9, 23));
  CPPUNIT_ASSERT(n.source() == QString("MySource"));
  CPPUNIT_ASSERT(n.rate("to") == MyMoneyMoney(1, 3));
}

void MyMoneyPriceTest::testValidity()
{
  QString emptyId;
  MyMoneyPrice n1(emptyId, QString("to"), QDate(2005, 9, 23), MyMoneyMoney(1, 3),  QString("MySource"));
  MyMoneyPrice n2(QString("from"), emptyId, QDate(2005, 9, 23), MyMoneyMoney(1, 3),  QString("MySource"));
  MyMoneyPrice n3(QString("from"), QString("to"), QDate(), MyMoneyMoney(1, 3),  QString("MySource"));
  MyMoneyPrice n4(QString("from"), QString("to"), QDate(2005, 9, 23), MyMoneyMoney(1, 3),  QString("MySource"));

  CPPUNIT_ASSERT(n1.isValid() == false);
  CPPUNIT_ASSERT(n2.isValid() == false);
  CPPUNIT_ASSERT(n3.isValid() == false);
  CPPUNIT_ASSERT(n4.isValid() == true);
}

void MyMoneyPriceTest::testRate()
{
  MyMoneyPrice n1(QString("from"), QString("to"), QDate(2005, 9, 23), MyMoneyMoney(1, 3),  QString("MySource"));
  MyMoneyPrice n2(QString("from"), QString("to"), QDate(), MyMoneyMoney(1, 3),  QString("MySource"));

  try {
    CPPUNIT_ASSERT(n1.rate("to") == MyMoneyMoney(1, 3));
    CPPUNIT_ASSERT(n1.rate("from") == MyMoneyMoney(3, 1));
    CPPUNIT_ASSERT(n1.rate(QString()) == MyMoneyMoney(1, 3));

    CPPUNIT_ASSERT(n2.isValid() == false);
    CPPUNIT_ASSERT(n2.rate("to") == MyMoneyMoney(1, 1));
  } catch (MyMoneyException *e) {
    CPPUNIT_FAIL("Unexpected exception");
    delete e;
  }

  try {
    n1.rate("unknown");
    CPPUNIT_FAIL("Missing expected exception");
  } catch (MyMoneyException *e) {
    delete e;
  }
}

