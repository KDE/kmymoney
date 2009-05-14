/***************************************************************************
                          kmymoneytransaction.h  -  description
                             -------------------
    begin                : Fri Sep 5 2003
    copyright            : (C) 2000-2003 by Michael Edwardes
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

#ifndef KMYMONEYTRANSACTION_H
#define KMYMONEYTRANSACTION_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include <mymoneytransaction.h>

/**
  * @author Thomas Baumgart
  */

/**
  * This class is used to store the information required to
  * display a transaction in a ledger view (register).
  * It is derived from MyMoneyTransaction but contains additional
  * information.
  */
class KMyMoneyTransaction : public MyMoneyTransaction {
public:
  KMyMoneyTransaction();
  KMyMoneyTransaction(const MyMoneyTransaction& t);
  ~KMyMoneyTransaction();

  void setSplitId(const QString& id);
  const QString& splitId(void) const { return m_splitId; };

private:
  QString    m_splitId;
};


#endif
