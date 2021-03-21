/*
    SPDX-FileCopyrightText: 2010-2012 Allan Anderson <agander93@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "parsedata-test.h"

#include <QtTest>
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
    QVector<FieldDelimiter> delimiters {FieldDelimiter::Comma, FieldDelimiter::Semicolon, FieldDelimiter::Colon, FieldDelimiter::Tab};
    foreach (const auto delimiter, delimiters) { //        All four delimiters should produce same result
        m_parse->setFieldDelimiter(delimiter);

        QString input = "abc,defgh,";//  When this string is QString::split(), two strings
        //  ....will result if ',' is the field delimiter.
        //      This is not good.
        input.prepend('"');  //            make input string quoted
        input.append('"');
        QStringList expected;
        expected << "abc,defgh,";
        QVERIFY(m_parse->parseLine(input) == expected);   // if parseLine() detects the condition,
    }                                                   // ...it rebuilds the string
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
