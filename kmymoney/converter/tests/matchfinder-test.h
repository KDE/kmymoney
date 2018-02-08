/***************************************************************************
    KMyMoney transaction importing module - tests for ExistingTransactionMatchFinder

    copyright            : (C) 2012 by Lukasz Maszczynski <lukasz@maszczynski.net>

***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MATCHFINDERTEST_H
#define MATCHFINDERTEST_H

#include <QObject>
#include <QScopedPointer>

#include "mymoneyaccount.h"
#include "mymoneypayee.h"
#include "mymoneystoragemgr.h"
#include "existingtransactionmatchfinder.h"
#include "scheduledtransactionmatchfinder.h"

class MyMoneyFile;

class MatchFinderTest : public QObject
{
  Q_OBJECT

private:
  MyMoneyFile *                       file;
  QScopedPointer<MyMoneyAccount>      account;
  QScopedPointer<MyMoneyAccount>      otherAccount;
  QScopedPointer<MyMoneyStorageMgr> storage;
  MyMoneyPayee                        payee;
  MyMoneyPayee                        otherPayee;
  static const int                    MATCH_WINDOW = 4;

  MyMoneyTransaction                  ledgerTransaction;
  MyMoneyTransaction                  importTransaction;
  TransactionMatchFinder::MatchResult matchResult;
  QScopedPointer<ExistingTransactionMatchFinder> existingTrFinder;

  MyMoneySchedule                     schedule;
  QScopedPointer<ScheduledTransactionMatchFinder> scheduledTrFinder;

  void setupStorage();
  void setupCurrency();
  void setupAccounts();
  void setupPayees();

  MyMoneyTransaction buildDefaultTransaction() const;
  MyMoneyTransaction buildMatchedTransaction(MyMoneyTransaction transaction) const;
  QString addTransactionToLedger(MyMoneyTransaction transaction) const;

  MyMoneySchedule buildNonOverdueSchedule() const;
  void addSchedule(MyMoneySchedule schedule) const;

  void expectMatchWithExistingTransaction(TransactionMatchFinder::MatchResult expectedResult);
  void expectMatchWithScheduledTransaction(TransactionMatchFinder::MatchResult expectedResult);

private Q_SLOTS:
  void init();
  void cleanup();

  void testDuplicate_allMatch();
  void testDuplicate_payeeEmpty();
  void testDuplicate_payeeMismatch();
  void testDuplicate_splitIsMarkedAsMatched();

  void testPreciseMatch_noBankId();
  void testPreciseMatch_importBankId();
  void testPreciseMatch_payeeEmpty();

  void testImpreciseMatch_matchWindowLowerBound();
  void testImpreciseMatch_matchWindowUpperBound();
  void testImpreciseMatch_payeeEmpty();

  void testNoMatch_bankIdMismatch();
  void testNoMatch_ledgerBankIdNotEmpty();
  void testNoMatch_accountMismatch_withBankId();
  void testNoMatch_accountMismatch_noBankId();
  void testNoMatch_amountMismatch_withBankId();
  void testNoMatch_amountMismatch_noBankId();
  void testNoMatch_payeeMismatch();
  void testNoMatch_splitIsMarkedAsMatched();
  void testNoMatch_postDateMismatch_withBankId();
  void testNoMatch_postDateMismatch_noBankId();

  void testExistingTransactionMatch_sameTransactionId_withBankId();
  void testExistingTransactionMatch_sameTransactionId_noBankId();
  void testExistingTransactionMatch_multipleAccounts_withBankId();
  void testExistingTransactionMatch_multipleAccounts_noBankId();

  void testScheduleMatch_allMatch();
  void testScheduleMatch_dueDateWithinMatchWindow();
  void testScheduleMatch_amountWithinAllowedVariation();
  void testScheduleMatch_overdue();
  void testScheduleMismatch_dueDate();
  void testScheduleMismatch_amount();
};

#endif // MATCHFINDERTEST_H
