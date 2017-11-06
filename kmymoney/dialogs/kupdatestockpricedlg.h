/***************************************************************************
                          kupdatestockpricedlg.h  -  description
                             -------------------
    begin                : Thu Feb 7 2002
    copyright            : (C) 2000-2002 by Michael Edwardes
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


#ifndef KUPDATESTOCKPRICEDLG_H
#define KUPDATESTOCKPRICEDLG_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QDialog>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

namespace Ui { class KUpdateStockPriceDlg; }

class QDate;

class MyMoneyMoney;

/**
  * @author Kevin Tambascio
  */

class KUpdateStockPriceDlg : public QDialog
{
  Q_OBJECT
  Q_DISABLE_COPY(KUpdateStockPriceDlg)

public:
  explicit KUpdateStockPriceDlg(QWidget* parent = nullptr);
  ~KUpdateStockPriceDlg();

  QDate date() const;
  MyMoneyMoney price() const;

  Ui::KUpdateStockPriceDlg *ui;

public slots:
  int exec() override;

protected slots:
  void slotCheckData();
  void slotCheckData(int idx);
};

#endif
