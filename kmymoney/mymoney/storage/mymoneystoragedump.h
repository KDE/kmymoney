/***************************************************************************
                           mymoneystoragedump.h  -  description
                             -------------------
    begin                : Sun May 5 2002
    copyright            : (C) 2000-2002 by Michael Edwardes
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

#ifndef MYMONEYSTORAGEDUMP_H
#define MYMONEYSTORAGEDUMP_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneykeyvaluecontainer.h"

/**
  * @author Thomas Baumgart
  */

class IMyMoneySerialize;
class IMyMoneyStorage;
class MyMoneyTransaction;

namespace eMyMoney { namespace Split { enum class State; } }

class MyMoneyStorageDump
{
public:
  MyMoneyStorageDump();
  ~MyMoneyStorageDump();

  void readStream(QDataStream& s, IMyMoneySerialize* storage);
  void writeStream(QDataStream& s, IMyMoneySerialize* storage);

private:
  void dumpTransaction(QTextStream& s, IMyMoneyStorage* storage, const MyMoneyTransaction& it_t);
  void dumpKVP(const QString& headline, QTextStream& s, const MyMoneyKeyValueContainer &kvp, int indent = 0);
  const QString reconcileToString(eMyMoney::Split::State flag) const;
};

#endif
