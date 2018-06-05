/***************************************************************************
                          selectedtransactions.h  -  description
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

#ifndef SELECTEDTRANSACTIONS_H
#define SELECTEDTRANSACTIONS_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QList>
#include <QMetaType>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "selectedtransaction.h"

namespace KMyMoneyRegister
{
  class Register;

  class SelectedTransactions: public QList<SelectedTransaction>
  {
  public:
    // TODO: find out how to move this ctor out of header
    SelectedTransactions() {} // krazy:exclude=inline
    explicit  SelectedTransactions(const Register* r);

    /**
   * @return the highest warnLevel of all transactions in the list
   */
    SelectedTransaction::warnLevel_t warnLevel() const;

    bool canModify() const;
    bool canDuplicate() const;
  };

} // namespace

Q_DECLARE_METATYPE(KMyMoneyRegister::SelectedTransactions)

#endif

