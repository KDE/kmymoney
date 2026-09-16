/*
    SPDX-FileCopyrightText: 2010-2012 Allan Anderson <agander93@gmail.com>
    SPDX-FileCopyrightText: 2022 Alexander Kuznetsov <alx.kuzza@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "parsedata-test.h"
#include <iostream>
#include <map>

#include <QString>
#include <QTest>

#include <mymoneymoney.h>

#include "../csvutil.h"

QTEST_GUILESS_MAIN(ParseDataTest);

ParseDataTest::ParseDataTest()
    : m_parse(nullptr)
{
}

void ParseDataTest::init()
{
    m_parse = new Parse;
}

void ParseDataTest::cleanup()
{
    delete m_parse;
}

void ParseDataTest::parseSplitString()
{
    QVector<FieldDelimiter> delimiters{FieldDelimiter::Comma, FieldDelimiter::Semicolon, FieldDelimiter::Colon, FieldDelimiter::Tab, FieldDelimiter::Pipe};
    QVector<QChar> delimiter_chars{',', ';', ':', '\t', '|'};
    for (auto i = 0; i < delimiters.size(); i++) {
        auto delimiter_char = delimiter_chars[i];
        auto delimiter = delimiters[i];

        m_parse->setFieldDelimiter(delimiter);

        // construct CSV string to parse like: abc,defgh,"abc "",def"
        QString input;
        input.append("abc");
        input.append(delimiter_char);
        input.append("defgh");
        input.append(delimiter_char);
        input.append("\"abc \"\",def\""); // outer quotes, comma inside and inside quote

        QStringList expected = {"abc", "defgh", "abc \",def"};

        auto result = m_parse->parseLine(input);

        QCOMPARE(result, expected);
    }
}

void ParseDataTest::testPossiblyReplaceSymbol_data()
{
    QTest::addColumn<QString>("inputValue");
    QTest::addColumn<FieldDelimiter>("fieldDelimiter");
    QTest::addColumn<DecimalSymbol>("decimalSymbol");
    QTest::addColumn<QString>("result");

    auto resultString = [](const QString& decimal, const QString& fraction = QString()) {
        if (fraction.isEmpty()) {
            return decimal;
        }
        return QString(decimal + MyMoneyMoney::decimalSeparator() + fraction);
    };

    QTest::newRow("comma, dot, dot used parens") << QStringLiteral("(12.34)") << FieldDelimiter::Comma << DecimalSymbol::Dot
                                                 << resultString(QStringLiteral("-12"), QLatin1String("34"));
    QTest::newRow("semicolon, comma, comma used negative")
        << QStringLiteral("-12,34") << FieldDelimiter::Semicolon << DecimalSymbol::Comma << resultString(QStringLiteral("-12"), QLatin1String("34"));
    QTest::newRow("semicolon, comma, comma used") << QStringLiteral("12,34") << FieldDelimiter::Semicolon << DecimalSymbol::Comma
                                                  << resultString(QStringLiteral("12"), QLatin1String("34"));
    QTest::newRow("comma, dot, dot used 1") << QStringLiteral("12.34") << FieldDelimiter::Comma << DecimalSymbol::Dot
                                            << resultString(QStringLiteral("12"), QLatin1String("34"));
    QTest::newRow("comma, dot, dot used 2") << QStringLiteral("12.") << FieldDelimiter::Comma << DecimalSymbol::Dot
                                            << resultString(QStringLiteral("12"), QLatin1String("00"));
    QTest::newRow("comma, dot, dot used 3") << QStringLiteral("12") << FieldDelimiter::Comma << DecimalSymbol::Dot
                                            << resultString(QStringLiteral("12"), QLatin1String("00"));
    QTest::newRow("semicolon, dot, dot used") << QStringLiteral("12.34") << FieldDelimiter::Semicolon << DecimalSymbol::Dot
                                              << resultString(QStringLiteral("12"), QLatin1String("34"));
    QTest::newRow("semicolon, dot, comma used") << QStringLiteral("12,34") << FieldDelimiter::Semicolon << DecimalSymbol::Dot
                                                << resultString(QStringLiteral("1234"));
    QTest::newRow("semicolon, dot, no decimals 1") << QStringLiteral("12,340") << FieldDelimiter::Semicolon << DecimalSymbol::Dot
                                                   << resultString(QStringLiteral("12340"), QLatin1String("00"));
    QTest::newRow("semicolon, dot, no decimals 2") << QStringLiteral("12,340.") << FieldDelimiter::Semicolon << DecimalSymbol::Dot
                                                   << resultString(QStringLiteral("12340"), QLatin1String("00"));
}

void ParseDataTest::testPossiblyReplaceSymbol()
{
    QFETCH(QString, inputValue);
    QFETCH(FieldDelimiter, fieldDelimiter);
    QFETCH(DecimalSymbol, decimalSymbol);
    QFETCH(QString, result);

    m_parse->setFieldDelimiter(fieldDelimiter);
    m_parse->setDecimalSymbol(decimalSymbol);

    QCOMPARE(m_parse->possiblyReplaceSymbol(inputValue), result);
}

void ParseDataTest::cleanupTestCase()
{
}

void ParseDataTest::testConstructor()
{
}

void ParseDataTest::testConstructor_data()
{
}

void ParseDataTest::testDefaultConstructor()
{
}

void ParseDataTest::testDefaultConstructor_data()
{
}

void ParseDataTest::initTestCase()
{
}

void ParseDataTest::initTestCase_data()
{
}
