/*
    SPDX-FileCopyrightText: 2022 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "mymoneyutils-test.h"
#include "mymoneyutils.h"

#include <QTest>
#include <QTimeZone>

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

void MyMoneyUtilsTest::testStringToDateTime_data()
{
    QTest::addColumn<QDate>("date");
    QTest::addColumn<QTime>("time");
    QTest::addColumn<int>("offset");
    QTest::addColumn<QString>("testvalue");

    QTest::newRow("Central European Summertime") << QDate(2023, 10, 3) << QTime(11, 12, 13) << 7200 << QStringLiteral("2023-10-03T11:12:13+02:00");
    QTest::newRow("Island of New Foundland") << QDate(2023, 10, 3) << QTime(11, 12, 13) << -12600 << QStringLiteral("2023-10-03T11:12:13-03:30");
    QTest::newRow("French Polynesia") << QDate(2023, 10, 3) << QTime(11, 12, 13) << -34200 << QStringLiteral("2023-10-03T11:12:13-09:30");
    QTest::newRow("Tonga") << QDate(2023, 10, 3) << QTime(11, 12, 13) << 46800 << QStringLiteral("2023-10-03T11:12:13+13:00");
    QTest::newRow("UTC") << QDate(2023, 10, 3) << QTime(11, 12, 13) << 0 << QStringLiteral("2023-10-03T11:12:13+00:00");
    QTest::newRow("Zulu") << QDate(2023, 10, 3) << QTime(11, 12, 13) << 0 << QStringLiteral("2023-10-03T11:12:13Z");
    QTest::newRow("Date only") << QDate(2023, 10, 3) << QTime(0, 0, 0) << QDateTime::currentDateTime().offsetFromUtc() << QStringLiteral("2023-10-03");
}

void MyMoneyUtilsTest::testStringToDateTime()
{
    QFETCH(QDate, date);
    QFETCH(QTime, time);
    QFETCH(int, offset);
    QFETCH(QString, testvalue);

    QCOMPARE(QDateTime(date, time, QTimeZone(offset)), MyMoneyUtils::stringToDateTime(testvalue));
}

void MyMoneyUtilsTest::testDateTimeToString_data()
{
    QTest::addColumn<QDate>("date");
    QTest::addColumn<QTime>("time");
    QTest::addColumn<int>("offset");
    QTest::addColumn<QString>("result");

    QTest::newRow("Central European Summertime") << QDate(2023, 10, 3) << QTime(11, 12, 13) << 7200 << QStringLiteral("2023-10-03T11:12:13+02:00");
    QTest::newRow("Island of New Foundland") << QDate(2023, 10, 3) << QTime(11, 12, 13) << -12600 << QStringLiteral("2023-10-03T11:12:13-03:30");
    QTest::newRow("French Polynesia") << QDate(2023, 10, 3) << QTime(11, 12, 13) << -34200 << QStringLiteral("2023-10-03T11:12:13-09:30");
    QTest::newRow("Tonga") << QDate(2023, 10, 3) << QTime(11, 12, 13) << 46800 << QStringLiteral("2023-10-03T11:12:13+13:00");
    QTest::newRow("UTC") << QDate(2023, 10, 3) << QTime(11, 12, 13) << 0 << QStringLiteral("2023-10-03T11:12:13+00:00");
}

void MyMoneyUtilsTest::testDateTimeToString()
{
    QFETCH(QDate, date);
    QFETCH(QTime, time);
    QFETCH(int, offset);
    QFETCH(QString, result);

    QCOMPARE(MyMoneyUtils::dateTimeToString(QDateTime(date, time, QTimeZone(offset))), result);
}
