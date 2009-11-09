/***************************************************************************
                          mymoneyobjecttest.cpp
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

#include "mymoneyobjecttest.h"
#include "mymoneyaccount.h"

MyMoneyObjectTest::MyMoneyObjectTest()
{
}


void MyMoneyObjectTest::setUp () {
}

void MyMoneyObjectTest::tearDown () {
}

void MyMoneyObjectTest::testEmptyConstructor() {
	MyMoneyAccount a;
	CPPUNIT_ASSERT(a.id().isEmpty());
}

void MyMoneyObjectTest::testConstructor() {
	MyMoneyAccount a(QString("thb"), MyMoneyAccount());

	CPPUNIT_ASSERT(!a.id().isEmpty());
	CPPUNIT_ASSERT(a.id() == QString("thb"));
}

void MyMoneyObjectTest::testClearId() {
	MyMoneyAccount a(QString("thb"), MyMoneyAccount());

	CPPUNIT_ASSERT(!a.id().isEmpty());
	a.clearId();
	CPPUNIT_ASSERT(a.id().isEmpty());
}

void MyMoneyObjectTest::testCopyConstructor() {
	MyMoneyAccount a(QString("thb"), MyMoneyAccount());
	MyMoneyAccount b(a);

	CPPUNIT_ASSERT(a.MyMoneyObject::operator==(b));
}

void MyMoneyObjectTest::testAssignmentConstructor() {
	MyMoneyAccount a(QString("thb"), MyMoneyAccount());
	MyMoneyAccount b = a;

	CPPUNIT_ASSERT(a.MyMoneyObject::operator==(b));
}

void MyMoneyObjectTest::testEquality() {
	MyMoneyAccount a(QString("thb"), MyMoneyAccount());
	MyMoneyAccount b(QString("thb"), MyMoneyAccount());
	MyMoneyAccount c(QString("ace"), MyMoneyAccount());

	CPPUNIT_ASSERT(a.MyMoneyObject::operator==(b));
	CPPUNIT_ASSERT(!(a.MyMoneyObject::operator==(c)));
}

