
/***************************************************************************
                          mymoneyinstitutiontest.h
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

#ifndef MYMONEYINSTITUTIONTEST_H
#define MYMONEYINSTITUTIONTEST_H

#include <cppunit/extensions/HelperMacros.h>

#define private public
#include "mymoneyinstitution.h"
#undef private

class MyMoneyInstitutionTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(MyMoneyInstitutionTest);
  CPPUNIT_TEST(testEmptyConstructor);
  CPPUNIT_TEST(testSetFunctions);
  CPPUNIT_TEST(testNonemptyConstructor);
  CPPUNIT_TEST(testCopyConstructor);
  CPPUNIT_TEST(testMyMoneyFileConstructor);
  CPPUNIT_TEST(testEquality);
  CPPUNIT_TEST(testInequality);
  CPPUNIT_TEST(testAccountIDList);
  CPPUNIT_TEST(testWriteXML);
  CPPUNIT_TEST(testReadXML);
  CPPUNIT_TEST_SUITE_END();

protected:
  MyMoneyInstitution *m, *n;

public:
  MyMoneyInstitutionTest();

  void setUp();
  void tearDown();
  void testEmptyConstructor();
  void testSetFunctions();
  void testNonemptyConstructor();
  void testCopyConstructor();
  void testMyMoneyFileConstructor();
  void testEquality();
  void testInequality();
  void testAccountIDList();
  void testWriteXML();
  void testReadXML();
};

#endif
