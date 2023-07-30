/*
    SPDX-FileCopyrightText: 2002-2011 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
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
    void testMaxPrecision();
    void testZeroIsMinusZero();
};

#endif
