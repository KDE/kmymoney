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
#include "mymoneyexception.h"
#include "mymoneytransaction.h"
#include "mymoneytransaction_p.h"
#include "storage/mymoneystoragemgr.h"

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
  /// @todo extend test to cover application usage (with a processing calendar defined) ?

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
