/*
    SPDX-FileCopyrightText: 2002-2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2004 Kevin Tambascio <ktambascio@users.sourceforge.net>
    SPDX-FileCopyrightText: 2005-2006 Ace Jones <acejones@users.sourceforge.net>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "mymoneysplit.h"
#include "mymoneysplit_p.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes
#include "klocalizedstring.h"

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyutils.h"
#include "mymoneytransaction.h"
#include "mymoneyexception.h"

MyMoneySplit::MyMoneySplit() :
    MyMoneyObject(*new MyMoneySplitPrivate)
{
}

MyMoneySplit::MyMoneySplit(const QString &id) :
    MyMoneyObject(*new MyMoneySplitPrivate, id)
{
    Q_D(MyMoneySplit);
    d->m_reconcileFlag = eMyMoney::Split::State::NotReconciled;
}

MyMoneySplit::MyMoneySplit(const MyMoneySplit& other) :
    MyMoneyObject(*new MyMoneySplitPrivate(*other.d_func()), other.id()),
    MyMoneyKeyValueContainer(other)
{
}

MyMoneySplit::MyMoneySplit(const QString& id, const MyMoneySplit& other) :
    MyMoneyObject(*new MyMoneySplitPrivate(*other.d_func()), id),
    MyMoneyKeyValueContainer(other)
{
}

MyMoneySplit::~MyMoneySplit()
{
}

bool MyMoneySplit::operator == (const MyMoneySplit& right) const
{
    Q_D(const MyMoneySplit);
    auto d2 = static_cast<const MyMoneySplitPrivate *>(right.d_func());
    // clang-format off
    return MyMoneyObject::operator==(right)
        && MyMoneyKeyValueContainer::operator==(right)
        && d->m_account == d2->m_account
        && d->m_costCenter == d2->m_costCenter
        && d->m_payee == d2->m_payee
        && d->m_tagList == d2->m_tagList
        && d->m_memo == d2->m_memo
        && d->m_action == d2->m_action
        && d->m_reconcileDate == d2->m_reconcileDate
        && d->m_reconcileFlag == d2->m_reconcileFlag
        && ((d->m_number.length() == 0 && d2->m_number.length() == 0) || d->m_number == d2->m_number)
        && d->m_shares == d2->m_shares
        && d->m_value == d2->m_value
        && d->m_price == d2->m_price
        && d->m_transactionId == d2->m_transactionId;
    // clang-format on
}

MyMoneySplit MyMoneySplit::operator-() const
{
    MyMoneySplit rc(*this);
    rc.d_func()->m_shares = -rc.d_func()->m_shares;
    rc.d_func()->m_value = -rc.d_func()->m_value;
    return rc;
}

QString MyMoneySplit::accountId() const
{
    Q_D(const MyMoneySplit);
    return d->m_account;
}

void MyMoneySplit::setAccountId(const QString& account)
{
    Q_D(MyMoneySplit);
    d->m_account = account;
    d->clearReferences();
}

QString MyMoneySplit::costCenterId() const
{
    Q_D(const MyMoneySplit);
    return d->m_costCenter;
}

void MyMoneySplit::setCostCenterId(const QString& costCenter)
{
    Q_D(MyMoneySplit);
    d->m_costCenter = costCenter;
    d->clearReferences();
}

QString MyMoneySplit::memo() const
{
    Q_D(const MyMoneySplit);
    return d->m_memo;
}

void MyMoneySplit::setMemo(const QString& memo)
{
    Q_D(MyMoneySplit);
    d->m_memo = memo;
}

eMyMoney::Split::State MyMoneySplit::reconcileFlag() const
{
    Q_D(const MyMoneySplit);
    return d->m_reconcileFlag;
}

QDate MyMoneySplit::reconcileDate() const
{
    Q_D(const MyMoneySplit);
    return d->m_reconcileDate;
}

void MyMoneySplit::setReconcileDate(const QDate& date)
{
    Q_D(MyMoneySplit);
    d->m_reconcileDate = date;
}

void MyMoneySplit::setReconcileFlag(const eMyMoney::Split::State flag)
{
    Q_D(MyMoneySplit);
    d->m_reconcileFlag = flag;
}

MyMoneyMoney MyMoneySplit::shares() const
{
    Q_D(const MyMoneySplit);
    return d->m_shares;
}

void MyMoneySplit::setShares(const MyMoneyMoney& shares)
{
    Q_D(MyMoneySplit);
    d->m_shares = shares;
}

void MyMoneySplit::negateShares()
{
    Q_D(MyMoneySplit);
    d->m_shares = -d->m_shares;
}

QString MyMoneySplit::value(const QString& key) const
{
    return MyMoneyKeyValueContainer::value(key);
}

void MyMoneySplit::setValue(const QString& key, const QString& value, const QString& defaultValue)
{
    MyMoneyKeyValueContainer::setValue(key, value, defaultValue);
}

void MyMoneySplit::setValue(const MyMoneyMoney& value)
{
    Q_D(MyMoneySplit);
    d->m_value = value;
}

void MyMoneySplit::setValue(const MyMoneyMoney& value, const QString& transactionCurrencyId, const QString& splitCurrencyId)
{
    if (transactionCurrencyId == splitCurrencyId)
        setValue(value);
    else
        setShares(value);
}

void MyMoneySplit::negateValue()
{
    Q_D(MyMoneySplit);
    d->m_value = -d->m_value;
}

QString MyMoneySplit::payeeId() const
{
    Q_D(const MyMoneySplit);
    return d->m_payee;
}

void MyMoneySplit::setPayeeId(const QString& payee)
{
    Q_D(MyMoneySplit);
    d->m_payee = payee;
    d->clearReferences();
}

QList<QString> MyMoneySplit::tagIdList() const
{
    Q_D(const MyMoneySplit);
    return d->m_tagList;
}

void MyMoneySplit::setTagIdList(const QList<QString>& tagList)
{
    Q_D(MyMoneySplit);
    d->m_tagList = tagList;
    d->clearReferences();
}

void MyMoneySplit::setAction(eMyMoney::Split::InvestmentTransactionType type)
{
    switch (type) {
    case eMyMoney::Split::InvestmentTransactionType::BuyShares:
    case eMyMoney::Split::InvestmentTransactionType::SellShares:
        setAction(actionName(Split::Action::BuyShares));
        break;
    case eMyMoney::Split::InvestmentTransactionType::Dividend:
        setAction(actionName(Split::Action::Dividend));
        break;
    case eMyMoney::Split::InvestmentTransactionType::Yield:
        setAction(actionName(Split::Action::Yield));
        break;
    case eMyMoney::Split::InvestmentTransactionType::ReinvestDividend:
        setAction(actionName(Split::Action::ReinvestDividend));
        break;
    case eMyMoney::Split::InvestmentTransactionType::AddShares:
    case eMyMoney::Split::InvestmentTransactionType::RemoveShares:
        setAction(actionName(Split::Action::AddShares));
        break;
    case eMyMoney::Split::InvestmentTransactionType::SplitShares:
        setAction(actionName(Split::Action::SplitShares));
        break;
    case eMyMoney::Split::InvestmentTransactionType::InterestIncome:
        setAction(actionName(Split::Action::InterestIncome));
        break;
    case eMyMoney::Split::InvestmentTransactionType::UnknownTransactionType:
        break;
    }
}

eMyMoney::Split::InvestmentTransactionType MyMoneySplit::investmentTransactionType() const
{
    Q_D(const MyMoneySplit);
    switch(actionStringToAction(d->m_action)) {
    case Split::Action::BuyShares:
        return (d->m_shares.isNegative()) ? Split::InvestmentTransactionType::SellShares : Split::InvestmentTransactionType::BuyShares;

    case Split::Action::Dividend:
        return Split::InvestmentTransactionType::Dividend;

    case Split::Action::Yield:
        return Split::InvestmentTransactionType::Yield;

    case Split::Action::ReinvestDividend:
        return Split::InvestmentTransactionType::ReinvestDividend;

    case Split::Action::AddShares:
        return (d->m_shares.isNegative()) ? Split::InvestmentTransactionType::RemoveShares : Split::InvestmentTransactionType::AddShares;

    case Split::Action::SplitShares:
        return Split::InvestmentTransactionType::SplitShares;

    case Split::Action::InterestIncome:
        return Split::InvestmentTransactionType::InterestIncome;

    default:
        return Split::InvestmentTransactionType::UnknownTransactionType;
    }
}

QString MyMoneySplit::action() const
{
    Q_D(const MyMoneySplit);
    return d->m_action;
}

void MyMoneySplit::setAction(const QString& action)
{
    Q_D(MyMoneySplit);
    d->m_action = action;
}

bool MyMoneySplit::isAmortizationSplit() const
{
    Q_D(const MyMoneySplit);
    return d->m_action == actionName(Split::Action::Amortization);
}

bool MyMoneySplit::isInterestSplit() const
{
    Q_D(const MyMoneySplit);
    return d->m_action == actionName(Split::Action::Interest);
}

QString MyMoneySplit::number() const
{
    Q_D(const MyMoneySplit);
    return d->m_number;
}

void MyMoneySplit::setNumber(const QString& number)
{
    Q_D(MyMoneySplit);
    d->m_number = number;
}

bool MyMoneySplit::isAutoCalc() const
{
    Q_D(const MyMoneySplit);
    return (d->m_shares == MyMoneyMoney::autoCalc) || (d->m_value == MyMoneyMoney::autoCalc);
}

QString MyMoneySplit::bankID() const
{
    Q_D(const MyMoneySplit);
    return d->m_bankID;
}

void MyMoneySplit::setBankID(const QString& bankID)
{
    Q_D(MyMoneySplit);
    d->m_bankID = bankID;
}

QString MyMoneySplit::transactionId() const
{
    Q_D(const MyMoneySplit);
    return d->m_transactionId;
}

void MyMoneySplit::setTransactionId(const QString& id)
{
    Q_D(MyMoneySplit);
    d->m_transactionId = id;
}


MyMoneyMoney MyMoneySplit::value() const
{
    Q_D(const MyMoneySplit);
    return d->m_value;
}

MyMoneyMoney MyMoneySplit::value(const QString& transactionCurrencyId, const QString& splitCurrencyId) const
{
    Q_D(const MyMoneySplit);
    return (transactionCurrencyId == splitCurrencyId) ? d->m_value : d->m_shares;
}

void MyMoneySplit::setPrice(const MyMoneyMoney& price)
{
    Q_D(MyMoneySplit);
    d->m_price = price;
}

MyMoneyMoney MyMoneySplit::price() const
{
    Q_D(const MyMoneySplit);
    return d->m_price;
}

MyMoneyMoney MyMoneySplit::possiblyCalculatedPrice() const
{
    Q_D(const MyMoneySplit);
    if (!d->m_price.isZero())
        return d->m_price;
    if (!d->m_value.isZero() && !d->m_shares.isZero())
        return d->m_value / d->m_shares;
    return MyMoneyMoney::ONE;
}

bool MyMoneySplit::isMatched() const
{
    Q_D(const MyMoneySplit);
    return d->m_isMatched;
}

void MyMoneySplit::addMatch(const MyMoneyTransaction& _t)
{
    Q_D(MyMoneySplit);
    //  now we allow matching of two manual transactions
    d->m_matchedTransaction = _t;
    d->m_matchedTransaction.clearId();
    d->m_isMatched = true;
}

void MyMoneySplit::removeMatch()
{
    Q_D(MyMoneySplit);
    d->m_matchedTransaction = MyMoneyTransaction();
    d->m_isMatched = false;
}

MyMoneyTransaction MyMoneySplit::matchedTransaction() const
{
    Q_D(const MyMoneySplit);
    if (d->m_isMatched)
        return d->m_matchedTransaction;

    return MyMoneyTransaction();
}

bool MyMoneySplit::replaceId(const QString& newId, const QString& oldId)
{
    auto changed = false;
    Q_D(MyMoneySplit);

    if (d->m_payee == oldId) {
        d->m_payee = newId;
        changed = true;
    } else if (d->m_account == oldId) {
        d->m_account = newId;
        changed = true;
    } else if (d->m_costCenter == oldId) {
        d->m_costCenter = newId;
        changed = true;
    }

    if (isMatched()) {
        MyMoneyTransaction t = matchedTransaction();
        if (t.replaceId(newId, oldId)) {
            removeMatch();
            addMatch(t);
            changed = true;
        }
    }

    if (changed) {
        d->clearReferences();
    }
    return changed;
}

QHash<Split::Action, QString> actionNamesLUT()
{
    static const QHash<Split::Action, QString> actionNames{
        {Split::Action::ATM, QStringLiteral("ATM")},
        {Split::Action::AddShares, QStringLiteral("Add")},
        {Split::Action::Amortization, QStringLiteral("Amortization")},
        {Split::Action::BuyShares, QStringLiteral("Buy")},
        {Split::Action::Check, QStringLiteral("Check")},
        {Split::Action::Deposit, QStringLiteral("Deposit")},
        {Split::Action::Dividend, QStringLiteral("Dividend")},
        {Split::Action::Interest, QStringLiteral("Interest")},
        {Split::Action::InterestIncome, QStringLiteral("IntIncome")},
        {Split::Action::ReinvestDividend, QStringLiteral("Reinvest")},
        // SellShares is not present as action
        {Split::Action::SplitShares, QStringLiteral("Split")},
        {Split::Action::Transfer, QStringLiteral("Transfer")},
        {Split::Action::Withdrawal, QStringLiteral("Withdrawal")},
        {Split::Action::Yield, QStringLiteral("Yield")},
    };
    return actionNames;
}

QString MyMoneySplit::actionName(Split::Action action)
{
    return actionNamesLUT().value(action);
}

Split::Action MyMoneySplit::actionStringToAction(const QString& text)
{
    return actionNamesLUT().key(text, Split::Action::Unknown);
}

QString MyMoneySplit::actionI18nName(eMyMoney::Split::Action action)
{
    static const QHash<Split::Action, QString> actionNames{
        {
            Split::Action::ATM,
            i18nc("Investment action", "ATM"),
        },
        {
            Split::Action::AddShares,
            i18nc("Investment action", "Add shares"),
        },
        {
            Split::Action::Amortization,
            i18nc("Investment action", "Amortization"),
        },
        {
            Split::Action::BuyShares,
            i18nc("Investment action", "Buy shares"),
        },
        {
            Split::Action::Check,
            i18nc("Investment action", "Check"),
        },
        {
            Split::Action::Deposit,
            i18nc("Investment action", "Deposit"),
        },
        {
            Split::Action::Dividend,
            i18nc("Investment action", "Dividend"),
        },
        {
            Split::Action::Interest,
            i18nc("Investment action", "Interest"),
        },
        {
            Split::Action::InterestIncome,
            i18nc("Investment action", "Interest income"),
        },
        {
            Split::Action::ReinvestDividend,
            i18nc("Investment action", "Reinvest dividends"),
        },
        // SellShares is not present as action
        {
            Split::Action::SplitShares,
            i18nc("Investment action", "Split shares"),
        },
        {
            Split::Action::Transfer,
            i18nc("Investment action", "Transfer"),
        },
        {
            Split::Action::Withdrawal,
            i18nc("Investment action", "Withdrawal"),
        },
        {
            Split::Action::Yield,
            i18nc("Investment action", "Yield"),
        },
    };
    return actionNames.value(action);
}

QString MyMoneySplit::actionI18nName(const QString& text)
{
    return actionI18nName(actionStringToAction(text));
}
