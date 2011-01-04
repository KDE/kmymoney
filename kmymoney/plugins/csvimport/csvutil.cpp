/***************************************************************************
                                      csvutil.cpp
                                     -------------
    begin                        : Sat Jan 01 2010
    copyright                : (C) 2010 by Allan Anderson
    email                        : aganderson@ukonline.co.uk
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
#include "investprocessing.h"

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QDebug>

ParseLine::ParseLine(): m_fieldDelimiterIndex(0)
{
  m_delimCharList << "," << ";" << ":" << "\t";
  m_fieldDelimiterCharacter = m_delimCharList[m_fieldDelimiterIndex];
  m_textDelimiterCharacter = '"';
}

ParseLine::~ParseLine()
{
}

QStringList ParseLine::parseLine(const QString& data)
{
  QStringList listIn;
  QStringList listOut;
  QString txt;
  QString txt1;

  m_inBuffer = data;
  m_fieldDelimiterCharacter = m_delimCharList[m_fieldDelimiterIndex];
  listIn = m_inBuffer.split(m_fieldDelimiterCharacter);// firstly, split on m_fieldDelimiterCharacter

  QStringList::const_iterator constIterator;

  for (constIterator = listIn.constBegin(); constIterator < listIn.constEnd();
      ++constIterator) {
    txt = (*constIterator);
    // detect where a "quoted" string has been erroneously split, because of a comma,
    // or in a value, a 'thousand separator' being mistaken for a field delimitor.

    while((txt.startsWith(m_textDelimiterCharacter)) && (!txt.endsWith(m_textDelimiterCharacter)))  {
      if (++constIterator < listIn.constEnd())  {
        txt1 = (*constIterator);//                       second part of the split string
        txt += m_fieldDelimiterCharacter + txt1;//       rejoin the string
      } else break;
    }
    listOut += txt.remove(m_textDelimiterCharacter);
  }
  return listOut;
}

QString ParseLine::fieldDelimiterCharacter(int index)
{
  return m_delimCharList[index];
}

void ParseLine::setFieldDelimiterCharacter(int index)
{
  m_fieldDelimiterCharacter = m_delimCharList[index];
}

void ParseLine::setFieldDelimiterIndex(int index)
{
  m_fieldDelimiterIndex = index;
}

void ParseLine::setTextDelimiterCharacter(QString chr)
{
  m_textDelimiterCharacter = chr;
}

QString ParseLine::textDelimiterCharacter()
{
  return m_textDelimiterCharacter;
}
