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
#include <QIcon>

// ----------------------------------------------------------------------------
// KDE Includes


// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyfile.h"
#include "icons/icons.h"

using namespace Icons;

KMyMoneySecuritySelector::KMyMoneySecuritySelector(QWidget *parent) :
    KComboBox(parent),
    m_displayItem(FullName),
    m_selectedItemId(0),
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
  static unsigned char empty_png[] = {
    0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A,
    0x00, 0x00, 0x00, 0x0D, 0x49, 0x48, 0x44, 0x52,
    0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x10,
    0x08, 0x06, 0x00, 0x00, 0x00, 0x1F, 0xF3, 0xFF,
    0x61, 0x00, 0x00, 0x00, 0x01, 0x73, 0x52, 0x47,
    0x42, 0x00, 0xAE, 0xCE, 0x1C, 0xE9, 0x00, 0x00,
    0x00, 0x06, 0x62, 0x4B, 0x47, 0x44, 0x00, 0xFF,
    0x00, 0xFF, 0x00, 0xFF, 0xA0, 0xBD, 0xA7, 0x93,
    0x00, 0x00, 0x00, 0x09, 0x70, 0x48, 0x59, 0x73,
    0x00, 0x00, 0x0B, 0x13, 0x00, 0x00, 0x0B, 0x13,
    0x01, 0x00, 0x9A, 0x9C, 0x18, 0x00, 0x00, 0x00,
    0x07, 0x74, 0x49, 0x4D, 0x45, 0x07, 0xDB, 0x07,
    0x08, 0x0B, 0x16, 0x09, 0xAA, 0xA8, 0x50, 0x21,
    0x00, 0x00, 0x00, 0x12, 0x49, 0x44, 0x41, 0x54,
    0x38, 0xCB, 0x63, 0x60, 0x18, 0x05, 0xA3, 0x60,
    0x14, 0x8C, 0x02, 0x08, 0x00, 0x00, 0x04, 0x10,
    0x00, 0x01, 0x85, 0x3F, 0xAA, 0x72, 0x00, 0x00,
    0x00, 0x00, 0x49, 0x45, 0x4E, 0x44, 0xAE, 0x42,
    0x60, 0x82
  };

  QPixmap empty;
  empty.loadFromData(empty_png, sizeof(empty_png), 0, Qt::AutoColor);
  QIcon emptyIcon(empty);

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
      insertItem(itemId,  QIcon::fromTheme(g_Icons[Icon::ViewBankAccount]), display);
    } else {
      insertItem(itemId, emptyIcon, display);
    }

    if (curr.id() == (*it).id()) {
      m_selectedItemId = itemId;
      m_currency = (*it);
    }

    itemId++;
  }
  setCurrentIndex(m_selectedItemId);
}

const MyMoneySecurity& KMyMoneySecuritySelector::security() const
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
