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
#include <QVBoxLayout>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KConfigGroup>
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoneyedit.h>
#include <kmymoneydateinput.h>
#include <kmymoneycurrencyselector.h>
#include <mymoneyprice.h>
#include <mymoneytransaction.h>
#include <kmymoneyglobalsettings.h>

#include "kmymoneyutils.h"

bool KCurrencyCalculator::setupSplitPrice(MyMoneyMoney& shares, const MyMoneyTransaction& t, const MyMoneySplit& s, const QMap<QString, MyMoneyMoney>& priceInfo, QWidget* parentWidget)
{
  bool rc = true;
  MyMoneyFile* file = MyMoneyFile::instance();

  if (!s.value().isZero()) {
    MyMoneyAccount cat = file->account(s.accountId());
    MyMoneySecurity toCurrency;
    toCurrency = file->security(cat.currencyId());
    // determine the fraction required for this category/account
    int fract = cat.fraction(toCurrency);

    if (cat.currencyId() != t.commodity()) {

      MyMoneySecurity fromCurrency;
      MyMoneyMoney fromValue, toValue;
      fromCurrency = file->security(t.commodity());
      // display only positive values to the user
      fromValue = s.value().abs();

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

KCurrencyCalculator::KCurrencyCalculator(const MyMoneySecurity& from, const MyMoneySecurity& to, const MyMoneyMoney& value, const MyMoneyMoney& shares, const QDate& date, const signed64 resultFraction, QWidget *parent) :
    KCurrencyCalculatorDecl(parent),
    m_fromCurrency(from),
    m_toCurrency(to),
    m_result(shares.abs()),
    m_value(value.abs()),
    m_resultFraction(resultFraction),
    m_buttonBox(0)
{
  MyMoneyFile* file = MyMoneyFile::instance();

  //set main widget of QDialog
  QVBoxLayout *mainLayout = new QVBoxLayout;
  setLayout(mainLayout);
  mainLayout->addWidget(m_layoutWidget);

  m_buttonBox = new QDialogButtonBox(this);
  m_buttonBox->setStandardButtons(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
  QPushButton *okButton = m_buttonBox->button(QDialogButtonBox::Ok);
  okButton->setDefault(true);
  /// @todo remove Ctrl-Enter behavior in future release
  okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
  connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect(m_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  mainLayout->addWidget(m_buttonBox);

  buttonGroup1->setId(m_amountButton, 0);
  buttonGroup1->setId(m_rateButton, 1);

  m_dateFrame->hide();
  if (date.isValid())
    m_dateEdit->setDate(date);
  else
    m_dateEdit->setDate(QDate::currentDate());

  m_fromCurrencyText->setText(QString(KMyMoneyUtils::securityTypeToString(m_fromCurrency.securityType()) + ' ' + (m_fromCurrency.isCurrency() ? m_fromCurrency.id() : m_fromCurrency.tradingSymbol())));
  m_toCurrencyText->setText(QString(KMyMoneyUtils::securityTypeToString(m_toCurrency.securityType()) + ' ' + (m_toCurrency.isCurrency() ? m_toCurrency.id() : m_toCurrency.tradingSymbol())));

  //set bold font
  QFont boldFont = m_fromCurrencyText->font();
  boldFont.setBold(true);
  m_fromCurrencyText->setFont(boldFont);
  boldFont = m_toCurrencyText->font();
  boldFont.setBold(true);
  m_toCurrencyText->setFont(boldFont);

  m_fromAmount->setText(m_value.formatMoney(QString(), MyMoneyMoney::denomToPrec(m_fromCurrency.smallestAccountFraction())));

  m_dateText->setText(QLocale().toString(date));

  m_updateButton->setChecked(KMyMoneyGlobalSettings::priceHistoryUpdate());

  // setup initial result
  if (m_result == MyMoneyMoney() && !m_value.isZero()) {
    const MyMoneyPrice &pr = file->price(m_fromCurrency.id(), m_toCurrency.id(), date);
    if (pr.isValid()) {
      m_result = m_value * pr.rate(m_toCurrency.id());
    }
  }

  // fill in initial values
  m_toAmount->loadText(m_result.formatMoney(QString(), MyMoneyMoney::denomToPrec(m_resultFraction)));
  m_toAmount->setPrecision(MyMoneyMoney::denomToPrec(m_resultFraction));

  m_conversionRate->setPrecision(m_fromCurrency.pricePrecision());

  connect(m_amountButton, SIGNAL(clicked()), this, SLOT(slotSetToAmount()));
  connect(m_rateButton, SIGNAL(clicked()), this, SLOT(slotSetExchangeRate()));

  connect(m_toAmount, SIGNAL(valueChanged(QString)), this, SLOT(slotUpdateResult(QString)));
  connect(m_conversionRate, SIGNAL(valueChanged(QString)), this, SLOT(slotUpdateRate(QString)));

  // use this as the default
  m_amountButton->animateClick();
  slotUpdateResult(m_toAmount->text());

  // If the from security is not a currency, we only allow entering a price
  if (!m_fromCurrency.isCurrency()) {
    m_rateButton->animateClick();
    m_amountButton->hide();
    m_toAmount->hide();
  }
  okButton->setFocus();
}

KCurrencyCalculator::~KCurrencyCalculator()
{
}

void KCurrencyCalculator::setupPriceEditor()
{
  m_dateFrame->show();
  m_amountDateFrame->hide();
  m_updateButton->setChecked(true);
  m_updateButton->hide();
}

void KCurrencyCalculator::slotSetToAmount()
{
  m_rateButton->setChecked(false);
  m_toAmount->setEnabled(true);
  m_conversionRate->setEnabled(false);
}

void KCurrencyCalculator::slotSetExchangeRate()
{
  m_amountButton->setChecked(false);
  m_toAmount->setEnabled(false);
  m_conversionRate->setEnabled(true);
}

void KCurrencyCalculator::slotUpdateResult(const QString& /*txt*/)
{
  MyMoneyMoney result = m_toAmount->value();
  MyMoneyMoney price(0, 1);

  if (result.isNegative()) {
    m_toAmount->setValue(-result);
    slotUpdateResult(QString());
    return;
  }

  if (!result.isZero()) {
    price = result / m_value;

    m_conversionRate->loadText(price.formatMoney(QString(), m_fromCurrency.pricePrecision()));
    m_result = (m_value * price).convert(m_resultFraction);
    m_toAmount->loadText(m_result.formatMoney(m_resultFraction));
  }
  updateExample(price);
}

void KCurrencyCalculator::slotUpdateRate(const QString& /*txt*/)
{
  MyMoneyMoney price = m_conversionRate->value();

  if (price.isNegative()) {
    m_conversionRate->setValue(-price);
    slotUpdateRate(QString());
    return;
  }

  if (!price.isZero()) {
    m_conversionRate->loadText(price.formatMoney(QString(), m_fromCurrency.pricePrecision()));
    m_result = (m_value * price).convert(m_resultFraction);
    m_toAmount->loadText(m_result.formatMoney(QString(), MyMoneyMoney::denomToPrec(m_resultFraction)));
  }
  updateExample(price);
}

void KCurrencyCalculator::updateExample(const MyMoneyMoney& price)
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
  m_conversionExample->setText(msg);
  m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!price.isZero());
}

void KCurrencyCalculator::accept()
{
  if (m_conversionRate->isEnabled())
    slotUpdateRate(QString());
  else
    slotUpdateResult(QString());

  if (m_updateButton->isChecked()) {
    MyMoneyPrice pr = MyMoneyFile::instance()->price(m_fromCurrency.id(), m_toCurrency.id(), m_dateEdit->date());
    if (!pr.isValid()
        || pr.date() != m_dateEdit->date()
        || (pr.date() == m_dateEdit->date() && pr.rate(m_fromCurrency.id()) != price())) {
      pr = MyMoneyPrice(m_fromCurrency.id(), m_toCurrency.id(), m_dateEdit->date(), price(), i18n("User"));
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
  KMyMoneyGlobalSettings::setPriceHistoryUpdate(m_updateButton->isChecked());

  KCurrencyCalculatorDecl::accept();
}

MyMoneyMoney KCurrencyCalculator::price() const
{
  // This should fix https://bugs.kde.org/show_bug.cgi?id=205254 and
  // https://bugs.kde.org/show_bug.cgi?id=325953 as well as
  // https://bugs.kde.org/show_bug.cgi?id=300965
  if (m_amountButton->isChecked())
    return m_toAmount->value().abs() / m_value.abs();
  else
    return m_conversionRate->value();
}
