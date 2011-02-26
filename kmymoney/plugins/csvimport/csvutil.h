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

class CsvImporterDlg;

class Parse: public QObject
{
  Q_OBJECT

public:
  Parse();
  ~Parse();

  CsvImporterDlg*     m_csvDialog;

  /**
   * This method is used to parse each line of data, splitting it into
   * separate fields, via the field delimiter character.  It also detects
   * where a string has been erroneously split because it contains one or
   * more 'thousand separators' which happen to be the same as the field
   * delimiter, and re-assembles the string.
   */
  QStringList      parseLine(const QString& data);

  QStringList      parseFile(const QString& buf, int strt, int end);

  QString          fieldDelimiterCharacter(int index);
  QString          decimalSymbol(int index);
  QString          textDelimiterCharacter(int index);
  void             thousandsSeparatorChanged(int index);
  QString          thousandsSeparator();

  void             setFieldDelimiterIndex(int index);
  void             setFieldDelimiterCharacter(int index);

  void             setTextDelimiterIndex(int index);
  void             setTextDelimiterCharacter(int index);

  void             setDecimalSymbolIndex(int index);
  void             setDecimalSymbol(int index);

  void             setThousandsSeparatorIndex(int index);
  void             setThousandsSeparator(int index);

  int              lastLine();

public slots:

  void             decimalSymbolSelected(int index);

private :

  QStringList      m_decimalSymbolList;
  QStringList      m_fieldDelimiterCharList;
  QStringList      m_textDelimiterCharList;
  QStringList      m_thousandsSeparatorList;

  QString          m_decimalSymbol;
  QString          m_fieldDelimiterCharacter;
  QString          m_textDelimiterCharacter;
  QString          m_thousandsSeparator;
  QString          m_inBuffer;

  int              m_decimalSymbolIndex;
  int              m_fieldDelimiterIndex;
  int              m_lastLine;
  int              m_textDelimiterIndex;
  int              m_thousandsSeparatorIndex;
}
;
#endif
