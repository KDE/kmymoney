/***************************************************************************
                          mymoneyscheduletest.cpp
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

#include "mymoneyscheduletest.h"

#include <QList>
#include <QtTest/QtTest>

// Include internationalization
#include <klocale.h>
#include <KHolidays/Holidays>

#define KMM_MYMONEY_UNIT_TESTABLE friend class MyMoneyScheduleTest;

#include "mymoneysplit.h"
#include "mymoneymoney.h"
#include "mymoneyscheduled.h"
#include "mymoneyfile.h"
#include "storage/mymoneyseqaccessmgr.h"

QTEST_MAIN(MyMoneyScheduleTest)

void MyMoneyScheduleTest::testEmptyConstructor()
{
  MyMoneySchedule s;

  QVERIFY(s.id().isEmpty());
  QVERIFY(s.m_occurrence == MyMoneySchedule::OCCUR_ANY);
  QVERIFY(s.m_type == MyMoneySchedule::TYPE_ANY);
  QVERIFY(s.m_paymentType == MyMoneySchedule::STYPE_ANY);
  QVERIFY(s.m_fixed == false);
  QVERIFY(!s.m_startDate.isValid());
  QVERIFY(!s.m_endDate.isValid());
  QVERIFY(!s.m_lastPayment.isValid());
  QVERIFY(s.m_autoEnter == false);
  QVERIFY(s.m_name.isEmpty());
  QVERIFY(s.willEnd() == false);
}

void MyMoneyScheduleTest::testConstructor()
{
  MyMoneySchedule s("A Name",
                    MyMoneySchedule::TYPE_BILL,
                    MyMoneySchedule::OCCUR_WEEKLY, 1,
                    MyMoneySchedule::STYPE_DIRECTDEBIT,
                    QDate::currentDate(),
                    QDate(),
                    true,
                    true);

  QVERIFY(s.type() == MyMoneySchedule::TYPE_BILL);
  QVERIFY(s.occurrence() == MyMoneySchedule::OCCUR_WEEKLY);
  QVERIFY(s.occurrenceMultiplier() == 1);
  QVERIFY(s.paymentType() == MyMoneySchedule::STYPE_DIRECTDEBIT);
  QVERIFY(s.startDate() == QDate());
  QVERIFY(s.willEnd() == false);
  QVERIFY(s.isFixed() == true);
  QVERIFY(s.autoEnter() == true);
  QVERIFY(s.name() == "A Name");
  QVERIFY(!s.m_endDate.isValid());
  QVERIFY(!s.m_lastPayment.isValid());
}

void MyMoneyScheduleTest::testSetFunctions()
{
  MyMoneySchedule s;

  s.setId("SCHED001");
  QVERIFY(s.id() == "SCHED001");

  s.setType(MyMoneySchedule::TYPE_BILL);
  QVERIFY(s.type() == MyMoneySchedule::TYPE_BILL);

  s.setEndDate(QDate::currentDate());
  QVERIFY(s.endDate() == QDate::currentDate());
  QVERIFY(s.willEnd() == true);
}

void MyMoneyScheduleTest::testCopyConstructor()
{
  MyMoneySchedule s;

  s.setId("SCHED001");
  s.setType(MyMoneySchedule::TYPE_BILL);

  MyMoneySchedule s2(s);

  QVERIFY(s.id() == s2.id());
  QVERIFY(s.type() == s2.type());
}

void MyMoneyScheduleTest::testAssignmentConstructor()
{
  MyMoneySchedule s;

  s.setId("SCHED001");
  s.setType(MyMoneySchedule::TYPE_BILL);

  MyMoneySchedule s2 = s;

  QVERIFY(s.id() == s2.id());
  QVERIFY(s.type() == s2.type());
}

void MyMoneyScheduleTest::testOverdue()
{
  MyMoneySchedule sch_overdue;
  MyMoneySchedule sch_intime;

  // the following checks only work correctly, if currentDate() is
  // between the 1st and 27th. If it is between 28th and 31st
  // we don't perform them. Note: this should be fixed.
  if (QDate::currentDate().day() > 27 || QDate::currentDate().day() == 1) {
    qDebug() << "testOverdue() skipped because current day is between 28th and 2nd";
    return;
  }

  QDate startDate = QDate::currentDate().addDays(-1).addMonths(-23);
  QDate lastPaymentDate = QDate::currentDate().addDays(-1).addMonths(-1);

  QString ref = QString(
                  "<!DOCTYPE TEST>\n"
                  "<SCHEDULE-CONTAINER>\n"
                  " <SCHEDULED_TX startDate=\"%1\" autoEnter=\"0\" weekendOption=\"2\" lastPayment=\"%2\" paymentType=\"8\" endDate=\"\" type=\"5\" id=\"SCH0002\" name=\"A Name\" fixed=\"0\" occurenceMultiplier=\"1\" occurence=\"32\" >\n" // krazy:exclude=spelling
                  "  <PAYMENTS>\n"
                  "   <PAYMENT date=\"%3\" />\n"
                  "  </PAYMENTS>\n"
                  "  <TRANSACTION postdate=\"\" memo=\"Wohnung:Miete\" id=\"\" commodity=\"EUR\" entrydate=\"\" >\n"
                  "   <SPLITS>\n"
                  "    <SPLIT payee=\"P000001\" reconciledate=\"\" shares=\"96379/100\" action=\"\" number=\"\" reconcileflag=\"2\" memo=\"\" value=\"96379/100\" account=\"A000076\" />\n"
                  "    <SPLIT payee=\"P000001\" reconciledate=\"\" shares=\"-96379/100\" action=\"\" number=\"\" reconcileflag=\"1\" memo=\"\" value=\"-96379/100\" account=\"A000276\" />\n"
                  "   </SPLITS>\n"
                  "   <KEYVALUEPAIRS>\n"
                  "    <PAIR key=\"key\" value=\"value\" />\n"
                  "   </KEYVALUEPAIRS>\n"
                  "  </TRANSACTION>\n"
                  " </SCHEDULED_TX>\n"
                  "</SCHEDULE-CONTAINER>\n");
  QString ref_overdue = ref.arg(startDate.toString(Qt::ISODate))
                        .arg(lastPaymentDate.toString(Qt::ISODate))
                        .arg(lastPaymentDate.toString(Qt::ISODate));

  QString ref_intime = ref.arg(startDate.addDays(1).toString(Qt::ISODate))
                       .arg(lastPaymentDate.addDays(1).toString(Qt::ISODate))
                       .arg(lastPaymentDate.addDays(1).toString(Qt::ISODate));

  QDomDocument doc;
  QDomElement node;

  // std::cout << ref_intime << std::endl;
  try {
    doc.setContent(ref_overdue);
    node = doc.documentElement().firstChild().toElement();
    sch_overdue = MyMoneySchedule(node);
    doc.setContent(ref_intime);
    node = doc.documentElement().firstChild().toElement();
    sch_intime = MyMoneySchedule(node);

    QVERIFY(sch_overdue.isOverdue() == true);
    QVERIFY(sch_intime.isOverdue() == false);

  } catch (MyMoneyException *e) {
    delete e;
    QFAIL("Unexpected exception");
  }
}

void MyMoneyScheduleTest::testNextPayment()
/*
 * Test for a schedule where a payment hasn't yet been made.
 * First payment is in the future.
*/
{
  MyMoneySchedule sch;
  QString future_sched = QString(
                           "<!DOCTYPE TEST>\n"
                           "<SCHEDULE-CONTAINER>\n"
                           "<SCHEDULED_TX startDate=\"2007-02-17\" autoEnter=\"1\" weekendOption=\"2\" lastPayment=\"\" paymentType=\"1\" endDate=\"\" type=\"1\" id=\"SCH000058\" name=\"Car Tax\" fixed=\"1\" occurenceMultiplier=\"1\" occurence=\"16384\" >\n" // krazy:exclude=spelling
                           "  <PAYMENTS/>\n"
                           "  <TRANSACTION postdate=\"\" memo=\"\" id=\"\" commodity=\"GBP\" entrydate=\"\" >\n"
                           "  <SPLITS>\n"
                           "    <SPLIT payee=\"P000044\" reconciledate=\"\" shares=\"-15000/100\" action=\"Withdrawal\" bankid=\"\" number=\"\" reconcileflag=\"0\" memo=\"\" value=\"-15000/100\" account=\"A000155\" />\n"
                           "    <SPLIT payee=\"\" reconciledate=\"\" shares=\"15000/100\" action=\"Withdrawal\" bankid=\"\" number=\"\" reconcileflag=\"0\" memo=\"\" value=\"15000/100\" account=\"A000182\" />\n"
                           "  </SPLITS>\n"
                           "  <KEYVALUEPAIRS/>\n"
                           "  </TRANSACTION>\n"
                           "</SCHEDULED_TX>\n"
                           "</SCHEDULE-CONTAINER>\n"
                         );

  QDomDocument doc;
  QDomElement node;
  doc.setContent(future_sched);
  node = doc.documentElement().firstChild().toElement();

  try {
    sch = MyMoneySchedule(node);
    QVERIFY(sch.nextPayment(QDate(2007, 2, 14)) == QDate(2007, 2, 17));
    QVERIFY(sch.nextPayment(QDate(2007, 2, 17)) == QDate(2008, 2, 17));
    QVERIFY(sch.nextPayment(QDate(2007, 2, 18)) == QDate(2008, 2, 17));

  } catch (MyMoneyException *e) {
    delete e;
    QFAIL("Unexpected exception");
  }
}

void MyMoneyScheduleTest::testAddHalfMonths()
{
  // addHalfMonths is private
  // Test a Schedule with occurrence OCCUR_EVERYHALFMONTH using nextPayment
  MyMoneySchedule s;
  s.setStartDate(QDate(2007, 1, 1));
  s.setOccurrence(MyMoneySchedule::OCCUR_EVERYHALFMONTH);
  s.setNextDueDate(s.startDate());
  s.setLastPayment(s.startDate());

  QString format("yyyy-MM-dd");
  QVERIFY(s.nextPayment(s.nextDueDate()).toString(format) == "2007-01-16");
  s.setNextDueDate(QDate(2007, 1, 2));
  QVERIFY(s.nextPayment(s.nextDueDate()).toString(format) == "2007-01-17");
  s.setNextDueDate(QDate(2007, 1, 3));
  QVERIFY(s.nextPayment(s.nextDueDate()).toString(format) == "2007-01-18");
  s.setNextDueDate(QDate(2007, 1, 4));
  QVERIFY(s.nextPayment(s.nextDueDate()).toString(format) == "2007-01-19");
  s.setNextDueDate(QDate(2007, 1, 5));
  QVERIFY(s.nextPayment(s.nextDueDate()).toString(format) == "2007-01-20");
  s.setNextDueDate(QDate(2007, 1, 6));
  QVERIFY(s.nextPayment(s.nextDueDate()).toString(format) == "2007-01-21");
  s.setNextDueDate(QDate(2007, 1, 7));
  QVERIFY(s.nextPayment(s.nextDueDate()).toString(format) == "2007-01-22");
  s.setNextDueDate(QDate(2007, 1, 8));
  QVERIFY(s.nextPayment(s.nextDueDate()).toString(format) == "2007-01-23");
  s.setNextDueDate(QDate(2007, 1, 9));
  QVERIFY(s.nextPayment(s.nextDueDate()).toString(format) == "2007-01-24");
  s.setNextDueDate(QDate(2007, 1, 10));
  QVERIFY(s.nextPayment(s.nextDueDate()).toString(format) == "2007-01-25");
  s.setNextDueDate(QDate(2007, 1, 11));
  QVERIFY(s.nextPayment(s.nextDueDate()).toString(format) == "2007-01-26");
  s.setNextDueDate(QDate(2007, 1, 12));
  QVERIFY(s.nextPayment(s.nextDueDate()).toString(format) == "2007-01-27");
  s.setNextDueDate(QDate(2007, 1, 13));
  QVERIFY(s.nextPayment(s.nextDueDate()).toString(format) == "2007-01-28");
  s.setNextDueDate(QDate(2007, 1, 14));
  QVERIFY(s.nextPayment(s.nextDueDate()).toString(format) == "2007-01-29");
  // 15 -> Last Day
  s.setNextDueDate(QDate(2007, 1, 15));
  QVERIFY(s.nextPayment(s.nextDueDate()).toString(format) == "2007-01-31");
  s.setNextDueDate(QDate(2007, 1, 16));
  QVERIFY(s.nextPayment(s.nextDueDate()).toString(format) == "2007-02-01");
  s.setNextDueDate(QDate(2007, 1, 17));
  QVERIFY(s.nextPayment(s.nextDueDate()).toString(format) == "2007-02-02");
  s.setNextDueDate(QDate(2007, 1, 18));
  QVERIFY(s.nextPayment(s.nextDueDate()).toString(format) == "2007-02-03");
  s.setNextDueDate(QDate(2007, 1, 19));
  QVERIFY(s.nextPayment(s.nextDueDate()).toString(format) == "2007-02-04");
  s.setNextDueDate(QDate(2007, 1, 20));
  QVERIFY(s.nextPayment(s.nextDueDate()).toString(format) == "2007-02-05");
  s.setNextDueDate(QDate(2007, 1, 21));
  QVERIFY(s.nextPayment(s.nextDueDate()).toString(format) == "2007-02-06");
  s.setNextDueDate(QDate(2007, 1, 22));
  QVERIFY(s.nextPayment(s.nextDueDate()).toString(format) == "2007-02-07");
  s.setNextDueDate(QDate(2007, 1, 23));
  QVERIFY(s.nextPayment(s.nextDueDate()).toString(format) == "2007-02-08");
  s.setNextDueDate(QDate(2007, 1, 24));
  QVERIFY(s.nextPayment(s.nextDueDate()).toString(format) == "2007-02-09");
  s.setNextDueDate(QDate(2007, 1, 25));
  QVERIFY(s.nextPayment(s.nextDueDate()).toString(format) == "2007-02-10");
  s.setNextDueDate(QDate(2007, 1, 26));
  QVERIFY(s.nextPayment(s.nextDueDate()).toString(format) == "2007-02-11");
  s.setNextDueDate(QDate(2007, 1, 27));
  QVERIFY(s.nextPayment(s.nextDueDate()).toString(format) == "2007-02-12");
  s.setNextDueDate(QDate(2007, 1, 28));
  QVERIFY(s.nextPayment(s.nextDueDate()).toString(format) == "2007-02-13");
  s.setNextDueDate(QDate(2007, 1, 29));
  QVERIFY(s.nextPayment(s.nextDueDate()).toString(format) == "2007-02-14");
  // 30th,31st -> 15th
  s.setNextDueDate(QDate(2007, 1, 30));
  QVERIFY(s.nextPayment(s.nextDueDate()).toString(format) == "2007-02-15");
  s.setNextDueDate(QDate(2007, 1, 31));
  QVERIFY(s.nextPayment(s.nextDueDate()).toString(format) == "2007-02-15");
  // 30th (last day)
  s.setNextDueDate(QDate(2007, 4, 30));
  QVERIFY(s.nextPayment(s.nextDueDate()).toString(format) == "2007-05-15");
  // 28th of February (Last day): to 15th
  s.setNextDueDate(QDate(1900, 2, 28));
  QVERIFY(s.nextPayment(s.nextDueDate()).toString(format) == "1900-03-15");
  // 28th of February (Leap year): to 13th
  s.setNextDueDate(QDate(2000, 2, 28));
  QVERIFY(s.nextPayment(s.nextDueDate()).toString(format) == "2000-03-13");
  // 29th of February (Leap year)
  s.setNextDueDate(QDate(2000, 2, 29));
  QVERIFY(s.nextPayment(s.nextDueDate()).toString(format) == "2000-03-15");
  // Add multiple transactions
  s.setStartDate(QDate(2007, 1, 1));
  QVERIFY(s.dateAfter(2).toString(format) == "2007-01-16");
  QVERIFY(s.dateAfter(3).toString(format) == "2007-02-01");
  QVERIFY(s.dateAfter(4).toString(format) == "2007-02-16");
  s.setStartDate(QDate(2007, 1, 12));
  QVERIFY(s.dateAfter(2).toString(format) == "2007-01-27");
  QVERIFY(s.dateAfter(3).toString(format) == "2007-02-12");
  QVERIFY(s.dateAfter(4).toString(format) == "2007-02-27");
  QVERIFY(s.dateAfter(5).toString(format) == "2007-03-12");
  s.setStartDate(QDate(2007, 1, 13));
  QVERIFY(s.dateAfter(2).toString(format) == "2007-01-28");
  QVERIFY(s.dateAfter(3).toString(format) == "2007-02-13");
  QVERIFY(s.dateAfter(4).toString(format) == "2007-02-28");
  QVERIFY(s.dateAfter(5).toString(format) == "2007-03-15");
  s.setStartDate(QDate(2007, 1, 14));
  QVERIFY(s.dateAfter(2).toString(format) == "2007-01-29");
  QVERIFY(s.dateAfter(3).toString(format) == "2007-02-14");
  QVERIFY(s.dateAfter(4).toString(format) == "2007-02-28");
  QVERIFY(s.dateAfter(5).toString(format) == "2007-03-15");
  s.setStartDate(QDate(2007, 1, 15));
  QVERIFY(s.dateAfter(2).toString(format) == "2007-01-31");
  QVERIFY(s.dateAfter(3).toString(format) == "2007-02-15");
  QVERIFY(s.dateAfter(4).toString(format) == "2007-02-28");
  QVERIFY(s.dateAfter(5).toString(format) == "2007-03-15");
  s.setStartDate(QDate(2007, 1, 16));
  QVERIFY(s.dateAfter(2).toString(format) == "2007-02-01");
  QVERIFY(s.dateAfter(3).toString(format) == "2007-02-16");
  QVERIFY(s.dateAfter(4).toString(format) == "2007-03-01");
  QVERIFY(s.dateAfter(5).toString(format) == "2007-03-16");
  s.setStartDate(QDate(2007, 1, 27));
  QVERIFY(s.dateAfter(2).toString(format) == "2007-02-12");
  QVERIFY(s.dateAfter(3).toString(format) == "2007-02-27");
  QVERIFY(s.dateAfter(4).toString(format) == "2007-03-12");
  QVERIFY(s.dateAfter(5).toString(format) == "2007-03-27");
  s.setStartDate(QDate(2007, 1, 28));
  QVERIFY(s.dateAfter(2).toString(format) == "2007-02-13");
  QVERIFY(s.dateAfter(3).toString(format) == "2007-02-28");
  QVERIFY(s.dateAfter(4).toString(format) == "2007-03-15");
  QVERIFY(s.dateAfter(5).toString(format) == "2007-03-31");
  s.setStartDate(QDate(2007, 1, 29));
  QVERIFY(s.dateAfter(2).toString(format) == "2007-02-14");
  QVERIFY(s.dateAfter(3).toString(format) == "2007-02-28");
  QVERIFY(s.dateAfter(4).toString(format) == "2007-03-15");
  QVERIFY(s.dateAfter(5).toString(format) == "2007-03-31");
  s.setStartDate(QDate(2007, 1, 30));
  QVERIFY(s.dateAfter(2).toString(format) == "2007-02-15");
  QVERIFY(s.dateAfter(3).toString(format) == "2007-02-28");
  QVERIFY(s.dateAfter(4).toString(format) == "2007-03-15");
  QVERIFY(s.dateAfter(5).toString(format) == "2007-03-31");
  s.setStartDate(QDate(2007, 1, 31));
  QVERIFY(s.dateAfter(2).toString(format) == "2007-02-15");
  QVERIFY(s.dateAfter(3).toString(format) == "2007-02-28");
  QVERIFY(s.dateAfter(4).toString(format) == "2007-03-15");
  QVERIFY(s.dateAfter(5).toString(format) == "2007-03-31");
  s.setStartDate(QDate(2007, 4, 29));
  QVERIFY(s.dateAfter(2).toString(format) == "2007-05-14");
  QVERIFY(s.dateAfter(3).toString(format) == "2007-05-29");
  QVERIFY(s.dateAfter(4).toString(format) == "2007-06-14");
  QVERIFY(s.dateAfter(5).toString(format) == "2007-06-29");
  s.setStartDate(QDate(2007, 4, 30));
  QVERIFY(s.dateAfter(2).toString(format) == "2007-05-15");
  QVERIFY(s.dateAfter(3).toString(format) == "2007-05-31");
  QVERIFY(s.dateAfter(4).toString(format) == "2007-06-15");
  QVERIFY(s.dateAfter(5).toString(format) == "2007-06-30");
}

void MyMoneyScheduleTest::testPaymentDates()
{
  MyMoneySchedule sch;
  QString ref_ok = QString(
                     "<!DOCTYPE TEST>\n"
                     "<SCHEDULE-CONTAINER>\n"

                     "<SCHEDULED_TX startDate=\"2003-12-31\" autoEnter=\"1\" weekendOption=\"0\" lastPayment=\"2006-01-31\" paymentType=\"2\" endDate=\"\" type=\"2\" id=\"SCH000032\" name=\"DSL\" fixed=\"0\" occurenceMultiplier=\"1\" occurence=\"32\" >\n" // krazy:exclude=spelling
                     " <PAYMENTS/>\n"
                     " <TRANSACTION postdate=\"2006-02-28\" memo=\"\" id=\"\" commodity=\"EUR\" entrydate=\"\" >\n"
                     "  <SPLITS>\n"
                     "   <SPLIT payee=\"P000076\" reconciledate=\"\" shares=\"1200/100\" action=\"Deposit\" bankid=\"\" number=\"\" reconcileflag=\"0\" memo=\"\" value=\"1200/100\" account=\"A000076\" />\n"
                     "   <SPLIT payee=\"\" reconciledate=\"\" shares=\"-1200/100\" action=\"Deposit\" bankid=\"\" number=\"\" reconcileflag=\"0\" memo=\"\" value=\"-1200/100\" account=\"A000009\" />\n"
                     "  </SPLITS>\n"
                     "  <KEYVALUEPAIRS/>\n"
                     " </TRANSACTION>\n"
                     "</SCHEDULED_TX>\n"

                     "</SCHEDULE-CONTAINER>\n"
                   );

  QDomDocument doc;
  QDomElement node;
  doc.setContent(ref_ok);
  node = doc.documentElement().firstChild().toElement();

  QDate startDate(2006, 1, 28);
  QDate endDate(2006, 5, 30);

  try {
    sch = MyMoneySchedule(node);
    QDate nextPayment = sch.nextPayment(startDate);
    QList<QDate> list = sch.paymentDates(nextPayment, endDate);
    QVERIFY(list.count() == 3);
    QVERIFY(list[0] == QDate(2006, 2, 28));
    QVERIFY(list[1] == QDate(2006, 3, 31));
    // Would fall on a Sunday so gets moved back to 28th.
    QVERIFY(list[2] == QDate(2006, 4, 28));

    // Add tests for each possible occurrence.
    // Check how paymentDates is meant to work
    // Build a list of expected dates and compare
    // MyMoneySchedule::OCCUR_ONCE
    sch.setOccurrence(MyMoneySchedule::OCCUR_ONCE);
    startDate.setYMD(2009, 1, 1);
    endDate.setYMD(2009, 12, 31);
    sch.setStartDate(startDate);
    sch.setNextDueDate(startDate);
    list = sch.paymentDates(startDate, endDate);
    QVERIFY(list.count() == 1);
    QVERIFY(list[0] == QDate(2009, 1, 1));
    // MyMoneySchedule::OCCUR_DAILY
    sch.setOccurrence(MyMoneySchedule::OCCUR_DAILY);
    startDate.setYMD(2009, 1, 1);
    endDate.setYMD(2009, 1, 5);
    sch.setStartDate(startDate);
    sch.setNextDueDate(startDate);
    list = sch.paymentDates(startDate, endDate);
    QVERIFY(list.count() == 5);
    QVERIFY(list[0] == QDate(2009, 1, 1));
    QVERIFY(list[1] == QDate(2009, 1, 2));
    // Would fall on Saturday so gets moved to 2nd.
    QVERIFY(list[2] == QDate(2009, 1, 2));
    // Would fall on Sunday so gets moved to 2nd.
    QVERIFY(list[3] == QDate(2009, 1, 2));
    QVERIFY(list[4] == QDate(2009, 1, 5));
    // MyMoneySchedule::OCCUR_DAILY with multiplier 2
    sch.setOccurrenceMultiplier(2);
    list = sch.paymentDates(startDate.addDays(1), endDate);
    QVERIFY(list.count() == 2);
    // Would fall on Sunday so gets moved to 2nd.
    QVERIFY(list[0] == QDate(2009, 1, 2));
    QVERIFY(list[1] == QDate(2009, 1, 5));
    sch.setOccurrenceMultiplier(1);
    // MyMoneySchedule::OCCUR_WEEKLY
    sch.setOccurrence(MyMoneySchedule::OCCUR_WEEKLY);
    startDate.setYMD(2009, 1, 6);
    endDate.setYMD(2009, 2, 4);
    sch.setStartDate(startDate);
    sch.setNextDueDate(startDate);
    list = sch.paymentDates(startDate, endDate);
    QVERIFY(list.count() == 5);
    QVERIFY(list[0] == QDate(2009, 1, 6));
    QVERIFY(list[1] == QDate(2009, 1, 13));
    QVERIFY(list[2] == QDate(2009, 1, 20));
    QVERIFY(list[3] == QDate(2009, 1, 27));
    QVERIFY(list[4] == QDate(2009, 2, 3));
    // MyMoneySchedule::OCCUR_EVERYOTHERWEEK
    sch.setOccurrence(MyMoneySchedule::OCCUR_EVERYOTHERWEEK);
    startDate.setYMD(2009, 2, 5);
    endDate.setYMD(2009, 4, 3);
    sch.setStartDate(startDate);
    sch.setNextDueDate(startDate);
    list = sch.paymentDates(startDate, endDate);
    QVERIFY(list.count() == 5);
    QVERIFY(list[0] == QDate(2009, 2, 5));
    QVERIFY(list[1] == QDate(2009, 2, 19));
    QVERIFY(list[2] == QDate(2009, 3, 5));
    QVERIFY(list[3] == QDate(2009, 3, 19));
    QVERIFY(list[4] == QDate(2009, 4, 2));
    // MyMoneySchedule::OCCUR_FORTNIGHTLY
    sch.setOccurrence(MyMoneySchedule::OCCUR_FORTNIGHTLY);
    startDate.setYMD(2009, 4, 4);
    endDate.setYMD(2009, 5, 31);
    sch.setStartDate(startDate);
    sch.setNextDueDate(startDate);
    list = sch.paymentDates(startDate, endDate);
    QVERIFY(list.count() == 4);
    // First one would fall on a Saturday and would get moved
    // to 3rd which is before start date so is not in list.
    // Would fall on a Saturday so gets moved to 17th.
    QVERIFY(list[0] == QDate(2009, 4, 17));
    // Would fall on a Saturday so gets moved to 1st.
    QVERIFY(list[1] == QDate(2009, 5, 1));
    // Would fall on a Saturday so gets moved to 15th.
    QVERIFY(list[2] == QDate(2009, 5, 15));
    // Would fall on a Saturday so gets moved to 29th.
    QVERIFY(list[3] == QDate(2009, 5, 29));
    // MyMoneySchedule::OCCUR_EVERYHALFMONTH
    sch.setOccurrence(MyMoneySchedule::OCCUR_EVERYHALFMONTH);
    startDate.setYMD(2009, 6, 1);
    endDate.setYMD(2009, 8, 11);
    sch.setStartDate(startDate);
    sch.setNextDueDate(startDate);
    list = sch.paymentDates(startDate, endDate);
    QVERIFY(list.count() == 5);
    QVERIFY(list[0] == QDate(2009, 6, 1));
    QVERIFY(list[1] == QDate(2009, 6, 16));
    QVERIFY(list[2] == QDate(2009, 7, 1));
    QVERIFY(list[3] == QDate(2009, 7, 16));
    // Would fall on a Saturday so gets moved to 31st.
    QVERIFY(list[4] == QDate(2009, 7, 31));
    // MyMoneySchedule::OCCUR_EVERYTHREEWEEKS
    sch.setOccurrence(MyMoneySchedule::OCCUR_EVERYTHREEWEEKS);
    startDate.setYMD(2009, 8, 12);
    endDate.setYMD(2009, 11, 12);
    sch.setStartDate(startDate);
    sch.setNextDueDate(startDate);
    list = sch.paymentDates(startDate, endDate);
    QVERIFY(list.count() == 5);
    QVERIFY(list[0] == QDate(2009, 8, 12));
    QVERIFY(list[1] == QDate(2009, 9, 2));
    QVERIFY(list[2] == QDate(2009, 9, 23));
    QVERIFY(list[3] == QDate(2009, 10, 14));
    QVERIFY(list[4] == QDate(2009, 11, 4));
    // MyMoneySchedule::OCCUR_EVERYFOURWEEKS
    sch.setOccurrence(MyMoneySchedule::OCCUR_EVERYFOURWEEKS);
    startDate.setYMD(2009, 11, 13);
    endDate.setYMD(2010, 3, 13);
    sch.setStartDate(startDate);
    sch.setNextDueDate(startDate);
    list = sch.paymentDates(startDate, endDate);
    QVERIFY(list.count() == 5);
    QVERIFY(list[0] == QDate(2009, 11, 13));
    QVERIFY(list[1] == QDate(2009, 12, 11));
    QVERIFY(list[2] == QDate(2010, 1, 8));
    QVERIFY(list[3] == QDate(2010, 2, 5));
    QVERIFY(list[4] == QDate(2010, 3, 5));
    // MyMoneySchedule::OCCUR_EVERYTHIRTYDAYS
    sch.setOccurrence(MyMoneySchedule::OCCUR_EVERYTHIRTYDAYS);
    startDate.setYMD(2010, 3, 19);
    endDate.setYMD(2010, 7, 19);
    sch.setStartDate(startDate);
    sch.setNextDueDate(startDate);
    list = sch.paymentDates(startDate, endDate);
    QVERIFY(list.count() == 5);
    QVERIFY(list[0] == QDate(2010, 3, 19));
    // Would fall on a Sunday so gets moved to 16th.
    QVERIFY(list[1] == QDate(2010, 4, 16));
    QVERIFY(list[2] == QDate(2010, 5, 18));
    QVERIFY(list[3] == QDate(2010, 6, 17));
    // Would fall on a Saturday so gets moved to 16th.
    QVERIFY(list[4] == QDate(2010, 7, 16));
    // MyMoneySchedule::OCCUR_EVERYEIGHTWEEKS
    sch.setOccurrence(MyMoneySchedule::OCCUR_EVERYEIGHTWEEKS);
    startDate.setYMD(2010, 7, 26);
    endDate.setYMD(2011, 3, 26);
    sch.setStartDate(startDate);
    sch.setNextDueDate(startDate);
    list = sch.paymentDates(startDate, endDate);
    QVERIFY(list.count() == 5);
    QVERIFY(list[0] == QDate(2010, 7, 26));
    QVERIFY(list[1] == QDate(2010, 9, 20));
    QVERIFY(list[2] == QDate(2010, 11, 15));
    QVERIFY(list[3] == QDate(2011, 1, 10));
    QVERIFY(list[4] == QDate(2011, 3, 7));
    // MyMoneySchedule::OCCUR_EVERYOTHERMONTH
    sch.setOccurrence(MyMoneySchedule::OCCUR_EVERYOTHERMONTH);
    startDate.setYMD(2011, 3, 14);
    endDate.setYMD(2011, 11, 20);
    sch.setStartDate(startDate);
    sch.setNextDueDate(startDate);
    list = sch.paymentDates(startDate, endDate);
    QVERIFY(list.count() == 5);
    QVERIFY(list[0] == QDate(2011, 3, 14));
    // Would fall on a Saturday so gets moved to 13th.
    QVERIFY(list[1] == QDate(2011, 5, 13));
    QVERIFY(list[2] == QDate(2011, 7, 14));
    QVERIFY(list[3] == QDate(2011, 9, 14));
    QVERIFY(list[4] == QDate(2011, 11, 14));
    // MyMoneySchedule::OCCUR_EVERYTHREEMONTHS
    sch.setOccurrence(MyMoneySchedule::OCCUR_EVERYTHREEMONTHS);
    startDate.setYMD(2011, 11, 15);
    endDate.setYMD(2012, 11, 19);
    sch.setStartDate(startDate);
    sch.setNextDueDate(startDate);
    list = sch.paymentDates(startDate, endDate);
    QVERIFY(list.count() == 5);
    QVERIFY(list[0] == QDate(2011, 11, 15));
    QVERIFY(list[1] == QDate(2012, 2, 15));
    QVERIFY(list[2] == QDate(2012, 5, 15));
    QVERIFY(list[3] == QDate(2012, 8, 15));
    QVERIFY(list[4] == QDate(2012, 11, 15));
    // MyMoneySchedule::OCCUR_QUARTERLY
    sch.setOccurrence(MyMoneySchedule::OCCUR_QUARTERLY);
    startDate.setYMD(2012, 11, 20);
    endDate.setYMD(2013, 11, 23);
    sch.setStartDate(startDate);
    sch.setNextDueDate(startDate);
    list = sch.paymentDates(startDate, endDate);
    QVERIFY(list.count() == 5);
    QVERIFY(list[0] == QDate(2012, 11, 20));
    QVERIFY(list[1] == QDate(2013, 2, 20));
    QVERIFY(list[2] == QDate(2013, 5, 20));
    QVERIFY(list[3] == QDate(2013, 8, 20));
    QVERIFY(list[4] == QDate(2013, 11, 20));
    // MyMoneySchedule::OCCUR_EVERYFOURMONTHS
    sch.setOccurrence(MyMoneySchedule::OCCUR_EVERYFOURMONTHS);
    startDate.setYMD(2013, 11, 21);
    endDate.setYMD(2015, 3, 23);
    sch.setStartDate(startDate);
    sch.setNextDueDate(startDate);
    list = sch.paymentDates(startDate, endDate);
    QVERIFY(list.count() == 5);
    QVERIFY(list[0] == QDate(2013, 11, 21));
    QVERIFY(list[1] == QDate(2014, 3, 21));
    QVERIFY(list[2] == QDate(2014, 7, 21));
    QVERIFY(list[3] == QDate(2014, 11, 21));
    // Would fall on a Saturday so gets moved to 20th.
    QVERIFY(list[4] == QDate(2015, 3, 20));
    // MyMoneySchedule::OCCUR_TWICEYEARLY
    sch.setOccurrence(MyMoneySchedule::OCCUR_TWICEYEARLY);
    startDate.setYMD(2015, 3, 22);
    endDate.setYMD(2017, 3, 29);
    sch.setStartDate(startDate);
    sch.setNextDueDate(startDate);
    list = sch.paymentDates(startDate, endDate);
    QVERIFY(list.count() == 4);
    // First date would fall on a Sunday which would get moved
    // to 20th which is before start date so not in list.
    QVERIFY(list[0] == QDate(2015, 9, 22));
    QVERIFY(list[1] == QDate(2016, 3, 22));
    QVERIFY(list[2] == QDate(2016, 9, 22));
    QVERIFY(list[3] == QDate(2017, 3, 22));
    // MyMoneySchedule::OCCUR_YEARLY
    sch.setOccurrence(MyMoneySchedule::OCCUR_YEARLY);
    startDate.setYMD(2017, 3, 23);
    endDate.setYMD(2021, 3, 29);
    sch.setStartDate(startDate);
    sch.setNextDueDate(startDate);
    list = sch.paymentDates(startDate, endDate);
    QVERIFY(list.count() == 5);
    QVERIFY(list[0] == QDate(2017, 3, 23));
    QVERIFY(list[1] == QDate(2018, 3, 23));
    // Would fall on a Saturday so gets moved to 22nd.
    QVERIFY(list[2] == QDate(2019, 3, 22));
    QVERIFY(list[3] == QDate(2020, 3, 23));
    QVERIFY(list[4] == QDate(2021, 3, 23));
    // MyMoneySchedule::OCCUR_EVERYOTHERYEAR
    sch.setOccurrence(MyMoneySchedule::OCCUR_EVERYOTHERYEAR);
    startDate.setYMD(2021, 3, 24);
    endDate.setYMD(2029, 3, 30);
    sch.setStartDate(startDate);
    sch.setNextDueDate(startDate);
    list = sch.paymentDates(startDate, endDate);
    QVERIFY(list.count() == 5);
    QVERIFY(list[0] == QDate(2021, 3, 24));
    QVERIFY(list[1] == QDate(2023, 3, 24));
    QVERIFY(list[2] == QDate(2025, 3, 24));
    QVERIFY(list[3] == QDate(2027, 3, 24));
    // Would fall on a Saturday so gets moved to 23rd.
    QVERIFY(list[4] == QDate(2029, 3, 23));
  } catch (MyMoneyException *e) {
    delete e;
    QFAIL("Unexpected exception");
  }
}

void MyMoneyScheduleTest::testWriteXML()
{
  MyMoneySchedule sch("A Name",
                      MyMoneySchedule::TYPE_BILL,
                      MyMoneySchedule::OCCUR_WEEKLY, 123,
                      MyMoneySchedule::STYPE_DIRECTDEBIT,
                      QDate::currentDate(),
                      QDate(),
                      true,
                      true);

  sch.setLastPayment(QDate::currentDate());
  sch.recordPayment(QDate::currentDate());
  sch.setId("SCH0001");

  MyMoneyTransaction t;
  t.setPostDate(QDate(2001, 12, 28));
  t.setEntryDate(QDate(2003, 9, 29));
  t.setId("T000000000000000001");
  t.setMemo("Wohnung:Miete");
  t.setCommodity("EUR");
  t.setValue("key", "value");

  MyMoneySplit s;
  s.setPayeeId("P000001");
  s.setShares(MyMoneyMoney(96379, 100));
  s.setValue(MyMoneyMoney(96379, 100));
  s.setAccountId("A000076");
  s.setBankID("SPID1");
  s.setReconcileFlag(MyMoneySplit::Reconciled);
  t.addSplit(s);

  s.setPayeeId("P000001");
  s.setShares(MyMoneyMoney(-96379, 100));
  s.setValue(MyMoneyMoney(-96379, 100));
  s.setAccountId("A000276");
  s.setBankID("SPID2");
  s.setReconcileFlag(MyMoneySplit::Cleared);
  s.clearId();
  t.addSplit(s);

  sch.setTransaction(t);

  QDomDocument doc("TEST");
  QDomElement el = doc.createElement("SCHEDULE-CONTAINER");
  doc.appendChild(el);
  sch.writeXML(doc, el);

  QString ref = QString(
                  "<!DOCTYPE TEST>\n"
                  "<SCHEDULE-CONTAINER>\n"
                  " <SCHEDULED_TX paymentType=\"1\" autoEnter=\"1\" occurenceMultiplier=\"123\" startDate=\"%1\" lastPayment=\"%2\" occurence=\"4\" weekendOption=\"2\" type=\"1\" id=\"SCH0001\" name=\"A Name\" endDate=\"\" fixed=\"1\" >\n" // krazy:exclude=spelling
                  "  <PAYMENTS>\n"
                  "   <PAYMENT date=\"%3\" />\n"
                  "  </PAYMENTS>\n"
                  "  <TRANSACTION postdate=\"2001-12-28\" commodity=\"EUR\" memo=\"Wohnung:Miete\" id=\"\" entrydate=\"2003-09-29\" >\n"
                  "   <SPLITS>\n"
                  "    <SPLIT payee=\"P000001\" reconcileflag=\"2\" shares=\"96379/100\" reconciledate=\"\" action=\"\" bankid=\"\" account=\"A000076\" number=\"\" value=\"96379/100\" memo=\"\" id=\"S0001\" />\n"
                  "    <SPLIT payee=\"P000001\" reconcileflag=\"1\" shares=\"-96379/100\" reconciledate=\"\" action=\"\" bankid=\"\" account=\"A000276\" number=\"\" value=\"-96379/100\" memo=\"\" id=\"S0002\" />\n"
                  "   </SPLITS>\n"
                  "   <KEYVALUEPAIRS>\n"
                  "    <PAIR key=\"key\" value=\"value\" />\n"
                  "   </KEYVALUEPAIRS>\n"
                  "  </TRANSACTION>\n"
                  " </SCHEDULED_TX>\n"
                  "</SCHEDULE-CONTAINER>\n"
                ).arg(QDate::currentDate().toString(Qt::ISODate))
                .arg(QDate::currentDate().toString(Qt::ISODate))
                .arg(QDate::currentDate().toString(Qt::ISODate));

#if QT_VERSION >= QT_VERSION_CHECK(4,6,0)
  ref.replace(QString(" />\n"), QString("/>\n"));
  ref.replace(QString(" >\n"), QString(">\n"));
#endif

  //qDebug("ref = '%s'", qPrintable(ref));
  //qDebug("doc = '%s'", qPrintable(doc.toString()));

  QVERIFY(doc.toString() == ref);
}

void MyMoneyScheduleTest::testReadXML()
{
  MyMoneySchedule sch;

  QString ref_ok1 = QString(
                      "<!DOCTYPE TEST>\n"
                      "<SCHEDULE-CONTAINER>\n"
                      " <SCHEDULED_TX startDate=\"%1\" autoEnter=\"1\" weekendOption=\"2\" lastPayment=\"%2\" paymentType=\"1\" endDate=\"\" type=\"1\" id=\"SCH0002\" name=\"A Name\" fixed=\"1\" occurenceMultiplier=\"1\" occurence=\"4\" >\n" // krazy:exclude=spelling
                      "  <PAYMENTS>\n"
                      "   <PAYMENT date=\"%3\" />\n"
                      "  </PAYMENTS>\n"
                      "  <TRANSACTION postdate=\"2001-12-28\" memo=\"Wohnung:Miete\" id=\"\" commodity=\"EUR\" entrydate=\"2003-09-29\" >\n"
                      "   <SPLITS>\n"
                      "    <SPLIT payee=\"P000001\" reconciledate=\"\" shares=\"96379/100\" action=\"\" bankid=\"SPID1\" number=\"\" reconcileflag=\"2\" memo=\"\" value=\"96379/100\" account=\"A000076\" />\n"
                      "    <SPLIT payee=\"P000001\" reconciledate=\"\" shares=\"-96379/100\" action=\"\" bankid=\"SPID2\" number=\"\" reconcileflag=\"1\" memo=\"\" value=\"-96379/100\" account=\"A000276\" />\n"
                      "   </SPLITS>\n"
                      "   <KEYVALUEPAIRS>\n"
                      "    <PAIR key=\"key\" value=\"value\" />\n"
                      "   </KEYVALUEPAIRS>\n"
                      "  </TRANSACTION>\n"
                      " </SCHEDULED_TX>\n"
                      "</SCHEDULE-CONTAINER>\n"
                    ).arg(QDate::currentDate().toString(Qt::ISODate))
                    .arg(QDate::currentDate().toString(Qt::ISODate))
                    .arg(QDate::currentDate().toString(Qt::ISODate));

  // diff to ref_ok1 is that we now have an empty entrydate
  // in the transaction parameters
  QString ref_ok2 = QString(
                      "<!DOCTYPE TEST>\n"
                      "<SCHEDULE-CONTAINER>\n"
                      " <SCHEDULED_TX startDate=\"%1\" autoEnter=\"1\" weekendOption=\"2\" lastPayment=\"%2\" paymentType=\"1\" endDate=\"\" type=\"1\" id=\"SCH0002\" name=\"A Name\" fixed=\"1\" occurenceMultiplier=\"1\" occurence=\"4\" >\n" // krazy:exclude=spelling
                      "  <PAYMENTS>\n"
                      "   <PAYMENT date=\"%3\" />\n"
                      "  </PAYMENTS>\n"
                      "  <TRANSACTION postdate=\"2001-12-28\" memo=\"Wohnung:Miete\" id=\"\" commodity=\"EUR\" entrydate=\"\" >\n"
                      "   <SPLITS>\n"
                      "    <SPLIT payee=\"P000001\" reconciledate=\"\" shares=\"96379/100\" action=\"\" bankid=\"SPID1\" number=\"\" reconcileflag=\"2\" memo=\"\" value=\"96379/100\" account=\"A000076\" />\n"
                      "    <SPLIT payee=\"P000001\" reconciledate=\"\" shares=\"-96379/100\" action=\"\" bankid=\"SPID2\" number=\"\" reconcileflag=\"1\" memo=\"\" value=\"-96379/100\" account=\"A000276\" />\n"
                      "   </SPLITS>\n"
                      "   <KEYVALUEPAIRS>\n"
                      "    <PAIR key=\"key\" value=\"value\" />\n"
                      "   </KEYVALUEPAIRS>\n"
                      "  </TRANSACTION>\n"
                      " </SCHEDULED_TX>\n"
                      "</SCHEDULE-CONTAINER>\n"
                    ).arg(QDate::currentDate().toString(Qt::ISODate))
                    .arg(QDate::currentDate().toString(Qt::ISODate))
                    .arg(QDate::currentDate().toString(Qt::ISODate));

  QString ref_false = QString(
                        "<!DOCTYPE TEST>\n"
                        "<SCHEDULE-CONTAINER>\n"
                        " <SCHEDULE startDate=\"%1\" autoEnter=\"1\" weekendOption=\"2\" lastPayment=\"%2\" paymentType=\"1\" endDate=\"\" type=\"1\" id=\"SCH0002\" name=\"A Name\" fixed=\"1\" occurenceMultiplier=\"1\" occurence=\"4\" >\n" // krazy:exclude=spelling
                        "  <PAYMENTS count=\"1\" >\n"
                        "   <PAYMENT date=\"%3\" />\n"
                        "  </PAYMENTS>\n"
                        "  <TRANSACTION postdate=\"2001-12-28\" memo=\"Wohnung:Miete\" id=\"\" commodity=\"EUR\" entrydate=\"2003-09-29\" >\n"
                        "   <SPLITS>\n"
                        "    <SPLIT payee=\"P000001\" reconciledate=\"\" shares=\"96379/100\" action=\"\" bankid=\"SPID1\" number=\"\" reconcileflag=\"2\" memo=\"\" value=\"96379/100\" account=\"A000076\" />\n"
                        "    <SPLIT payee=\"P000001\" reconciledate=\"\" shares=\"-96379/100\" action=\"\" bankid=\"SPID2\" number=\"\" reconcileflag=\"1\" memo=\"\" value=\"-96379/100\" account=\"A000276\" />\n"
                        "   </SPLITS>\n"
                        "   <KEYVALUEPAIRS>\n"
                        "    <PAIR key=\"key\" value=\"value\" />\n"
                        "   </KEYVALUEPAIRS>\n"
                        "  </TRANSACTION>\n"
                        " </SCHEDULED_TX>\n"
                        "</SCHEDULE-CONTAINER>\n"
                      ).arg(QDate::currentDate().toString(Qt::ISODate))
                      .arg(QDate::currentDate().toString(Qt::ISODate))
                      .arg(QDate::currentDate().toString(Qt::ISODate));

  QDomDocument doc;
  QDomElement node;
  doc.setContent(ref_false);
  node = doc.documentElement().firstChild().toElement();

  try {
    sch = MyMoneySchedule(node);
    QFAIL("Missing expected exception");
  } catch (MyMoneyException *e) {
    delete e;
  }

  doc.setContent(ref_ok1);
  node = doc.documentElement().firstChild().toElement();


  try {
    sch = MyMoneySchedule(node);
    QVERIFY(sch.id() == "SCH0002");
    QVERIFY(sch.nextDueDate() == QDate::currentDate().addDays(7));
    QVERIFY(sch.startDate() == QDate::currentDate());
    QVERIFY(sch.endDate() == QDate());
    QVERIFY(sch.autoEnter() == true);
    QVERIFY(sch.isFixed() == true);
    QVERIFY(sch.weekendOption() == MyMoneySchedule::MoveNothing);
    QVERIFY(sch.lastPayment() == QDate::currentDate());
    QVERIFY(sch.paymentType() == MyMoneySchedule::STYPE_DIRECTDEBIT);
    QVERIFY(sch.type() == MyMoneySchedule::TYPE_BILL);
    QVERIFY(sch.name() == "A Name");
    QVERIFY(sch.occurrence() == MyMoneySchedule::OCCUR_WEEKLY);
    QVERIFY(sch.occurrenceMultiplier() == 1);
    QVERIFY(sch.nextDueDate() == sch.lastPayment().addDays(7));
    QVERIFY(sch.recordedPayments().count() == 1);
    QVERIFY(sch.recordedPayments()[0] == QDate::currentDate());
  } catch (MyMoneyException *e) {
    delete e;
    QFAIL("Unexpected exception");
  }

  doc.setContent(ref_ok2);
  node = doc.documentElement().firstChild().toElement();


  try {
    sch = MyMoneySchedule(node);
    QVERIFY(sch.id() == "SCH0002");
    QVERIFY(sch.nextDueDate() == QDate::currentDate().addDays(7));
    QVERIFY(sch.startDate() == QDate::currentDate());
    QVERIFY(sch.endDate() == QDate());
    QVERIFY(sch.autoEnter() == true);
    QVERIFY(sch.isFixed() == true);
    QVERIFY(sch.weekendOption() == MyMoneySchedule::MoveNothing);
    QVERIFY(sch.lastPayment() == QDate::currentDate());
    QVERIFY(sch.paymentType() == MyMoneySchedule::STYPE_DIRECTDEBIT);
    QVERIFY(sch.type() == MyMoneySchedule::TYPE_BILL);
    QVERIFY(sch.name() == "A Name");
    QVERIFY(sch.occurrence() == MyMoneySchedule::OCCUR_WEEKLY);
    QVERIFY(sch.occurrenceMultiplier() == 1);
    QVERIFY(sch.nextDueDate() == sch.lastPayment().addDays(7));
    QVERIFY(sch.recordedPayments().count() == 1);
    QVERIFY(sch.recordedPayments()[0] == QDate::currentDate());
  } catch (MyMoneyException *e) {
    delete e;
    QFAIL("Unexpected exception");
  }
}

void MyMoneyScheduleTest::testHasReferenceTo()
{
  MyMoneySchedule sch;
  QString ref_ok = QString(
                     "<!DOCTYPE TEST>\n"
                     "<SCHEDULE-CONTAINER>\n"
                     " <SCHEDULED_TX startDate=\"%1\" autoEnter=\"1\" weekendOption=\"2\" lastPayment=\"%2\" paymentType=\"1\" endDate=\"\" type=\"1\" id=\"SCH0002\" name=\"A Name\" fixed=\"1\" occurenceMultiplier=\"1\" occurence=\"4\" >\n" // krazy:exclude=spelling
                     "  <PAYMENTS>\n"
                     "   <PAYMENT date=\"%3\" />\n"
                     "  </PAYMENTS>\n"
                     "  <TRANSACTION postdate=\"2001-12-28\" memo=\"Wohnung:Miete\" id=\"\" commodity=\"EUR\" entrydate=\"2003-09-29\" >\n"
                     "   <SPLITS>\n"
                     "    <SPLIT payee=\"P000001\" reconciledate=\"\" shares=\"96379/100\" action=\"\" number=\"\" reconcileflag=\"2\" memo=\"\" value=\"96379/100\" account=\"A000076\" />\n"
                     "    <SPLIT payee=\"P000001\" reconciledate=\"\" shares=\"-96379/100\" action=\"\" number=\"\" reconcileflag=\"1\" memo=\"\" value=\"-96379/100\" account=\"A000276\" />\n"
                     "   </SPLITS>\n"
                     "   <KEYVALUEPAIRS>\n"
                     "    <PAIR key=\"key\" value=\"value\" />\n"
                     "   </KEYVALUEPAIRS>\n"
                     "  </TRANSACTION>\n"
                     " </SCHEDULED_TX>\n"
                     "</SCHEDULE-CONTAINER>\n"
                   ).arg(QDate::currentDate().toString(Qt::ISODate))
                   .arg(QDate::currentDate().toString(Qt::ISODate))
                   .arg(QDate::currentDate().toString(Qt::ISODate));

  QDomDocument doc;
  QDomElement node;
  doc.setContent(ref_ok);
  node = doc.documentElement().firstChild().toElement();

  try {
    sch = MyMoneySchedule(node);

  } catch (MyMoneyException *e) {
    delete e;
    QFAIL("Unexpected exception");
  }

  QVERIFY(sch.hasReferenceTo("P000001") == true);
  QVERIFY(sch.hasReferenceTo("A000276") == true);
  QVERIFY(sch.hasReferenceTo("A000076") == true);
  QVERIFY(sch.hasReferenceTo("EUR") == true);
}

void MyMoneyScheduleTest::testAdjustedNextDueDate()
{
  MyMoneySchedule s;

  QDate dueDate(2007, 9, 3); // start on a Monday
  for (int i = 0; i < 7; ++i) {
    s.setNextDueDate(dueDate);
    s.setWeekendOption(MyMoneySchedule::MoveNothing);
    QVERIFY(s.adjustedNextDueDate() == dueDate);

    s.setWeekendOption(MyMoneySchedule::MoveBefore);
    switch (i) {
      case 5: // Saturday
      case 6: // Sunday
        QVERIFY(s.adjustedNextDueDate() == QDate(2007, 9, 7));
        break;
      default:
        QVERIFY(s.adjustedNextDueDate() == dueDate);
        break;
    }

    s.setWeekendOption(MyMoneySchedule::MoveAfter);
    switch (i) {
      case 5: // Saturday
      case 6: // Sunday
        QVERIFY(s.adjustedNextDueDate() == QDate(2007, 9, 10));
        break;
      default:
        QVERIFY(s.adjustedNextDueDate() == dueDate);
        break;
    }
    dueDate = dueDate.addDays(1);
  }
}

void MyMoneyScheduleTest::testModifyNextDueDate(void)
{
  MyMoneySchedule s;
  s.setStartDate(QDate(2007, 1, 1));
  s.setOccurrence(MyMoneySchedule::OCCUR_MONTHLY);
  s.setNextDueDate(s.startDate().addMonths(1));
  s.setLastPayment(s.startDate());

  QList<QDate> dates;
  dates = s.paymentDates(QDate(2007, 2, 1), QDate(2007, 2, 1));
  QVERIFY(s.nextDueDate() == QDate(2007, 2, 1));
  QVERIFY(dates.count() == 1);
  QVERIFY(dates[0] == QDate(2007, 2, 1));

  s.setNextDueDate(QDate(2007, 1, 24));

  dates = s.paymentDates(QDate(2007, 2, 1), QDate(2007, 2, 1));
  QVERIFY(s.nextDueDate() == QDate(2007, 1, 24));
  QVERIFY(dates.count() == 0);

  dates = s.paymentDates(QDate(2007, 1, 24), QDate(2007, 1, 24));
  QVERIFY(dates.count() == 1);

  dates = s.paymentDates(QDate(2007, 1, 24), QDate(2007, 2, 24));
  QVERIFY(dates.count() == 2);
  QVERIFY(dates[0] == QDate(2007, 1, 24));
  QVERIFY(dates[1] == QDate(2007, 2, 24));

}

void MyMoneyScheduleTest::testDaysBetweenEvents()
{
  QVERIFY(MyMoneySchedule::daysBetweenEvents(MyMoneySchedule::OCCUR_ONCE) == 0);
  QVERIFY(MyMoneySchedule::daysBetweenEvents(MyMoneySchedule::OCCUR_DAILY) == 1);
  QVERIFY(MyMoneySchedule::daysBetweenEvents(MyMoneySchedule::OCCUR_WEEKLY) == 7);
  QVERIFY(MyMoneySchedule::daysBetweenEvents(MyMoneySchedule::OCCUR_EVERYOTHERWEEK) == 14);
  QVERIFY(MyMoneySchedule::daysBetweenEvents(MyMoneySchedule::OCCUR_FORTNIGHTLY) == 14);
  QVERIFY(MyMoneySchedule::daysBetweenEvents(MyMoneySchedule::OCCUR_EVERYHALFMONTH) == 15);
  QVERIFY(MyMoneySchedule::daysBetweenEvents(MyMoneySchedule::OCCUR_EVERYTHREEWEEKS) == 21);
  QVERIFY(MyMoneySchedule::daysBetweenEvents(MyMoneySchedule::OCCUR_EVERYFOURWEEKS) == 28);
  QVERIFY(MyMoneySchedule::daysBetweenEvents(MyMoneySchedule::OCCUR_EVERYTHIRTYDAYS) == 30);
  QVERIFY(MyMoneySchedule::daysBetweenEvents(MyMoneySchedule::OCCUR_MONTHLY) == 30);
  QVERIFY(MyMoneySchedule::daysBetweenEvents(MyMoneySchedule::OCCUR_EVERYEIGHTWEEKS) == 56);
  QVERIFY(MyMoneySchedule::daysBetweenEvents(MyMoneySchedule::OCCUR_EVERYOTHERMONTH) == 60);
  QVERIFY(MyMoneySchedule::daysBetweenEvents(MyMoneySchedule::OCCUR_EVERYTHREEMONTHS) == 90);
  QVERIFY(MyMoneySchedule::daysBetweenEvents(MyMoneySchedule::OCCUR_QUARTERLY) == 90);
  QVERIFY(MyMoneySchedule::daysBetweenEvents(MyMoneySchedule::OCCUR_EVERYFOURMONTHS) == 120);
  QVERIFY(MyMoneySchedule::daysBetweenEvents(MyMoneySchedule::OCCUR_TWICEYEARLY) == 180);
  QVERIFY(MyMoneySchedule::daysBetweenEvents(MyMoneySchedule::OCCUR_YEARLY) == 360);
  QVERIFY(MyMoneySchedule::daysBetweenEvents(MyMoneySchedule::OCCUR_EVERYOTHERYEAR) == 0);
}

void MyMoneyScheduleTest::testEventsPerYear()
{
  QVERIFY(MyMoneySchedule::eventsPerYear(MyMoneySchedule::OCCUR_ONCE) == 0);
  QVERIFY(MyMoneySchedule::eventsPerYear(MyMoneySchedule::OCCUR_DAILY) == 365);
  QVERIFY(MyMoneySchedule::eventsPerYear(MyMoneySchedule::OCCUR_WEEKLY) == 52);
  QVERIFY(MyMoneySchedule::eventsPerYear(MyMoneySchedule::OCCUR_EVERYOTHERWEEK) == 26);
  QVERIFY(MyMoneySchedule::eventsPerYear(MyMoneySchedule::OCCUR_FORTNIGHTLY) == 26);
  QVERIFY(MyMoneySchedule::eventsPerYear(MyMoneySchedule::OCCUR_EVERYHALFMONTH) == 24);
  QVERIFY(MyMoneySchedule::eventsPerYear(MyMoneySchedule::OCCUR_EVERYTHREEWEEKS) == 17);
  QVERIFY(MyMoneySchedule::eventsPerYear(MyMoneySchedule::OCCUR_EVERYFOURWEEKS) == 13);
  QVERIFY(MyMoneySchedule::eventsPerYear(MyMoneySchedule::OCCUR_EVERYTHIRTYDAYS) == 12);
  QVERIFY(MyMoneySchedule::eventsPerYear(MyMoneySchedule::OCCUR_MONTHLY) == 12);
  QVERIFY(MyMoneySchedule::eventsPerYear(MyMoneySchedule::OCCUR_EVERYEIGHTWEEKS) == 6);
  QVERIFY(MyMoneySchedule::eventsPerYear(MyMoneySchedule::OCCUR_EVERYOTHERMONTH) == 6);
  QVERIFY(MyMoneySchedule::eventsPerYear(MyMoneySchedule::OCCUR_EVERYTHREEMONTHS) == 4);
  QVERIFY(MyMoneySchedule::eventsPerYear(MyMoneySchedule::OCCUR_QUARTERLY) == 4);
  QVERIFY(MyMoneySchedule::eventsPerYear(MyMoneySchedule::OCCUR_EVERYFOURMONTHS) == 3);
  QVERIFY(MyMoneySchedule::eventsPerYear(MyMoneySchedule::OCCUR_TWICEYEARLY) == 2);
  QVERIFY(MyMoneySchedule::eventsPerYear(MyMoneySchedule::OCCUR_YEARLY) == 1);
  QVERIFY(MyMoneySchedule::eventsPerYear(MyMoneySchedule::OCCUR_EVERYOTHERYEAR) == 0);
}

void MyMoneyScheduleTest::testOccurrenceToString()
{
  // For each occurrenceE test MyMoneySchedule::occurrenceToString(occurrenceE)
  QVERIFY(MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_ONCE) == "Once");
  QVERIFY(MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_DAILY) == "Daily");
  QVERIFY(MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_WEEKLY) == "Weekly");
  QVERIFY(MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_EVERYOTHERWEEK) == "Every other week");
  QVERIFY(MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_FORTNIGHTLY) == "Fortnightly");
  QVERIFY(MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_EVERYHALFMONTH) == "Every half month");
  QVERIFY(MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_EVERYTHREEWEEKS) == "Every three weeks");
  QVERIFY(MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_EVERYFOURWEEKS) == "Every four weeks");
  QVERIFY(MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_EVERYTHIRTYDAYS) == "Every thirty days");
  QVERIFY(MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_MONTHLY) == "Monthly");
  QVERIFY(MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_EVERYEIGHTWEEKS) == "Every eight weeks");
  QVERIFY(MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_EVERYOTHERMONTH) == "Every two months");
  QVERIFY(MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_EVERYTHREEMONTHS) == "Every three months");
  QVERIFY(MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_QUARTERLY) == "Quarterly");
  QVERIFY(MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_EVERYFOURMONTHS) == "Every four months");
  QVERIFY(MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_TWICEYEARLY) == "Twice yearly");
  QVERIFY(MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_YEARLY) == "Yearly");
  QVERIFY(MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_EVERYOTHERYEAR) == "Every other year");
  // For each occurrenceE set occurrence and compare occurrenceToString() with oTS(occurrence())
  MyMoneySchedule s;
  s.setStartDate(QDate(2007, 1, 1));
  s.setNextDueDate(s.startDate());
  s.setLastPayment(s.startDate());
  s.setOccurrence(MyMoneySchedule::OCCUR_ONCE); QVERIFY(s.occurrenceToString() == "Once");
  s.setOccurrence(MyMoneySchedule::OCCUR_DAILY); QVERIFY(s.occurrenceToString() == "Daily");
  s.setOccurrence(MyMoneySchedule::OCCUR_WEEKLY); QVERIFY(s.occurrenceToString() == "Weekly");
  s.setOccurrence(MyMoneySchedule::OCCUR_EVERYOTHERWEEK); QVERIFY(s.occurrenceToString() == "Every other week");
  // Fortnightly no longer used: Every other week used instead
  s.setOccurrence(MyMoneySchedule::OCCUR_FORTNIGHTLY); QVERIFY(s.occurrenceToString() == "Every other week");
  s.setOccurrence(MyMoneySchedule::OCCUR_EVERYHALFMONTH); QVERIFY(s.occurrenceToString() == "Every half month");
  s.setOccurrence(MyMoneySchedule::OCCUR_EVERYTHREEWEEKS); QVERIFY(s.occurrenceToString() == "Every three weeks");
  s.setOccurrence(MyMoneySchedule::OCCUR_EVERYFOURWEEKS); QVERIFY(s.occurrenceToString() == "Every four weeks");
  s.setOccurrence(MyMoneySchedule::OCCUR_EVERYTHIRTYDAYS); QVERIFY(s.occurrenceToString() == "Every thirty days");
  s.setOccurrence(MyMoneySchedule::OCCUR_MONTHLY); QVERIFY(s.occurrenceToString() == "Monthly");
  s.setOccurrence(MyMoneySchedule::OCCUR_EVERYEIGHTWEEKS); QVERIFY(s.occurrenceToString() == "Every eight weeks");
  s.setOccurrence(MyMoneySchedule::OCCUR_EVERYOTHERMONTH); QVERIFY(s.occurrenceToString() == "Every two months");
  s.setOccurrence(MyMoneySchedule::OCCUR_EVERYTHREEMONTHS); QVERIFY(s.occurrenceToString() == "Every three months");
  // Quarterly no longer used.  Every three months used instead
  s.setOccurrence(MyMoneySchedule::OCCUR_QUARTERLY); QVERIFY(s.occurrenceToString() == "Every three months");
  s.setOccurrence(MyMoneySchedule::OCCUR_EVERYFOURMONTHS); QVERIFY(s.occurrenceToString() == "Every four months");
  s.setOccurrence(MyMoneySchedule::OCCUR_TWICEYEARLY); QVERIFY(s.occurrenceToString() == "Twice yearly");
  s.setOccurrence(MyMoneySchedule::OCCUR_YEARLY); QVERIFY(s.occurrenceToString() == "Yearly");
  s.setOccurrence(MyMoneySchedule::OCCUR_EVERYOTHERYEAR); QVERIFY(s.occurrenceToString() == "Every other year");
  // Test occurrenceToString(mult,occ)
  // Test all pairs equivalent to simple occurrences: should return the same as occurrenceToString(simpleOcc)
  // TODO replace string with (mult,occ) call.
  QVERIFY(MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_ONCE) == MyMoneySchedule::occurrenceToString(1, MyMoneySchedule::OCCUR_ONCE));
  QVERIFY(MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_DAILY) == MyMoneySchedule::occurrenceToString(1, MyMoneySchedule::OCCUR_DAILY));
  QVERIFY(MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_WEEKLY) == MyMoneySchedule::occurrenceToString(1, MyMoneySchedule::OCCUR_WEEKLY));
  QVERIFY(MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_EVERYOTHERWEEK) == MyMoneySchedule::occurrenceToString(2, MyMoneySchedule::OCCUR_WEEKLY));
  // OCCUR_FORTNIGHTLY will no longer be used: only Every Other Week
  QVERIFY(MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_EVERYHALFMONTH) == MyMoneySchedule::occurrenceToString(1, MyMoneySchedule::OCCUR_EVERYHALFMONTH));
  QVERIFY(MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_EVERYTHREEWEEKS) == MyMoneySchedule::occurrenceToString(3, MyMoneySchedule::OCCUR_WEEKLY));
  QVERIFY(MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_EVERYFOURWEEKS) == MyMoneySchedule::occurrenceToString(4, MyMoneySchedule::OCCUR_WEEKLY));
  QVERIFY(MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_MONTHLY) == MyMoneySchedule::occurrenceToString(1, MyMoneySchedule::OCCUR_MONTHLY));
  QVERIFY(MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_EVERYEIGHTWEEKS) == MyMoneySchedule::occurrenceToString(8, MyMoneySchedule::OCCUR_WEEKLY));
  QVERIFY(MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_EVERYOTHERMONTH) == MyMoneySchedule::occurrenceToString(2, MyMoneySchedule::OCCUR_MONTHLY));
  QVERIFY(MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_EVERYTHREEMONTHS) == MyMoneySchedule::occurrenceToString(3, MyMoneySchedule::OCCUR_MONTHLY));
  // OCCUR_QUARTERLY will no longer be used: only Every Three Months
  QVERIFY(MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_EVERYFOURMONTHS) == MyMoneySchedule::occurrenceToString(4, MyMoneySchedule::OCCUR_MONTHLY));
  QVERIFY(MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_TWICEYEARLY) == MyMoneySchedule::occurrenceToString(6, MyMoneySchedule::OCCUR_MONTHLY));
  QVERIFY(MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_YEARLY) == MyMoneySchedule::occurrenceToString(1, MyMoneySchedule::OCCUR_YEARLY));
  QVERIFY(MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_EVERYOTHERYEAR) == MyMoneySchedule::occurrenceToString(2, MyMoneySchedule::OCCUR_YEARLY));
  // Test additional calls with other mult,occ
  QVERIFY(MyMoneySchedule::occurrenceToString(2, MyMoneySchedule::OCCUR_ONCE) == "2 times");
  QVERIFY(MyMoneySchedule::occurrenceToString(2, MyMoneySchedule::OCCUR_DAILY) == "Every 2 days");
  QVERIFY(MyMoneySchedule::occurrenceToString(5, MyMoneySchedule::OCCUR_WEEKLY) == "Every 5 weeks");
  QVERIFY(MyMoneySchedule::occurrenceToString(2, MyMoneySchedule::OCCUR_EVERYHALFMONTH) == "Every 2 half months");
  QVERIFY(MyMoneySchedule::occurrenceToString(5, MyMoneySchedule::OCCUR_MONTHLY) == "Every 5 months");
  QVERIFY(MyMoneySchedule::occurrenceToString(3, MyMoneySchedule::OCCUR_YEARLY) == "Every 3 years");
  QVERIFY(MyMoneySchedule::occurrenceToString(37, MyMoneySchedule::OCCUR_ONCE) == "37 times");
  QVERIFY(MyMoneySchedule::occurrenceToString(43, MyMoneySchedule::OCCUR_DAILY) == "Every 43 days");
  QVERIFY(MyMoneySchedule::occurrenceToString(61, MyMoneySchedule::OCCUR_WEEKLY) == "Every 61 weeks");
  QVERIFY(MyMoneySchedule::occurrenceToString(73, MyMoneySchedule::OCCUR_EVERYHALFMONTH) == "Every 73 half months");
  QVERIFY(MyMoneySchedule::occurrenceToString(83, MyMoneySchedule::OCCUR_MONTHLY) == "Every 83 months");
  QVERIFY(MyMoneySchedule::occurrenceToString(89, MyMoneySchedule::OCCUR_YEARLY) == "Every 89 years");
  // Test instance-level occurrenceToString method is using occurrencePeriod and multiplier
  // For each base occurrence set occurrencePeriod and multiplier
  s.setOccurrencePeriod(MyMoneySchedule::OCCUR_ONCE); s.setOccurrenceMultiplier(1);
  s.setOccurrence(MyMoneySchedule::OCCUR_ONCE);
  s.setOccurrenceMultiplier(1); QVERIFY(s.occurrenceToString() == "Once");
  s.setOccurrenceMultiplier(2); QVERIFY(s.occurrenceToString() == "2 times");
  s.setOccurrenceMultiplier(3); QVERIFY(s.occurrenceToString() == "3 times");
  s.setOccurrencePeriod(MyMoneySchedule::OCCUR_DAILY);
  s.setOccurrenceMultiplier(1); QVERIFY(s.occurrenceToString() == "Daily");
  s.setOccurrenceMultiplier(30); QVERIFY(s.occurrenceToString() == "Every thirty days");
  s.setOccurrenceMultiplier(3); QVERIFY(s.occurrenceToString() == "Every 3 days");
  s.setOccurrence(MyMoneySchedule::OCCUR_WEEKLY);
  QVERIFY(s.occurrenceToString() == "Weekly");
  s.setOccurrenceMultiplier(2); QVERIFY(s.occurrenceToString() == "Every other week");
  s.setOccurrenceMultiplier(3); QVERIFY(s.occurrenceToString() == "Every three weeks");
  s.setOccurrenceMultiplier(4); QVERIFY(s.occurrenceToString() == "Every four weeks");
  s.setOccurrenceMultiplier(5); QVERIFY(s.occurrenceToString() == "Every 5 weeks");
  s.setOccurrenceMultiplier(7); QVERIFY(s.occurrenceToString() == "Every 7 weeks");
  s.setOccurrenceMultiplier(8); QVERIFY(s.occurrenceToString() == "Every eight weeks");
  s.setOccurrenceMultiplier(9); QVERIFY(s.occurrenceToString() == "Every 9 weeks");
  s.setOccurrence(MyMoneySchedule::OCCUR_EVERYHALFMONTH);
  s.setOccurrenceMultiplier(1); QVERIFY(s.occurrenceToString() == "Every half month");
  s.setOccurrenceMultiplier(2); QVERIFY(s.occurrenceToString() == "Every 2 half months");
  s.setOccurrence(MyMoneySchedule::OCCUR_MONTHLY);
  s.setOccurrenceMultiplier(1); QVERIFY(s.occurrenceToString() == "Monthly");
  s.setOccurrenceMultiplier(2); QVERIFY(s.occurrenceToString() == "Every two months");
  s.setOccurrenceMultiplier(3); QVERIFY(s.occurrenceToString() == "Every three months");
  s.setOccurrenceMultiplier(4); QVERIFY(s.occurrenceToString() == "Every four months");
  s.setOccurrenceMultiplier(5); QVERIFY(s.occurrenceToString() == "Every 5 months");
  s.setOccurrenceMultiplier(6); QVERIFY(s.occurrenceToString() == "Twice yearly");
  s.setOccurrenceMultiplier(7); QVERIFY(s.occurrenceToString() == "Every 7 months");
  s.setOccurrence(MyMoneySchedule::OCCUR_YEARLY);
  s.setOccurrenceMultiplier(1); QVERIFY(s.occurrenceToString() == "Yearly");
  s.setOccurrenceMultiplier(2); QVERIFY(s.occurrenceToString() == "Every other year");
  s.setOccurrenceMultiplier(3); QVERIFY(s.occurrenceToString() == "Every 3 years");
}

void MyMoneyScheduleTest::testOccurrencePeriodToString()
{
  // For each occurrenceE test MyMoneySchedule::occurrencePeriodToString(occurrenceE)
  // Base occurrences are translated
  QVERIFY(MyMoneySchedule::occurrencePeriodToString(MyMoneySchedule::OCCUR_ONCE) == "Once");
  QVERIFY(MyMoneySchedule::occurrencePeriodToString(MyMoneySchedule::OCCUR_DAILY) == "Day");
  QVERIFY(MyMoneySchedule::occurrencePeriodToString(MyMoneySchedule::OCCUR_WEEKLY) == "Week");
  QVERIFY(MyMoneySchedule::occurrencePeriodToString(MyMoneySchedule::OCCUR_EVERYHALFMONTH) == "Half-month");
  QVERIFY(MyMoneySchedule::occurrencePeriodToString(MyMoneySchedule::OCCUR_MONTHLY) == "Month");
  QVERIFY(MyMoneySchedule::occurrencePeriodToString(MyMoneySchedule::OCCUR_YEARLY) == "Year");
  // All others are not translated so return Any
  QVERIFY(MyMoneySchedule::occurrencePeriodToString(MyMoneySchedule::OCCUR_EVERYOTHERWEEK) == "Any");
  QVERIFY(MyMoneySchedule::occurrencePeriodToString(MyMoneySchedule::OCCUR_FORTNIGHTLY) == "Any");
  QVERIFY(MyMoneySchedule::occurrencePeriodToString(MyMoneySchedule::OCCUR_EVERYTHREEWEEKS) == "Any");
  QVERIFY(MyMoneySchedule::occurrencePeriodToString(MyMoneySchedule::OCCUR_EVERYFOURWEEKS) == "Any");
  QVERIFY(MyMoneySchedule::occurrencePeriodToString(MyMoneySchedule::OCCUR_EVERYTHIRTYDAYS) == "Any");
  QVERIFY(MyMoneySchedule::occurrencePeriodToString(MyMoneySchedule::OCCUR_EVERYEIGHTWEEKS) == "Any");
  QVERIFY(MyMoneySchedule::occurrencePeriodToString(MyMoneySchedule::OCCUR_EVERYOTHERMONTH) == "Any");
  QVERIFY(MyMoneySchedule::occurrencePeriodToString(MyMoneySchedule::OCCUR_EVERYTHREEMONTHS) == "Any");
  QVERIFY(MyMoneySchedule::occurrencePeriodToString(MyMoneySchedule::OCCUR_QUARTERLY) == "Any");
  QVERIFY(MyMoneySchedule::occurrencePeriodToString(MyMoneySchedule::OCCUR_EVERYFOURMONTHS) == "Any");
  QVERIFY(MyMoneySchedule::occurrencePeriodToString(MyMoneySchedule::OCCUR_TWICEYEARLY) == "Any");
  QVERIFY(MyMoneySchedule::occurrencePeriodToString(MyMoneySchedule::OCCUR_EVERYOTHERYEAR) == "Any");
}

void MyMoneyScheduleTest::testOccurrencePeriod()
{
  // Each occurrence:
  // Set occurrence using setOccurrencePeriod
  // occurrencePeriod should match what we set
  // occurrence depends on multiplier
  // TODO:
  // Once occurrence() and setOccurrence() are converting between compound and simple occurrences
  // we need to change the occurrence() check and add an occurrenceMultiplier() check
  MyMoneySchedule s;
  s.setStartDate(QDate(2007, 1, 1));
  s.setNextDueDate(s.startDate());
  s.setLastPayment(s.startDate());
  // Set all base occurrences
  s.setOccurrencePeriod(MyMoneySchedule::OCCUR_ONCE);
  QVERIFY(s.occurrencePeriod() == MyMoneySchedule::OCCUR_ONCE);
  s.setOccurrenceMultiplier(1);
  QVERIFY(s.occurrenceMultiplier() == 1);
  QVERIFY(s.occurrencePeriod() == MyMoneySchedule::OCCUR_ONCE);
  QVERIFY(s.occurrence() == MyMoneySchedule::OCCUR_ONCE);
  s.setOccurrenceMultiplier(2);
  QVERIFY(s.occurrenceMultiplier() == 2);
  QVERIFY(s.occurrencePeriod() == MyMoneySchedule::OCCUR_ONCE);
  QVERIFY(s.occurrence() == MyMoneySchedule::OCCUR_ONCE);

  s.setOccurrencePeriod(MyMoneySchedule::OCCUR_DAILY);
  QVERIFY(s.occurrencePeriod() == MyMoneySchedule::OCCUR_DAILY);
  s.setOccurrenceMultiplier(1);
  QVERIFY(s.occurrenceMultiplier() == 1);
  QVERIFY(s.occurrencePeriod() == MyMoneySchedule::OCCUR_DAILY);
  QVERIFY(s.occurrence() == MyMoneySchedule::OCCUR_DAILY);
  s.setOccurrenceMultiplier(30);
  QVERIFY(s.occurrenceMultiplier() == 30);
  QVERIFY(s.occurrencePeriod() == MyMoneySchedule::OCCUR_DAILY);
  QVERIFY(s.occurrence() == MyMoneySchedule::OCCUR_EVERYTHIRTYDAYS);
  s.setOccurrenceMultiplier(2);
  QVERIFY(s.occurrenceMultiplier() == 2);
  QVERIFY(s.occurrencePeriod() == MyMoneySchedule::OCCUR_DAILY);
  QVERIFY(s.occurrence() == MyMoneySchedule::OCCUR_DAILY);

  s.setOccurrencePeriod(MyMoneySchedule::OCCUR_WEEKLY);
  QVERIFY(s.occurrencePeriod() == MyMoneySchedule::OCCUR_WEEKLY);
  s.setOccurrenceMultiplier(1);
  QVERIFY(s.occurrenceMultiplier() == 1);
  QVERIFY(s.occurrencePeriod() == MyMoneySchedule::OCCUR_WEEKLY);
  QVERIFY(s.occurrence() == MyMoneySchedule::OCCUR_WEEKLY);
  s.setOccurrenceMultiplier(2);
  QVERIFY(s.occurrenceMultiplier() == 2);
  QVERIFY(s.occurrencePeriod() == MyMoneySchedule::OCCUR_WEEKLY);
  QVERIFY(s.occurrence() == MyMoneySchedule::OCCUR_EVERYOTHERWEEK);
  s.setOccurrenceMultiplier(3);
  QVERIFY(s.occurrenceMultiplier() == 3);
  QVERIFY(s.occurrencePeriod() == MyMoneySchedule::OCCUR_WEEKLY);
  QVERIFY(s.occurrence() == MyMoneySchedule::OCCUR_EVERYTHREEWEEKS);
  s.setOccurrenceMultiplier(4);
  QVERIFY(s.occurrenceMultiplier() == 4);
  QVERIFY(s.occurrencePeriod() == MyMoneySchedule::OCCUR_WEEKLY);
  QVERIFY(s.occurrence() == MyMoneySchedule::OCCUR_EVERYFOURWEEKS);
  s.setOccurrenceMultiplier(5);
  QVERIFY(s.occurrenceMultiplier() == 5);
  QVERIFY(s.occurrencePeriod() == MyMoneySchedule::OCCUR_WEEKLY);
  QVERIFY(s.occurrence() == MyMoneySchedule::OCCUR_WEEKLY);
  s.setOccurrenceMultiplier(8);
  QVERIFY(s.occurrenceMultiplier() == 8);
  QVERIFY(s.occurrencePeriod() == MyMoneySchedule::OCCUR_WEEKLY);
  QVERIFY(s.occurrence() == MyMoneySchedule::OCCUR_EVERYEIGHTWEEKS);

  s.setOccurrencePeriod(MyMoneySchedule::OCCUR_EVERYHALFMONTH);
  QVERIFY(s.occurrencePeriod() == MyMoneySchedule::OCCUR_EVERYHALFMONTH);
  s.setOccurrenceMultiplier(1);
  QVERIFY(s.occurrenceMultiplier() == 1);
  QVERIFY(s.occurrencePeriod() == MyMoneySchedule::OCCUR_EVERYHALFMONTH);
  QVERIFY(s.occurrence() == MyMoneySchedule::OCCUR_EVERYHALFMONTH);
  s.setOccurrenceMultiplier(2);
  QVERIFY(s.occurrenceMultiplier() == 2);
  QVERIFY(s.occurrencePeriod() == MyMoneySchedule::OCCUR_EVERYHALFMONTH);
  QVERIFY(s.occurrence() == MyMoneySchedule::OCCUR_EVERYHALFMONTH);

  s.setOccurrencePeriod(MyMoneySchedule::OCCUR_MONTHLY);
  QVERIFY(s.occurrencePeriod() == MyMoneySchedule::OCCUR_MONTHLY);
  s.setOccurrenceMultiplier(1);
  QVERIFY(s.occurrenceMultiplier() == 1);
  QVERIFY(s.occurrencePeriod() == MyMoneySchedule::OCCUR_MONTHLY);
  QVERIFY(s.occurrence() == MyMoneySchedule::OCCUR_MONTHLY);
  s.setOccurrenceMultiplier(2);
  QVERIFY(s.occurrenceMultiplier() == 2);
  QVERIFY(s.occurrencePeriod() == MyMoneySchedule::OCCUR_MONTHLY);
  QVERIFY(s.occurrence() == MyMoneySchedule::OCCUR_EVERYOTHERMONTH);
  s.setOccurrenceMultiplier(3);
  QVERIFY(s.occurrenceMultiplier() == 3);
  QVERIFY(s.occurrencePeriod() == MyMoneySchedule::OCCUR_MONTHLY);
  QVERIFY(s.occurrence() == MyMoneySchedule::OCCUR_EVERYTHREEMONTHS);
  s.setOccurrenceMultiplier(4);
  QVERIFY(s.occurrenceMultiplier() == 4);
  QVERIFY(s.occurrencePeriod() == MyMoneySchedule::OCCUR_MONTHLY);
  QVERIFY(s.occurrence() == MyMoneySchedule::OCCUR_EVERYFOURMONTHS);
  s.setOccurrenceMultiplier(5);
  QVERIFY(s.occurrenceMultiplier() == 5);
  QVERIFY(s.occurrencePeriod() == MyMoneySchedule::OCCUR_MONTHLY);
  QVERIFY(s.occurrence() == MyMoneySchedule::OCCUR_MONTHLY);
  s.setOccurrenceMultiplier(6);
  QVERIFY(s.occurrenceMultiplier() == 6);
  QVERIFY(s.occurrencePeriod() == MyMoneySchedule::OCCUR_MONTHLY);
  QVERIFY(s.occurrence() == MyMoneySchedule::OCCUR_TWICEYEARLY);

  s.setOccurrencePeriod(MyMoneySchedule::OCCUR_YEARLY);
  QVERIFY(s.occurrencePeriod() == MyMoneySchedule::OCCUR_YEARLY);
  s.setOccurrenceMultiplier(1);
  QVERIFY(s.occurrenceMultiplier() == 1);
  QVERIFY(s.occurrencePeriod() == MyMoneySchedule::OCCUR_YEARLY);
  QVERIFY(s.occurrence() == MyMoneySchedule::OCCUR_YEARLY);
  s.setOccurrenceMultiplier(2);
  QVERIFY(s.occurrenceMultiplier() == 2);
  QVERIFY(s.occurrencePeriod() == MyMoneySchedule::OCCUR_YEARLY);
  QVERIFY(s.occurrence() == MyMoneySchedule::OCCUR_EVERYOTHERYEAR);
  s.setOccurrenceMultiplier(3);
  QVERIFY(s.occurrenceMultiplier() == 3);
  QVERIFY(s.occurrencePeriod() == MyMoneySchedule::OCCUR_YEARLY);
  QVERIFY(s.occurrence() == MyMoneySchedule::OCCUR_YEARLY);

  // Set occurrence: check occurrence, Period and Multiplier
  s.setOccurrence(MyMoneySchedule::OCCUR_ONCE);
  QVERIFY(s.occurrence() == MyMoneySchedule::OCCUR_ONCE);
  QVERIFY(s.occurrencePeriod() == MyMoneySchedule::OCCUR_ONCE);
  QVERIFY(s.occurrenceMultiplier() == 1);

  s.setOccurrence(MyMoneySchedule::OCCUR_DAILY);
  QVERIFY(s.occurrence() == MyMoneySchedule::OCCUR_DAILY);
  QVERIFY(s.occurrencePeriod() == MyMoneySchedule::OCCUR_DAILY);
  QVERIFY(s.occurrenceMultiplier() == 1);
  s.setOccurrence(MyMoneySchedule::OCCUR_EVERYTHIRTYDAYS);
  QVERIFY(s.occurrence() == MyMoneySchedule::OCCUR_EVERYTHIRTYDAYS);
  QVERIFY(s.occurrencePeriod() == MyMoneySchedule::OCCUR_DAILY);
  QVERIFY(s.occurrenceMultiplier() == 30);

  s.setOccurrence(MyMoneySchedule::OCCUR_WEEKLY);
  QVERIFY(s.occurrence() == MyMoneySchedule::OCCUR_WEEKLY);
  QVERIFY(s.occurrencePeriod() == MyMoneySchedule::OCCUR_WEEKLY);
  QVERIFY(s.occurrenceMultiplier() == 1);
  s.setOccurrence(MyMoneySchedule::OCCUR_EVERYOTHERWEEK);
  QVERIFY(s.occurrence() == MyMoneySchedule::OCCUR_EVERYOTHERWEEK);
  QVERIFY(s.occurrencePeriod() == MyMoneySchedule::OCCUR_WEEKLY);
  QVERIFY(s.occurrenceMultiplier() == 2);
  // Fortnightly no longer used: Every other week used instead
  s.setOccurrence(MyMoneySchedule::OCCUR_FORTNIGHTLY);
  QVERIFY(s.occurrence() == MyMoneySchedule::OCCUR_EVERYOTHERWEEK);
  QVERIFY(s.occurrencePeriod() == MyMoneySchedule::OCCUR_WEEKLY);
  QVERIFY(s.occurrenceMultiplier() == 2);
  s.setOccurrence(MyMoneySchedule::OCCUR_EVERYTHREEWEEKS);
  QVERIFY(s.occurrence() == MyMoneySchedule::OCCUR_EVERYTHREEWEEKS);
  QVERIFY(s.occurrencePeriod() == MyMoneySchedule::OCCUR_WEEKLY);
  QVERIFY(s.occurrenceMultiplier() == 3);
  s.setOccurrence(MyMoneySchedule::OCCUR_EVERYFOURWEEKS);
  QVERIFY(s.occurrence() == MyMoneySchedule::OCCUR_EVERYFOURWEEKS);
  QVERIFY(s.occurrencePeriod() == MyMoneySchedule::OCCUR_WEEKLY);
  QVERIFY(s.occurrenceMultiplier() == 4);
  s.setOccurrence(MyMoneySchedule::OCCUR_EVERYEIGHTWEEKS);
  QVERIFY(s.occurrence() == MyMoneySchedule::OCCUR_EVERYEIGHTWEEKS);
  QVERIFY(s.occurrencePeriod() == MyMoneySchedule::OCCUR_WEEKLY);
  QVERIFY(s.occurrenceMultiplier() == 8);

  s.setOccurrence(MyMoneySchedule::OCCUR_EVERYHALFMONTH);
  QVERIFY(s.occurrence() == MyMoneySchedule::OCCUR_EVERYHALFMONTH);
  QVERIFY(s.occurrencePeriod() == MyMoneySchedule::OCCUR_EVERYHALFMONTH);
  QVERIFY(s.occurrenceMultiplier() == 1);

  s.setOccurrence(MyMoneySchedule::OCCUR_MONTHLY);
  QVERIFY(s.occurrence() == MyMoneySchedule::OCCUR_MONTHLY);
  QVERIFY(s.occurrencePeriod() == MyMoneySchedule::OCCUR_MONTHLY);
  QVERIFY(s.occurrenceMultiplier() == 1);
  s.setOccurrence(MyMoneySchedule::OCCUR_EVERYOTHERMONTH);
  QVERIFY(s.occurrence() == MyMoneySchedule::OCCUR_EVERYOTHERMONTH);
  QVERIFY(s.occurrencePeriod() == MyMoneySchedule::OCCUR_MONTHLY);
  QVERIFY(s.occurrenceMultiplier() == 2);
  s.setOccurrence(MyMoneySchedule::OCCUR_EVERYTHREEMONTHS);
  QVERIFY(s.occurrence() == MyMoneySchedule::OCCUR_EVERYTHREEMONTHS);
  QVERIFY(s.occurrencePeriod() == MyMoneySchedule::OCCUR_MONTHLY);
  QVERIFY(s.occurrenceMultiplier() == 3);
  // Quarterly no longer used.  Every three months used instead
  s.setOccurrence(MyMoneySchedule::OCCUR_QUARTERLY);
  QVERIFY(s.occurrence() == MyMoneySchedule::OCCUR_EVERYTHREEMONTHS);
  QVERIFY(s.occurrencePeriod() == MyMoneySchedule::OCCUR_MONTHLY);
  QVERIFY(s.occurrenceMultiplier() == 3);
  s.setOccurrence(MyMoneySchedule::OCCUR_EVERYFOURMONTHS);
  QVERIFY(s.occurrence() == MyMoneySchedule::OCCUR_EVERYFOURMONTHS);
  QVERIFY(s.occurrencePeriod() == MyMoneySchedule::OCCUR_MONTHLY);
  QVERIFY(s.occurrenceMultiplier() == 4);
  s.setOccurrence(MyMoneySchedule::OCCUR_TWICEYEARLY);
  QVERIFY(s.occurrence() == MyMoneySchedule::OCCUR_TWICEYEARLY);
  QVERIFY(s.occurrencePeriod() == MyMoneySchedule::OCCUR_MONTHLY);
  QVERIFY(s.occurrenceMultiplier() == 6);

  s.setOccurrence(MyMoneySchedule::OCCUR_YEARLY);
  QVERIFY(s.occurrence() == MyMoneySchedule::OCCUR_YEARLY);
  QVERIFY(s.occurrencePeriod() == MyMoneySchedule::OCCUR_YEARLY);
  QVERIFY(s.occurrenceMultiplier() == 1);
  s.setOccurrence(MyMoneySchedule::OCCUR_EVERYOTHERYEAR);
  QVERIFY(s.occurrence() == MyMoneySchedule::OCCUR_EVERYOTHERYEAR);
  QVERIFY(s.occurrencePeriod() == MyMoneySchedule::OCCUR_YEARLY);
  QVERIFY(s.occurrenceMultiplier() == 2);
}

void MyMoneyScheduleTest::testSimpleToFromCompoundOccurrence()
{
  // Conversion between Simple and Compound occurrences
  // Each simple occurrence to compound occurrence
  MyMoneySchedule::occurrenceE occ;
  int mult;
  occ = MyMoneySchedule::OCCUR_ONCE; mult = 1;
  MyMoneySchedule::simpleToCompoundOccurrence(mult, occ);
  QVERIFY(occ == MyMoneySchedule::OCCUR_ONCE && mult == 1);
  occ = MyMoneySchedule::OCCUR_DAILY; mult = 1;
  MyMoneySchedule::simpleToCompoundOccurrence(mult, occ);
  QVERIFY(occ == MyMoneySchedule::OCCUR_DAILY && mult == 1);
  occ = MyMoneySchedule::OCCUR_WEEKLY; mult = 1;
  MyMoneySchedule::simpleToCompoundOccurrence(mult, occ);
  QVERIFY(occ == MyMoneySchedule::OCCUR_WEEKLY && mult == 1);
  occ = MyMoneySchedule::OCCUR_EVERYOTHERWEEK; mult = 1;
  MyMoneySchedule::simpleToCompoundOccurrence(mult, occ);
  QVERIFY(occ == MyMoneySchedule::OCCUR_WEEKLY && mult == 2);
  occ = MyMoneySchedule::OCCUR_FORTNIGHTLY; mult = 1;
  MyMoneySchedule::simpleToCompoundOccurrence(mult, occ);
  QVERIFY(occ == MyMoneySchedule::OCCUR_WEEKLY && mult == 2);
  occ = MyMoneySchedule::OCCUR_EVERYHALFMONTH; mult = 1;
  MyMoneySchedule::simpleToCompoundOccurrence(mult, occ);
  QVERIFY(occ == MyMoneySchedule::OCCUR_EVERYHALFMONTH && mult == 1);
  occ = MyMoneySchedule::OCCUR_EVERYTHREEWEEKS; mult = 1;
  MyMoneySchedule::simpleToCompoundOccurrence(mult, occ);
  QVERIFY(occ == MyMoneySchedule::OCCUR_WEEKLY && mult == 3);
  occ = MyMoneySchedule::OCCUR_EVERYFOURWEEKS; mult = 1;
  MyMoneySchedule::simpleToCompoundOccurrence(mult, occ);
  QVERIFY(occ == MyMoneySchedule::OCCUR_WEEKLY && mult == 4);
  occ = MyMoneySchedule::OCCUR_EVERYTHIRTYDAYS; mult = 1;
  MyMoneySchedule::simpleToCompoundOccurrence(mult, occ);
  QVERIFY(occ == MyMoneySchedule::OCCUR_DAILY && mult == 30);
  occ = MyMoneySchedule::OCCUR_MONTHLY; mult = 1;
  MyMoneySchedule::simpleToCompoundOccurrence(mult, occ);
  QVERIFY(occ == MyMoneySchedule::OCCUR_MONTHLY && mult == 1);
  occ = MyMoneySchedule::OCCUR_EVERYEIGHTWEEKS; mult = 1;
  MyMoneySchedule::simpleToCompoundOccurrence(mult, occ);
  QVERIFY(occ == MyMoneySchedule::OCCUR_WEEKLY && mult == 8);
  occ = MyMoneySchedule::OCCUR_EVERYOTHERMONTH; mult = 1;
  MyMoneySchedule::simpleToCompoundOccurrence(mult, occ);
  QVERIFY(occ == MyMoneySchedule::OCCUR_MONTHLY && mult == 2);
  occ = MyMoneySchedule::OCCUR_EVERYTHREEMONTHS; mult = 1;
  MyMoneySchedule::simpleToCompoundOccurrence(mult, occ);
  QVERIFY(occ == MyMoneySchedule::OCCUR_MONTHLY && mult == 3);
  occ = MyMoneySchedule::OCCUR_QUARTERLY; mult = 1;
  MyMoneySchedule::simpleToCompoundOccurrence(mult, occ);
  QVERIFY(occ == MyMoneySchedule::OCCUR_MONTHLY && mult == 3);
  occ = MyMoneySchedule::OCCUR_EVERYFOURMONTHS; mult = 1;
  MyMoneySchedule::simpleToCompoundOccurrence(mult, occ);
  QVERIFY(occ == MyMoneySchedule::OCCUR_MONTHLY && mult == 4);
  occ = MyMoneySchedule::OCCUR_TWICEYEARLY; mult = 1;
  MyMoneySchedule::simpleToCompoundOccurrence(mult, occ);
  QVERIFY(occ == MyMoneySchedule::OCCUR_MONTHLY && mult == 6);
  occ = MyMoneySchedule::OCCUR_YEARLY; mult = 1;
  MyMoneySchedule::simpleToCompoundOccurrence(mult, occ);
  QVERIFY(occ == MyMoneySchedule::OCCUR_YEARLY && mult == 1);
  occ = MyMoneySchedule::OCCUR_EVERYOTHERYEAR; mult = 1;
  MyMoneySchedule::simpleToCompoundOccurrence(mult, occ);
  QVERIFY(occ == MyMoneySchedule::OCCUR_YEARLY && mult == 2);
  // Compound to Simple Occurrences
  occ = MyMoneySchedule::OCCUR_ONCE; mult = 1;
  MyMoneySchedule::compoundToSimpleOccurrence(mult, occ);
  QVERIFY(occ == MyMoneySchedule::OCCUR_ONCE && mult == 1);
  occ = MyMoneySchedule::OCCUR_DAILY; mult = 1;
  MyMoneySchedule::compoundToSimpleOccurrence(mult, occ);
  QVERIFY(occ == MyMoneySchedule::OCCUR_DAILY && mult == 1);
  occ = MyMoneySchedule::OCCUR_WEEKLY; mult = 1;
  MyMoneySchedule::compoundToSimpleOccurrence(mult, occ);
  QVERIFY(occ == MyMoneySchedule::OCCUR_WEEKLY && mult == 1);
  occ = MyMoneySchedule::OCCUR_WEEKLY; mult = 2;
  MyMoneySchedule::compoundToSimpleOccurrence(mult, occ);
  QVERIFY(occ == MyMoneySchedule::OCCUR_EVERYOTHERWEEK && mult == 1);
  // MyMoneySchedule::OCCUR_FORTNIGHTLY not converted back
  occ = MyMoneySchedule::OCCUR_EVERYHALFMONTH; mult = 1;
  MyMoneySchedule::compoundToSimpleOccurrence(mult, occ);
  QVERIFY(occ == MyMoneySchedule::OCCUR_EVERYHALFMONTH && mult == 1);
  occ = MyMoneySchedule::OCCUR_WEEKLY; mult = 3;
  MyMoneySchedule::compoundToSimpleOccurrence(mult, occ);
  QVERIFY(occ == MyMoneySchedule::OCCUR_EVERYTHREEWEEKS && mult == 1);
  occ = MyMoneySchedule::OCCUR_WEEKLY ; mult = 4;
  MyMoneySchedule::compoundToSimpleOccurrence(mult, occ);
  QVERIFY(occ == MyMoneySchedule::OCCUR_EVERYFOURWEEKS && mult == 1);
  occ = MyMoneySchedule::OCCUR_DAILY; mult = 30;
  MyMoneySchedule::compoundToSimpleOccurrence(mult, occ);
  QVERIFY(occ == MyMoneySchedule::OCCUR_EVERYTHIRTYDAYS && mult == 1);
  occ = MyMoneySchedule::OCCUR_MONTHLY; mult = 1;
  MyMoneySchedule::compoundToSimpleOccurrence(mult, occ);
  QVERIFY(occ == MyMoneySchedule::OCCUR_MONTHLY && mult == 1);
  occ = MyMoneySchedule::OCCUR_WEEKLY; mult = 8;
  MyMoneySchedule::compoundToSimpleOccurrence(mult, occ);
  QVERIFY(occ == MyMoneySchedule::OCCUR_EVERYEIGHTWEEKS && mult == 1);
  occ = MyMoneySchedule::OCCUR_MONTHLY; mult = 2;
  MyMoneySchedule::compoundToSimpleOccurrence(mult, occ);
  QVERIFY(occ == MyMoneySchedule::OCCUR_EVERYOTHERMONTH && mult == 1);
  occ = MyMoneySchedule::OCCUR_MONTHLY; mult = 3;
  MyMoneySchedule::compoundToSimpleOccurrence(mult, occ);
  QVERIFY(occ == MyMoneySchedule::OCCUR_EVERYTHREEMONTHS && mult == 1);
  // MyMoneySchedule::OCCUR_QUARTERLY not converted back
  occ = MyMoneySchedule::OCCUR_MONTHLY; mult = 4;
  MyMoneySchedule::compoundToSimpleOccurrence(mult, occ);
  QVERIFY(occ == MyMoneySchedule::OCCUR_EVERYFOURMONTHS && mult == 1);
  occ = MyMoneySchedule::OCCUR_MONTHLY; mult = 6;
  MyMoneySchedule::compoundToSimpleOccurrence(mult, occ);
  QVERIFY(occ == MyMoneySchedule::OCCUR_TWICEYEARLY && mult == 1);
  occ = MyMoneySchedule::OCCUR_YEARLY; mult = 1;
  MyMoneySchedule::compoundToSimpleOccurrence(mult, occ);
  QVERIFY(occ == MyMoneySchedule::OCCUR_YEARLY && mult == 1);
  occ = MyMoneySchedule::OCCUR_YEARLY; mult = 2;
  MyMoneySchedule::compoundToSimpleOccurrence(mult, occ);
  QVERIFY(occ == MyMoneySchedule::OCCUR_EVERYOTHERYEAR && mult == 1);
}

void MyMoneyScheduleTest::testProcessingDates()
{
  // There should be no processing calendar defined so
  // make sure fall back works

  MyMoneySchedule s;
  // Check there is no processing caledar defined.
  QVERIFY(s.processingCalendar() == 0);
  // This should be a processing day.
  QVERIFY(s.isProcessingDate(QDate(2009, 12, 31)));
  // This should be a processing day when there is no calendar.
  QVERIFY(s.isProcessingDate(QDate(2010, 1, 1)));
  // This should be a non-processing day as it is on a weekend.
  QVERIFY(!s.isProcessingDate(QDate(2010, 1, 2)));
}

void MyMoneyScheduleTest::testPaidEarlyOneTime()
{
// this tries to figure out what's wrong with
// https://bugs.kde.org/show_bug.cgi?id=231029

  MyMoneySchedule sch;
  QDate paymentInFuture = QDate::currentDate().addDays(7);

  QString ref_ok = QString(
                     "<!DOCTYPE TEST>\n"
                     "<SCHEDULE-CONTAINER>\n"
                     " <SCHEDULED_TX startDate=\"%1\" autoEnter=\"0\" weekendOption=\"1\" lastPayment=\"%2\" paymentType=\"2\" endDate=\"%3\" type=\"4\" id=\"SCH0042\" name=\"A Name\" fixed=\"1\" occurenceMultiplier=\"1\" occurence=\"32\" >\n" // krazy:exclude=spelling
                     "  <PAYMENTS/>\n"
                     "  <TRANSACTION postdate=\"\" memo=\"\" id=\"\" commodity=\"GBP\" entrydate=\"\" >\n"
                     "   <SPLITS>\n"
                     "    <SPLIT payee=\"P000001\" reconciledate=\"\" shares=\"96379/100\" action=\"Transfer\" number=\"\" reconcileflag=\"2\" memo=\"\" value=\"96379/100\" id=\"S0001\" account=\"A000076\" />\n"
                     "    <SPLIT payee=\"P000001\" reconciledate=\"\" shares=\"-96379/100\" action=\"Transfer\" number=\"\" reconcileflag=\"2\" memo=\"\" value=\"-96379/100\" id=\"S0002\" account=\"A000276\" />\n"
                     "   </SPLITS>\n"
                     "  </TRANSACTION>\n"
                     " </SCHEDULED_TX>\n"
                     "</SCHEDULE-CONTAINER>\n"
                   ).arg(paymentInFuture.toString(Qt::ISODate))
                   .arg(paymentInFuture.toString(Qt::ISODate))
                   .arg(paymentInFuture.toString(Qt::ISODate));

  QDomDocument doc;
  QDomElement node;
  doc.setContent(ref_ok);
  node = doc.documentElement().firstChild().toElement();

  try {
    sch = MyMoneySchedule(node);
    QVERIFY(sch.isFinished() == true);
    QVERIFY(sch.occurrencePeriod() == MyMoneySchedule::OCCUR_MONTHLY);
    QVERIFY(sch.paymentDates(QDate::currentDate(), QDate::currentDate().addDays(21)).count() == 0);
  } catch (MyMoneyException *e) {
    delete e;
    QFAIL("Unexpected exception");
  }

}

void MyMoneyScheduleTest::testAdjustedNextPayment()
{
  MyMoneySchedule s;

  QDate dueDate(2010, 5, 23);
  QDate adjustedDueDate(2010, 5, 21);
  s.setNextDueDate(dueDate);
  s.setOccurrence(MyMoneySchedule::OCCUR_MONTHLY);
  s.setWeekendOption(MyMoneySchedule::MoveBefore);

  //if adjustedNextPayment works ok with adjusted date prior to the current date, it should return 2010-06-23
  QDate nextDueDate(2010, 6, 23);
  //this is the current behaviour, and it is wrong
  //QVERIFY(s.adjustedNextPayment(adjustedDueDate) == adjustedDueDate);

  //this is the expected behaviour
  QVERIFY(s.adjustedNextPayment(s.adjustedNextDueDate()) == s.adjustedDate(nextDueDate, s.weekendOption()));
}
#include "mymoneyscheduletest.moc"

