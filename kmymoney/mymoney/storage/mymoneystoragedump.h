/*
    SPDX-FileCopyrightText: 2002-2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2004 Ace Jones <acejones@users.sourceforge.net>
    SPDX-License-Identifier: GPL-2.0-or-later
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

namespace eMyMoney {
namespace Split {
enum class State;
}
}

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
