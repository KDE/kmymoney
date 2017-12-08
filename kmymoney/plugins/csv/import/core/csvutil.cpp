/***************************************************************************
                                      csvutil.cpp
                                     -------------
    begin                    :      Sat Jan 01 2010
    copyright                : (C) 2010 by Allan Anderson
    email                    :    agander93@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "csvutil.h"
//#include <QStringList>
//#include <QVector>
#include <QRegularExpression>
#include <QLocale>

Parse::Parse() :
    m_lastLine(0),
    m_symbolFound(false),
    m_invalidConversion(false)
{
  m_fieldDelimiters = {QLatin1Char(','), QLatin1Char(';'), QLatin1Char(':'), QLatin1Char('\t')};
  m_textDelimiters = {QLatin1Char('"'), QLatin1Char('\'')};
  m_decimalSymbols = {QLatin1Char('.'), QLatin1Char(',')};
  m_thousandsSeparators = {QLatin1Char(','), QLatin1Char('.')};

  setFieldDelimiter(FieldDelimiter::Comma);
  setTextDelimiter(TextDelimiter::DoubleQuote);
  setDecimalSymbol(DecimalSymbol::Dot);
}

Parse::~Parse()
{
}

QStringList Parse::parseLine(const QString& data)
{
  QStringList listOut;
  const QStringList listIn = data.split(m_fieldDelimiter);  // firstly, split on m_fieldDelimiterCharacter
  QString cell;
  foreach (const auto it, listIn) {
    cell.append(it);
    // detect where a "quoted" string has been erroneously split, because of a comma,
    // or in a value, a 'thousand separator' being mistaken for a field delimiter.
    //Also, where a 'field separator' is within quotes and the quotes don't include the whole of the field.
    if (cell.startsWith(m_textDelimiter)) {
      if (!cell.endsWith(m_textDelimiter)) {
        cell.append(m_fieldDelimiter);
        continue;
      }
      cell.remove(m_textDelimiter);
    }
    listOut.append(cell);
    cell.clear();
  }
  return listOut;
}

QStringList Parse::parseFile(const QString &buf)
{
  int lineCount = 0;
  bool inQuotes = false;
  QString line;
  QStringList lines;

  foreach (const auto chr, buf) {
    if (chr == m_textDelimiter) {
      line += chr;
      inQuotes = !inQuotes;
      continue;
    } else if (chr == QLatin1Char('\r') || chr == QLatin1Char('\n')) {
      if (inQuotes) {
        line += QLatin1Char('~');
        continue;
      }
      if (line.isEmpty())
        continue;
      ++lineCount;
      lines += line;
      line.clear();
    } else {
      line += chr;
      continue;
    }
  }
  m_lastLine = lineCount;
  return lines;
}

void Parse::setFieldDelimiter(const FieldDelimiter _d)
{
  if (_d == FieldDelimiter::Auto)
    return;
  m_fieldDelimiter = m_fieldDelimiters.at((int)_d);
}

void Parse::setTextDelimiter(const TextDelimiter _d)
{
  m_textDelimiter = m_textDelimiters.at((int)_d);
}

void Parse::setDecimalSymbol(const DecimalSymbol _d)
{
  if (_d == DecimalSymbol::Auto)
    return;
  m_decimalSymbol = m_decimalSymbols.at((int)_d);
  if (_d == DecimalSymbol::Comma)
    m_thousandsSeparator = m_thousandsSeparators.at((int)ThousandSeparator::Dot);
  else
    m_thousandsSeparator = m_thousandsSeparators.at((int)ThousandSeparator::Comma);
}

QChar Parse::decimalSymbol(const DecimalSymbol _d)
{
  if (_d == DecimalSymbol::Auto)
    return QChar();
  return m_decimalSymbols.at((int)_d);
}

int Parse::lastLine()
{
  return m_lastLine;
}

QString Parse::possiblyReplaceSymbol(const QString&  str)
{
  // examples given if decimal symbol is '.' and thousand symbol is ','
  m_symbolFound = false;
  m_invalidConversion = true;

  QString txt = str.trimmed();
  if (txt.isEmpty())  // empty strings not allowed
    return txt;

  bool parentheses = false;
  if (txt.contains(QLatin1Char('('))) // (1.23) is in fact -1.23
    parentheses = true;

  int length = txt.length();
  int decimalIndex = txt.indexOf(m_decimalSymbol);
  int thouIndex = txt.lastIndexOf(m_thousandsSeparator);

  txt.remove(QRegularExpression(QStringLiteral("\\D.,-+")));     // remove all non-digits
  txt.remove(m_thousandsSeparator);

  if (txt.isEmpty())  // empty strings not allowed
    return txt;

  if (decimalIndex == -1) {                                         // e.g. 1 ; 1,234 ; 1,234,567; 12,
    if (thouIndex == -1 || thouIndex == length - 4)  {              // e.g. 1 ; 1,234 ; 1,234,567
      txt.append(QLocale().decimalPoint() + QLatin1String("00"));   // e.g. 1.00 ; 1234.00 ; 1234567.00
      m_invalidConversion = false;
    }
    return txt;
  }
  m_symbolFound = true;             // decimal symbol found

  if (decimalIndex < thouIndex)     // e.g. 1.234,567 ; 1.23,45
    return txt;

  m_invalidConversion = false;      // it cannot be true after this point
  txt.replace(m_decimalSymbol, QLocale().decimalPoint());  // so swap it

  if (decimalIndex == length - 1)   // e.g. 1. ; 123.
    txt.append(QLatin1String("00"));

  if (parentheses)
    txt.prepend(QLatin1Char('-'));

  return txt;
}

bool Parse::invalidConversion()
{
  return m_invalidConversion;
}

//--------------------------------------------------------------------------------------------------------------------------------
