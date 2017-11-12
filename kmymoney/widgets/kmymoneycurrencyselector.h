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

#ifndef KMYMONEYCURRENCYSELECTOR_H
#define KMYMONEYCURRENCYSELECTOR_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <KComboBox>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneysecurity.h"

/**
  * @author Thomas Baumgart
  */
class KMyMoneySecuritySelectorPrivate;
class KMyMoneySecuritySelector : public KComboBox
{
  Q_OBJECT  
  Q_DISABLE_COPY(KMyMoneySecuritySelector)
  Q_PROPERTY(MyMoneySecurity security READ security WRITE setSecurity DESIGNABLE false STORED false)

public:
  explicit KMyMoneySecuritySelector(QWidget* parent = nullptr);
  virtual ~KMyMoneySecuritySelector();

  const MyMoneySecurity& security() const;
  void setSecurity(const MyMoneySecurity& currency);

  void update(const QString& id);

protected:
  KMyMoneySecuritySelectorPrivate * const d_ptr;
  Q_DECLARE_PRIVATE(KMyMoneySecuritySelector)
};

class KMyMoneyCurrencySelector : public KMyMoneySecuritySelector
{
  Q_OBJECT
  Q_DISABLE_COPY(KMyMoneyCurrencySelector)

public:
  explicit KMyMoneyCurrencySelector(QWidget* parent = nullptr);
  ~KMyMoneyCurrencySelector() override;
};

#endif
