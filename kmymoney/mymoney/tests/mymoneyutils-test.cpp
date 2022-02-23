/*
    SPDX-FileCopyrightText: 2022 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "mymoneyutils-test.h"
#include "mymoneyutils.h"

#include <QTest>

#define KMM_MYMONEY_UNIT_TESTABLE friend class MyMoneyUtilsTest;

#include <config-kmymoney.h>

QTEST_GUILESS_MAIN(MyMoneyUtilsTest)

void MyMoneyUtilsTest::init()
{
}

void MyMoneyUtilsTest::cleanup()
{
}

void MyMoneyUtilsTest::commonTestData()
{
    QTest::addColumn<QString>("wildcard");
    QTest::addColumn<QString>("regex");

    QTest::newRow("empty") << QString() << QString();
    QTest::newRow("fixed") << QStringLiteral("thb") << QStringLiteral("thb");

    QTest::newRow("star") << QStringLiteral("*") << QStringLiteral(".*");
    QTest::newRow("starLeadOut") << QStringLiteral("thb*") << QStringLiteral("thb.*");
    QTest::newRow("starLeadIn") << QStringLiteral("*thb") << QStringLiteral(".*thb");
    QTest::newRow("starLeadInAndOut") << QStringLiteral("*thb*") << QStringLiteral(".*thb.*");
    QTest::newRow("starMulti") << QStringLiteral("*t*h*b*") << QStringLiteral(".*t.*h.*b.*");

    QTest::newRow("question") << QStringLiteral("?") << QStringLiteral(".");
    QTest::newRow("questionLeadOut") << QStringLiteral("thb?") << QStringLiteral("thb.");
    QTest::newRow("questionLeadIn") << QStringLiteral("?thb") << QStringLiteral(".thb");
    QTest::newRow("questionLeadInAndOut") << QStringLiteral("?thb?") << QStringLiteral(".thb.");
    QTest::newRow("questionMulti") << QStringLiteral("?t?h?b?") << QStringLiteral(".t.h.b.");

    QTest::newRow("range") << QStringLiteral("[abc]") << QStringLiteral("[abc]");
    QTest::newRow("starInRange") << QStringLiteral("[a*bc]") << QStringLiteral("[a\\*bc]");
    QTest::newRow("questionInRange") << QStringLiteral("[a?bc]") << QStringLiteral("[a\\?bc]");
}

void MyMoneyUtilsTest::testConvertWildcardToRegularExpression_data()
{
    commonTestData();
}

void MyMoneyUtilsTest::testConvertWildcardToRegularExpression()
{
    QFETCH(QString, wildcard);
    QFETCH(QString, regex);

    QCOMPARE(MyMoneyUtils::convertWildcardToRegularExpression(wildcard), regex);
}

void MyMoneyUtilsTest::testConvertRegularExpressionToWildcard_data()
{
    commonTestData();
}

void MyMoneyUtilsTest::testConvertRegularExpressionToWildcard()
{
    QFETCH(QString, wildcard);
    QFETCH(QString, regex);

    QCOMPARE(MyMoneyUtils::convertRegularExpressionToWildcard(regex), wildcard);
}

void MyMoneyUtilsTest::testExtractId_data()
{
    QTest::addColumn<QString>("string");
    QTest::addColumn<unsigned long>("result");

    QTest::newRow("empty") << QString() << 0UL;
    QTest::newRow("no-number") << QStringLiteral("thb") << 0UL;
    QTest::newRow("number-only") << QStringLiteral("123") << 123UL;
    QTest::newRow("text-leadin") << QStringLiteral("TEST-123") << 123UL;
    QTest::newRow("text-leadout") << QStringLiteral("123-TEST") << 0UL;
}

void MyMoneyUtilsTest::testExtractId()
{
    QFETCH(QString, string);
    QFETCH(unsigned long, result);

    QCOMPARE(MyMoneyUtils::extractId(string), result);
}
