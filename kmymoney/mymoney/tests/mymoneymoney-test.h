/*
 * Copyright 2002-2011  Thomas Baumgart <tbaumgart@kde.org>
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

#ifndef MYMONEYMONEYTEST_H
#define MYMONEYMONEYTEST_H

#include <QObject>

class MyMoneyMoney;

class MyMoneyMoneyTest : public QObject
{
  Q_OBJECT

protected:
  MyMoneyMoney *m_0, *m_1, *m_2, *m_3, *m_4, *m_5, *m_6;
private Q_SLOTS:
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
