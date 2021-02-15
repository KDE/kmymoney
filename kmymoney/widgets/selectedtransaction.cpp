/*
 * SPDX-FileCopyrightText: 2006-2018 Thomas Baumgart <tbaumgart@kde.org>
 * SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "selectedtransaction.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyfile.h"
#include "mymoneyaccount.h"
#include "mymoneysplit.h"
#include "mymoneytransaction.h"
#include "mymoneyexception.h"
#include "mymoneyenums.h"

using namespace KMyMoneyRegister;

namespace KMyMoneyRegister
{
  class SelectedTransactionPrivate
  {
  public:
    MyMoneyTransaction      m_transaction;
    MyMoneySplit            m_split;
    QString                 m_scheduleId;
  };
}

SelectedTransaction::SelectedTransaction() :
  d_ptr(new SelectedTransactionPrivate)
{
}

SelectedTransaction::SelectedTransaction(const MyMoneyTransaction& t, const MyMoneySplit& s, const QString& scheduleId = QString()) :
  d_ptr(new SelectedTransactionPrivate)
{
  Q_D(SelectedTransaction);
  d->m_transaction = t;
  d->m_split = s;
  d->m_scheduleId = scheduleId;
}

SelectedTransaction::SelectedTransaction(const SelectedTransaction& other) :
  d_ptr(new SelectedTransactionPrivate(*other.d_func()))
{
}

SelectedTransaction::~SelectedTransaction()
{
  Q_D(SelectedTransaction);
  delete d;
}

MyMoneyTransaction& SelectedTransaction::transaction()
{
  Q_D(SelectedTransaction);
  return d->m_transaction;
}

MyMoneyTransaction SelectedTransaction::transaction() const
{
  Q_D(const SelectedTransaction);
  return d->m_transaction;
}

MyMoneySplit& SelectedTransaction::split()
{
  Q_D(SelectedTransaction);
  return d->m_split;
}

MyMoneySplit SelectedTransaction::split() const
{
  Q_D(const SelectedTransaction);
  return d->m_split;
}

bool SelectedTransaction::isScheduled() const
{
  Q_D(const SelectedTransaction);
  return !d->m_scheduleId.isEmpty();
}

QString SelectedTransaction::scheduleId() const
{
  Q_D(const SelectedTransaction);
  return d->m_scheduleId;
}

SelectedTransaction::warnLevel_t SelectedTransaction::warnLevel() const
{
  auto warnLevel = NoWarning;
  foreach (const auto split, transaction().splits()) {
    try {
      auto acc = MyMoneyFile::instance()->account(split.accountId());
      if (acc.isClosed())
        warnLevel = OneAccountClosed;
      else if (split.reconcileFlag() == eMyMoney::Split::State::Frozen)
        warnLevel = OneSplitFrozen;
      else if (split.reconcileFlag() == eMyMoney::Split::State::Reconciled && warnLevel < 1)
        warnLevel = OneSplitReconciled;
    } catch (const MyMoneyException &) {
      //qDebug("Exception in SelectedTransaction::warnLevel(): %s", e.what());
      warnLevel = NoWarning;
    }
  }
  return warnLevel;
}

