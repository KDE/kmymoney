/***************************************************************************
                          kcurrencycalculator.cpp  -  description
                             -------------------
    begin                : Thu Apr 8 2004
    copyright            : (C) 2000-2004 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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
#include "kmymoneyedit.h"
#include "kmymoneydateinput.h"
#include "mymoneyprice.h"
#include "mymoneymoney.h"
#include "mymoneysplit.h"
#include "mymoneytransaction.h"
#include "kmymoneyglobalsettings.h"

class KCurrencyCalculatorPrivate
{
  Q_DISABLE_COPY(KCurrencyCalculatorPrivate)
  Q_DECLARE_PUBLIC(KCurrencyCalculator)

public:
  explicit KCurrencyCalculatorPrivate(KCurrencyCalculator *qq,
                                      const MyMoneySecurity& from,
                                      const MyMoneySecurity& to,
                                      const MyMoneyMoney& value,
                                      const MyMoneyMoney& shares,
                                      const QDate& date,
                                      const signed64 resultFraction) :
    q_ptr(qq),
    ui(new Ui::KCurrencyCalculator),
    m_fromCurrency(from),
    m_toCurrency(to),
    m_result(shares.abs()),
    m_value(value.abs()),
    m_date(date),
    m_resultFraction(resultFraction)
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

    //set main widget of QDialog
    ui->buttonGroup1->setId(ui->m_amountButton, 0);
    ui->buttonGroup1->setId(ui->m_rateButton, 1);

    ui->m_dateFrame->hide();
    if (m_date.isValid())
      ui->m_dateEdit->setDate(m_date);
    else
      ui->m_dateEdit->setDate(QDate::currentDate());

    ui->m_fromCurrencyText->setText(QString(MyMoneySecurity::securityTypeToString(m_fromCurrency.securityType()) + ' ' + (m_fromCurrency.isCurrency() ? m_fromCurrency.id() : m_fromCurrency.tradingSymbol())));
    ui->m_toCurrencyText->setText(QString(MyMoneySecurity::securityTypeToString(m_toCurrency.securityType()) + ' ' + (m_toCurrency.isCurrency() ? m_toCurrency.id() : m_toCurrency.tradingSymbol())));

    //set bold font
    auto boldFont = ui->m_fromCurrencyText->font();
    boldFont.setBold(true);
    ui->m_fromCurrencyText->setFont(boldFont);
    boldFont = ui->m_toCurrencyText->font();
    boldFont.setBold(true);
    ui->m_toCurrencyText->setFont(boldFont);

    ui->m_fromAmount->setText(m_value.formatMoney(QString(), MyMoneyMoney::denomToPrec(m_fromCurrency.smallestAccountFraction())));

    ui->m_dateText->setText(QLocale().toString(m_date));

    ui->m_updateButton->setChecked(KMyMoneyGlobalSettings::priceHistoryUpdate());

    // setup initial result
    if (m_result == MyMoneyMoney() && !m_value.isZero()) {
      const MyMoneyPrice &pr = file->price(m_fromCurrency.id(), m_toCurrency.id(), m_date);
      if (pr.isValid()) {
        m_result = m_value * pr.rate(m_toCurrency.id());
      }
    }

    // fill in initial values
    ui->m_toAmount->loadText(m_result.formatMoney(QString(), MyMoneyMoney::denomToPrec(m_resultFraction)));
    ui->m_toAmount->setPrecision(MyMoneyMoney::denomToPrec(m_resultFraction));

    ui->m_conversionRate->setPrecision(m_fromCurrency.pricePrecision());

    q->connect(ui->m_amountButton, &QAbstractButton::clicked, q, &KCurrencyCalculator::slotSetToAmount);
    q->connect(ui->m_rateButton, &QAbstractButton::clicked, q, &KCurrencyCalculator::slotSetExchangeRate);

    q->connect(ui->m_toAmount, &KMyMoneyEdit::valueChanged, q, &KCurrencyCalculator::slotUpdateResult);
    q->connect(ui->m_conversionRate, &KMyMoneyEdit::valueChanged, q, &KCurrencyCalculator::slotUpdateRate);

    // use this as the default
    ui->m_amountButton->animateClick();
    q->slotUpdateResult(ui->m_toAmount->text());

    // If the from security is not a currency, we only allow entering a price
    if (!m_fromCurrency.isCurrency()) {
      ui->m_rateButton->animateClick();
      ui->m_amountButton->hide();
      ui->m_toAmount->hide();
    }
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

  KCurrencyCalculator     *q_ptr;
  Ui::KCurrencyCalculator *ui;
  MyMoneySecurity          m_fromCurrency;
  MyMoneySecurity          m_toCurrency;
  MyMoneyMoney             m_result;
  MyMoneyMoney             m_value;
  QDate                    m_date;
  signed64                 m_resultFraction;
};

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
                                fract,
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
  MyMoneyMoney price(0, 1);

  if (result.isNegative()) {
    d->ui->m_toAmount->setValue(-result);
    slotUpdateResult(QString());
    return;
  }

  if (!result.isZero()) {
    price = result / d->m_value;

    d->ui->m_conversionRate->loadText(price.formatMoney(QString(), d->m_fromCurrency.pricePrecision()));
    d->m_result = (d->m_value * price).convert(d->m_resultFraction);
    d->ui->m_toAmount->loadText(d->m_result.formatMoney(d->m_resultFraction));
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
    d->ui->m_conversionRate->loadText(price.formatMoney(QString(), d->m_fromCurrency.pricePrecision()));
    d->m_result = (d->m_value * price).convert(d->m_resultFraction);
    d->ui->m_toAmount->loadText(d->m_result.formatMoney(QString(), MyMoneyMoney::denomToPrec(d->m_resultFraction)));
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
    if (!pr.isValid()
        || pr.date() != d->ui->m_dateEdit->date()
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
  KMyMoneyGlobalSettings::setPriceHistoryUpdate(d->ui->m_updateButton->isChecked());
  QDialog::accept();
}

MyMoneyMoney KCurrencyCalculator::price() const
{
  Q_D(const KCurrencyCalculator);
  // This should fix https://bugs.kde.org/show_bug.cgi?id=205254 and
  // https://bugs.kde.org/show_bug.cgi?id=325953 as well as
  // https://bugs.kde.org/show_bug.cgi?id=300965
  if (d->ui->m_amountButton->isChecked())
    return d->ui->m_toAmount->value().abs() / d->m_value.abs();
  else
    return d->ui->m_conversionRate->value();
}
