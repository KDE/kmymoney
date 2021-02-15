/*
    KMyMoney transaction importing module - searches for a matching transaction in the ledger

    SPDX-FileCopyrightText: 2012 Lukasz Maszczynski <lukasz@maszczynski.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef EXISTINGTRANSACTIONMATCHFINDER_H
#define EXISTINGTRANSACTIONMATCHFINDER_H

#include <QPair>
#include <QList>

#include "transactionmatchfinder.h"

/** Implements searching for a matching transaction in the ledger
 */
class ExistingTransactionMatchFinder : public TransactionMatchFinder
{
public:
  /** Ctor, initializes the match finder
   * @param matchWindow max number of days the transactions may vary and still be considered to be matching
   */
  explicit ExistingTransactionMatchFinder(int m_matchWindow = 3);

protected:
  typedef QPair<MyMoneyTransaction, MyMoneySplit> TransactionAndSplitPair;
  QList<TransactionAndSplitPair> listOfMatchCandidates;

  /** Creates a list of transactions within matchWindow range and with the same amount as the imported transaction we're trying to match
   */
  void createListOfMatchCandidates() final override;

  /** Searches for a matching transaction in the ledger
   *
   * Match result can be set to any value described in @ref TransactionMatchFinder::findMatch().
   * @ref MatchDuplicate is set if the imported transaction has the same bank id as the existing transaction;
   * @ref MatchImprecise is set when transaction dates are not equal, but within matchWindow range
   */
  void findMatchInMatchCandidatesList() final override;
};

#endif // EXISTINGTRANSACTIONMATCHFINDER_H
