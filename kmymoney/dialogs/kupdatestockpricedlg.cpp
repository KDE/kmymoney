/***************************************************************************
                          kupdatestockpricedlg.cpp  -  description
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

#include "kupdatestockpricedlg.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDate>
#include <QPushButton>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kupdatestockpricedlg.h"

#include "kmymoneydateinput.h"
#include "kmymoneycurrencyselector.h"

KUpdateStockPriceDlg::KUpdateStockPriceDlg(QWidget* parent) :
  QDialog(parent),
  ui(new Ui::KUpdateStockPriceDlg)
{
  ui->setupUi(this);
  setModal(true);
  ui->m_date->setDate(QDate::currentDate());

  connect(ui->m_security, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, static_cast<void (KUpdateStockPriceDlg::*)(int)>(&KUpdateStockPriceDlg::slotCheckData));
  connect(ui->m_currency, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, static_cast<void (KUpdateStockPriceDlg::*)(int)>(&KUpdateStockPriceDlg::slotCheckData));

  // load initial values into the selection widgets
  ui->m_currency->update(QString());
  ui->m_security->update(QString());

  slotCheckData();
}

KUpdateStockPriceDlg::~KUpdateStockPriceDlg()
{
  delete  ui;
}

int KUpdateStockPriceDlg::exec()
{
  slotCheckData();
  return QDialog::exec();
}

QDate KUpdateStockPriceDlg::date() const
{
  return ui->m_date->date();
}

void KUpdateStockPriceDlg::slotCheckData()
{
  auto from = ui->m_security->security().id();
  auto to   = ui->m_currency->security().id();

  ui->m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!from.isEmpty() && !to.isEmpty() && from != to);
}

void KUpdateStockPriceDlg::slotCheckData(int)
{
  slotCheckData();
}
