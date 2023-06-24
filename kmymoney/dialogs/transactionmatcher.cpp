/*
    SPDX-FileCopyrightText: 2008-2015 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2015 Christian Dávid <christian-david@web.de>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "transactionmatcher.h"

#include <QDate>

#include <KLocalizedString>

#include "mymoneyaccount.h"
#include "mymoneymoney.h"
#include "mymoneysecurity.h"
#include "mymoneysplit.h"
#include "mymoneytransaction.h"
#include "mymoneyutils.h"
#include "mymoneyfile.h"
#include "mymoneyexception.h"
#include "mymoneyenums.h"

class TransactionMatcherPrivate
{
    Q_DISABLE_COPY(TransactionMatcherPrivate)

public:
    TransactionMatcherPrivate()
    {
    }
};

TransactionMatcher::TransactionMatcher()
    : d_ptr(new TransactionMatcherPrivate)
{
}

TransactionMatcher::~TransactionMatcher()
{
    Q_D(TransactionMatcher);
    delete d;
}

void TransactionMatcher::match(MyMoneyTransaction tm, MyMoneySplit sm, MyMoneyTransaction ti, MyMoneySplit si, bool allowImportedTransactions)
{
    if (sm.accountId() != si.accountId()) {
        throw MYMONEYEXCEPTION_CSTRING("Both splits must reference the same account for matching");
    }

    const auto file = MyMoneyFile::instance();
    auto account = file->account(sm.accountId());
    auto sec = file->security(account.currencyId());

    // Now match the transactions.
    //
    // 'Matching' the transactions entails DELETING the end transaction,
    // and MODIFYING the start transaction as needed.
    //
    // There are a variety of ways that a transaction can conflict.
    // Post date, splits, amount are the ones that seem to matter.
    // TODO: Handle these conflicts intelligently, at least warning
    // the user, or better yet letting the user choose which to use.
    //
    // For now, we will just use the transaction details from the start
    // transaction.  The only thing we'll take from the end transaction
    // are the bank ID's.
    //
    // What we have to do here is iterate over the splits in the end
    // transaction, and find the corresponding split in the start
    // transaction.  If there is a bankID in the end split but not the
    // start split, add it to the start split.  If there is a bankID
    // in BOTH, then this transaction cannot be merged (both transactions
    // were imported!!)  If the corresponding start split cannot  be
    // found and the end split has a bankID, we should probably just fail.
    // Although we could ADD it to the transaction.

    // ipwizard: Don't know if iterating over the transactions is a good idea.
    // In case of a split transaction recorded with KMyMoney and the transaction
    // data being imported consisting only of a single category assignment, this
    // does not make much sense. The same applies for investment transactions
    // stored in KMyMoney against imported transactions. I think a better solution
    // is to just base the match on the splits referencing the same (currently
    // selected) account.

    // verify, that tm is a manual (non-matched) transaction
    // allow matching two manual transactions

    if ((!allowImportedTransactions && tm.isImported()) || sm.isMatched())
        throw MYMONEYEXCEPTION_CSTRING("First transaction does not match requirement for matching");

    // verify that the amounts are the same, otherwise we should not be matching!
    if (sm.shares() != si.shares()) {
        throw MYMONEYEXCEPTION(
            QString::fromLatin1("Splits for %1 have conflicting values (%2,%3)")
                .arg(account.name(), MyMoneyUtils::formatMoney(sm.shares(), account, sec), MyMoneyUtils::formatMoney(si.shares(), account, sec)));
    }

    // ipwizard: I took over the code to keep the bank id found in the endMatchTransaction
    // This might not work for QIF imports as they don't setup this information. It sure
    // makes sense for OFX and HBCI.
    const QString& bankID = si.bankID();
    if (!bankID.isEmpty()) {
        try {
            if (sm.bankID().isEmpty()) {
                sm.setBankID(bankID);
                tm.modifySplit(sm);
            }
        } catch (const MyMoneyException &e) {
            throw MYMONEYEXCEPTION(QString::fromLatin1("Unable to match all splits (%1)").arg(e.what()));
        }
    }
    //
    //  we now allow matching of two non-imported transactions
    //

    // mark the split as cleared if it does not have a reconciliation information yet
    if (sm.reconcileFlag() == eMyMoney::Split::State::NotReconciled) {
        sm.setReconcileFlag(eMyMoney::Split::State::Cleared);
    }

    // if we don't have a payee assigned to the manually entered transaction
    // we use the one we found in the imported transaction
    if (sm.payeeId().isEmpty() && !si.payeeId().isEmpty()) {
        sm.setValue("kmm-orig-payee", sm.payeeId());
        sm.setPayeeId(si.payeeId());
    }

    // We use the imported postdate and keep the previous one for unmatch
    if (tm.postDate() != ti.postDate()) {
        sm.setValue("kmm-orig-postdate", tm.postDate().toString(Qt::ISODate));
        tm.setPostDate(ti.postDate());
    }

    // combine the two memos into one
    QString memo = sm.memo();
    if (!si.memo().isEmpty() && si.memo() != memo) {
        sm.setValue("kmm-orig-memo", memo);
        if (!memo.isEmpty())
            memo += '\n';
        memo += si.memo();
    }
    sm.setMemo(memo);

    // remember the split we matched
    sm.setValue("kmm-match-split", si.id());

    sm.addMatch(ti);
    tm.modifySplit(sm);

    ti.modifySplit(si);///

    if (file->isInvestmentTransaction(tm)) {
        // find the security split and set the memo to the same as sm.memo
        const auto splits = tm.splits();
        for (auto split : splits) {
            const auto acc = file->account(split.accountId());
            const auto security = file->security(acc.currencyId());
            if (!security.isCurrency()) {
                split.setMemo(memo);
                tm.modifySplit(split);
                break;
            }
        }
    }

    file->modifyTransaction(tm);
    // Delete the end transaction if it was stored in the engine
    if (!ti.id().isEmpty())
        file->removeTransaction(ti);
}

void TransactionMatcher::unmatch(const MyMoneyTransaction& _t, const MyMoneySplit& _s)
{
    if (_s.isMatched()) {
        MyMoneyTransaction tm(_t);
        MyMoneySplit sm(_s);
        MyMoneyTransaction ti(sm.matchedTransaction());
        MyMoneySplit si;
        // if we don't have a split, then we don't have a memo
        try {
            si = ti.splitById(sm.value("kmm-match-split"));
        } catch (const MyMoneyException &) {
        }
        sm.removeMatch();

        // restore the postdate if modified
        if (!sm.value("kmm-orig-postdate").isEmpty()) {
            tm.setPostDate(QDate::fromString(sm.value("kmm-orig-postdate"), Qt::ISODate));
        }

        // restore payee if modified
        if (!sm.value("kmm-orig-payee").isEmpty()) {
            sm.setPayeeId(sm.value("kmm-orig-payee"));
        }

        // restore memo if modified
        if (!sm.value("kmm-orig-memo").isEmpty()) {
            sm.setMemo(sm.value("kmm-orig-memo"));
        }

        sm.deletePair("kmm-orig-postdate");
        sm.deletePair("kmm-orig-payee");
        sm.deletePair("kmm-orig-memo");
        sm.deletePair("kmm-match-split");
        sm.setBankID(QString());
        tm.modifySplit(sm);

        MyMoneyFile::instance()->modifyTransaction(tm);
        MyMoneyFile::instance()->addTransaction(ti);
    }
}

void TransactionMatcher::accept(const MyMoneyTransaction& _t, const MyMoneySplit& _s)
{
    if (_s.isMatched()) {
        MyMoneyTransaction tm(_t);
        MyMoneySplit sm(_s);
        sm.removeMatch();
        sm.deletePair("kmm-orig-postdate");
        sm.deletePair("kmm-orig-payee");
        sm.deletePair("kmm-orig-memo");
        sm.deletePair("kmm-match-split");
        tm.modifySplit(sm);

        MyMoneyFile::instance()->modifyTransaction(tm);
    }
}
