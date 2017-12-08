/***************************************************************************
                           currenciesdlg.h
                         --------------------
begin                 : Sat Jan 15 2017
copyright             : (C) 2017 by Łukasz Wojnilowicz
email                 : lukasz.wojnilowicz@gmail.com
****************************************************************************/

/***************************************************************************

*   This program is free software; you can redistribute it and/or modify   *
*   it under the terms of the GNU General Public License as published      *
*   by the Free Software Foundation; either version 2 of the License,      *
*   or  (at your option) any later version.                                *
*                                                                          *
****************************************************************************/

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
