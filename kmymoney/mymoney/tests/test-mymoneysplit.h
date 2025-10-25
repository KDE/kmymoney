/*
    SPDX-FileCopyrightText: 2002-2016 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2004-2006 Ace Jones <acejones@users.sourceforge.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MYMONEYSPLITTEST_H
#define MYMONEYSPLITTEST_H

#include <QObject>

#include "mymoneytestutils.h"

class MyMoneySplit;

class MyMoneySplitTest : public QObject, public MyMoneyTestBase
{
    Q_OBJECT

protected:
    MyMoneySplit* m;

private Q_SLOTS:
    void init();
    void cleanup();
    void testEmptyConstructor();
    void testSetFunctions();
    void testCopyConstructor();
    void testAssignmentConstructor();
    void testEquality();
    void testInequality();
    void testAmortization();
    void testValue();
    void testSetValue();
    void testSetAction();
    void testIsAutoCalc();
    void testUnaryMinus();
    void testPrice();
    void testSetPrice();
    void testPriceCalculation();
    void testPriceAssignment();
};

#endif
