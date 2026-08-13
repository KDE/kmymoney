/*
    SPDX-FileCopyrightText: 2011-2012 Allan Anderson <agander93@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "test-symbol.h"

#include "../csvutil.h"

#include <QLocale>
#include <QString>
#include <QTest>

#include "mymoneymoney.h"

QTEST_GUILESS_MAIN(SymbolTest);

Parse* m_parse;

SymbolTest::SymbolTest()
    : m_parse(nullptr)
{
}

void SymbolTest::init()
{
    m_parse = new Parse;
    m_parse->setDecimalSymbol(DecimalSymbol::Dot);
    // the converted strings are fed into MyMoneyMoney, so they must use
    // the decimal separator MyMoneyMoney parses with and not the one of QLocale()
    m_localeDecimal = MyMoneyMoney::decimalSeparator();
    m_localeThousands = MyMoneyMoney::thousandSeparator();
    m_savedDecimal = MyMoneyMoney::decimalSeparator();
    m_savedThousands = MyMoneyMoney::thousandSeparator();
}

void SymbolTest::cleanup()
{
    MyMoneyMoney::setDecimalSeparator(m_savedDecimal);
    MyMoneyMoney::setThousandSeparator(m_savedThousands);
    delete m_parse;
}

void SymbolTest::testDecimalSymbolDot()
{
    //  Detect '.' as decimal and replace from locale

    m_parse->setDecimalSymbol(DecimalSymbol::Dot); //  "."

    QFETCH(QString, input);
    QFETCH(QString, result);

    QVERIFY(m_parse->possiblyReplaceSymbol(input) == result);
}

void SymbolTest::testDecimalSymbolComma()
{
    //  Detect ',' as decimal and replace from locale

    m_parse->setDecimalSymbol(DecimalSymbol::Comma); //   ","

    QFETCH(QString, input);
    QFETCH(QString, result);

    QCOMPARE(m_parse->possiblyReplaceSymbol(input), result);
}

void SymbolTest::testDecimalSymbolInvalid()
{
    //  Check for ',' as decimal, and none present

    m_parse->setDecimalSymbol(DecimalSymbol::Comma); //   ","
    m_testDecimal = m_parse->decimalSymbol(DecimalSymbol::Comma);

    QFETCH(QString, input);
    // QFETCH(QString, result);
    m_parse->possiblyReplaceSymbol(input);

    QVERIFY(m_parse->invalidConversion() == true);
}

void SymbolTest::testDecimalSymbolDot_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("result");

    //  Detect '.' as decimal and replace from locale
    QTest::newRow("test 1") << "1234.56" << QString("1234" + m_localeDecimal + "56");

    //  Check for '.' as decimal, and none present
    QTest::newRow("test 2") << "145" << QString("145" + m_localeDecimal + "00");

    //  Detect '.' as decimal and replace from locale,
    //  with thousands separator present
    QTest::newRow("test 3") << "-123,456.78" << QString("-123456" + m_localeDecimal + "78");

    //  Detect '.' as decimal and replace from locale
    //  and thousands separator present
    QTest::newRow("test 4") << "123,456.78" << QString("123456" + m_localeDecimal + "78");

    //  Detect '.' as decimal and replace from locale
    //  and thousands separator present
    QTest::newRow("test 5") << "987,654.32" << QString("987654" + m_localeDecimal + "32");
}

void SymbolTest::testDecimalSymbolComma_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("result");

    //  Detect ',' as decimal and replace from locale

    QTest::newRow("test 1") << "$987,654" << QString("$987" + m_localeDecimal + "654");

    //  Detect ',' as decimal and replace from locale
    //  with thousands separator present

    QTest::newRow("test 2") << "-123.456,78" << QString("-123456" + m_localeDecimal + "78");

    QTest::newRow("test 3") << "145" << QString("145" + m_localeDecimal + "00");

    //  Check for ',' as decimal
    QTest::newRow("test 4") << "123.456" << QString("123456" + m_localeDecimal + "00");
}

void SymbolTest::testDecimalSymbolInvalid_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("result");

    //  Check for ',' as decimal, and none present
    QTest::newRow("test 1") << "1234.56"
                            << "invalid";

    //  Detect ',' as decimal and replace from locale
    //  with thousands separator present
    QTest::newRow("test 2") << "987,654.32"
                            << "invalid";
}

void SymbolTest::testMonetaryLocaleDiffersFromNumericLocale()
{
    //  The numeric locale (QLocale) and the monetary locale used by
    //  MyMoneyMoney may differ, e.g. LC_NUMERIC=C but LC_MONETARY=ru_RU.UTF-8.
    //  In that case the string returned by possiblyReplaceSymbol() must still
    //  be parseable by MyMoneyMoney, otherwise the decimal separator is taken
    //  for a thousands separator and the value is off by a factor of 100.

    QFETCH(QString, monetaryDecimal);
    QFETCH(QString, monetaryThousands);
    QFETCH(int, decimalSymbol);
    QFETCH(QString, input);
    QFETCH(int, expectedNumerator);
    QFETCH(int, expectedDenominator);

    MyMoneyMoney::setDecimalSeparator(monetaryDecimal);
    MyMoneyMoney::setThousandSeparator(monetaryThousands);

    m_parse->setDecimalSymbol(static_cast<DecimalSymbol>(decimalSymbol));

    const auto converted = m_parse->possiblyReplaceSymbol(input);
    QCOMPARE(m_parse->invalidConversion(), false);
    QCOMPARE(MyMoneyMoney(converted).toString(), MyMoneyMoney(expectedNumerator, expectedDenominator).toString());
}

void SymbolTest::testMonetaryLocaleDiffersFromNumericLocale_data()
{
    QTest::addColumn<QString>("monetaryDecimal");
    QTest::addColumn<QString>("monetaryThousands");
    QTest::addColumn<int>("decimalSymbol");
    QTest::addColumn<QString>("input");
    QTest::addColumn<int>("expectedNumerator");
    QTest::addColumn<int>("expectedDenominator");

    //  monetary locale uses ',' as decimal separator, CSV file uses '.'
    QTest::newRow("dot in comma monetary locale") << QStringLiteral(",") << QStringLiteral(".") << static_cast<int>(DecimalSymbol::Dot)
                                                  << QStringLiteral("12.34") << 1234 << 100;
    QTest::newRow("dot with thousands in comma monetary locale")
        << QStringLiteral(",") << QStringLiteral(".") << static_cast<int>(DecimalSymbol::Dot) << QStringLiteral("1,234.56") << 123456 << 100;
    QTest::newRow("integer in comma monetary locale") << QStringLiteral(",") << QStringLiteral(".") << static_cast<int>(DecimalSymbol::Dot)
                                                      << QStringLiteral("145") << 145 << 1;

    //  monetary locale uses '.' as decimal separator, CSV file uses ','
    QTest::newRow("comma in dot monetary locale") << QStringLiteral(".") << QStringLiteral(",") << static_cast<int>(DecimalSymbol::Comma)
                                                  << QStringLiteral("12,34") << 1234 << 100;
    QTest::newRow("comma with thousands in dot monetary locale")
        << QStringLiteral(".") << QStringLiteral(",") << static_cast<int>(DecimalSymbol::Comma) << QStringLiteral("1.234,56") << 123456 << 100;
    QTest::newRow("integer in dot monetary locale") << QStringLiteral(".") << QStringLiteral(",") << static_cast<int>(DecimalSymbol::Comma)
                                                    << QStringLiteral("145") << 145 << 1;
}

void SymbolTest::cleanupTestCase()
{
}

void SymbolTest::testConstructor()
{
}

void SymbolTest::testConstructor_data()
{
}

void SymbolTest::testDefaultConstructor()
{
}

void SymbolTest::testDefaultConstructor_data()
{
}

void SymbolTest::initTestCase()
{
}

void SymbolTest::initTestCase_data()
{
}
