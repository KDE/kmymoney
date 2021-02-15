/*
 * SPDX-FileCopyrightText: 2009-2010 Cristian One ț <onet.cristian@gmail.com>
 * SPDX-FileCopyrightText: 2009-2010 Alvaro Soliverez <asoliverez@gmail.com>
 * SPDX-FileCopyrightText: 2011-2017 Thomas Baumgart <tbaumgart@kde.org>
 * SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kmymoneyoccurrencecombo.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyenums.h"

using namespace eMyMoney;

KMyMoneyOccurrenceCombo::KMyMoneyOccurrenceCombo(QWidget* parent) :
    KMyMoneyGeneralCombo(parent)
{
}

KMyMoneyOccurrenceCombo::~KMyMoneyOccurrenceCombo()
{
}

Schedule::Occurrence KMyMoneyOccurrenceCombo::currentItem() const
{
  return static_cast<Schedule::Occurrence>(KMyMoneyGeneralCombo::currentItem());
}

