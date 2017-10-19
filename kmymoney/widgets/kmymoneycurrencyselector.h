/***************************************************************************
                          kmymoneycurrencyselector.h  -  description
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

#ifndef KMYMONEYCURRENCYSELECTOR_H
#define KMYMONEYCURRENCYSELECTOR_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QList>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KComboBox>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneysecurity.h"

/**
  * @author Thomas Baumgart
  */

class KMyMoneySecuritySelector : public KComboBox
{
  Q_OBJECT
  Q_PROPERTY(MyMoneySecurity security READ security WRITE setSecurity DESIGNABLE false STORED false)
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

  explicit KMyMoneySecuritySelector(QWidget *parent = 0);
  virtual ~KMyMoneySecuritySelector();

  const MyMoneySecurity& security() const;
  void setSecurity(const MyMoneySecurity& currency);
  void selectDisplayItem(KMyMoneySecuritySelector::displayItemE item);

  void setDisplayType(displayTypeE type);

  void update(const QString& id);

private:
  MyMoneySecurity m_currency;
  displayItemE    m_displayItem;
  int             m_selectedItemId;
  bool            m_displayOnly;
  displayTypeE    m_displayType;
  QList<MyMoneySecurity> m_list;
};

class KMyMoneyCurrencySelector : public KMyMoneySecuritySelector
{
  Q_OBJECT
public:
  KMyMoneyCurrencySelector(QWidget *parent = 0);
  virtual ~KMyMoneyCurrencySelector() {}
};

#endif
