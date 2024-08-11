/*
    SPDX-FileCopyrightText: 2024 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kcurrencyconverter.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QMap>
#include <QPair>
#include <QPointer>
#include <QString>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "amountedit.h"
#include "kcurrencycalculator.h"
#include "mymoneyfile.h"
#include "mymoneymoney.h"
#include "mymoneyprice.h"
#include "mymoneysecurity.h"

typedef QPair<QString, QString> CacheKey;

class KCurrencyConverterPrivate
{
    Q_DISABLE_COPY(KCurrencyConverterPrivate)
    Q_DECLARE_PUBLIC(KCurrencyConverter)

public:
    explicit KCurrencyConverterPrivate(KCurrencyConverter* qq)
        : q_ptr(qq)
    {
    }

    KCurrencyConverter* q_ptr;
    QMap<CacheKey, MyMoneyMoney> m_prices;
};

KCurrencyConverter::KCurrencyConverter()
    : d_ptr(new KCurrencyConverterPrivate(this))
{
}

KCurrencyConverter::~KCurrencyConverter()
{
    Q_D(KCurrencyConverter);
    delete d;
}

MyMoneyMoney KCurrencyConverter::updateRate(MultiCurrencyEdit* amountEdit, const QDate& date)
{
    // no need to do anything when the same currency is used
    if (!amountEdit->hasMultipleCurrencies()) {
        return MyMoneyMoney::ONE;
    }

    Q_D(KCurrencyConverter);
    QPointer<KCurrencyCalculator> calc;

    MyMoneyMoney fromValue;
    MyMoneyMoney toValue;
    MyMoneySecurity fromSecurity;
    MyMoneySecurity toSecurity;

    const auto state = amountEdit->displayState();
    const int fraction = MyMoneyMoney::precToDenom(amountEdit->precision(state));
    const CacheKey cacheKey = qMakePair(amountEdit->sharesCommodity().id(), amountEdit->valueCommodity().id());

    switch (state) {
    case MultiCurrencyEdit::DisplayShares:
        fromValue = amountEdit->shares();
        toValue = amountEdit->value();
        fromSecurity = amountEdit->sharesCommodity();
        toSecurity = amountEdit->valueCommodity();
        break;

    case MultiCurrencyEdit::DisplayValue:
        fromValue = amountEdit->value();
        toValue = amountEdit->shares();
        fromSecurity = amountEdit->valueCommodity();
        toSecurity = amountEdit->sharesCommodity();
        break;
    }

    MyMoneyMoney rate = amountEdit->initialExchangeRate();
    bool foundCachedValue = (amountEdit->lastChangedByUser() != AmountEdit::SharesChanged);

    // invert rate if needed
    switch (state) {
    case MultiCurrencyEdit::DisplayShares:
        break;
    case MultiCurrencyEdit::DisplayValue:
        rate = MyMoneyMoney::ONE / rate;
        break;
    }

    if (rate == MyMoneyMoney::ONE) {
        foundCachedValue = false;
        if (d->m_prices.contains(cacheKey)) {
            rate = d->m_prices.value(cacheKey);
            if (state == MultiCurrencyEdit::DisplayValue) {
                rate = MyMoneyMoney::ONE / rate;
            }
        } else {
            rate = MyMoneyFile::instance()->price(fromSecurity.id(), toSecurity.id(), date).rate(toSecurity.id());
        }
        if (rate != MyMoneyMoney::ONE) {
            foundCachedValue = true;
        }
        toValue *= rate;
    }

    if (!foundCachedValue || (amountEdit->lastChangedByUser() == AmountEdit::SharesChanged)) {
        calc = new KCurrencyCalculator(fromSecurity, toSecurity, fromValue, toValue, date, fraction, amountEdit->widget());
        calc->blockSignalsDuringUpdate(true);

        if (calc->exec() == QDialog::Accepted) {
            if (calc) {
                rate = calc->price();
            }
        }
    }

    // invert rate if needed
    switch (state) {
    case MultiCurrencyEdit::DisplayShares:
        break;
    case MultiCurrencyEdit::DisplayValue:
        rate = MyMoneyMoney::ONE / rate;
        break;
    }

    d->m_prices[cacheKey] = rate;
    return rate;
}
