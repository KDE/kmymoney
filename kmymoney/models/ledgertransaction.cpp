/*
    SPDX-FileCopyrightText: 2016-2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ledgertransaction.h"
#include "ledgertransaction_p.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>
#include <QDate>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneysplit.h"
#include "mymoneytransaction.h"
#include "mymoneymoney.h"
#include "mymoneyenums.h"

using namespace eMyMoney;

LedgerTransaction::LedgerTransaction() :
    LedgerItem(),
    d_ptr(new LedgerTransactionPrivate)
{
}

LedgerTransaction::LedgerTransaction(const MyMoneyTransaction& t, const MyMoneySplit& s) :
    LedgerItem(),
    d_ptr(new LedgerTransactionPrivate)
{
    Q_D(LedgerTransaction);
    d->init(t,s);
}

LedgerTransaction::LedgerTransaction(LedgerTransactionPrivate &dd) :
    d_ptr(&dd)
{
}

LedgerTransaction::LedgerTransaction(LedgerTransactionPrivate &dd, const MyMoneyTransaction& t, const MyMoneySplit& s) :
    d_ptr(&dd)
{
    Q_D(LedgerTransaction);
    d->init(t,s);
}

LedgerTransaction::LedgerTransaction(const LedgerTransaction& other) :
    d_ptr(new LedgerTransactionPrivate(*other.d_func()))
{
}

LedgerTransaction::~LedgerTransaction()
{
    delete d_ptr;
}

QDate LedgerTransaction::postDate() const
{
    Q_D(const LedgerTransaction);
    return d->m_transaction.postDate();
}

MyMoneyTransaction LedgerTransaction::transaction() const
{
    Q_D(const LedgerTransaction);
    return d->m_transaction;
}

const MyMoneySplit& LedgerTransaction::split() const
{
    Q_D(const LedgerTransaction);
    return d->m_split;
}

QString LedgerTransaction::accountId() const
{
    Q_D(const LedgerTransaction);
    return d->m_split.accountId();
}

QString LedgerTransaction::account() const
{
    Q_D(const LedgerTransaction);
    return d->m_account;
}

QString LedgerTransaction::counterAccountId() const
{
    Q_D(const LedgerTransaction);
    return d->m_counterAccountId;
}

QString LedgerTransaction::counterAccount() const
{
    Q_D(const LedgerTransaction);
    return d->m_counterAccount;
}

QString LedgerTransaction::costCenterId() const
{
    Q_D(const LedgerTransaction);
    return d->m_costCenterId;
}

QString LedgerTransaction::payeeName() const
{
    Q_D(const LedgerTransaction);
    return d->m_payeeName;
}

QString LedgerTransaction::payeeId() const
{
    Q_D(const LedgerTransaction);
    return d->m_payeeId;
}

QString LedgerTransaction::transactionNumber() const
{
    Q_D(const LedgerTransaction);
    return d->m_split.number();
}

Qt::ItemFlags LedgerTransaction::flags() const
{
    return (Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
}

QString LedgerTransaction::transactionSplitId() const
{
    Q_D(const LedgerTransaction);
    QString rc;
    if(!d->m_transaction.id().isEmpty()) {
        rc = QString("%1-%2").arg(d->m_transaction.id(), d->m_split.id());
    }
    return rc;
}

int LedgerTransaction::splitCount() const
{
    Q_D(const LedgerTransaction);
    return d->m_transaction.splitCount();
}

QString LedgerTransaction::transactionId() const
{
    Q_D(const LedgerTransaction);
    return d->m_transaction.id();
}

eMyMoney::Split::State LedgerTransaction::reconciliationState() const
{
    Q_D(const LedgerTransaction);
    return d->m_split.reconcileFlag();
}

QString LedgerTransaction::reconciliationStateShort() const
{
    Q_D(const LedgerTransaction);
    QString rc;
    switch(d->m_split.reconcileFlag()) {
    case eMyMoney::Split::State::NotReconciled:
    default:
        break;
    case eMyMoney::Split::State::Cleared:
        rc = i18nc("Reconciliation flag C", "C");
        break;
    case eMyMoney::Split::State::Reconciled:
        rc = i18nc("Reconciliation flag R", "R");
        break;
    case eMyMoney::Split::State::Frozen:
        rc = i18nc("Reconciliation flag F", "F");
        break;
    }
    return rc;
}

QString LedgerTransaction::reconciliationStateLong() const
{
    Q_D(const LedgerTransaction);
    QString rc;
    switch(d->m_split.reconcileFlag()) {
    case eMyMoney::Split::State::NotReconciled:
    default:
        rc = i18nc("Reconciliation flag empty", "Not reconciled");
        break;
    case eMyMoney::Split::State::Cleared:
        rc = i18nc("Reconciliation flag C", "Cleared");
        break;
    case eMyMoney::Split::State::Reconciled:
        rc = i18nc("Reconciliation flag R", "Reconciled");
        break;
    case eMyMoney::Split::State::Frozen:
        rc = i18nc("Reconciliation flag F", "Frozen");
        break;
    }
    return rc;
}

QString LedgerTransaction::payment() const
{
    Q_D(const LedgerTransaction);
    return d->m_payment;
}

QString LedgerTransaction::deposit() const
{
    Q_D(const LedgerTransaction);
    return d->m_deposit;
}

void LedgerTransaction::setBalance(QString txt)
{
    Q_D(LedgerTransaction);
    d->m_balance = txt;
}

QString LedgerTransaction::balance() const
{
    Q_D(const LedgerTransaction);
    return d->m_balance;
}

MyMoneyMoney LedgerTransaction::shares() const
{
    Q_D(const LedgerTransaction);
    return d->m_split.shares();
}

QString LedgerTransaction::sharesAmount() const
{
    Q_D(const LedgerTransaction);
    return d->m_shares;
}

QString LedgerTransaction::signedSharesAmount() const
{
    Q_D(const LedgerTransaction);
    return d->m_signedShares;
}

QString LedgerTransaction::sharesSuffix() const
{
    Q_D(const LedgerTransaction);
    return d->m_sharesSuffix;
}

MyMoneyMoney LedgerTransaction::value() const
{
    Q_D(const LedgerTransaction);
    return d->m_split.value();
}

QString LedgerTransaction::memo() const
{
    Q_D(const LedgerTransaction);
    auto memo = d->m_split.memo();
    if(memo.isEmpty()) {
        memo = d->m_transaction.memo();
    }
    return memo;
}

bool LedgerTransaction::isErroneous() const
{
    Q_D(const LedgerTransaction);
    return d->m_erroneous;
}

bool LedgerTransaction::isImported() const
{
    Q_D(const LedgerTransaction);
    return d->m_transaction.isImported();
}

LedgerTransaction LedgerTransaction::newTransactionEntry()
{
    // create a dummy entry for new transactions
    MyMoneyTransaction t;
    t.setPostDate(QDate(2900,12,31));
    return LedgerTransaction(t, MyMoneySplit());
}

bool LedgerTransaction::isNewTransactionEntry() const
{
    Q_D(const LedgerTransaction);
    return d->m_transaction.id().isEmpty() && d->m_split.id().isEmpty();
}

QString LedgerTransaction::transactionCommodity() const
{
    Q_D(const LedgerTransaction);
    return d->m_transaction.commodity();
}
