/*
    SPDX-FileCopyrightText: 2009-2010 Cristian One ț <onet.cristian@gmail.com>
    SPDX-FileCopyrightText: 2009-2010 Alvaro Soliverez <asoliverez@gmail.com>
    SPDX-FileCopyrightText: 2011-2017 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KMYMONEYOCCURRENCECOMBO_H
#define KMYMONEYOCCURRENCECOMBO_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneygeneralcombo.h"

namespace eMyMoney {
namespace Schedule {
enum class Occurrence;
}
}

/**
 * This class implements an occurrence selector
 * as a parent class for both OccurrencePeriod and Frequency combos
 *
 * @author Colin Wright
 */
class KMM_WIDGETS_EXPORT KMyMoneyOccurrenceCombo : public KMyMoneyGeneralCombo
{
    Q_OBJECT
    Q_DISABLE_COPY(KMyMoneyOccurrenceCombo)

public:
    explicit KMyMoneyOccurrenceCombo(QWidget* parent = nullptr);
    ~KMyMoneyOccurrenceCombo() override;

    eMyMoney::Schedule::Occurrence currentItem() const;
};

#endif
