/***************************************************************************
                               csvutil.h
                              -----------
begin                : Sat Jan 01 2010
copyright            : (C) 2010 by Allan Anderson
email                : aganderson@ukonline.co.uk
***************************************************************************/

/**************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef CSVUTIL_H
#define CSVUTIL_H

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QDate>
#include <QtCore/QStringList>
#include <QObject>


class ParseLine: public QObject
{
  Q_OBJECT

public:
  ParseLine();
  ~ParseLine();

  /**
   * This method is used to parse each line of data, splitting it into
   * separate fields, via the field delimiter character.  It also detects
   * where a string has been erroneously split because it contains one or
   * more 'thousand separators' which happen to be the same as the field
   * delimiter, and re-assembles the string.
   */
  QStringList      parseLine(const QString& data);

  QString          fieldDelimiterCharacter(int index);
  QString          textDelimiterCharacter();

  void             setFieldDelimiterIndex(int index);
  void             setFieldDelimiterCharacter(int index);

  void             setTextDelimiterCharacter(QString val);

private :

  QStringList      m_delimCharList;

  QString          m_fieldDelimiterCharacter;
  QString          m_textDelimiterCharacter;
  QString          m_inBuffer;

  int              m_fieldDelimiterIndex;
  int              m_textDelimiterIndex;
}
;
#endif
