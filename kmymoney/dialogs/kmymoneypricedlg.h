/*
 * Copyright 2004-2017  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2017-2018  Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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

#ifndef KMYMONEYPRICEDLG_H
#define KMYMONEYPRICEDLG_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QDialog>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class MyMoneyPrice;
class QTreeWidgetItem;

class KMyMoneyPriceDlgPrivate;
class KMyMoneyPriceDlg : public QDialog
{
  Q_OBJECT
  Q_DISABLE_COPY(KMyMoneyPriceDlg)

public:
  explicit KMyMoneyPriceDlg(QWidget* parent);
  ~KMyMoneyPriceDlg();

private:
  QTreeWidgetItem* loadPriceItem(const MyMoneyPrice& basePrice);

protected Q_SLOTS:
  void slotSelectPrice();
  void slotNewPrice();
  void slotDeletePrice();
  void slotEditPrice();
  void slotLoadWidgets();
  void slotOnlinePriceUpdate();
  void slotShowPriceMenu(const QPoint& p);

private:
  KMyMoneyPriceDlgPrivate * const d_ptr;
  Q_DECLARE_PRIVATE(KMyMoneyPriceDlg)
};

#endif // KMYMONEYPRICEDLG_H
