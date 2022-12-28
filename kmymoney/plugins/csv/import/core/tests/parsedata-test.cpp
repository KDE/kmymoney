/*
    SPDX-FileCopyrightText: 2010-2012 Allan Anderson <agander93@gmail.com>
    SPDX-FileCopyrightText: 2022 Alexander Kuznetsov <alx.kuzza@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "parsedata-test.h"

#include <QTest>
#include <QString>

#include "../csvutil.h"

QTEST_GUILESS_MAIN(ParseDataTest);

ParseDataTest::ParseDataTest() :
    m_parse(nullptr)
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
    QVector<FieldDelimiter> delimiters{FieldDelimiter::Comma, FieldDelimiter::Semicolon, FieldDelimiter::Colon, FieldDelimiter::Tab};
    QVector<QChar> delimiter_chars{',', ';', ':', '\t'};
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

        QVERIFY(result == expected);
    }
}

void ParseDataTest::parse_data()
{
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
