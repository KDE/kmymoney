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
#include "mymoneysecurity.h"
#include "icons/icons.h"

using namespace Icons;

class KMyMoneySecuritySelectorPrivate
{
  Q_DISABLE_COPY(KMyMoneySecuritySelectorPrivate)
  Q_DECLARE_PUBLIC(KMyMoneySecuritySelector)

public:
  enum displayItemE {
    Symbol = 0,
    FullName
  };

  enum displayTypeE {
    TypeCurrencies = 0x01,
    TypeSecurities = 0x02,
    TypeAll        = 0x03
  };

  explicit KMyMoneySecuritySelectorPrivate(KMyMoneySecuritySelector *qq):
    q_ptr(qq),
    m_displayItem(FullName),
    m_selectedItemId(0),
    m_displayOnly(false),
    m_displayType(TypeAll)
  {
  }

  void selectDisplayItem(displayItemE item)
  {
    Q_Q(KMyMoneySecuritySelector);
    m_displayItem = item;
    q->update(QString());
  }

  void setDisplayType(displayTypeE type)
  {
    m_displayType = type;
  }

  KMyMoneySecuritySelector *q_ptr;
  MyMoneySecurity m_currency;
  displayItemE    m_displayItem;
  int             m_selectedItemId;
  bool            m_displayOnly;
  displayTypeE    m_displayType;
  QList<MyMoneySecurity> m_list;
};

KMyMoneySecuritySelector::KMyMoneySecuritySelector(QWidget *parent) :
  KComboBox(parent),
  d_ptr(new KMyMoneySecuritySelectorPrivate(this))
{
  // update(QString());
}

KMyMoneySecuritySelector::~KMyMoneySecuritySelector()
{
  Q_D(KMyMoneySecuritySelector);
  delete d;
}

void KMyMoneySecuritySelector::update(const QString& id)
{
  Q_D(KMyMoneySecuritySelector);
  MyMoneySecurity curr = MyMoneyFile::instance()->baseCurrency();
  QString baseCurrency = curr.id();

  if (!id.isEmpty())
    curr = d->m_currency;

  this->clear();
  d->m_list.clear();
  if (d->m_displayType & KMyMoneySecuritySelectorPrivate::TypeCurrencies)
    d->m_list += MyMoneyFile::instance()->currencyList();
  if (d->m_displayType & KMyMoneySecuritySelectorPrivate::TypeSecurities)
    d->m_list += MyMoneyFile::instance()->securityList();

  // sort
  qSort(d->m_list);

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
  for (it = d->m_list.constBegin(); it != d->m_list.constEnd(); ++it) {
    QString display;
    switch (d->m_displayItem) {
      default:
      case KMyMoneySecuritySelectorPrivate::FullName:
        if ((*it).isCurrency()) {
          display = QString("%2 (%1)").arg((*it).id()).arg((*it).name());
        } else
          display = QString("%2 (%1)").arg((*it).tradingSymbol()).arg((*it).name());
        break;
        break;

      case KMyMoneySecuritySelectorPrivate::Symbol:
        if ((*it).isCurrency())
          display = (*it).id();
        else
          display = (*it).tradingSymbol();
        break;
    }
    if ((*it).id() == baseCurrency) {
      insertItem(itemId,  Icons::get(Icon::ViewBankAccount), display);
    } else {
      insertItem(itemId, emptyIcon, display);
    }

    if (curr.id() == (*it).id()) {
      m_selectedItemId = itemId;
      d->m_currency = (*it);
    }

    itemId++;
  }
  setCurrentIndex(m_selectedItemId);
}

const MyMoneySecurity& KMyMoneySecuritySelector::security() const
{
  Q_D(const KMyMoneySecuritySelector);
  int index = currentIndex();
  if ((0 <= index) && (index < d->m_list.size()))
    return d->m_list[index];
  else
    return d->m_currency;
}

void KMyMoneySecuritySelector::setSecurity(const MyMoneySecurity& currency)
{
  Q_D(KMyMoneySecuritySelector);
  d->m_currency = currency;
  update(QString("x"));
}

KMyMoneyCurrencySelector::KMyMoneyCurrencySelector(QWidget *parent) :
  KMyMoneySecuritySelector(parent)
{
  Q_D(KMyMoneySecuritySelector);
  d->setDisplayType(KMyMoneySecuritySelectorPrivate::TypeCurrencies);
}

KMyMoneyCurrencySelector::~KMyMoneyCurrencySelector()
{
}
