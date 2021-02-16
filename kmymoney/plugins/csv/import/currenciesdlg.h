/*
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CURRENCIESDLG_H
#define CURRENCIESDLG_H

#include <QDialog>

namespace Ui
{
class CurrenciesDlg;
}

class CurrenciesDlg : public QDialog
{
  Q_OBJECT

public:
  CurrenciesDlg();
  ~CurrenciesDlg();

  Ui::CurrenciesDlg*   ui;

  /**
  * This method initializes currencies comboboxes.
  */
  void             initializeCurrencies(const QString &presetFromCurrency, const QString &presetToCurrency);

  QString          fromCurrency();
  QString          toCurrency();
  int              dontAsk();

private:
  QPushButton*     m_buttonOK;

private Q_SLOTS:
  void             slotIndexChanged(int index);
};

#endif // CURRENCIESDLG_H
