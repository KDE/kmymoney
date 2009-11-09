/***************************************************************************
                          mymoneyfinancialcalculatortest.h
                          -------------------
    copyright            : (C) 2003 by Thomas Baumgart
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

#ifndef __MYMONEYFINANCIALCALCULATORTEST_H__
#define __MYMONEYFINANCIALCALCULATORTEST_H__

#include <cppunit/extensions/HelperMacros.h>
#include "autotest.h"

#define private public
#define protected public
#include "mymoneyfinancialcalculator.h"
#undef private
#undef protected

class MyMoneyFinancialCalculatorTest : public CppUnit::TestFixture  {
        CPPUNIT_TEST_SUITE(MyMoneyFinancialCalculatorTest);
	CPPUNIT_TEST(testEmptyConstructor);
        CPPUNIT_TEST(testSetPrec);
        CPPUNIT_TEST(testSetNpp);
        CPPUNIT_TEST(testSetPF);
        CPPUNIT_TEST(testSetCF);
        CPPUNIT_TEST(testSetBep);
        CPPUNIT_TEST(testSetDisc);
        CPPUNIT_TEST(testSetIr);
        CPPUNIT_TEST(testSetPv);
        CPPUNIT_TEST(testSetPmt);
        CPPUNIT_TEST(testSetFv);
        CPPUNIT_TEST(testCombinedSet);
	CPPUNIT_TEST(testNumPayments);
	CPPUNIT_TEST(testUseCase1);
	CPPUNIT_TEST(testUseCase2);
	CPPUNIT_TEST_SUITE_END();
protected:
	MyMoneyFinancialCalculator	*m;

public:
	MyMoneyFinancialCalculatorTest ();

	void setUp ();
	void tearDown ();
	void testEmptyConstructor();
	void testSetPrec();
	void testSetNpp();
	void testSetPF();
	void testSetCF();
	void testSetBep();
	void testSetDisc();
	void testSetIr();
	void testSetPv();
	void testSetPmt();
	void testSetFv();
	void testCombinedSet();
	void testNumPayments();
	void testUseCase1();
	void testUseCase2();
};

#endif
