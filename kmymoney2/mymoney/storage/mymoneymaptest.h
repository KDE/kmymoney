/***************************************************************************
                          mymoneymaptest.h
                          -------------------
    copyright            : (C) 2007 by Thomas Baumgart
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

#ifndef __MYMONEYMAPTEST_H__
#define __MYMONEYMAPTEST_H__

#include <cppunit/TestCaller.h>
#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>
#include <cppunit/extensions/HelperMacros.h>

#include "autotest.h"

#define private public
#include "mymoneyseqaccessmgr.h"
#undef private

class MyMoneyMapTest : public CppUnit::TestFixture  {
	CPPUNIT_TEST_SUITE(MyMoneyMapTest);
	CPPUNIT_TEST(testArrayOperator);
	CPPUNIT_TEST_SUITE_END();

protected:
	MyMoneyMap<QString, QString> *m;
public:
	MyMoneyMapTest();


	void setUp();
	void tearDown();
	void testArrayOperator(void);
};

#endif
