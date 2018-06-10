/*
 * Copyright 2009-2014  Cristian Oneț <onet.cristian@gmail.com>
 * Copyright 2009-2017  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2016       Christian Dávid <christian-david@web.de>
 * Copyright 2017-2018  Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "mymoneypayee-test.h"

#include <QDomDocument>
#include <QDomElement>

#include <QtTest>

#define KMM_MYMONEY_UNIT_TESTABLE friend class MyMoneyPayeeTest;

#include "mymoneypayee.h"
#include "mymoneypayee_p.h"
#include "mymoneyenums.h"

using namespace std;

QTEST_GUILESS_MAIN(MyMoneyPayeeTest)

void MyMoneyPayeeTest::testXml()
{
  QDomDocument doc;
  QDomElement parent = doc.createElement("Test");
  doc.appendChild(parent);
  MyMoneyPayee payee1;
  payee1.d_func()->m_id = "some random id";//if the ID isn't set, w ethrow an exception
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

  payee.setMatchData(eMyMoney::Payee::MatchType::Key, false, "\ntest1\ntest2");
  QVERIFY(payee.matchData(ignoreCase, keys) == eMyMoney::Payee::MatchType::Key);
  QVERIFY(ignoreCase == false);
  QVERIFY(keys == QLatin1String("test1\ntest2"));

  payee.setMatchData(eMyMoney::Payee::MatchType::Key, false, "\n\ntest1\ntest2");
  QVERIFY(payee.matchData(ignoreCase, keys) == eMyMoney::Payee::MatchType::Key);
  QVERIFY(ignoreCase == false);
  QVERIFY(keys == QLatin1String("test1\ntest2"));
}

void MyMoneyPayeeTest::testEmptyMatchKeyEnd()
{
  MyMoneyPayee payee;
  QString keys;
  bool ignoreCase;

  payee.setMatchData(eMyMoney::Payee::MatchType::Key, false, "test1\ntest2\n");
  QVERIFY(payee.matchData(ignoreCase, keys) == eMyMoney::Payee::MatchType::Key);
  QVERIFY(ignoreCase == false);
  QVERIFY(keys == QLatin1String("test1\ntest2"));

  payee.setMatchData(eMyMoney::Payee::MatchType::Key, false, "test1\ntest2\n\n");
  QVERIFY(payee.matchData(ignoreCase, keys) == eMyMoney::Payee::MatchType::Key);
  QVERIFY(ignoreCase == false);
  QVERIFY(keys == QLatin1String("test1\ntest2"));
}

void MyMoneyPayeeTest::testEmptyMatchKeyMiddle()
{
  MyMoneyPayee payee;
  QString keys;
  bool ignoreCase;

  payee.setMatchData(eMyMoney::Payee::MatchType::Key, false, "test1\n\ntest2");
  QVERIFY(payee.matchData(ignoreCase, keys) == eMyMoney::Payee::MatchType::Key);
  QVERIFY(ignoreCase == false);
  QVERIFY(keys == QLatin1String("test1\ntest2"));

  payee.setMatchData(eMyMoney::Payee::MatchType::Key, false, "test1\n\n\ntest2");
  QVERIFY(payee.matchData(ignoreCase, keys) == eMyMoney::Payee::MatchType::Key);
  QVERIFY(ignoreCase == false);
  QVERIFY(keys == QLatin1String("test1\ntest2"));
}

void MyMoneyPayeeTest::testEmptyMatchKeyMix()
{
  MyMoneyPayee payee;
  QString keys;
  bool ignoreCase;

  payee.setMatchData(eMyMoney::Payee::MatchType::Key, false, "\ntest1\n\ntest2\n");
  QVERIFY(payee.matchData(ignoreCase, keys) == eMyMoney::Payee::MatchType::Key);
  QVERIFY(ignoreCase == false);
  QVERIFY(keys == QLatin1String("test1\ntest2"));

  payee.setMatchData(eMyMoney::Payee::MatchType::Key, false, "\n\ntest1\n\n\ntest2\n\n");
  QVERIFY(payee.matchData(ignoreCase, keys) == eMyMoney::Payee::MatchType::Key);
  QVERIFY(ignoreCase == false);
  QVERIFY(keys == QLatin1String("test1\ntest2"));
}

void MyMoneyPayeeTest::testMatchKeyDisallowSingleSpace()
{
  MyMoneyPayee payee;
  QString keys;
  bool ignoreCase;

  payee.setMatchData(eMyMoney::Payee::MatchType::Key, false, " ");
  QVERIFY(payee.matchData(ignoreCase, keys) == eMyMoney::Payee::MatchType::Key);
  QVERIFY(ignoreCase == false);
  QVERIFY(keys == QLatin1String(""));
}

void MyMoneyPayeeTest::testMatchKeyDisallowMultipleSpace()
{
  MyMoneyPayee payee;
  QString keys;
  bool ignoreCase;

  payee.setMatchData(eMyMoney::Payee::MatchType::Key, false, "  ");
  QVERIFY(payee.matchData(ignoreCase, keys) == eMyMoney::Payee::MatchType::Key);
  QVERIFY(ignoreCase == false);
  QVERIFY(keys == QLatin1String(""));
}

void MyMoneyPayeeTest::testMatchKeyAllowSpaceAtStart()
{
  MyMoneyPayee payee;
  QString keys;
  bool ignoreCase;

  payee.setMatchData(eMyMoney::Payee::MatchType::Key, false, " payee");
  QVERIFY(payee.matchData(ignoreCase, keys) == eMyMoney::Payee::MatchType::Key);
  QVERIFY(ignoreCase == false);
  QVERIFY(keys == QLatin1String(" payee"));
}

void MyMoneyPayeeTest::testMatchKeyAllowSpaceAtEnd()
{
  MyMoneyPayee payee;
  QString keys;
  bool ignoreCase;

  payee.setMatchData(eMyMoney::Payee::MatchType::Key, false, "payee ");
  QVERIFY(payee.matchData(ignoreCase, keys) == eMyMoney::Payee::MatchType::Key);
  QVERIFY(ignoreCase == false);
  QVERIFY(keys == QLatin1String("payee "));
}

void MyMoneyPayeeTest::testMatchNameExact()
{
  MyMoneyPayee payee;
  QString keys;
  bool ignoreCase;

  payee.setMatchData(eMyMoney::Payee::MatchType::NameExact, false, keys);
  keys = QLatin1String("payee ");
  QCOMPARE(payee.matchData(ignoreCase, keys), eMyMoney::Payee::MatchType::NameExact);
  QCOMPARE(ignoreCase, false);
  QVERIFY(keys.isEmpty());
}

void MyMoneyPayeeTest::testElementNames()
{
  for (auto i = (int)Payee::Element::Address; i <= (int)Payee::Element::Address; ++i) {
    auto isEmpty = MyMoneyPayeePrivate::getElName(static_cast<Payee::Element>(i)).isEmpty();
    if (isEmpty)
      qWarning() << "Empty element's name " << i;
    QVERIFY(!isEmpty);
  }
}

void MyMoneyPayeeTest::testAttributeNames()
{
  for (auto i = (int)Payee::Attribute::Name; i < (int)Payee::Attribute::LastAttribute; ++i) {
    auto isEmpty = MyMoneyPayeePrivate::getAttrName(static_cast<Payee::Attribute>(i)).isEmpty();
    if (isEmpty)
      qWarning() << "Empty attribute's name " << i;
    QVERIFY(!isEmpty);
  }
}
