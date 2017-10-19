/***************************************************************************
                          kcurrencyeditordlg.cpp  -  description
                             -------------------
    begin                : Sat Apr 09 2017
    copyright            : (C) 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kcurrencyeditordlg.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kcurrencyeditordlg.h"
#include "mymoneymoney.h"
#include "mymoneysecurity.h"

KCurrencyEditorDlg::KCurrencyEditorDlg(MyMoneySecurity& currency, QWidget *parent) : ui(new Ui::KCurrencyEditorDlg)
{
  Q_UNUSED(parent);
  ui->setupUi(this);
  loadCurrency(currency);
}

KCurrencyEditorDlg::~KCurrencyEditorDlg()
{
  delete ui;
}

void KCurrencyEditorDlg::loadCurrency(MyMoneySecurity& currency)
{
  ui->leName->setText(currency.name());
  ui->leSymbol->setText(currency.tradingSymbol());
  int precision = MyMoneyMoney::denomToPrec(currency.smallestCashFraction());
  MyMoneyMoney smallestFraction = MyMoneyMoney::ONE / MyMoneyMoney(currency.smallestCashFraction());
  ui->leCashFraction->setText(smallestFraction.formatMoney(currency.tradingSymbol(), precision));
  precision = MyMoneyMoney::denomToPrec(currency.smallestAccountFraction());
  smallestFraction = MyMoneyMoney::ONE / MyMoneyMoney(currency.smallestAccountFraction());
  ui->leAccountFraction->setText(smallestFraction.formatMoney(currency.tradingSymbol(), precision));
  ui->leRoundingMethod->setText(currency.roundingMethodToString(currency.roundingMethod()));
  ui->m_pricePrecision->setValue(currency.pricePrecision());
}
