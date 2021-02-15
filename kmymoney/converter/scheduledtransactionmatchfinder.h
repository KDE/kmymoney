/*
    KMyMoney transaction importing module - searches for a matching scheduled transaction

    SPDX-FileCopyrightText: 2012 Lukasz Maszczynski <lukasz@maszczynski.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SCHEDULEDTRANSACTIONMATCHFINDER_H
#define SCHEDULEDTRANSACTIONMATCHFINDER_H

#include <QList>

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
  ScheduledTransactionMatchFinder(const MyMoneyAccount& m_account, int m_matchWindow);

private:
  MyMoneyAccount          m_account;
  QList<MyMoneySchedule>  listOfMatchCandidates;

  /** Fills @ref listOfSchedules member with list of scheduled transactions for the @ref account
   */
  void createListOfMatchCandidates() final override;

  /** Searches for a matching transaction in the scheduled transactions
   *
   * The search result can be set to any value except @ref MatchDuplicate as described in
   * @ref TransactionMatchFinder::findMatch().
   * @ref MatchImprecise is returned when transaction dates are not equal, but within matchWindow range
   */
  void findMatchInMatchCandidatesList() final override;
};

#endif // SCHEDULEDTRANSACTIONMATCHFINDER_H
