/***************************************************************************
                             userinfo.cpp
                             -------------------
    begin                : Fri Jun  1 2007
    copyright            : (C) 2007 Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
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

#include <klocale.h>
#include <k3listview.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "currency.h"

Currency::Currency(QWidget* parent, const char* name) :
  CurrencyDecl(parent, name)
{
  m_currencyList->setAllColumnsShowFocus(true);
  m_currencyList->setMultiSelection(false);
}

Q3ListViewItem* Currency::insertCurrency(const MyMoneySecurity& sec)
{
  return new K3ListViewItem(m_currencyList, sec.name(), QString(sec.id()), sec.tradingSymbol());
}

void Currency::selectCurrency(const MyMoneySecurity& sec)
{
  Q3ListViewItem* it_v;
  Q3ListViewItemIterator it(m_currencyList);
  while((it_v = it.current()) != 0) {
    if(it_v->text(1) == QString(sec.id())) {
      m_currencyList->setSelected(it_v, true);
      m_currencyList->ensureItemVisible(it_v);
      break;
    }
  }
}

QString Currency::selectedCurrency(void) const
{
  QString id;
  if(m_currencyList->selectedItem()) {
    id = m_currencyList->selectedItem()->text(1);
  }
  return id;
}

#include "currency.moc"
