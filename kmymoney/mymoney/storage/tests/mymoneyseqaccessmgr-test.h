/***************************************************************************
                          mymoneyseqaccessmgrtest.h
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

#ifndef MYMONEYSEQACCESSMGRTEST_H
#define MYMONEYSEQACCESSMGRTEST_H

#include <QObject>

#define KMM_MYMONEY_UNIT_TESTABLE friend class MyMoneySeqAccessMgrTest;

#include "mymoneyobject.h"
#include "mymoneyseqaccessmgr.h"

class MyMoneySeqAccessMgrTest : public QObject
{
  Q_OBJECT

public:
  void testAccount();
protected:
  MyMoneySeqAccessMgr *m;
private slots:
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
  void testAssignment();
  void testEquality(const MyMoneySeqAccessMgr* t);
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
  void testLoaderFunctions();
  void testAddOnlineJob();
};

#endif
