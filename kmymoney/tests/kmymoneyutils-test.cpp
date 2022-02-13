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

void KMyMoneyUtilsTest::testGetAdjacentNumber_data()
{
    QTest::addColumn<QString>("lastnumber");
    QTest::addColumn<QString>("nextnumber");

    QTest::newRow("empty") << QString() << QStringLiteral("1");
    QTest::newRow("simple number") << QStringLiteral("123") << QStringLiteral("124");
    QTest::newRow("text in front") << QStringLiteral("No 123") << QStringLiteral("No 124");
    QTest::newRow("text following") << QStringLiteral("123 ABC") << QStringLiteral("124 ABC");
    QTest::newRow("enclosed in text") << QStringLiteral("No 123 ABC") << QStringLiteral("No 124 ABC");
    QTest::newRow("number with hyphen") << QStringLiteral("No 123-001 ABC") << QStringLiteral("No 123-002 ABC");
    QTest::newRow("number with dot") << QStringLiteral("2012.001") << QStringLiteral("2012.002");
}

void KMyMoneyUtilsTest::testGetAdjacentNumber()
{
    QFETCH(QString, lastnumber);
    QFETCH(QString, nextnumber);

    QCOMPARE(KMyMoneyUtils::getAdjacentNumber(lastnumber), nextnumber);
}
