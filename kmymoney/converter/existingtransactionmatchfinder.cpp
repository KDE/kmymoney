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

#include "existingtransactionmatchfinder.h"

#include <QDebug>
#include <QDate>

#include "mymoneymoney.h"
#include "mymoneyfile.h"
#include "mymoneytransactionfilter.h"

ExistingTransactionMatchFinder::ExistingTransactionMatchFinder(int matchWindow)
    : TransactionMatchFinder(matchWindow)
{
}

void ExistingTransactionMatchFinder::createListOfMatchCandidates()
{
  MyMoneyTransactionFilter filter(importedSplit.accountId());
  filter.setReportAllSplits(false);
  filter.setDateFilter(importedTransaction.postDate().addDays(-matchWindow), importedTransaction.postDate().addDays(matchWindow));
  filter.setAmountFilter(importedSplit.shares(), importedSplit.shares());

  MyMoneyFile::instance()->transactionList(listOfMatchCandidates, filter);
  qDebug() << "Considering" << listOfMatchCandidates.size() << "existing transaction(s) for matching";
}

void ExistingTransactionMatchFinder::findMatchInMatchCandidatesList()
{
  foreach (const TransactionAndSplitPair & transactionAndSplit, listOfMatchCandidates) {
    const MyMoneyTransaction & theTransaction = transactionAndSplit.first;

    if (theTransaction.id() == importedTransaction.id()) {
      // just skip myself
      continue;
    }

    findMatchingSplit(theTransaction, 0);

    if (matchResult != MatchNotFound) {
      return;
    }
  }
}
