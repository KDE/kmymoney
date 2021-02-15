/*
 * SPDX-FileCopyrightText: 2004-2017 Thomas Baumgart <tbaumgart@kde.org>
 * SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KMYMONEYACCOUNTCOMPLETION_H
#define KMYMONEYACCOUNTCOMPLETION_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneycompletion.h"

namespace eMyMoney { namespace Account { enum class Type; } }

class KMyMoneyAccountSelector;

/**
  * @author Thomas Baumgart
  */
class KMyMoneyAccountCompletion : public KMyMoneyCompletion
{
  Q_OBJECT
  Q_DISABLE_COPY(KMyMoneyAccountCompletion)

public:

  explicit KMyMoneyAccountCompletion(QWidget* parent = nullptr);
  ~KMyMoneyAccountCompletion() override;

  QStringList accountList(const QList<eMyMoney::Account::Type>& list) const;
  QStringList accountList() const;

  /**
    * reimplemented from KMyMoneyCompletion
    */
  KMyMoneyAccountSelector* selector() const;

public Q_SLOTS:
  void slotMakeCompletion(const QString& txt);
};

#endif
