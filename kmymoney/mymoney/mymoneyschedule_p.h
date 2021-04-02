/*
    SPDX-FileCopyrightText: 2000-2004 Michael Edwardes <mte@users.sourceforge.net>
    SPDX-FileCopyrightText: 2002-2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2005 Ace Jones <acejones@users.sourceforge.net>
    SPDX-License-Identifier: GPL-2.0-or-later
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
