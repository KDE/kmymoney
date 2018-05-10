/***************************************************************************
                          selectedtransaction.cpp  -  description
                             -------------------
    begin                : Fri Jun 2008
    copyright            : (C) 2000-2008 by Thomas Baumgart
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

int SelectedTransaction::warnLevel() const
{
  auto warnLevel = 0;
  foreach (const auto split, transaction().splits()) {
    try {
      auto acc = MyMoneyFile::instance()->account(split.accountId());
      if (acc.isClosed())
        warnLevel = 3;
      else if (split.reconcileFlag() == eMyMoney::Split::State::Frozen)
        warnLevel = 2;
      else if (split.reconcileFlag() == eMyMoney::Split::State::Reconciled && warnLevel < 1)
        warnLevel = 1;
    } catch (const MyMoneyException &) {
      //qDebug("Exception in SelectedTransaction::warnLevel(): %s", e.what());
      warnLevel = 0;
    }
  }
  return warnLevel;
}

