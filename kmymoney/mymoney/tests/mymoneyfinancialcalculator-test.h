/*
    SPDX-FileCopyrightText: 2003-2008 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MYMONEYFINANCIALCALCULATORTEST_H
#define MYMONEYFINANCIALCALCULATORTEST_H

#include <QObject>

class MyMoneyFinancialCalculator;

class MyMoneyFinancialCalculatorTest : public QObject
{
    Q_OBJECT
protected:
    MyMoneyFinancialCalculator *m;

private Q_SLOTS:

    void init();
    void cleanup();
    void testEmptyConstructor();
    void testSetPrec();
    void testSetNpp();
    void testSetPF();
    void testSetCF();
    void testSetBep();
    void testSetDisc();
    void testSetIr();
    void testSetPv();
    void testSetPmt();
    void testSetFv();
    void testCombinedSet();
    void testNumPayments();
    void testUseCase1();
    void testUseCase2();
};

#endif
