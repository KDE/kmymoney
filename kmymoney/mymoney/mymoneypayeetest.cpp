/***************************************************************************
                          mymoneypayeetest.cpp
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "mymoneypayeetest.h"
#include <iostream>
#include <fstream>
using namespace std;

MyMoneyPayeeTest:: MyMoneyPayeeTest () {
}

void MyMoneyPayeeTest::setUp () {
}

void MyMoneyPayeeTest::tearDown () {
}

void MyMoneyPayeeTest::testXml(){
  QDomDocument doc;
  QDomElement parent = doc.createElement("Test");
  doc.appendChild(parent);
  MyMoneyPayee payee1;
  payee1.m_id = "some random id";//if the ID isn't set, w ethrow an exception
  payee1.writeXML(doc,parent);
  QString temp1 = "Account1";
  payee1.setDefaultAccountId(temp1);
  payee1.writeXML(doc,parent);
  QString temp2 = "Account2";
  payee1.setDefaultAccountId(temp2);
  payee1.writeXML(doc,parent);
  payee1.setDefaultAccountId();
  payee1.writeXML(doc,parent);
  QDomElement el = parent.firstChild().toElement();
  CPPUNIT_ASSERT(!el.isNull());
  MyMoneyPayee payee2(el);
  CPPUNIT_ASSERT(!payee2.defaultAccountEnabled());
  CPPUNIT_ASSERT(payee2.defaultAccountId().isEmpty());
  el = el.nextSibling().toElement();
  CPPUNIT_ASSERT(!el.isNull());
  MyMoneyPayee payee3(el);
  CPPUNIT_ASSERT(payee3.defaultAccountEnabled());
  CPPUNIT_ASSERT(payee3.defaultAccountId()==temp1);
  el = el.nextSibling().toElement();
  CPPUNIT_ASSERT(!el.isNull());
  MyMoneyPayee payee4(el);
  CPPUNIT_ASSERT(payee4.defaultAccountEnabled());
  CPPUNIT_ASSERT(payee4.defaultAccountId()==temp2);
  el = el.nextSibling().toElement();
  CPPUNIT_ASSERT(!el.isNull());
  MyMoneyPayee payee5(el);
  CPPUNIT_ASSERT(!payee5.defaultAccountEnabled());
  CPPUNIT_ASSERT(payee5.defaultAccountId().isEmpty());
}

void MyMoneyPayeeTest::testDefaultAccount(){
  MyMoneyPayee payee;
  CPPUNIT_ASSERT(!payee.defaultAccountEnabled());
  CPPUNIT_ASSERT(payee.defaultAccountId().isEmpty());
  QString temp = "Account1";
  payee.setDefaultAccountId(temp);
  CPPUNIT_ASSERT(payee.defaultAccountEnabled());
  CPPUNIT_ASSERT(payee.defaultAccountId()==temp);
  payee.setDefaultAccountId();
  CPPUNIT_ASSERT(!payee.defaultAccountEnabled());
  CPPUNIT_ASSERT(payee.defaultAccountId().isEmpty());
}
