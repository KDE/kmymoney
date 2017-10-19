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


// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kmymoneypricedlgdecl.h"

class MyMoneyPrice;
class KTreeWidgetSearchLineWidget;
class QTreeWidgetItem;
class KMyMoneyPriceDlgDecl : public QDialog, public Ui::KMyMoneyPriceDlgDecl
{
public:
  KMyMoneyPriceDlgDecl(QWidget *parent) : QDialog(parent) {
    setupUi(this);
  }
};

class KMyMoneyPriceDlg : public KMyMoneyPriceDlgDecl
{
  Q_OBJECT
public:
  KMyMoneyPriceDlg(QWidget* parent);
  ~KMyMoneyPriceDlg();

private:
  QTreeWidgetItem* loadPriceItem(const MyMoneyPrice& basePrice);

protected slots:
  void slotSelectPrice();
  void slotNewPrice();
  void slotDeletePrice();
  int slotEditPrice();
  void slotLoadWidgets();
  void slotOnlinePriceUpdate();
  void slotOpenContextMenu(const QPoint& p);

signals:
  void openContextMenu(const MyMoneyPrice& price);
  void selectObject(const MyMoneyPrice& price);

private:
  QTreeWidgetItem*              m_currentItem;
  /**
    * Search widget for the list
    */
  KTreeWidgetSearchLineWidget*  m_searchWidget;
  QMap<QString, QString>        m_stockNameMap;
};

#endif // KMYMONEYPRICEDLG_H
