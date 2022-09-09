/*
    SPDX-FileCopyrightText: 2010-2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2010-2016 Cristian Oneț <onet.cristian@gmail.com>
    SPDX-FileCopyrightText: 2010 Alvaro Soliverez <asoliverez@gmail.com>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kmymoneyfrequencycombo.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QByteArray>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyschedule.h"
#include "mymoneyenums.h"

using namespace eMyMoney;

KMyMoneyFrequencyCombo::KMyMoneyFrequencyCombo(QWidget* parent) :
    KMyMoneyOccurrenceCombo(parent)
{
    addItem(MyMoneySchedule::occurrenceToString(Schedule::Occurrence::Once), (int)Schedule::Occurrence::Once);
    addItem(MyMoneySchedule::occurrenceToString(Schedule::Occurrence::Daily), (int)Schedule::Occurrence::Daily);
    addItem(MyMoneySchedule::occurrenceToString(Schedule::Occurrence::Weekly), (int)Schedule::Occurrence::Weekly);
    addItem(MyMoneySchedule::occurrenceToString(Schedule::Occurrence::EveryOtherWeek), (int)Schedule::Occurrence::EveryOtherWeek);
    addItem(MyMoneySchedule::occurrenceToString(Schedule::Occurrence::EveryHalfMonth), (int)Schedule::Occurrence::EveryHalfMonth);
    addItem(MyMoneySchedule::occurrenceToString(Schedule::Occurrence::EveryThreeWeeks), (int)Schedule::Occurrence::EveryThreeWeeks);
    addItem(MyMoneySchedule::occurrenceToString(Schedule::Occurrence::EveryThirtyDays), (int)Schedule::Occurrence::EveryThirtyDays);
    addItem(MyMoneySchedule::occurrenceToString(Schedule::Occurrence::EveryFourWeeks), (int)Schedule::Occurrence::EveryFourWeeks);
    addItem(MyMoneySchedule::occurrenceToString(Schedule::Occurrence::Monthly), (int)Schedule::Occurrence::Monthly);
    addItem(MyMoneySchedule::occurrenceToString(Schedule::Occurrence::EveryEightWeeks), (int)Schedule::Occurrence::EveryEightWeeks);
    addItem(MyMoneySchedule::occurrenceToString(Schedule::Occurrence::EveryOtherMonth), (int)Schedule::Occurrence::EveryOtherMonth);
    addItem(MyMoneySchedule::occurrenceToString(Schedule::Occurrence::EveryThreeMonths), (int)Schedule::Occurrence::EveryThreeMonths);
    addItem(MyMoneySchedule::occurrenceToString(Schedule::Occurrence::EveryFourMonths), (int)Schedule::Occurrence::EveryFourMonths);
    addItem(MyMoneySchedule::occurrenceToString(Schedule::Occurrence::TwiceYearly), (int)Schedule::Occurrence::TwiceYearly);
    addItem(MyMoneySchedule::occurrenceToString(Schedule::Occurrence::Yearly), (int)Schedule::Occurrence::Yearly);
    addItem(MyMoneySchedule::occurrenceToString(Schedule::Occurrence::EveryOtherYear), (int)Schedule::Occurrence::EveryOtherYear);

    connect(this, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &KMyMoneyFrequencyCombo::slotCurrentDataChanged);
}

KMyMoneyFrequencyCombo::~KMyMoneyFrequencyCombo()
{
}

int KMyMoneyFrequencyCombo::daysBetweenEvents() const
{
    return MyMoneySchedule::daysBetweenEvents(currentItem());
}

int KMyMoneyFrequencyCombo::eventsPerYear() const
{
    return MyMoneySchedule::eventsPerYear(currentItem());
}

QVariant KMyMoneyFrequencyCombo::currentData() const
{
    return itemData(currentIndex(), Qt::UserRole);
}

void KMyMoneyFrequencyCombo::setCurrentData(QVariant datavar)
{
    setItemData(currentIndex(), datavar, Qt::UserRole);
}

void KMyMoneyFrequencyCombo::slotCurrentDataChanged()
{
    Q_EMIT currentDataChanged(currentData());
}
