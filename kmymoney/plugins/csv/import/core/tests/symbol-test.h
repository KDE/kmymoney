/*
 * SPDX-FileCopyrightText: 2011-2012 Allan Anderson <agander93@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

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

private Q_SLOTS:
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
