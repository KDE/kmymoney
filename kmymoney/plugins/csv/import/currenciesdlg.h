/*
 * Copyright 2017  Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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
