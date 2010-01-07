/***************************************************************************
                          mymoneyinstitutiontest.cpp
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

#include "mymoneyinstitutiontest.h"
#include <mymoneyexception.h>

MyMoneyInstitutionTest::MyMoneyInstitutionTest()
{
}


void MyMoneyInstitutionTest::setUp () {
	m = new MyMoneyInstitution();
	n = new MyMoneyInstitution("name", "town", "street", "postcode",
			    "telephone", "manager", "sortcode");
}

void MyMoneyInstitutionTest::tearDown () {
	delete m;
	delete n;
}

void MyMoneyInstitutionTest::testEmptyConstructor() {
	CPPUNIT_ASSERT(m->id().isEmpty());
	CPPUNIT_ASSERT(m->street().isEmpty());
	CPPUNIT_ASSERT(m->town().isEmpty());
	CPPUNIT_ASSERT(m->postcode().isEmpty());
	CPPUNIT_ASSERT(m->telephone().isEmpty());
	CPPUNIT_ASSERT(m->manager().isEmpty());

	CPPUNIT_ASSERT(m->accountCount() == 0);
}

void MyMoneyInstitutionTest::testSetFunctions() {
	m->setStreet("street");
	m->setTown("town");
	m->setPostcode("postcode");
	m->setTelephone("telephone");
	m->setManager("manager");
	m->setName("name");

	CPPUNIT_ASSERT(m->id().isEmpty());
	CPPUNIT_ASSERT(m->street() == "street");
	CPPUNIT_ASSERT(m->town() == "town");
	CPPUNIT_ASSERT(m->postcode() == "postcode");
	CPPUNIT_ASSERT(m->telephone() == "telephone");
	CPPUNIT_ASSERT(m->manager() == "manager");
	CPPUNIT_ASSERT(m->name() == "name");
}

void MyMoneyInstitutionTest::testNonemptyConstructor() {
	CPPUNIT_ASSERT(n->id().isEmpty());
	CPPUNIT_ASSERT(n->street() == "street");
	CPPUNIT_ASSERT(n->town() == "town");
	CPPUNIT_ASSERT(n->postcode() == "postcode");
	CPPUNIT_ASSERT(n->telephone() == "telephone");
	CPPUNIT_ASSERT(n->manager() == "manager");
	CPPUNIT_ASSERT(n->name() == "name");
	CPPUNIT_ASSERT(n->sortcode() == "sortcode");
}

void MyMoneyInstitutionTest::testCopyConstructor() {
	MyMoneyInstitution* n1 = new MyMoneyInstitution("GUID1", *n);
	MyMoneyInstitution n2(*n1);

	CPPUNIT_ASSERT(*n1 == n2);

	delete n1;
}

void MyMoneyInstitutionTest::testMyMoneyFileConstructor() {
	MyMoneyInstitution *t = new MyMoneyInstitution("GUID", *n);

	CPPUNIT_ASSERT(t->id() == "GUID");

	CPPUNIT_ASSERT(t->street() == "street");
	CPPUNIT_ASSERT(t->town() == "town");
	CPPUNIT_ASSERT(t->postcode() == "postcode");
	CPPUNIT_ASSERT(t->telephone() == "telephone");
	CPPUNIT_ASSERT(t->manager() == "manager");
	CPPUNIT_ASSERT(t->name() == "name");
	CPPUNIT_ASSERT(t->sortcode() == "sortcode");

	delete t;
}

void MyMoneyInstitutionTest::testEquality () {
	MyMoneyInstitution t("name", "town", "street", "postcode",
			"telephone", "manager", "sortcode");

	CPPUNIT_ASSERT(t == *n);
	t.setStreet("x");
	CPPUNIT_ASSERT(!(t == *n));
	t.setStreet("street");
	CPPUNIT_ASSERT(t == *n);
	t.setName("x");
	CPPUNIT_ASSERT(!(t == *n));
	t.setName("name");
	CPPUNIT_ASSERT(t == *n);
	t.setTown("x");
	CPPUNIT_ASSERT(!(t == *n));
	t.setTown("town");
	CPPUNIT_ASSERT(t == *n);
	t.setPostcode("x");
	CPPUNIT_ASSERT(!(t == *n));
	t.setPostcode("postcode");
	CPPUNIT_ASSERT(t == *n);
	t.setTelephone("x");
	CPPUNIT_ASSERT(!(t == *n));
	t.setTelephone("telephone");
	CPPUNIT_ASSERT(t == *n);
	t.setManager("x");
	CPPUNIT_ASSERT(!(t == *n));
	t.setManager("manager");
	CPPUNIT_ASSERT(t == *n);

	MyMoneyInstitution* n1 = new MyMoneyInstitution("GUID1", *n);
	MyMoneyInstitution* n2 = new MyMoneyInstitution("GUID1", *n);

	n1->addAccountId("A000001");
	n2->addAccountId("A000001");

	CPPUNIT_ASSERT(*n1 == *n2);

	delete n1;
	delete n2;
}

void MyMoneyInstitutionTest::testInequality () {
	MyMoneyInstitution* n1 = new MyMoneyInstitution("GUID0", *n);
	MyMoneyInstitution* n2 = new MyMoneyInstitution("GUID1", *n);
	MyMoneyInstitution* n3 = new MyMoneyInstitution("GUID2", *n);
	MyMoneyInstitution* n4 = new MyMoneyInstitution("GUID2", *n);

	CPPUNIT_ASSERT(!(*n1 == *n2));
	CPPUNIT_ASSERT(!(*n1 == *n3));
	CPPUNIT_ASSERT(*n3 == *n4);

	n3->addAccountId("A000001");
	n4->addAccountId("A000002");
	CPPUNIT_ASSERT(!(*n3 == *n4));

	delete n1;
	delete n2;
	delete n3;
	delete n4;
}

void MyMoneyInstitutionTest::testAccountIDList () {
	MyMoneyInstitution institution;
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

void MyMoneyInstitutionTest::testWriteXML() {
	MyMoneyKeyValueContainer kvp;

	n->addAccountId("A000001");
	n->addAccountId("A000003");
	n->setValue(QString("key"), "value");

	QDomDocument doc("TEST");
	QDomElement el = doc.createElement("INSTITUTION-CONTAINER");
	doc.appendChild(el);

	MyMoneyInstitution i("I00001", *n);

	i.writeXML(doc, el);

	QString ref = QString(
		"<!DOCTYPE TEST>\n"
		"<INSTITUTION-CONTAINER>\n"
		" <INSTITUTION manager=\"manager\" id=\"I00001\" name=\"name\" sortcode=\"sortcode\" >\n"
		"  <ADDRESS street=\"street\" telephone=\"telephone\" zip=\"postcode\" city=\"town\" />\n"
		"  <ACCOUNTIDS>\n"
		"   <ACCOUNTID id=\"A000001\" />\n"
		"   <ACCOUNTID id=\"A000003\" />\n"
		"  </ACCOUNTIDS>\n"
                "  <KEYVALUEPAIRS>\n"
                "   <PAIR key=\"key\" value=\"value\" />\n"
                "  </KEYVALUEPAIRS>\n"
		" </INSTITUTION>\n"
		"</INSTITUTION-CONTAINER>\n");

#if QT_VERSION >= QT_VERSION_CHECK(4,6,0)
	ref.replace(QString(" />\n"), QString("/>\n"));
	ref.replace(QString(" >\n"), QString(">\n"));
#endif

	// qDebug("ref = '%s'", qPrintable(ref));
	// qDebug("doc = '%s'", qPrintable(doc.toString()));

	CPPUNIT_ASSERT(doc.toString() == ref);
}

void MyMoneyInstitutionTest::testReadXML() {
	MyMoneyInstitution i;
	QString ref_ok = QString(
		"<!DOCTYPE TEST>\n"
		"<INSTITUTION-CONTAINER>\n"
		" <INSTITUTION sortcode=\"sortcode\" id=\"I00001\" manager=\"manager\" name=\"name\" >\n"
		"  <ADDRESS street=\"street\" zip=\"postcode\" city=\"town\" telephone=\"telephone\" />\n"
		"  <ACCOUNTIDS>\n"
		"   <ACCOUNTID id=\"A000001\" />\n"
		"   <ACCOUNTID id=\"A000003\" />\n"
		"  </ACCOUNTIDS>\n"
                "  <KEYVALUEPAIRS>\n"
                "   <PAIR key=\"key\" value=\"value\" />\n"
                "  </KEYVALUEPAIRS>\n"
		" </INSTITUTION>\n"
		"</INSTITUTION-CONTAINER>\n");

	QString ref_false = QString(
		"<!DOCTYPE TEST>\n"
		"<INSTITUTION-CONTAINER>\n"
		" <KINSTITUTION sortcode=\"sortcode\" id=\"I00001\" manager=\"manager\" name=\"name\" >\n"
		"  <ADDRESS street=\"street\" zip=\"postcode\" city=\"town\" telephone=\"telephone\" />\n"
		"  <ACCOUNTIDS>\n"
		"   <ACCOUNTID id=\"A000001\" />\n"
		"   <ACCOUNTID id=\"A000003\" />\n"
		"  </ACCOUNTIDS>\n"
		" </KINSTITUTION>\n"
		"</INSTITUTION-CONTAINER>\n");

	QDomDocument doc;
	QDomElement node;

	doc.setContent(ref_false);
	node = doc.documentElement().firstChild().toElement();
	try {
		i = MyMoneyInstitution(node);
		CPPUNIT_FAIL("Missing expected exception");
	} catch(MyMoneyException *e) {
		delete e;
	}

	i.addAccountId("TEST");

	doc.setContent(ref_ok);
	node = doc.documentElement().firstChild().toElement();
	try {
		QStringList alist;
		alist << "A000001" << "A000003";
		i = MyMoneyInstitution(node);

		CPPUNIT_ASSERT(i.sortcode() == "sortcode");
		CPPUNIT_ASSERT(i.id() == "I00001");
		CPPUNIT_ASSERT(i.manager() == "manager");
		CPPUNIT_ASSERT(i.name() == "name");
		CPPUNIT_ASSERT(i.street() == "street");
		CPPUNIT_ASSERT(i.postcode() == "postcode");
		CPPUNIT_ASSERT(i.city() == "town");
		CPPUNIT_ASSERT(i.telephone() == "telephone");
		CPPUNIT_ASSERT(i.accountList() == alist);
		CPPUNIT_ASSERT(i.value(QString("key")) == "value");

	} catch(MyMoneyException *e) {
		delete e;
		CPPUNIT_FAIL("Unexpected exception");
	}
}
