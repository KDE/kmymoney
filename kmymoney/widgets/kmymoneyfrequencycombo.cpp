/***************************************************************************
                          kmymoneyfrequencycombo.cpp  -  description
                             -------------------
    begin                : Sat Jan 09 2010
    copyright            : (C) 2010 by Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Cristian Onet <cristian.onet@gmail.com>
                           Alvaro Soliverez <asoliverez@gmail.com>
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
  addItem(i18nc("Frequency of schedule", MyMoneySchedule::occurrenceToString(Schedule::Occurrence::Once).toLatin1()), (int)Schedule::Occurrence::Once);
  addItem(i18nc("Frequency of schedule", MyMoneySchedule::occurrenceToString(Schedule::Occurrence::Daily).toLatin1()), (int)Schedule::Occurrence::Daily);
  addItem(i18nc("Frequency of schedule", MyMoneySchedule::occurrenceToString(Schedule::Occurrence::Weekly).toLatin1()), (int)Schedule::Occurrence::Weekly);
  addItem(i18nc("Frequency of schedule", MyMoneySchedule::occurrenceToString(Schedule::Occurrence::EveryOtherWeek).toLatin1()), (int)Schedule::Occurrence::EveryOtherWeek);
  addItem(i18nc("Frequency of schedule", MyMoneySchedule::occurrenceToString(Schedule::Occurrence::EveryHalfMonth).toLatin1()), (int)Schedule::Occurrence::EveryHalfMonth);
  addItem(i18nc("Frequency of schedule", MyMoneySchedule::occurrenceToString(Schedule::Occurrence::EveryThreeWeeks).toLatin1()), (int)Schedule::Occurrence::EveryThreeWeeks);
  addItem(i18nc("Frequency of schedule", MyMoneySchedule::occurrenceToString(Schedule::Occurrence::EveryThirtyDays).toLatin1()), (int)Schedule::Occurrence::EveryThirtyDays);
  addItem(i18nc("Frequency of schedule", MyMoneySchedule::occurrenceToString(Schedule::Occurrence::EveryFourWeeks).toLatin1()), (int)Schedule::Occurrence::EveryFourWeeks);
  addItem(i18nc("Frequency of schedule", MyMoneySchedule::occurrenceToString(Schedule::Occurrence::Monthly).toLatin1()), (int)Schedule::Occurrence::Monthly);
  addItem(i18nc("Frequency of schedule", MyMoneySchedule::occurrenceToString(Schedule::Occurrence::EveryEightWeeks).toLatin1()), (int)Schedule::Occurrence::EveryEightWeeks);
  addItem(i18nc("Frequency of schedule", MyMoneySchedule::occurrenceToString(Schedule::Occurrence::EveryOtherMonth).toLatin1()), (int)Schedule::Occurrence::EveryOtherMonth);
  addItem(i18nc("Frequency of schedule", MyMoneySchedule::occurrenceToString(Schedule::Occurrence::EveryThreeMonths).toLatin1()), (int)Schedule::Occurrence::EveryThreeMonths);
  addItem(i18nc("Frequency of schedule", MyMoneySchedule::occurrenceToString(Schedule::Occurrence::EveryFourMonths).toLatin1()), (int)Schedule::Occurrence::EveryFourMonths);
  addItem(i18nc("Frequency of schedule", MyMoneySchedule::occurrenceToString(Schedule::Occurrence::TwiceYearly).toLatin1()), (int)Schedule::Occurrence::TwiceYearly);
  addItem(i18nc("Frequency of schedule", MyMoneySchedule::occurrenceToString(Schedule::Occurrence::Yearly).toLatin1()), (int)Schedule::Occurrence::Yearly);
  addItem(i18nc("Frequency of schedule", MyMoneySchedule::occurrenceToString(Schedule::Occurrence::EveryOtherYear).toLatin1()), (int)Schedule::Occurrence::EveryOtherYear);

  connect(this, SIGNAL(currentIndexChanged(int)), this, SLOT(slotCurrentDataChanged()));
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

void KMyMoneyFrequencyCombo::setCurrentData(QVariant data)
{
  setItemData(currentIndex(), data, Qt::UserRole);
}

void KMyMoneyFrequencyCombo::slotCurrentDataChanged()
{
  emit currentDataChanged(currentData());
}
