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

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QDebug>

#include <KGlobal>
#include <KLocale>

Parse::Parse(): m_fieldDelimiterIndex(0), m_textDelimiterIndex(0)
{
  m_fieldDelimiterCharList << "," << ";" << ":" << "\t";
  m_fieldDelimiterCharacter = m_fieldDelimiterCharList[m_fieldDelimiterIndex];
  m_textDelimiterCharList << "\"" << "'";
  m_textDelimiterCharacter = m_textDelimiterCharList[m_textDelimiterIndex];
  m_decimalSymbolList << "." << ",";
  m_thousandsSeparatorList << "," << ".";
  m_invalidConversion = false;
}

Parse::~Parse()
{
}

QStringList Parse::parseLine(const QString& data)
{
  QStringList listIn;
  QStringList listOut;
  QString txt;
  QString txt1;

  m_inBuffer = data;
  if (m_inBuffer.endsWith(',')) {
    m_inBuffer.chop(1);
  }

  m_fieldDelimiterCharacter = m_fieldDelimiterCharList[m_fieldDelimiterIndex];
  listIn = m_inBuffer.split(m_fieldDelimiterCharacter);// firstly, split on m_fieldDelimiterCharacter

  QStringList::const_iterator constIterator;

  for (constIterator = listIn.constBegin(); constIterator < listIn.constEnd();
       ++constIterator) {
    txt = (*constIterator);
    // detect where a "quoted" string has been erroneously split, because of a comma,
    // or in a value, a 'thousand separator' being mistaken for a field delimitor.

    while ((txt.startsWith(m_textDelimiterCharacter)) && (!txt.endsWith(m_textDelimiterCharacter)))  {
      if (++constIterator < listIn.constEnd())  {
        txt1 = (*constIterator);//                       second part of the split string
        txt += m_fieldDelimiterCharacter + txt1;//       rejoin the string
      } else break;
    }
    listOut += txt.remove(m_textDelimiterCharacter);
  }
  return listOut;
}

QStringList Parse::parseFile(const QString& buf, int strt, int end)
{
  QStringList outBuffer;
  outBuffer.clear();
  int lineCount = -1;
  QString tmpBuffer;
  tmpBuffer.clear();
  bool inQuotes = false;
  int count = buf.count();

  QString::const_iterator constIterator;

  for (constIterator = buf.constBegin(); constIterator != buf.constEnd();
       ++constIterator) {
    QString chr = (*constIterator);
    count -= 1;
    if (chr == m_textDelimiterCharacter) {
      tmpBuffer += chr;
      if (inQuotes == true) { //                if already in quoted field..
        inQuotes = false;//                    ..end it
      } else {//                               if not..
        inQuotes = true;//                     ..start it
      }
      continue;
    } else if (chr == "\n") {
      if (inQuotes == true) { //               embedded '\n' in quoted field
        chr = '~';//                           replace it with ~ for now
        tmpBuffer += chr;
        if (count > 0) //                       more chars yet
          continue;//                          more chars yet
      }
      //                                       true EOL (not in quotes)
      if (tmpBuffer.isEmpty()) {
        continue;
      }
      lineCount ++;
      if (lineCount < strt) { //   startLine      not yet reached first wanted line
        tmpBuffer.clear();
        continue;
      }
      outBuffer << tmpBuffer;
      tmpBuffer.clear();

      //                                       look for start of wanted data
      //  if first pass or if not at last line, proceed
      if ((!end == 0) && (lineCount >= end - 1)) { // m_endLine is set from UI after first pass
        m_lastLine = lineCount - 1;
        break;
      }
    }//                                        end of 'EOL detected' loop
    else {//                                   must be data char
      tmpBuffer += chr;
      if (count > 0) { //                       more chars yet
        continue;
      }//                                      else eoFile = true;
    }

    if (!tmpBuffer.isEmpty()) {
      outBuffer << tmpBuffer;
    }
  }
  m_lastLine = lineCount + 1;
  return outBuffer;
}

QString Parse::fieldDelimiterCharacter(int index)
{
  return m_fieldDelimiterCharList[index];
}

void Parse::setFieldDelimiterCharacter(int index)
{
  m_fieldDelimiterCharacter = m_fieldDelimiterCharList[index];
}

void Parse::setFieldDelimiterIndex(int index)
{
  m_fieldDelimiterIndex = index;
}

QString Parse::textDelimiterCharacter(int index)
{
  return m_textDelimiterCharList[index];
}

void Parse::setTextDelimiterCharacter(int index)
{
  m_textDelimiterCharacter = m_textDelimiterCharList[index];
}

void Parse::setTextDelimiterIndex(int index)
{
  m_textDelimiterIndex = index;
}

void Parse::decimalSymbolSelected(int val)
{
  if (val < 0) return;

  m_decimalSymbolIndex = val;
  m_decimalSymbol = m_decimalSymbolList[val];
  thousandsSeparatorChanged(val);
}

QString Parse::decimalSymbol(int index)
{
  return m_decimalSymbolList[index];
}

void Parse::setDecimalSymbol(int index)
{
  m_decimalSymbol = m_decimalSymbolList[index];
}

void Parse::setDecimalSymbolIndex(int index)
{
  m_decimalSymbolIndex = index;
}

void Parse::thousandsSeparatorChanged(int val)
{
  m_thousandsSeparatorIndex = val;
  m_thousandsSeparator = m_thousandsSeparatorList[val];
  if (m_thousandsSeparator == KGlobal::locale()->thousandsSeparator()) {
    return;
  }
}

QString Parse::thousandsSeparator()
{
  return m_thousandsSeparator;
}

void Parse::setThousandsSeparator(int index)
{
  m_thousandsSeparator = m_thousandsSeparatorList[index];
}

void Parse::setThousandsSeparatorIndex(int index)
{
  m_thousandsSeparatorIndex = index;
}

int Parse::lastLine()
{
  return m_lastLine;
}

bool Parse::symbolFound()
{
  return m_symbolFound;
}

void Parse::setSymbolFound(bool found)
{
  m_symbolFound = found;
}

QString Parse::possiblyReplaceSymbol(const QString&  str)
{
  m_symbolFound = false;
  m_invalidConversion = false;

  if (str.isEmpty()) return str;
  QString txt = str.trimmed();//                 don't want trailing blanks
  if (txt.contains('(')) {//              "(" or "Af" = debit
    txt = txt.remove(QRegExp("[()]"));
    txt = '-' + txt;
  }
  int decimalIndex = txt.indexOf(m_decimalSymbol, 0);
  int length = txt.length();
  int thouIndex = txt.lastIndexOf(m_thousandsSeparator, -1);

  //  Check if this col/cell contains decimal symbol

  if (decimalIndex == -1) {//                     there is no decimal
    m_symbolFound = false;
    if ((thouIndex == -1) || (thouIndex == length - 4))  { //no separator || correct format
      txt.remove(m_thousandsSeparator);
      QString tmp = txt + KGlobal::locale()->decimalSymbol() + "00";
      return tmp;
    } else
      m_invalidConversion = true;
    return txt;
  }

  txt.remove(m_thousandsSeparator);//    remove unwanted old thousands separator
  //  Found decimal

  m_symbolFound = true;//                        found genuine decimal

  if (thouIndex >= 0) { //                        there was a separator
    if (decimalIndex < thouIndex) { //            invalid conversion
      m_invalidConversion = true;
    }
    if (length == decimalIndex + 4) { //          ...thousands separator with no decimal part
      txt += m_decimalSymbol + "00";
    }
  }//  thouIndex = -1                            no thousands separator

  //  m_symbolFound = true                      found genuine decimal

  txt.replace(m_decimalSymbol, KGlobal::locale()->decimalSymbol());// so swap it
  return txt;
}

bool Parse::invalidConversion()
{
  return m_invalidConversion;
}
