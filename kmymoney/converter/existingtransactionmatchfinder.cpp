/*
    KMyMoney transaction importing module - searches for a matching transaction in the ledger

    SPDX-FileCopyrightText: 2012 Lukasz Maszczynski <lukasz@maszczynski.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
    MyMoneyTransactionFilter filter(m_importedSplit.accountId());
    filter.setReportAllSplits(false);
    filter.setDateFilter(importedTransaction.postDate().addDays(-m_matchWindow), importedTransaction.postDate().addDays(m_matchWindow));
    filter.setAmountFilter(m_importedSplit.shares(), m_importedSplit.shares());

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
