/***************************************************************************
                          mymoneyfiletest.h
                          -------------------
    copyright            : (C) 2002 by Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __MYMONEYFILETEST_H__
#define __MYMONEYFILETEST_H__

#include <cppunit/extensions/HelperMacros.h>
#include "autotest.h"

#define private public
#define protected public
#include <Q3ValueList>
#include "mymoneyfile.h"
#include "storage/mymoneyseqaccessmgr.h"
#undef private
#undef protected

class MyMoneyFileTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(MyMoneyFileTest);
  CPPUNIT_TEST(testEmptyConstructor);
  CPPUNIT_TEST(testAddOneInstitution);
  CPPUNIT_TEST(testAddTwoInstitutions);
  CPPUNIT_TEST(testInstitutionRetrieval);
  CPPUNIT_TEST(testRemoveInstitution);
  CPPUNIT_TEST(testInstitutionListRetrieval);
  CPPUNIT_TEST(testInstitutionModify);
  CPPUNIT_TEST(testSetFunctions);
  CPPUNIT_TEST(testAddAccounts);
  CPPUNIT_TEST(testModifyAccount);
  CPPUNIT_TEST(testModifyStdAccount);
  CPPUNIT_TEST(testReparentAccount);
  CPPUNIT_TEST(testRemoveAccount);
  CPPUNIT_TEST(testRemoveAccountTree);
  CPPUNIT_TEST(testAccountListRetrieval);
  CPPUNIT_TEST(testAddTransaction);
  CPPUNIT_TEST(testHasActiveSplits);
  CPPUNIT_TEST(testIsStandardAccount);
  CPPUNIT_TEST(testModifyTransactionSimple);
  CPPUNIT_TEST(testModifyTransactionNewPostDate);
  CPPUNIT_TEST(testModifyTransactionNewAccount);
  CPPUNIT_TEST(testRemoveTransaction);
  CPPUNIT_TEST(testBalanceTotal);
  CPPUNIT_TEST(testSetAccountName);
  CPPUNIT_TEST(testAddPayee);
  CPPUNIT_TEST(testModifyPayee);
  CPPUNIT_TEST(testRemovePayee);
  CPPUNIT_TEST(testAddTransactionStd);
  CPPUNIT_TEST(testAttachStorage);
  CPPUNIT_TEST(testAccount2Category);
  CPPUNIT_TEST(testCategory2Account);
  CPPUNIT_TEST(testAttachedStorage);
  CPPUNIT_TEST(testHasAccount);
  CPPUNIT_TEST(testAddEquityAccount);
  CPPUNIT_TEST(testReparentEquity);
  CPPUNIT_TEST(testBaseCurrency);
  CPPUNIT_TEST(testOpeningBalanceNoBase);
  CPPUNIT_TEST(testOpeningBalance);
#if 0
  CPPUNIT_TEST(testMoveSplits);
#endif
  CPPUNIT_TEST_SUITE_END();
protected:
  MyMoneyFile *m;
  MyMoneySeqAccessMgr* storage;
  MyMoneyAccount  m_inv;

public:
  MyMoneyFileTest();

  void setUp();
  void tearDown();
  void testEmptyConstructor();
  void testAddOneInstitution();
  void testAddTwoInstitutions();
  void testRemoveInstitution();
  void testInstitutionRetrieval();
  void testInstitutionListRetrieval();
  void testInstitutionModify();
  void testSetFunctions();
  void testAddAccounts();
  void testModifyAccount();
  void testModifyStdAccount();
  void testReparentAccount();
  void testRemoveAccount();
  void testRemoveAccountTree();
  void testAccountListRetrieval();
  void testAddTransaction();
  void testIsStandardAccount();
  void testHasActiveSplits();
  void testModifyTransactionSimple();
  void testModifyTransactionNewPostDate();
  void testModifyTransactionNewAccount();
  void testRemoveTransaction();
  void testBalanceTotal();
  void testSetAccountName();
  void testAddPayee();
  void testModifyPayee();
  void testRemovePayee();
  void testAddTransactionStd();
  void testAttachStorage();
  void testAccount2Category();
  void testCategory2Account();
  void testAttachedStorage();
  void testHasAccount();
  void testAddEquityAccount();
  void testReparentEquity();
  void testReparentEquity(Q3ValueList<MyMoneyAccount::accountTypeE>& list, MyMoneyAccount& parent);
  void testBaseCurrency();
  void testOpeningBalanceNoBase();
  void testOpeningBalance();

private:
  void testRemoveStdAccount(const MyMoneyAccount& acc);
};

#endif
