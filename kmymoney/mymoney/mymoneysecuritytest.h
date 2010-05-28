/***************************************************************************
                          mymoneysecuritytest.h
                          -------------------
    copyright            : (C) 2004 by Kevin Tambascio
    email                : ktambascio@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MYMONEYSECURITYTEST_H
#define MYMONEYSECURITYTEST_H

#include <cppunit/extensions/HelperMacros.h>

#define private public
#include "mymoneysecurity.h"
#undef private

class MyMoneySecurityTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(MyMoneySecurityTest);
  CPPUNIT_TEST(testEmptyConstructor);
  CPPUNIT_TEST(testNonemptyConstructor);
  CPPUNIT_TEST(testCopyConstructor);
  CPPUNIT_TEST(testSetFunctions);
  CPPUNIT_TEST(testEquality);
  CPPUNIT_TEST(testInequality);
  /*
   CPPUNIT_TEST(testMyMoneyFileConstructor);
   CPPUNIT_TEST(testAccountIDList);
  */
  CPPUNIT_TEST_SUITE_END();

protected:
  MyMoneySecurity *m;

public:
  MyMoneySecurityTest();

  void setUp();
  void tearDown();
  void testEmptyConstructor();
  void testNonemptyConstructor();
  void testCopyConstructor();
  void testSetFunctions();
  void testEquality();
  void testInequality();
  // void testMyMoneyFileConstructor();
  // void testAccountIDList ();
};

#endif
