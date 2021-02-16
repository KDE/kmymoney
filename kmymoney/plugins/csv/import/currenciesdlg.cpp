/*
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "currenciesdlg.h"

#include <QPushButton>

#include "ui_currenciesdlg.h"
#include "mymoneyfile.h"
#include "mymoneysecurity.h"

CurrenciesDlg::CurrenciesDlg() : ui(new Ui::CurrenciesDlg)
{
  ui->setupUi(this);
  m_buttonOK = ui->buttonBox->button(QDialogButtonBox::Ok);
  m_buttonOK->setDefault(true);
  m_buttonOK->setShortcut(Qt::CTRL | Qt::Key_Return);
  m_buttonOK->setEnabled(false);

  connect(ui->cbFrom,  SIGNAL(currentIndexChanged(int)), this,  SLOT(slotIndexChanged(int)));
  connect(ui->cbTo,  SIGNAL(currentIndexChanged(int)), this,  SLOT(slotIndexChanged(int)));
}

CurrenciesDlg::~CurrenciesDlg()
{
  delete ui;
}

void CurrenciesDlg::initializeCurrencies(const QString &presetFromCurrency, const QString &presetToCurrency)
{
  QList<MyMoneySecurity> currencies = MyMoneyFile::instance()->currencyList();

  ui->cbFrom->blockSignals(true);
  ui->cbTo->blockSignals(true);
  int presetFromIndex = -1;
  int presetToIndex = -1;
  for (QList<MyMoneySecurity>::const_iterator currency = currencies.cbegin(); currency != currencies.cend(); ++currency) {
    QString name = (*currency).name();
    QString id = (*currency).id();
    QString symbol = (*currency).tradingSymbol();
    if (id == presetFromCurrency)
      presetFromIndex = ui->cbFrom->count();
    if (id == presetToCurrency)
      presetToIndex = ui->cbTo->count();
    ui->cbFrom->addItem(name + QChar(' ') + QChar('(') + symbol + QChar(')'), QVariant(id));
    ui->cbTo->addItem(name + QChar(' ') + QChar('(') + symbol + QChar(')'), QVariant(id));
  }
  ui->cbFrom->blockSignals(false);
  ui->cbTo->blockSignals(false);
  ui->cbFrom->setCurrentIndex(presetFromIndex);
  ui->cbTo->setCurrentIndex(presetToIndex);
  emit ui->cbFrom->currentIndexChanged(presetFromIndex); // in case currentIndex == presetIndex and no signal would be emitted
}

QString CurrenciesDlg::fromCurrency() {
  return ui->cbFrom->currentData().toString();
}

QString CurrenciesDlg::toCurrency() {
  return ui->cbTo->currentData().toString();
}

int CurrenciesDlg::dontAsk() {
  return int(ui->cbDontAsk->isChecked());
}

void CurrenciesDlg::slotIndexChanged(int index)
{
  Q_UNUSED(index);
  if (ui->cbFrom->currentIndex() != ui->cbTo->currentIndex() &&
      ui->cbFrom->currentIndex() != -1 && ui->cbTo->currentIndex() != -1)
    m_buttonOK->setEnabled(true);
  else
    m_buttonOK->setEnabled(false);
}
