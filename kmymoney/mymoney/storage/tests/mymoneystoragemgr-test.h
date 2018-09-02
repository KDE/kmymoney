/*
 * Copyright 2009-2012  Thomas Baumgart <tbaumgart@kde.org>
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
