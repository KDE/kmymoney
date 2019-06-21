/*
 * Copyright 2009-2014  Cristian Oneț <onet.cristian@gmail.com>
 * Copyright 2009-2019  Thomas Baumgart <tbaumgart@kde.org>
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

#include <QtTest>

#define KMM_MYMONEY_UNIT_TESTABLE friend class MyMoneyPayeeTest;

#include "mymoneypayee.h"
#include "mymoneypayee_p.h"
#include "mymoneyenums.h"

using namespace std;

QTEST_GUILESS_MAIN(MyMoneyPayeeTest)

void MyMoneyPayeeTest::testDefaultAccount()
{
  MyMoneyPayee payee;
  QCOMPARE(payee.defaultAccountId().isEmpty(), true);
  QString temp = "Account1";
  payee.setDefaultAccountId(temp);
  QCOMPARE(payee.defaultAccountId().isEmpty(), false);
  QCOMPARE(payee.defaultAccountId(), temp);
  payee.setDefaultAccountId();
  QCOMPARE(payee.defaultAccountId().isEmpty(), true);
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
