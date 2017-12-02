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

#include "mymoneyschedule-test.h"

#include <QList>
#include <QtTest>
#include <QDomDocument>
#include <QDomElement>

#define KMM_MYMONEY_UNIT_TESTABLE friend class MyMoneyScheduleTest;

#include "mymoneysplit.h"
#include "mymoneymoney.h"
#include "mymoneyschedule.h"
#include "mymoneyschedule_p.h"
#include "mymoneyfile.h"
#include "mymoneytransaction.h"
#include "mymoneytransaction_p.h"
#include "storage/mymoneyseqaccessmgr.h"

QTEST_GUILESS_MAIN(MyMoneyScheduleTest)

using namespace eMyMoney;

void MyMoneyScheduleTest::testEmptyConstructor()
{
  MyMoneySchedule s;

  QCOMPARE(s.id().isEmpty(), true);
  QCOMPARE(s.occurrence(), Schedule::Occurrence::Any);
  QCOMPARE(s.type(), Schedule::Type::Any);
  QCOMPARE(s.paymentType(), Schedule::PaymentType::Any);
  QCOMPARE(s.isFinished(), false);
  QCOMPARE(!s.startDate().isValid(), true);
  QCOMPARE(!s.endDate().isValid(), true);
  QCOMPARE(!s.lastPayment().isValid(), true);
  QCOMPARE(s.autoEnter(), false);
  QCOMPARE(s.name().isEmpty(), true);
  QCOMPARE(s.willEnd(), false);
  QCOMPARE(s.lastDayInMonth(), false);
}

void MyMoneyScheduleTest::testConstructor()
{
  MyMoneySchedule s("A Name",
                    Schedule::Type::Bill,
                    Schedule::Occurrence::Weekly, 1,
                    Schedule::PaymentType::DirectDebit,
                    QDate::currentDate(),
                    QDate(),
                    true,
                    true);

  QCOMPARE(s.type(), Schedule::Type::Bill);
  QCOMPARE(s.occurrence(), Schedule::Occurrence::Weekly);
  QCOMPARE(s.occurrenceMultiplier(), 1);
  QCOMPARE(s.paymentType(), Schedule::PaymentType::DirectDebit);
  QCOMPARE(s.startDate(), QDate());
  QCOMPARE(s.willEnd(), false);
  QCOMPARE(s.isFixed(), true);
  QCOMPARE(s.autoEnter(), true);
  QCOMPARE(s.name(), QLatin1String("A Name"));
  QCOMPARE(!s.endDate().isValid(), true);
  QCOMPARE(!s.lastPayment().isValid(), true);
}

void MyMoneyScheduleTest::testSetFunctions()
{
  MyMoneySchedule s;

  s.d_func()->setId("SCHED001");
  QCOMPARE(s.id(), QLatin1String("SCHED001"));

  s.setType(Schedule::Type::Bill);
  QCOMPARE(s.type(), Schedule::Type::Bill);

  s.setEndDate(QDate::currentDate());
  QCOMPARE(s.endDate(), QDate::currentDate());
  QCOMPARE(s.willEnd(), true);
}

void MyMoneyScheduleTest::testCopyConstructor()
{
  MyMoneySchedule s;

  s.d_func()->setId("SCHED001");
  s.setType(Schedule::Type::Bill);

  MyMoneySchedule s2(s);

  QCOMPARE(s.id(), s2.id());
  QCOMPARE(s.type(), s2.type());
}

void MyMoneyScheduleTest::testAssignmentConstructor()
{
  MyMoneySchedule s;

  s.d_func()->setId("SCHED001");
  s.setType(Schedule::Type::Bill);

  MyMoneySchedule s2 = s;

  QCOMPARE(s.id(), s2.id());
  QCOMPARE(s.type(), s2.type());
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

    QCOMPARE(sch_overdue.isOverdue(), true);
    QCOMPARE(sch_intime.isOverdue(), false);

  } catch (const MyMoneyException &) {
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
    QCOMPARE(sch.nextPayment(QDate(2007, 2, 14)), QDate(2007, 2, 17));
    QCOMPARE(sch.nextPayment(QDate(2007, 2, 17)), QDate(2008, 2, 17));
    QCOMPARE(sch.nextPayment(QDate(2007, 2, 18)), QDate(2008, 2, 17));

  } catch (const MyMoneyException &) {
    QFAIL("Unexpected exception");
  }
}

void MyMoneyScheduleTest::testNextPaymentOnLastDayOfMonth()
{
  MyMoneySchedule sch;
  QString future_sched = QString(
                           "<!DOCTYPE TEST>\n"
                           "<SCHEDULE-CONTAINER>\n"
                           "<SCHEDULED_TX startDate=\"2014-10-31\" autoEnter=\"1\" weekendOption=\"2\" lastPayment=\"\" paymentType=\"1\" endDate=\"\" type=\"1\" id=\"SCH000058\" name=\"Car Tax\" fixed=\"1\" occurenceMultiplier=\"1\" occurence=\"32\" >\n" // krazy:exclude=spelling
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
    QDate nextPayment;

    // check for the first payment to happen
    nextPayment = sch.nextPayment(QDate(2014, 10, 1));
    QCOMPARE(nextPayment, QDate(2014, 10, 31));
    sch.setLastPayment(nextPayment);

    QCOMPARE(sch.nextPayment(QDate(2014, 11, 1)), QDate(2014, 11, 30));
    QCOMPARE(sch.nextPayment(QDate(2014, 12, 1)), QDate(2014, 12, 31));
    QCOMPARE(sch.nextPayment(QDate(2015, 1, 1)), QDate(2015, 1, 31));
    QCOMPARE(sch.nextPayment(QDate(2015, 2, 1)), QDate(2015, 2, 28));
    QCOMPARE(sch.nextPayment(QDate(2015, 3, 1)), QDate(2015, 3, 31));

    // now check that we also cover leap years
    QCOMPARE(sch.nextPayment(QDate(2016, 2, 1)), QDate(2016, 2, 29));
    QCOMPARE(sch.nextPayment(QDate(2016, 3, 1)), QDate(2016, 3, 31));

  } catch (const MyMoneyException &) {
    QFAIL("Unexpected exception");
  }
}

void MyMoneyScheduleTest::testAddHalfMonths()
{
  // addHalfMonths is private
  // Test a Schedule with occurrence EveryHalfMonth using nextPayment
  MyMoneySchedule s;
  s.setStartDate(QDate(2007, 1, 1));
  s.setOccurrence(Schedule::Occurrence::EveryHalfMonth);
  s.setNextDueDate(s.startDate());
  s.setLastPayment(s.startDate());

  QString format("yyyy-MM-dd");
  QCOMPARE(s.nextPayment(s.nextDueDate()).toString(format), QLatin1String("2007-01-16"));
  s.setNextDueDate(QDate(2007, 1, 2));
  QCOMPARE(s.nextPayment(s.nextDueDate()).toString(format), QLatin1String("2007-01-17"));
  s.setNextDueDate(QDate(2007, 1, 3));
  QCOMPARE(s.nextPayment(s.nextDueDate()).toString(format), QLatin1String("2007-01-18"));
  s.setNextDueDate(QDate(2007, 1, 4));
  QCOMPARE(s.nextPayment(s.nextDueDate()).toString(format), QLatin1String("2007-01-19"));
  s.setNextDueDate(QDate(2007, 1, 5));
  QCOMPARE(s.nextPayment(s.nextDueDate()).toString(format), QLatin1String("2007-01-20"));
  s.setNextDueDate(QDate(2007, 1, 6));
  QCOMPARE(s.nextPayment(s.nextDueDate()).toString(format), QLatin1String("2007-01-21"));
  s.setNextDueDate(QDate(2007, 1, 7));
  QCOMPARE(s.nextPayment(s.nextDueDate()).toString(format), QLatin1String("2007-01-22"));
  s.setNextDueDate(QDate(2007, 1, 8));
  QCOMPARE(s.nextPayment(s.nextDueDate()).toString(format), QLatin1String("2007-01-23"));
  s.setNextDueDate(QDate(2007, 1, 9));
  QCOMPARE(s.nextPayment(s.nextDueDate()).toString(format), QLatin1String("2007-01-24"));
  s.setNextDueDate(QDate(2007, 1, 10));
  QCOMPARE(s.nextPayment(s.nextDueDate()).toString(format), QLatin1String("2007-01-25"));
  s.setNextDueDate(QDate(2007, 1, 11));
  QCOMPARE(s.nextPayment(s.nextDueDate()).toString(format), QLatin1String("2007-01-26"));
  s.setNextDueDate(QDate(2007, 1, 12));
  QCOMPARE(s.nextPayment(s.nextDueDate()).toString(format), QLatin1String("2007-01-27"));
  s.setNextDueDate(QDate(2007, 1, 13));
  QCOMPARE(s.nextPayment(s.nextDueDate()).toString(format), QLatin1String("2007-01-28"));
  s.setNextDueDate(QDate(2007, 1, 14));
  QCOMPARE(s.nextPayment(s.nextDueDate()).toString(format), QLatin1String("2007-01-29"));
  // 15 -> Last Day
  s.setNextDueDate(QDate(2007, 1, 15));
  QCOMPARE(s.nextPayment(s.nextDueDate()).toString(format), QLatin1String("2007-01-31"));
  s.setNextDueDate(QDate(2007, 1, 16));
  QCOMPARE(s.nextPayment(s.nextDueDate()).toString(format), QLatin1String("2007-02-01"));
  s.setNextDueDate(QDate(2007, 1, 17));
  QCOMPARE(s.nextPayment(s.nextDueDate()).toString(format), QLatin1String("2007-02-02"));
  s.setNextDueDate(QDate(2007, 1, 18));
  QCOMPARE(s.nextPayment(s.nextDueDate()).toString(format), QLatin1String("2007-02-03"));
  s.setNextDueDate(QDate(2007, 1, 19));
  QCOMPARE(s.nextPayment(s.nextDueDate()).toString(format), QLatin1String("2007-02-04"));
  s.setNextDueDate(QDate(2007, 1, 20));
  QCOMPARE(s.nextPayment(s.nextDueDate()).toString(format), QLatin1String("2007-02-05"));
  s.setNextDueDate(QDate(2007, 1, 21));
  QCOMPARE(s.nextPayment(s.nextDueDate()).toString(format), QLatin1String("2007-02-06"));
  s.setNextDueDate(QDate(2007, 1, 22));
  QCOMPARE(s.nextPayment(s.nextDueDate()).toString(format), QLatin1String("2007-02-07"));
  s.setNextDueDate(QDate(2007, 1, 23));
  QCOMPARE(s.nextPayment(s.nextDueDate()).toString(format), QLatin1String("2007-02-08"));
  s.setNextDueDate(QDate(2007, 1, 24));
  QCOMPARE(s.nextPayment(s.nextDueDate()).toString(format), QLatin1String("2007-02-09"));
  s.setNextDueDate(QDate(2007, 1, 25));
  QCOMPARE(s.nextPayment(s.nextDueDate()).toString(format), QLatin1String("2007-02-10"));
  s.setNextDueDate(QDate(2007, 1, 26));
  QCOMPARE(s.nextPayment(s.nextDueDate()).toString(format), QLatin1String("2007-02-11"));
  s.setNextDueDate(QDate(2007, 1, 27));
  QCOMPARE(s.nextPayment(s.nextDueDate()).toString(format), QLatin1String("2007-02-12"));
  s.setNextDueDate(QDate(2007, 1, 28));
  QCOMPARE(s.nextPayment(s.nextDueDate()).toString(format), QLatin1String("2007-02-13"));
  s.setNextDueDate(QDate(2007, 1, 29));
  QCOMPARE(s.nextPayment(s.nextDueDate()).toString(format), QLatin1String("2007-02-14"));
  // 30th,31st -> 15th
  s.setNextDueDate(QDate(2007, 1, 30));
  QCOMPARE(s.nextPayment(s.nextDueDate()).toString(format), QLatin1String("2007-02-15"));
  s.setNextDueDate(QDate(2007, 1, 31));
  QCOMPARE(s.nextPayment(s.nextDueDate()).toString(format), QLatin1String("2007-02-15"));
  // 30th (last day)
  s.setNextDueDate(QDate(2007, 4, 30));
  QCOMPARE(s.nextPayment(s.nextDueDate()).toString(format), QLatin1String("2007-05-15"));
  // 28th of February (Last day): to 15th
  s.setNextDueDate(QDate(1900, 2, 28));
  QCOMPARE(s.nextPayment(s.nextDueDate()).toString(format), QLatin1String("1900-03-15"));
  // 28th of February (Leap year): to 13th
  s.setNextDueDate(QDate(2000, 2, 28));
  QCOMPARE(s.nextPayment(s.nextDueDate()).toString(format), QLatin1String("2000-03-13"));
  // 29th of February (Leap year)
  s.setNextDueDate(QDate(2000, 2, 29));
  QCOMPARE(s.nextPayment(s.nextDueDate()).toString(format), QLatin1String("2000-03-15"));
  // Add multiple transactions
  s.setStartDate(QDate(2007, 1, 1));
  QCOMPARE(s.dateAfter(2).toString(format), QLatin1String("2007-01-16"));
  QCOMPARE(s.dateAfter(3).toString(format), QLatin1String("2007-02-01"));
  QCOMPARE(s.dateAfter(4).toString(format), QLatin1String("2007-02-16"));
  s.setStartDate(QDate(2007, 1, 12));
  QCOMPARE(s.dateAfter(2).toString(format), QLatin1String("2007-01-27"));
  QCOMPARE(s.dateAfter(3).toString(format), QLatin1String("2007-02-12"));
  QCOMPARE(s.dateAfter(4).toString(format), QLatin1String("2007-02-27"));
  QCOMPARE(s.dateAfter(5).toString(format), QLatin1String("2007-03-12"));
  s.setStartDate(QDate(2007, 1, 13));
  QCOMPARE(s.dateAfter(2).toString(format), QLatin1String("2007-01-28"));
  QCOMPARE(s.dateAfter(3).toString(format), QLatin1String("2007-02-13"));
  QCOMPARE(s.dateAfter(4).toString(format), QLatin1String("2007-02-28"));
  QCOMPARE(s.dateAfter(5).toString(format), QLatin1String("2007-03-15"));
  s.setStartDate(QDate(2007, 1, 14));
  QCOMPARE(s.dateAfter(2).toString(format), QLatin1String("2007-01-29"));
  QCOMPARE(s.dateAfter(3).toString(format), QLatin1String("2007-02-14"));
  QCOMPARE(s.dateAfter(4).toString(format), QLatin1String("2007-02-28"));
  QCOMPARE(s.dateAfter(5).toString(format), QLatin1String("2007-03-15"));
  s.setStartDate(QDate(2007, 1, 15));
  QCOMPARE(s.dateAfter(2).toString(format), QLatin1String("2007-01-31"));
  QCOMPARE(s.dateAfter(3).toString(format), QLatin1String("2007-02-15"));
  QCOMPARE(s.dateAfter(4).toString(format), QLatin1String("2007-02-28"));
  QCOMPARE(s.dateAfter(5).toString(format), QLatin1String("2007-03-15"));
  s.setStartDate(QDate(2007, 1, 16));
  QCOMPARE(s.dateAfter(2).toString(format), QLatin1String("2007-02-01"));
  QCOMPARE(s.dateAfter(3).toString(format), QLatin1String("2007-02-16"));
  QCOMPARE(s.dateAfter(4).toString(format), QLatin1String("2007-03-01"));
  QCOMPARE(s.dateAfter(5).toString(format), QLatin1String("2007-03-16"));
  s.setStartDate(QDate(2007, 1, 27));
  QCOMPARE(s.dateAfter(2).toString(format), QLatin1String("2007-02-12"));
  QCOMPARE(s.dateAfter(3).toString(format), QLatin1String("2007-02-27"));
  QCOMPARE(s.dateAfter(4).toString(format), QLatin1String("2007-03-12"));
  QCOMPARE(s.dateAfter(5).toString(format), QLatin1String("2007-03-27"));
  s.setStartDate(QDate(2007, 1, 28));
  QCOMPARE(s.dateAfter(2).toString(format), QLatin1String("2007-02-13"));
  QCOMPARE(s.dateAfter(3).toString(format), QLatin1String("2007-02-28"));
  QCOMPARE(s.dateAfter(4).toString(format), QLatin1String("2007-03-15"));
  QCOMPARE(s.dateAfter(5).toString(format), QLatin1String("2007-03-31"));
  s.setStartDate(QDate(2007, 1, 29));
  QCOMPARE(s.dateAfter(2).toString(format), QLatin1String("2007-02-14"));
  QCOMPARE(s.dateAfter(3).toString(format), QLatin1String("2007-02-28"));
  QCOMPARE(s.dateAfter(4).toString(format), QLatin1String("2007-03-15"));
  QCOMPARE(s.dateAfter(5).toString(format), QLatin1String("2007-03-31"));
  s.setStartDate(QDate(2007, 1, 30));
  QCOMPARE(s.dateAfter(2).toString(format), QLatin1String("2007-02-15"));
  QCOMPARE(s.dateAfter(3).toString(format), QLatin1String("2007-02-28"));
  QCOMPARE(s.dateAfter(4).toString(format), QLatin1String("2007-03-15"));
  QCOMPARE(s.dateAfter(5).toString(format), QLatin1String("2007-03-31"));
  s.setStartDate(QDate(2007, 1, 31));
  QCOMPARE(s.dateAfter(2).toString(format), QLatin1String("2007-02-15"));
  QCOMPARE(s.dateAfter(3).toString(format), QLatin1String("2007-02-28"));
  QCOMPARE(s.dateAfter(4).toString(format), QLatin1String("2007-03-15"));
  QCOMPARE(s.dateAfter(5).toString(format), QLatin1String("2007-03-31"));
  s.setStartDate(QDate(2007, 4, 29));
  QCOMPARE(s.dateAfter(2).toString(format), QLatin1String("2007-05-14"));
  QCOMPARE(s.dateAfter(3).toString(format), QLatin1String("2007-05-29"));
  QCOMPARE(s.dateAfter(4).toString(format), QLatin1String("2007-06-14"));
  QCOMPARE(s.dateAfter(5).toString(format), QLatin1String("2007-06-29"));
  s.setStartDate(QDate(2007, 4, 30));
  QCOMPARE(s.dateAfter(2).toString(format), QLatin1String("2007-05-15"));
  QCOMPARE(s.dateAfter(3).toString(format), QLatin1String("2007-05-31"));
  QCOMPARE(s.dateAfter(4).toString(format), QLatin1String("2007-06-15"));
  QCOMPARE(s.dateAfter(5).toString(format), QLatin1String("2007-06-30"));
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
    QCOMPARE(list.count(), 3);
    QCOMPARE(list[0], QDate(2006, 2, 28));
    QCOMPARE(list[1], QDate(2006, 3, 31));
    // Would fall on a Sunday so gets moved back to 28th.
    QCOMPARE(list[2], QDate(2006, 4, 28));

    // Add tests for each possible occurrence.
    // Check how paymentDates is meant to work
    // Build a list of expected dates and compare
    // Schedule::Occurrence::Once
    sch.setOccurrence(Schedule::Occurrence::Once);
    startDate.setDate(2009, 1, 1);
    endDate.setDate(2009, 12, 31);
    sch.setStartDate(startDate);
    sch.setNextDueDate(startDate);
    list = sch.paymentDates(startDate, endDate);
    QCOMPARE(list.count(), 1);
    QCOMPARE(list[0], QDate(2009, 1, 1));
    // Schedule::Occurrence::Daily
    sch.setOccurrence(Schedule::Occurrence::Daily);
    startDate.setDate(2009, 1, 1);
    endDate.setDate(2009, 1, 5);
    sch.setStartDate(startDate);
    sch.setNextDueDate(startDate);
    list = sch.paymentDates(startDate, endDate);
    QCOMPARE(list.count(), 5);
    QCOMPARE(list[0], QDate(2009, 1, 1));
    QCOMPARE(list[1], QDate(2009, 1, 2));
    // Would fall on Saturday so gets moved to 2nd.
    QCOMPARE(list[2], QDate(2009, 1, 2));
    // Would fall on Sunday so gets moved to 2nd.
    QCOMPARE(list[3], QDate(2009, 1, 2));
    QCOMPARE(list[4], QDate(2009, 1, 5));
    // Schedule::Occurrence::Daily with multiplier 2
    sch.setOccurrenceMultiplier(2);
    list = sch.paymentDates(startDate.addDays(1), endDate);
    QCOMPARE(list.count(), 2);
    // Would fall on Sunday so gets moved to 2nd.
    QCOMPARE(list[0], QDate(2009, 1, 2));
    QCOMPARE(list[1], QDate(2009, 1, 5));
    sch.setOccurrenceMultiplier(1);
    // Schedule::Occurrence::Weekly
    sch.setOccurrence(Schedule::Occurrence::Weekly);
    startDate.setDate(2009, 1, 6);
    endDate.setDate(2009, 2, 4);
    sch.setStartDate(startDate);
    sch.setNextDueDate(startDate);
    list = sch.paymentDates(startDate, endDate);
    QCOMPARE(list.count(), 5);
    QCOMPARE(list[0], QDate(2009, 1, 6));
    QCOMPARE(list[1], QDate(2009, 1, 13));
    QCOMPARE(list[2], QDate(2009, 1, 20));
    QCOMPARE(list[3], QDate(2009, 1, 27));
    QCOMPARE(list[4], QDate(2009, 2, 3));
    // Schedule::Occurrence::EveryOtherWeek
    sch.setOccurrence(Schedule::Occurrence::EveryOtherWeek);
    startDate.setDate(2009, 2, 5);
    endDate.setDate(2009, 4, 3);
    sch.setStartDate(startDate);
    sch.setNextDueDate(startDate);
    list = sch.paymentDates(startDate, endDate);
    QCOMPARE(list.count(), 5);
    QCOMPARE(list[0], QDate(2009, 2, 5));
    QCOMPARE(list[1], QDate(2009, 2, 19));
    QCOMPARE(list[2], QDate(2009, 3, 5));
    QCOMPARE(list[3], QDate(2009, 3, 19));
    QCOMPARE(list[4], QDate(2009, 4, 2));
    // Schedule::Occurrence::Fortnightly
    sch.setOccurrence(Schedule::Occurrence::Fortnightly);
    startDate.setDate(2009, 4, 4);
    endDate.setDate(2009, 5, 31);
    sch.setStartDate(startDate);
    sch.setNextDueDate(startDate);
    list = sch.paymentDates(startDate, endDate);
    QCOMPARE(list.count(), 4);
    // First one would fall on a Saturday and would get moved
    // to 3rd which is before start date so is not in list.
    // Would fall on a Saturday so gets moved to 17th.
    QCOMPARE(list[0], QDate(2009, 4, 17));
    // Would fall on a Saturday so gets moved to 1st.
    QCOMPARE(list[1], QDate(2009, 5, 1));
    // Would fall on a Saturday so gets moved to 15th.
    QCOMPARE(list[2], QDate(2009, 5, 15));
    // Would fall on a Saturday so gets moved to 29th.
    QCOMPARE(list[3], QDate(2009, 5, 29));
    // Schedule::Occurrence::EveryHalfMonth
    sch.setOccurrence(Schedule::Occurrence::EveryHalfMonth);
    startDate.setDate(2009, 6, 1);
    endDate.setDate(2009, 8, 11);
    sch.setStartDate(startDate);
    sch.setNextDueDate(startDate);
    list = sch.paymentDates(startDate, endDate);
    QCOMPARE(list.count(), 5);
    QCOMPARE(list[0], QDate(2009, 6, 1));
    QCOMPARE(list[1], QDate(2009, 6, 16));
    QCOMPARE(list[2], QDate(2009, 7, 1));
    QCOMPARE(list[3], QDate(2009, 7, 16));
    // Would fall on a Saturday so gets moved to 31st.
    QCOMPARE(list[4], QDate(2009, 7, 31));
    // Schedule::Occurrence::EveryThreeWeeks
    sch.setOccurrence(Schedule::Occurrence::EveryThreeWeeks);
    startDate.setDate(2009, 8, 12);
    endDate.setDate(2009, 11, 12);
    sch.setStartDate(startDate);
    sch.setNextDueDate(startDate);
    list = sch.paymentDates(startDate, endDate);
    QCOMPARE(list.count(), 5);
    QCOMPARE(list[0], QDate(2009, 8, 12));
    QCOMPARE(list[1], QDate(2009, 9, 2));
    QCOMPARE(list[2], QDate(2009, 9, 23));
    QCOMPARE(list[3], QDate(2009, 10, 14));
    QCOMPARE(list[4], QDate(2009, 11, 4));
    // Schedule::Occurrence::EveryFourWeeks
    sch.setOccurrence(Schedule::Occurrence::EveryFourWeeks);
    startDate.setDate(2009, 11, 13);
    endDate.setDate(2010, 3, 13);
    sch.setStartDate(startDate);
    sch.setNextDueDate(startDate);
    list = sch.paymentDates(startDate, endDate);
    QCOMPARE(list.count(), 5);
    QCOMPARE(list[0], QDate(2009, 11, 13));
    QCOMPARE(list[1], QDate(2009, 12, 11));
    QCOMPARE(list[2], QDate(2010, 1, 8));
    QCOMPARE(list[3], QDate(2010, 2, 5));
    QCOMPARE(list[4], QDate(2010, 3, 5));
    // Schedule::Occurrence::EveryThirtyDays
    sch.setOccurrence(Schedule::Occurrence::EveryThirtyDays);
    startDate.setDate(2010, 3, 19);
    endDate.setDate(2010, 7, 19);
    sch.setStartDate(startDate);
    sch.setNextDueDate(startDate);
    list = sch.paymentDates(startDate, endDate);
    QCOMPARE(list.count(), 5);
    QCOMPARE(list[0], QDate(2010, 3, 19));
    // Would fall on a Sunday so gets moved to 16th.
    QCOMPARE(list[1], QDate(2010, 4, 16));
    QCOMPARE(list[2], QDate(2010, 5, 18));
    QCOMPARE(list[3], QDate(2010, 6, 17));
    // Would fall on a Saturday so gets moved to 16th.
    QCOMPARE(list[4], QDate(2010, 7, 16));
    // Schedule::Occurrence::EveryEightWeeks
    sch.setOccurrence(Schedule::Occurrence::EveryEightWeeks);
    startDate.setDate(2010, 7, 26);
    endDate.setDate(2011, 3, 26);
    sch.setStartDate(startDate);
    sch.setNextDueDate(startDate);
    list = sch.paymentDates(startDate, endDate);
    QCOMPARE(list.count(), 5);
    QCOMPARE(list[0], QDate(2010, 7, 26));
    QCOMPARE(list[1], QDate(2010, 9, 20));
    QCOMPARE(list[2], QDate(2010, 11, 15));
    QCOMPARE(list[3], QDate(2011, 1, 10));
    QCOMPARE(list[4], QDate(2011, 3, 7));
    // Schedule::Occurrence::EveryOtherMonth
    sch.setOccurrence(Schedule::Occurrence::EveryOtherMonth);
    startDate.setDate(2011, 3, 14);
    endDate.setDate(2011, 11, 20);
    sch.setStartDate(startDate);
    sch.setNextDueDate(startDate);
    list = sch.paymentDates(startDate, endDate);
    QCOMPARE(list.count(), 5);
    QCOMPARE(list[0], QDate(2011, 3, 14));
    // Would fall on a Saturday so gets moved to 13th.
    QCOMPARE(list[1], QDate(2011, 5, 13));
    QCOMPARE(list[2], QDate(2011, 7, 14));
    QCOMPARE(list[3], QDate(2011, 9, 14));
    QCOMPARE(list[4], QDate(2011, 11, 14));
    // Schedule::Occurrence::EveryThreeMonths
    sch.setOccurrence(Schedule::Occurrence::EveryThreeMonths);
    startDate.setDate(2011, 11, 15);
    endDate.setDate(2012, 11, 19);
    sch.setStartDate(startDate);
    sch.setNextDueDate(startDate);
    list = sch.paymentDates(startDate, endDate);
    QCOMPARE(list.count(), 5);
    QCOMPARE(list[0], QDate(2011, 11, 15));
    QCOMPARE(list[1], QDate(2012, 2, 15));
    QCOMPARE(list[2], QDate(2012, 5, 15));
    QCOMPARE(list[3], QDate(2012, 8, 15));
    QCOMPARE(list[4], QDate(2012, 11, 15));
    // Schedule::Occurrence::Quarterly
    sch.setOccurrence(Schedule::Occurrence::Quarterly);
    startDate.setDate(2012, 11, 20);
    endDate.setDate(2013, 11, 23);
    sch.setStartDate(startDate);
    sch.setNextDueDate(startDate);
    list = sch.paymentDates(startDate, endDate);
    QCOMPARE(list.count(), 5);
    QCOMPARE(list[0], QDate(2012, 11, 20));
    QCOMPARE(list[1], QDate(2013, 2, 20));
    QCOMPARE(list[2], QDate(2013, 5, 20));
    QCOMPARE(list[3], QDate(2013, 8, 20));
    QCOMPARE(list[4], QDate(2013, 11, 20));
    // Schedule::Occurrence::EveryFourMonths
    sch.setOccurrence(Schedule::Occurrence::EveryFourMonths);
    startDate.setDate(2013, 11, 21);
    endDate.setDate(2015, 3, 23);
    sch.setStartDate(startDate);
    sch.setNextDueDate(startDate);
    list = sch.paymentDates(startDate, endDate);
    QCOMPARE(list.count(), 5);
    QCOMPARE(list[0], QDate(2013, 11, 21));
    QCOMPARE(list[1], QDate(2014, 3, 21));
    QCOMPARE(list[2], QDate(2014, 7, 21));
    QCOMPARE(list[3], QDate(2014, 11, 21));
    // Would fall on a Saturday so gets moved to 20th.
    QCOMPARE(list[4], QDate(2015, 3, 20));
    // Schedule::Occurrence::TwiceYearly
    sch.setOccurrence(Schedule::Occurrence::TwiceYearly);
    startDate.setDate(2015, 3, 22);
    endDate.setDate(2017, 3, 29);
    sch.setStartDate(startDate);
    sch.setNextDueDate(startDate);
    list = sch.paymentDates(startDate, endDate);
    QCOMPARE(list.count(), 4);
    // First date would fall on a Sunday which would get moved
    // to 20th which is before start date so not in list.
    QCOMPARE(list[0], QDate(2015, 9, 22));
    QCOMPARE(list[1], QDate(2016, 3, 22));
    QCOMPARE(list[2], QDate(2016, 9, 22));
    QCOMPARE(list[3], QDate(2017, 3, 22));
    // Schedule::Occurrence::Yearly
    sch.setOccurrence(Schedule::Occurrence::Yearly);
    startDate.setDate(2017, 3, 23);
    endDate.setDate(2021, 3, 29);
    sch.setStartDate(startDate);
    sch.setNextDueDate(startDate);
    list = sch.paymentDates(startDate, endDate);
    QCOMPARE(list.count(), 5);
    QCOMPARE(list[0], QDate(2017, 3, 23));
    QCOMPARE(list[1], QDate(2018, 3, 23));
    // Would fall on a Saturday so gets moved to 22nd.
    QCOMPARE(list[2], QDate(2019, 3, 22));
    QCOMPARE(list[3], QDate(2020, 3, 23));
    QCOMPARE(list[4], QDate(2021, 3, 23));
    // Schedule::Occurrence::EveryOtherYear
    sch.setOccurrence(Schedule::Occurrence::EveryOtherYear);
    startDate.setDate(2021, 3, 24);
    endDate.setDate(2029, 3, 30);
    sch.setStartDate(startDate);
    sch.setNextDueDate(startDate);
    list = sch.paymentDates(startDate, endDate);
    QCOMPARE(list.count(), 5);
    QCOMPARE(list[0], QDate(2021, 3, 24));
    QCOMPARE(list[1], QDate(2023, 3, 24));
    QCOMPARE(list[2], QDate(2025, 3, 24));
    QCOMPARE(list[3], QDate(2027, 3, 24));
    // Would fall on a Saturday so gets moved to 23rd.
    QCOMPARE(list[4], QDate(2029, 3, 23));
  } catch (const MyMoneyException &) {
    QFAIL("Unexpected exception");
  }
}

void MyMoneyScheduleTest::testWriteXML()
{
  MyMoneySchedule sch("A Name",
                      Schedule::Type::Bill,
                      Schedule::Occurrence::Weekly, 123,
                      Schedule::PaymentType::DirectDebit,
                      QDate::currentDate(),
                      QDate(),
                      true,
                      true);

  sch.setLastPayment(QDate::currentDate());
  sch.recordPayment(QDate::currentDate());
  sch.d_func()->setId("SCH0001");

  MyMoneyTransaction t;
  t.setPostDate(QDate(2001, 12, 28));
  t.setEntryDate(QDate(2003, 9, 29));
  t.d_func()->setId("T000000000000000001");
  t.setMemo("Wohnung:Miete");
  t.setCommodity("EUR");
  t.setValue("key", "value");

  MyMoneySplit s;
  s.setPayeeId("P000001");
  s.setShares(MyMoneyMoney(96379, 100));
  s.setValue(MyMoneyMoney(96379, 100));
  s.setAccountId("A000076");
  s.setBankID("SPID1");
  s.setReconcileFlag(eMyMoney::Split::State::Reconciled);
  t.addSplit(s);

  s.setPayeeId("P000001");
  s.setShares(MyMoneyMoney(-96379, 100));
  s.setValue(MyMoneyMoney(-96379, 100));
  s.setAccountId("A000276");
  s.setBankID("SPID2");
  s.setReconcileFlag(eMyMoney::Split::State::Cleared);
  s.clearId();
  t.addSplit(s);

  sch.setTransaction(t);

  QDomDocument doc("TEST");
  QDomElement el = doc.createElement("SCHEDULE-CONTAINER");
  doc.appendChild(el);
  sch.writeXML(doc, el);

  QCOMPARE(doc.doctype().name(), QLatin1String("TEST"));
  QDomElement scheduleContainer = doc.documentElement();
  QVERIFY(scheduleContainer.isElement());
  QCOMPARE(scheduleContainer.tagName(), QLatin1String("SCHEDULE-CONTAINER"));
  QCOMPARE(scheduleContainer.childNodes().size(), 1);
  QVERIFY(scheduleContainer.childNodes().at(0).isElement());

  QDomElement schedule = scheduleContainer.childNodes().at(0).toElement();
  QCOMPARE(schedule.tagName(), QLatin1String("SCHEDULED_TX"));
  QCOMPARE(schedule.attribute("id"), QLatin1String("SCH0001"));
  QCOMPARE(schedule.attribute("paymentType"), QLatin1String("1"));
  QCOMPARE(schedule.attribute("autoEnter"), QLatin1String("1"));
  QCOMPARE(schedule.attribute("occurenceMultiplier"), QLatin1String("123")); // krazy:exclude=spelling
  QCOMPARE(schedule.attribute("startDate"), QDate::currentDate().toString(Qt::ISODate));
  QCOMPARE(schedule.attribute("lastPayment"), QDate::currentDate().toString(Qt::ISODate));
  QCOMPARE(schedule.attribute("occurenceMultiplier"), QLatin1String("123")); // krazy:exclude=spelling
  QCOMPARE(schedule.attribute("occurence"), QLatin1String("4")); // krazy:exclude=spelling
  QCOMPARE(schedule.attribute("type"), QLatin1String("1"));
  QCOMPARE(schedule.attribute("name"), QLatin1String("A Name"));
  QCOMPARE(schedule.attribute("fixed"), QLatin1String("1"));
  QCOMPARE(schedule.attribute("endDate"), QString());
  QCOMPARE(schedule.childNodes().size(), 2);

  QVERIFY(schedule.childNodes().at(0).isElement());
  QDomElement payments = schedule.childNodes().at(0).toElement();

  QVERIFY(schedule.childNodes().at(1).isElement());
  QDomElement transaction = schedule.childNodes().at(1).toElement();
  QCOMPARE(transaction.tagName(), QLatin1String("TRANSACTION"));
  QCOMPARE(transaction.attribute("id"), QString());
  QCOMPARE(transaction.attribute("postdate"), QLatin1String("2001-12-28"));
  QCOMPARE(transaction.attribute("commodity"), QLatin1String("EUR"));
  QCOMPARE(transaction.attribute("memo"), QLatin1String("Wohnung:Miete"));
  QCOMPARE(transaction.attribute("entrydate"), QLatin1String("2003-09-29"));
  QCOMPARE(transaction.childNodes().size(), 2);

  QVERIFY(transaction.childNodes().at(0).isElement());
  QDomElement splits = transaction.childNodes().at(0).toElement();
  QCOMPARE(splits.tagName(), QLatin1String("SPLITS"));
  QCOMPARE(splits.childNodes().size(), 2);
  QVERIFY(splits.childNodes().at(0).isElement());
  QDomElement split1 = splits.childNodes().at(0).toElement();
  QCOMPARE(split1.tagName(), QLatin1String("SPLIT"));
  QCOMPARE(split1.attribute("id"), QLatin1String("S0001"));
  QCOMPARE(split1.attribute("payee"), QLatin1String("P000001"));
  QCOMPARE(split1.attribute("reconcileflag"), QLatin1String("2"));
  QCOMPARE(split1.attribute("shares"), QLatin1String("96379/100"));
  QCOMPARE(split1.attribute("reconciledate"), QString());
  QCOMPARE(split1.attribute("action"), QString());
  QCOMPARE(split1.attribute("bankid"), QString());
  QCOMPARE(split1.attribute("account"), QLatin1String("A000076"));
  QCOMPARE(split1.attribute("number"), QString());
  QCOMPARE(split1.attribute("value"), QLatin1String("96379/100"));
  QCOMPARE(split1.attribute("memo"), QString());
  QCOMPARE(split1.childNodes().size(), 0);

  QVERIFY(splits.childNodes().at(1).isElement());
  QDomElement split2 = splits.childNodes().at(1).toElement();
  QCOMPARE(split2.tagName(), QLatin1String("SPLIT"));
  QCOMPARE(split2.attribute("id"), QLatin1String("S0002"));
  QCOMPARE(split2.attribute("payee"), QLatin1String("P000001"));
  QCOMPARE(split2.attribute("reconcileflag"), QLatin1String("1"));
  QCOMPARE(split2.attribute("shares"), QLatin1String("-96379/100"));
  QCOMPARE(split2.attribute("reconciledate"), QString());
  QCOMPARE(split2.attribute("action"), QString());
  QCOMPARE(split2.attribute("bankid"), QString());
  QCOMPARE(split2.attribute("account"), QLatin1String("A000276"));
  QCOMPARE(split2.attribute("number"), QString());
  QCOMPARE(split2.attribute("value"), QLatin1String("-96379/100"));
  QCOMPARE(split2.attribute("memo"), QString());
  QCOMPARE(split2.childNodes().size(), 0);

  QDomElement keyValuePairs = transaction.childNodes().at(1).toElement();
  QCOMPARE(keyValuePairs.tagName(), QLatin1String("KEYVALUEPAIRS"));
  QCOMPARE(keyValuePairs.childNodes().size(), 1);

  QVERIFY(keyValuePairs.childNodes().at(0).isElement());
  QDomElement keyValuePair1 = keyValuePairs.childNodes().at(0).toElement();
  QCOMPARE(keyValuePair1.tagName(), QLatin1String("PAIR"));
  QCOMPARE(keyValuePair1.attribute("key"), QLatin1String("key"));
  QCOMPARE(keyValuePair1.attribute("value"), QLatin1String("value"));
  QCOMPARE(keyValuePair1.childNodes().size(), 0);
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
  } catch (const MyMoneyException &) {
  }

  doc.setContent(ref_ok1);
  node = doc.documentElement().firstChild().toElement();


  try {
    sch = MyMoneySchedule(node);
    QCOMPARE(sch.id(), QLatin1String("SCH0002"));
    QCOMPARE(sch.nextDueDate(), QDate::currentDate().addDays(7));
    QCOMPARE(sch.startDate(), QDate::currentDate());
    QCOMPARE(sch.endDate(), QDate());
    QCOMPARE(sch.autoEnter(), true);
    QCOMPARE(sch.isFixed(), true);
    QCOMPARE(sch.weekendOption(), Schedule::WeekendOption::MoveNothing);
    QCOMPARE(sch.lastPayment(), QDate::currentDate());
    QCOMPARE(sch.paymentType(), Schedule::PaymentType::DirectDebit);
    QCOMPARE(sch.type(), Schedule::Type::Bill);
    QCOMPARE(sch.name(), QLatin1String("A Name"));
    QCOMPARE(sch.occurrence(), Schedule::Occurrence::Weekly);
    QCOMPARE(sch.occurrenceMultiplier(), 1);
    QCOMPARE(sch.nextDueDate(), sch.lastPayment().addDays(7));
    QCOMPARE(sch.recordedPayments().count(), 1);
    QCOMPARE(sch.recordedPayments()[0], QDate::currentDate());
  } catch (const MyMoneyException &) {
    QFAIL("Unexpected exception");
  }

  doc.setContent(ref_ok2);
  node = doc.documentElement().firstChild().toElement();


  try {
    sch = MyMoneySchedule(node);
    QCOMPARE(sch.id(), QLatin1String("SCH0002"));
    QCOMPARE(sch.nextDueDate(), QDate::currentDate().addDays(7));
    QCOMPARE(sch.startDate(), QDate::currentDate());
    QCOMPARE(sch.endDate(), QDate());
    QCOMPARE(sch.autoEnter(), true);
    QCOMPARE(sch.isFixed(), true);
    QCOMPARE(sch.weekendOption(), Schedule::WeekendOption::MoveNothing);
    QCOMPARE(sch.lastPayment(), QDate::currentDate());
    QCOMPARE(sch.paymentType(), Schedule::PaymentType::DirectDebit);
    QCOMPARE(sch.type(), Schedule::Type::Bill);
    QCOMPARE(sch.name(), QLatin1String("A Name"));
    QCOMPARE(sch.occurrence(), Schedule::Occurrence::Weekly);
    QCOMPARE(sch.occurrenceMultiplier(), 1);
    QCOMPARE(sch.nextDueDate(), sch.lastPayment().addDays(7));
    QCOMPARE(sch.recordedPayments().count(), 1);
    QCOMPARE(sch.recordedPayments()[0], QDate::currentDate());
  } catch (const MyMoneyException &) {
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

  } catch (const MyMoneyException &) {
    QFAIL("Unexpected exception");
  }

  QCOMPARE(sch.hasReferenceTo(QLatin1String("P000001")), true);
  QCOMPARE(sch.hasReferenceTo(QLatin1String("A000276")), true);
  QCOMPARE(sch.hasReferenceTo(QLatin1String("A000076")), true);
  QCOMPARE(sch.hasReferenceTo(QLatin1String("EUR")), true);
}

void MyMoneyScheduleTest::testAdjustedNextDueDate()
{
  MyMoneySchedule s;

  QDate dueDate(2007, 9, 3); // start on a Monday
  for (int i = 0; i < 7; ++i) {
    s.setNextDueDate(dueDate);
    s.setWeekendOption(Schedule::WeekendOption::MoveNothing);
    QCOMPARE(s.adjustedNextDueDate(), dueDate);

    s.setWeekendOption(Schedule::WeekendOption::MoveBefore);
    switch (i) {
      case 5: // Saturday
      case 6: // Sunday
        QCOMPARE(s.adjustedNextDueDate(), QDate(2007, 9, 7));
        break;
      default:
        QCOMPARE(s.adjustedNextDueDate(), dueDate);
        break;
    }

    s.setWeekendOption(Schedule::WeekendOption::MoveAfter);
    switch (i) {
      case 5: // Saturday
      case 6: // Sunday
        QCOMPARE(s.adjustedNextDueDate(), QDate(2007, 9, 10));
        break;
      default:
        QCOMPARE(s.adjustedNextDueDate(), dueDate);
        break;
    }
    dueDate = dueDate.addDays(1);
  }
}

void MyMoneyScheduleTest::testModifyNextDueDate()
{
  MyMoneySchedule s;
  s.setStartDate(QDate(2007, 1, 2));
  s.setOccurrence(Schedule::Occurrence::Monthly);
  s.setNextDueDate(s.startDate().addMonths(1));
  s.setLastPayment(s.startDate());

  QList<QDate> dates;
  dates = s.paymentDates(QDate(2007, 2, 2), QDate(2007, 2, 2));
  QCOMPARE(s.nextDueDate(), QDate(2007, 2, 2));
  QCOMPARE(dates.count(), 1);
  QCOMPARE(dates[0], QDate(2007, 2, 2));

  s.setNextDueDate(QDate(2007, 1, 24));

  dates = s.paymentDates(QDate(2007, 2, 1), QDate(2007, 2, 1));
  QCOMPARE(s.nextDueDate(), QDate(2007, 1, 24));
  QCOMPARE(dates.count(), 0);

  dates = s.paymentDates(QDate(2007, 1, 24), QDate(2007, 1, 24));
  QCOMPARE(dates.count(), 1);

  dates = s.paymentDates(QDate(2007, 1, 24), QDate(2007, 2, 24));
  QCOMPARE(dates.count(), 2);
  QCOMPARE(dates[0], QDate(2007, 1, 24));
  QCOMPARE(dates[1], QDate(2007, 2, 2));

}

void MyMoneyScheduleTest::testDaysBetweenEvents()
{
  QCOMPARE(MyMoneySchedule::daysBetweenEvents(Schedule::Occurrence::Once), 0);
  QCOMPARE(MyMoneySchedule::daysBetweenEvents(Schedule::Occurrence::Daily), 1);
  QCOMPARE(MyMoneySchedule::daysBetweenEvents(Schedule::Occurrence::Weekly), 7);
  QCOMPARE(MyMoneySchedule::daysBetweenEvents(Schedule::Occurrence::EveryOtherWeek), 14);
  QCOMPARE(MyMoneySchedule::daysBetweenEvents(Schedule::Occurrence::Fortnightly), 14);
  QCOMPARE(MyMoneySchedule::daysBetweenEvents(Schedule::Occurrence::EveryHalfMonth), 15);
  QCOMPARE(MyMoneySchedule::daysBetweenEvents(Schedule::Occurrence::EveryThreeWeeks), 21);
  QCOMPARE(MyMoneySchedule::daysBetweenEvents(Schedule::Occurrence::EveryFourWeeks), 28);
  QCOMPARE(MyMoneySchedule::daysBetweenEvents(Schedule::Occurrence::EveryThirtyDays), 30);
  QCOMPARE(MyMoneySchedule::daysBetweenEvents(Schedule::Occurrence::Monthly), 30);
  QCOMPARE(MyMoneySchedule::daysBetweenEvents(Schedule::Occurrence::EveryEightWeeks), 56);
  QCOMPARE(MyMoneySchedule::daysBetweenEvents(Schedule::Occurrence::EveryOtherMonth), 60);
  QCOMPARE(MyMoneySchedule::daysBetweenEvents(Schedule::Occurrence::EveryThreeMonths), 90);
  QCOMPARE(MyMoneySchedule::daysBetweenEvents(Schedule::Occurrence::Quarterly), 90);
  QCOMPARE(MyMoneySchedule::daysBetweenEvents(Schedule::Occurrence::EveryFourMonths), 120);
  QCOMPARE(MyMoneySchedule::daysBetweenEvents(Schedule::Occurrence::TwiceYearly), 180);
  QCOMPARE(MyMoneySchedule::daysBetweenEvents(Schedule::Occurrence::Yearly), 360);
  QCOMPARE(MyMoneySchedule::daysBetweenEvents(Schedule::Occurrence::EveryOtherYear), 0);
}

void MyMoneyScheduleTest::testEventsPerYear()
{
  QCOMPARE(MyMoneySchedule::eventsPerYear(Schedule::Occurrence::Once), 0);
  QCOMPARE(MyMoneySchedule::eventsPerYear(Schedule::Occurrence::Daily), 365);
  QCOMPARE(MyMoneySchedule::eventsPerYear(Schedule::Occurrence::Weekly), 52);
  QCOMPARE(MyMoneySchedule::eventsPerYear(Schedule::Occurrence::EveryOtherWeek), 26);
  QCOMPARE(MyMoneySchedule::eventsPerYear(Schedule::Occurrence::Fortnightly), 26);
  QCOMPARE(MyMoneySchedule::eventsPerYear(Schedule::Occurrence::EveryHalfMonth), 24);
  QCOMPARE(MyMoneySchedule::eventsPerYear(Schedule::Occurrence::EveryThreeWeeks), 17);
  QCOMPARE(MyMoneySchedule::eventsPerYear(Schedule::Occurrence::EveryFourWeeks), 13);
  QCOMPARE(MyMoneySchedule::eventsPerYear(Schedule::Occurrence::EveryThirtyDays), 12);
  QCOMPARE(MyMoneySchedule::eventsPerYear(Schedule::Occurrence::Monthly), 12);
  QCOMPARE(MyMoneySchedule::eventsPerYear(Schedule::Occurrence::EveryEightWeeks), 6);
  QCOMPARE(MyMoneySchedule::eventsPerYear(Schedule::Occurrence::EveryOtherMonth), 6);
  QCOMPARE(MyMoneySchedule::eventsPerYear(Schedule::Occurrence::EveryThreeMonths), 4);
  QCOMPARE(MyMoneySchedule::eventsPerYear(Schedule::Occurrence::Quarterly), 4);
  QCOMPARE(MyMoneySchedule::eventsPerYear(Schedule::Occurrence::EveryFourMonths), 3);
  QCOMPARE(MyMoneySchedule::eventsPerYear(Schedule::Occurrence::TwiceYearly), 2);
  QCOMPARE(MyMoneySchedule::eventsPerYear(Schedule::Occurrence::Yearly), 1);
  QCOMPARE(MyMoneySchedule::eventsPerYear(Schedule::Occurrence::EveryOtherYear), 0);
}

void MyMoneyScheduleTest::testOccurrenceToString()
{
  // For each occurrenceE test MyMoneySchedule::occurrenceToString(occurrenceE)
  QCOMPARE(MyMoneySchedule::occurrenceToString(Schedule::Occurrence::Once), QLatin1String("Once"));
  QCOMPARE(MyMoneySchedule::occurrenceToString(Schedule::Occurrence::Daily), QLatin1String("Daily"));
  QCOMPARE(MyMoneySchedule::occurrenceToString(Schedule::Occurrence::Weekly), QLatin1String("Weekly"));
  QCOMPARE(MyMoneySchedule::occurrenceToString(Schedule::Occurrence::EveryOtherWeek), QLatin1String("Every other week"));
  QCOMPARE(MyMoneySchedule::occurrenceToString(Schedule::Occurrence::Fortnightly), QLatin1String("Fortnightly"));
  QCOMPARE(MyMoneySchedule::occurrenceToString(Schedule::Occurrence::EveryHalfMonth), QLatin1String("Every half month"));
  QCOMPARE(MyMoneySchedule::occurrenceToString(Schedule::Occurrence::EveryThreeWeeks), QLatin1String("Every three weeks"));
  QCOMPARE(MyMoneySchedule::occurrenceToString(Schedule::Occurrence::EveryFourWeeks), QLatin1String("Every four weeks"));
  QCOMPARE(MyMoneySchedule::occurrenceToString(Schedule::Occurrence::EveryThirtyDays), QLatin1String("Every thirty days"));
  QCOMPARE(MyMoneySchedule::occurrenceToString(Schedule::Occurrence::Monthly), QLatin1String("Monthly"));
  QCOMPARE(MyMoneySchedule::occurrenceToString(Schedule::Occurrence::EveryEightWeeks), QLatin1String("Every eight weeks"));
  QCOMPARE(MyMoneySchedule::occurrenceToString(Schedule::Occurrence::EveryOtherMonth), QLatin1String("Every two months"));
  QCOMPARE(MyMoneySchedule::occurrenceToString(Schedule::Occurrence::EveryThreeMonths), QLatin1String("Every three months"));
  QCOMPARE(MyMoneySchedule::occurrenceToString(Schedule::Occurrence::Quarterly), QLatin1String("Quarterly"));
  QCOMPARE(MyMoneySchedule::occurrenceToString(Schedule::Occurrence::EveryFourMonths), QLatin1String("Every four months"));
  QCOMPARE(MyMoneySchedule::occurrenceToString(Schedule::Occurrence::TwiceYearly), QLatin1String("Twice yearly"));
  QCOMPARE(MyMoneySchedule::occurrenceToString(Schedule::Occurrence::Yearly), QLatin1String("Yearly"));
  QCOMPARE(MyMoneySchedule::occurrenceToString(Schedule::Occurrence::EveryOtherYear), QLatin1String("Every other year"));
  // For each occurrenceE set occurrence and compare occurrenceToString() with oTS(occurrence())
  MyMoneySchedule s;
  s.setStartDate(QDate(2007, 1, 1));
  s.setNextDueDate(s.startDate());
  s.setLastPayment(s.startDate());
  s.setOccurrence(Schedule::Occurrence::Once); QCOMPARE(s.occurrenceToString(), QLatin1String("Once"));
  s.setOccurrence(Schedule::Occurrence::Daily); QCOMPARE(s.occurrenceToString(), QLatin1String("Daily"));
  s.setOccurrence(Schedule::Occurrence::Weekly); QCOMPARE(s.occurrenceToString(), QLatin1String("Weekly"));
  s.setOccurrence(Schedule::Occurrence::EveryOtherWeek); QCOMPARE(s.occurrenceToString(), QLatin1String("Every other week"));
  // Fortnightly no longer used: Every other week used instead
  s.setOccurrence(Schedule::Occurrence::Fortnightly); QCOMPARE(s.occurrenceToString(), QLatin1String("Every other week"));
  s.setOccurrence(Schedule::Occurrence::EveryHalfMonth); QCOMPARE(s.occurrenceToString(), QLatin1String("Every half month"));
  s.setOccurrence(Schedule::Occurrence::EveryThreeWeeks); QCOMPARE(s.occurrenceToString(), QLatin1String("Every three weeks"));
  s.setOccurrence(Schedule::Occurrence::EveryFourWeeks); QCOMPARE(s.occurrenceToString(), QLatin1String("Every four weeks"));
  s.setOccurrence(Schedule::Occurrence::EveryThirtyDays); QCOMPARE(s.occurrenceToString(), QLatin1String("Every thirty days"));
  s.setOccurrence(Schedule::Occurrence::Monthly); QCOMPARE(s.occurrenceToString(), QLatin1String("Monthly"));
  s.setOccurrence(Schedule::Occurrence::EveryEightWeeks); QCOMPARE(s.occurrenceToString(), QLatin1String("Every eight weeks"));
  s.setOccurrence(Schedule::Occurrence::EveryOtherMonth); QCOMPARE(s.occurrenceToString(), QLatin1String("Every two months"));
  s.setOccurrence(Schedule::Occurrence::EveryThreeMonths); QCOMPARE(s.occurrenceToString(), QLatin1String("Every three months"));
  // Quarterly no longer used.  Every three months used instead
  s.setOccurrence(Schedule::Occurrence::Quarterly); QCOMPARE(s.occurrenceToString(), QLatin1String("Every three months"));
  s.setOccurrence(Schedule::Occurrence::EveryFourMonths); QCOMPARE(s.occurrenceToString(), QLatin1String("Every four months"));
  s.setOccurrence(Schedule::Occurrence::TwiceYearly); QCOMPARE(s.occurrenceToString(), QLatin1String("Twice yearly"));
  s.setOccurrence(Schedule::Occurrence::Yearly); QCOMPARE(s.occurrenceToString(), QLatin1String("Yearly"));
  s.setOccurrence(Schedule::Occurrence::EveryOtherYear); QCOMPARE(s.occurrenceToString(), QLatin1String("Every other year"));
  // Test occurrenceToString(mult,occ)
  // Test all pairs equivalent to simple occurrences: should return the same as occurrenceToString(simpleOcc)
  // TODO replace string with (mult,occ) call.
  QCOMPARE(MyMoneySchedule::occurrenceToString(Schedule::Occurrence::Once), MyMoneySchedule::occurrenceToString(1, Schedule::Occurrence::Once));
  QCOMPARE(MyMoneySchedule::occurrenceToString(Schedule::Occurrence::Daily), MyMoneySchedule::occurrenceToString(1, Schedule::Occurrence::Daily));
  QCOMPARE(MyMoneySchedule::occurrenceToString(Schedule::Occurrence::Weekly), MyMoneySchedule::occurrenceToString(1, Schedule::Occurrence::Weekly));
  QCOMPARE(MyMoneySchedule::occurrenceToString(Schedule::Occurrence::EveryOtherWeek), MyMoneySchedule::occurrenceToString(2, Schedule::Occurrence::Weekly));
  // Fortnightly will no longer be used: only Every Other Week
  QCOMPARE(MyMoneySchedule::occurrenceToString(Schedule::Occurrence::EveryHalfMonth), MyMoneySchedule::occurrenceToString(1, Schedule::Occurrence::EveryHalfMonth));
  QCOMPARE(MyMoneySchedule::occurrenceToString(Schedule::Occurrence::EveryThreeWeeks), MyMoneySchedule::occurrenceToString(3, Schedule::Occurrence::Weekly));
  QCOMPARE(MyMoneySchedule::occurrenceToString(Schedule::Occurrence::EveryFourWeeks), MyMoneySchedule::occurrenceToString(4, Schedule::Occurrence::Weekly));
  QCOMPARE(MyMoneySchedule::occurrenceToString(Schedule::Occurrence::Monthly), MyMoneySchedule::occurrenceToString(1, Schedule::Occurrence::Monthly));
  QCOMPARE(MyMoneySchedule::occurrenceToString(Schedule::Occurrence::EveryEightWeeks), MyMoneySchedule::occurrenceToString(8, Schedule::Occurrence::Weekly));
  QCOMPARE(MyMoneySchedule::occurrenceToString(Schedule::Occurrence::EveryOtherMonth), MyMoneySchedule::occurrenceToString(2, Schedule::Occurrence::Monthly));
  QCOMPARE(MyMoneySchedule::occurrenceToString(Schedule::Occurrence::EveryThreeMonths), MyMoneySchedule::occurrenceToString(3, Schedule::Occurrence::Monthly));
  // Quarterly will no longer be used: only Every Three Months
  QCOMPARE(MyMoneySchedule::occurrenceToString(Schedule::Occurrence::EveryFourMonths), MyMoneySchedule::occurrenceToString(4, Schedule::Occurrence::Monthly));
  QCOMPARE(MyMoneySchedule::occurrenceToString(Schedule::Occurrence::TwiceYearly), MyMoneySchedule::occurrenceToString(6, Schedule::Occurrence::Monthly));
  QCOMPARE(MyMoneySchedule::occurrenceToString(Schedule::Occurrence::Yearly), MyMoneySchedule::occurrenceToString(1, Schedule::Occurrence::Yearly));
  QCOMPARE(MyMoneySchedule::occurrenceToString(Schedule::Occurrence::EveryOtherYear), MyMoneySchedule::occurrenceToString(2, Schedule::Occurrence::Yearly));
  // Test additional calls with other mult,occ
  QCOMPARE(MyMoneySchedule::occurrenceToString(2, Schedule::Occurrence::Once), QLatin1String("2 times"));
  QCOMPARE(MyMoneySchedule::occurrenceToString(2, Schedule::Occurrence::Daily), QLatin1String("Every 2 days"));
  QCOMPARE(MyMoneySchedule::occurrenceToString(5, Schedule::Occurrence::Weekly), QLatin1String("Every 5 weeks"));
  QCOMPARE(MyMoneySchedule::occurrenceToString(2, Schedule::Occurrence::EveryHalfMonth), QLatin1String("Every 2 half months"));
  QCOMPARE(MyMoneySchedule::occurrenceToString(5, Schedule::Occurrence::Monthly), QLatin1String("Every 5 months"));
  QCOMPARE(MyMoneySchedule::occurrenceToString(3, Schedule::Occurrence::Yearly), QLatin1String("Every 3 years"));
  QCOMPARE(MyMoneySchedule::occurrenceToString(37, Schedule::Occurrence::Once), QLatin1String("37 times"));
  QCOMPARE(MyMoneySchedule::occurrenceToString(43, Schedule::Occurrence::Daily), QLatin1String("Every 43 days"));
  QCOMPARE(MyMoneySchedule::occurrenceToString(61, Schedule::Occurrence::Weekly), QLatin1String("Every 61 weeks"));
  QCOMPARE(MyMoneySchedule::occurrenceToString(73, Schedule::Occurrence::EveryHalfMonth), QLatin1String("Every 73 half months"));
  QCOMPARE(MyMoneySchedule::occurrenceToString(83, Schedule::Occurrence::Monthly), QLatin1String("Every 83 months"));
  QCOMPARE(MyMoneySchedule::occurrenceToString(89, Schedule::Occurrence::Yearly), QLatin1String("Every 89 years"));
  // Test instance-level occurrenceToString method is using occurrencePeriod and multiplier
  // For each base occurrence set occurrencePeriod and multiplier
  s.setOccurrencePeriod(Schedule::Occurrence::Once); s.setOccurrenceMultiplier(1);
  s.setOccurrence(Schedule::Occurrence::Once);
  s.setOccurrenceMultiplier(1); QCOMPARE(s.occurrenceToString(), QLatin1String("Once"));
  s.setOccurrenceMultiplier(2); QCOMPARE(s.occurrenceToString(), QLatin1String("2 times"));
  s.setOccurrenceMultiplier(3); QCOMPARE(s.occurrenceToString(), QLatin1String("3 times"));
  s.setOccurrencePeriod(Schedule::Occurrence::Daily);
  s.setOccurrenceMultiplier(1); QCOMPARE(s.occurrenceToString(), QLatin1String("Daily"));
  s.setOccurrenceMultiplier(30); QCOMPARE(s.occurrenceToString(), QLatin1String("Every thirty days"));
  s.setOccurrenceMultiplier(3); QCOMPARE(s.occurrenceToString(), QLatin1String("Every 3 days"));
  s.setOccurrence(Schedule::Occurrence::Weekly);
  QCOMPARE(s.occurrenceToString(), QLatin1String("Weekly"));
  s.setOccurrenceMultiplier(2); QCOMPARE(s.occurrenceToString(), QLatin1String("Every other week"));
  s.setOccurrenceMultiplier(3); QCOMPARE(s.occurrenceToString(), QLatin1String("Every three weeks"));
  s.setOccurrenceMultiplier(4); QCOMPARE(s.occurrenceToString(), QLatin1String("Every four weeks"));
  s.setOccurrenceMultiplier(5); QCOMPARE(s.occurrenceToString(), QLatin1String("Every 5 weeks"));
  s.setOccurrenceMultiplier(7); QCOMPARE(s.occurrenceToString(), QLatin1String("Every 7 weeks"));
  s.setOccurrenceMultiplier(8); QCOMPARE(s.occurrenceToString(), QLatin1String("Every eight weeks"));
  s.setOccurrenceMultiplier(9); QCOMPARE(s.occurrenceToString(), QLatin1String("Every 9 weeks"));
  s.setOccurrence(Schedule::Occurrence::EveryHalfMonth);
  s.setOccurrenceMultiplier(1); QCOMPARE(s.occurrenceToString(), QLatin1String("Every half month"));
  s.setOccurrenceMultiplier(2); QCOMPARE(s.occurrenceToString(), QLatin1String("Every 2 half months"));
  s.setOccurrence(Schedule::Occurrence::Monthly);
  s.setOccurrenceMultiplier(1); QCOMPARE(s.occurrenceToString(), QLatin1String("Monthly"));
  s.setOccurrenceMultiplier(2); QCOMPARE(s.occurrenceToString(), QLatin1String("Every two months"));
  s.setOccurrenceMultiplier(3); QCOMPARE(s.occurrenceToString(), QLatin1String("Every three months"));
  s.setOccurrenceMultiplier(4); QCOMPARE(s.occurrenceToString(), QLatin1String("Every four months"));
  s.setOccurrenceMultiplier(5); QCOMPARE(s.occurrenceToString(), QLatin1String("Every 5 months"));
  s.setOccurrenceMultiplier(6); QCOMPARE(s.occurrenceToString(), QLatin1String("Twice yearly"));
  s.setOccurrenceMultiplier(7); QCOMPARE(s.occurrenceToString(), QLatin1String("Every 7 months"));
  s.setOccurrence(Schedule::Occurrence::Yearly);
  s.setOccurrenceMultiplier(1); QCOMPARE(s.occurrenceToString(), QLatin1String("Yearly"));
  s.setOccurrenceMultiplier(2); QCOMPARE(s.occurrenceToString(), QLatin1String("Every other year"));
  s.setOccurrenceMultiplier(3); QCOMPARE(s.occurrenceToString(), QLatin1String("Every 3 years"));
}

void MyMoneyScheduleTest::testOccurrencePeriodToString()
{
  // For each occurrenceE test MyMoneySchedule::occurrencePeriodToString(occurrenceE)
  // Base occurrences are translated
  QCOMPARE(MyMoneySchedule::occurrencePeriodToString(Schedule::Occurrence::Once), QLatin1String("Once"));
  QCOMPARE(MyMoneySchedule::occurrencePeriodToString(Schedule::Occurrence::Daily), QLatin1String("Day"));
  QCOMPARE(MyMoneySchedule::occurrencePeriodToString(Schedule::Occurrence::Weekly), QLatin1String("Week"));
  QCOMPARE(MyMoneySchedule::occurrencePeriodToString(Schedule::Occurrence::EveryHalfMonth), QLatin1String("Half-month"));
  QCOMPARE(MyMoneySchedule::occurrencePeriodToString(Schedule::Occurrence::Monthly), QLatin1String("Month"));
  QCOMPARE(MyMoneySchedule::occurrencePeriodToString(Schedule::Occurrence::Yearly), QLatin1String("Year"));
  // All others are not translated so return Any
  QCOMPARE(MyMoneySchedule::occurrencePeriodToString(Schedule::Occurrence::EveryOtherWeek), QLatin1String("Any"));
  QCOMPARE(MyMoneySchedule::occurrencePeriodToString(Schedule::Occurrence::Fortnightly), QLatin1String("Any"));
  QCOMPARE(MyMoneySchedule::occurrencePeriodToString(Schedule::Occurrence::EveryThreeWeeks), QLatin1String("Any"));
  QCOMPARE(MyMoneySchedule::occurrencePeriodToString(Schedule::Occurrence::EveryFourWeeks), QLatin1String("Any"));
  QCOMPARE(MyMoneySchedule::occurrencePeriodToString(Schedule::Occurrence::EveryThirtyDays), QLatin1String("Any"));
  QCOMPARE(MyMoneySchedule::occurrencePeriodToString(Schedule::Occurrence::EveryEightWeeks), QLatin1String("Any"));
  QCOMPARE(MyMoneySchedule::occurrencePeriodToString(Schedule::Occurrence::EveryOtherMonth), QLatin1String("Any"));
  QCOMPARE(MyMoneySchedule::occurrencePeriodToString(Schedule::Occurrence::EveryThreeMonths), QLatin1String("Any"));
  QCOMPARE(MyMoneySchedule::occurrencePeriodToString(Schedule::Occurrence::Quarterly), QLatin1String("Any"));
  QCOMPARE(MyMoneySchedule::occurrencePeriodToString(Schedule::Occurrence::EveryFourMonths), QLatin1String("Any"));
  QCOMPARE(MyMoneySchedule::occurrencePeriodToString(Schedule::Occurrence::TwiceYearly), QLatin1String("Any"));
  QCOMPARE(MyMoneySchedule::occurrencePeriodToString(Schedule::Occurrence::EveryOtherYear), QLatin1String("Any"));
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
  s.setOccurrencePeriod(Schedule::Occurrence::Once);
  QCOMPARE(s.occurrencePeriod(), Schedule::Occurrence::Once);
  s.setOccurrenceMultiplier(1);
  QCOMPARE(s.occurrenceMultiplier(), 1);
  QCOMPARE(s.occurrencePeriod(), Schedule::Occurrence::Once);
  QCOMPARE(s.occurrence(), Schedule::Occurrence::Once);
  s.setOccurrenceMultiplier(2);
  QCOMPARE(s.occurrenceMultiplier(), 2);
  QCOMPARE(s.occurrencePeriod(), Schedule::Occurrence::Once);
  QCOMPARE(s.occurrence(), Schedule::Occurrence::Once);

  s.setOccurrencePeriod(Schedule::Occurrence::Daily);
  QCOMPARE(s.occurrencePeriod(), Schedule::Occurrence::Daily);
  s.setOccurrenceMultiplier(1);
  QCOMPARE(s.occurrenceMultiplier(), 1);
  QCOMPARE(s.occurrencePeriod(), Schedule::Occurrence::Daily);
  QCOMPARE(s.occurrence(), Schedule::Occurrence::Daily);
  s.setOccurrenceMultiplier(30);
  QCOMPARE(s.occurrenceMultiplier(), 30);
  QCOMPARE(s.occurrencePeriod(), Schedule::Occurrence::Daily);
  QCOMPARE(s.occurrence(), Schedule::Occurrence::EveryThirtyDays);
  s.setOccurrenceMultiplier(2);
  QCOMPARE(s.occurrenceMultiplier(), 2);
  QCOMPARE(s.occurrencePeriod(), Schedule::Occurrence::Daily);
  QCOMPARE(s.occurrence(), Schedule::Occurrence::Daily);

  s.setOccurrencePeriod(Schedule::Occurrence::Weekly);
  QCOMPARE(s.occurrencePeriod(), Schedule::Occurrence::Weekly);
  s.setOccurrenceMultiplier(1);
  QCOMPARE(s.occurrenceMultiplier(), 1);
  QCOMPARE(s.occurrencePeriod(), Schedule::Occurrence::Weekly);
  QCOMPARE(s.occurrence(), Schedule::Occurrence::Weekly);
  s.setOccurrenceMultiplier(2);
  QCOMPARE(s.occurrenceMultiplier(), 2);
  QCOMPARE(s.occurrencePeriod(), Schedule::Occurrence::Weekly);
  QCOMPARE(s.occurrence(), Schedule::Occurrence::EveryOtherWeek);
  s.setOccurrenceMultiplier(3);
  QCOMPARE(s.occurrenceMultiplier(), 3);
  QCOMPARE(s.occurrencePeriod(), Schedule::Occurrence::Weekly);
  QCOMPARE(s.occurrence(), Schedule::Occurrence::EveryThreeWeeks);
  s.setOccurrenceMultiplier(4);
  QCOMPARE(s.occurrenceMultiplier(), 4);
  QCOMPARE(s.occurrencePeriod(), Schedule::Occurrence::Weekly);
  QCOMPARE(s.occurrence(), Schedule::Occurrence::EveryFourWeeks);
  s.setOccurrenceMultiplier(5);
  QCOMPARE(s.occurrenceMultiplier(), 5);
  QCOMPARE(s.occurrencePeriod(), Schedule::Occurrence::Weekly);
  QCOMPARE(s.occurrence(), Schedule::Occurrence::Weekly);
  s.setOccurrenceMultiplier(8);
  QCOMPARE(s.occurrenceMultiplier(), 8);
  QCOMPARE(s.occurrencePeriod(), Schedule::Occurrence::Weekly);
  QCOMPARE(s.occurrence(), Schedule::Occurrence::EveryEightWeeks);

  s.setOccurrencePeriod(Schedule::Occurrence::EveryHalfMonth);
  QCOMPARE(s.occurrencePeriod(), Schedule::Occurrence::EveryHalfMonth);
  s.setOccurrenceMultiplier(1);
  QCOMPARE(s.occurrenceMultiplier(), 1);
  QCOMPARE(s.occurrencePeriod(), Schedule::Occurrence::EveryHalfMonth);
  QCOMPARE(s.occurrence(), Schedule::Occurrence::EveryHalfMonth);
  s.setOccurrenceMultiplier(2);
  QCOMPARE(s.occurrenceMultiplier(), 2);
  QCOMPARE(s.occurrencePeriod(), Schedule::Occurrence::EveryHalfMonth);
  QCOMPARE(s.occurrence(), Schedule::Occurrence::EveryHalfMonth);

  s.setOccurrencePeriod(Schedule::Occurrence::Monthly);
  QCOMPARE(s.occurrencePeriod(), Schedule::Occurrence::Monthly);
  s.setOccurrenceMultiplier(1);
  QCOMPARE(s.occurrenceMultiplier(), 1);
  QCOMPARE(s.occurrencePeriod(), Schedule::Occurrence::Monthly);
  QCOMPARE(s.occurrence(), Schedule::Occurrence::Monthly);
  s.setOccurrenceMultiplier(2);
  QCOMPARE(s.occurrenceMultiplier(), 2);
  QCOMPARE(s.occurrencePeriod(), Schedule::Occurrence::Monthly);
  QCOMPARE(s.occurrence(), Schedule::Occurrence::EveryOtherMonth);
  s.setOccurrenceMultiplier(3);
  QCOMPARE(s.occurrenceMultiplier(), 3);
  QCOMPARE(s.occurrencePeriod(), Schedule::Occurrence::Monthly);
  QCOMPARE(s.occurrence(), Schedule::Occurrence::EveryThreeMonths);
  s.setOccurrenceMultiplier(4);
  QCOMPARE(s.occurrenceMultiplier(), 4);
  QCOMPARE(s.occurrencePeriod(), Schedule::Occurrence::Monthly);
  QCOMPARE(s.occurrence(), Schedule::Occurrence::EveryFourMonths);
  s.setOccurrenceMultiplier(5);
  QCOMPARE(s.occurrenceMultiplier(), 5);
  QCOMPARE(s.occurrencePeriod(), Schedule::Occurrence::Monthly);
  QCOMPARE(s.occurrence(), Schedule::Occurrence::Monthly);
  s.setOccurrenceMultiplier(6);
  QCOMPARE(s.occurrenceMultiplier(), 6);
  QCOMPARE(s.occurrencePeriod(), Schedule::Occurrence::Monthly);
  QCOMPARE(s.occurrence(), Schedule::Occurrence::TwiceYearly);

  s.setOccurrencePeriod(Schedule::Occurrence::Yearly);
  QCOMPARE(s.occurrencePeriod(), Schedule::Occurrence::Yearly);
  s.setOccurrenceMultiplier(1);
  QCOMPARE(s.occurrenceMultiplier(), 1);
  QCOMPARE(s.occurrencePeriod(), Schedule::Occurrence::Yearly);
  QCOMPARE(s.occurrence(), Schedule::Occurrence::Yearly);
  s.setOccurrenceMultiplier(2);
  QCOMPARE(s.occurrenceMultiplier(), 2);
  QCOMPARE(s.occurrencePeriod(), Schedule::Occurrence::Yearly);
  QCOMPARE(s.occurrence(), Schedule::Occurrence::EveryOtherYear);
  s.setOccurrenceMultiplier(3);
  QCOMPARE(s.occurrenceMultiplier(), 3);
  QCOMPARE(s.occurrencePeriod(), Schedule::Occurrence::Yearly);
  QCOMPARE(s.occurrence(), Schedule::Occurrence::Yearly);

  // Set occurrence: check occurrence, Period and Multiplier
  s.setOccurrence(Schedule::Occurrence::Once);
  QCOMPARE(s.occurrence(), Schedule::Occurrence::Once);
  QCOMPARE(s.occurrencePeriod(), Schedule::Occurrence::Once);
  QCOMPARE(s.occurrenceMultiplier(), 1);

  s.setOccurrence(Schedule::Occurrence::Daily);
  QCOMPARE(s.occurrence(), Schedule::Occurrence::Daily);
  QCOMPARE(s.occurrencePeriod(), Schedule::Occurrence::Daily);
  QCOMPARE(s.occurrenceMultiplier(), 1);
  s.setOccurrence(Schedule::Occurrence::EveryThirtyDays);
  QCOMPARE(s.occurrence(), Schedule::Occurrence::EveryThirtyDays);
  QCOMPARE(s.occurrencePeriod(), Schedule::Occurrence::Daily);
  QCOMPARE(s.occurrenceMultiplier(), 30);

  s.setOccurrence(Schedule::Occurrence::Weekly);
  QCOMPARE(s.occurrence(), Schedule::Occurrence::Weekly);
  QCOMPARE(s.occurrencePeriod(), Schedule::Occurrence::Weekly);
  QCOMPARE(s.occurrenceMultiplier(), 1);
  s.setOccurrence(Schedule::Occurrence::EveryOtherWeek);
  QCOMPARE(s.occurrence(), Schedule::Occurrence::EveryOtherWeek);
  QCOMPARE(s.occurrencePeriod(), Schedule::Occurrence::Weekly);
  QCOMPARE(s.occurrenceMultiplier(), 2);
  // Fortnightly no longer used: Every other week used instead
  s.setOccurrence(Schedule::Occurrence::Fortnightly);
  QCOMPARE(s.occurrence(), Schedule::Occurrence::EveryOtherWeek);
  QCOMPARE(s.occurrencePeriod(), Schedule::Occurrence::Weekly);
  QCOMPARE(s.occurrenceMultiplier(), 2);
  s.setOccurrence(Schedule::Occurrence::EveryThreeWeeks);
  QCOMPARE(s.occurrence(), Schedule::Occurrence::EveryThreeWeeks);
  QCOMPARE(s.occurrencePeriod(), Schedule::Occurrence::Weekly);
  QCOMPARE(s.occurrenceMultiplier(), 3);
  s.setOccurrence(Schedule::Occurrence::EveryFourWeeks);
  QCOMPARE(s.occurrence(), Schedule::Occurrence::EveryFourWeeks);
  QCOMPARE(s.occurrencePeriod(), Schedule::Occurrence::Weekly);
  QCOMPARE(s.occurrenceMultiplier(), 4);
  s.setOccurrence(Schedule::Occurrence::EveryEightWeeks);
  QCOMPARE(s.occurrence(), Schedule::Occurrence::EveryEightWeeks);
  QCOMPARE(s.occurrencePeriod(), Schedule::Occurrence::Weekly);
  QCOMPARE(s.occurrenceMultiplier(), 8);

  s.setOccurrence(Schedule::Occurrence::EveryHalfMonth);
  QCOMPARE(s.occurrence(), Schedule::Occurrence::EveryHalfMonth);
  QCOMPARE(s.occurrencePeriod(), Schedule::Occurrence::EveryHalfMonth);
  QCOMPARE(s.occurrenceMultiplier(), 1);

  s.setOccurrence(Schedule::Occurrence::Monthly);
  QCOMPARE(s.occurrence(), Schedule::Occurrence::Monthly);
  QCOMPARE(s.occurrencePeriod(), Schedule::Occurrence::Monthly);
  QCOMPARE(s.occurrenceMultiplier(), 1);
  s.setOccurrence(Schedule::Occurrence::EveryOtherMonth);
  QCOMPARE(s.occurrence(), Schedule::Occurrence::EveryOtherMonth);
  QCOMPARE(s.occurrencePeriod(), Schedule::Occurrence::Monthly);
  QCOMPARE(s.occurrenceMultiplier(), 2);
  s.setOccurrence(Schedule::Occurrence::EveryThreeMonths);
  QCOMPARE(s.occurrence(), Schedule::Occurrence::EveryThreeMonths);
  QCOMPARE(s.occurrencePeriod(), Schedule::Occurrence::Monthly);
  QCOMPARE(s.occurrenceMultiplier(), 3);
  // Quarterly no longer used.  Every three months used instead
  s.setOccurrence(Schedule::Occurrence::Quarterly);
  QCOMPARE(s.occurrence(), Schedule::Occurrence::EveryThreeMonths);
  QCOMPARE(s.occurrencePeriod(), Schedule::Occurrence::Monthly);
  QCOMPARE(s.occurrenceMultiplier(), 3);
  s.setOccurrence(Schedule::Occurrence::EveryFourMonths);
  QCOMPARE(s.occurrence(), Schedule::Occurrence::EveryFourMonths);
  QCOMPARE(s.occurrencePeriod(), Schedule::Occurrence::Monthly);
  QCOMPARE(s.occurrenceMultiplier(), 4);
  s.setOccurrence(Schedule::Occurrence::TwiceYearly);
  QCOMPARE(s.occurrence(), Schedule::Occurrence::TwiceYearly);
  QCOMPARE(s.occurrencePeriod(), Schedule::Occurrence::Monthly);
  QCOMPARE(s.occurrenceMultiplier(), 6);

  s.setOccurrence(Schedule::Occurrence::Yearly);
  QCOMPARE(s.occurrence(), Schedule::Occurrence::Yearly);
  QCOMPARE(s.occurrencePeriod(), Schedule::Occurrence::Yearly);
  QCOMPARE(s.occurrenceMultiplier(), 1);
  s.setOccurrence(Schedule::Occurrence::EveryOtherYear);
  QCOMPARE(s.occurrence(), Schedule::Occurrence::EveryOtherYear);
  QCOMPARE(s.occurrencePeriod(), Schedule::Occurrence::Yearly);
  QCOMPARE(s.occurrenceMultiplier(), 2);
}

void MyMoneyScheduleTest::testSimpleToFromCompoundOccurrence()
{
  // Conversion between Simple and Compound occurrences
  // Each simple occurrence to compound occurrence
  Schedule::Occurrence occ;
  int mult;
  occ = Schedule::Occurrence::Once; mult = 1;
  MyMoneySchedule::simpleToCompoundOccurrence(mult, occ);
  QVERIFY(occ == Schedule::Occurrence::Once && mult == 1);
  occ = Schedule::Occurrence::Daily; mult = 1;
  MyMoneySchedule::simpleToCompoundOccurrence(mult, occ);
  QVERIFY(occ == Schedule::Occurrence::Daily && mult == 1);
  occ = Schedule::Occurrence::Weekly; mult = 1;
  MyMoneySchedule::simpleToCompoundOccurrence(mult, occ);
  QVERIFY(occ == Schedule::Occurrence::Weekly && mult == 1);
  occ = Schedule::Occurrence::EveryOtherWeek; mult = 1;
  MyMoneySchedule::simpleToCompoundOccurrence(mult, occ);
  QVERIFY(occ == Schedule::Occurrence::Weekly && mult == 2);
  occ = Schedule::Occurrence::Fortnightly; mult = 1;
  MyMoneySchedule::simpleToCompoundOccurrence(mult, occ);
  QVERIFY(occ == Schedule::Occurrence::Weekly && mult == 2);
  occ = Schedule::Occurrence::EveryHalfMonth; mult = 1;
  MyMoneySchedule::simpleToCompoundOccurrence(mult, occ);
  QVERIFY(occ == Schedule::Occurrence::EveryHalfMonth && mult == 1);
  occ = Schedule::Occurrence::EveryThreeWeeks; mult = 1;
  MyMoneySchedule::simpleToCompoundOccurrence(mult, occ);
  QVERIFY(occ == Schedule::Occurrence::Weekly && mult == 3);
  occ = Schedule::Occurrence::EveryFourWeeks; mult = 1;
  MyMoneySchedule::simpleToCompoundOccurrence(mult, occ);
  QVERIFY(occ == Schedule::Occurrence::Weekly && mult == 4);
  occ = Schedule::Occurrence::EveryThirtyDays; mult = 1;
  MyMoneySchedule::simpleToCompoundOccurrence(mult, occ);
  QVERIFY(occ == Schedule::Occurrence::Daily && mult == 30);
  occ = Schedule::Occurrence::Monthly; mult = 1;
  MyMoneySchedule::simpleToCompoundOccurrence(mult, occ);
  QVERIFY(occ == Schedule::Occurrence::Monthly && mult == 1);
  occ = Schedule::Occurrence::EveryEightWeeks; mult = 1;
  MyMoneySchedule::simpleToCompoundOccurrence(mult, occ);
  QVERIFY(occ == Schedule::Occurrence::Weekly && mult == 8);
  occ = Schedule::Occurrence::EveryOtherMonth; mult = 1;
  MyMoneySchedule::simpleToCompoundOccurrence(mult, occ);
  QVERIFY(occ == Schedule::Occurrence::Monthly && mult == 2);
  occ = Schedule::Occurrence::EveryThreeMonths; mult = 1;
  MyMoneySchedule::simpleToCompoundOccurrence(mult, occ);
  QVERIFY(occ == Schedule::Occurrence::Monthly && mult == 3);
  occ = Schedule::Occurrence::Quarterly; mult = 1;
  MyMoneySchedule::simpleToCompoundOccurrence(mult, occ);
  QVERIFY(occ == Schedule::Occurrence::Monthly && mult == 3);
  occ = Schedule::Occurrence::EveryFourMonths; mult = 1;
  MyMoneySchedule::simpleToCompoundOccurrence(mult, occ);
  QVERIFY(occ == Schedule::Occurrence::Monthly && mult == 4);
  occ = Schedule::Occurrence::TwiceYearly; mult = 1;
  MyMoneySchedule::simpleToCompoundOccurrence(mult, occ);
  QVERIFY(occ == Schedule::Occurrence::Monthly && mult == 6);
  occ = Schedule::Occurrence::Yearly; mult = 1;
  MyMoneySchedule::simpleToCompoundOccurrence(mult, occ);
  QVERIFY(occ == Schedule::Occurrence::Yearly && mult == 1);
  occ = Schedule::Occurrence::EveryOtherYear; mult = 1;
  MyMoneySchedule::simpleToCompoundOccurrence(mult, occ);
  QVERIFY(occ == Schedule::Occurrence::Yearly && mult == 2);
  // Compound to Simple Occurrences
  occ = Schedule::Occurrence::Once; mult = 1;
  MyMoneySchedule::compoundToSimpleOccurrence(mult, occ);
  QVERIFY(occ == Schedule::Occurrence::Once && mult == 1);
  occ = Schedule::Occurrence::Daily; mult = 1;
  MyMoneySchedule::compoundToSimpleOccurrence(mult, occ);
  QVERIFY(occ == Schedule::Occurrence::Daily && mult == 1);
  occ = Schedule::Occurrence::Weekly; mult = 1;
  MyMoneySchedule::compoundToSimpleOccurrence(mult, occ);
  QVERIFY(occ == Schedule::Occurrence::Weekly && mult == 1);
  occ = Schedule::Occurrence::Weekly; mult = 2;
  MyMoneySchedule::compoundToSimpleOccurrence(mult, occ);
  QVERIFY(occ == Schedule::Occurrence::EveryOtherWeek && mult == 1);
  // Schedule::Occurrence::Fortnightly not converted back
  occ = Schedule::Occurrence::EveryHalfMonth; mult = 1;
  MyMoneySchedule::compoundToSimpleOccurrence(mult, occ);
  QVERIFY(occ == Schedule::Occurrence::EveryHalfMonth && mult == 1);
  occ = Schedule::Occurrence::Weekly; mult = 3;
  MyMoneySchedule::compoundToSimpleOccurrence(mult, occ);
  QVERIFY(occ == Schedule::Occurrence::EveryThreeWeeks && mult == 1);
  occ = Schedule::Occurrence::Weekly ; mult = 4;
  MyMoneySchedule::compoundToSimpleOccurrence(mult, occ);
  QVERIFY(occ == Schedule::Occurrence::EveryFourWeeks && mult == 1);
  occ = Schedule::Occurrence::Daily; mult = 30;
  MyMoneySchedule::compoundToSimpleOccurrence(mult, occ);
  QVERIFY(occ == Schedule::Occurrence::EveryThirtyDays && mult == 1);
  occ = Schedule::Occurrence::Monthly; mult = 1;
  MyMoneySchedule::compoundToSimpleOccurrence(mult, occ);
  QVERIFY(occ == Schedule::Occurrence::Monthly && mult == 1);
  occ = Schedule::Occurrence::Weekly; mult = 8;
  MyMoneySchedule::compoundToSimpleOccurrence(mult, occ);
  QVERIFY(occ == Schedule::Occurrence::EveryEightWeeks && mult == 1);
  occ = Schedule::Occurrence::Monthly; mult = 2;
  MyMoneySchedule::compoundToSimpleOccurrence(mult, occ);
  QVERIFY(occ == Schedule::Occurrence::EveryOtherMonth && mult == 1);
  occ = Schedule::Occurrence::Monthly; mult = 3;
  MyMoneySchedule::compoundToSimpleOccurrence(mult, occ);
  QVERIFY(occ == Schedule::Occurrence::EveryThreeMonths && mult == 1);
  // Schedule::Occurrence::Quarterly not converted back
  occ = Schedule::Occurrence::Monthly; mult = 4;
  MyMoneySchedule::compoundToSimpleOccurrence(mult, occ);
  QVERIFY(occ == Schedule::Occurrence::EveryFourMonths && mult == 1);
  occ = Schedule::Occurrence::Monthly; mult = 6;
  MyMoneySchedule::compoundToSimpleOccurrence(mult, occ);
  QVERIFY(occ == Schedule::Occurrence::TwiceYearly && mult == 1);
  occ = Schedule::Occurrence::Yearly; mult = 1;
  MyMoneySchedule::compoundToSimpleOccurrence(mult, occ);
  QVERIFY(occ == Schedule::Occurrence::Yearly && mult == 1);
  occ = Schedule::Occurrence::Yearly; mult = 2;
  MyMoneySchedule::compoundToSimpleOccurrence(mult, occ);
  QVERIFY(occ == Schedule::Occurrence::EveryOtherYear && mult == 1);
}

void MyMoneyScheduleTest::testProcessingDates()
{
  // There should be no processing calendar defined so
  // make sure fall back works

  MyMoneySchedule s;
  // Check there is no processing caledar defined.
  QVERIFY(s.processingCalendar() == nullptr);
  // This should be a processing day.
  QCOMPARE(s.isProcessingDate(QDate(2009, 12, 31)), true);
  // This should be a processing day when there is no calendar.
  QCOMPARE(s.isProcessingDate(QDate(2010, 1, 1)), true);
  // This should be a non-processing day as it is on a weekend.
  QCOMPARE(s.isProcessingDate(QDate(2010, 1, 2)), false);
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
    QCOMPARE(sch.isFinished(), true);
    QCOMPARE(sch.occurrencePeriod(), Schedule::Occurrence::Monthly);
    QCOMPARE(sch.paymentDates(QDate::currentDate(), QDate::currentDate().addDays(21)).count(), 0);
  } catch (const MyMoneyException &) {
    QFAIL("Unexpected exception");
  }

}

void MyMoneyScheduleTest::testAdjustedNextPayment()
{
  MyMoneySchedule s;

  QDate dueDate(2010, 5, 23);
  QDate adjustedDueDate(2010, 5, 21);
  s.setNextDueDate(dueDate);
  s.setOccurrence(Schedule::Occurrence::Monthly);
  s.setWeekendOption(Schedule::WeekendOption::MoveBefore);

  //if adjustedNextPayment works ok with adjusted date prior to the current date, it should return 2010-06-23
  QDate nextDueDate(2010, 6, 23);
  //this is the current behaviour, and it is wrong
  //QCOMPARE(s.adjustedNextPayment(adjustedDueDate), adjustedDueDate);

  //this is the expected behaviour
  QCOMPARE(s.adjustedNextPayment(s.adjustedNextDueDate()), s.adjustedDate(nextDueDate, s.weekendOption()));
}

void MyMoneyScheduleTest::testReplaceId()
{
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
    QCOMPARE(sch.transaction().postDate().isValid(), false);
    QCOMPARE(sch.transaction().splits()[0].accountId(), QLatin1String("A000076"));
    QCOMPARE(sch.transaction().splits()[1].accountId(), QLatin1String("A000276"));
    QCOMPARE(sch.replaceId(QLatin1String("A000079"), QLatin1String("A000076")), true);
    QCOMPARE(sch.transaction().splits()[0].accountId(), QLatin1String("A000079"));
    QCOMPARE(sch.transaction().splits()[1].accountId(), QLatin1String("A000276"));

  } catch (const MyMoneyException &) {
    QFAIL("Unexpected exception");
  }

}

void MyMoneyScheduleTest::testAdjustedWhenItWillEnd()
{
  MyMoneySchedule s;

  QDate endDate(2011, 8, 13); // this is a nonprocessing day because
  // it's a Saturday
  QDate refDate(2011, 8, 10); // just some ref date before the last payment

  s.setStartDate(endDate.addMonths(-1));
  s.setOccurrence(Schedule::Occurrence::Monthly);
  s.setEndDate(endDate);
  // the next due date is on this day but the policy is to move the
  // schedule to the next processing day (Monday)
  s.setWeekendOption(Schedule::WeekendOption::MoveAfter);
  s.setNextDueDate(endDate);

  // the payment should be found between the respective date and one month after
  QCOMPARE(s.paymentDates(endDate, endDate.addMonths(1)).count(), 1);

  // the next payment must be the final one
  QCOMPARE(s.nextPayment(refDate), endDate);

  // and since it is on a Saturday, the adjusted one must be on the
  // following Monday
  QCOMPARE(s.adjustedNextPayment(refDate), endDate.addDays(2));

  // reference for Sunday is still OK
  QCOMPARE(s.adjustedNextPayment(QDate(2011, 8, 14)), endDate.addDays(2));

  // but it is finished on Monday (as reference date)
  QVERIFY(!s.adjustedNextPayment(QDate(2011, 8, 15)).isValid());

  // check the # of remaining transactions
  s.setNextDueDate(endDate.addMonths(-1));
  QCOMPARE(s.transactionsRemaining(), 2);
}

void MyMoneyScheduleTest::testElementNames()
{
  for (auto i = (int)Schedule::Element::Payment; i <= (int)Schedule::Element::Payments; ++i) {
    auto isEmpty = MyMoneySchedulePrivate::getElName(static_cast<Schedule::Element>(i)).isEmpty();
    if (isEmpty)
      qWarning() << "Empty element's name " << i;
    QVERIFY(!isEmpty);
  }
}

void MyMoneyScheduleTest::testAttributeNames()
{
  for (auto i = (int)Schedule::Attribute::Name; i < (int)Schedule::Attribute::LastAttribute; ++i) {
    auto isEmpty = MyMoneySchedulePrivate::getAttrName(static_cast<Schedule::Attribute>(i)).isEmpty();
    if (isEmpty)
      qWarning() << "Empty attribute's name " << i;
    QVERIFY(!isEmpty);
  }
}

void MyMoneyScheduleTest::testProcessLastDayInMonth()
{
  MyMoneySchedule s;
  // occurrence is unrelated
  s.setOccurrence(Schedule::Occurrence::Any);
  s.setLastDayInMonth(true);
  s.setNextDueDate(QDate(2010, 1, 1));
  QCOMPARE(s.adjustedNextDueDate(), QDate(2010,1,31));
  s.setNextDueDate(QDate(2010, 2, 1));
  QCOMPARE(s.adjustedNextDueDate(), QDate(2010,2,28));
  s.setNextDueDate(QDate(2016, 2, 1));
  QCOMPARE(s.adjustedNextDueDate(), QDate(2016,2,29));
  s.setNextDueDate(QDate(2016, 4, 1));
  QCOMPARE(s.adjustedNextDueDate(), QDate(2016,4,30));
  s.setLastDayInMonth(false);
  QCOMPARE(s.adjustedNextDueDate(), QDate(2016,4,1));
}
