/***************************************************************************
                          selectedtransaction.h  -  description
                             -------------------
    begin                : Tue Jun 13 2006
    copyright            : (C) 2000-2006 by Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SELECTEDTRANSACTION_H
#define SELECTEDTRANSACTION_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include <mymoneytransaction.h>
#include <mymoneyscheduled.h>
#include <mymoneysplit.h>
//Added by qt3to4:
#include <QList>

namespace KMyMoneyRegister
{

class SelectedTransaction
{
public:
  SelectedTransaction() {}
  SelectedTransaction(const MyMoneyTransaction& t, const MyMoneySplit& s, const QString& scheduleId = QString()) :
      m_transaction(t), m_split(s), m_scheduleId(scheduleId) {}

  MyMoneyTransaction& transaction(void) {
    return m_transaction;
  }
  const MyMoneyTransaction& transaction(void) const {
    return m_transaction;
  }
  MyMoneySplit& split(void) {
    return m_split;
  }
  const MyMoneySplit& split(void) const {
    return m_split;
  }

  bool isScheduled(void) const {
    return !m_scheduleId.isEmpty();
  }
  const QString& scheduleId(void) const {
    return m_scheduleId;
  }

  /**
   * checks the transaction for specific reasons which would
   * speak against editing/modifying it.
   * @retval 0 no sweat, user can modify
   * @retval 1 at least one split has been reconciled already
   * @retval 2 some transactions cannot be changed anymore - parts of them are frozen
   * @retval 3 some transactions cannot be changed anymore - they touch closed accounts
   */
  int warnLevel() const;

private:
  MyMoneyTransaction      m_transaction;
  MyMoneySplit            m_split;
  QString                 m_scheduleId;
};

class Register;

class SelectedTransactions: public QList<SelectedTransaction>
{
public:
  SelectedTransactions() {}
  SelectedTransactions(const Register* r);

  /**
   * @return the highest warnLevel of all transactions in the list
   */
  int warnLevel() const;

  bool canModify() const;
  bool canDuplicate() const;
};

} // namespace

#endif

