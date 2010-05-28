/***************************************************************************
                          mymoneyaccounttest.h
                          -------------------
    copyright            : (C) 2002 by Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
                           Ace Jones <ace.jones@hotpop.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KREPORTSVIEWTEST_H
#define KREPORTSVIEWTEST_H

#include <cppunit/extensions/HelperMacros.h>
#include "mymoneyfile.h"
#include "mymoneyseqaccessmgr.h"

class KReportsViewTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(KReportsViewTest);
  CPPUNIT_TEST(testNetWorthSingle);
  CPPUNIT_TEST(testNetWorthOfsetting);
  CPPUNIT_TEST(testNetWorthOpeningPrior);
  CPPUNIT_TEST(testNetWorthDateFilter);
  CPPUNIT_TEST(testSpendingEmpty);
  CPPUNIT_TEST(testSingleTransaction);
  CPPUNIT_TEST(testSubAccount);
  CPPUNIT_TEST(testFilterIEvsIE);
  CPPUNIT_TEST(testFilterALvsAL);
  CPPUNIT_TEST(testFilterALvsIE);
  CPPUNIT_TEST(testFilterAllvsIE);
  CPPUNIT_TEST(testFilterBasics);
  CPPUNIT_TEST(testMultipleCurrencies);
  CPPUNIT_TEST(testAdvancedFilter);
  CPPUNIT_TEST(testColumnType);
  CPPUNIT_TEST(testXMLWrite);
  CPPUNIT_TEST(testQueryBasics);
  CPPUNIT_TEST(testCashFlowAnalysis);
  CPPUNIT_TEST(testAccountQuery);
  CPPUNIT_TEST(testInvestment);
  CPPUNIT_TEST(testWebQuotes);
  CPPUNIT_TEST(testDateFormat);
  CPPUNIT_TEST(testHasReferenceTo);
  CPPUNIT_TEST_SUITE_END();

private:
  MyMoneyAccount  *m;

  MyMoneySeqAccessMgr* storage;
  MyMoneyFile* file;

public:
  KReportsViewTest();
  void setUp();
  void tearDown();
  void testNetWorthSingle();
  void testNetWorthOfsetting();
  void testNetWorthOpeningPrior();
  void testNetWorthDateFilter();
  void testSpendingEmpty();
  void testSingleTransaction();
  void testSubAccount();
  void testFilterIEvsIE();
  void testFilterALvsAL();
  void testFilterALvsIE();
  void testFilterAllvsIE();
  void testFilterBasics();
  void testMultipleCurrencies();
  void testAdvancedFilter();
  void testColumnType();
  void testXMLWrite();
  void testQueryBasics();
  void testCashFlowAnalysis();
  void testAccountQuery();
  void testInvestment();
  void testWebQuotes();
  void testDateFormat();
  void testHasReferenceTo();
};

#endif
