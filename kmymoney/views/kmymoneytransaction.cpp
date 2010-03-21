/***************************************************************************
                          kmymoneytransaction.cpp  -  description
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

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes


#include "kmymoneytransaction.h"

KMyMoneyTransaction::KMyMoneyTransaction()
{
}

KMyMoneyTransaction::KMyMoneyTransaction(const MyMoneyTransaction& t) :
    MyMoneyTransaction(t)
{
}

KMyMoneyTransaction::~KMyMoneyTransaction()
{
}

void KMyMoneyTransaction::setSplitId(const QString& id)
{
  m_splitId = id;
}

bool KMyMoneyTransaction::operator<(const KMyMoneyTransaction& right)
{
  bool result = false;

  if(postDate() < right.postDate())
    result = true;
  return result;
}
