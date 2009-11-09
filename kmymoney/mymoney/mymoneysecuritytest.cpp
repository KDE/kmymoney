/***************************************************************************
                          mymoneysecuritytest.cpp
                          -------------------
    copyright            : (C) 2002 by Kevin Tambascio
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

#include "mymoneysecuritytest.h"

MyMoneySecurityTest::MyMoneySecurityTest()
{
}


void MyMoneySecurityTest::setUp () {
	m = new MyMoneySecurity();
}

void MyMoneySecurityTest::tearDown () {
	delete m;
}

void MyMoneySecurityTest::testEmptyConstructor() {
	CPPUNIT_ASSERT(m->id().isEmpty());
	CPPUNIT_ASSERT(m->name().isEmpty());
	CPPUNIT_ASSERT(m->tradingSymbol().isEmpty());
	CPPUNIT_ASSERT(m->securityType() == MyMoneySecurity::SECURITY_NONE);
	CPPUNIT_ASSERT(m->tradingMarket().isEmpty());
	CPPUNIT_ASSERT(m->tradingCurrency().isEmpty());
	CPPUNIT_ASSERT(m->smallestCashFraction() == 100);
	CPPUNIT_ASSERT(m->smallestAccountFraction() == 100);
	CPPUNIT_ASSERT(m->partsPerUnit() == 100);
}

void MyMoneySecurityTest::testCopyConstructor() {
	MyMoneySecurity* n1 = new MyMoneySecurity("GUID1", *m);
	MyMoneySecurity n2(*n1);

	// CPPUNIT_ASSERT(*n1 == n2);

	delete n1;
}

void MyMoneySecurityTest::testNonemptyConstructor() {
	QDate date(2004,4,1);
	MyMoneyMoney val("1234/100");

	m->setName("name");
	m->setTradingSymbol("symbol");
	m->setSecurityType(MyMoneySecurity::SECURITY_CURRENCY);
	// m->addPriceHistory(date, val);

	MyMoneySecurity n("id", *m);

	CPPUNIT_ASSERT(n.id() == QString("id"));
	CPPUNIT_ASSERT(n.tradingSymbol() == QString("symbol"));
	CPPUNIT_ASSERT(n.securityType() == MyMoneySecurity::SECURITY_CURRENCY);
	// CPPUNIT_ASSERT(n.priceHistory().count() == 1);
}


void MyMoneySecurityTest::testSetFunctions() {
	m->setName("Name");
	m->setTradingSymbol("Symbol");
	m->setTradingMarket("Market");
	m->setTradingCurrency("Currency");
	m->setSecurityType(MyMoneySecurity::SECURITY_STOCK);
	m->setSmallestAccountFraction(50);
	m->setSmallestCashFraction(2);
	m->setPartsPerUnit(30);

	CPPUNIT_ASSERT(m->name() == "Name");
	CPPUNIT_ASSERT(m->tradingSymbol() == "Symbol");
	CPPUNIT_ASSERT(m->tradingMarket() == "Market");
	CPPUNIT_ASSERT(m->tradingCurrency() == "Currency");
	CPPUNIT_ASSERT(m->securityType() == MyMoneySecurity::SECURITY_STOCK);
	CPPUNIT_ASSERT(m->smallestAccountFraction() == 50);
	CPPUNIT_ASSERT(m->smallestCashFraction() == 2);
	CPPUNIT_ASSERT(m->partsPerUnit() == 30);
}

/*
void MyMoneySecurityTest::testMyMoneyFileConstructor() {
	MyMoneySecurity *t = new MyMoneySecurity("GUID", *n);

	CPPUNIT_ASSERT(t->id() == "GUID");

	delete t;
}
*/

void MyMoneySecurityTest::testEquality () {
	testSetFunctions();
	m->setValue("Key", "Value");

	MyMoneySecurity n;
	n = *m;

	CPPUNIT_ASSERT(n == *m);
	n.setName("NewName");
	CPPUNIT_ASSERT(!(n == *m));
	n = *m;
	n.setTradingSymbol("NewSymbol");
	CPPUNIT_ASSERT(!(n == *m));
	n = *m;
	n.setTradingMarket("NewMarket");
	CPPUNIT_ASSERT(!(n == *m));
	n = *m;
	n.setTradingCurrency("NewCurrency");
	CPPUNIT_ASSERT(!(n == *m));
	n = *m;
	n.setSecurityType(MyMoneySecurity::SECURITY_CURRENCY);
	CPPUNIT_ASSERT(!(n == *m));
	n = *m;
	n.setSmallestAccountFraction(40);
	CPPUNIT_ASSERT(!(n == *m));
	n = *m;
	n.setSmallestCashFraction(20);
	CPPUNIT_ASSERT(!(n == *m));
	n = *m;
	n.setPartsPerUnit(3);
	CPPUNIT_ASSERT(!(n == *m));
	n = *m;
	n.setValue("Key", "NewValue");
	CPPUNIT_ASSERT(!(n == *m));
}

void MyMoneySecurityTest::testInequality () {
	testSetFunctions();
	m->setValue("Key", "Value");

	MyMoneySecurity n;
	n = *m;

	CPPUNIT_ASSERT(!(n != *m));
	n.setName("NewName");
	CPPUNIT_ASSERT(n != *m);
	n = *m;
	n.setTradingSymbol("NewSymbol");
	CPPUNIT_ASSERT(n != *m);
	n = *m;
	n.setTradingMarket("NewMarket");
	CPPUNIT_ASSERT(n != *m);
	n = *m;
	n.setTradingCurrency("NewCurrency");
	CPPUNIT_ASSERT(n != *m);
	n = *m;
	n.setSecurityType(MyMoneySecurity::SECURITY_CURRENCY);
	CPPUNIT_ASSERT(n != *m);
	n = *m;
	n.setSmallestAccountFraction(40);
	CPPUNIT_ASSERT(n != *m);
	n = *m;
	n.setSmallestCashFraction(20);
	CPPUNIT_ASSERT(n != *m);
	n = *m;
	n.setPartsPerUnit(3);
	CPPUNIT_ASSERT(n != *m);
	n = *m;
	n.setValue("Key", "NewValue");
	CPPUNIT_ASSERT(n != *m);
}

/*
void MyMoneySecurityTest::testAccountIDList () {
	MyMoneySecurity equity;
	QStringList list;
	QString id;

	// list must be empty
	list = institution.accountList();
	CPPUNIT_ASSERT(list.count() == 0);

	// add one account
	institution.addAccountId("A000002");
	list = institution.accountList();
	CPPUNIT_ASSERT(list.count() == 1);
	CPPUNIT_ASSERT(list.contains("A000002") == 1);

	// adding same account shouldn't make a difference
	institution.addAccountId("A000002");
	list = institution.accountList();
	CPPUNIT_ASSERT(list.count() == 1);
	CPPUNIT_ASSERT(list.contains("A000002") == 1);

	// now add another account
	institution.addAccountId("A000001");
	list = institution.accountList();
	CPPUNIT_ASSERT(list.count() == 2);
	CPPUNIT_ASSERT(list.contains("A000002") == 1);
	CPPUNIT_ASSERT(list.contains("A000001") == 1);

	id = institution.removeAccountId("A000001");
	CPPUNIT_ASSERT(id == "A000001");
	list = institution.accountList();
	CPPUNIT_ASSERT(list.count() == 1);
	CPPUNIT_ASSERT(list.contains("A000002") == 1);

}
*/

