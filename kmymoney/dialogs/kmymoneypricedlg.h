/*
    SPDX-FileCopyrightText: 2004-2017 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
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
