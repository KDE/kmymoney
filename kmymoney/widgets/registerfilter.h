/***************************************************************************
                          registerfilter.h  -  description
                             -------------------
    begin                : Tue Jun 13 2006
    copyright            : (C) 2000-2006 by Thomas Baumgart
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

#ifndef REGISTERFILTER_H
#define REGISTERFILTER_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

namespace eWidgets { namespace eRegister { enum class ItemState; } }

namespace KMyMoneyRegister
{
  /**
  * Used to filter items from the register.
  */
  struct RegisterFilter {
    explicit RegisterFilter(const QString &t, eWidgets::eRegister::ItemState s);

    eWidgets::eRegister::ItemState state;
    QString text;
  };

} // namespace

#endif
