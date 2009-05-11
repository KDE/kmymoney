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
// Include internationalization
#include <klocale.h>

#include "mymoneyscheduletest.h"

#include "mymoneysplit.h"
#include "mymoneymoney.h"

#include <iostream>
//Added by qt3to4:
#include <Q3ValueList>

MyMoneyScheduleTest::MyMoneyScheduleTest()
{
}


void MyMoneyScheduleTest::setUp () {
}

void MyMoneyScheduleTest::tearDown () {
}

void MyMoneyScheduleTest::testEmptyConstructor() {
	MyMoneySchedule s;
	
	CPPUNIT_ASSERT(s.id().isEmpty());
	CPPUNIT_ASSERT(s.m_occurence == MyMoneySchedule::OCCUR_ANY);
	CPPUNIT_ASSERT(s.m_type == MyMoneySchedule::TYPE_ANY);
	CPPUNIT_ASSERT(s.m_paymentType == MyMoneySchedule::STYPE_ANY);
	CPPUNIT_ASSERT(s.m_fixed == false);
	CPPUNIT_ASSERT(!s.m_startDate.isValid());
	CPPUNIT_ASSERT(!s.m_endDate.isValid());
	CPPUNIT_ASSERT(!s.m_lastPayment.isValid());
	CPPUNIT_ASSERT(s.m_autoEnter == false);
	CPPUNIT_ASSERT(s.m_name.isEmpty());
	CPPUNIT_ASSERT(s.willEnd() == false);
}

void MyMoneyScheduleTest::testConstructor() {
	MyMoneySchedule s(	"A Name",
				MyMoneySchedule::TYPE_BILL,
				MyMoneySchedule::OCCUR_WEEKLY, 1,
				MyMoneySchedule::STYPE_DIRECTDEBIT,
				QDate::currentDate(),
				QDate(),
				true,
				true);

	CPPUNIT_ASSERT(s.type() == MyMoneySchedule::TYPE_BILL);
	CPPUNIT_ASSERT(s.occurence() == MyMoneySchedule::OCCUR_WEEKLY);
	CPPUNIT_ASSERT(s.occurenceMultiplier() == 1 );
	CPPUNIT_ASSERT(s.paymentType() == MyMoneySchedule::STYPE_DIRECTDEBIT);
	CPPUNIT_ASSERT(s.startDate() == QDate());
	CPPUNIT_ASSERT(s.willEnd() == false);
	CPPUNIT_ASSERT(s.isFixed() == true);
	CPPUNIT_ASSERT(s.autoEnter() == true);
	CPPUNIT_ASSERT(s.name() == "A Name");
	CPPUNIT_ASSERT(!s.m_endDate.isValid());
	CPPUNIT_ASSERT(!s.m_lastPayment.isValid());
}

void MyMoneyScheduleTest::testSetFunctions() {
	MyMoneySchedule s;
	
	s.setId("SCHED001");
	CPPUNIT_ASSERT(s.id() == "SCHED001");
	
	s.setType(MyMoneySchedule::TYPE_BILL);
	CPPUNIT_ASSERT(s.type() == MyMoneySchedule::TYPE_BILL);

	s.setEndDate(QDate::currentDate());
	CPPUNIT_ASSERT(s.endDate() == QDate::currentDate());
	CPPUNIT_ASSERT(s.willEnd() == true);
}

void MyMoneyScheduleTest::testCopyConstructor() {
	MyMoneySchedule s;
	
	s.setId("SCHED001");
	s.setType(MyMoneySchedule::TYPE_BILL);
	
	MyMoneySchedule s2(s);

	CPPUNIT_ASSERT(s.id() == s2.id());
	CPPUNIT_ASSERT(s.type() == s2.type());
}

void MyMoneyScheduleTest::testAssignmentConstructor() {
	MyMoneySchedule s;
	
	s.setId("SCHED001");
	s.setType(MyMoneySchedule::TYPE_BILL);
	
	MyMoneySchedule s2 = s;

	CPPUNIT_ASSERT(s.id() == s2.id());
	CPPUNIT_ASSERT(s.type() == s2.type());
}

void MyMoneyScheduleTest::testSingleton() {
/*
	MyMoneyScheduled *m = MyMoneyScheduled::instance();
	CPPUNIT_ASSERT(m!=NULL);

	CPPUNIT_ASSERT(m->m_instance != NULL);
	CPPUNIT_ASSERT(m->m_nextId == 1);
*/
}

void MyMoneyScheduleTest::testAddSchedule()
{
/*
	MyMoneyScheduled *m = MyMoneyScheduled::instance();
	CPPUNIT_ASSERT(m!=NULL);
	
	try {

  	MyMoneySplit sp1;
  	sp1.setShares(MyMoneyMoney(1));
  	sp1.setValue(MyMoneyMoney(1));
  	sp1.setAccountId("MTE1");
  	sp1.setMemo("MTE1");
  	sp1.setPayeeId("MTE1");

  	MyMoneySplit sp2;
  	sp2.setShares(MyMoneyMoney(1));
  	sp2.setValue(MyMoneyMoney(1));
  	sp2.setAccountId("MTE2");
  	sp2.setMemo("MTE2");
  	sp2.setPayeeId("MTE2");

  	MyMoneyTransaction t;
  	t.addSplit(sp1);
  	t.addSplit(sp2);

  	MyMoneySchedule s1(	"s1",
				MyMoneySchedule::TYPE_BILL,
  				MyMoneySchedule::OCCUR_WEEKLY, 1,
  				MyMoneySchedule::STYPE_DIRECTDEBIT,
  				QDate(2001, 1, 1),
  				false,
  				true,
  				true);
  	s1.setTransaction(t);
  	MyMoneySchedule s2(	"s2",
				MyMoneySchedule::TYPE_DEPOSIT,
  				MyMoneySchedule::OCCUR_MONTHLY, 1,
  				MyMoneySchedule::STYPE_MANUALDEPOSIT,
  				QDate(2001, 2, 1),
  				false,
  				true,
  				true);
  	s2.setTransaction(t);
  	MyMoneySchedule s3(	"s3",
				MyMoneySchedule::TYPE_TRANSFER,
  				MyMoneySchedule::OCCUR_YEARLY, 1,
  				MyMoneySchedule::STYPE_WRITECHEQUE,
  				QDate(2001, 3, 1),
  				false,
  				true,
  				true);
  	s3.setTransaction(t);

		
		m->addSchedule("A000001", s1);
		m->addSchedule("A000001", s2);
		m->addSchedule("A000001", s3);
	} catch(MyMoneyException *e) {
		char buf[256];
		sprintf(buf, "Unexpected exception: %s", e->what().toLatin1());
		CPPUNIT_FAIL(buf);
		delete e;
	}

	CPPUNIT_ASSERT(m->m_nextId == 4);
	CPPUNIT_ASSERT(m->m_accountsScheduled["A000001"].size() == 3);
*/
}

void MyMoneyScheduleTest::testAnyScheduled()
{
/*
	MyMoneyScheduled *m = MyMoneyScheduled::instance();
	CPPUNIT_ASSERT(m!=NULL);

	// Successes
	CPPUNIT_ASSERT(m->anyScheduled("A000001"));
	CPPUNIT_ASSERT(m->anyScheduled("A000001", MyMoneySchedule::TYPE_BILL));
	CPPUNIT_ASSERT(m->anyScheduled("A000001", MyMoneySchedule::TYPE_DEPOSIT));
	CPPUNIT_ASSERT(m->anyScheduled("A000001", MyMoneySchedule::TYPE_TRANSFER));
	CPPUNIT_ASSERT(m->anyScheduled("A000001", MyMoneySchedule::TYPE_ANY,
				MyMoneySchedule::OCCUR_MONTHLY));
	CPPUNIT_ASSERT(m->anyScheduled("A000001", MyMoneySchedule::TYPE_ANY,
				MyMoneySchedule::OCCUR_WEEKLY));
	CPPUNIT_ASSERT(m->anyScheduled("A000001", MyMoneySchedule::TYPE_ANY,
				MyMoneySchedule::OCCUR_YEARLY));
	CPPUNIT_ASSERT(m->anyScheduled("A000001", MyMoneySchedule::TYPE_ANY,
				MyMoneySchedule::OCCUR_ANY,
				MyMoneySchedule::STYPE_DIRECTDEBIT));
	CPPUNIT_ASSERT(m->anyScheduled("A000001", MyMoneySchedule::TYPE_ANY,
				MyMoneySchedule::OCCUR_ANY,
				MyMoneySchedule::STYPE_MANUALDEPOSIT));
	CPPUNIT_ASSERT(m->anyScheduled("A000001", MyMoneySchedule::TYPE_ANY,
				MyMoneySchedule::OCCUR_ANY,
				MyMoneySchedule::STYPE_WRITECHEQUE));

	// Failures
	CPPUNIT_ASSERT(m->anyScheduled("A000001", MyMoneySchedule::TYPE_BILL,
				MyMoneySchedule::OCCUR_MONTHLY) == false);
*/
}

void MyMoneyScheduleTest::testOverdue()
{
	MyMoneySchedule sch_overdue;
	MyMoneySchedule sch_intime;

	// the following checks only work correctly, if currentDate() is
	// between the 1st and 27th. If it is between 28th and 31st
	// we don't perform them. Note: this should be fixed.
	if(QDate::currentDate().day() > 27 || QDate::currentDate().day() == 1) {
		std::cout << std::endl << "testOverdue() skipped because current day is between 28th and 2nd" << std::endl;
		return;
	}

	QDate startDate = QDate::currentDate().addDays(-1).addMonths(-23);
	QDate lastPaymentDate = QDate::currentDate().addDays(-1).addMonths(-1);

	QString ref = QString(
		"<!DOCTYPE TEST>\n"
		"<SCHEDULE-CONTAINER>\n"
		" <SCHEDULED_TX startDate=\"%1\" autoEnter=\"0\" weekendOption=\"2\" lastPayment=\"%2\" paymentType=\"8\" endDate=\"\" type=\"5\" id=\"SCH0002\" name=\"A Name\" fixed=\"0\" occurenceMultiplier=\"1\" occurence=\"32\" >\n"
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

		CPPUNIT_ASSERT(sch_overdue.isOverdue() == true);
		CPPUNIT_ASSERT(sch_intime.isOverdue() == false);

	} catch(MyMoneyException *e) {
		delete e;
		CPPUNIT_FAIL("Unexpected exception");
	}
/*
	MyMoneyScheduled *m = MyMoneyScheduled::instance();
	CPPUNIT_ASSERT(m!=NULL);

	try
	{
		CPPUNIT_ASSERT(m->anyOverdue("A000001"));
		CPPUNIT_ASSERT(m->anyOverdue("A000001", MyMoneySchedule::TYPE_BILL));
		CPPUNIT_ASSERT(m->anyOverdue("A000001", MyMoneySchedule::TYPE_TRANSFER));
		CPPUNIT_ASSERT(m->anyOverdue("A000001", MyMoneySchedule::TYPE_DEPOSIT));
	} catch(MyMoneyException *e) {
		char buf[256];
		sprintf(buf, "Unexpected exception: %s", e->what().toLatin1());
		CPPUNIT_FAIL(buf);
		delete e;
	}
*/
}

void MyMoneyScheduleTest::testGetSchedule()
{
/*
	MyMoneyScheduled *m = MyMoneyScheduled::instance();
	CPPUNIT_ASSERT(m!=NULL);

	try
	{
		MyMoneySchedule s = m->getSchedule("A000001", "SCHED00002");

		CPPUNIT_ASSERT(s.type() == MyMoneySchedule::TYPE_DEPOSIT);
		CPPUNIT_ASSERT(s.occurence() == MyMoneySchedule::OCCUR_MONTHLY);
    CPPUNIT_ASSERT(s.occurenceMultiplier() == 1);
		CPPUNIT_ASSERT(s.paymentType() == MyMoneySchedule::STYPE_MANUALDEPOSIT);
		CPPUNIT_ASSERT(s.startDate() == QDate(2001, 2, 1));
		CPPUNIT_ASSERT(s.willEnd() == false);
		CPPUNIT_ASSERT(s.isFixed() == true);
		CPPUNIT_ASSERT(s.autoEnter() == true);

		MyMoneyTransaction t = s.transaction();
		CPPUNIT_ASSERT(t.splitCount() == 2);

		s = m->getSchedule("A000001", "SCHED00005");

		CPPUNIT_FAIL("Exception expected while getting schedule SCHED00005");
	} catch (MyMoneyException *e)
	{
		delete e;
	}
*/
}

void MyMoneyScheduleTest::testGetScheduled()
{
/*
	MyMoneyScheduled *m = MyMoneyScheduled::instance();
	CPPUNIT_ASSERT(m!=NULL);

	try
	{
		QValueList<QString> testList;

		testList = m->getScheduled("A000001");
		CPPUNIT_ASSERT(testList.size() == 3);
		CPPUNIT_ASSERT(testList[0] == "SCHED00001");
		CPPUNIT_ASSERT(testList[1] == "SCHED00002");
		CPPUNIT_ASSERT(testList[2] == "SCHED00003");

		testList = m->getScheduled("A000001", MyMoneySchedule::TYPE_DEPOSIT);
		CPPUNIT_ASSERT(testList.size() == 1);
		CPPUNIT_ASSERT(testList[0] == "SCHED00002");

		testList = m->getScheduled("A000001", MyMoneySchedule::TYPE_BILL);
		CPPUNIT_ASSERT(testList.size() == 1);
		CPPUNIT_ASSERT(testList[0] == "SCHED00001");

		testList = m->getScheduled("A000001", MyMoneySchedule::TYPE_TRANSFER);
		CPPUNIT_ASSERT(testList.size() == 1);
		CPPUNIT_ASSERT(testList[0] == "SCHED00003");

		testList = m->getScheduled("A000001", MyMoneySchedule::TYPE_DEPOSIT,
				MyMoneySchedule::STYPE_MANUALDEPOSIT,
				MyMoneySchedule::OCCUR_MONTHLY);
		CPPUNIT_ASSERT(testList.size() == 1);
		CPPUNIT_ASSERT(testList[0] == "SCHED00002");

		testList = m->getScheduled("A000001", QDate(2001, 1, 1), QDate(2001, 2, 1));
		CPPUNIT_ASSERT(testList.size() == 2);
		CPPUNIT_ASSERT(testList[0] == "SCHED00001");
		CPPUNIT_ASSERT(testList[1] == "SCHED00002");

	} catch(MyMoneyException *e) {
		char buf[256];
		sprintf(buf, "Unexpected exception: %s", e->what().toLatin1());
		CPPUNIT_FAIL(buf);
		delete e;
	}
*/
}

void MyMoneyScheduleTest::testGetOverdue()
{
/*
	MyMoneyScheduled *m = MyMoneyScheduled::instance();
	CPPUNIT_ASSERT(m!=NULL);

	try
	{
		QValueList<QString> testList;

		testList = m->getOverdue("A000001");
		CPPUNIT_ASSERT(testList.size() == 3);
		CPPUNIT_ASSERT(testList[0] == "SCHED00001");
		CPPUNIT_ASSERT(testList[1] == "SCHED00002");
		CPPUNIT_ASSERT(testList[2] == "SCHED00003");

		testList = m->getOverdue("A000001", MyMoneySchedule::TYPE_DEPOSIT);
		CPPUNIT_ASSERT(testList.size() == 1);
		CPPUNIT_ASSERT(testList[0] == "SCHED00002");

		testList = m->getOverdue("A000001", MyMoneySchedule::TYPE_BILL);
		CPPUNIT_ASSERT(testList.size() == 1);
		CPPUNIT_ASSERT(testList[0] == "SCHED00001");

		testList = m->getOverdue("A000001", MyMoneySchedule::TYPE_TRANSFER);
		CPPUNIT_ASSERT(testList.size() == 1);
		CPPUNIT_ASSERT(testList[0] == "SCHED00003");

		testList = m->getOverdue("A000001", MyMoneySchedule::TYPE_DEPOSIT,
				MyMoneySchedule::STYPE_MANUALDEPOSIT,
				MyMoneySchedule::OCCUR_MONTHLY);
		CPPUNIT_ASSERT(testList.size() == 1);
		CPPUNIT_ASSERT(testList[0] == "SCHED00002");
	} catch(MyMoneyException *e) {
		char buf[256];
		sprintf(buf, "Unexpected exception: %s", e->what().toLatin1());
		CPPUNIT_FAIL(buf);
		delete e;
	}
*/
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
		"<SCHEDULED_TX startDate=\"2007-02-17\" autoEnter=\"1\" weekendOption=\"2\" lastPayment=\"\" paymentType=\"1\" endDate=\"\" type=\"1\" id=\"SCH000058\" name=\"Car Tax\" fixed=\"1\" occurenceMultiplier=\"1\" occurence=\"16384\" >\n"
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
		CPPUNIT_ASSERT(sch.nextPayment(QDate(2007,2,14)) == QDate(2007,2,17));
		CPPUNIT_ASSERT(sch.nextPayment(QDate(2007,2,17)) == QDate(2008,2,17));
		CPPUNIT_ASSERT(sch.nextPayment(QDate(2007,2,18)) == QDate(2008,2,17));

	} catch(MyMoneyException *e) {
		delete e;
		CPPUNIT_FAIL("Unexpected exception");
	}
/*
	MyMoneyScheduled *m = MyMoneyScheduled::instance();
	CPPUNIT_ASSERT(m!=NULL);

	try
	{
		MyMoneySchedule s1 = m->getSchedule("A000001", "SCHED00001");
		MyMoneySchedule s2 = m->getSchedule("A000001", "SCHED00002");
		MyMoneySchedule s3 = m->getSchedule("A000001", "SCHED00003");

		QDate nextPayment1 = s1.nextPayment();
		QDate nextPayment2 = s2.nextPayment();
		QDate nextPayment3 = s3.nextPayment();
		
		CPPUNIT_ASSERT(nextPayment1.year() != 1900);
		CPPUNIT_ASSERT(nextPayment2.year() != 1900);
		CPPUNIT_ASSERT(nextPayment3.year() != 1900);
	} catch (MyMoneyException *e)
	{
		CPPUNIT_FAIL("Unexpected exception");
		delete e;
	}
*/
}

void MyMoneyScheduleTest::testAddHalfMonths()
{
  // addHalfMonths is private
  // Test a Schedule with occurence OCCUR_EVERYHALFMONTH using nextPayment
  MyMoneySchedule s;
  s.setStartDate(QDate(2007, 1, 1));
  s.setOccurence(MyMoneySchedule::OCCUR_EVERYHALFMONTH);
  s.setNextDueDate(s.startDate());
  s.setLastPayment(s.startDate());
  
  QString format("yyyy-MM-dd");
  CPPUNIT_ASSERT( s.nextPayment(s.nextDueDate()).toString(format) == "2007-01-16" );
  s.setNextDueDate(QDate(2007, 1, 2));
  CPPUNIT_ASSERT( s.nextPayment(s.nextDueDate()).toString(format) == "2007-01-17" );
  s.setNextDueDate(QDate(2007, 1, 3));
  CPPUNIT_ASSERT( s.nextPayment(s.nextDueDate()).toString(format) == "2007-01-18" );
  s.setNextDueDate(QDate(2007, 1, 4));
  CPPUNIT_ASSERT( s.nextPayment(s.nextDueDate()).toString(format) == "2007-01-19" );
  s.setNextDueDate(QDate(2007, 1, 5));
  CPPUNIT_ASSERT( s.nextPayment(s.nextDueDate()).toString(format) == "2007-01-20" );
  s.setNextDueDate(QDate(2007, 1, 6));
  CPPUNIT_ASSERT( s.nextPayment(s.nextDueDate()).toString(format) == "2007-01-21" );
  s.setNextDueDate(QDate(2007, 1, 7));
  CPPUNIT_ASSERT( s.nextPayment(s.nextDueDate()).toString(format) == "2007-01-22" );
  s.setNextDueDate(QDate(2007, 1, 8));
  CPPUNIT_ASSERT( s.nextPayment(s.nextDueDate()).toString(format) == "2007-01-23" );
  s.setNextDueDate(QDate(2007, 1, 9));
  CPPUNIT_ASSERT( s.nextPayment(s.nextDueDate()).toString(format) == "2007-01-24" );
  s.setNextDueDate(QDate(2007, 1, 10));
  CPPUNIT_ASSERT( s.nextPayment(s.nextDueDate()).toString(format) == "2007-01-25" );
  s.setNextDueDate(QDate(2007, 1, 11));
  CPPUNIT_ASSERT( s.nextPayment(s.nextDueDate()).toString(format) == "2007-01-26" );
  s.setNextDueDate(QDate(2007, 1, 12));
  CPPUNIT_ASSERT( s.nextPayment(s.nextDueDate()).toString(format) == "2007-01-27" );
  s.setNextDueDate(QDate(2007, 1, 13));
  CPPUNIT_ASSERT( s.nextPayment(s.nextDueDate()).toString(format) == "2007-01-28" );
  s.setNextDueDate(QDate(2007, 1, 14));
  CPPUNIT_ASSERT( s.nextPayment(s.nextDueDate()).toString(format) == "2007-01-29" );
  // 15 -> Last Day
  s.setNextDueDate(QDate(2007, 1, 15));
  CPPUNIT_ASSERT( s.nextPayment(s.nextDueDate()).toString(format) == "2007-01-31" );
  s.setNextDueDate(QDate(2007, 1, 16));
  CPPUNIT_ASSERT( s.nextPayment(s.nextDueDate()).toString(format) == "2007-02-01" );
  s.setNextDueDate(QDate(2007, 1, 17));
  CPPUNIT_ASSERT( s.nextPayment(s.nextDueDate()).toString(format) == "2007-02-02" );
  s.setNextDueDate(QDate(2007, 1, 18));
  CPPUNIT_ASSERT( s.nextPayment(s.nextDueDate()).toString(format) == "2007-02-03" );
  s.setNextDueDate(QDate(2007, 1, 19));
  CPPUNIT_ASSERT( s.nextPayment(s.nextDueDate()).toString(format) == "2007-02-04" );
  s.setNextDueDate(QDate(2007, 1, 20));
  CPPUNIT_ASSERT( s.nextPayment(s.nextDueDate()).toString(format) == "2007-02-05" );
  s.setNextDueDate(QDate(2007, 1, 21));
  CPPUNIT_ASSERT( s.nextPayment(s.nextDueDate()).toString(format) == "2007-02-06" );
  s.setNextDueDate(QDate(2007, 1, 22));
  CPPUNIT_ASSERT( s.nextPayment(s.nextDueDate()).toString(format) == "2007-02-07" );
  s.setNextDueDate(QDate(2007, 1, 23));
  CPPUNIT_ASSERT( s.nextPayment(s.nextDueDate()).toString(format) == "2007-02-08" );
  s.setNextDueDate(QDate(2007, 1, 24));
  CPPUNIT_ASSERT( s.nextPayment(s.nextDueDate()).toString(format) == "2007-02-09" );
  s.setNextDueDate(QDate(2007, 1, 25));
  CPPUNIT_ASSERT( s.nextPayment(s.nextDueDate()).toString(format) == "2007-02-10" );
  s.setNextDueDate(QDate(2007, 1, 26));
  CPPUNIT_ASSERT( s.nextPayment(s.nextDueDate()).toString(format) == "2007-02-11" );
  s.setNextDueDate(QDate(2007, 1, 27));
  CPPUNIT_ASSERT( s.nextPayment(s.nextDueDate()).toString(format) == "2007-02-12" );
  s.setNextDueDate(QDate(2007, 1, 28));
  CPPUNIT_ASSERT( s.nextPayment(s.nextDueDate()).toString(format) == "2007-02-13" );
  s.setNextDueDate(QDate(2007, 1, 29));
  CPPUNIT_ASSERT( s.nextPayment(s.nextDueDate()).toString(format) == "2007-02-14" );
  // 30th,31st -> 15th
  s.setNextDueDate(QDate(2007, 1, 30));
  CPPUNIT_ASSERT( s.nextPayment(s.nextDueDate()).toString(format) == "2007-02-15" );
  s.setNextDueDate(QDate(2007, 1, 31));
  CPPUNIT_ASSERT( s.nextPayment(s.nextDueDate()).toString(format) == "2007-02-15" );
  // 30th (last day)
  s.setNextDueDate(QDate(2007, 4, 30));
  CPPUNIT_ASSERT( s.nextPayment(s.nextDueDate()).toString(format) == "2007-05-15" );
  // 28th of February (Last day): to 15th
  s.setNextDueDate(QDate(1900, 2, 28));
  CPPUNIT_ASSERT( s.nextPayment(s.nextDueDate()).toString(format) == "1900-03-15" );
  // 28th of February (Leap year): to 13th
  s.setNextDueDate(QDate(2000, 2, 28));
  CPPUNIT_ASSERT( s.nextPayment(s.nextDueDate()).toString(format) == "2000-03-13" );
  // 29th of February (Leap year)
  s.setNextDueDate(QDate(2000, 2, 29));
  CPPUNIT_ASSERT( s.nextPayment(s.nextDueDate()).toString(format) == "2000-03-15" );
  // Add multiple transactions
  s.setStartDate(QDate(2007, 1, 1));
  CPPUNIT_ASSERT( s.dateAfter(2).toString(format) == "2007-01-16" );
  CPPUNIT_ASSERT( s.dateAfter(3).toString(format) == "2007-02-01" );
  CPPUNIT_ASSERT( s.dateAfter(4).toString(format) == "2007-02-16" );
  s.setStartDate(QDate(2007, 1, 12));
  CPPUNIT_ASSERT( s.dateAfter(2).toString(format) == "2007-01-27" );
  CPPUNIT_ASSERT( s.dateAfter(3).toString(format) == "2007-02-12" );
  CPPUNIT_ASSERT( s.dateAfter(4).toString(format) == "2007-02-27" );
  CPPUNIT_ASSERT( s.dateAfter(5).toString(format) == "2007-03-12" );
  s.setStartDate(QDate(2007, 1, 13));
  CPPUNIT_ASSERT( s.dateAfter(2).toString(format) == "2007-01-28" );
  CPPUNIT_ASSERT( s.dateAfter(3).toString(format) == "2007-02-13" );
  CPPUNIT_ASSERT( s.dateAfter(4).toString(format) == "2007-02-28" );
  CPPUNIT_ASSERT( s.dateAfter(5).toString(format) == "2007-03-15" );
  s.setStartDate(QDate(2007, 1, 14));
  CPPUNIT_ASSERT( s.dateAfter(2).toString(format) == "2007-01-29" );
  CPPUNIT_ASSERT( s.dateAfter(3).toString(format) == "2007-02-14" );
  CPPUNIT_ASSERT( s.dateAfter(4).toString(format) == "2007-02-28" );
  CPPUNIT_ASSERT( s.dateAfter(5).toString(format) == "2007-03-15" );
  s.setStartDate(QDate(2007, 1, 15));
  CPPUNIT_ASSERT( s.dateAfter(2).toString(format) == "2007-01-31" );
  CPPUNIT_ASSERT( s.dateAfter(3).toString(format) == "2007-02-15" );
  CPPUNIT_ASSERT( s.dateAfter(4).toString(format) == "2007-02-28" );
  CPPUNIT_ASSERT( s.dateAfter(5).toString(format) == "2007-03-15" );
  s.setStartDate(QDate(2007, 1, 16));
  CPPUNIT_ASSERT( s.dateAfter(2).toString(format) == "2007-02-01" );
  CPPUNIT_ASSERT( s.dateAfter(3).toString(format) == "2007-02-16" );
  CPPUNIT_ASSERT( s.dateAfter(4).toString(format) == "2007-03-01" );
  CPPUNIT_ASSERT( s.dateAfter(5).toString(format) == "2007-03-16" );
  s.setStartDate(QDate(2007, 1, 27));
  CPPUNIT_ASSERT( s.dateAfter(2).toString(format) == "2007-02-12" );
  CPPUNIT_ASSERT( s.dateAfter(3).toString(format) == "2007-02-27" );
  CPPUNIT_ASSERT( s.dateAfter(4).toString(format) == "2007-03-12" );
  CPPUNIT_ASSERT( s.dateAfter(5).toString(format) == "2007-03-27" );
  s.setStartDate(QDate(2007, 1, 28));
  CPPUNIT_ASSERT( s.dateAfter(2).toString(format) == "2007-02-13" );
  CPPUNIT_ASSERT( s.dateAfter(3).toString(format) == "2007-02-28" );
  CPPUNIT_ASSERT( s.dateAfter(4).toString(format) == "2007-03-15" );
  CPPUNIT_ASSERT( s.dateAfter(5).toString(format) == "2007-03-31" );
  s.setStartDate(QDate(2007, 1, 29));
  CPPUNIT_ASSERT( s.dateAfter(2).toString(format) == "2007-02-14" );
  CPPUNIT_ASSERT( s.dateAfter(3).toString(format) == "2007-02-28" );
  CPPUNIT_ASSERT( s.dateAfter(4).toString(format) == "2007-03-15" );
  CPPUNIT_ASSERT( s.dateAfter(5).toString(format) == "2007-03-31" );
  s.setStartDate(QDate(2007, 1, 30));
  CPPUNIT_ASSERT( s.dateAfter(2).toString(format) == "2007-02-15" );
  CPPUNIT_ASSERT( s.dateAfter(3).toString(format) == "2007-02-28" );
  CPPUNIT_ASSERT( s.dateAfter(4).toString(format) == "2007-03-15" );
  CPPUNIT_ASSERT( s.dateAfter(5).toString(format) == "2007-03-31" );
  s.setStartDate(QDate(2007, 1, 31));
  CPPUNIT_ASSERT( s.dateAfter(2).toString(format) == "2007-02-15" );
  CPPUNIT_ASSERT( s.dateAfter(3).toString(format) == "2007-02-28" );
  CPPUNIT_ASSERT( s.dateAfter(4).toString(format) == "2007-03-15" );
  CPPUNIT_ASSERT( s.dateAfter(5).toString(format) == "2007-03-31" );
  s.setStartDate(QDate(2007, 4, 29));
  CPPUNIT_ASSERT( s.dateAfter(2).toString(format) == "2007-05-14" );
  CPPUNIT_ASSERT( s.dateAfter(3).toString(format) == "2007-05-29" );
  CPPUNIT_ASSERT( s.dateAfter(4).toString(format) == "2007-06-14" );
  CPPUNIT_ASSERT( s.dateAfter(5).toString(format) == "2007-06-29" );
  s.setStartDate(QDate(2007, 4, 30));
  CPPUNIT_ASSERT( s.dateAfter(2).toString(format) == "2007-05-15" );
  CPPUNIT_ASSERT( s.dateAfter(3).toString(format) == "2007-05-31" );
  CPPUNIT_ASSERT( s.dateAfter(4).toString(format) == "2007-06-15" );
  CPPUNIT_ASSERT( s.dateAfter(5).toString(format) == "2007-06-30" );
}

void MyMoneyScheduleTest::testPaymentDates()
{
	MyMoneySchedule sch;
	QString ref_ok = QString(
		"<!DOCTYPE TEST>\n"
		"<SCHEDULE-CONTAINER>\n"

		"<SCHEDULED_TX startDate=\"2003-12-31\" autoEnter=\"1\" weekendOption=\"0\" lastPayment=\"2006-01-31\" paymentType=\"2\" endDate=\"\" type=\"2\" id=\"SCH000032\" name=\"DSL\" fixed=\"0\" occurenceMultiplier=\"1\" occurence=\"32\" >\n"
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

	QDate startDate(2006,1,28);
	QDate endDate(2006,5,30);

	try {
		sch = MyMoneySchedule(node);
		QDate nextPayment = sch.nextPayment(startDate);
		Q3ValueList<QDate> list = sch.paymentDates(nextPayment, endDate);
		CPPUNIT_ASSERT(list.count() == 3);
		CPPUNIT_ASSERT(list[0] == QDate(2006,2,28));
		CPPUNIT_ASSERT(list[1] == QDate(2006,3,31));
		CPPUNIT_ASSERT(list[2] == QDate(2006,4,30));

		// Add tests for each possible occurence.
		// Check how paymentDates is meant to work
		// Build a list of expected dates and compare
		// MyMoneySchedule::OCCUR_ONCE
		sch.setOccurence(MyMoneySchedule::OCCUR_ONCE);
		startDate.setYMD(2009,1,1);
                endDate.setYMD(2009,12,31);
		sch.setStartDate(startDate);
                sch.setNextDueDate(startDate);
                list = sch.paymentDates(startDate,endDate);
                CPPUNIT_ASSERT(list.count() == 1);
 		CPPUNIT_ASSERT(list[0] == QDate(2009,1,1));
		// MyMoneySchedule::OCCUR_DAILY
		sch.setOccurence(MyMoneySchedule::OCCUR_DAILY);
		startDate.setYMD(2009,1,1);
                endDate.setYMD(2009,1,5);
		sch.setStartDate(startDate);
                sch.setNextDueDate(startDate);
                list = sch.paymentDates(startDate,endDate);
                CPPUNIT_ASSERT(list.count() == 5);
 		CPPUNIT_ASSERT(list[0] == QDate(2009, 1, 1));
 		CPPUNIT_ASSERT(list[1] == QDate(2009, 1, 2));
 		CPPUNIT_ASSERT(list[2] == QDate(2009, 1, 3));
 		CPPUNIT_ASSERT(list[3] == QDate(2009, 1, 4));
 		CPPUNIT_ASSERT(list[4] == QDate(2009, 1, 5));
		// MyMoneySchedule::OCCUR_DAILY with multiplier 2
		sch.setOccurenceMultiplier(2);
                list = sch.paymentDates(startDate.addDays(1),endDate);
                CPPUNIT_ASSERT(list.count() == 2);
 		CPPUNIT_ASSERT(list[0] == QDate(2009, 1, 3));
 		CPPUNIT_ASSERT(list[1] == QDate(2009, 1, 5));
		sch.setOccurenceMultiplier(1);
		// MyMoneySchedule::OCCUR_WEEKLY
		sch.setOccurence(MyMoneySchedule::OCCUR_WEEKLY);
		startDate.setYMD(2009,1,6);
                endDate.setYMD(2009,2,4);
		sch.setStartDate(startDate);
                sch.setNextDueDate(startDate);
                list = sch.paymentDates(startDate,endDate);
                CPPUNIT_ASSERT(list.count() == 5);
 		CPPUNIT_ASSERT(list[0] == QDate(2009, 1, 6));
 		CPPUNIT_ASSERT(list[1] == QDate(2009, 1,13));
 		CPPUNIT_ASSERT(list[2] == QDate(2009, 1,20));
 		CPPUNIT_ASSERT(list[3] == QDate(2009, 1,27));
 		CPPUNIT_ASSERT(list[4] == QDate(2009, 2, 3));
		// MyMoneySchedule::OCCUR_EVERYOTHERWEEK
		sch.setOccurence(MyMoneySchedule::OCCUR_EVERYOTHERWEEK);
		startDate.setYMD(2009,2,5);
                endDate.setYMD(2009,4,3);
		sch.setStartDate(startDate);
                sch.setNextDueDate(startDate);
                list = sch.paymentDates(startDate,endDate);
                CPPUNIT_ASSERT(list.count() == 5);
 		CPPUNIT_ASSERT(list[0] == QDate(2009, 2, 5));
 		CPPUNIT_ASSERT(list[1] == QDate(2009, 2,19));
 		CPPUNIT_ASSERT(list[2] == QDate(2009, 3, 5));
 		CPPUNIT_ASSERT(list[3] == QDate(2009, 3,19));
 		CPPUNIT_ASSERT(list[4] == QDate(2009, 4, 2));
		// MyMoneySchedule::OCCUR_FORTNIGHTLY
		sch.setOccurence(MyMoneySchedule::OCCUR_FORTNIGHTLY);
		startDate.setYMD(2009,4,4);
                endDate.setYMD(2009,5,31);
		sch.setStartDate(startDate);
                sch.setNextDueDate(startDate);
                list = sch.paymentDates(startDate,endDate);
                CPPUNIT_ASSERT(list.count() == 5);
 		CPPUNIT_ASSERT(list[0] == QDate(2009, 4, 4));
 		CPPUNIT_ASSERT(list[1] == QDate(2009, 4,18));
 		CPPUNIT_ASSERT(list[2] == QDate(2009, 5, 2));
 		CPPUNIT_ASSERT(list[3] == QDate(2009, 5,16));
 		CPPUNIT_ASSERT(list[4] == QDate(2009, 5,30));
		// MyMoneySchedule::OCCUR_EVERYHALFMONTH
		sch.setOccurence(MyMoneySchedule::OCCUR_EVERYHALFMONTH);
		startDate.setYMD(2009,6,1);
                endDate.setYMD(2009,8,11);
		sch.setStartDate(startDate);
                sch.setNextDueDate(startDate);
                list = sch.paymentDates(startDate,endDate);
                CPPUNIT_ASSERT(list.count() == 5);
 		CPPUNIT_ASSERT(list[0] == QDate(2009, 6, 1));
 		CPPUNIT_ASSERT(list[1] == QDate(2009, 6,16));
 		CPPUNIT_ASSERT(list[2] == QDate(2009, 7, 1));
 		CPPUNIT_ASSERT(list[3] == QDate(2009, 7,16));
 		CPPUNIT_ASSERT(list[4] == QDate(2009, 8, 1));
		// MyMoneySchedule::OCCUR_EVERYTHREEWEEKS
		sch.setOccurence(MyMoneySchedule::OCCUR_EVERYTHREEWEEKS);
		startDate.setYMD(2009,8,12);
                endDate.setYMD(2009,11,12);
		sch.setStartDate(startDate);
                sch.setNextDueDate(startDate);
                list = sch.paymentDates(startDate,endDate);
                CPPUNIT_ASSERT(list.count() == 5);
 		CPPUNIT_ASSERT(list[0] == QDate(2009, 8,12));
 		CPPUNIT_ASSERT(list[1] == QDate(2009, 9, 2));
 		CPPUNIT_ASSERT(list[2] == QDate(2009, 9,23));
 		CPPUNIT_ASSERT(list[3] == QDate(2009,10,14));
 		CPPUNIT_ASSERT(list[4] == QDate(2009,11, 4));
		// MyMoneySchedule::OCCUR_EVERYFOURWEEKS
		sch.setOccurence(MyMoneySchedule::OCCUR_EVERYFOURWEEKS);
		startDate.setYMD(2009,11,13);
                endDate.setYMD(2010,3,13);
		sch.setStartDate(startDate);
                sch.setNextDueDate(startDate);
                list = sch.paymentDates(startDate,endDate);
                CPPUNIT_ASSERT(list.count() == 5);
 		CPPUNIT_ASSERT(list[0] == QDate(2009,11,13));
 		CPPUNIT_ASSERT(list[1] == QDate(2009,12,11));
 		CPPUNIT_ASSERT(list[2] == QDate(2010, 1, 8));
 		CPPUNIT_ASSERT(list[3] == QDate(2010, 2, 5));
 		CPPUNIT_ASSERT(list[4] == QDate(2010, 3, 5));
		// MyMoneySchedule::OCCUR_EVERYTHIRTYDAYS
		sch.setOccurence(MyMoneySchedule::OCCUR_EVERYTHIRTYDAYS);
		startDate.setYMD(2010,3,19);
                endDate.setYMD(2010,7,19);
		sch.setStartDate(startDate);
                sch.setNextDueDate(startDate);
                list = sch.paymentDates(startDate,endDate);
                CPPUNIT_ASSERT(list.count() == 5);
 		CPPUNIT_ASSERT(list[0] == QDate(2010, 3,19));
 		CPPUNIT_ASSERT(list[1] == QDate(2010, 4,18));
 		CPPUNIT_ASSERT(list[2] == QDate(2010, 5,18));
 		CPPUNIT_ASSERT(list[3] == QDate(2010, 6,17));
 		CPPUNIT_ASSERT(list[4] == QDate(2010, 7,17));
		// MyMoneySchedule::OCCUR_EVERYEIGHTWEEKS
		sch.setOccurence(MyMoneySchedule::OCCUR_EVERYEIGHTWEEKS);
		startDate.setYMD(2010,7,26);
                endDate.setYMD(2011,3,26);
		sch.setStartDate(startDate);
                sch.setNextDueDate(startDate);
                list = sch.paymentDates(startDate,endDate);
                CPPUNIT_ASSERT(list.count() == 5);
 		CPPUNIT_ASSERT(list[0] == QDate(2010, 7,26));
 		CPPUNIT_ASSERT(list[1] == QDate(2010, 9,20));
 		CPPUNIT_ASSERT(list[2] == QDate(2010,11,15));
 		CPPUNIT_ASSERT(list[3] == QDate(2011, 1,10));
 		CPPUNIT_ASSERT(list[4] == QDate(2011, 3, 7));
		// MyMoneySchedule::OCCUR_EVERYOTHERMONTH
		sch.setOccurence(MyMoneySchedule::OCCUR_EVERYOTHERMONTH);
		startDate.setYMD(2011,3,14);
                endDate.setYMD(2011,11,20);
		sch.setStartDate(startDate);
                sch.setNextDueDate(startDate);
                list = sch.paymentDates(startDate,endDate);
                CPPUNIT_ASSERT(list.count() == 5);
 		CPPUNIT_ASSERT(list[0] == QDate(2011, 3,14));
 		CPPUNIT_ASSERT(list[1] == QDate(2011, 5,14));
 		CPPUNIT_ASSERT(list[2] == QDate(2011, 7,14));
 		CPPUNIT_ASSERT(list[3] == QDate(2011, 9,14));
 		CPPUNIT_ASSERT(list[4] == QDate(2011,11,14));
		// MyMoneySchedule::OCCUR_EVERYTHREEMONTHS
		sch.setOccurence(MyMoneySchedule::OCCUR_EVERYTHREEMONTHS);
		startDate.setYMD(2011,11,15);
                endDate.setYMD(2012,11,19);
		sch.setStartDate(startDate);
                sch.setNextDueDate(startDate);
                list = sch.paymentDates(startDate,endDate);
                CPPUNIT_ASSERT(list.count() == 5);
 		CPPUNIT_ASSERT(list[0] == QDate(2011,11,15));
 		CPPUNIT_ASSERT(list[1] == QDate(2012, 2,15));
 		CPPUNIT_ASSERT(list[2] == QDate(2012, 5,15));
 		CPPUNIT_ASSERT(list[3] == QDate(2012, 8,15));
 		CPPUNIT_ASSERT(list[4] == QDate(2012,11,15));
		// MyMoneySchedule::OCCUR_QUARTERLY
		sch.setOccurence(MyMoneySchedule::OCCUR_QUARTERLY);
		startDate.setYMD(2012,11,20);
                endDate.setYMD(2013,11,23);
		sch.setStartDate(startDate);
                sch.setNextDueDate(startDate);
                list = sch.paymentDates(startDate,endDate);
                CPPUNIT_ASSERT(list.count() == 5);
 		CPPUNIT_ASSERT(list[0] == QDate(2012,11,20));
 		CPPUNIT_ASSERT(list[1] == QDate(2013, 2,20));
 		CPPUNIT_ASSERT(list[2] == QDate(2013, 5,20));
 		CPPUNIT_ASSERT(list[3] == QDate(2013, 8,20));
 		CPPUNIT_ASSERT(list[4] == QDate(2013,11,20));
		// MyMoneySchedule::OCCUR_EVERYFOURMONTHS
		sch.setOccurence(MyMoneySchedule::OCCUR_EVERYFOURMONTHS);
		startDate.setYMD(2013,11,21);
                endDate.setYMD(2015, 3,23);
		sch.setStartDate(startDate);
                sch.setNextDueDate(startDate);
                list = sch.paymentDates(startDate,endDate);
                CPPUNIT_ASSERT(list.count() == 5);
 		CPPUNIT_ASSERT(list[0] == QDate(2013,11,21));
 		CPPUNIT_ASSERT(list[1] == QDate(2014, 3,21));
 		CPPUNIT_ASSERT(list[2] == QDate(2014, 7,21));
 		CPPUNIT_ASSERT(list[3] == QDate(2014,11,21));
 		CPPUNIT_ASSERT(list[4] == QDate(2015, 3,21));
		// MyMoneySchedule::OCCUR_TWICEYEARLY
		sch.setOccurence(MyMoneySchedule::OCCUR_TWICEYEARLY);
		startDate.setYMD(2015, 3,22);
                endDate.setYMD(2017, 3,29);
		sch.setStartDate(startDate);
                sch.setNextDueDate(startDate);
                list = sch.paymentDates(startDate,endDate);
                CPPUNIT_ASSERT(list.count() == 5);
 		CPPUNIT_ASSERT(list[0] == QDate(2015, 3,22));
 		CPPUNIT_ASSERT(list[1] == QDate(2015, 9,22));
 		CPPUNIT_ASSERT(list[2] == QDate(2016, 3,22));
 		CPPUNIT_ASSERT(list[3] == QDate(2016, 9,22));
 		CPPUNIT_ASSERT(list[4] == QDate(2017, 3,22));
		// MyMoneySchedule::OCCUR_YEARLY
		sch.setOccurence(MyMoneySchedule::OCCUR_YEARLY);
		startDate.setYMD(2017, 3,23);
                endDate.setYMD(2021, 3,29);
		sch.setStartDate(startDate);
                sch.setNextDueDate(startDate);
                list = sch.paymentDates(startDate,endDate);
                CPPUNIT_ASSERT(list.count() == 5);
 		CPPUNIT_ASSERT(list[0] == QDate(2017, 3,23));
 		CPPUNIT_ASSERT(list[1] == QDate(2018, 3,23));
 		CPPUNIT_ASSERT(list[2] == QDate(2019, 3,23));
 		CPPUNIT_ASSERT(list[3] == QDate(2020, 3,23));
 		CPPUNIT_ASSERT(list[4] == QDate(2021, 3,23));
		// MyMoneySchedule::OCCUR_EVERYOTHERYEAR
		sch.setOccurence(MyMoneySchedule::OCCUR_EVERYOTHERYEAR);
		startDate.setYMD(2021, 3,24);
                endDate.setYMD(2029, 3,30);
		sch.setStartDate(startDate);
                sch.setNextDueDate(startDate);
                list = sch.paymentDates(startDate,endDate);
                CPPUNIT_ASSERT(list.count() == 5);
 		CPPUNIT_ASSERT(list[0] == QDate(2021, 3,24));
 		CPPUNIT_ASSERT(list[1] == QDate(2023, 3,24));
 		CPPUNIT_ASSERT(list[2] == QDate(2025, 3,24));
 		CPPUNIT_ASSERT(list[3] == QDate(2027, 3,24));
 		CPPUNIT_ASSERT(list[4] == QDate(2029, 3,24));
	} catch(MyMoneyException *e) {
		delete e;
		CPPUNIT_FAIL("Unexpected exception");
	}
/*
	MyMoneyScheduled *m = MyMoneyScheduled::instance();
	CPPUNIT_ASSERT(m!=NULL);

	try
	{
		MyMoneySchedule s1 = m->getSchedule("A000001", "SCHED00001");
		MyMoneySchedule s2 = m->getSchedule("A000001", "SCHED00002");
		MyMoneySchedule s3 = m->getSchedule("A000001", "SCHED00003");

		QValueList<QDate> payments1 = s1.paymentDates(QDate(2001, 1, 1), QDate(2001, 2, 1));
		QValueList<QDate> payments2 = s2.paymentDates(QDate(2001, 2, 1), QDate(2001, 6, 1));
		QValueList<QDate> payments3 = s3.paymentDates(QDate(2001, 3, 1), QDate(2005, 3, 1));

		CPPUNIT_ASSERT(payments1.size() == 5);
		CPPUNIT_ASSERT(payments2.size() == 5);
		CPPUNIT_ASSERT(payments3.size() == 5);
	} catch (MyMoneyException *e)
	{
		CPPUNIT_FAIL("Unexpected exception");
		delete e;
	}
*/
}

void MyMoneyScheduleTest::testReplaceSchedule()
{
/*
	MyMoneyScheduled *m = MyMoneyScheduled::instance();
	CPPUNIT_ASSERT(m!=NULL);

	try
	{
		MyMoneySchedule s = m->getSchedule("A000001", "SCHED00002");
		CPPUNIT_ASSERT(s.type() == MyMoneySchedule::TYPE_DEPOSIT);
		s.setType(MyMoneySchedule::TYPE_TRANSFER);
		m->replaceSchedule("A000001", "SCHED00002", s);
		s = m->getSchedule("A000001", "SCHED00002");
		CPPUNIT_ASSERT(s.type() == MyMoneySchedule::TYPE_TRANSFER);

	} catch(MyMoneyException *e) {
		char buf[256];
		sprintf(buf, "Unexpected exception: %s", e->what().toLatin1());
		CPPUNIT_FAIL(buf);
		delete e;
	}
*/
}

void MyMoneyScheduleTest::testRemoveSchedule()
{
/*
	MyMoneyScheduled *m = MyMoneyScheduled::instance();
	CPPUNIT_ASSERT(m!=NULL);

	try
	{
		QValueList<QString> testList;

		testList = m->getScheduled("A000001");
		CPPUNIT_ASSERT(testList.size() == 3);

		m->removeSchedule("A000001", "SCHED00002");

		testList = m->getScheduled("A000001");
		CPPUNIT_ASSERT(testList.size() == 2);

		m->getSchedule("A000001", "SCHED00002");

		CPPUNIT_FAIL("Exception expected while getting schedule SCHED00002");
	} catch (MyMoneyException *e)
	{
		delete e;
	}
*/
}

void MyMoneyScheduleTest::testWriteXML() {
	MyMoneySchedule sch(	"A Name",
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
	t.setPostDate(QDate(2001,12,28));
	t.setEntryDate(QDate(2003,9,29));
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
		" <SCHEDULED_TX startDate=\"%1\" autoEnter=\"1\" weekendOption=\"2\" lastPayment=\"%2\" paymentType=\"1\" endDate=\"\" type=\"1\" id=\"SCH0001\" name=\"A Name\" fixed=\"1\" occurenceMultiplier=\"123\" occurence=\"4\" >\n"
		"  <PAYMENTS>\n"
		"   <PAYMENT date=\"%3\" />\n"
		"  </PAYMENTS>\n"
		"  <TRANSACTION postdate=\"2001-12-28\" memo=\"Wohnung:Miete\" id=\"\" commodity=\"EUR\" entrydate=\"2003-09-29\" >\n"
		"   <SPLITS>\n"
		"    <SPLIT payee=\"P000001\" reconciledate=\"\" shares=\"96379/100\" action=\"\" bankid=\"\" number=\"\" reconcileflag=\"2\" memo=\"\" value=\"96379/100\" id=\"S0001\" account=\"A000076\" />\n"
		"    <SPLIT payee=\"\" reconciledate=\"\" shares=\"-96379/100\" action=\"\" bankid=\"\" number=\"\" reconcileflag=\"1\" memo=\"\" value=\"-96379/100\" id=\"S0002\" account=\"A000276\" />\n"
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

	CPPUNIT_ASSERT(doc.toString() == ref);
}

void MyMoneyScheduleTest::testReadXML() {
	MyMoneySchedule sch;

	QString ref_ok1 = QString(
		"<!DOCTYPE TEST>\n"
		"<SCHEDULE-CONTAINER>\n"
		" <SCHEDULED_TX startDate=\"%1\" autoEnter=\"1\" weekendOption=\"2\" lastPayment=\"%2\" paymentType=\"1\" endDate=\"\" type=\"1\" id=\"SCH0002\" name=\"A Name\" fixed=\"1\" occurenceMultiplier=\"1\" occurence=\"4\" >\n"
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
		" <SCHEDULED_TX startDate=\"%1\" autoEnter=\"1\" weekendOption=\"2\" lastPayment=\"%2\" paymentType=\"1\" endDate=\"\" type=\"1\" id=\"SCH0002\" name=\"A Name\" fixed=\"1\" occurenceMultiplier=\"1\" occurence=\"4\" >\n"
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
		" <SCHEDULE startDate=\"%1\" autoEnter=\"1\" weekendOption=\"2\" lastPayment=\"%2\" paymentType=\"1\" endDate=\"\" type=\"1\" id=\"SCH0002\" name=\"A Name\" fixed=\"1\" occurenceMultiplier=\"1\" occurence=\"4\" >\n"
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
		CPPUNIT_FAIL("Missing expected exception");
	} catch(MyMoneyException *e) {
		delete e;
	}

	doc.setContent(ref_ok1);
	node = doc.documentElement().firstChild().toElement();


	try {
		sch = MyMoneySchedule(node);
		CPPUNIT_ASSERT(sch.id() == "SCH0002");
		CPPUNIT_ASSERT(sch.nextDueDate() == QDate::currentDate().addDays(7));
		CPPUNIT_ASSERT(sch.startDate() == QDate::currentDate());
		CPPUNIT_ASSERT(sch.endDate() == QDate());
		CPPUNIT_ASSERT(sch.autoEnter() == true);
		CPPUNIT_ASSERT(sch.isFixed() == true);
		CPPUNIT_ASSERT(sch.weekendOption() == MyMoneySchedule::MoveNothing);
		CPPUNIT_ASSERT(sch.lastPayment() == QDate::currentDate());
		CPPUNIT_ASSERT(sch.paymentType() == MyMoneySchedule::STYPE_DIRECTDEBIT);
		CPPUNIT_ASSERT(sch.type() == MyMoneySchedule::TYPE_BILL);
		CPPUNIT_ASSERT(sch.name() == "A Name");
		CPPUNIT_ASSERT(sch.occurence() == MyMoneySchedule::OCCUR_WEEKLY);
		CPPUNIT_ASSERT(sch.occurenceMultiplier() == 1);
		CPPUNIT_ASSERT(sch.nextDueDate() == sch.lastPayment().addDays(7));
		CPPUNIT_ASSERT(sch.recordedPayments().count() == 1);
		CPPUNIT_ASSERT(sch.recordedPayments()[0] == QDate::currentDate());
	} catch(MyMoneyException *e) {
		delete e;
		CPPUNIT_FAIL("Unexpected exception");
	}

	doc.setContent(ref_ok2);
	node = doc.documentElement().firstChild().toElement();


	try {
		sch = MyMoneySchedule(node);
		CPPUNIT_ASSERT(sch.id() == "SCH0002");
		CPPUNIT_ASSERT(sch.nextDueDate() == QDate::currentDate().addDays(7));
		CPPUNIT_ASSERT(sch.startDate() == QDate::currentDate());
		CPPUNIT_ASSERT(sch.endDate() == QDate());
		CPPUNIT_ASSERT(sch.autoEnter() == true);
		CPPUNIT_ASSERT(sch.isFixed() == true);
		CPPUNIT_ASSERT(sch.weekendOption() == MyMoneySchedule::MoveNothing);
		CPPUNIT_ASSERT(sch.lastPayment() == QDate::currentDate());
		CPPUNIT_ASSERT(sch.paymentType() == MyMoneySchedule::STYPE_DIRECTDEBIT);
		CPPUNIT_ASSERT(sch.type() == MyMoneySchedule::TYPE_BILL);
		CPPUNIT_ASSERT(sch.name() == "A Name");
		CPPUNIT_ASSERT(sch.occurence() == MyMoneySchedule::OCCUR_WEEKLY);
    CPPUNIT_ASSERT(sch.occurenceMultiplier() == 1);
		CPPUNIT_ASSERT(sch.nextDueDate() == sch.lastPayment().addDays(7));
		CPPUNIT_ASSERT(sch.recordedPayments().count() == 1);
		CPPUNIT_ASSERT(sch.recordedPayments()[0] == QDate::currentDate());
	} catch(MyMoneyException *e) {
		delete e;
		CPPUNIT_FAIL("Unexpected exception");
	}
}

void MyMoneyScheduleTest::testHasReferenceTo()
{
	MyMoneySchedule sch;
	QString ref_ok = QString(
		"<!DOCTYPE TEST>\n"
		"<SCHEDULE-CONTAINER>\n"
		" <SCHEDULED_TX startDate=\"%1\" autoEnter=\"1\" weekendOption=\"2\" lastPayment=\"%2\" paymentType=\"1\" endDate=\"\" type=\"1\" id=\"SCH0002\" name=\"A Name\" fixed=\"1\" occurenceMultiplier=\"1\" occurence=\"4\" >\n"
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

	} catch(MyMoneyException *e) {
		delete e;
		CPPUNIT_FAIL("Unexpected exception");
	}

	CPPUNIT_ASSERT(sch.hasReferenceTo("P000001") == true);
	CPPUNIT_ASSERT(sch.hasReferenceTo("A000276") == true);
	CPPUNIT_ASSERT(sch.hasReferenceTo("A000076") == true);
	CPPUNIT_ASSERT(sch.hasReferenceTo("EUR") == true);
}

void MyMoneyScheduleTest::testAdjustedNextDueDate()
{
	MyMoneySchedule s;

	QDate dueDate(2007,9,3); // start on a monday
	for(int i = 0; i < 7; ++i) {
		s.setNextDueDate(dueDate);
		s.setWeekendOption(MyMoneySchedule::MoveNothing);
		CPPUNIT_ASSERT(s.adjustedNextDueDate() == dueDate);

		s.setWeekendOption(MyMoneySchedule::MoveFriday);
		switch(i) {
		    case 5: // saturday
		    case 6: // sunday
			break;
			CPPUNIT_ASSERT(s.adjustedNextDueDate() == QDate(2007,9,7));
		    default:
			CPPUNIT_ASSERT(s.adjustedNextDueDate() == dueDate);
			break;
		}

		s.setWeekendOption(MyMoneySchedule::MoveMonday);
		switch(i) {
		    case 5: // saturday
		    case 6: // sunday
			CPPUNIT_ASSERT(s.adjustedNextDueDate() == QDate(2007,9,10));
			break;
		    default:
			CPPUNIT_ASSERT(s.adjustedNextDueDate() == dueDate);
			break;
		}
		dueDate = dueDate.addDays(1);
	}
}

void MyMoneyScheduleTest::testModifyNextDueDate(void)
{
	MyMoneySchedule s;
	s.setStartDate(QDate(2007, 1, 1));
	s.setOccurence(MyMoneySchedule::OCCUR_MONTHLY);
	s.setNextDueDate(s.startDate().addMonths(1));
	s.setLastPayment(s.startDate());

	Q3ValueList<QDate> dates;
	dates = s.paymentDates(QDate(2007,2,1), QDate(2007,2,1));
	CPPUNIT_ASSERT(s.nextDueDate() == QDate(2007,2,1));
	CPPUNIT_ASSERT(dates.count() == 1);
	CPPUNIT_ASSERT(dates[0] == QDate(2007,2,1));

	s.setNextDueDate(QDate(2007,1,24));

	dates = s.paymentDates(QDate(2007,2,1), QDate(2007,2,1));
	CPPUNIT_ASSERT(s.nextDueDate() == QDate(2007,1,24));
	CPPUNIT_ASSERT(dates.count() == 0);

	dates = s.paymentDates(QDate(2007,1,24), QDate(2007,1,24));
	CPPUNIT_ASSERT(dates.count() == 1);

	dates = s.paymentDates(QDate(2007,1,24), QDate(2007,2,24));
	CPPUNIT_ASSERT(dates.count() == 2);
	CPPUNIT_ASSERT(dates[0] == QDate(2007,1,24));
	CPPUNIT_ASSERT(dates[1] == QDate(2007,2,24));
	
}

void MyMoneyScheduleTest::testDaysBetweenEvents()
{
  CPPUNIT_ASSERT(MyMoneySchedule::daysBetweenEvents(MyMoneySchedule::OCCUR_ONCE) == 0);
  CPPUNIT_ASSERT(MyMoneySchedule::daysBetweenEvents(MyMoneySchedule::OCCUR_DAILY) == 1);
  CPPUNIT_ASSERT(MyMoneySchedule::daysBetweenEvents(MyMoneySchedule::OCCUR_WEEKLY) == 7);
  CPPUNIT_ASSERT(MyMoneySchedule::daysBetweenEvents(MyMoneySchedule::OCCUR_EVERYOTHERWEEK) == 14);
  CPPUNIT_ASSERT(MyMoneySchedule::daysBetweenEvents(MyMoneySchedule::OCCUR_FORTNIGHTLY) == 14);
  CPPUNIT_ASSERT(MyMoneySchedule::daysBetweenEvents(MyMoneySchedule::OCCUR_EVERYHALFMONTH) == 15);
  CPPUNIT_ASSERT(MyMoneySchedule::daysBetweenEvents(MyMoneySchedule::OCCUR_EVERYTHREEWEEKS) == 21);
  CPPUNIT_ASSERT(MyMoneySchedule::daysBetweenEvents(MyMoneySchedule::OCCUR_EVERYFOURWEEKS) == 28);
  CPPUNIT_ASSERT(MyMoneySchedule::daysBetweenEvents(MyMoneySchedule::OCCUR_EVERYTHIRTYDAYS) == 30);
  CPPUNIT_ASSERT(MyMoneySchedule::daysBetweenEvents(MyMoneySchedule::OCCUR_MONTHLY) == 30);
  CPPUNIT_ASSERT(MyMoneySchedule::daysBetweenEvents(MyMoneySchedule::OCCUR_EVERYEIGHTWEEKS) == 56);
  CPPUNIT_ASSERT(MyMoneySchedule::daysBetweenEvents(MyMoneySchedule::OCCUR_EVERYOTHERMONTH) == 60);
  CPPUNIT_ASSERT(MyMoneySchedule::daysBetweenEvents(MyMoneySchedule::OCCUR_EVERYTHREEMONTHS) == 90);
  CPPUNIT_ASSERT(MyMoneySchedule::daysBetweenEvents(MyMoneySchedule::OCCUR_QUARTERLY) == 90);
  CPPUNIT_ASSERT(MyMoneySchedule::daysBetweenEvents(MyMoneySchedule::OCCUR_EVERYFOURMONTHS) == 120);
  CPPUNIT_ASSERT(MyMoneySchedule::daysBetweenEvents(MyMoneySchedule::OCCUR_TWICEYEARLY) == 180);
  CPPUNIT_ASSERT(MyMoneySchedule::daysBetweenEvents(MyMoneySchedule::OCCUR_YEARLY) == 360);
  CPPUNIT_ASSERT(MyMoneySchedule::daysBetweenEvents(MyMoneySchedule::OCCUR_EVERYOTHERYEAR) == 0);
}

void MyMoneyScheduleTest::testStringToOccurence()
{
  // For each occurenceE:
  // test MyMoneySchedule::stringToOccurence(QString) == occurence
  CPPUNIT_ASSERT( MyMoneySchedule::stringToOccurence(i18n("Once")) == MyMoneySchedule::OCCUR_ONCE );
  CPPUNIT_ASSERT( MyMoneySchedule::stringToOccurence(i18n("Daily")) == MyMoneySchedule::OCCUR_DAILY );
  CPPUNIT_ASSERT( MyMoneySchedule::stringToOccurence(i18n("Weekly")) == MyMoneySchedule::OCCUR_WEEKLY );
  CPPUNIT_ASSERT( MyMoneySchedule::stringToOccurence(i18n("Every other week")) == MyMoneySchedule::OCCUR_EVERYOTHERWEEK );
  CPPUNIT_ASSERT( MyMoneySchedule::stringToOccurence(i18n("Fortnightly")) == MyMoneySchedule::OCCUR_FORTNIGHTLY );
  CPPUNIT_ASSERT( MyMoneySchedule::stringToOccurence(i18n("Every half month")) == MyMoneySchedule::OCCUR_EVERYHALFMONTH );
  CPPUNIT_ASSERT( MyMoneySchedule::stringToOccurence(i18n("Every four weeks")) == MyMoneySchedule::OCCUR_EVERYFOURWEEKS );
  CPPUNIT_ASSERT( MyMoneySchedule::stringToOccurence(i18n("Monthly")) == MyMoneySchedule::OCCUR_MONTHLY );
  CPPUNIT_ASSERT( MyMoneySchedule::stringToOccurence(i18n("Every eight weeks")) == MyMoneySchedule::OCCUR_EVERYEIGHTWEEKS );
  CPPUNIT_ASSERT( MyMoneySchedule::stringToOccurence(i18n("Every two months")) == MyMoneySchedule::OCCUR_EVERYOTHERMONTH );
  CPPUNIT_ASSERT( MyMoneySchedule::stringToOccurence(i18n("Every three months")) == MyMoneySchedule::OCCUR_EVERYTHREEMONTHS );
  CPPUNIT_ASSERT( MyMoneySchedule::stringToOccurence(i18n("Quarterly")) == MyMoneySchedule::OCCUR_QUARTERLY );
  CPPUNIT_ASSERT( MyMoneySchedule::stringToOccurence(i18n("Every four months")) == MyMoneySchedule::OCCUR_EVERYFOURMONTHS );
  CPPUNIT_ASSERT( MyMoneySchedule::stringToOccurence(i18n("Twice yearly")) == MyMoneySchedule::OCCUR_TWICEYEARLY );
  CPPUNIT_ASSERT( MyMoneySchedule::stringToOccurence(i18n("Yearly")) == MyMoneySchedule::OCCUR_YEARLY );
  CPPUNIT_ASSERT( MyMoneySchedule::stringToOccurence(i18n("Every other year")) == MyMoneySchedule::OCCUR_EVERYOTHERYEAR );
  // test occurence == stringToOccurence(i18n(occurenceToString(occurence)))
  CPPUNIT_ASSERT( MyMoneySchedule::OCCUR_ONCE == MyMoneySchedule::stringToOccurence(i18n(MyMoneySchedule::occurenceToString(MyMoneySchedule::OCCUR_ONCE))) );
  CPPUNIT_ASSERT( MyMoneySchedule::OCCUR_DAILY == MyMoneySchedule::stringToOccurence(i18n(MyMoneySchedule::occurenceToString(MyMoneySchedule::OCCUR_DAILY))) );
  CPPUNIT_ASSERT( MyMoneySchedule::OCCUR_WEEKLY == MyMoneySchedule::stringToOccurence(i18n(MyMoneySchedule::occurenceToString(MyMoneySchedule::OCCUR_WEEKLY))) );
  CPPUNIT_ASSERT( MyMoneySchedule::OCCUR_EVERYOTHERWEEK == MyMoneySchedule::stringToOccurence(i18n(MyMoneySchedule::occurenceToString(MyMoneySchedule::OCCUR_EVERYOTHERWEEK))) );
  CPPUNIT_ASSERT( MyMoneySchedule::OCCUR_FORTNIGHTLY == MyMoneySchedule::stringToOccurence(i18n(MyMoneySchedule::occurenceToString(MyMoneySchedule::OCCUR_FORTNIGHTLY))) );
  CPPUNIT_ASSERT( MyMoneySchedule::OCCUR_EVERYHALFMONTH == MyMoneySchedule::stringToOccurence(i18n(MyMoneySchedule::occurenceToString(MyMoneySchedule::OCCUR_EVERYHALFMONTH))) );
  CPPUNIT_ASSERT( MyMoneySchedule::OCCUR_EVERYTHREEWEEKS == MyMoneySchedule::stringToOccurence(i18n(MyMoneySchedule::occurenceToString(MyMoneySchedule::OCCUR_EVERYTHREEWEEKS))) );
  CPPUNIT_ASSERT( MyMoneySchedule::OCCUR_EVERYTHIRTYDAYS == MyMoneySchedule::stringToOccurence(i18n(MyMoneySchedule::occurenceToString(MyMoneySchedule::OCCUR_EVERYTHIRTYDAYS))) );
  CPPUNIT_ASSERT( MyMoneySchedule::OCCUR_EVERYTHREEWEEKS == MyMoneySchedule::stringToOccurence(i18n(MyMoneySchedule::occurenceToString(MyMoneySchedule::OCCUR_EVERYTHREEWEEKS))) );
  CPPUNIT_ASSERT( MyMoneySchedule::OCCUR_EVERYFOURWEEKS == MyMoneySchedule::stringToOccurence(i18n(MyMoneySchedule::occurenceToString(MyMoneySchedule::OCCUR_EVERYFOURWEEKS))) );
  CPPUNIT_ASSERT( MyMoneySchedule::OCCUR_EVERYTHIRTYDAYS == MyMoneySchedule::stringToOccurence(i18n(MyMoneySchedule::occurenceToString(MyMoneySchedule::OCCUR_EVERYTHIRTYDAYS))) );
  CPPUNIT_ASSERT( MyMoneySchedule::OCCUR_MONTHLY == MyMoneySchedule::stringToOccurence(i18n(MyMoneySchedule::occurenceToString(MyMoneySchedule::OCCUR_MONTHLY))) );
  CPPUNIT_ASSERT( MyMoneySchedule::OCCUR_EVERYEIGHTWEEKS == MyMoneySchedule::stringToOccurence(i18n(MyMoneySchedule::occurenceToString(MyMoneySchedule::OCCUR_EVERYEIGHTWEEKS))) );
  CPPUNIT_ASSERT( MyMoneySchedule::OCCUR_EVERYOTHERMONTH == MyMoneySchedule::stringToOccurence(i18n(MyMoneySchedule::occurenceToString(MyMoneySchedule::OCCUR_EVERYOTHERMONTH))) );
  CPPUNIT_ASSERT( MyMoneySchedule::OCCUR_EVERYTHREEMONTHS == MyMoneySchedule::stringToOccurence(i18n(MyMoneySchedule::occurenceToString(MyMoneySchedule::OCCUR_EVERYTHREEMONTHS))) );
  CPPUNIT_ASSERT( MyMoneySchedule::OCCUR_QUARTERLY == MyMoneySchedule::stringToOccurence(i18n(MyMoneySchedule::occurenceToString(MyMoneySchedule::OCCUR_QUARTERLY))) );
  CPPUNIT_ASSERT( MyMoneySchedule::OCCUR_EVERYFOURMONTHS == MyMoneySchedule::stringToOccurence(i18n(MyMoneySchedule::occurenceToString(MyMoneySchedule::OCCUR_EVERYFOURMONTHS))) );
  CPPUNIT_ASSERT( MyMoneySchedule::OCCUR_TWICEYEARLY == MyMoneySchedule::stringToOccurence(i18n(MyMoneySchedule::occurenceToString(MyMoneySchedule::OCCUR_TWICEYEARLY))) );
  CPPUNIT_ASSERT( MyMoneySchedule::OCCUR_YEARLY == MyMoneySchedule::stringToOccurence(i18n(MyMoneySchedule::occurenceToString(MyMoneySchedule::OCCUR_YEARLY))) );
  CPPUNIT_ASSERT( MyMoneySchedule::OCCUR_EVERYOTHERYEAR == MyMoneySchedule::stringToOccurence(i18n(MyMoneySchedule::occurenceToString(MyMoneySchedule::OCCUR_EVERYOTHERYEAR))) );
}
void MyMoneyScheduleTest::testEventsPerYear()
{
  CPPUNIT_ASSERT(MyMoneySchedule::eventsPerYear(MyMoneySchedule::OCCUR_ONCE) == 0);
  CPPUNIT_ASSERT(MyMoneySchedule::eventsPerYear(MyMoneySchedule::OCCUR_DAILY) == 365);
  CPPUNIT_ASSERT(MyMoneySchedule::eventsPerYear(MyMoneySchedule::OCCUR_WEEKLY) == 52);
  CPPUNIT_ASSERT(MyMoneySchedule::eventsPerYear(MyMoneySchedule::OCCUR_EVERYOTHERWEEK) == 26);
  CPPUNIT_ASSERT(MyMoneySchedule::eventsPerYear(MyMoneySchedule::OCCUR_FORTNIGHTLY) == 26);
  CPPUNIT_ASSERT(MyMoneySchedule::eventsPerYear(MyMoneySchedule::OCCUR_EVERYHALFMONTH) == 24);
  CPPUNIT_ASSERT(MyMoneySchedule::eventsPerYear(MyMoneySchedule::OCCUR_EVERYTHREEWEEKS) == 17);
  CPPUNIT_ASSERT(MyMoneySchedule::eventsPerYear(MyMoneySchedule::OCCUR_EVERYFOURWEEKS) == 13);
  CPPUNIT_ASSERT(MyMoneySchedule::eventsPerYear(MyMoneySchedule::OCCUR_EVERYTHIRTYDAYS) == 12);
  CPPUNIT_ASSERT(MyMoneySchedule::eventsPerYear(MyMoneySchedule::OCCUR_MONTHLY) == 12);
  CPPUNIT_ASSERT(MyMoneySchedule::eventsPerYear(MyMoneySchedule::OCCUR_EVERYEIGHTWEEKS) == 6);
  CPPUNIT_ASSERT(MyMoneySchedule::eventsPerYear(MyMoneySchedule::OCCUR_EVERYOTHERMONTH) == 6);
  CPPUNIT_ASSERT(MyMoneySchedule::eventsPerYear(MyMoneySchedule::OCCUR_EVERYTHREEMONTHS) == 4);
  CPPUNIT_ASSERT(MyMoneySchedule::eventsPerYear(MyMoneySchedule::OCCUR_QUARTERLY) == 4);
  CPPUNIT_ASSERT(MyMoneySchedule::eventsPerYear(MyMoneySchedule::OCCUR_EVERYFOURMONTHS) == 3);
  CPPUNIT_ASSERT(MyMoneySchedule::eventsPerYear(MyMoneySchedule::OCCUR_TWICEYEARLY) == 2);
  CPPUNIT_ASSERT(MyMoneySchedule::eventsPerYear(MyMoneySchedule::OCCUR_YEARLY) == 1);
  CPPUNIT_ASSERT(MyMoneySchedule::eventsPerYear(MyMoneySchedule::OCCUR_EVERYOTHERYEAR) == 0);
}

void MyMoneyScheduleTest::testOccurenceToString()
{
  // For each occurenceE test MyMoneySchedule::occurenceToString(occurenceE)
  CPPUNIT_ASSERT(MyMoneySchedule::occurenceToString(MyMoneySchedule::OCCUR_ONCE) == "Once" );
  CPPUNIT_ASSERT(MyMoneySchedule::occurenceToString(MyMoneySchedule::OCCUR_DAILY) == "Daily" );
  CPPUNIT_ASSERT(MyMoneySchedule::occurenceToString(MyMoneySchedule::OCCUR_WEEKLY) == "Weekly" );
  CPPUNIT_ASSERT(MyMoneySchedule::occurenceToString(MyMoneySchedule::OCCUR_EVERYOTHERWEEK) == "Every other week" );
  CPPUNIT_ASSERT(MyMoneySchedule::occurenceToString(MyMoneySchedule::OCCUR_FORTNIGHTLY) == "Fortnightly" );
  CPPUNIT_ASSERT(MyMoneySchedule::occurenceToString(MyMoneySchedule::OCCUR_EVERYHALFMONTH) == "Every half month" );
  CPPUNIT_ASSERT(MyMoneySchedule::occurenceToString(MyMoneySchedule::OCCUR_EVERYTHREEWEEKS) == "Every three weeks" );
  CPPUNIT_ASSERT(MyMoneySchedule::occurenceToString(MyMoneySchedule::OCCUR_EVERYFOURWEEKS) == "Every four weeks" );
  CPPUNIT_ASSERT(MyMoneySchedule::occurenceToString(MyMoneySchedule::OCCUR_EVERYTHIRTYDAYS) == "Every thirty days" );
  CPPUNIT_ASSERT(MyMoneySchedule::occurenceToString(MyMoneySchedule::OCCUR_MONTHLY) == "Monthly" );
  CPPUNIT_ASSERT(MyMoneySchedule::occurenceToString(MyMoneySchedule::OCCUR_EVERYEIGHTWEEKS) == "Every eight weeks" );
  CPPUNIT_ASSERT(MyMoneySchedule::occurenceToString(MyMoneySchedule::OCCUR_EVERYOTHERMONTH) == "Every two months" );
  CPPUNIT_ASSERT(MyMoneySchedule::occurenceToString(MyMoneySchedule::OCCUR_EVERYTHREEMONTHS) == "Every three months" );
  CPPUNIT_ASSERT(MyMoneySchedule::occurenceToString(MyMoneySchedule::OCCUR_QUARTERLY) == "Quarterly" );
  CPPUNIT_ASSERT(MyMoneySchedule::occurenceToString(MyMoneySchedule::OCCUR_EVERYFOURMONTHS) == "Every four months" );
  CPPUNIT_ASSERT(MyMoneySchedule::occurenceToString(MyMoneySchedule::OCCUR_TWICEYEARLY) == "Twice yearly" );
  CPPUNIT_ASSERT(MyMoneySchedule::occurenceToString(MyMoneySchedule::OCCUR_YEARLY) == "Yearly" );
  CPPUNIT_ASSERT(MyMoneySchedule::occurenceToString(MyMoneySchedule::OCCUR_EVERYOTHERYEAR) == "Every other year" );
  // For each occurenceE set occurence and compare occurenceToString() with oTS(occurence())
  MyMoneySchedule s;
  s.setStartDate(QDate(2007, 1, 1));
  s.setNextDueDate(s.startDate());
  s.setLastPayment(s.startDate());
  s.setOccurence(MyMoneySchedule::OCCUR_ONCE); CPPUNIT_ASSERT(s.occurenceToString() == "Once" );
  s.setOccurence(MyMoneySchedule::OCCUR_DAILY); CPPUNIT_ASSERT(s.occurenceToString() == "Daily" );
  s.setOccurence(MyMoneySchedule::OCCUR_WEEKLY); CPPUNIT_ASSERT(s.occurenceToString() == "Weekly" );
  s.setOccurence(MyMoneySchedule::OCCUR_EVERYOTHERWEEK); CPPUNIT_ASSERT(s.occurenceToString() == "Every other week" );
  // Fortnightly no longer used: Every other week used instead
  s.setOccurence(MyMoneySchedule::OCCUR_FORTNIGHTLY); CPPUNIT_ASSERT(s.occurenceToString() == "Every other week" );
  s.setOccurence(MyMoneySchedule::OCCUR_EVERYHALFMONTH); CPPUNIT_ASSERT(s.occurenceToString() == "Every half month" );
  s.setOccurence(MyMoneySchedule::OCCUR_EVERYTHREEWEEKS); CPPUNIT_ASSERT(s.occurenceToString() == "Every three weeks" );
  s.setOccurence(MyMoneySchedule::OCCUR_EVERYFOURWEEKS); CPPUNIT_ASSERT(s.occurenceToString() == "Every four weeks" );
  s.setOccurence(MyMoneySchedule::OCCUR_EVERYTHIRTYDAYS); CPPUNIT_ASSERT(s.occurenceToString() == "Every thirty days" );
  s.setOccurence(MyMoneySchedule::OCCUR_MONTHLY); CPPUNIT_ASSERT(s.occurenceToString() == "Monthly" );
  s.setOccurence(MyMoneySchedule::OCCUR_EVERYEIGHTWEEKS); CPPUNIT_ASSERT(s.occurenceToString() == "Every eight weeks" );
  s.setOccurence(MyMoneySchedule::OCCUR_EVERYOTHERMONTH); CPPUNIT_ASSERT(s.occurenceToString() == "Every two months" );
  s.setOccurence(MyMoneySchedule::OCCUR_EVERYTHREEMONTHS); CPPUNIT_ASSERT(s.occurenceToString() == "Every three months" );
  // Quarterly no longer used.  Every three months used instead
  s.setOccurence(MyMoneySchedule::OCCUR_QUARTERLY); CPPUNIT_ASSERT(s.occurenceToString() == "Every three months" );
  s.setOccurence(MyMoneySchedule::OCCUR_EVERYFOURMONTHS); CPPUNIT_ASSERT(s.occurenceToString() == "Every four months" );
  s.setOccurence(MyMoneySchedule::OCCUR_TWICEYEARLY); CPPUNIT_ASSERT(s.occurenceToString() == "Twice yearly" );
  s.setOccurence(MyMoneySchedule::OCCUR_YEARLY); CPPUNIT_ASSERT(s.occurenceToString() == "Yearly" );
  s.setOccurence(MyMoneySchedule::OCCUR_EVERYOTHERYEAR); CPPUNIT_ASSERT(s.occurenceToString() == "Every other year" );
  // Test occurenceToString(mult,occ)
  // Test all pairs equivalent to simple occurences: should return the same as occurenceToString(simpleOcc)
  // TODO replace string with (mult,occ) call. 
  CPPUNIT_ASSERT(MyMoneySchedule::occurenceToString(MyMoneySchedule::OCCUR_ONCE) == MyMoneySchedule::occurenceToString(1,MyMoneySchedule::OCCUR_ONCE) );
  CPPUNIT_ASSERT(MyMoneySchedule::occurenceToString(MyMoneySchedule::OCCUR_DAILY) == MyMoneySchedule::occurenceToString(1,MyMoneySchedule::OCCUR_DAILY) );
  CPPUNIT_ASSERT(MyMoneySchedule::occurenceToString(MyMoneySchedule::OCCUR_WEEKLY) == MyMoneySchedule::occurenceToString(1,MyMoneySchedule::OCCUR_WEEKLY) );
  CPPUNIT_ASSERT(MyMoneySchedule::occurenceToString(MyMoneySchedule::OCCUR_EVERYOTHERWEEK) == MyMoneySchedule::occurenceToString(2,MyMoneySchedule::OCCUR_WEEKLY) );
  // OCCUR_FORTNIGHTLY will no longer be used: only Every Other Week
  CPPUNIT_ASSERT(MyMoneySchedule::occurenceToString(MyMoneySchedule::OCCUR_EVERYHALFMONTH) == MyMoneySchedule::occurenceToString(1,MyMoneySchedule::OCCUR_EVERYHALFMONTH) );
  CPPUNIT_ASSERT(MyMoneySchedule::occurenceToString(MyMoneySchedule::OCCUR_EVERYTHREEWEEKS) == MyMoneySchedule::occurenceToString(3,MyMoneySchedule::OCCUR_WEEKLY) );
  CPPUNIT_ASSERT(MyMoneySchedule::occurenceToString(MyMoneySchedule::OCCUR_EVERYFOURWEEKS) == MyMoneySchedule::occurenceToString(4,MyMoneySchedule::OCCUR_WEEKLY) );
  CPPUNIT_ASSERT(MyMoneySchedule::occurenceToString(MyMoneySchedule::OCCUR_MONTHLY) == MyMoneySchedule::occurenceToString(1,MyMoneySchedule::OCCUR_MONTHLY) );
  CPPUNIT_ASSERT(MyMoneySchedule::occurenceToString(MyMoneySchedule::OCCUR_EVERYEIGHTWEEKS) == MyMoneySchedule::occurenceToString(8,MyMoneySchedule::OCCUR_WEEKLY) );
  CPPUNIT_ASSERT(MyMoneySchedule::occurenceToString(MyMoneySchedule::OCCUR_EVERYOTHERMONTH) == MyMoneySchedule::occurenceToString(2,MyMoneySchedule::OCCUR_MONTHLY) );
  CPPUNIT_ASSERT(MyMoneySchedule::occurenceToString(MyMoneySchedule::OCCUR_EVERYTHREEMONTHS) == MyMoneySchedule::occurenceToString(3,MyMoneySchedule::OCCUR_MONTHLY) );
  // OCCUR_QUARTERLY will no longer be used: only Every Three Months
  CPPUNIT_ASSERT(MyMoneySchedule::occurenceToString(MyMoneySchedule::OCCUR_EVERYFOURMONTHS) == MyMoneySchedule::occurenceToString(4,MyMoneySchedule::OCCUR_MONTHLY) );
  CPPUNIT_ASSERT(MyMoneySchedule::occurenceToString(MyMoneySchedule::OCCUR_TWICEYEARLY) == MyMoneySchedule::occurenceToString(6,MyMoneySchedule::OCCUR_MONTHLY) );
  CPPUNIT_ASSERT(MyMoneySchedule::occurenceToString(MyMoneySchedule::OCCUR_YEARLY) == MyMoneySchedule::occurenceToString(1,MyMoneySchedule::OCCUR_YEARLY) );
  CPPUNIT_ASSERT(MyMoneySchedule::occurenceToString(MyMoneySchedule::OCCUR_EVERYOTHERYEAR) == MyMoneySchedule::occurenceToString(2,MyMoneySchedule::OCCUR_YEARLY) );
  // Test additional calls with other mult,occ
  CPPUNIT_ASSERT(MyMoneySchedule::occurenceToString(2,MyMoneySchedule::OCCUR_ONCE) == "2 times" );
  CPPUNIT_ASSERT(MyMoneySchedule::occurenceToString(2,MyMoneySchedule::OCCUR_DAILY) == "Every 2 days" );
  CPPUNIT_ASSERT(MyMoneySchedule::occurenceToString(5,MyMoneySchedule::OCCUR_WEEKLY) == "Every 5 weeks" );
  CPPUNIT_ASSERT(MyMoneySchedule::occurenceToString(2,MyMoneySchedule::OCCUR_EVERYHALFMONTH) == "Every 2 half months" );
  CPPUNIT_ASSERT(MyMoneySchedule::occurenceToString(5,MyMoneySchedule::OCCUR_MONTHLY) == "Every 5 months" );
  CPPUNIT_ASSERT(MyMoneySchedule::occurenceToString(3,MyMoneySchedule::OCCUR_YEARLY) == "Every 3 years" );
  CPPUNIT_ASSERT(MyMoneySchedule::occurenceToString(37,MyMoneySchedule::OCCUR_ONCE) == "37 times" );
  CPPUNIT_ASSERT(MyMoneySchedule::occurenceToString(43,MyMoneySchedule::OCCUR_DAILY) == "Every 43 days" );
  CPPUNIT_ASSERT(MyMoneySchedule::occurenceToString(61,MyMoneySchedule::OCCUR_WEEKLY) == "Every 61 weeks" );
  CPPUNIT_ASSERT(MyMoneySchedule::occurenceToString(73,MyMoneySchedule::OCCUR_EVERYHALFMONTH) == "Every 73 half months" );
  CPPUNIT_ASSERT(MyMoneySchedule::occurenceToString(83,MyMoneySchedule::OCCUR_MONTHLY) == "Every 83 months" );
  CPPUNIT_ASSERT(MyMoneySchedule::occurenceToString(89,MyMoneySchedule::OCCUR_YEARLY) == "Every 89 years" );
  // Test instance-level occurenceToString method is using occurencePeriod and multiplier
  // For each base occurence set occurencePeriod and multiplier
  s.setOccurencePeriod(MyMoneySchedule::OCCUR_ONCE); s.setOccurenceMultiplier(1);
  s.setOccurence(MyMoneySchedule::OCCUR_ONCE);
  s.setOccurenceMultiplier(1); CPPUNIT_ASSERT(s.occurenceToString() == "Once" );
  s.setOccurenceMultiplier(2); CPPUNIT_ASSERT(s.occurenceToString() == "2 times" );
  s.setOccurenceMultiplier(3); CPPUNIT_ASSERT(s.occurenceToString() == "3 times" );
  s.setOccurencePeriod(MyMoneySchedule::OCCUR_DAILY);
  s.setOccurenceMultiplier(1); CPPUNIT_ASSERT(s.occurenceToString() == "Daily" );
  s.setOccurenceMultiplier(30); CPPUNIT_ASSERT(s.occurenceToString() == "Every thirty days" );
  s.setOccurenceMultiplier(3); CPPUNIT_ASSERT(s.occurenceToString() == "Every 3 days" );
  s.setOccurence(MyMoneySchedule::OCCUR_WEEKLY);
  CPPUNIT_ASSERT(s.occurenceToString() == "Weekly" );
  s.setOccurenceMultiplier(2); CPPUNIT_ASSERT(s.occurenceToString() == "Every other week" );
  s.setOccurenceMultiplier(3); CPPUNIT_ASSERT(s.occurenceToString() == "Every three weeks" );
  s.setOccurenceMultiplier(4); CPPUNIT_ASSERT(s.occurenceToString() == "Every four weeks" );
  s.setOccurenceMultiplier(5); CPPUNIT_ASSERT(s.occurenceToString() == "Every 5 weeks" );
  s.setOccurenceMultiplier(7); CPPUNIT_ASSERT(s.occurenceToString() == "Every 7 weeks" );
  s.setOccurenceMultiplier(8); CPPUNIT_ASSERT(s.occurenceToString() == "Every eight weeks" );
  s.setOccurenceMultiplier(9); CPPUNIT_ASSERT(s.occurenceToString() == "Every 9 weeks" );
  s.setOccurence(MyMoneySchedule::OCCUR_EVERYHALFMONTH);
  s.setOccurenceMultiplier(1); CPPUNIT_ASSERT(s.occurenceToString() == "Every half month" );
  s.setOccurenceMultiplier(2); CPPUNIT_ASSERT(s.occurenceToString() == "Every 2 half months" );
  s.setOccurence(MyMoneySchedule::OCCUR_MONTHLY);
  s.setOccurenceMultiplier(1); CPPUNIT_ASSERT(s.occurenceToString() == "Monthly" );
  s.setOccurenceMultiplier(2); CPPUNIT_ASSERT(s.occurenceToString() == "Every two months" );
  s.setOccurenceMultiplier(3); CPPUNIT_ASSERT(s.occurenceToString() == "Every three months" );
  s.setOccurenceMultiplier(4); CPPUNIT_ASSERT(s.occurenceToString() == "Every four months" );
  s.setOccurenceMultiplier(5); CPPUNIT_ASSERT(s.occurenceToString() == "Every 5 months" );
  s.setOccurenceMultiplier(6); CPPUNIT_ASSERT(s.occurenceToString() == "Twice yearly" );
  s.setOccurenceMultiplier(7); CPPUNIT_ASSERT(s.occurenceToString() == "Every 7 months" );
  s.setOccurence(MyMoneySchedule::OCCUR_YEARLY);
  s.setOccurenceMultiplier(1); CPPUNIT_ASSERT(s.occurenceToString() == "Yearly" );
  s.setOccurenceMultiplier(2); CPPUNIT_ASSERT(s.occurenceToString() == "Every other year" );
  s.setOccurenceMultiplier(3); CPPUNIT_ASSERT(s.occurenceToString() == "Every 3 years" );
}

void MyMoneyScheduleTest::testOccurencePeriodToString()
{
  // For each occurenceE test MyMoneySchedule::occurencePeriodToString(occurenceE)
  // Base occurences are translated
  CPPUNIT_ASSERT(MyMoneySchedule::occurencePeriodToString(MyMoneySchedule::OCCUR_ONCE) == "Once" );
  CPPUNIT_ASSERT(MyMoneySchedule::occurencePeriodToString(MyMoneySchedule::OCCUR_DAILY) == "Day" );
  CPPUNIT_ASSERT(MyMoneySchedule::occurencePeriodToString(MyMoneySchedule::OCCUR_WEEKLY) == "Week" );
  CPPUNIT_ASSERT(MyMoneySchedule::occurencePeriodToString(MyMoneySchedule::OCCUR_EVERYHALFMONTH) == "Half-month" );
  CPPUNIT_ASSERT(MyMoneySchedule::occurencePeriodToString(MyMoneySchedule::OCCUR_MONTHLY) == "Month" );
  CPPUNIT_ASSERT(MyMoneySchedule::occurencePeriodToString(MyMoneySchedule::OCCUR_YEARLY) == "Year" );
  // All others are not translated so return Any
  CPPUNIT_ASSERT(MyMoneySchedule::occurencePeriodToString(MyMoneySchedule::OCCUR_EVERYOTHERWEEK) == "Any" );
  CPPUNIT_ASSERT(MyMoneySchedule::occurencePeriodToString(MyMoneySchedule::OCCUR_FORTNIGHTLY) == "Any" );
  CPPUNIT_ASSERT(MyMoneySchedule::occurencePeriodToString(MyMoneySchedule::OCCUR_EVERYTHREEWEEKS) == "Any" );
  CPPUNIT_ASSERT(MyMoneySchedule::occurencePeriodToString(MyMoneySchedule::OCCUR_EVERYFOURWEEKS) == "Any" );
  CPPUNIT_ASSERT(MyMoneySchedule::occurencePeriodToString(MyMoneySchedule::OCCUR_EVERYTHIRTYDAYS) == "Any" );
  CPPUNIT_ASSERT(MyMoneySchedule::occurencePeriodToString(MyMoneySchedule::OCCUR_EVERYEIGHTWEEKS) == "Any" );
  CPPUNIT_ASSERT(MyMoneySchedule::occurencePeriodToString(MyMoneySchedule::OCCUR_EVERYOTHERMONTH) == "Any" );
  CPPUNIT_ASSERT(MyMoneySchedule::occurencePeriodToString(MyMoneySchedule::OCCUR_EVERYTHREEMONTHS) == "Any" );
  CPPUNIT_ASSERT(MyMoneySchedule::occurencePeriodToString(MyMoneySchedule::OCCUR_QUARTERLY) == "Any" );
  CPPUNIT_ASSERT(MyMoneySchedule::occurencePeriodToString(MyMoneySchedule::OCCUR_EVERYFOURMONTHS) == "Any" );
  CPPUNIT_ASSERT(MyMoneySchedule::occurencePeriodToString(MyMoneySchedule::OCCUR_TWICEYEARLY) == "Any" );
  CPPUNIT_ASSERT(MyMoneySchedule::occurencePeriodToString(MyMoneySchedule::OCCUR_EVERYOTHERYEAR) == "Any" );
}

void MyMoneyScheduleTest::testOccurencePeriod()
{
  // Each occurence:
  // Set occurence using setOccurencePeriod
  // occurencePeriod should match what we set
  // occurence depends on multiplier
  // TODO:
  // Once occurence() and setOccurence() are converting between compound and simple occurences
  // we need to change the occurence() check and add an occurenceMultiplier() check
  MyMoneySchedule s;
  s.setStartDate(QDate(2007, 1, 1));
  s.setNextDueDate(s.startDate());
  s.setLastPayment(s.startDate());
  // Set all base occurences
  s.setOccurencePeriod(MyMoneySchedule::OCCUR_ONCE);
  CPPUNIT_ASSERT(s.occurencePeriod() == MyMoneySchedule::OCCUR_ONCE );
  s.setOccurenceMultiplier(1);
  CPPUNIT_ASSERT(s.occurenceMultiplier() == 1);
  CPPUNIT_ASSERT(s.occurencePeriod() == MyMoneySchedule::OCCUR_ONCE );
  CPPUNIT_ASSERT(s.occurence() == MyMoneySchedule::OCCUR_ONCE );
  s.setOccurenceMultiplier(2);
  CPPUNIT_ASSERT(s.occurenceMultiplier() == 2);
  CPPUNIT_ASSERT(s.occurencePeriod() == MyMoneySchedule::OCCUR_ONCE );
  CPPUNIT_ASSERT(s.occurence() == MyMoneySchedule::OCCUR_ONCE );

  s.setOccurencePeriod(MyMoneySchedule::OCCUR_DAILY);
  CPPUNIT_ASSERT(s.occurencePeriod() == MyMoneySchedule::OCCUR_DAILY);
  s.setOccurenceMultiplier(1);
  CPPUNIT_ASSERT(s.occurenceMultiplier() == 1);
  CPPUNIT_ASSERT(s.occurencePeriod() == MyMoneySchedule::OCCUR_DAILY);
  CPPUNIT_ASSERT(s.occurence() == MyMoneySchedule::OCCUR_DAILY);
  s.setOccurenceMultiplier(30);
  CPPUNIT_ASSERT(s.occurenceMultiplier() == 30);
  CPPUNIT_ASSERT(s.occurencePeriod() == MyMoneySchedule::OCCUR_DAILY);
  CPPUNIT_ASSERT(s.occurence() == MyMoneySchedule::OCCUR_EVERYTHIRTYDAYS);
  s.setOccurenceMultiplier(2);
  CPPUNIT_ASSERT(s.occurenceMultiplier() == 2);
  CPPUNIT_ASSERT(s.occurencePeriod() == MyMoneySchedule::OCCUR_DAILY);
  CPPUNIT_ASSERT(s.occurence() == MyMoneySchedule::OCCUR_DAILY);

  s.setOccurencePeriod(MyMoneySchedule::OCCUR_WEEKLY);
  CPPUNIT_ASSERT(s.occurencePeriod() == MyMoneySchedule::OCCUR_WEEKLY);
  s.setOccurenceMultiplier(1);
  CPPUNIT_ASSERT(s.occurenceMultiplier() == 1);
  CPPUNIT_ASSERT(s.occurencePeriod() == MyMoneySchedule::OCCUR_WEEKLY);
  CPPUNIT_ASSERT(s.occurence() == MyMoneySchedule::OCCUR_WEEKLY);
  s.setOccurenceMultiplier(2);
  CPPUNIT_ASSERT(s.occurenceMultiplier() == 2);
  CPPUNIT_ASSERT(s.occurencePeriod() == MyMoneySchedule::OCCUR_WEEKLY);
  CPPUNIT_ASSERT(s.occurence() == MyMoneySchedule::OCCUR_EVERYOTHERWEEK);
  s.setOccurenceMultiplier(3);
  CPPUNIT_ASSERT(s.occurenceMultiplier() == 3);
  CPPUNIT_ASSERT(s.occurencePeriod() == MyMoneySchedule::OCCUR_WEEKLY);
  CPPUNIT_ASSERT(s.occurence() == MyMoneySchedule::OCCUR_EVERYTHREEWEEKS);
  s.setOccurenceMultiplier(4);
  CPPUNIT_ASSERT(s.occurenceMultiplier() == 4);
  CPPUNIT_ASSERT(s.occurencePeriod() == MyMoneySchedule::OCCUR_WEEKLY);
  CPPUNIT_ASSERT(s.occurence() == MyMoneySchedule::OCCUR_EVERYFOURWEEKS);
  s.setOccurenceMultiplier(5);
  CPPUNIT_ASSERT(s.occurenceMultiplier() == 5);
  CPPUNIT_ASSERT(s.occurencePeriod() == MyMoneySchedule::OCCUR_WEEKLY);
  CPPUNIT_ASSERT(s.occurence() == MyMoneySchedule::OCCUR_WEEKLY);
  s.setOccurenceMultiplier(8);
  CPPUNIT_ASSERT(s.occurenceMultiplier() == 8);
  CPPUNIT_ASSERT(s.occurencePeriod() == MyMoneySchedule::OCCUR_WEEKLY);
  CPPUNIT_ASSERT(s.occurence() == MyMoneySchedule::OCCUR_EVERYEIGHTWEEKS);

  s.setOccurencePeriod(MyMoneySchedule::OCCUR_EVERYHALFMONTH);
  CPPUNIT_ASSERT(s.occurencePeriod() == MyMoneySchedule::OCCUR_EVERYHALFMONTH);
  s.setOccurenceMultiplier(1);
  CPPUNIT_ASSERT(s.occurenceMultiplier() == 1);
  CPPUNIT_ASSERT(s.occurencePeriod() == MyMoneySchedule::OCCUR_EVERYHALFMONTH);
  CPPUNIT_ASSERT(s.occurence() == MyMoneySchedule::OCCUR_EVERYHALFMONTH);
  s.setOccurenceMultiplier(2);
  CPPUNIT_ASSERT(s.occurenceMultiplier() == 2);
  CPPUNIT_ASSERT(s.occurencePeriod() == MyMoneySchedule::OCCUR_EVERYHALFMONTH);
  CPPUNIT_ASSERT(s.occurence() == MyMoneySchedule::OCCUR_EVERYHALFMONTH);

  s.setOccurencePeriod(MyMoneySchedule::OCCUR_MONTHLY);
  CPPUNIT_ASSERT(s.occurencePeriod() == MyMoneySchedule::OCCUR_MONTHLY);
  s.setOccurenceMultiplier(1);
  CPPUNIT_ASSERT(s.occurenceMultiplier() == 1);
  CPPUNIT_ASSERT(s.occurencePeriod() == MyMoneySchedule::OCCUR_MONTHLY);
  CPPUNIT_ASSERT(s.occurence() == MyMoneySchedule::OCCUR_MONTHLY);
  s.setOccurenceMultiplier(2);
  CPPUNIT_ASSERT(s.occurenceMultiplier() == 2);
  CPPUNIT_ASSERT(s.occurencePeriod() == MyMoneySchedule::OCCUR_MONTHLY);
  CPPUNIT_ASSERT(s.occurence() == MyMoneySchedule::OCCUR_EVERYOTHERMONTH);
  s.setOccurenceMultiplier(3);
  CPPUNIT_ASSERT(s.occurenceMultiplier() == 3);
  CPPUNIT_ASSERT(s.occurencePeriod() == MyMoneySchedule::OCCUR_MONTHLY);
  CPPUNIT_ASSERT(s.occurence() == MyMoneySchedule::OCCUR_EVERYTHREEMONTHS);
  s.setOccurenceMultiplier(4);
  CPPUNIT_ASSERT(s.occurenceMultiplier() == 4);
  CPPUNIT_ASSERT(s.occurencePeriod() == MyMoneySchedule::OCCUR_MONTHLY);
  CPPUNIT_ASSERT(s.occurence() == MyMoneySchedule::OCCUR_EVERYFOURMONTHS);
  s.setOccurenceMultiplier(5);
  CPPUNIT_ASSERT(s.occurenceMultiplier() == 5);
  CPPUNIT_ASSERT(s.occurencePeriod() == MyMoneySchedule::OCCUR_MONTHLY);
  CPPUNIT_ASSERT(s.occurence() == MyMoneySchedule::OCCUR_MONTHLY);
  s.setOccurenceMultiplier(6);
  CPPUNIT_ASSERT(s.occurenceMultiplier() == 6);
  CPPUNIT_ASSERT(s.occurencePeriod() == MyMoneySchedule::OCCUR_MONTHLY);
  CPPUNIT_ASSERT(s.occurence() == MyMoneySchedule::OCCUR_TWICEYEARLY);

  s.setOccurencePeriod(MyMoneySchedule::OCCUR_YEARLY);
  CPPUNIT_ASSERT(s.occurencePeriod() == MyMoneySchedule::OCCUR_YEARLY);
  s.setOccurenceMultiplier(1);
  CPPUNIT_ASSERT(s.occurenceMultiplier() == 1);
  CPPUNIT_ASSERT(s.occurencePeriod() == MyMoneySchedule::OCCUR_YEARLY);
  CPPUNIT_ASSERT(s.occurence() == MyMoneySchedule::OCCUR_YEARLY);
  s.setOccurenceMultiplier(2);
  CPPUNIT_ASSERT(s.occurenceMultiplier() == 2);
  CPPUNIT_ASSERT(s.occurencePeriod() == MyMoneySchedule::OCCUR_YEARLY);
  CPPUNIT_ASSERT(s.occurence() == MyMoneySchedule::OCCUR_EVERYOTHERYEAR);
  s.setOccurenceMultiplier(3);
  CPPUNIT_ASSERT(s.occurenceMultiplier() == 3);
  CPPUNIT_ASSERT(s.occurencePeriod() == MyMoneySchedule::OCCUR_YEARLY);
  CPPUNIT_ASSERT(s.occurence() == MyMoneySchedule::OCCUR_YEARLY);

  // Set occurence: check occurence, Period and Multiplier
  s.setOccurence(MyMoneySchedule::OCCUR_ONCE);
  CPPUNIT_ASSERT(s.occurence() == MyMoneySchedule::OCCUR_ONCE);
  CPPUNIT_ASSERT(s.occurencePeriod() == MyMoneySchedule::OCCUR_ONCE);
  CPPUNIT_ASSERT(s.occurenceMultiplier() == 1);

  s.setOccurence(MyMoneySchedule::OCCUR_DAILY);
  CPPUNIT_ASSERT(s.occurence() == MyMoneySchedule::OCCUR_DAILY);
  CPPUNIT_ASSERT(s.occurencePeriod() == MyMoneySchedule::OCCUR_DAILY);
  CPPUNIT_ASSERT(s.occurenceMultiplier() == 1);
  s.setOccurence(MyMoneySchedule::OCCUR_EVERYTHIRTYDAYS);
  CPPUNIT_ASSERT(s.occurence() == MyMoneySchedule::OCCUR_EVERYTHIRTYDAYS);
  CPPUNIT_ASSERT(s.occurencePeriod() == MyMoneySchedule::OCCUR_DAILY);
  CPPUNIT_ASSERT(s.occurenceMultiplier() == 30);

  s.setOccurence(MyMoneySchedule::OCCUR_WEEKLY);
  CPPUNIT_ASSERT(s.occurence() == MyMoneySchedule::OCCUR_WEEKLY);
  CPPUNIT_ASSERT(s.occurencePeriod() == MyMoneySchedule::OCCUR_WEEKLY);
  CPPUNIT_ASSERT(s.occurenceMultiplier() == 1);
  s.setOccurence(MyMoneySchedule::OCCUR_EVERYOTHERWEEK);
  CPPUNIT_ASSERT(s.occurence() == MyMoneySchedule::OCCUR_EVERYOTHERWEEK);
  CPPUNIT_ASSERT(s.occurencePeriod() == MyMoneySchedule::OCCUR_WEEKLY);
  CPPUNIT_ASSERT(s.occurenceMultiplier() == 2);
  // Fortnightly no longer used: Every other week used instead
  s.setOccurence(MyMoneySchedule::OCCUR_FORTNIGHTLY);
  CPPUNIT_ASSERT(s.occurence() == MyMoneySchedule::OCCUR_EVERYOTHERWEEK);
  CPPUNIT_ASSERT(s.occurencePeriod() == MyMoneySchedule::OCCUR_WEEKLY);
  CPPUNIT_ASSERT(s.occurenceMultiplier() == 2);
  s.setOccurence(MyMoneySchedule::OCCUR_EVERYTHREEWEEKS);
  CPPUNIT_ASSERT(s.occurence() == MyMoneySchedule::OCCUR_EVERYTHREEWEEKS);
  CPPUNIT_ASSERT(s.occurencePeriod() == MyMoneySchedule::OCCUR_WEEKLY);
  CPPUNIT_ASSERT(s.occurenceMultiplier() == 3);
  s.setOccurence(MyMoneySchedule::OCCUR_EVERYFOURWEEKS);
  CPPUNIT_ASSERT(s.occurence() == MyMoneySchedule::OCCUR_EVERYFOURWEEKS);
  CPPUNIT_ASSERT(s.occurencePeriod() == MyMoneySchedule::OCCUR_WEEKLY);
  CPPUNIT_ASSERT(s.occurenceMultiplier() == 4);
  s.setOccurence(MyMoneySchedule::OCCUR_EVERYEIGHTWEEKS);
  CPPUNIT_ASSERT(s.occurence() == MyMoneySchedule::OCCUR_EVERYEIGHTWEEKS);
  CPPUNIT_ASSERT(s.occurencePeriod() == MyMoneySchedule::OCCUR_WEEKLY);
  CPPUNIT_ASSERT(s.occurenceMultiplier() == 8);

  s.setOccurence(MyMoneySchedule::OCCUR_EVERYHALFMONTH);
  CPPUNIT_ASSERT(s.occurence() == MyMoneySchedule::OCCUR_EVERYHALFMONTH);
  CPPUNIT_ASSERT(s.occurencePeriod() == MyMoneySchedule::OCCUR_EVERYHALFMONTH);
  CPPUNIT_ASSERT(s.occurenceMultiplier() == 1);

  s.setOccurence(MyMoneySchedule::OCCUR_MONTHLY);
  CPPUNIT_ASSERT(s.occurence() == MyMoneySchedule::OCCUR_MONTHLY);
  CPPUNIT_ASSERT(s.occurencePeriod() == MyMoneySchedule::OCCUR_MONTHLY);
  CPPUNIT_ASSERT(s.occurenceMultiplier() == 1);
  s.setOccurence(MyMoneySchedule::OCCUR_EVERYOTHERMONTH);
  CPPUNIT_ASSERT(s.occurence() == MyMoneySchedule::OCCUR_EVERYOTHERMONTH);
  CPPUNIT_ASSERT(s.occurencePeriod() == MyMoneySchedule::OCCUR_MONTHLY);
  CPPUNIT_ASSERT(s.occurenceMultiplier() == 2);
  s.setOccurence(MyMoneySchedule::OCCUR_EVERYTHREEMONTHS);
  CPPUNIT_ASSERT(s.occurence() == MyMoneySchedule::OCCUR_EVERYTHREEMONTHS);
  CPPUNIT_ASSERT(s.occurencePeriod() == MyMoneySchedule::OCCUR_MONTHLY);
  CPPUNIT_ASSERT(s.occurenceMultiplier() == 3);
  // Quarterly no longer used.  Every three months used instead
  s.setOccurence(MyMoneySchedule::OCCUR_QUARTERLY);
  CPPUNIT_ASSERT(s.occurence() == MyMoneySchedule::OCCUR_EVERYTHREEMONTHS);
  CPPUNIT_ASSERT(s.occurencePeriod() == MyMoneySchedule::OCCUR_MONTHLY);
  CPPUNIT_ASSERT(s.occurenceMultiplier() == 3);
  s.setOccurence(MyMoneySchedule::OCCUR_EVERYFOURMONTHS);
  CPPUNIT_ASSERT(s.occurence() == MyMoneySchedule::OCCUR_EVERYFOURMONTHS);
  CPPUNIT_ASSERT(s.occurencePeriod() == MyMoneySchedule::OCCUR_MONTHLY);
  CPPUNIT_ASSERT(s.occurenceMultiplier() == 4);
  s.setOccurence(MyMoneySchedule::OCCUR_TWICEYEARLY);
  CPPUNIT_ASSERT(s.occurence() == MyMoneySchedule::OCCUR_TWICEYEARLY);
  CPPUNIT_ASSERT(s.occurencePeriod() == MyMoneySchedule::OCCUR_MONTHLY);
  CPPUNIT_ASSERT(s.occurenceMultiplier() == 6);

  s.setOccurence(MyMoneySchedule::OCCUR_YEARLY);
  CPPUNIT_ASSERT(s.occurence() == MyMoneySchedule::OCCUR_YEARLY);
  CPPUNIT_ASSERT(s.occurencePeriod() == MyMoneySchedule::OCCUR_YEARLY);
  CPPUNIT_ASSERT(s.occurenceMultiplier() == 1);
  s.setOccurence(MyMoneySchedule::OCCUR_EVERYOTHERYEAR);
  CPPUNIT_ASSERT(s.occurence() == MyMoneySchedule::OCCUR_EVERYOTHERYEAR);
  CPPUNIT_ASSERT(s.occurencePeriod() == MyMoneySchedule::OCCUR_YEARLY);
  CPPUNIT_ASSERT(s.occurenceMultiplier() == 2);
}

void MyMoneyScheduleTest::testSimpleToFromCompoundOccurence()
{
  // Conversion between Simple and Compound occurences
  // Each simple occurence to compound occurence
  MyMoneySchedule::occurenceE occ;
  int mult; 
  occ = MyMoneySchedule::OCCUR_ONCE; mult = 1;
  MyMoneySchedule::simpleToCompoundOccurence(mult, occ);
  CPPUNIT_ASSERT( occ == MyMoneySchedule::OCCUR_ONCE && mult == 1 );
  occ = MyMoneySchedule::OCCUR_DAILY; mult = 1;
  MyMoneySchedule::simpleToCompoundOccurence(mult, occ);
  CPPUNIT_ASSERT( occ == MyMoneySchedule::OCCUR_DAILY && mult == 1 );
  occ = MyMoneySchedule::OCCUR_WEEKLY; mult = 1;
  MyMoneySchedule::simpleToCompoundOccurence(mult, occ);
  CPPUNIT_ASSERT( occ == MyMoneySchedule::OCCUR_WEEKLY && mult == 1 );
  occ = MyMoneySchedule::OCCUR_EVERYOTHERWEEK; mult = 1;
  MyMoneySchedule::simpleToCompoundOccurence(mult, occ);
  CPPUNIT_ASSERT( occ == MyMoneySchedule::OCCUR_WEEKLY && mult == 2 );
  occ = MyMoneySchedule::OCCUR_FORTNIGHTLY; mult = 1;
  MyMoneySchedule::simpleToCompoundOccurence(mult, occ);
  CPPUNIT_ASSERT( occ == MyMoneySchedule::OCCUR_WEEKLY && mult == 2 );
  occ = MyMoneySchedule::OCCUR_EVERYHALFMONTH; mult = 1;
  MyMoneySchedule::simpleToCompoundOccurence(mult, occ);
  CPPUNIT_ASSERT( occ == MyMoneySchedule::OCCUR_EVERYHALFMONTH && mult == 1 );
  occ = MyMoneySchedule::OCCUR_EVERYTHREEWEEKS; mult = 1;
  MyMoneySchedule::simpleToCompoundOccurence(mult, occ);
  CPPUNIT_ASSERT( occ == MyMoneySchedule::OCCUR_WEEKLY && mult == 3 );
  occ = MyMoneySchedule::OCCUR_EVERYFOURWEEKS; mult = 1;
  MyMoneySchedule::simpleToCompoundOccurence(mult, occ);
  CPPUNIT_ASSERT( occ == MyMoneySchedule::OCCUR_WEEKLY && mult == 4 );
  occ = MyMoneySchedule::OCCUR_EVERYTHIRTYDAYS; mult = 1;
  MyMoneySchedule::simpleToCompoundOccurence(mult, occ);
  CPPUNIT_ASSERT( occ == MyMoneySchedule::OCCUR_DAILY && mult == 30 );
  occ = MyMoneySchedule::OCCUR_MONTHLY; mult = 1;
  MyMoneySchedule::simpleToCompoundOccurence(mult, occ);
  CPPUNIT_ASSERT( occ == MyMoneySchedule::OCCUR_MONTHLY && mult == 1 );
  occ = MyMoneySchedule::OCCUR_EVERYEIGHTWEEKS; mult = 1;
  MyMoneySchedule::simpleToCompoundOccurence(mult, occ);
  CPPUNIT_ASSERT( occ == MyMoneySchedule::OCCUR_WEEKLY && mult == 8 );
  occ = MyMoneySchedule::OCCUR_EVERYOTHERMONTH; mult = 1;
  MyMoneySchedule::simpleToCompoundOccurence(mult, occ);
  CPPUNIT_ASSERT( occ == MyMoneySchedule::OCCUR_MONTHLY && mult == 2 );
  occ = MyMoneySchedule::OCCUR_EVERYTHREEMONTHS; mult = 1;
  MyMoneySchedule::simpleToCompoundOccurence(mult, occ);
  CPPUNIT_ASSERT( occ == MyMoneySchedule::OCCUR_MONTHLY && mult == 3 );
  occ = MyMoneySchedule::OCCUR_QUARTERLY; mult = 1;
  MyMoneySchedule::simpleToCompoundOccurence(mult, occ);
  CPPUNIT_ASSERT( occ == MyMoneySchedule::OCCUR_MONTHLY && mult == 3 );
  occ = MyMoneySchedule::OCCUR_EVERYFOURMONTHS; mult = 1;
  MyMoneySchedule::simpleToCompoundOccurence(mult, occ);
  CPPUNIT_ASSERT( occ == MyMoneySchedule::OCCUR_MONTHLY && mult == 4 );
  occ = MyMoneySchedule::OCCUR_TWICEYEARLY; mult = 1;
  MyMoneySchedule::simpleToCompoundOccurence(mult, occ);
  CPPUNIT_ASSERT( occ == MyMoneySchedule::OCCUR_MONTHLY && mult == 6 );
  occ = MyMoneySchedule::OCCUR_YEARLY; mult = 1;
  MyMoneySchedule::simpleToCompoundOccurence(mult, occ);
  CPPUNIT_ASSERT( occ == MyMoneySchedule::OCCUR_YEARLY && mult == 1 );
  occ = MyMoneySchedule::OCCUR_EVERYOTHERYEAR; mult = 1;
  MyMoneySchedule::simpleToCompoundOccurence(mult, occ);
  CPPUNIT_ASSERT( occ == MyMoneySchedule::OCCUR_YEARLY && mult == 2 );
  // Compound to Simple Occurences
  occ = MyMoneySchedule::OCCUR_ONCE; mult = 1;
  MyMoneySchedule::compoundToSimpleOccurence(mult, occ);
  CPPUNIT_ASSERT( occ == MyMoneySchedule::OCCUR_ONCE && mult == 1 );
  occ = MyMoneySchedule::OCCUR_DAILY; mult = 1;
  MyMoneySchedule::compoundToSimpleOccurence(mult, occ);
  CPPUNIT_ASSERT( occ == MyMoneySchedule::OCCUR_DAILY && mult == 1 );
  occ = MyMoneySchedule::OCCUR_WEEKLY; mult = 1;
  MyMoneySchedule::compoundToSimpleOccurence(mult, occ);
  CPPUNIT_ASSERT( occ == MyMoneySchedule::OCCUR_WEEKLY && mult == 1 );
  occ = MyMoneySchedule::OCCUR_WEEKLY; mult = 2;
  MyMoneySchedule::compoundToSimpleOccurence(mult, occ);
  CPPUNIT_ASSERT( occ == MyMoneySchedule::OCCUR_EVERYOTHERWEEK && mult == 1 );
  // MyMoneySchedule::OCCUR_FORTNIGHTLY not converted back
  occ = MyMoneySchedule::OCCUR_EVERYHALFMONTH; mult = 1;
  MyMoneySchedule::compoundToSimpleOccurence(mult, occ);
  CPPUNIT_ASSERT( occ == MyMoneySchedule::OCCUR_EVERYHALFMONTH && mult == 1 );
  occ = MyMoneySchedule::OCCUR_WEEKLY; mult = 3;
  MyMoneySchedule::compoundToSimpleOccurence(mult, occ);
  CPPUNIT_ASSERT( occ == MyMoneySchedule::OCCUR_EVERYTHREEWEEKS && mult == 1 );
  occ = MyMoneySchedule::OCCUR_WEEKLY ; mult = 4;
  MyMoneySchedule::compoundToSimpleOccurence(mult, occ);
  CPPUNIT_ASSERT( occ == MyMoneySchedule::OCCUR_EVERYFOURWEEKS && mult == 1 );
  occ = MyMoneySchedule::OCCUR_DAILY; mult = 30;
  MyMoneySchedule::compoundToSimpleOccurence(mult, occ);
  CPPUNIT_ASSERT( occ == MyMoneySchedule::OCCUR_EVERYTHIRTYDAYS && mult == 1 );
  occ = MyMoneySchedule::OCCUR_MONTHLY; mult = 1;
  MyMoneySchedule::compoundToSimpleOccurence(mult, occ);
  CPPUNIT_ASSERT( occ == MyMoneySchedule::OCCUR_MONTHLY && mult == 1 );
  occ = MyMoneySchedule::OCCUR_WEEKLY; mult = 8;
  MyMoneySchedule::compoundToSimpleOccurence(mult, occ);
  CPPUNIT_ASSERT( occ == MyMoneySchedule::OCCUR_EVERYEIGHTWEEKS && mult == 1 );
  occ = MyMoneySchedule::OCCUR_MONTHLY; mult = 2;
  MyMoneySchedule::compoundToSimpleOccurence(mult, occ);
  CPPUNIT_ASSERT( occ == MyMoneySchedule::OCCUR_EVERYOTHERMONTH && mult == 1 );
  occ = MyMoneySchedule::OCCUR_MONTHLY; mult = 3;
  MyMoneySchedule::compoundToSimpleOccurence(mult, occ);
  CPPUNIT_ASSERT( occ == MyMoneySchedule::OCCUR_EVERYTHREEMONTHS && mult == 1 );
  // MyMoneySchedule::OCCUR_QUARTERLY not converted back 
  occ = MyMoneySchedule::OCCUR_MONTHLY; mult = 4;
  MyMoneySchedule::compoundToSimpleOccurence(mult, occ);
  CPPUNIT_ASSERT( occ == MyMoneySchedule::OCCUR_EVERYFOURMONTHS && mult == 1 );
  occ = MyMoneySchedule::OCCUR_MONTHLY; mult = 6;
  MyMoneySchedule::compoundToSimpleOccurence(mult, occ);
  CPPUNIT_ASSERT( occ == MyMoneySchedule::OCCUR_TWICEYEARLY && mult == 1 );
  occ = MyMoneySchedule::OCCUR_YEARLY; mult = 1;
  MyMoneySchedule::compoundToSimpleOccurence(mult, occ);
  CPPUNIT_ASSERT( occ == MyMoneySchedule::OCCUR_YEARLY && mult == 1 );
  occ = MyMoneySchedule::OCCUR_YEARLY; mult = 2;
  MyMoneySchedule::compoundToSimpleOccurence(mult, occ);
  CPPUNIT_ASSERT( occ == MyMoneySchedule::OCCUR_EVERYOTHERYEAR && mult == 1 );
}
