/*
    SPDX-FileCopyrightText: 2016-2017 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
    } catch (const MyMoneyException &) {
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
}

QString LedgerSplit::memo() const
{
  Q_D(const LedgerSplit);
  return d->m_split.memo();
}
