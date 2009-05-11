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

class QListViewItem;

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneypricedlgdecl.h"
#include "kmymoney/mymoneyprice.h"

class KMyMoneyPriceDlg : public KMyMoneyPriceDlgDecl
{
  Q_OBJECT
public:
  KMyMoneyPriceDlg(QWidget* parent, const char *name);
  ~KMyMoneyPriceDlg();

protected slots:
  void slotSelectPrice(QListViewItem* item);
  void slotNewPrice(void);
  void slotDeletePrice(void);
  int slotEditPrice(void);
  void slotLoadWidgets(void);
  void slotOnlinePriceUpdate(void);

private:
  QListViewItem*    m_currentItem;
};

#endif // KMYMONEYPRICEDLG_H
