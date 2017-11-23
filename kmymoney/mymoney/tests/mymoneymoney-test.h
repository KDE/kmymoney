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

#include <QObject>

class MyMoneyMoney;

class MyMoneyMoneyTest : public QObject
{
  Q_OBJECT

protected:
  MyMoneyMoney *m_0, *m_1, *m_2, *m_3, *m_4, *m_5, *m_6;
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
