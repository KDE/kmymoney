/***************************************************************************
                          mymoneymoneytest.h
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

#ifndef __MYMONEYMONEYTEST_H__
#define __MYMONEYMONEYTEST_H__

// Check for standard definitions
#ifdef HAVE_STDINT_H
  #ifndef __STDC_LIMIT_MACROS
    #define __STDC_LIMIT_MACROS         // force definition of min and max values
  #endif
  #include <stdint.h>
#else
  #include <limits.h>
  #define INT64_MAX LLONG_MAX
  #define INT64_MIN LLONG_MIN
#endif

#include <cppunit/extensions/HelperMacros.h>

#define private public
#include "mymoneymoney.h"
#undef private

class MyMoneyMoneyTest : public CppUnit::TestFixture  {
	CPPUNIT_TEST_SUITE(MyMoneyMoneyTest);
	CPPUNIT_TEST(testEmptyConstructor);
	CPPUNIT_TEST(testIntConstructor);
	CPPUNIT_TEST(testStringConstructor);
	CPPUNIT_TEST(testAssignment);
	CPPUNIT_TEST(testConvert);
	CPPUNIT_TEST(testEquality);
	CPPUNIT_TEST(testInequality);
	CPPUNIT_TEST(testAddition);
	CPPUNIT_TEST(testSubtraction);
	CPPUNIT_TEST(testMultiplication);
	CPPUNIT_TEST(testDivision);
	CPPUNIT_TEST(testSetDecimalSeparator);
	CPPUNIT_TEST(testSetThousandSeparator);
	CPPUNIT_TEST(testFormatMoney);
	CPPUNIT_TEST(testRelation);
	CPPUNIT_TEST(testUnaryMinus);
	CPPUNIT_TEST(testDoubleConstructor);
	CPPUNIT_TEST(testAbsoluteFunction);
	CPPUNIT_TEST(testToString);
	CPPUNIT_TEST(testFromString);
	CPPUNIT_TEST(testNegativeSignPos);
	CPPUNIT_TEST(testPositiveSignPos);
	CPPUNIT_TEST(testNegativeStringConstructor);
	CPPUNIT_TEST(testReduce);
	CPPUNIT_TEST(testZeroDenominator);
	CPPUNIT_TEST_SUITE_END();

protected:
	MyMoneyMoney *m_0, *m_1, *m_2, *m_3, *m_4, *m_5;
public:
	MyMoneyMoneyTest();

	void setUp();
	void tearDown();
	void testEmptyConstructor();
	void testIntConstructor();
	void testStringConstructor();
	void testAssignment();
	void testConvert();
	void testEquality();
	void testInequality();
	void testAddition();
	void testSubtraction();
	void testMultiplication();
	void testDivision();
	void testFormatMoney();
	void testSetDecimalSeparator();
	void testSetThousandSeparator();
	void testRelation();
	void testUnaryMinus();
	void testDoubleConstructor();
	void testAbsoluteFunction();
	void testToString();
	void testFromString();
	void testNegativeSignPos();
	void testPositiveSignPos();
	void testNegativeStringConstructor();
	void testReduce();
	void testZeroDenominator();
};

#endif
