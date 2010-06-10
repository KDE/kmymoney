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

class Q3ListViewItem;

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kmymoneypricedlgdecl.h"
#include "mymoneyprice.h"


class KMyMoneyPriceDlgDecl : public KDialog, public Ui::KMyMoneyPriceDlgDecl
{
public:
  KMyMoneyPriceDlgDecl(QWidget *parent) : KDialog(parent) {
    setupUi(this);
  }
};

class KMyMoneyPriceDlg : public KMyMoneyPriceDlgDecl
{
  Q_OBJECT
public:
  KMyMoneyPriceDlg(QWidget* parent);
  ~KMyMoneyPriceDlg();

protected slots:
  void slotSelectPrice(Q3ListViewItem* item);
  void slotNewPrice(void);
  void slotDeletePrice(void);
  int slotEditPrice(void);
  void slotLoadWidgets(void);
  void slotOnlinePriceUpdate(void);

signals:
  void openContextMenu(const MyMoneyPrice& price);
  void selectObject(const MyMoneyPrice& price);

private:
  Q3ListViewItem*    m_currentItem;
};

#endif // KMYMONEYPRICEDLG_H
