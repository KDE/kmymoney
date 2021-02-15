/*
 * SPDX-FileCopyrightText: 2009-2012 Thomas Baumgart <tbaumgart@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef MYMONEYSTORAGEMGRTEST_H
#define MYMONEYSTORAGEMGRTEST_H

#include <QObject>

#define KMM_MYMONEY_UNIT_TESTABLE friend class MyMoneyStorageMgrTest;

#include "mymoneyobject.h"
#include "mymoneystoragemgr.h"

class MyMoneyStorageMgrTest : public QObject
{
  Q_OBJECT

public:
  void testAccount();
protected:
  MyMoneyStorageMgr *m;
private Q_SLOTS:
  void init();
  void cleanup();
  void testEmptyConstructor();
  void testSetFunctions();
  void testIsStandardAccount();
  void testNewAccount();
  void testAddNewAccount();
  void testAddInstitution();
  void testInstitution();
  void testAccount2Institution();
  void testModifyAccount();
  void testModifyInstitution();
  void testReparentAccount();
  void testAddTransactions();
  void testTransactionCount();
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
  void testAddTag();
  void testModifyTag();
  void testTagName();
  void testRemoveTag();
  void testRemoveAccountFromTree();
//  void testAssignment();
  void testEquality(const MyMoneyStorageMgr* t);
//  void testDuplicate();
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
  void testLoaderFunctions();
  void testAddOnlineJob();
};

#endif
