/***************************************************************************
                          mymoneykeyvaluecontainertest.h
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

#ifndef __MYMONEYKEYVALUECONTAINERTEST_H__
#define __MYMONEYKEYVALUECONTAINERTEST_H__

#include <cppunit/extensions/HelperMacros.h>

#define private public
#include "mymoneykeyvaluecontainer.h"
#undef private

class MyMoneyKeyValueContainerTest : public CppUnit::TestFixture  {
        CPPUNIT_TEST_SUITE(MyMoneyKeyValueContainerTest);
	CPPUNIT_TEST(testEmptyConstructor);
	CPPUNIT_TEST(testRetrieveValue);
	CPPUNIT_TEST(testSetValue);
	CPPUNIT_TEST(testDeletePair);
	CPPUNIT_TEST(testClear);
	CPPUNIT_TEST(testRetrieveList);
	CPPUNIT_TEST(testLoadList);
	CPPUNIT_TEST(testWriteXML);
	CPPUNIT_TEST(testReadXML);
	CPPUNIT_TEST(testArrayRead);
	CPPUNIT_TEST(testArrayWrite);
	CPPUNIT_TEST_SUITE_END();

protected:
	MyMoneyKeyValueContainer	*m;

public:
	MyMoneyKeyValueContainerTest();
	void setUp ();
	void tearDown ();
	void testEmptyConstructor();
	void testRetrieveValue();
	void testSetValue();
	void testDeletePair();
	void testClear();
	void testRetrieveList();
	void testLoadList();
	void testArrayRead();
	void testArrayWrite();
	void testWriteXML();
	void testReadXML();
};

#endif
