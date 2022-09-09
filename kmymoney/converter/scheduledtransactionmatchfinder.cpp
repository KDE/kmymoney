/*
    KMyMoney transaction importing module - searches for a matching scheduled transaction

    SPDX-FileCopyrightText: 2012 Lukasz Maszczynski <lukasz@maszczynski.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "scheduledtransactionmatchfinder.h"

#include <QDebug>
#include <QDate>

#include "mymoneyfile.h"
#include "mymoneyschedule.h"
#include "kmymoneyutils.h"

ScheduledTransactionMatchFinder::ScheduledTransactionMatchFinder(const MyMoneyAccount& account, int matchWindow)
    : TransactionMatchFinder(matchWindow), m_account(account)
{
}

void ScheduledTransactionMatchFinder::createListOfMatchCandidates()
{
    listOfMatchCandidates = MyMoneyFile::instance()->scheduleList(m_account.id());
    qDebug() << "Considering" << listOfMatchCandidates.size() << "schedule(s) for matching the transaction";
}

void ScheduledTransactionMatchFinder::findMatchInMatchCandidatesList()
{
    Q_FOREACH (const MyMoneySchedule & schedule, listOfMatchCandidates) {
        QDate nextDueDate = schedule.nextDueDate();
        bool nextDueDateWithinMatchWindowRange = (nextDueDate >= importedTransaction.postDate().addDays(-m_matchWindow))
                && (nextDueDate <= importedTransaction.postDate().addDays(m_matchWindow));
        if (schedule.isOverdue() || nextDueDateWithinMatchWindowRange) {
            MyMoneyTransaction scheduledTransaction = KMyMoneyUtils::scheduledTransaction(schedule);

            findMatchingSplit(scheduledTransaction, schedule.variation());
            if (matchResult != MatchNotFound) {
                matchedSchedule.reset(new MyMoneySchedule(schedule));
                return;
            }
        }
    }
}
