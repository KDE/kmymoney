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

#include <QtTest/QtTest>

#define KMM_MYMONEY_UNIT_TESTABLE friend class MyMoneyPayeeTest;

#include "mymoneypayee.h"

using namespace std;

QTEST_MAIN(MyMoneyPayeeTest)

void MyMoneyPayeeTest::testXml()
{
  QDomDocument doc;
  QDomElement parent = doc.createElement("Test");
  doc.appendChild(parent);
  MyMoneyPayee payee1;
  payee1.m_id = "some random id";//if the ID isn't set, w ethrow an exception
  payee1.writeXML(doc, parent);
  QString temp1 = "Account1";
  payee1.setDefaultAccountId(temp1);
  payee1.writeXML(doc, parent);
  QString temp2 = "Account2";
  payee1.setDefaultAccountId(temp2);
  payee1.writeXML(doc, parent);
  payee1.setDefaultAccountId();
  payee1.writeXML(doc, parent);
  QDomElement el = parent.firstChild().toElement();
  QVERIFY(!el.isNull());
  MyMoneyPayee payee2(el);
  QVERIFY(!payee2.defaultAccountEnabled());
  QVERIFY(payee2.defaultAccountId().isEmpty());
  el = el.nextSibling().toElement();
  QVERIFY(!el.isNull());
  MyMoneyPayee payee3(el);
  QVERIFY(payee3.defaultAccountEnabled());
  QVERIFY(payee3.defaultAccountId() == temp1);
  el = el.nextSibling().toElement();
  QVERIFY(!el.isNull());
  MyMoneyPayee payee4(el);
  QVERIFY(payee4.defaultAccountEnabled());
  QVERIFY(payee4.defaultAccountId() == temp2);
  el = el.nextSibling().toElement();
  QVERIFY(!el.isNull());
  MyMoneyPayee payee5(el);
  QVERIFY(!payee5.defaultAccountEnabled());
  QVERIFY(payee5.defaultAccountId().isEmpty());
}

void MyMoneyPayeeTest::testDefaultAccount()
{
  MyMoneyPayee payee;
  QVERIFY(!payee.defaultAccountEnabled());
  QVERIFY(payee.defaultAccountId().isEmpty());
  QString temp = "Account1";
  payee.setDefaultAccountId(temp);
  QVERIFY(payee.defaultAccountEnabled());
  QVERIFY(payee.defaultAccountId() == temp);
  payee.setDefaultAccountId();
  QVERIFY(!payee.defaultAccountEnabled());
  QVERIFY(payee.defaultAccountId().isEmpty());
}

void MyMoneyPayeeTest::testEmptyMatchKeyBegin()
{
  MyMoneyPayee payee;
  QString keys;
  bool ignoreCase;

  payee.setMatchData(MyMoneyPayee::matchKey, false, ";test1;test2");
  QVERIFY(payee.matchData(ignoreCase, keys) == MyMoneyPayee::matchKey);
  QVERIFY(ignoreCase == false);
  QVERIFY(keys == QLatin1String("test1;test2"));

  payee.setMatchData(MyMoneyPayee::matchKey, false, ";;test1;test2");
  QVERIFY(payee.matchData(ignoreCase, keys) == MyMoneyPayee::matchKey);
  QVERIFY(ignoreCase == false);
  QVERIFY(keys == QLatin1String("test1;test2"));
}

void MyMoneyPayeeTest::testEmptyMatchKeyEnd()
{
  MyMoneyPayee payee;
  QString keys;
  bool ignoreCase;

  payee.setMatchData(MyMoneyPayee::matchKey, false, "test1;test2;");
  QVERIFY(payee.matchData(ignoreCase, keys) == MyMoneyPayee::matchKey);
  QVERIFY(ignoreCase == false);
  QVERIFY(keys == QLatin1String("test1;test2"));

  payee.setMatchData(MyMoneyPayee::matchKey, false, "test1;test2;;");
  QVERIFY(payee.matchData(ignoreCase, keys) == MyMoneyPayee::matchKey);
  QVERIFY(ignoreCase == false);
  QVERIFY(keys == QLatin1String("test1;test2"));
}

void MyMoneyPayeeTest::testEmptyMatchKeyMiddle()
{
  MyMoneyPayee payee;
  QString keys;
  bool ignoreCase;

  payee.setMatchData(MyMoneyPayee::matchKey, false, "test1;;test2");
  QVERIFY(payee.matchData(ignoreCase, keys) == MyMoneyPayee::matchKey);
  QVERIFY(ignoreCase == false);
  QVERIFY(keys == QLatin1String("test1;test2"));

  payee.setMatchData(MyMoneyPayee::matchKey, false, "test1;;;test2");
  QVERIFY(payee.matchData(ignoreCase, keys) == MyMoneyPayee::matchKey);
  QVERIFY(ignoreCase == false);
  QVERIFY(keys == QLatin1String("test1;test2"));
}

void MyMoneyPayeeTest::testEmptyMatchKeyMix()
{
  MyMoneyPayee payee;
  QString keys;
  bool ignoreCase;

  payee.setMatchData(MyMoneyPayee::matchKey, false, ";test1;;test2;");
  QVERIFY(payee.matchData(ignoreCase, keys) == MyMoneyPayee::matchKey);
  QVERIFY(ignoreCase == false);
  QVERIFY(keys == QLatin1String("test1;test2"));

  payee.setMatchData(MyMoneyPayee::matchKey, false, ";;test1;;;test2;;");
  QVERIFY(payee.matchData(ignoreCase, keys) == MyMoneyPayee::matchKey);
  QVERIFY(ignoreCase == false);
  QVERIFY(keys == QLatin1String("test1;test2"));
}

void MyMoneyPayeeTest::testMatchKeyDisallowSingleSpace()
{
  MyMoneyPayee payee;
  QString keys;
  bool ignoreCase;

  payee.setMatchData(MyMoneyPayee::matchKey, false, " ");
  QVERIFY(payee.matchData(ignoreCase, keys) == MyMoneyPayee::matchKey);
  QVERIFY(ignoreCase == false);
  QVERIFY(keys == QLatin1String(""));
}

void MyMoneyPayeeTest::testMatchKeyDisallowMultipleSpace()
{
  MyMoneyPayee payee;
  QString keys;
  bool ignoreCase;

  payee.setMatchData(MyMoneyPayee::matchKey, false, "  ");
  QVERIFY(payee.matchData(ignoreCase, keys) == MyMoneyPayee::matchKey);
  QVERIFY(ignoreCase == false);
  QVERIFY(keys == QLatin1String(""));
}

void MyMoneyPayeeTest::testMatchKeyAllowSpaceAtStart()
{
  MyMoneyPayee payee;
  QString keys;
  bool ignoreCase;

  payee.setMatchData(MyMoneyPayee::matchKey, false, " payee");
  QVERIFY(payee.matchData(ignoreCase, keys) == MyMoneyPayee::matchKey);
  QVERIFY(ignoreCase == false);
  QVERIFY(keys == QLatin1String(" payee"));
}

void MyMoneyPayeeTest::testMatchKeyAllowSpaceAtEnd()
{
  MyMoneyPayee payee;
  QString keys;
  bool ignoreCase;

  payee.setMatchData(MyMoneyPayee::matchKey, false, "payee ");
  QVERIFY(payee.matchData(ignoreCase, keys) == MyMoneyPayee::matchKey);
  QVERIFY(ignoreCase == false);
  QVERIFY(keys == QLatin1String("payee "));
}



#include "mymoneypayeetest.moc"

