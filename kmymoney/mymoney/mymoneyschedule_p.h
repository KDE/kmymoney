/*
 * Copyright 2000-2004  Michael Edwardes <mte@users.sourceforge.net>
 * Copyright 2002-2018  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2005       Ace Jones <acejones@users.sourceforge.net>
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

#ifndef MYMONEYSCHEDULE_P_H
#define MYMONEYSCHEDULE_P_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QDate>
#include <QMap>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyobject_p.h"
#include "mymoneytransaction.h"
#include "mymoneyenums.h"

using namespace eMyMoney;

class MyMoneySchedulePrivate : public MyMoneyObjectPrivate
{
public:
  MyMoneySchedulePrivate()
  : m_occurrence(Schedule::Occurrence::Any)
  , m_occurrenceMultiplier(1)
  , m_type(Schedule::Type::Any)
  , m_paymentType(Schedule::PaymentType::Any)
  , m_fixed(false)
  , m_lastDayInMonth(false)
  , m_autoEnter(false)
  , m_weekendOption(Schedule::WeekendOption::MoveNothing)
  {}

  /// Its occurrence
  eMyMoney::Schedule::Occurrence m_occurrence;

  /// Its occurrence multiplier
  int m_occurrenceMultiplier;

  /// Its type
  eMyMoney::Schedule::Type m_type;

  /// The date the schedule commences
  QDate m_startDate;

  /// The payment type
  eMyMoney::Schedule::PaymentType m_paymentType;

  /// Can the amount vary
  bool m_fixed;

  /// The, possibly estimated, amount plus all other relevant details
  MyMoneyTransaction m_transaction;

  /// The last transaction date if the schedule does end at a fixed date
  QDate m_endDate;

  /// the last day in month flag
  bool m_lastDayInMonth;

  /// Enter the transaction into the register automatically
  bool m_autoEnter;

  /// Internal date used for calculations
  QDate m_lastPayment;

  /// The name
  QString m_name;

  /// The recorded payments
  QList<QDate> m_recordedPayments;

  /// The weekend option
  eMyMoney::Schedule::WeekendOption m_weekendOption;
};

#endif
