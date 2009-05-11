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
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <kguiitem.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kupdatestockpricedlg.h"
#include "../widgets/kmymoneycurrencyselector.h"

KUpdateStockPriceDlg::KUpdateStockPriceDlg(QWidget* parent,  const char* name) :
  kUpdateStockPriceDecl(parent, name, true)
{
  m_date->setDate(QDate::currentDate());
  init();
}

KUpdateStockPriceDlg::~KUpdateStockPriceDlg()
{
}

void KUpdateStockPriceDlg::init()
{
  m_okButton->setGuiItem(KStandardGuiItem::ok());
  m_cancelButton->setGuiItem(KStandardGuiItem::cancel());

  connect(m_okButton, SIGNAL(clicked()), this, SLOT(accept()));
  connect(m_cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

  connect(m_security, SIGNAL(activated(int)), this, SLOT(slotCheckData()));
  connect(m_currency, SIGNAL(activated(int)), this, SLOT(slotCheckData()));

  // load initial values into the selection widgets
  m_currency->update(QString());
  m_security->update(QString());

  slotCheckData();
}

int KUpdateStockPriceDlg::exec(void)
{
  slotCheckData();
  return kUpdateStockPriceDecl::exec();
}

void KUpdateStockPriceDlg::slotCheckData(void)
{
  QString from = m_security->security().id();
  QString to   = m_currency->security().id();

  m_okButton->setEnabled(!from.isEmpty() && !to.isEmpty() && from != to);
}








































#include "kupdatestockpricedlg.moc"
