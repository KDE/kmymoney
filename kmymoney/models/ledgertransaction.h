/*
    SPDX-FileCopyrightText: 2016-2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef LEDGERTRANSACTION_H
#define LEDGERTRANSACTION_H

// ----------------------------------------------------------------------------
// QT Includes

//#include <QScopedPointer>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ledgeritem.h"

class MyMoneyMoney;
class MyMoneySplit;
class MyMoneyTransaction;

class LedgerTransactionPrivate;
class LedgerTransaction : public LedgerItem
{
public:
    explicit LedgerTransaction(const MyMoneyTransaction& t, const MyMoneySplit& s);
    LedgerTransaction(const LedgerTransaction & other);
    LedgerTransaction(LedgerTransaction && other);
    LedgerTransaction & operator=(LedgerTransaction other);
    friend void swap(LedgerTransaction& first, LedgerTransaction& second);

    virtual ~LedgerTransaction();

    static LedgerTransaction newTransactionEntry();

    /// @copydoc LedgerItem::postDate()
    QDate postDate() const override;

    /// @copydoc LedgerItem::transaction()
    MyMoneyTransaction transaction() const override;

    /// @copydoc LedgerItem::split()
    const MyMoneySplit& split() const override;

    /// @copydoc LedgerItem::accountId()
    QString accountId() const override;

    /// @copydoc LedgerItem::account()
    QString account() const override;

    /// @copydoc LedgerItem::counterAccountId()
    QString counterAccountId() const override;

    /// @copydoc LedgerItem::counterAccount()
    QString counterAccount() const override;

    /// @copydoc LedgerItem::costCenterId()
    QString costCenterId() const override;

    /// @copydoc LedgerItem::payeeName()
    QString payeeName() const override;

    /// @copydoc LedgerItem::payeeId()
    QString payeeId() const override;

    /// @copydoc LedgerItem::transactionNumber()
    QString transactionNumber() const override;

    /// @copydoc LedgerItem::flags()
    Qt::ItemFlags flags() const override;

    /// @copydoc LedgerItem::transactionSplitId()
    QString transactionSplitId() const override;

    /// @copydoc LedgerItem::splitCount()
    int splitCount() const override;

    /// @copydoc LedgerItem::transactionId()
    QString transactionId() const override;

    /// @copydoc LedgerItem::reconciliationState()
    eMyMoney::Split::State reconciliationState() const override;

    /// @copydoc LedgerItem::reconciliationStateShort()
    QString reconciliationStateShort() const override;

    /// @copydoc LedgerItem::reconciliationStateShort()
    QString reconciliationStateLong() const override;

    /// @copydoc LedgerItem::payment()
    QString payment() const override;

    /// @copydoc LedgerItem::deposit()
    QString deposit() const override;

    /// @copydoc LedgerItem::setBalance()
    void setBalance(QString txt) override;

    /// @copydoc LedgerItem::balance()
    QString balance() const override;

    /// @copydoc LedgerItem::shares()
    MyMoneyMoney shares() const override;

    /// @copydoc LedgerItem::sharesAmount()
    QString sharesAmount() const override;

    /// @copydoc LedgerItem::signedSharesAmount()
    QString signedSharesAmount() const override;

    /// @copydoc LedgerItem::sharesSuffix()
    QString sharesSuffix() const override;

    /// @copydoc LedgerItem::value()
    MyMoneyMoney value() const override;

    /// @copydoc LedgerItem::memo()
    QString memo() const override;

    /// @copydoc LedgerItem::isErroneous()
    bool isErroneous() const override;

    /// @copydoc LedgerItem::isImported()
    bool isImported() const override;

    /// @copydoc LedgerItem::isNewTransactionEntry()
    bool isNewTransactionEntry() const override;

    /// @copydoc LedgerItem::transactionCommodity()
    QString transactionCommodity() const override;

protected:
    LedgerTransactionPrivate *d_ptr;
    LedgerTransaction(LedgerTransactionPrivate &dd);
    LedgerTransaction(LedgerTransactionPrivate &dd,
                      const MyMoneyTransaction &t,
                      const MyMoneySplit &s);

private:
    LedgerTransaction();
    Q_DECLARE_PRIVATE(LedgerTransaction)
};

inline void swap(LedgerTransaction& first, LedgerTransaction& second) // krazy:exclude=inline
{
    using std::swap;
    swap(first.d_ptr, second.d_ptr);
}

inline LedgerTransaction::LedgerTransaction(LedgerTransaction && other) : LedgerTransaction() // krazy:exclude=inline
{
    swap(*this, other);
}

inline LedgerTransaction & LedgerTransaction::operator=(LedgerTransaction other) // krazy:exclude=inline
{
    swap(*this, other);
    return *this;
}

#endif // LEDGERTRANSACTION_H

