/***************************************************************************
                           symboltest.cpp
                          -------------------
    begin                : Sat Jan 01 2010
    copyright            : (C) 2010 by Allan Anderson
    email                : agander93@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "symbol-test.h"
#include "../csvutil.h"

#include <QtTest>
#include <QString>
#include <QLocale>

QTEST_GUILESS_MAIN(SymbolTest);

Parse* m_parse;

SymbolTest::SymbolTest() :
  m_parse(nullptr)
{
}

void SymbolTest::init()
{
  m_parse = new Parse;
  m_parse->setDecimalSymbol(DecimalSymbol::Dot);
  m_localeDecimal = QLocale().decimalPoint();
  m_localeThousands = QLocale().groupSeparator();
}

void SymbolTest::cleanup()
{
  delete m_parse;
}

void SymbolTest::testDecimalSymbolDot()
{
  //  Detect '.' as decimal and replace from locale

  m_parse->setDecimalSymbol(DecimalSymbol::Dot);  //  "."

  QFETCH(QString, input);
  QFETCH(QString, result);

  QVERIFY(m_parse->possiblyReplaceSymbol(input) == result);
}

void SymbolTest::testDecimalSymbolComma()
{
  //  Detect ',' as decimal and replace from locale

  m_parse->setDecimalSymbol(DecimalSymbol::Comma);  //   ","

  QFETCH(QString, input);
  QFETCH(QString, result);

  QCOMPARE(m_parse->possiblyReplaceSymbol(input), result);
}

void SymbolTest::testDecimalSymbolInvalid()
{
  //  Check for ',' as decimal, and none present

  m_parse->setDecimalSymbol(DecimalSymbol::Comma);  //   ","
  m_testDecimal = m_parse->decimalSymbol(DecimalSymbol::Comma);

  QFETCH(QString, input);
  QFETCH(QString, result);
  QString res = m_parse->possiblyReplaceSymbol(input);

  QVERIFY(m_parse->invalidConversion() == true);
}

void SymbolTest::testDecimalSymbolDot_data()
{
  QTest::addColumn<QString> ("input");
  QTest::addColumn<QString> ("result");

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
  QTest::addColumn<QString> ("input");
  QTest::addColumn<QString> ("result");

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
  QTest::addColumn<QString> ("input");
  QTest::addColumn<QString> ("result");

  //  Check for ',' as decimal, and none present
  QTest::newRow("test 1") << "1234.56" << "invalid";

  //  Detect ',' as decimal and replace from locale
  //  with thousands separator present
  QTest::newRow("test 2") << "987,654.32" << "invalid";
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
