/***************************************************************************
                          convertertest.cpp
                          -------------------
    copyright            : (C) 2002 by Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
                           Ace Jones <ace.j@hotpop.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <config-kmymoney.h>

#include <q3valuelist.h>
#include <q3valuevector.h>
#include <qdom.h>
#include <qfile.h>

#include <kdebug.h>
#include <kdeversion.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include "convertertest.h"

// uses helper functions from reports tests
#include "../reports/reportstestcommon.h"
using namespace test;

#include <mymoneysecurity.h>
#include <mymoneyprice.h>
#include <mymoneyreport.h>
#include <mymoneystatement.h>
#include "storage/mymoneystoragexml.h"
#include "storage/mymoneystoragedump.h"

#define private public
#include "../converter/webpricequote.h"
#undef private

ConverterTest::ConverterTest()
{
}

using namespace convertertest;

void ConverterTest::setUp () {

  storage = new MyMoneySeqAccessMgr;
  file = MyMoneyFile::instance();
  file->attachStorage(storage);

  MyMoneyFileTransaction ft;

  file->addCurrency(MyMoneySecurity("CAD", "Canadian Dollar",        "C$"));
  file->addCurrency(MyMoneySecurity("USD", "US Dollar",              "$"));
  file->addCurrency(MyMoneySecurity("JPY", "Japanese Yen",           QChar(0x00A5), 100, 1));
  file->addCurrency(MyMoneySecurity("GBP", "British Pound",           "#"));
  file->setBaseCurrency(file->currency("USD"));

  MyMoneyPayee payeeTest("Test Payee");
  file->addPayee(payeeTest);
  MyMoneyPayee payeeTest2("Thomas Baumgart");
  file->addPayee(payeeTest2);

  acAsset = (MyMoneyFile::instance()->asset().id());
  acLiability = (MyMoneyFile::instance()->liability().id());
  acExpense = (MyMoneyFile::instance()->expense().id());
  acIncome = (MyMoneyFile::instance()->income().id());
  acChecking = makeAccount("Checking Account",MyMoneyAccount::Checkings,moConverterCheckingOpen,QDate(2004,5,15),acAsset);
  acCredit = makeAccount("Credit Card",MyMoneyAccount::CreditCard,moConverterCreditOpen,QDate(2004,7,15),acLiability);
  acSolo = makeAccount("Solo",MyMoneyAccount::Expense,0,QDate(2004,1,11),acExpense);
  acParent = makeAccount("Parent",MyMoneyAccount::Expense,0,QDate(2004,1,11),acExpense);
  acChild = makeAccount("Child",MyMoneyAccount::Expense,0,QDate(2004,2,11),acParent);
  acForeign = makeAccount("Foreign",MyMoneyAccount::Expense,0,QDate(2004,1,11),acExpense);

  MyMoneyInstitution i("Bank of the World","","","","","","");
  file->addInstitution(i);
  inBank = i.id();
  ft.commit();
}

void ConverterTest::tearDown ()
{
  file->detachStorage(storage);
  delete storage;
}

void ConverterTest::testWebQuotes()
{
#ifdef PERFORM_ONLINE_TESTS
  try
  {
    WebPriceQuote q;
    QuoteReceiver qr(&q);

    q.launch("DIS");

//    kDebug(2) << "ConverterTest::testWebQuotes(): quote for " << q.m_symbol << " on " << qr.m_date.toString() << " is " << qr.m_price.toString() << " errors(" << qr.m_errors.count() << "): " << qr.m_errors.join(" /// ");

    // No errors allowed
    CPPUNIT_ASSERT(qr.m_errors.count() == 0);

    // Quote date should be within the last week, or something bad is going on.
    CPPUNIT_ASSERT(qr.m_date <= QDate::currentDate());
    CPPUNIT_ASSERT(qr.m_date >= QDate::currentDate().addDays(-7));

    // Quote value should at least be positive
    CPPUNIT_ASSERT(qr.m_price.isPositive());

    q.launch("MF8AAUKS.L","Yahoo UK");

//    kDebug(2) << "ConverterTest::testWebQuotes(): quote for " << q.m_symbol << " on " << qr.m_date.toString() << " is " << qr.m_price.toString() << " errors(" << qr.m_errors.count() << "): " << qr.m_errors.join(" /// ");

    CPPUNIT_ASSERT(qr.m_errors.count() == 0);
    CPPUNIT_ASSERT(qr.m_date <= QDate::currentDate().addDays(1));
    CPPUNIT_ASSERT(qr.m_date >= QDate::currentDate().addDays(-7));
    CPPUNIT_ASSERT(qr.m_price.isPositive());

    q.launch("EUR > USD","Yahoo Currency");

//    kDebug(2) << "ConverterTest::testWebQuotes(): quote for " << q.m_symbol << " on " << qr.m_date.toString() << " is " << qr.m_price.toString() << " errors(" << qr.m_errors.count() << "): " << qr.m_errors.join(" /// ");

    CPPUNIT_ASSERT(qr.m_errors.count() == 0);
    CPPUNIT_ASSERT(qr.m_date <= QDate::currentDate().addDays(1));
    CPPUNIT_ASSERT(qr.m_date >= QDate::currentDate().addDays(-7));
    CPPUNIT_ASSERT(qr.m_price.isPositive());

    q.launch("50492","Globe & Mail");

//    kDebug(2) << "ConverterTest::testWebQuotes(): quote for " << q.m_symbol << " on " << qr.m_date.toString() << " is " << qr.m_price.toString() << " errors(" << qr.m_errors.count() << "): " << qr.m_errors.join(" /// ");

    CPPUNIT_ASSERT(qr.m_errors.count() == 0);
    CPPUNIT_ASSERT(qr.m_date <= QDate::currentDate().addDays(1));
    CPPUNIT_ASSERT(qr.m_date >= QDate::currentDate().addDays(-7));
    CPPUNIT_ASSERT(qr.m_price.isPositive());

    q.launch("TDB647","MSN.CA");

//    kDebug(2) << "ConverterTest::testWebQuotes(): quote for " << q.m_symbol << " on " << qr.m_date.toString() << " is " << qr.m_price.toString() << " errors(" << qr.m_errors.count() << "): " << qr.m_errors.join(" /// ");

    CPPUNIT_ASSERT(qr.m_errors.count() == 0);
    CPPUNIT_ASSERT(qr.m_date <= QDate::currentDate().addDays(1));
    CPPUNIT_ASSERT(qr.m_date >= QDate::currentDate().addDays(-7));
    CPPUNIT_ASSERT(qr.m_price.isPositive());

  }
  catch (MyMoneyException* e)
  {
    CPPUNIT_FAIL(e->what());
  }
#endif
}

void ConverterTest::testDateFormat()
{
  try
  {
    MyMoneyDateFormat format("%mm-%dd-%yyyy");

    CPPUNIT_ASSERT(format.convertString("1-5-2005") == QDate(2005,1,5));
    CPPUNIT_ASSERT(format.convertString("jan-15-2005") == QDate(2005,1,15));
    CPPUNIT_ASSERT(format.convertString("august-25-2005") == QDate(2005,8,25));

    format = MyMoneyDateFormat("%mm/%dd/%yy");

    CPPUNIT_ASSERT(format.convertString("1/5/05") == QDate(2005,1,5));
    CPPUNIT_ASSERT(format.convertString("jan/15/05") == QDate(2005,1,15));
    CPPUNIT_ASSERT(format.convertString("august/25/05") == QDate(2005,8,25));

    format = MyMoneyDateFormat("%d\\.%m\\.%yy");

    CPPUNIT_ASSERT(format.convertString("1.5.05") == QDate(2005,5,1));
    CPPUNIT_ASSERT(format.convertString("15.jan.05") == QDate(2005,1,15));
    CPPUNIT_ASSERT(format.convertString("25.august.05") == QDate(2005,8,25));

    format = MyMoneyDateFormat("%yyyy\\\\%dddd\\\\%mmmmmmmmmmm");

    CPPUNIT_ASSERT(format.convertString("2005\\31\\12") == QDate(2005,12,31));
    CPPUNIT_ASSERT(format.convertString("2005\\15\\jan") == QDate(2005,1,15));
    CPPUNIT_ASSERT(format.convertString("2005\\25\\august") == QDate(2005,8,25));

    format = MyMoneyDateFormat("%m %dd, %yyyy");

    CPPUNIT_ASSERT(format.convertString("jan 15, 2005") == QDate(2005,1,15));
    CPPUNIT_ASSERT(format.convertString("august 25, 2005") == QDate(2005,8,25));
    CPPUNIT_ASSERT(format.convertString("january 1st, 2005") == QDate(2005,1,1));

    format = MyMoneyDateFormat("%m %d %y");

    CPPUNIT_ASSERT(format.convertString("12/31/50",false,2000) == QDate(1950,12,31));
    CPPUNIT_ASSERT(format.convertString("1/1/90",false,2000) == QDate(1990,1,1));
    CPPUNIT_ASSERT(format.convertString("december 31st, 5",false) == QDate(2005,12,31));
  }
  catch (MyMoneyException* e)
  {
    CPPUNIT_FAIL(e->what());
  }
}

// vim:cin:si:ai:et:ts=2:sw=2:
