/*
    SPDX-FileCopyrightText: 2014-2017 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ledgerschedule.h"
#include "ledgertransaction_p.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneysplit.h"
#include "mymoneytransaction.h"
#include "mymoneyschedule.h"

using namespace eMyMoney;

class LedgerSchedulePrivate : public LedgerTransactionPrivate
{
public:
  MyMoneySchedule     m_schedule;
};

LedgerSchedule::LedgerSchedule() :
  LedgerTransaction(*new LedgerSchedulePrivate, MyMoneyTransaction(), MyMoneySplit())
{
}

LedgerSchedule::LedgerSchedule(const MyMoneySchedule& s, const MyMoneyTransaction& t, const MyMoneySplit& sp) :
 LedgerTransaction(*new LedgerSchedulePrivate, t, sp)
{
  Q_D(LedgerSchedule);
  d->m_schedule = s;
}

LedgerSchedule::LedgerSchedule(const LedgerSchedule& other) :
  LedgerTransaction(*new LedgerSchedulePrivate(*other.d_func()))
{
}

LedgerSchedule::~LedgerSchedule()
{
  // deletion of d_ptr is taken care of by base class
}

QString LedgerSchedule::transactionSplitId() const
{
  Q_D(const LedgerSchedule);
  return QString::fromLatin1("%1-%2").arg(d->m_schedule.id(), d->m_split.id());
}

QString LedgerSchedule::scheduleId() const
{
  Q_D(const LedgerSchedule);
  return d->m_schedule.id();
}

bool LedgerSchedule::isImported() const
{
  return false;
}
