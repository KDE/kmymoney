/*
 * Copyright 2004-2011  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2017       Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
