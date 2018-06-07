/*
 * Copyright 2006-2018  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2017-2018  Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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
