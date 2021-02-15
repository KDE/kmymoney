/*
    SPDX-FileCopyrightText: 2002-2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2004 Ace Jones <acejones@users.sourceforge.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MYMONEYFILETEST_H
#define MYMONEYFILETEST_H

#include <QObject>
#include <QList>

#define KMM_MYMONEY_UNIT_TESTABLE friend class MyMoneyFileTest;

#include "mymoneyfile.h"
#include "mymoneyaccount.h"

class MyMoneyFileTest : public QObject
{
  Q_OBJECT
public:
  MyMoneyFileTest();

protected:
  MyMoneyFile *m;
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
  /// @todo cleanup
  // void testSetAccountName();
  void testAddPayee();
  void testModifyPayee();
  void testRemovePayee();
  void testPayeeWithIdentifier();
  void testAddTransactionStd();
  void testAccount2Category();
  void testCategory2Account();
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
  void addOneAccount();
  void setupBaseCurrency();

private:
  QStringList m_objectsAdded;
  QStringList m_objectsModified;
  QStringList m_objectsRemoved;
  QStringList m_balanceChanged;
  QStringList m_valueChanged;
};

#endif
