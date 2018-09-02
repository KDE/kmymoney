/*
 * Copyright 2017       Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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
  explicit KCurrencyEditorDlg(MyMoneySecurity &currency, QWidget *parent = nullptr);
  ~KCurrencyEditorDlg();

  Ui::KCurrencyEditorDlg*   ui;

protected Q_SLOTS:
  void loadCurrency(MyMoneySecurity& currency);

};

#endif
