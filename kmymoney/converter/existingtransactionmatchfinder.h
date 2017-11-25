/***************************************************************************
    KMyMoney transaction importing module - searches for a matching transaction in the ledger

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
  ExistingTransactionMatchFinder(int matchWindow = 3);

protected:
  typedef QPair<MyMoneyTransaction, MyMoneySplit> TransactionAndSplitPair;
  QList<TransactionAndSplitPair> listOfMatchCandidates;

  /** Creates a list of transactions within matchWindow range and with the same amount as the imported transaction we're trying to match
   */
  virtual void createListOfMatchCandidates();

  /** Searches for a matching transaction in the ledger
   *
   * Match result can be set to any value described in @ref TransactionMatchFinder::findMatch().
   * @ref MatchDuplicate is set if the imported transaction has the same bank id as the existing transaction;
   * @ref MatchImprecise is set when transaction dates are not equal, but within matchWindow range
   */
  virtual void findMatchInMatchCandidatesList();
};

#endif // EXISTINGTRANSACTIONMATCHFINDER_H
