/*
    SPDX-FileCopyrightText: 2021 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "creditdebitedit.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDebug>
#include <QPointer>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "amountedit.h"
#include "mymoneysecurity.h"
#include "ui_creditdebitedit.h"

class CreditDebitEditPrivate
{
    Q_DISABLE_COPY(CreditDebitEditPrivate)
    Q_DECLARE_PUBLIC(CreditDebitEdit)

public:
    explicit CreditDebitEditPrivate(CreditDebitEdit* qq)
        : q_ptr(qq)
        , ui(new Ui::CreditDebitEdit)
    {
        ui->setupUi(qq);
    }

    void widgetChanged(AmountEdit* src, AmountEdit* dst)
    {
        // make sure the objects exist
        if (!src || !dst) {
            return;
        }

        // in case both are filled with text, the src wins
        if (!src->text().isEmpty() && !dst->text().isEmpty()) {
            dst->clear();
        }

        // in case the source is negative, we negate the value
        // and load it into destination.
        if (src->value().isNegative()) {
            dst->setValue(-(src->value()));
            dst->setShares(-(src->shares()));
            src->clear();
        }
        Q_Q(CreditDebitEdit);
        Q_EMIT q->amountChanged();
    }

    CreditDebitEdit* q_ptr;
    Ui::CreditDebitEdit* ui;
};

CreditDebitEdit::CreditDebitEdit(QWidget* parent)
    : QWidget(parent)
    , d_ptr(new CreditDebitEditPrivate(this))
{
    Q_D(CreditDebitEdit);
    connect(d->ui->creditAmount, &AmountEdit::amountChanged, this, &CreditDebitEdit::creditChanged);
    connect(d->ui->debitAmount, &AmountEdit::amountChanged, this, &CreditDebitEdit::debitChanged);

    // propagate display state changes
    connect(d->ui->creditAmount, &AmountEdit::displayStateChanged, d->ui->debitAmount, &AmountEdit::setDisplayState);
    connect(d->ui->debitAmount, &AmountEdit::displayStateChanged, d->ui->creditAmount, &AmountEdit::setDisplayState);

    setFocusPolicy(Qt::StrongFocus);
    setFocusProxy(d->ui->creditAmount);
}

CreditDebitEdit::~CreditDebitEdit()
{
    Q_D(CreditDebitEdit);
    delete d;
}

void CreditDebitEdit::creditChanged()
{
    Q_D(CreditDebitEdit);
    d->widgetChanged(d->ui->creditAmount, d->ui->debitAmount);
}

void CreditDebitEdit::debitChanged()
{
    Q_D(CreditDebitEdit);
    d->widgetChanged(d->ui->debitAmount, d->ui->creditAmount);
}

bool CreditDebitEdit::haveValue() const
{
    Q_D(const CreditDebitEdit);
    return (!d->ui->creditAmount->text().isEmpty()) || (!d->ui->debitAmount->text().isEmpty());
}

MyMoneyMoney CreditDebitEdit::value() const
{
    Q_D(const CreditDebitEdit);
    MyMoneyMoney value;

    if (!d->ui->creditAmount->text().isEmpty()) {
        value = -d->ui->creditAmount->value();
    } else {
        value = d->ui->debitAmount->value();
    }
    return value;
}

void CreditDebitEdit::setValue(const MyMoneyMoney& amount)
{
    Q_D(CreditDebitEdit);

    if (amount.isNegative()) {
        d->ui->creditAmount->setValue(-amount);
        d->ui->debitAmount->clear();
    } else {
        d->ui->debitAmount->setValue(amount);
        d->ui->creditAmount->clear();
    }
}

MyMoneyMoney CreditDebitEdit::shares() const
{
    Q_D(const CreditDebitEdit);
    MyMoneyMoney value;

    if (!d->ui->creditAmount->text().isEmpty()) {
        value = -d->ui->creditAmount->shares();
    } else {
        value = d->ui->debitAmount->shares();
    }
    return value;
}

void CreditDebitEdit::setShares(const MyMoneyMoney& amount)
{
    Q_D(CreditDebitEdit);

    if (amount.isNegative()) {
        d->ui->creditAmount->setShares(-amount);
        d->ui->debitAmount->clear();
    } else {
        d->ui->debitAmount->setShares(amount);
        d->ui->creditAmount->clear();
    }
}

void CreditDebitEdit::setCurrencySymbol(const QString& symbol, const QString& name)
{
    Q_D(CreditDebitEdit);
    d->ui->creditAmount->setCurrencySymbol(symbol, name);
    d->ui->debitAmount->setCurrencySymbol(symbol, name);
}

void CreditDebitEdit::setValueCommodity(const MyMoneySecurity& commodity)
{
    Q_D(CreditDebitEdit);

    d->ui->creditAmount->setValueCommodity(commodity);
    d->ui->debitAmount->setValueCommodity(commodity);
}

void CreditDebitEdit::setSharesCommodity(const MyMoneySecurity& commodity)
{
    Q_D(CreditDebitEdit);

    d->ui->creditAmount->setSharesCommodity(commodity);
    d->ui->debitAmount->setSharesCommodity(commodity);
}

MyMoneySecurity CreditDebitEdit::valueCommodity() const
{
    Q_D(const CreditDebitEdit);
    return d->ui->creditAmount->valueCommodity();
}

MyMoneySecurity CreditDebitEdit::sharesCommodity() const
{
    Q_D(const CreditDebitEdit);
    return d->ui->creditAmount->sharesCommodity();
}

void CreditDebitEdit::setCommodity(const MyMoneySecurity& commodity)
{
    setValueCommodity(commodity);
    setSharesCommodity(commodity);
}

void CreditDebitEdit::setInitialExchangeRate(const MyMoneyMoney& price)
{
    Q_D(CreditDebitEdit);
    d->ui->creditAmount->setInitialExchangeRate(price);
    d->ui->debitAmount->setInitialExchangeRate(price);
}

MyMoneyMoney CreditDebitEdit::initialExchangeRate() const
{
    Q_D(const CreditDebitEdit);
    // we only need to return the exchange rate of one widget
    return d->ui->creditAmount->initialExchangeRate();
}

void CreditDebitEdit::swapCreditDebit()
{
    Q_D(CreditDebitEdit);
    disconnect(d->ui->creditAmount, &AmountEdit::amountChanged, this, &CreditDebitEdit::creditChanged);
    disconnect(d->ui->debitAmount, &AmountEdit::amountChanged, this, &CreditDebitEdit::debitChanged);

    std::swap(d->ui->creditAmount, d->ui->debitAmount);

    connect(d->ui->creditAmount, &AmountEdit::amountChanged, this, &CreditDebitEdit::creditChanged);
    connect(d->ui->debitAmount, &AmountEdit::amountChanged, this, &CreditDebitEdit::debitChanged);
}

void CreditDebitEdit::setAllowEmpty(bool allowed)
{
    Q_D(CreditDebitEdit);
    d->ui->creditAmount->setAllowEmpty(allowed);
    d->ui->debitAmount->setAllowEmpty(allowed);
}

void CreditDebitEdit::setPlaceholderText(const QString& creditText, const QString& debitText)
{
    Q_D(CreditDebitEdit);
    d->ui->creditAmount->setPlaceholderText(creditText);
    d->ui->debitAmount->setPlaceholderText(debitText);
}

QWidget* CreditDebitEdit::widget()
{
    return this;
}

MultiCurrencyEdit::DisplayState CreditDebitEdit::displayState() const
{
    Q_D(const CreditDebitEdit);
    return d->ui->creditAmount->displayState();
}

bool CreditDebitEdit::hasMultipleCurrencies() const
{
    Q_D(const CreditDebitEdit);
    return d->ui->creditAmount->hasMultipleCurrencies();
}

int CreditDebitEdit::precision(MultiCurrencyEdit::DisplayState state) const
{
    Q_D(const CreditDebitEdit);
    return d->ui->creditAmount->precision(state);
}

void CreditDebitEdit::setDisplayState(MultiCurrencyEdit::DisplayState state)
{
    Q_D(CreditDebitEdit);
    d->ui->creditAmount->setDisplayState(state);
    d->ui->debitAmount->setDisplayState(state);
}
