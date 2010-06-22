/***************************************************************************
                          mymoneymoneytest.h
                          -------------------
    copyright            : (C) 2002 by Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MYMONEYMONEYTEST_H
#define MYMONEYMONEYTEST_H

// Check for standard definitions
#ifdef HAVE_STDINT_H
#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS         // force definition of min and max values
#endif
#include <stdint.h>
#else
#include <limits.h>
#define INT64_MAX LLONG_MAX
#define INT64_MIN LLONG_MIN
#endif

#include <QtCore/QObject>

#define private public
#include "mymoneymoney.h"
#undef private

class MyMoneyMoneyTest : public QObject
{
  Q_OBJECT

protected:
  MyMoneyMoney *m_0, *m_1, *m_2, *m_3, *m_4, *m_5;
private slots:
  void init();
  void cleanup();
  void testEmptyConstructor();
  void testIntConstructor();
  void testStringConstructor();
  void testAssignment();
  void testConvert();
  void testEquality();
  void testInequality();
  void testAddition();
  void testSubtraction();
  void testMultiplication();
  void testDivision();
  void testFormatMoney();
  void testSetDecimalSeparator();
  void testSetThousandSeparator();
  void testRelation();
  void testUnaryMinus();
  void testDoubleConstructor();
  void testAbsoluteFunction();
  void testToString();
  void testNegativeSignPos();
  void testPositiveSignPos();
  void testNegativeStringConstructor();
  void testReduce();
  void testZeroDenominator();
};

#endif
