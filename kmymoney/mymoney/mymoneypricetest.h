/***************************************************************************
                          mymoneypricetest.h
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

#ifndef MYMONEYPRICETEST_H
#define MYMONEYPRICETEST_H

#include <cppunit/extensions/HelperMacros.h>

#define private public
#include "mymoneyprice.h"
#undef private

class MyMoneyPriceTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(MyMoneyPriceTest);
  CPPUNIT_TEST(testDefaultConstructor);
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST(testValidity);
  CPPUNIT_TEST_SUITE_END();

protected:
  MyMoneyPrice* m;

public:
  MyMoneyPriceTest();

  void setUp();
  void tearDown();

  void testDefaultConstructor();
  void testConstructor();
  void testValidity();
  void testRate();

};
#endif
