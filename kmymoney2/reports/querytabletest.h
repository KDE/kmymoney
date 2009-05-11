/***************************************************************************
                          querytabletest.h
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

#ifndef QUERYTABLETEST_H
#define QUERYTABLETEST_H

#include <cppunit/extensions/HelperMacros.h>
#include "../mymoney/mymoneyfile.h"
#include "../mymoney/storage/mymoneyseqaccessmgr.h"

class QueryTableTest : public CppUnit::TestFixture  {
  CPPUNIT_TEST_SUITE(QueryTableTest);
  CPPUNIT_TEST(testQueryBasics);
  CPPUNIT_TEST(testCashFlowAnalysis);
  CPPUNIT_TEST(testAccountQuery);
  CPPUNIT_TEST(testInvestment);
  CPPUNIT_TEST(testBalanceColumn);
  CPPUNIT_TEST(testTaxReport);
  CPPUNIT_TEST_SUITE_END();

private:
  MyMoneyAccount  *m;

  MyMoneySeqAccessMgr* storage;
  MyMoneyFile* file;

public:
  QueryTableTest();
  void setUp ();
  void tearDown ();
  void testQueryBasics();
  void testCashFlowAnalysis();
  void testAccountQuery();
  void testInvestment();
  void testBalanceColumn();
  void testTaxReport();
};

#endif
