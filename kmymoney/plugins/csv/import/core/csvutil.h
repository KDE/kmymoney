/*
 * Copyright 2010-2015  Allan Anderson <agander93@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CSVUTIL_H
#define CSVUTIL_H

#include <QVector>
#include "csvenums.h"

#include "csv/import/core/kmm_csvimportercore_export.h"

class KMM_CSVIMPORTERCORE_EXPORT Parse
{
public:
  Parse();
  ~Parse();

  /**
   * This method is used to parse each line of data, splitting it into
   * separate fields, via the field delimiter character.  It also detects
   * where a string has been erroneously split because it contains one or
   * more 'thousand separators' which happen to be the same as the field
   * delimiter, and re-assembles the string.
   */
  QStringList      parseLine(const QString &data);
  QStringList      parseFile(const QString &buf);

  QChar decimalSymbol(const DecimalSymbol _d);

  /**
   * Check for presence of the selected decimal symbol
   * and evaluate if the proposed conversion is valid.
   * If so, change the symbol.
   */
  QString          possiblyReplaceSymbol(const QString&  str);

  void             setFieldDelimiter(const FieldDelimiter _d);

  void             setTextDelimiter(const TextDelimiter _d);

  void             setDecimalSymbol(const DecimalSymbol _d);

  bool             invalidConversion();

  int              lastLine();

private :

  QVector<QChar> m_fieldDelimiters;
  QVector<QChar> m_textDelimiters;
  QVector<QChar> m_decimalSymbols;
  QVector<QChar> m_thousandsSeparators;

  QChar          m_fieldDelimiter;
  QChar          m_textDelimiter;
  QChar          m_decimalSymbol;
  QChar          m_thousandsSeparator;

  int              m_lastLine;

  bool             m_symbolFound;
  bool             m_invalidConversion;
};

#endif
