/***************************************************************************
    KMyMoney transaction importing module - searches for a matching scheduled transaction

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


#ifndef SCHEDULEDTRANSACTIONMATCHFINDER_H
#define SCHEDULEDTRANSACTIONMATCHFINDER_H

#include "transactionmatchfinder.h"
#include "mymoneyaccount.h"

/** Implements searching for a matching transaction in the scheduled transactions
 */
class ScheduledTransactionMatchFinder : public TransactionMatchFinder
{
public:
  /** Ctor, initializes the match finder
   * @param account the account for which the scheduled transactions shall be considered
   * @param matchWindow max number of days the transactions may vary and still be considered to be matching
   */
  ScheduledTransactionMatchFinder(const MyMoneyAccount& account, int matchWindow);

private:
  MyMoneyAccount          account;
  QList<MyMoneySchedule>  listOfMatchCandidates;

  /** Fills @ref listOfSchedules member with list of scheduled transactions for the @ref account
   */
  virtual void createListOfMatchCandidates();

  /** Searches for a matching transaction in the scheduled transactions
   *
   * The search result can be set to any value except @ref MatchDuplicate as described in
   * @ref TransactionMatchFinder::findMatch().
   * @ref MatchImprecise is returned when transaction dates are not equal, but within matchWindow range
   */
  virtual void findMatchInMatchCandidatesList();
};

#endif // SCHEDULEDTRANSACTIONMATCHFINDER_H
