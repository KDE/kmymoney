/*
 * Copyright 2002-2018  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2004       Ace Jones <acejones@users.sourceforge.net>
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

#ifndef MYMONEYFILETEST_H
#define MYMONEYFILETEST_H

#include <QObject>
#include <QList>

#define KMM_MYMONEY_UNIT_TESTABLE friend class MyMoneyFileTest;

#include "mymoneyfile.h"
#include "mymoneyaccount.h"
#include "storage/mymoneystoragemgr.h"

class MyMoneyFileTest : public QObject
{
  Q_OBJECT
public:
  MyMoneyFileTest();

protected:
  MyMoneyFile *m;
  MyMoneyStorageMgr* storage;
  MyMoneyAccount  m_inv;

private Q_SLOTS:
  void initTestCase();
  void init();
  void cleanup();
  void testEmptyConstructor();
  void testAddOneInstitution();
  void testAddTwoInstitutions();
  void testRemoveInstitution();
  void testInstitutionRetrieval();
  void testInstitutionListRetrieval();
  void testInstitutionModify();
  void testSetFunctions();
  void testAddAccounts();
  void testAddCategories();
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
  void testPayeeWithIdentifier();
  void testAddTransactionStd();
  void testAttachStorage();
  void testAccount2Category();
  void testCategory2Account();
  void testAttachedStorage();
  void testHasAccount();
  void testAddEquityAccount();
  void testReparentEquity();
  void testBaseCurrency();
  void testOpeningBalanceNoBase();
  void testOpeningBalance();
  void testAddPrice();
  void testRemovePrice();
  void testGetPrice();
  void testAddAccountMissingCurrency();
  void testAddTransactionToClosedAccount();
  void testRemoveTransactionFromClosedAccount();
  void testModifyTransactionInClosedAccount();
  void testStorageId();
  void testHasMatchingOnlineBalance_emptyAccountWithoutImportedBalance();
  void testHasMatchingOnlineBalance_emptyAccountWithEqualImportedBalance();
  void testHasMatchingOnlineBalance_emptyAccountWithUnequalImportedBalance();
  void testHasNewerTransaction_withoutAnyTransaction_afterLastImportedTransaction();
  void testHasNewerTransaction_withoutNewerTransaction_afterLastImportedTransaction();
  void testHasNewerTransaction_withNewerTransaction_afterLastImportedTransaction();
  void testCountTransactionsWithSpecificReconciliationState_noTransactions();
  void testCountTransactionsWithSpecificReconciliationState_transactionWithWantedReconcileState();
  void testCountTransactionsWithSpecificReconciliationState_transactionWithUnwantedReconcileState();
  void testAddOnlineJob();
  void testGetOnlineJob();
  void testRemoveOnlineJob();
  void testRemoveLockedOnlineJob();
  void testOnlineJobRollback();
  void testModifyOnlineJob();
  void testClearedBalance();
  void testAdjustedValues();
  void testVatAssignment();
  void testEmptyFilter();
  void testAddSecurity();

private Q_SLOTS:
  void objectAdded(eMyMoney::File::Object type, const QString &id);
  void objectModified(eMyMoney::File::Object type, const QString &id);
  void objectRemoved(eMyMoney::File::Object type, const QString& id);
  void balanceChanged(const MyMoneyAccount& account);
  void valueChanged(const MyMoneyAccount& account);

private:
  void testRemoveStdAccount(const MyMoneyAccount& acc);
  void testReparentEquity(QList<eMyMoney::Account::Type>& list, MyMoneyAccount& parent);
  void clearObjectLists();
  void AddOneAccount();

private:
  QStringList m_objectsAdded;
  QStringList m_objectsModified;
  QStringList m_objectsRemoved;
  QStringList m_balanceChanged;
  QStringList m_valueChanged;
};

#endif
