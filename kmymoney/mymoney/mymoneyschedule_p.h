/***************************************************************************
                          mymoneyschedule.cpp
                             -------------------
    copyright            : (C) 2000-2002 by Michael Edwardes <mte@users.sourceforge.net>
                           (C) 2007 by Thomas Baumgart <ipwizard@users.sourceforge.net>

***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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

#include "mymoneytransaction.h"
#include "mymoneyenums.h"

using namespace eMyMoney;

namespace eMyMoney
{
  namespace Schedule
  {
    enum class Element { Payment,
                         Payments
                       };

    enum class Attribute { Name = 0,
                           Type,
                           Occurrence,
                           OccurrenceMultiplier,
                           PaymentType,
                           Fixed,
                           AutoEnter,
                           LastPayment,
                           WeekendOption,
                           Date,
                           StartDate,
                           EndDate,
                           LastDayInMonth,
                           // insert new entries above this line
                           LastAttribute
                         };
    uint qHash(const Attribute key, uint seed) { return ::qHash(static_cast<uint>(key), seed); }
  }
}

class MyMoneySchedulePrivate {

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

  static QString getElName(const Schedule::Element el)
  {
    static const QMap<Schedule::Element, QString> elNames = {
      {Schedule::Element::Payment,  QStringLiteral("PAYMENT")},
      {Schedule::Element::Payments, QStringLiteral("PAYMENTS")}
    };
    return elNames[el];
  }

  static QString getAttrName(const Schedule::Attribute attr)
  {
    static const QHash<Schedule::Attribute, QString> attrNames = {
      {Schedule::Attribute::Name,                 QStringLiteral("name")},
      {Schedule::Attribute::Type,                 QStringLiteral("type")},
      {Schedule::Attribute::Occurrence,           QStringLiteral("occurrence")},
      {Schedule::Attribute::OccurrenceMultiplier, QStringLiteral("occurrenceMultiplier")},
      {Schedule::Attribute::PaymentType,          QStringLiteral("paymentType")},
      {Schedule::Attribute::Fixed,                QStringLiteral("fixed")},
      {Schedule::Attribute::AutoEnter,            QStringLiteral("autoEnter")},
      {Schedule::Attribute::LastPayment,          QStringLiteral("lastPayment")},
      {Schedule::Attribute::WeekendOption,        QStringLiteral("weekendOption")},
      {Schedule::Attribute::Date,                 QStringLiteral("date")},
      {Schedule::Attribute::StartDate,            QStringLiteral("startDate")},
      {Schedule::Attribute::EndDate,              QStringLiteral("endDate")},
      {Schedule::Attribute::LastDayInMonth,       QStringLiteral("lastDayInMonth")}
    };
    return attrNames[attr];
  }


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
