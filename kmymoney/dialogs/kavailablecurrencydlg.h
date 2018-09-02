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

#ifndef KAVAILABLECURRENCYDLG_H
#define KAVAILABLECURRENCYDLG_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <QDialog>

// ----------------------------------------------------------------------------
// Project Includes

namespace Ui
{
class KAvailableCurrencyDlg;
}

class KTreeWidgetSearchLineWidget;
class KAvailableCurrencyDlg : public QDialog
{
  Q_OBJECT
public:
  explicit KAvailableCurrencyDlg(QWidget *parent = nullptr);
  ~KAvailableCurrencyDlg();

  Ui::KAvailableCurrencyDlg*   ui;

protected Q_SLOTS:
  void slotLoadCurrencies();
  void slotItemSelectionChanged();

private:
  KTreeWidgetSearchLineWidget*  m_searchWidget;
};

#endif
