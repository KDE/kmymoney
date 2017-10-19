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

#include <QDateTime>
#include <QPushButton>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kupdatestockpricedlgdecl.h"

#include "mymoneyprice.h"
#include "kmymoneydateinput.h"
#include "kmymoneyedit.h"

/**
  * @author Kevin Tambascio
  */
class kUpdateStockPriceDecl : public QDialog, public Ui::kUpdateStockPriceDecl
{
public:
  kUpdateStockPriceDecl(QWidget *parent) : QDialog(parent) {
    setupUi(this);
  }
};

class KUpdateStockPriceDlg : public kUpdateStockPriceDecl
{
  Q_OBJECT

public:
  KUpdateStockPriceDlg(QWidget* parent = 0);
  ~KUpdateStockPriceDlg();

  const QDate date() const {
    return m_date->date();
  };
  const MyMoneyMoney price() const;

public slots:
  int exec();

protected slots:
  void slotCheckData();

private:
  void init();

};

#endif
