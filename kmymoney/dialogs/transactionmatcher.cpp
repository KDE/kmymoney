/*
    SPDX-FileCopyrightText: 2008-2015 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2015 Christian Dávid <christian-david@web.de>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "transactionmatcher.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDate>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyaccount.h"
#include "mymoneyenums.h"
#include "mymoneyexception.h"
#include "mymoneyfile.h"
#include "mymoneymoney.h"
#include "mymoneysecurity.h"
#include "mymoneysplit.h"
#include "mymoneytransaction.h"
#include "mymoneyutils.h"

class TransactionMatcherPrivate
{
    Q_DISABLE_COPY(TransactionMatcherPrivate)

public:
    TransactionMatcherPrivate()
    {
    }

    void clearMatchFlags(MyMoneySplit& s)
    {
        s.deletePair("kmm-orig-postdate");
        s.deletePair("kmm-orig-payee");
        s.deletePair("kmm-orig-memo");
        s.deletePair("kmm-orig-onesplit");
        s.deletePair("kmm-orig-not-reconciled");
        s.deletePair("kmm-match-split");
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
    // 'Matching' the transactions entails DELETING the imported transaction (ti),
    // and MODIFYING the manual entered transaction (tm) as needed.
    //
    // There are a variety of ways that a transaction can conflict.
    // Post date, splits, amount are the ones that seem to matter.
    // TODO: Handle these conflicts intelligently, at least warning
    // the user, or better yet letting the user choose which to use.
    //
    // If the imported split (si) contains a bankID but none is in sm
    // use the one found in si and add it to sm.
    // If there is a bankID in BOTH, then this transaction
    // cannot be merged (both transactions were imported!!)
    //
    // If the postdate of si differs from the one in sm, we use the one in si
    // if ti has the imported flag set.

    // verify, that tm is a manual (non-matched) transaction
    // allow matching two manual transactions

    if ((!allowImportedTransactions && tm.isImported()) || sm.isMatched() || si.isMatched())
        throw MYMONEYEXCEPTION_CSTRING("Transactions does not fullfil requirements for matching");

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
        sm.MyMoneyKeyValueContainer::setValue("kmm-orig-not-reconciled", true, false);
        sm.setReconcileFlag(eMyMoney::Split::State::Cleared);
    }

    // if we don't have a payee assigned to the manually entered transaction
    // we use the one we found in the imported transaction
    if (sm.payeeId().isEmpty() && !si.payeeId().isEmpty()) {
        // we use "\xff\xff\xff\xff" as the default so that
        // the entry will be written, since payeeId is empty
        sm.setValue("kmm-orig-payee", sm.payeeId(), QLatin1String("\xff\xff\xff\xff"));
        sm.setPayeeId(si.payeeId());
    }

    // We use the imported postdate and keep the previous one for unmatch
    if ((tm.postDate() != ti.postDate()) && ti.isImported()) {
        sm.setValue("kmm-orig-postdate", tm.postDate().toString(Qt::ISODate));
        tm.setPostDate(ti.postDate());
    }

    // combine the two memos into one if they differ
    QString memo = sm.memo();
    if (si.memo() != memo) {
        // we use "\xff\xff\xff\xff" as the default so that
        // the entry will be written, even if memo is empty
        sm.setValue("kmm-orig-memo", memo, QLatin1String("\xff\xff\xff\xff"));
        if (!memo.isEmpty() && !si.memo().isEmpty())
            memo += '\n';
        memo += si.memo();
    }
    sm.setMemo(memo);

    // if tm has only one split and ti has more than one, then simply
    // copy all splits from ti to tm (skipping si) and remember that
    // tm had no splits.
    if (tm.splitCount() == 1 && ti.splitCount() > 1) {
        sm.MyMoneyKeyValueContainer::setValue("kmm-orig-onesplit", true, false);
        const auto splits = ti.splits();
        for (auto split : splits) {
            if (split.id() == si.id())
                continue;
            split.clearId();
            tm.addSplit(split);
        }
    }

    // remember the split we matched
    sm.setValue("kmm-match-split", si.id());

    sm.addMatch(ti);
    tm.modifySplit(sm);

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

    // Delete the imported transaction if it was previously stored in the engine
    if (!ti.id().isEmpty())
        file->removeTransaction(ti);
}

void TransactionMatcher::unmatch(const MyMoneyTransaction& _t, const MyMoneySplit& _s)
{
    Q_D(TransactionMatcher);
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
        if (sm.pairs().contains("kmm-orig-payee")) {
            sm.setPayeeId(sm.value("kmm-orig-payee"));
        }

        // restore memo if modified
        if (sm.pairs().contains("kmm-orig-memo")) {
            sm.setMemo(sm.value("kmm-orig-memo"));
        }

        // restore reconcileFlag if modified
        if (sm.MyMoneyKeyValueContainer::value("kmm-orig-not-reconciled", false)) {
            sm.setReconcileFlag(eMyMoney::Split::State::NotReconciled);
            sm.setReconcileDate(QDate());
        }

        // remove splits if they were added during matching
        if (sm.MyMoneyKeyValueContainer::value("kmm-orig-onesplit", false)) {
            const auto splits = tm.splits();
            for (auto split : splits) {
                if (split.id() == sm.id())
                    continue;
                tm.removeSplit(split);
            }
        }

        d->clearMatchFlags(sm);
        sm.setBankID(QString());
        tm.modifySplit(sm);

        MyMoneyFile::instance()->modifyTransaction(tm);
        MyMoneyFile::instance()->addTransaction(ti);
    }
}

void TransactionMatcher::accept(const MyMoneyTransaction& _t, const MyMoneySplit& _s)
{
    Q_D(TransactionMatcher);
    if (_s.isMatched()) {
        MyMoneyTransaction tm(_t);
        MyMoneySplit sm(_s);
        sm.removeMatch();
        d->clearMatchFlags(sm);
        tm.modifySplit(sm);

        MyMoneyFile::instance()->modifyTransaction(tm);
    }
}
