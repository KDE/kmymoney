/***************************************************************************
                          kmymoneycurrencyselector.cpp  -  description
                             -------------------
    begin                : Tue Apr 6 2004
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

#include "kmymoneycurrencyselector.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QPixmap>
#include <QBitmap>

// ----------------------------------------------------------------------------
// KDE Includes

//#include <kstandarddirs.h>
#include <KIcon>

// ----------------------------------------------------------------------------
// Project Includes

KMyMoneySecuritySelector::KMyMoneySecuritySelector(QWidget *parent) :
    KComboBox(parent),
    m_displayItem(FullName),
    m_displayOnly(false),
    m_displayType(TypeAll)
{
  // update(QString());
}

KMyMoneySecuritySelector::~KMyMoneySecuritySelector()
{
}

void KMyMoneySecuritySelector::selectDisplayItem(KMyMoneySecuritySelector::displayItemE item)
{
  m_displayItem = item;
  update(QString());
}

void KMyMoneySecuritySelector::setDisplayType(displayTypeE type)
{
  m_displayType = type;
}

void KMyMoneySecuritySelector::update(const QString& id)
{
  MyMoneySecurity curr = MyMoneyFile::instance()->baseCurrency();
  QString baseCurrency = curr.id();

  if (!id.isEmpty())
    curr = m_currency;

  this->clear();
  m_list.clear();
  if (m_displayType & TypeCurrencies)
    m_list += MyMoneyFile::instance()->currencyList();
  if (m_displayType & TypeSecurities)
    m_list += MyMoneyFile::instance()->securityList();

  // sort
  qSort(m_list);

  QList<MyMoneySecurity>::ConstIterator it;

  // construct a transparent 16x16 pixmap
  QPixmap empty(16, 16);
  QBitmap mask(16, 16);
  mask.clear();
  empty.fill(palette().color(QPalette::Base));
  empty.setMask(mask);

  int itemId = 0;
  int m_selectedItemId = 0;
  for (it = m_list.constBegin(); it != m_list.constEnd(); ++it) {
    QString display;
    switch (m_displayItem) {
      default:
      case FullName:
        if ((*it).isCurrency()) {
          display = QString("%2 (%1)").arg((*it).id()).arg((*it).name());
        } else
          display = QString("%2 (%1)").arg((*it).tradingSymbol()).arg((*it).name());
        break;
        break;

      case Symbol:
        if ((*it).isCurrency())
          display = (*it).id();
        else
          display = (*it).tradingSymbol();
        break;
    }
    if ((*it).id() == baseCurrency) {
      insertItem(itemId, KIcon("account"), display); //krazy:exclude=iconnames
    } else {
      insertItem(itemId, QIcon(empty), display);
    }

    if (curr.id() == (*it).id()) {
      m_selectedItemId = itemId;
      m_currency = (*it);
    }

    itemId++;
  }
  setCurrentIndex(m_selectedItemId);
}

void KMyMoneySecuritySelector::setDisplayOnly(const bool disp)
{
  if (disp == m_displayOnly)
    return;

  switch (disp) {
    case true:
      connect(this, SIGNAL(activated(int)), this, SLOT(slotSetInitialCurrency()));
      break;
    case false:
      disconnect(this, SIGNAL(activated(int)), this, SLOT(slotSetInitialCurrency()));
      break;
  }
  m_displayOnly = disp;
}

void KMyMoneySecuritySelector::slotSetInitialSecurity(void)
{
  setCurrentIndex(m_selectedItemId);
}

const MyMoneySecurity& KMyMoneySecuritySelector::security(void) const
{
  int index = currentIndex();
  if ((0 <= index) && (index < m_list.size()))
    return m_list[index];
  else
    return m_currency;
}

void KMyMoneySecuritySelector::setSecurity(const MyMoneySecurity& currency)
{
  m_currency = currency;
  update(QString("x"));
}

KMyMoneyCurrencySelector::KMyMoneyCurrencySelector(QWidget *parent) :
    KMyMoneySecuritySelector(parent)
{
  setDisplayType(TypeCurrencies);
}

#include "kmymoneycurrencyselector.moc"
