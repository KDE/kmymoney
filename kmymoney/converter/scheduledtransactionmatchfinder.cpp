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
  foreach (const MyMoneySchedule & schedule, listOfMatchCandidates) {
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
