/*
    KMyMoney transaction importing module - base class for searching for a matching transaction

    SPDX-FileCopyrightText: 2012 Lukasz Maszczynski <lukasz@maszczynski.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "transactionmatchfinder.h"

#include <QDebug>
#include <QDate>

#include <KLocalizedString>

#include "mymoneymoney.h"
#include "mymoneyaccount.h"
#include "mymoneyfile.h"
#include "mymoneypayee.h"
#include "mymoneyexception.h"

TransactionMatchFinder::TransactionMatchFinder(int _matchWindow) :
    m_matchWindow(_matchWindow),
    matchResult(MatchNotFound)
{
}

TransactionMatchFinder::~TransactionMatchFinder()
{
}

TransactionMatchFinder::MatchResult TransactionMatchFinder::findMatch(const MyMoneyTransaction& transactionToMatch, const MyMoneySplit& splitToMatch)
{
    importedTransaction = transactionToMatch;
    m_importedSplit = splitToMatch;
    matchResult = MatchNotFound;
    matchedTransaction.reset();
    matchedSplit.reset();
    matchedSchedule.reset();

    QString date = importedTransaction.postDate().toString(Qt::ISODate);
    QString payeeName = MyMoneyFile::instance()->payee(m_importedSplit.payeeId()).name();
    QString amount = m_importedSplit.shares().formatMoney("", 2);
    QString account = MyMoneyFile::instance()->account(m_importedSplit.accountId()).name();
    qDebug() << "Looking for a match with transaction: " << date << "," << payeeName << "," << amount
             << "(referenced account: " << account << ")";

    createListOfMatchCandidates();
    findMatchInMatchCandidatesList();
    return matchResult;
}

MyMoneySplit TransactionMatchFinder::getMatchedSplit() const
{
    if (matchedSplit.isNull()) {
        throw MYMONEYEXCEPTION(QString::fromLatin1("Internal error - no matching splits"));
    }

    return *matchedSplit;
}

MyMoneyTransaction TransactionMatchFinder::getMatchedTransaction() const
{
    if (matchedTransaction.isNull()) {
        throw MYMONEYEXCEPTION(QString::fromLatin1("Internal error - no matching transactions"));
    }

    return *matchedTransaction;
}

MyMoneySchedule TransactionMatchFinder::getMatchedSchedule() const
{
    if (matchedSchedule.isNull()) {
        throw MYMONEYEXCEPTION(QString::fromLatin1("Internal error - no matching schedules"));
    }

    return *matchedSchedule;
}

bool TransactionMatchFinder::splitsAreDuplicates(const MyMoneySplit& split1, const MyMoneySplit& split2, int amountVariation) const
{
    return (splitsAmountsMatch(split1, split2, amountVariation) && splitsBankIdsDuplicated(split1, split2));
}

bool TransactionMatchFinder::splitsMatch(const MyMoneySplit& importedSplit, const MyMoneySplit& existingSplit, int amountVariation) const
{
    return (splitsAccountsMatch(importedSplit, existingSplit) //
            && splitsBankIdsMatch(importedSplit, existingSplit) //
            && splitsAmountsMatch(importedSplit, existingSplit, amountVariation)  //
            && splitsPayeesMatchOrEmpty(importedSplit, existingSplit) //
            && !existingSplit.isMatched());
}

bool TransactionMatchFinder::splitsAccountsMatch(const MyMoneySplit & split1, const MyMoneySplit & split2) const
{
    return split1.accountId() == split2.accountId();
}

bool TransactionMatchFinder::splitsAmountsMatch(const MyMoneySplit& split1, const MyMoneySplit& split2, int amountVariation) const
{
    MyMoneyMoney upper(split1.shares());
    MyMoneyMoney lower(upper);
    if ((amountVariation > 0) && (amountVariation < 100)) {
        lower = lower - (lower.abs() * MyMoneyMoney(amountVariation, 100));
        upper = upper + (upper.abs() * MyMoneyMoney(amountVariation, 100));
    }

    return (split2.shares() >= lower) && (split2.shares() <= upper);
}

bool TransactionMatchFinder::splitsBankIdsDuplicated(const MyMoneySplit& split1, const MyMoneySplit& split2) const
{
    return (!split1.bankID().isEmpty()) && (split1.bankID() == split2.bankID());
}

bool TransactionMatchFinder::splitsBankIdsMatch(const MyMoneySplit& importedSplit, const MyMoneySplit& existingSplit) const
{
    return (existingSplit.bankID().isEmpty() || existingSplit.bankID() == importedSplit.bankID());
}

bool TransactionMatchFinder::splitsPayeesMatchOrEmpty(const MyMoneySplit& split1, const MyMoneySplit& split2) const
{
    bool payeesMatch = (split1.payeeId() == split2.payeeId());
    bool atLeastOnePayeeIsNotSet = (split1.payeeId().isEmpty() || split2.payeeId().isEmpty());
    return payeesMatch || atLeastOnePayeeIsNotSet;
}

void TransactionMatchFinder::findMatchingSplit(const MyMoneyTransaction& transaction, int amountVariation)
{
    foreach (const MyMoneySplit & split, transaction.splits()) {
        if (splitsAreDuplicates(m_importedSplit, split, amountVariation)) {
            matchedTransaction.reset(new MyMoneyTransaction(transaction));
            matchedSplit.reset(new MyMoneySplit(split));
            matchResult = MatchDuplicate;
            break;
        }

        if (splitsMatch(m_importedSplit, split, amountVariation)) {
            matchedTransaction.reset(new MyMoneyTransaction(transaction));
            matchedSplit.reset(new MyMoneySplit(split));

            bool datesMatchPrecisely = importedTransaction.postDate() == transaction.postDate();
            if (datesMatchPrecisely) {
                matchResult = MatchPrecise;
            } else {
                matchResult = MatchImprecise;
            }
            break;
        }
    }
}
