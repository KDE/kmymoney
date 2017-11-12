/***************************************************************************
                          kmymoneyoccurrenceperiodcombo.cpp  -  description
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
