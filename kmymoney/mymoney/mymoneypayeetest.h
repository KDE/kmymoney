/***************************************************************************
                          mymoneypayeetest.h
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __MYMONEYPAYEETEST_H__
#define __MYMONEYPAYEETEST_H__

#include <cppunit/extensions/HelperMacros.h>
#include "autotest.h"

#define private public
#define protected public
#include "mymoneypayee.h"
#undef private
#undef protected

class MyMoneyPayeeTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(MyMoneyPayeeTest);
  CPPUNIT_TEST(testXml);
  CPPUNIT_TEST(testDefaultAccount);
  CPPUNIT_TEST_SUITE_END();

public:
  MyMoneyPayeeTest();

  void setUp();
  void tearDown();
  void testXml();
  void testDefaultAccount();
};

#endif
