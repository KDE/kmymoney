/***************************************************************************
                          ledgersplit.cpp
                             -------------------
    begin                : Sat Aug 8 2015
    copyright            : (C) 2015 by Thomas Baumgart
    email                : Thomas Baumgart <tbaumgart@kde.org>
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

#include "ledgersplit.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDebug>
#include <QString>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ledgertransaction_p.h"
#include "mymoneytransaction.h"
#include "mymoneyfile.h"
#include "mymoneypayee.h"
#include "mymoneyexception.h"

using namespace eMyMoney;

class LedgerSplitPrivate : public LedgerTransactionPrivate
{
public:
};

LedgerSplit::LedgerSplit() :
  LedgerTransaction(*new LedgerSplitPrivate, MyMoneyTransaction(), MyMoneySplit())
{
}

LedgerSplit::LedgerSplit(const MyMoneyTransaction& t, const MyMoneySplit& s) :
  LedgerTransaction(*new LedgerSplitPrivate, t, s)
{
  Q_D(LedgerSplit);
  // override the settings made in the base class
  d->m_payeeName.clear();
  d->m_payeeId = d->m_split.payeeId();
  if(!d->m_payeeId.isEmpty()) {
    try {
      d->m_payeeName = MyMoneyFile::instance()->payee(d->m_payeeId).name();
    } catch(MyMoneyException&) {
      qDebug() << "payee" << d->m_payeeId << "not found.";
    }
  }
}

LedgerSplit::LedgerSplit(const LedgerSplit& other) :
  LedgerTransaction(*new LedgerSplitPrivate(*other.d_func()))
{
}

LedgerSplit::~LedgerSplit()
{
  delete d_ptr;
}

QString LedgerSplit::memo() const
{
  Q_D(const LedgerSplit);
  return d->m_split.memo();
}
