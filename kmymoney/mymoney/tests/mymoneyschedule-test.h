/*
 * Copyright 2003       Michael Edwardes <mte@users.sourceforge.net>
 * Copyright 2006       Ace Jones <acejones@users.sourceforge.net>
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
