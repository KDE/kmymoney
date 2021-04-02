/*
    SPDX-FileCopyrightText: 2009-2016 Cristian One ț <onet.cristian@gmail.com>
    SPDX-FileCopyrightText: 2009-2010 Alvaro Soliverez <asoliverez@gmail.com>
    SPDX-FileCopyrightText: 2010-2016 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kmymoneyoccurrenceperiodcombo.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyschedule.h"
#include "mymoneyenums.h"

using namespace eMyMoney;

KMyMoneyOccurrencePeriodCombo::KMyMoneyOccurrencePeriodCombo(QWidget* parent) :
    KMyMoneyOccurrenceCombo(parent)
{
    addItem(i18nc("Schedule occurrence period", MyMoneySchedule::occurrencePeriodToString(Schedule::Occurrence::Once).toLatin1()), (int)Schedule::Occurrence::Once);
    addItem(i18nc("Schedule occurrence period", MyMoneySchedule::occurrencePeriodToString(Schedule::Occurrence::Daily).toLatin1()), (int)Schedule::Occurrence::Daily);
    addItem(i18nc("Schedule occurrence period", MyMoneySchedule::occurrencePeriodToString(Schedule::Occurrence::Weekly).toLatin1()), (int)Schedule::Occurrence::Weekly);
    addItem(i18nc("Schedule occurrence period", MyMoneySchedule::occurrencePeriodToString(Schedule::Occurrence::EveryHalfMonth).toLatin1()), (int)Schedule::Occurrence::EveryHalfMonth);
    addItem(i18nc("Schedule occurrence period", MyMoneySchedule::occurrencePeriodToString(Schedule::Occurrence::Monthly).toLatin1()), (int)Schedule::Occurrence::Monthly);
    addItem(i18nc("Schedule occurrence period", MyMoneySchedule::occurrencePeriodToString(Schedule::Occurrence::Yearly).toLatin1()), (int)Schedule::Occurrence::Yearly);
}

KMyMoneyOccurrencePeriodCombo::~KMyMoneyOccurrencePeriodCombo()
{
}
