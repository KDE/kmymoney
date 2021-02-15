/*
 * SPDX-FileCopyrightText: 2006-2018 Thomas Baumgart <tbaumgart@kde.org>
 * SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef ITEMPTRVECTOR_H
#define ITEMPTRVECTOR_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QVector>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

namespace KMyMoneyRegister
{
  class RegisterItem;
  class ItemPtrVector : public QVector<RegisterItem *>
  {
  public:
    void sort();

  protected:
    /**
    * sorter's compare routine. Returns true if i1 < i2
    */
    static bool item_cmp(RegisterItem* i1, RegisterItem* i2);
  };
} // namespace

#endif
