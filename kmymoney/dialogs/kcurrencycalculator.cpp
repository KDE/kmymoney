/*
    SPDX-FileCopyrightText: 2004-2021 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kcurrencycalculator.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QButtonGroup>
#include <QLabel>
#include <QRadioButton>
#include <QCheckBox>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QPointer>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kcurrencycalculator.h"

#include "mymoneyfile.h"
#include "mymoneyaccount.h"
#include "mymoneysecurity.h"
#include "mymoneyprice.h"
#include "mymoneymoney.h"
#include "mymoneyexception.h"
#include "mymoneysplit.h"
#include "mymoneytransaction.h"
#include "kmymoneysettings.h"

class KCurrencyCalculatorPrivate
{
    Q_DISABLE_COPY(KCurrencyCalculatorPrivate)
    Q_DECLARE_PUBLIC(KCurrencyCalculator)

public:
    explicit KCurrencyCalculatorPrivate(KCurrencyCalculator* qq)
        : q_ptr(qq)
        , ui(new Ui::KCurrencyCalculator)
    {
    }

    KCurrencyCalculatorPrivate(KCurrencyCalculator* qq,
                               const MyMoneySecurity& from,
                               const MyMoneySecurity& to,
                               const MyMoneyMoney& fromAmount,
                               const MyMoneyMoney& toAmount,
                               const QDate& date,
                               const signed64 resultFraction)
        : q_ptr(qq)
        , ui(new Ui::KCurrencyCalculator)
        , m_fromCurrency(from)
        , m_toCurrency(to)
        , m_toAmount(toAmount.abs())
        , m_fromAmount(fromAmount.abs())
        , m_date(date)
        , m_resultFraction(resultFraction)
    {
    }

    ~KCurrencyCalculatorPrivate()
    {
        delete ui;
    }

    void init()
    {
        Q_Q(KCurrencyCalculator);
        ui->setupUi(q);
        auto file = MyMoneyFile::instance();

        // set main widget of QDialog
        ui->buttonGroup1->setId(ui->m_amountButton, 0);
        ui->buttonGroup1->setId(ui->m_rateButton, 1);

        ui->m_dateFrame->hide();

        // set bold font
        auto boldFont = ui->m_fromCurrencyText->font();
        boldFont.setBold(true);
        ui->m_fromCurrencyText->setFont(boldFont);
        ui->m_toCurrencyText->setFont(boldFont);

        ui->m_updateButton->setChecked(KMyMoneySettings::priceHistoryUpdate());

        // setup initial result
        if (m_toAmount.isZero() && !m_fromAmount.isZero()) {
            const MyMoneyPrice &pr = file->price(m_fromCurrency.id(), m_toCurrency.id(), m_date);
            if (pr.isValid()) {
                m_toAmount = m_fromAmount * pr.rate(m_toCurrency.id());
            }
        }

        // fill in initial values
        ui->m_toAmount->setCommodity(MyMoneySecurity());
        ui->m_toAmount->setPrecision(-1);
        ui->m_toAmount->setValue(m_toAmount);

        ui->m_conversionRate->setCommodity(MyMoneySecurity());
        ui->m_conversionRate->setPrecision(-1);

        q->connect(ui->m_amountButton, &QAbstractButton::clicked, q, &KCurrencyCalculator::slotSetToAmount);
        q->connect(ui->m_rateButton, &QAbstractButton::clicked, q, &KCurrencyCalculator::slotSetExchangeRate);

        q->connect(ui->m_toAmount, &AmountEdit::textChanged, q, &KCurrencyCalculator::slotUpdateResult);
        q->connect(ui->m_conversionRate, &AmountEdit::textChanged, q, &KCurrencyCalculator::slotUpdateRate);

        q->connect(ui->m_toAmount, &AmountEdit::returnPressed, q, &KCurrencyCalculator::accept);
        q->connect(ui->m_conversionRate, &AmountEdit::returnPressed, q, &KCurrencyCalculator::accept);

        // use this as the default
        ui->m_amountButton->animateClick();
        q->slotUpdateResult(ui->m_toAmount->text());

        // If the from security is not a currency, we only allow entering a price
        if (!m_fromCurrency.isCurrency()) {
            ui->m_rateButton->animateClick();
            ui->m_amountButton->hide();
            ui->m_toAmount->hide();
        }

        updateWidgets();
    }

    void updateExample(const MyMoneyMoney& price)
    {
        QString msg;
        if (price.isZero()) {
            msg = QString("1 %1 = ? %2").arg(m_fromCurrency.tradingSymbol())
                  .arg(m_toCurrency.tradingSymbol());
            if (m_fromCurrency.isCurrency()) {
                msg += QString("\n");
                msg += QString("1 %1 = ? %2").arg(m_toCurrency.tradingSymbol())
                       .arg(m_fromCurrency.tradingSymbol());
            }
        } else {
            msg = QString("1 %1 = %2 %3").arg(m_fromCurrency.tradingSymbol())
                  .arg(price.formatMoney(QString(), m_fromCurrency.pricePrecision()))
                  .arg(m_toCurrency.tradingSymbol());
            if (m_fromCurrency.isCurrency()) {
                msg += QString("\n");
                msg += QString("1 %1 = %2 %3").arg(m_toCurrency.tradingSymbol())
                       .arg((MyMoneyMoney::ONE / price).formatMoney(QString(), m_toCurrency.pricePrecision()))
                       .arg(m_fromCurrency.tradingSymbol());
            }
        }
        ui->m_conversionExample->setText(msg);
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!price.isZero());
    }

    void updateWidgets()
    {
        if (m_date.isValid())
            ui->m_dateEdit->setDate(m_date);
        else
            ui->m_dateEdit->setDate(QDate::currentDate());

        ui->m_dateText->setText(QLocale().toString(m_date));

        ui->m_fromCurrencyText->setText(QStringLiteral("%1 %2").arg(MyMoneySecurity::securityTypeToString(m_fromCurrency.securityType()),
                                                                    (m_fromCurrency.isCurrency() ? m_fromCurrency.id() : m_fromCurrency.tradingSymbol())));
        ui->m_toCurrencyText->setText(QStringLiteral("%1 %2").arg(MyMoneySecurity::securityTypeToString(m_toCurrency.securityType()),
                                                                  (m_toCurrency.isCurrency() ? m_toCurrency.id() : m_toCurrency.tradingSymbol())));

        ui->m_fromAmount->setText(m_fromAmount.formatMoney(QString(), MyMoneyMoney::denomToPrec(m_fromCurrency.smallestAccountFraction())));
    }

    KCurrencyCalculator* q_ptr;
    Ui::KCurrencyCalculator *ui;
    MyMoneySecurity m_fromCurrency;
    MyMoneySecurity m_toCurrency;
    MyMoneyMoney m_toAmount;
    MyMoneyMoney m_fromAmount;
    QDate m_date;
    signed64 m_resultFraction;
};

KCurrencyCalculator::KCurrencyCalculator(QWidget* parent)
    : QDialog(parent)
    , d_ptr(new KCurrencyCalculatorPrivate(this))
{
    Q_D(KCurrencyCalculator);
    d->init();
}

KCurrencyCalculator::KCurrencyCalculator(const MyMoneySecurity& from,
        const MyMoneySecurity& to,
        const MyMoneyMoney& value,
        const MyMoneyMoney& shares,
        const QDate& date,
        const signed64 resultFraction,
        QWidget *parent) :
    QDialog(parent),
    d_ptr(new KCurrencyCalculatorPrivate(this,
                                         from,
                                         to,
                                         value,
                                         shares,
                                         date,
                                         resultFraction))
{
    Q_D(KCurrencyCalculator);
    d->init();
}

KCurrencyCalculator::~KCurrencyCalculator()
{
    Q_D(KCurrencyCalculator);
    delete d;
}

void KCurrencyCalculator::setDate(const QDate& date)
{
    Q_D(KCurrencyCalculator);
    d->m_date = date;
    if (date.isValid())
        d->ui->m_dateEdit->setDate(date);
    else
        d->ui->m_dateEdit->setDate(QDate::currentDate());

    d->ui->m_dateText->setText(QLocale().toString(date));
}

void KCurrencyCalculator::setFromCurrency(const MyMoneySecurity& sec)
{
    Q_D(KCurrencyCalculator);
    d->m_fromCurrency = sec;
    d->ui->m_fromCurrencyText->setText(
        QString(MyMoneySecurity::securityTypeToString(sec.securityType()) + ' ' + (sec.isCurrency() ? sec.id() : sec.tradingSymbol())));

    // If the from security is not a currency, we only allow entering a price
    if (!sec.isCurrency()) {
        d->ui->m_rateButton->animateClick();
        d->ui->m_amountButton->hide();
        d->ui->m_toAmount->hide();
    } else {
        d->ui->m_amountButton->show();
        d->ui->m_toAmount->show();
    }
    d->updateWidgets();
}

void KCurrencyCalculator::setToCurrency(const MyMoneySecurity& sec)
{
    Q_D(KCurrencyCalculator);
    d->m_toCurrency = sec;
    d->ui->m_toCurrencyText->setText(
        QString(MyMoneySecurity::securityTypeToString(sec.securityType()) + ' ' + (sec.isCurrency() ? sec.id() : sec.tradingSymbol())));
    d->updateWidgets();
}

void KCurrencyCalculator::setFromAmount(const MyMoneyMoney& amount)
{
    Q_D(KCurrencyCalculator);
    d->m_fromAmount = amount;
    d->updateWidgets();
}

void KCurrencyCalculator::setToAmount(const MyMoneyMoney& amount)
{
    Q_D(KCurrencyCalculator);
    d->m_toAmount = amount;
    d->updateWidgets();
}

void KCurrencyCalculator::setResultFraction(signed64 fraction)
{
    Q_D(KCurrencyCalculator);
    d->m_resultFraction = fraction;
    d->updateWidgets();
}

bool KCurrencyCalculator::setupSplitPrice(MyMoneyMoney& shares,
        const MyMoneyTransaction& t,
        const MyMoneySplit& s,
        const QMap<QString,
        MyMoneyMoney>& priceInfo,
        QWidget* parentWidget)
{
    auto rc = true;
    auto file = MyMoneyFile::instance();

    if (!s.value().isZero()) {
        auto cat = file->account(s.accountId());
        MyMoneySecurity toCurrency;
        toCurrency = file->security(cat.currencyId());
        // determine the fraction required for this category/account
        int fract = cat.fraction(toCurrency);

        if (cat.currencyId() != t.commodity()) {

            MyMoneyMoney toValue;
            auto fromCurrency = file->security(t.commodity());
            // display only positive values to the user
            auto fromValue = s.value().abs();

            // if we had a price info in the beginning, we use it here
            if (priceInfo.find(cat.currencyId()) != priceInfo.end()) {
                toValue = (fromValue * priceInfo[cat.currencyId()]).convert(fract);
            }
            // if the shares are still 0, we need to change that
            if (toValue.isZero()) {
                const MyMoneyPrice &price = file->price(fromCurrency.id(), toCurrency.id(), t.postDate());
                // if the price is valid calculate the shares. If it is invalid
                // assume a conversion rate of 1.0
                if (price.isValid()) {
                    toValue = (price.rate(toCurrency.id()) * fromValue).convert(fract);
                } else {
                    toValue = fromValue;
                }
            }

            // now present all that to the user
            QPointer<KCurrencyCalculator> calc =
                new KCurrencyCalculator(fromCurrency,
                                        toCurrency,
                                        fromValue,
                                        toValue,
                                        t.postDate(),
                                        10000000000,
                                        parentWidget);

            if (calc->exec() == QDialog::Rejected) {
                rc = false;
            } else
                shares = (s.value() * calc->price()).convert(fract);

            delete calc;

        } else {
            shares = s.value().convert(fract);
        }
    } else
        shares = s.value();

    return rc;
}

void KCurrencyCalculator::setupPriceEditor()
{
    Q_D(KCurrencyCalculator);
    d->ui->m_dateFrame->show();
    d->ui->m_amountDateFrame->hide();
    d->ui->m_updateButton->setChecked(true);
    d->ui->m_updateButton->hide();
}

void KCurrencyCalculator::slotSetToAmount()
{
    Q_D(KCurrencyCalculator);
    d->ui->m_rateButton->setChecked(false);
    d->ui->m_toAmount->setEnabled(true);
    d->ui->m_conversionRate->setEnabled(false);
}

void KCurrencyCalculator::slotSetExchangeRate()
{
    Q_D(KCurrencyCalculator);
    d->ui->m_amountButton->setChecked(false);
    d->ui->m_toAmount->setEnabled(false);
    d->ui->m_conversionRate->setEnabled(true);
}

void KCurrencyCalculator::slotUpdateResult(const QString& /*txt*/)
{
    Q_D(KCurrencyCalculator);
    MyMoneyMoney result = d->ui->m_toAmount->value();
    MyMoneyMoney price(MyMoneyMoney::ONE);

    if (result.isNegative()) {
        d->ui->m_toAmount->setValue(-result);
        slotUpdateResult(QString());
        return;
    }

    if (!result.isZero() && !d->m_fromAmount.isZero()) {
        price = result / d->m_fromAmount;

        d->ui->m_conversionRate->setValue(price);
        d->m_toAmount = (d->m_fromAmount * price).convert(d->m_resultFraction);

        d->ui->m_toAmount->setValue(d->m_toAmount);
    }
    d->updateExample(price);
}

void KCurrencyCalculator::slotUpdateRate(const QString& /*txt*/)
{
    Q_D(KCurrencyCalculator);
    auto price = d->ui->m_conversionRate->value();

    if (price.isNegative()) {
        d->ui->m_conversionRate->setValue(-price);
        slotUpdateRate(QString());
        return;
    }

    if (!price.isZero()) {
        d->ui->m_conversionRate->setValue(price);
        d->m_toAmount = (d->m_fromAmount * price).convert(d->m_resultFraction);
        d->ui->m_toAmount->setValue(d->m_toAmount);
    }
    d->updateExample(price);
}

void KCurrencyCalculator::accept()
{
    Q_D(KCurrencyCalculator);
    if (d->ui->m_conversionRate->isEnabled())
        slotUpdateRate(QString());
    else
        slotUpdateResult(QString());

    if (d->ui->m_updateButton->isChecked()) {
        auto pr = MyMoneyFile::instance()->price(d->m_fromCurrency.id(), d->m_toCurrency.id(), d->ui->m_dateEdit->date());
        if (!pr.isValid() //
                || pr.date() != d->ui->m_dateEdit->date() //
                || (pr.date() == d->ui->m_dateEdit->date() && pr.rate(d->m_fromCurrency.id()) != price())) {
            pr = MyMoneyPrice(d->m_fromCurrency.id(), d->m_toCurrency.id(), d->ui->m_dateEdit->date(), price(), i18n("User"));
            MyMoneyFileTransaction ft;
            try {
                MyMoneyFile::instance()->addPrice(pr);
                ft.commit();
            } catch (const MyMoneyException &) {
                qDebug("Cannot add price");
            }
        }
    }

    // remember setting for next round
    KMyMoneySettings::setPriceHistoryUpdate(d->ui->m_updateButton->isChecked());
    QDialog::accept();
}

MyMoneyMoney KCurrencyCalculator::price() const
{
    Q_D(const KCurrencyCalculator);
    // This should fix https://bugs.kde.org/show_bug.cgi?id=205254 and
    // https://bugs.kde.org/show_bug.cgi?id=325953 as well as
    // https://bugs.kde.org/show_bug.cgi?id=300965
    if (d->ui->m_amountButton->isChecked()) {
        if (!d->m_fromAmount.isZero()) {
            return d->ui->m_toAmount->value().abs() / d->m_fromAmount.abs();
        }
        return MyMoneyMoney::ONE;
    } else
        return d->ui->m_conversionRate->value();
}

void KCurrencyCalculator::updateConversion(MultiCurrencyEdit* amountEdit, const QDate date)
{
    // in case the widget does not have multiple currencies we're done
    if (!amountEdit->hasMultipleCurrencies())
        return;

    const auto file = MyMoneyFile::instance();
    QPointer<KCurrencyCalculator> calc;

    MyMoneyMoney fromValue;
    MyMoneyMoney toValue;
    MyMoneySecurity fromSecurity;
    MyMoneySecurity toSecurity;

    const auto state = amountEdit->displayState();
    int fraction = MyMoneyMoney::precToDenom(amountEdit->precision(state));

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

    // in case the two values are identical, we search for
    // a matching conversion rate in the price list and
    // apply it.
    if (fromValue == toValue) {
        const auto rate = file->price(fromSecurity.id(), toSecurity.id(), date).rate(toSecurity.id());
        toValue *= rate;
    }

    calc = new KCurrencyCalculator(fromSecurity, toSecurity, fromValue, toValue, date, fraction, amountEdit->widget());

    if (calc->exec() == QDialog::Accepted && calc) {
        switch (state) {
        case MultiCurrencyEdit::DisplayShares:
            amountEdit->setValue(fromValue * calc->price());
            amountEdit->setShares(fromValue);
            break;
        case MultiCurrencyEdit::DisplayValue:
            amountEdit->setValue(fromValue);
            amountEdit->setShares(fromValue * calc->price());
            break;
        }
    }
    delete calc;
}
