/*
    SPDX-FileCopyrightText: 2004-2011 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KMYMONEYCURRENCYSELECTOR_H
#define KMYMONEYCURRENCYSELECTOR_H

#include "kmm_base_widgets_export.h"

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
class KMM_BASE_WIDGETS_EXPORT KMyMoneySecuritySelector : public KComboBox
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

class KMM_BASE_WIDGETS_EXPORT KMyMoneyCurrencySelector : public KMyMoneySecuritySelector
{
  Q_OBJECT
  Q_DISABLE_COPY(KMyMoneyCurrencySelector)

public:
  explicit KMyMoneyCurrencySelector(QWidget* parent = nullptr);
  ~KMyMoneyCurrencySelector() override;
};

#endif
