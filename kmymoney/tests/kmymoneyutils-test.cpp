/*
    SPDX-FileCopyrightText: 2012 Thomas Baumgart <ipwizard@users.sourceforge.net>
    SPDX-FileCopyrightText: 2015 Christian Dávid <christian-david@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kmymoneyutils-test.h"
#include "mymoneyaccount.h"

#include <QTest>

QTEST_GUILESS_MAIN(KMyMoneyUtilsTest)

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

    // a number preceded by text
    acc.setValue("lastNumberUsed", QLatin1String("No 123"));
    QVERIFY(KMyMoneyUtils::nextCheckNumber(acc) == QLatin1String("No 124"));

    // a number followed by text
    acc.setValue("lastNumberUsed", QLatin1String("123 ABC"));
    QVERIFY(KMyMoneyUtils::nextCheckNumber(acc) == QLatin1String("124 ABC"));

    // a number enclosed by text
    acc.setValue("lastNumberUsed", QLatin1String("No 123 ABC"));
    QVERIFY(KMyMoneyUtils::nextCheckNumber(acc) == QLatin1String("No 124 ABC"));

    // a number containing a dash (e.g. invoice number)
    acc.setValue("lastNumberUsed", QLatin1String("No 123-001 ABC"));
    QVERIFY(KMyMoneyUtils::nextCheckNumber(acc) == QLatin1String("No 123-002 ABC"));

    // a number containing a dot (e.g. invoice number)
    acc.setValue("lastNumberUsed", QLatin1String("2012.001"));
    QVERIFY(KMyMoneyUtils::nextCheckNumber(acc) == QLatin1String("2012.002"));

}
