/***************************************************************************
                          mymoneyexceptiontest.h
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

#ifndef __MYMONEYEXCEPTIONTEST_H__
#define __MYMONEYEXCEPTIONTEST_H__

#include <cppunit/extensions/HelperMacros.h>

#define private public
#include "mymoneyutils.h"
#include "mymoneyexception.h"
#undef private

class MyMoneyExceptionTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(MyMoneyExceptionTest);
  CPPUNIT_TEST(testDefaultConstructor);
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST_SUITE_END();

protected:
public:
  MyMoneyExceptionTest();


  void setUp();

  void tearDown();

  void testDefaultConstructor();

  void testConstructor();

};
#endif
