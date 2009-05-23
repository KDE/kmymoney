/***************************************************************************
                          mymoneydatabasemgrtest.h
                          -------------------
    copyright            : (C) 2008 by Fernando Vilas
    email                : fvilas@iname.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __MYMONEYDATABASEMGRTEST_H__
#define __MYMONEYDATABASEMGRTEST_H__

#include <cppunit/TestCaller.h>
#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>
#include <cppunit/extensions/HelperMacros.h>

#include "autotest.h"

#define private public
#define protected public
#include "mymoneyobject.h"
#include "mymoneydatabasemgr.h"
#undef private

class MyMoneyDatabaseMgrTest : public CppUnit::TestFixture  {
  CPPUNIT_TEST_SUITE(MyMoneyDatabaseMgrTest);
  CPPUNIT_TEST(testEmptyConstructor);
  CPPUNIT_TEST(testCreateDb);
  CPPUNIT_TEST(testAttachDb);
  CPPUNIT_TEST(testSetFunctions);
  CPPUNIT_TEST(testSupportFunctions);
  CPPUNIT_TEST(testIsStandardAccount);
  CPPUNIT_TEST(testNewAccount);
  CPPUNIT_TEST(testAddNewAccount);
  CPPUNIT_TEST(testReparentAccount);
  CPPUNIT_TEST(testAddInstitution);
  CPPUNIT_TEST(testInstitution);
  CPPUNIT_TEST(testAccount2Institution);
  CPPUNIT_TEST(testModifyAccount);
  CPPUNIT_TEST(testModifyInstitution);
  CPPUNIT_TEST(testAddTransactions);
  CPPUNIT_TEST(testTransactionCount);
  CPPUNIT_TEST(testBalance);
  CPPUNIT_TEST(testAddBudget);
  CPPUNIT_TEST(testCopyBudget);
  CPPUNIT_TEST(testModifyBudget);
  CPPUNIT_TEST(testRemoveBudget);
  CPPUNIT_TEST(testModifyTransaction);
  CPPUNIT_TEST(testRemoveUnusedAccount);
  CPPUNIT_TEST(testRemoveUsedAccount);
  CPPUNIT_TEST(testRemoveInstitution);
  CPPUNIT_TEST(testRemoveTransaction);
  CPPUNIT_TEST(testTransactionList);
  CPPUNIT_TEST(testAddPayee);
  CPPUNIT_TEST(testSetAccountName);
  CPPUNIT_TEST(testModifyPayee);
  CPPUNIT_TEST(testPayeeName);
  CPPUNIT_TEST(testRemovePayee);
  CPPUNIT_TEST(testRemoveAccountFromTree);
  CPPUNIT_TEST(testAssignment);
  CPPUNIT_TEST(testDuplicate);
  CPPUNIT_TEST(testAddSchedule);
  CPPUNIT_TEST(testModifySchedule);
  CPPUNIT_TEST(testRemoveSchedule);
  CPPUNIT_TEST(testSchedule);
  CPPUNIT_TEST(testScheduleList);
  CPPUNIT_TEST(testAddCurrency);
  CPPUNIT_TEST(testModifyCurrency);
  CPPUNIT_TEST(testRemoveCurrency);
  CPPUNIT_TEST(testCurrency);
  CPPUNIT_TEST(testCurrencyList);
  CPPUNIT_TEST(testAccountList);
  CPPUNIT_TEST_SUITE_END();

protected:
  MyMoneyDatabaseMgr *m;
  bool m_dbAttached;
  bool m_canOpen;
  KUrl m_url;
public:
  MyMoneyDatabaseMgrTest();

  void setUp();
  void tearDown();
  void testEmptyConstructor();
  void testCreateDb();
  void testAttachDb();
  void testSetFunctions();
  void testIsStandardAccount();
  void testNewAccount();
  void testAccount();
  void testAddNewAccount();
  void testAddInstitution();
  void testInstitution();
  void testAccount2Institution();
  void testModifyAccount();
  void testModifyInstitution();
  void testReparentAccount();
  void testAddTransactions();
  void testTransactionCount();
  void testAddBudget();
  void testCopyBudget();
  void testModifyBudget();
  void testRemoveBudget();
  void testBalance();
  void testModifyTransaction();
  void testRemoveUnusedAccount();
  void testRemoveUsedAccount();
  void testRemoveInstitution();
  void testRemoveTransaction();
  void testTransactionList();
  void testAddPayee();
  void testSetAccountName();
  void testModifyPayee();
  void testPayeeName();
  void testRemovePayee();
  void testRemoveAccountFromTree();
  void testAssignment();
  void testEquality(const MyMoneyDatabaseMgr* t);
  void testDuplicate();
  void testAddSchedule();
  void testSchedule();
  void testModifySchedule();
  void testRemoveSchedule();
  void testSupportFunctions();
  void testScheduleList();
  void testAddCurrency();
  void testModifyCurrency();
  void testRemoveCurrency();
  void testCurrency();
  void testCurrencyList();
  void testAccountList();
};

#endif
