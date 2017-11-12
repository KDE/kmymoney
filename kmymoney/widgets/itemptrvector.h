/***************************************************************************
                             itemptrvector.h
                             ----------
    begin                : Fri Mar 10 2006
    copyright            : (C) 2006 by Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
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
