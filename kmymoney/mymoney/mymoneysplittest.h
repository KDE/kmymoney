/***************************************************************************
                          mymoneysplittest.h
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

#ifndef MYMONEYSPLITTEST_H
#define MYMONEYSPLITTEST_H

#include <cppunit/extensions/HelperMacros.h>

#define private public
#define protected public
#include "mymoneysplit.h"
#undef private

class MyMoneySplitTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(MyMoneySplitTest);
  CPPUNIT_TEST(testEmptyConstructor);
  CPPUNIT_TEST(testSetFunctions);
  CPPUNIT_TEST(testCopyConstructor);
  CPPUNIT_TEST(testAssignmentConstructor);
  CPPUNIT_TEST(testEquality);
  CPPUNIT_TEST(testInequality);
  CPPUNIT_TEST(testAmortization);
  CPPUNIT_TEST(testValue);
  CPPUNIT_TEST(testSetValue);
  CPPUNIT_TEST(testSetAction);
  CPPUNIT_TEST(testIsAutoCalc);
  CPPUNIT_TEST(testWriteXML);
  CPPUNIT_TEST(testReadXML);
  CPPUNIT_TEST_SUITE_END();

protected:
  MyMoneySplit *m;

public:
  MyMoneySplitTest();

  void setUp();
  void tearDown();
  void testEmptyConstructor();
  void testSetFunctions();
  void testCopyConstructor();
  void testAssignmentConstructor();
  void testEquality();
  void testInequality();
  void testAmortization();
  void testValue();
  void testSetValue();
  void testSetAction();
  void testIsAutoCalc();
  void testWriteXML();
  void testReadXML();
};

#endif
