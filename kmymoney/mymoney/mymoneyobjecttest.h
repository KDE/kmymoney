/***************************************************************************
                          mymoneyobjecttest.h
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

#ifndef MYMONEYOBJECTTEST_H
#define MYMONEYOBJECTTEST_H

#include <cppunit/extensions/HelperMacros.h>

#define private public
#include "mymoneyobject.h"
#undef private

class MyMoneyObjectTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(MyMoneyObjectTest);
  CPPUNIT_TEST(testEmptyConstructor);
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST(testClearId);
  CPPUNIT_TEST(testCopyConstructor);
  CPPUNIT_TEST(testAssignmentConstructor);
  CPPUNIT_TEST(testEquality);
  CPPUNIT_TEST_SUITE_END();

protected:

public:
  MyMoneyObjectTest();
  void setUp();
  void tearDown();
  void testEmptyConstructor();
  void testConstructor();
  void testClearId();
  void testCopyConstructor();
  void testAssignmentConstructor();
  void testEquality();
};

#endif
