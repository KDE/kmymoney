/*
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2020 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kcurrencyeditordlg.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QPushButton>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include <ui_kcurrencyeditordlg.h>
#include "mymoneymoney.h"
#include "mymoneysecurity.h"

class KCurrencyEditorDlgPrivate
{
public:
    KCurrencyEditorDlgPrivate()
        : ui(new Ui::KCurrencyEditorDlg)
    {
    }

    ~KCurrencyEditorDlgPrivate()
    {
        delete ui;
    }

    void validateValues()
    {
        // enable the OK button only, if all values are valid
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);

        if (ui->leIsoCode->text().isEmpty()
                || ui->leName->text().isEmpty()
                || ui->leSymbol->text().isEmpty()
                || MyMoneyMoney(ui->leCashFraction->text()).isZero()
                || MyMoneyMoney(ui->leCashFraction->text()).isNegative()
                || MyMoneyMoney(ui->leAccountFraction->text()).isZero()
                || MyMoneyMoney(ui->leAccountFraction->text()).isNegative()) {
            ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        }
    }

    MyMoneySecurity currency;
    Ui::KCurrencyEditorDlg* ui;
};

KCurrencyEditorDlg::KCurrencyEditorDlg(const MyMoneySecurity& currency, QWidget *parent)
    : d_ptr(new KCurrencyEditorDlgPrivate)
{
    Q_UNUSED(parent);
    Q_D(KCurrencyEditorDlg);
    d->currency = currency;
    d->ui->setupUi(this);

    connect(d->ui->leIsoCode, &QLineEdit::textChanged, this, [&]() {
        Q_D(KCurrencyEditorDlg);
        d->validateValues();
    });
    connect(d->ui->leName, &QLineEdit::textChanged, this, [&]() {
        Q_D(KCurrencyEditorDlg);
        d->validateValues();
    });
    connect(d->ui->leSymbol, &QLineEdit::textChanged, this, [&]() {
        Q_D(KCurrencyEditorDlg);
        d->validateValues();
    });
    connect(d->ui->leCashFraction, &QLineEdit::textChanged, this, [&]() {
        Q_D(KCurrencyEditorDlg);
        d->validateValues();
    });
    connect(d->ui->leAccountFraction, &QLineEdit::textChanged, this, [&]() {
        Q_D(KCurrencyEditorDlg);
        d->validateValues();
    });

    // fill the fields
    d->ui->leIsoCode->setText(currency.id());

    d->ui->leName->setText(currency.name());
    d->ui->leSymbol->setText(currency.tradingSymbol());

    int precision = MyMoneyMoney::denomToPrec(currency.smallestCashFraction());
    MyMoneyMoney smallestFraction = MyMoneyMoney::ONE / MyMoneyMoney(currency.smallestCashFraction());
    d->ui->leCashFraction->setText(smallestFraction.formatMoney(QString(), precision));

    precision = MyMoneyMoney::denomToPrec(currency.smallestAccountFraction());
    smallestFraction = MyMoneyMoney::ONE / MyMoneyMoney(currency.smallestAccountFraction());
    d->ui->leAccountFraction->setText(smallestFraction.formatMoney(QString(), precision));

    d->ui->comboRoundingMethod->setCurrentIndex(currency.roundingMethod());
    d->ui->spbPricePrecision->setValue(currency.pricePrecision());


    // make those widgets readonly that are not allowed to change
    if (!currency.id().isEmpty()) {
        d->ui->leIsoCode->setReadOnly(true);
        d->ui->leCashFraction->setReadOnly(true);
        d->ui->leAccountFraction->setReadOnly(true);
    }

    d->validateValues();
}

KCurrencyEditorDlg::~KCurrencyEditorDlg()
{
    Q_D(KCurrencyEditorDlg);
    delete d;
}

MyMoneySecurity KCurrencyEditorDlg::currency() const
{
    // MyMoneySecurity handles the maximum fraction as
    // int. mpz_class only supports conversion with 32bit
    // ints so we limit the maximum fraction to a value
    // representable by such a variable. Modifying the
    // handling of fraction to mpz_class based integers
    // could solve the problem.
    auto maxFraction = [&](unsigned int f) {
        if ((f == 0) || (f > 2147483647)) {
            f = 1000000000;
        }
        return f;
    };

    Q_D(const KCurrencyEditorDlg);
    MyMoneySecurity newCurrency = d->currency;

    // if we are creating a new currency, we need to assign the ID
    if (d->currency.id().isEmpty()) {
        newCurrency = MyMoneySecurity(d->ui->leIsoCode->text(), newCurrency);
    }

    newCurrency.setName(d->ui->leName->text());
    newCurrency.setTradingSymbol(d->ui->leSymbol->text());

    MyMoneyMoney value(d->ui->leCashFraction->text());
    int fraction = maxFraction(static_cast<unsigned int>((MyMoneyMoney::ONE / value.abs()).toDouble()));
    newCurrency.setSmallestCashFraction(fraction);

    value = MyMoneyMoney(d->ui->leAccountFraction->text());
    fraction = maxFraction(static_cast<unsigned int>((MyMoneyMoney::ONE / value.abs()).toDouble()));
    newCurrency.setSmallestAccountFraction(fraction);

    newCurrency.setRoundingMethod(static_cast<AlkValue::RoundingMethod>(d->ui->comboRoundingMethod->currentIndex()));
    newCurrency.setPricePrecision(d->ui->spbPricePrecision->value());
    return newCurrency;
}

