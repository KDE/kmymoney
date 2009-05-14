/***************************************************************************
                          mymoneyaccounttest.cpp
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

#include "mymoneyaccounttest.h"
#include <mymoneyexception.h>
#include <mymoneysplit.h>

MyMoneyAccountTest::MyMoneyAccountTest()
{
}


void MyMoneyAccountTest::setUp () {
}

void MyMoneyAccountTest::tearDown () {
}

void MyMoneyAccountTest::testEmptyConstructor() {
	MyMoneyAccount a;

	CPPUNIT_ASSERT(a.id().isEmpty());
	CPPUNIT_ASSERT(a.name().isEmpty());
	CPPUNIT_ASSERT(a.accountType() == MyMoneyAccount::UnknownAccountType);
	CPPUNIT_ASSERT(a.openingDate() == QDate());
	CPPUNIT_ASSERT(a.lastModified() == QDate());
	CPPUNIT_ASSERT(a.lastReconciliationDate() == QDate());
	CPPUNIT_ASSERT(a.accountList().count() == 0);
	CPPUNIT_ASSERT(a.balance().isZero());
}

void MyMoneyAccountTest::testConstructor() {
	QString id = "A000001";
	QString institutionid = "B000001";
	QString parent = "Parent";
	MyMoneyAccount r;
	MyMoneySplit s;
	r.setAccountType(MyMoneyAccount::Asset);
	r.setOpeningDate(QDate::currentDate());
	r.setLastModified(QDate::currentDate());
	r.setDescription("Desc");
	r.setNumber("465500");
	r.setParentAccountId(parent);
	r.setValue(QString("key"), "value");
	s.setShares(MyMoneyMoney(1,1));
	r.adjustBalance(s);
	CPPUNIT_ASSERT(r.m_kvp.count() == 1);
	CPPUNIT_ASSERT(r.value("key") == "value");

	MyMoneyAccount a(id, r);

	CPPUNIT_ASSERT(a.id() == id);
	CPPUNIT_ASSERT(a.institutionId().isEmpty());
	CPPUNIT_ASSERT(a.accountType() == MyMoneyAccount::Asset);
	CPPUNIT_ASSERT(a.openingDate() == QDate::currentDate());
	CPPUNIT_ASSERT(a.lastModified() == QDate::currentDate());
	CPPUNIT_ASSERT(a.number() == "465500");
	CPPUNIT_ASSERT(a.description() == "Desc");
	CPPUNIT_ASSERT(a.accountList().count() == 0);
	CPPUNIT_ASSERT(a.parentAccountId() == "Parent");
	CPPUNIT_ASSERT(a.balance() == MyMoneyMoney(1,1));

	QMap<QString, QString> copy;
	copy = r.pairs();
	CPPUNIT_ASSERT(copy.count() == 1);
	CPPUNIT_ASSERT(copy[QString("key")] == "value");
}

void MyMoneyAccountTest::testSetFunctions() {
	MyMoneyAccount a;

	QDate today(QDate::currentDate());
	CPPUNIT_ASSERT(a.name().isEmpty());
	CPPUNIT_ASSERT(a.lastModified() == QDate());
	CPPUNIT_ASSERT(a.description().isEmpty());

	a.setName("Account");
	a.setInstitutionId("Institution1");
	a.setLastModified(today);
	a.setDescription("Desc");
	a.setNumber("123456");
	a.setAccountType(MyMoneyAccount::MoneyMarket);

	CPPUNIT_ASSERT(a.name() == "Account");
	CPPUNIT_ASSERT(a.institutionId() == "Institution1");
	CPPUNIT_ASSERT(a.lastModified() == today);
	CPPUNIT_ASSERT(a.description() == "Desc");
	CPPUNIT_ASSERT(a.number() == "123456");
	CPPUNIT_ASSERT(a.accountType() == MyMoneyAccount::MoneyMarket);
}

void MyMoneyAccountTest::testCopyConstructor() {
	QString id = "A000001";
	QString institutionid = "B000001";
	QString parent = "ParentAccount";
	MyMoneyAccount r;
	r.setAccountType(MyMoneyAccount::Expense);
	r.setOpeningDate(QDate::currentDate());
	r.setLastModified(QDate::currentDate());
	r.setName("Account");
	r.setInstitutionId("Inst1");
	r.setDescription("Desc1");
	r.setNumber("Number");
	r.setParentAccountId(parent);
	r.setValue("Key", "Value");

	MyMoneyAccount a(id, r);
	a.setInstitutionId(institutionid);

	MyMoneyAccount b(a);

	CPPUNIT_ASSERT(b.name() == "Account");
	CPPUNIT_ASSERT(b.institutionId() == institutionid);
	CPPUNIT_ASSERT(b.accountType() == MyMoneyAccount::Expense);
	CPPUNIT_ASSERT(b.lastModified() == QDate::currentDate());
	CPPUNIT_ASSERT(b.openingDate() == QDate::currentDate());
	CPPUNIT_ASSERT(b.description() == "Desc1");
	CPPUNIT_ASSERT(b.number() == "Number");
	CPPUNIT_ASSERT(b.parentAccountId() == "ParentAccount");

	CPPUNIT_ASSERT(b.value("Key") == "Value");
}

void MyMoneyAccountTest::testAssignmentConstructor() {
	MyMoneyAccount a;
	a.setAccountType(MyMoneyAccount::Checkings);
	a.setName("Account");
	a.setInstitutionId("Inst1");
	a.setDescription("Bla");
	a.setNumber("assigned Number");
	a.setValue("Key", "Value");
	a.addAccountId("ChildAccount");

	MyMoneyAccount b;

	b.setLastModified(QDate::currentDate());

	b = a;

	CPPUNIT_ASSERT(b.name() == "Account");
	CPPUNIT_ASSERT(b.institutionId() == "Inst1");
	CPPUNIT_ASSERT(b.accountType() == MyMoneyAccount::Checkings);
	CPPUNIT_ASSERT(b.lastModified() == QDate());
	CPPUNIT_ASSERT(b.openingDate() == a.openingDate());
	CPPUNIT_ASSERT(b.description() == "Bla");
	CPPUNIT_ASSERT(b.number() == "assigned Number");
	CPPUNIT_ASSERT(b.value("Key") == "Value");
	CPPUNIT_ASSERT(b.accountList().count() == 1);
	CPPUNIT_ASSERT(b.accountList()[0] == "ChildAccount");
}

void MyMoneyAccountTest::testAdjustBalance() {
	MyMoneyAccount a;
	MyMoneySplit s;
	s.setShares(MyMoneyMoney(3,1));
	a.adjustBalance(s);
	CPPUNIT_ASSERT(a.balance() == MyMoneyMoney(3,1));
	s.setShares(MyMoneyMoney(5,1));
	a.adjustBalance(s, true);
	CPPUNIT_ASSERT(a.balance() == MyMoneyMoney(-2,1));
	s.setShares(MyMoneyMoney(2,1));
	s.setAction(MyMoneySplit::ActionSplitShares);
	a.adjustBalance(s);
	CPPUNIT_ASSERT(a.balance() == MyMoneyMoney(-4,1));
	s.setShares(MyMoneyMoney(4,1));
	s.setAction(QString());
	a.adjustBalance(s);
	CPPUNIT_ASSERT(a.balance().isZero());
}

void MyMoneyAccountTest::testSubAccounts()
{
	MyMoneyAccount a;
	a.setAccountType(MyMoneyAccount::Checkings);

	a.addAccountId("Subaccount1");
	CPPUNIT_ASSERT(a.accountList().count() == 1);
	a.addAccountId("Subaccount1");
	CPPUNIT_ASSERT(a.accountList().count() == 1);
	a.addAccountId("Subaccount2");
	CPPUNIT_ASSERT(a.accountList().count() == 2);

}

void MyMoneyAccountTest::testEquality()
{
	MyMoneyAccount a;

	a.setLastModified(QDate::currentDate());
	a.setName("Name");
	a.setNumber("Number");
	a.setDescription("Desc");
	a.setInstitutionId("I-ID");
	a.setOpeningDate(QDate::currentDate());
	a.setLastReconciliationDate(QDate::currentDate());
	a.setAccountType(MyMoneyAccount::Asset);
	a.setParentAccountId("P-ID");
	a.setId("A-ID");
	a.setCurrencyId("C-ID");
	a.setValue("Key", "Value");

	MyMoneyAccount b;

	b = a;
	CPPUNIT_ASSERT(b == a);

	a.setName("Noname");
	CPPUNIT_ASSERT(!(b == a));
	b = a;

	a.setLastModified(QDate::currentDate().addDays(-1));
	CPPUNIT_ASSERT(!(b == a));
	b = a;

	a.setNumber("Nonumber");
	CPPUNIT_ASSERT(!(b == a));
	b = a;

	a.setDescription("NoDesc");
	CPPUNIT_ASSERT(!(b == a));
	b = a;

	a.setInstitutionId("I-noID");
	CPPUNIT_ASSERT(!(b == a));
	b = a;

	a.setOpeningDate(QDate::currentDate().addDays(-1));
	CPPUNIT_ASSERT(!(b == a));
	b = a;

	a.setLastReconciliationDate(QDate::currentDate().addDays(-1));
	CPPUNIT_ASSERT(!(b == a));
	b = a;

	a.setAccountType(MyMoneyAccount::Liability);
	CPPUNIT_ASSERT(!(b == a));
	b = a;

	a.setParentAccountId("P-noID");
	CPPUNIT_ASSERT(!(b == a));
	b = a;

	a.setId("A-noID");
	CPPUNIT_ASSERT(!(b == a));
	b = a;

	a.setCurrencyId("C-noID");
	CPPUNIT_ASSERT(!(b == a));
	b = a;

	a.setValue("Key", "noValue");
	CPPUNIT_ASSERT(!(b == a));
	b = a;

	a.setValue("noKey", "Value");
	CPPUNIT_ASSERT(!(b == a));
	b = a;

}

void MyMoneyAccountTest::testWriteXML() {
	QString id = "A000001";
	QString institutionid = "B000001";
	QString parent = "Parent";

	MyMoneyAccount r;
	r.setAccountType(MyMoneyAccount::Asset);
	r.setOpeningDate(QDate::currentDate());
	r.setLastModified(QDate::currentDate());
	r.setDescription("Desc");
	r.setName("AccountName");
	r.setNumber("465500");
	r.setParentAccountId(parent);
	r.setInstitutionId(institutionid);
	r.setValue(QString("key"), "value");
	r.addAccountId("A000002");
	// CPPUNIT_ASSERT(r.m_kvp.count() == 1);
	// CPPUNIT_ASSERT(r.value("key") == "value");

	MyMoneyAccount a(id, r);

	QDomDocument doc("TEST");
	QDomElement el = doc.createElement("ACCOUNT-CONTAINER");
	doc.appendChild(el);
	a.writeXML(doc, el);

	QString ref = QString(
		"<!DOCTYPE TEST>\n"
		"<ACCOUNT-CONTAINER>\n"
		" <ACCOUNT parentaccount=\"Parent\" lastmodified=\"%1\" lastreconciled=\"\" institution=\"B000001\" number=\"465500\" opened=\"%2\" type=\"9\" id=\"A000001\" name=\"AccountName\" description=\"Desc\" >\n"
		"  <SUBACCOUNTS>\n"
		"   <SUBACCOUNT id=\"A000002\" />\n"
		"  </SUBACCOUNTS>\n"
		"  <KEYVALUEPAIRS>\n"
		"   <PAIR key=\"key\" value=\"value\" />\n"
		"  </KEYVALUEPAIRS>\n"
		" </ACCOUNT>\n"
		"</ACCOUNT-CONTAINER>\n").
			arg(QDate::currentDate().toString(Qt::ISODate)).arg(QDate::currentDate().toString(Qt::ISODate));

	CPPUNIT_ASSERT(doc.toString() == ref);
}

void MyMoneyAccountTest::testReadXML() {
	MyMoneyAccount a;
	QString ref_ok = QString(
		"<!DOCTYPE TEST>\n"
		"<ACCOUNT-CONTAINER>\n"
		" <ACCOUNT parentaccount=\"Parent\" lastmodified=\"%1\" lastreconciled=\"\" institution=\"B000001\" number=\"465500\" opened=\"%2\" type=\"9\" id=\"A000001\" name=\"AccountName\" description=\"Desc\" >\n"
		"  <SUBACCOUNTS>\n"
		"   <SUBACCOUNT id=\"A000002\" />\n"
		"   <SUBACCOUNT id=\"A000003\" />\n"
		"  </SUBACCOUNTS>\n"
		"  <KEYVALUEPAIRS>\n"
		"   <PAIR key=\"key\" value=\"value\" />\n"
		"   <PAIR key=\"Key\" value=\"Value\" />\n"
		"  </KEYVALUEPAIRS>\n"
		" </ACCOUNT>\n"
		"</ACCOUNT-CONTAINER>\n").
			arg(QDate::currentDate().toString(Qt::ISODate)).arg(QDate::currentDate().toString(Qt::ISODate));

	QString ref_false = QString(
		"<!DOCTYPE TEST>\n"
		"<ACCOUNT-CONTAINER>\n"
		" <KACCOUNT parentaccount=\"Parent\" lastmodified=\"%1\" lastreconciled=\"\" institution=\"B000001\" number=\"465500\" opened=\"%2\" type=\"9\" id=\"A000001\" name=\"AccountName\" description=\"Desc\" >\n"
		"  <SUBACCOUNTS>\n"
		"   <SUBACCOUNT id=\"A000002\" />\n"
		"   <SUBACCOUNT id=\"A000003\" />\n"
		"  </SUBACCOUNTS>\n"
		"  <KEYVALUEPAIRS>\n"
		"   <PAIR key=\"key\" value=\"value\" />\n"
		"   <PAIR key=\"Key\" value=\"Value\" />\n"
		"  </KEYVALUEPAIRS>\n"
		" </KACCOUNT>\n"
		"</ACCOUNT-CONTAINER>\n").
			arg(QDate::currentDate().toString(Qt::ISODate)).arg(QDate::currentDate().toString(Qt::ISODate));

	QDomDocument doc;
	QDomElement node;
	doc.setContent(ref_false);
	node = doc.documentElement().firstChild().toElement();

	try {
		a = MyMoneyAccount(node);
		CPPUNIT_FAIL("Missing expected exception");
	} catch(MyMoneyException *e) {
		delete e;
	}

	doc.setContent(ref_ok);
	node = doc.documentElement().firstChild().toElement();

	a.addAccountId("TEST");
	a.setValue("KEY", "VALUE");

	try {
		a = MyMoneyAccount(node);
		CPPUNIT_ASSERT(a.id() == "A000001");
		CPPUNIT_ASSERT(a.m_name == "AccountName");
		CPPUNIT_ASSERT(a.m_parentAccount == "Parent");
		CPPUNIT_ASSERT(a.m_lastModified == QDate::currentDate());
		CPPUNIT_ASSERT(a.m_lastReconciliationDate == QDate());
		CPPUNIT_ASSERT(a.m_institution == "B000001");
		CPPUNIT_ASSERT(a.m_number == "465500");
		CPPUNIT_ASSERT(a.m_openingDate == QDate::currentDate());
		CPPUNIT_ASSERT(a.m_accountType == MyMoneyAccount::Asset);
		CPPUNIT_ASSERT(a.m_description == "Desc");
		CPPUNIT_ASSERT(a.accountList().count() == 2);
		CPPUNIT_ASSERT(a.accountList()[0] == "A000002");
		CPPUNIT_ASSERT(a.accountList()[1] == "A000003");
		CPPUNIT_ASSERT(a.pairs().count() == 3);
		CPPUNIT_ASSERT(a.value("key") == "value");
		CPPUNIT_ASSERT(a.value("Key") == "Value");
		CPPUNIT_ASSERT(a.value("lastStatementDate").isEmpty());
	} catch(MyMoneyException *e) {
		delete e;
		CPPUNIT_FAIL("Unexpected exception");
	}
}

void MyMoneyAccountTest::testHasReferenceTo(void)
{
	MyMoneyAccount a;

	a.setInstitutionId("I0001");
	a.addAccountId("A_001");
	a.addAccountId("A_002");
	a.setParentAccountId("A_Parent");
	a.setCurrencyId("Currency");

	CPPUNIT_ASSERT(a.hasReferenceTo("I0001") == true);
	CPPUNIT_ASSERT(a.hasReferenceTo("I0002") == false);
	CPPUNIT_ASSERT(a.hasReferenceTo("A_001") == false);
	CPPUNIT_ASSERT(a.hasReferenceTo("A_Parent") == true);
	CPPUNIT_ASSERT(a.hasReferenceTo("Currency") == true);
}

void MyMoneyAccountTest::testSetClosed(void)
{
	MyMoneyAccount a;

	CPPUNIT_ASSERT(a.isClosed() == false);
	a.setClosed(true);
	CPPUNIT_ASSERT(a.isClosed() == true);
	a.setClosed(false);
	CPPUNIT_ASSERT(a.isClosed() == false);
}

void MyMoneyAccountTest::testIsIncomeExpense(void)
{
	MyMoneyAccount a;

	a.setAccountType(MyMoneyAccount::UnknownAccountType);
	CPPUNIT_ASSERT(a.isIncomeExpense() == false);

	a.setAccountType(MyMoneyAccount::Checkings);
	CPPUNIT_ASSERT(a.isIncomeExpense() == false);

	a.setAccountType(MyMoneyAccount::Savings);
	CPPUNIT_ASSERT(a.isIncomeExpense() == false);

	a.setAccountType(MyMoneyAccount::Cash);
	CPPUNIT_ASSERT(a.isIncomeExpense() == false);

	a.setAccountType(MyMoneyAccount::CreditCard);
	CPPUNIT_ASSERT(a.isIncomeExpense() == false);

	a.setAccountType(MyMoneyAccount::Loan);
	CPPUNIT_ASSERT(a.isIncomeExpense() == false);

	a.setAccountType(MyMoneyAccount::CertificateDep);
	CPPUNIT_ASSERT(a.isIncomeExpense() == false);

	a.setAccountType(MyMoneyAccount::Investment);
	CPPUNIT_ASSERT(a.isIncomeExpense() == false);

	a.setAccountType(MyMoneyAccount::MoneyMarket);
	CPPUNIT_ASSERT(a.isIncomeExpense() == false);

	a.setAccountType(MyMoneyAccount::Asset);
	CPPUNIT_ASSERT(a.isIncomeExpense() == false);

	a.setAccountType(MyMoneyAccount::Liability);
	CPPUNIT_ASSERT(a.isIncomeExpense() == false);

	a.setAccountType(MyMoneyAccount::Currency);
	CPPUNIT_ASSERT(a.isIncomeExpense() == false);

	a.setAccountType(MyMoneyAccount::Income);
	CPPUNIT_ASSERT(a.isIncomeExpense() == true);

	a.setAccountType(MyMoneyAccount::Expense);
	CPPUNIT_ASSERT(a.isIncomeExpense() == true);

	a.setAccountType(MyMoneyAccount::AssetLoan);
	CPPUNIT_ASSERT(a.isIncomeExpense() == false);

	a.setAccountType(MyMoneyAccount::Stock);
	CPPUNIT_ASSERT(a.isIncomeExpense() == false);

	a.setAccountType(MyMoneyAccount::Equity);
	CPPUNIT_ASSERT(a.isIncomeExpense() == false);
}

void MyMoneyAccountTest::testIsAssetLiability(void)
{
	MyMoneyAccount a;

	a.setAccountType(MyMoneyAccount::UnknownAccountType);
	CPPUNIT_ASSERT(a.isAssetLiability() == false);

	a.setAccountType(MyMoneyAccount::Checkings);
	CPPUNIT_ASSERT(a.isAssetLiability() == true);

	a.setAccountType(MyMoneyAccount::Savings);
	CPPUNIT_ASSERT(a.isAssetLiability() == true);

	a.setAccountType(MyMoneyAccount::Cash);
	CPPUNIT_ASSERT(a.isAssetLiability() == true);

	a.setAccountType(MyMoneyAccount::CreditCard);
	CPPUNIT_ASSERT(a.isAssetLiability() == true);

	a.setAccountType(MyMoneyAccount::Loan);
	CPPUNIT_ASSERT(a.isAssetLiability() == true);

	a.setAccountType(MyMoneyAccount::CertificateDep);
	CPPUNIT_ASSERT(a.isAssetLiability() == true);

	a.setAccountType(MyMoneyAccount::Investment);
	CPPUNIT_ASSERT(a.isAssetLiability() == true);

	a.setAccountType(MyMoneyAccount::MoneyMarket);
	CPPUNIT_ASSERT(a.isAssetLiability() == true);

	a.setAccountType(MyMoneyAccount::Asset);
	CPPUNIT_ASSERT(a.isAssetLiability() == true);

	a.setAccountType(MyMoneyAccount::Liability);
	CPPUNIT_ASSERT(a.isAssetLiability() == true);

	a.setAccountType(MyMoneyAccount::Currency);
	CPPUNIT_ASSERT(a.isAssetLiability() == true);

	a.setAccountType(MyMoneyAccount::Income);
	CPPUNIT_ASSERT(a.isAssetLiability() == false);

	a.setAccountType(MyMoneyAccount::Expense);
	CPPUNIT_ASSERT(a.isAssetLiability() == false);

	a.setAccountType(MyMoneyAccount::AssetLoan);
	CPPUNIT_ASSERT(a.isAssetLiability() == true);

	a.setAccountType(MyMoneyAccount::Stock);
	CPPUNIT_ASSERT(a.isAssetLiability() == true);

	a.setAccountType(MyMoneyAccount::Equity);
	CPPUNIT_ASSERT(a.isAssetLiability() == false);
}

void MyMoneyAccountTest::testIsLoan(void)
{
	MyMoneyAccount a;

	a.setAccountType(MyMoneyAccount::UnknownAccountType);
	CPPUNIT_ASSERT(a.isLoan() == false);

	a.setAccountType(MyMoneyAccount::Checkings);
	CPPUNIT_ASSERT(a.isLoan() == false);

	a.setAccountType(MyMoneyAccount::Savings);
	CPPUNIT_ASSERT(a.isLoan() == false);

	a.setAccountType(MyMoneyAccount::Cash);
	CPPUNIT_ASSERT(a.isLoan() == false);

	a.setAccountType(MyMoneyAccount::CreditCard);
	CPPUNIT_ASSERT(a.isLoan() == false);

	a.setAccountType(MyMoneyAccount::Loan);
	CPPUNIT_ASSERT(a.isLoan() == true);

	a.setAccountType(MyMoneyAccount::CertificateDep);
	CPPUNIT_ASSERT(a.isLoan() == false);

	a.setAccountType(MyMoneyAccount::Investment);
	CPPUNIT_ASSERT(a.isLoan() == false);

	a.setAccountType(MyMoneyAccount::MoneyMarket);
	CPPUNIT_ASSERT(a.isLoan() == false);

	a.setAccountType(MyMoneyAccount::Asset);
	CPPUNIT_ASSERT(a.isLoan() == false);

	a.setAccountType(MyMoneyAccount::Liability);
	CPPUNIT_ASSERT(a.isLoan() == false);

	a.setAccountType(MyMoneyAccount::Currency);
	CPPUNIT_ASSERT(a.isLoan() == false);

	a.setAccountType(MyMoneyAccount::Income);
	CPPUNIT_ASSERT(a.isLoan() == false);

	a.setAccountType(MyMoneyAccount::Expense);
	CPPUNIT_ASSERT(a.isLoan() == false);

	a.setAccountType(MyMoneyAccount::AssetLoan);
	CPPUNIT_ASSERT(a.isLoan() == true);

	a.setAccountType(MyMoneyAccount::Stock);
	CPPUNIT_ASSERT(a.isLoan() == false);

	a.setAccountType(MyMoneyAccount::Equity);
	CPPUNIT_ASSERT(a.isLoan() == false);
}

