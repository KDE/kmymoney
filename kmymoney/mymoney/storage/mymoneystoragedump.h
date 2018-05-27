/*
 * Copyright 2002-2018  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2004       Ace Jones <acejones@users.sourceforge.net>
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

#ifndef MYMONEYSTORAGEDUMP_H
#define MYMONEYSTORAGEDUMP_H

#include "kmm_mymoney_export.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneykeyvaluecontainer.h"

/**
  * @author Thomas Baumgart
  */

class MyMoneyStorageMgr;
class MyMoneyTransaction;
class QTextStream;

namespace eMyMoney { namespace Split { enum class State; } }

class KMM_MYMONEY_EXPORT MyMoneyStorageDump
{
public:
  MyMoneyStorageDump();
  ~MyMoneyStorageDump();

  void readStream(QDataStream& s, MyMoneyStorageMgr* storage);
  void writeStream(QDataStream& s, MyMoneyStorageMgr* storage);

private:
  void dumpTransaction(QTextStream& s, MyMoneyStorageMgr* storage, const MyMoneyTransaction& it_t);
  void dumpKVP(const QString& headline, QTextStream& s, const MyMoneyKeyValueContainer &kvp, int indent = 0);
  const QString reconcileToString(eMyMoney::Split::State flag) const;
};

#endif
