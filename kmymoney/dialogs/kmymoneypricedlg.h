/***************************************************************************
                          kmymoneypricedlg.h
                             -------------------
    begin                : Wed Nov 24 2004
    copyright            : (C) 2000-2004 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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
  int slotEditPrice();
  void slotLoadWidgets();
  void slotOnlinePriceUpdate();
  void slotOpenContextMenu(const QPoint& p);

Q_SIGNALS:
  void openContextMenu(const MyMoneyPrice& price);
  void selectObject(const MyMoneyPrice& price);

private:
  KMyMoneyPriceDlgPrivate * const d_ptr;
  Q_DECLARE_PRIVATE(KMyMoneyPriceDlg)
};

#endif // KMYMONEYPRICEDLG_H
