/***************************************************************************
                           symboltest.h
                         ------------------
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
#ifndef SYMBOLTEST_H
#define SYMBOLTEST_H

#include "../csvutil.h"

#include <QObject>

class Parse;

class SymbolTest : public QObject
{
  Q_OBJECT

public:
  SymbolTest();

  Parse*           m_parse;

  QStringList      m_input;
  QStringList      m_expected;

private:
  QString          m_localeDecimal;
  QString          m_testDecimal;
  QString          m_localeThousands;

private slots:
  void init();
  void cleanup();
  void cleanupTestCase();
  void testDefaultConstructor();
  void testDefaultConstructor_data();
  void testConstructor();
  void testConstructor_data();
  void initTestCase();
  void initTestCase_data();

  /**
  * There are three sub-tests in this class, for checking various
  * data strings, using both the dot '.' and the comma ',' as
  * decimal symbols, plus a format that should produce an invalid
  * conversion.
  */
  void       testDecimalSymbolDot();
  void       testDecimalSymbolDot_data();
  void       testDecimalSymbolComma();
  void       testDecimalSymbolComma_data();
  void       testDecimalSymbolInvalid();
  void       testDecimalSymbolInvalid_data();

};
#endif
