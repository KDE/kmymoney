/***************************************************************************
                          kcurrencyeditordlg.h  -  description
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

#ifndef KCURRENCYEDITORDLG_H
#define KCURRENCYEDITORDLG_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <QDialog>

// ----------------------------------------------------------------------------
// Project Includes

namespace Ui
{
class KCurrencyEditorDlg;
}

class MyMoneySecurity;
class KCurrencyEditorDlg : public QDialog
{
  Q_OBJECT
public:
  KCurrencyEditorDlg(MyMoneySecurity &currency, QWidget *parent = nullptr);
  ~KCurrencyEditorDlg();

  Ui::KCurrencyEditorDlg*   ui;

protected slots:
  void loadCurrency(MyMoneySecurity& currency);

};

#endif
