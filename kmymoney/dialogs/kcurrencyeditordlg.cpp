/*
 * Copyright 2017       Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 * Copyright 2020       Thomas Baumgart <tbaumgart@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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

  connect(d->ui->leIsoCode, &QLineEdit::textChanged, this, [&]() { Q_D(KCurrencyEditorDlg); d->validateValues(); });
  connect(d->ui->leName, &QLineEdit::textChanged, this, [&]() { Q_D(KCurrencyEditorDlg); d->validateValues(); });
  connect(d->ui->leSymbol, &QLineEdit::textChanged, this, [&]() { Q_D(KCurrencyEditorDlg); d->validateValues(); });
  connect(d->ui->leCashFraction, &QLineEdit::textChanged, this, [&]() { Q_D(KCurrencyEditorDlg); d->validateValues(); });
  connect(d->ui->leAccountFraction, &QLineEdit::textChanged, this, [&]() { Q_D(KCurrencyEditorDlg); d->validateValues(); });

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
  Q_D(const KCurrencyEditorDlg);
  MyMoneySecurity newCurrency = d->currency;

  // if we are creating a new currency, we need to assing the ID
  if (d->currency.id().isEmpty()) {
    newCurrency = MyMoneySecurity(d->ui->leIsoCode->text(), newCurrency);
  }

  newCurrency.setName(d->ui->leName->text());
  newCurrency.setTradingSymbol(d->ui->leSymbol->text());

  MyMoneyMoney value(d->ui->leCashFraction->text());
  int fraction = static_cast<int>((MyMoneyMoney::ONE / value).toDouble());
  newCurrency.setSmallestCashFraction(fraction);

  value = MyMoneyMoney(d->ui->leAccountFraction->text());
  fraction = static_cast<int>((MyMoneyMoney::ONE / value).toDouble());
  newCurrency.setSmallestAccountFraction(fraction);

  newCurrency.setRoundingMethod(static_cast<AlkValue::RoundingMethod>(d->ui->comboRoundingMethod->currentIndex()));
  newCurrency.setPricePrecision(d->ui->spbPricePrecision->value());
  return newCurrency;
}

