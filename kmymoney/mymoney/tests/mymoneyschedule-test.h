/***************************************************************************
                          mymoneyscheduletest.h
                          -------------------
    copyright            : (C) 2002 by Michael Edwardes
    email                : mte@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MYMONEYSCHEDULETEST_H
#define MYMONEYSCHEDULETEST_H

#include <QtCore/QObject>

class MyMoneyScheduleTest : public QObject
{
  Q_OBJECT

private slots:
  void testEmptyConstructor();
  void testConstructor();
  void testSetFunctions();
  void testCopyConstructor();
  void testAssignmentConstructor();
  void testOverdue();
  void testNextPayment();
  void testAddHalfMonths();
  void testPaymentDates();
  void testWriteXML();
  void testReadXML();
  void testHasReferenceTo();
  void testAdjustedNextDueDate();
  void testModifyNextDueDate();
  void testDaysBetweenEvents();
  void testEventsPerYear();
  void testOccurrenceToString();
  void testOccurrencePeriodToString();
  void testOccurrencePeriod();
  void testSimpleToFromCompoundOccurrence();
  void testProcessingDates();
  void testPaidEarlyOneTime();
  void testAdjustedNextPayment();
  void testReplaceId();
  void testAdjustedWhenItWillEnd();
  void testProcessLastDayInMonth();
  void testNextPaymentOnLastDayOfMonth();
  void testElementNames();
  void testAttributeNames();
};

#endif
