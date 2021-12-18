/*
    SPDX-FileCopyrightText: 2003 Michael Edwardes <mte@users.sourceforge.net>
    SPDX-FileCopyrightText: 2006 Ace Jones <acejones@users.sourceforge.net>
    SPDX-FileCopyrightText: 2021 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MYMONEYSCHEDULETEST_H
#define MYMONEYSCHEDULETEST_H

#include <QObject>

class MyMoneyScheduleTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testEmptyConstructor();
    void testConstructor();
    void testSetFunctions();
    void testCopyConstructor();
    void testAssignmentConstructor();
    void testAddHalfMonths();
    void testAdjustedNextDueDate();
    void testAdjustedNextPaymentOnLastDayInMonth();
    void testModifyNextDueDate();
    void testDaysBetweenEvents();
    void testEventsPerYear();
    void testOccurrenceToString();
    void testOccurrencePeriodToString();
    void testOccurrencePeriod();
    void testSimpleToFromCompoundOccurrence();
    void testProcessingDates();
    void testAdjustedNextPayment();
    void testAdjustedWhenItWillEnd();
    void testProcessLastDayInMonth();
    void testFixDate();
};

#endif
