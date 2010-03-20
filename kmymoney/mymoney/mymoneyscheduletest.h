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

#ifndef __MYMONEYSCHEDULETEST_H__
#define __MYMONEYSCHEDULETEST_H__

#include <cppunit/extensions/HelperMacros.h>

#define private public
#define protected public
#include "mymoneyscheduled.h"
#include "mymoneyfile.h"
#include "storage/mymoneyseqaccessmgr.h"
#undef private

class MyMoneyScheduleTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(MyMoneyScheduleTest);
  CPPUNIT_TEST(testEmptyConstructor);
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST(testSetFunctions);
  CPPUNIT_TEST(testCopyConstructor);
  CPPUNIT_TEST(testAssignmentConstructor);
  // The following tests must be done in this order.
  CPPUNIT_TEST(testSingleton);
  CPPUNIT_TEST(testAddSchedule);
  CPPUNIT_TEST(testAnyScheduled);
  CPPUNIT_TEST(testOverdue);
  CPPUNIT_TEST(testGetSchedule);
  CPPUNIT_TEST(testGetScheduled);
  CPPUNIT_TEST(testGetOverdue);
  CPPUNIT_TEST(testNextPayment);
  CPPUNIT_TEST(testPaymentDates);
  CPPUNIT_TEST(testReplaceSchedule);
  CPPUNIT_TEST(testRemoveSchedule);
  CPPUNIT_TEST(testWriteXML);
  CPPUNIT_TEST(testReadXML);
  CPPUNIT_TEST(testHasReferenceTo);
  CPPUNIT_TEST(testAdjustedNextDueDate);
  CPPUNIT_TEST(testModifyNextDueDate);
  CPPUNIT_TEST(testDaysBetweenEvents);
  CPPUNIT_TEST(testEventsPerYear);
  CPPUNIT_TEST(testAddHalfMonths);
  CPPUNIT_TEST(testOccurrenceToString);
  CPPUNIT_TEST(testOccurrencePeriodToString);
  CPPUNIT_TEST(testStringToOccurrence);
  CPPUNIT_TEST(testOccurrencePeriod);
  CPPUNIT_TEST(testSimpleToFromCompoundOccurrence);
  CPPUNIT_TEST(testProcessingDates);
  CPPUNIT_TEST(testPaidEarlyOneTime);
  CPPUNIT_TEST_SUITE_END();

protected:
// MyMoneyFile *m_file;
// MyMoneySeqAccessMgr* storage;
  //TestObserverSet *observer;
  //TestObserverSet *hierarchyObserver;

public:
  MyMoneyScheduleTest();
  void setUp();
  void tearDown();
  void testEmptyConstructor();
  void testConstructor();
  void testSetFunctions();
  void testCopyConstructor();
  void testAssignmentConstructor();
  void testSingleton();
  void testAddSchedule();
  void testAnyScheduled();
  void testOverdue();
  void testGetSchedule();
  void testGetScheduled();
  void testGetOverdue();
  void testNextPayment();
  void testAddHalfMonths();
  void testPaymentDates();
  void testReplaceSchedule();
  void testRemoveSchedule();
  void testWriteXML();
  void testReadXML();
  void testHasReferenceTo();
  void testAdjustedNextDueDate();
  void testModifyNextDueDate();
  void testDaysBetweenEvents();
  void testEventsPerYear();
  void testOccurrenceToString();
  void testOccurrencePeriodToString();
  void testStringToOccurrence();
  void testOccurrencePeriod();
  void testSimpleToFromCompoundOccurrence();
  void testProcessingDates();
  void testPaidEarlyOneTime();
};

#endif
