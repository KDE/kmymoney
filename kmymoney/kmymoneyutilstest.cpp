/***************************************************************************
 *   Copyright 2012  Thomas Baumgart  ipwizard@users.sourceforge.net       *
 *                                                                         *
 *   This file is part of KMyMoney.                                        *
 *                                                                         *
 *   KMyMoney is free software; you can redistribute it and/or             *
 *   modify it under the terms of the GNU General Public License           *
 *   as published by the Free Software Foundation; either version 2 of     *
 *   the License or (at your option) version 3 or any later version.       *
 *                                                                         *
 *   KMyMoney is distributed in the hope that it will be useful,           *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>  *
 ***************************************************************************/

#include "kmymoneyutilstest.h"

#include <QtTest/QtTest>

QTEST_MAIN(KMyMoneyUtilsTest)

void KMyMoneyUtilsTest::init()
{
}

void KMyMoneyUtilsTest::cleanup()
{
}

void KMyMoneyUtilsTest::initTestCase()
{
}

void KMyMoneyUtilsTest::testNextCheckNumber()
{
  MyMoneyAccount acc;

  // make sure first check number is 1
  acc.setValue("lastNumberUsed", QString());
  QVERIFY(KMyMoneyUtils::nextCheckNumber(acc) == QLatin1String("1"));

  // a simple increment of a plain value
  acc.setValue("lastNumberUsed", QLatin1String("123"));
  QVERIFY(KMyMoneyUtils::nextCheckNumber(acc) == QLatin1String("124"));

  // a number preceeded by text
  acc.setValue("lastNumberUsed", QLatin1String("No 123"));
  QVERIFY(KMyMoneyUtils::nextCheckNumber(acc) == QLatin1String("No 124"));

  // a number followed by text
  acc.setValue("lastNumberUsed", QLatin1String("123 ABC"));
  QVERIFY(KMyMoneyUtils::nextCheckNumber(acc) == QLatin1String("124 ABC"));

  // a number enclosed by text
  acc.setValue("lastNumberUsed", QLatin1String("No 123 ABC"));
  QVERIFY(KMyMoneyUtils::nextCheckNumber(acc) == QLatin1String("No 124 ABC"));

  // a number containig a dash (e.g. invoice number)
  acc.setValue("lastNumberUsed", QLatin1String("No 123-001 ABC"));
  QVERIFY(KMyMoneyUtils::nextCheckNumber(acc) == QLatin1String("No 123-002 ABC"));

  // a number containing a dot (e.g. invoice number)
  acc.setValue("lastNumberUsed", QLatin1String("2012.001"));
  QVERIFY(KMyMoneyUtils::nextCheckNumber(acc) == QLatin1String("2012.002"));

}

// required for link phase. we need to cleanup the register code which
// still references it
void timetrace(char const *)
{
}
