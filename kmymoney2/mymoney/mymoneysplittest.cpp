/***************************************************************************
                          mymoneysplittest.cpp
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

#include "mymoneysplittest.h"
#include <mymoneyexception.h>

MyMoneySplitTest::MyMoneySplitTest()
{
}


void MyMoneySplitTest::setUp () {
	m = new MyMoneySplit();
}

void MyMoneySplitTest::tearDown () {
	delete m;
}

void MyMoneySplitTest::testEmptyConstructor() {
	CPPUNIT_ASSERT(m->accountId().isEmpty());
	CPPUNIT_ASSERT(m->id().isEmpty());
	CPPUNIT_ASSERT(m->memo().isEmpty());
	CPPUNIT_ASSERT(m->action().isEmpty());
	CPPUNIT_ASSERT(m->shares().isZero());
	CPPUNIT_ASSERT(m->value().isZero());
	CPPUNIT_ASSERT(m->reconcileFlag() == MyMoneySplit::NotReconciled);
	CPPUNIT_ASSERT(m->reconcileDate() == QDate());
	CPPUNIT_ASSERT(m->transactionId().isEmpty());
}

void MyMoneySplitTest::testSetFunctions() {
	m->setAccountId("Account");
	m->setMemo("Memo");
	m->setReconcileDate(QDate(1,2,3));
	m->setReconcileFlag(MyMoneySplit::Cleared);
	m->setShares(1234);
	m->setValue(3456);
	m->setId("MyID");
	m->setPayeeId("Payee");
	m->setAction("Action");
	m->setTransactionId("TestTransaction");
	m->setValue("Key", "Value");

	CPPUNIT_ASSERT(m->accountId() == "Account");
	CPPUNIT_ASSERT(m->memo() == "Memo");
	CPPUNIT_ASSERT(m->reconcileDate() == QDate(1,2,3));
	CPPUNIT_ASSERT(m->reconcileFlag() == MyMoneySplit::Cleared);
	CPPUNIT_ASSERT(m->shares() == MyMoneyMoney(1234));
	CPPUNIT_ASSERT(m->value() == MyMoneyMoney(3456));
	CPPUNIT_ASSERT(m->id() == "MyID");
	CPPUNIT_ASSERT(m->payeeId() == "Payee");
	CPPUNIT_ASSERT(m->action() == "Action");
	CPPUNIT_ASSERT(m->transactionId() == "TestTransaction");
	CPPUNIT_ASSERT(m->value("Key") == "Value");
}


void MyMoneySplitTest::testCopyConstructor() {
	testSetFunctions();

	MyMoneySplit n(*m);

	CPPUNIT_ASSERT(n.accountId() == "Account");
	CPPUNIT_ASSERT(n.memo() == "Memo");
	CPPUNIT_ASSERT(n.reconcileDate() == QDate(1,2,3));
	CPPUNIT_ASSERT(n.reconcileFlag() == MyMoneySplit::Cleared);
	CPPUNIT_ASSERT(n.shares() == MyMoneyMoney(1234));
	CPPUNIT_ASSERT(n.value() == MyMoneyMoney(3456));
	CPPUNIT_ASSERT(n.id() == "MyID");
	CPPUNIT_ASSERT(n.payeeId() == "Payee");
	CPPUNIT_ASSERT(n.action() == "Action");
	CPPUNIT_ASSERT(n.transactionId() == "TestTransaction");
	CPPUNIT_ASSERT(n.value("Key") == "Value");
}

void MyMoneySplitTest::testAssignmentConstructor() {
	testSetFunctions();

	MyMoneySplit n;

	n = *m;

	CPPUNIT_ASSERT(n.accountId() == "Account");
	CPPUNIT_ASSERT(n.memo() == "Memo");
	CPPUNIT_ASSERT(n.reconcileDate() == QDate(1,2,3));
	CPPUNIT_ASSERT(n.reconcileFlag() == MyMoneySplit::Cleared);
	CPPUNIT_ASSERT(n.shares() == MyMoneyMoney(1234));
	CPPUNIT_ASSERT(n.value() == MyMoneyMoney(3456));
	CPPUNIT_ASSERT(n.id() == "MyID");
	CPPUNIT_ASSERT(n.payeeId() == "Payee");
	CPPUNIT_ASSERT(n.action() == "Action");
	CPPUNIT_ASSERT(n.transactionId() == "TestTransaction");
	CPPUNIT_ASSERT(n.value("Key") == "Value");
}

void MyMoneySplitTest::testEquality() {
	testSetFunctions();

	MyMoneySplit n(*m);

	CPPUNIT_ASSERT(n == *m);
}

void MyMoneySplitTest::testInequality() {
	testSetFunctions();

	MyMoneySplit n(*m);

	n.setShares(3456);
	CPPUNIT_ASSERT(!(n == *m));

	n = *m;
	n.setId("Not My ID");
	CPPUNIT_ASSERT(!(n == *m));

	n = *m;
	n.setPayeeId("No payee");
	CPPUNIT_ASSERT(!(n == *m));

	n = *m;
	n.setAction("No action");
	CPPUNIT_ASSERT(!(n == *m));

	n = *m;
	n.setNumber("No number");
	CPPUNIT_ASSERT(!(n == *m));

	n = *m;
	n.setAccountId("No account");
	CPPUNIT_ASSERT(!(n == *m));

	n = *m;
	n.setMemo("No memo");
	CPPUNIT_ASSERT(!(n == *m));

	n = *m;
	n.setReconcileDate(QDate(3,4,5));
	CPPUNIT_ASSERT(!(n == *m));

	n = *m;
	n.setReconcileFlag(MyMoneySplit::Frozen);
	CPPUNIT_ASSERT(!(n == *m));

	n = *m;
	n.setShares(4567);
	CPPUNIT_ASSERT(!(n == *m));

	n = *m;
	n.setValue(9876);
	CPPUNIT_ASSERT(!(n == *m));

	n = *m;
	n.setTransactionId("NoTransaction");
	CPPUNIT_ASSERT(!(n == *m));

	n = *m;
	n.setValue("Key", "NoValue");
	CPPUNIT_ASSERT(!(n == *m));
}


void MyMoneySplitTest::testAmortization() {
	CPPUNIT_ASSERT(m->isAmortizationSplit() == false);
	testSetFunctions();
	CPPUNIT_ASSERT(m->isAmortizationSplit() == false);
	m->setAction(MyMoneySplit::ActionAmortization);
	CPPUNIT_ASSERT(m->isAmortizationSplit() == true);
}

void MyMoneySplitTest::testValue() {
	m->setValue(1);
	m->setShares(2);
	CPPUNIT_ASSERT(m->value("EUR", "EUR") == MyMoneyMoney(1));
	CPPUNIT_ASSERT(m->value("EUR", "USD") == MyMoneyMoney(2));
}

void MyMoneySplitTest::testSetValue() {
	CPPUNIT_ASSERT(m->value().isZero());
	CPPUNIT_ASSERT(m->shares().isZero());
	m->setValue(1, "EUR", "EUR");
	CPPUNIT_ASSERT(m->value() == MyMoneyMoney(1));
	CPPUNIT_ASSERT(m->shares().isZero());
	m->setValue(3, "EUR", "USD");
	CPPUNIT_ASSERT(m->value() == MyMoneyMoney(1));
	CPPUNIT_ASSERT(m->shares() == MyMoneyMoney(3));
}

void MyMoneySplitTest::testSetAction() {
	CPPUNIT_ASSERT(m->action() == QString());
	m->setAction(MyMoneySplit::BuyShares);
	CPPUNIT_ASSERT(m->action() == MyMoneySplit::ActionBuyShares);
	m->setAction(MyMoneySplit::SellShares);
	CPPUNIT_ASSERT(m->action() == MyMoneySplit::ActionBuyShares);
	m->setAction(MyMoneySplit::Dividend);
	CPPUNIT_ASSERT(m->action() == MyMoneySplit::ActionDividend);
	m->setAction(MyMoneySplit::Yield);
	CPPUNIT_ASSERT(m->action() == MyMoneySplit::ActionYield);
	m->setAction(MyMoneySplit::ReinvestDividend);
	CPPUNIT_ASSERT(m->action() == MyMoneySplit::ActionReinvestDividend);
	m->setAction(MyMoneySplit::AddShares);
	CPPUNIT_ASSERT(m->action() == MyMoneySplit::ActionAddShares);
	m->setAction(MyMoneySplit::RemoveShares);
	CPPUNIT_ASSERT(m->action() == MyMoneySplit::ActionAddShares);
	m->setAction(MyMoneySplit::SplitShares);
	CPPUNIT_ASSERT(m->action() == MyMoneySplit::ActionSplitShares);
}

void MyMoneySplitTest::testIsAutoCalc() {
	CPPUNIT_ASSERT(m->isAutoCalc() == false);
	m->setValue(MyMoneyMoney::autoCalc);
	CPPUNIT_ASSERT(m->isAutoCalc() == true);
	m->setShares(MyMoneyMoney::autoCalc);
	CPPUNIT_ASSERT(m->isAutoCalc() == true);
	m->setValue(0);
	CPPUNIT_ASSERT(m->isAutoCalc() == true);
	m->setShares(1);
	CPPUNIT_ASSERT(m->isAutoCalc() == false);
}

void MyMoneySplitTest::testWriteXML() {
	MyMoneySplit s;

	s.setPayeeId("P000001");
	s.setShares(MyMoneyMoney(96379, 100));
	s.setValue(MyMoneyMoney(96379, 1000));
	s.setAccountId("A000076");
	s.setNumber("124");
	s.setBankID("SPID");
	s.setAction(MyMoneySplit::ActionDeposit);
	s.setReconcileFlag(MyMoneySplit::Reconciled);

	QDomDocument doc("TEST");
	QDomElement el = doc.createElement("SPLIT-CONTAINER");
	doc.appendChild(el);
	s.writeXML(doc, el);

	QString ref = QString(
		"<!DOCTYPE TEST>\n"
		"<SPLIT-CONTAINER>\n"
		" <SPLIT payee=\"P000001\" reconciledate=\"\" shares=\"96379/100\" action=\"Deposit\" bankid=\"SPID\" number=\"124\" reconcileflag=\"2\" memo=\"\" value=\"96379/1000\" id=\"\" account=\"A000076\" />\n"
		"</SPLIT-CONTAINER>\n");

	CPPUNIT_ASSERT(doc.toString() == ref);
}

void MyMoneySplitTest::testReadXML() {
	MyMoneySplit s;
	QString ref_ok = QString(
		"<!DOCTYPE TEST>\n"
		"<SPLIT-CONTAINER>\n"
		" <SPLIT payee=\"P000001\" reconciledate=\"\" shares=\"96379/100\" action=\"Deposit\" bankid=\"SPID\" number=\"124\" reconcileflag=\"2\" memo=\"MyMemo\" value=\"96379/1000\" account=\"A000076\" />\n"
		"</SPLIT-CONTAINER>\n");

	QString ref_false = QString(
		"<!DOCTYPE TEST>\n"
		"<SPLIT-CONTAINER>\n"
		" <SPLITS payee=\"P000001\" reconciledate=\"\" shares=\"96379/100\" action=\"Deposit\" bankid=\"SPID\" number=\"124\" reconcileflag=\"2\" memo=\"\" value=\"96379/1000\" account=\"A000076\" />\n"
		"</SPLIT-CONTAINER>\n");

	QDomDocument doc;
	QDomElement node;
	doc.setContent(ref_false);
	node = doc.documentElement().firstChild().toElement();

	try {
		s = MyMoneySplit(node);
		CPPUNIT_FAIL("Missing expected exception");
	} catch(MyMoneyException *e) {
		delete e;
	}

	doc.setContent(ref_ok);
	node = doc.documentElement().firstChild().toElement();

	try {
		s = MyMoneySplit(node);
		CPPUNIT_ASSERT(s.id().isEmpty());
		CPPUNIT_ASSERT(s.payeeId() == "P000001");
		CPPUNIT_ASSERT(s.reconcileDate() == QDate());
		CPPUNIT_ASSERT(s.shares() == MyMoneyMoney(96379, 100));
		CPPUNIT_ASSERT(s.value() == MyMoneyMoney(96379, 1000));
		CPPUNIT_ASSERT(s.number() == "124");
		CPPUNIT_ASSERT(s.bankID() == "SPID");
		CPPUNIT_ASSERT(s.reconcileFlag() == MyMoneySplit::Reconciled);
		CPPUNIT_ASSERT(s.action() == MyMoneySplit::ActionDeposit);
		CPPUNIT_ASSERT(s.accountId() == "A000076");
		CPPUNIT_ASSERT(s.memo() == "MyMemo");
	} catch(MyMoneyException *e) {
		delete e;
	}

}
