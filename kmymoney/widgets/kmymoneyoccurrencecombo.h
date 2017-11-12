/***************************************************************************
                          kmymoneyoccurrencecombo.h  -  description
                             -------------------
    begin                : Mon Jan 09 2010
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

#ifndef KMYMONEYOCCURRENCECOMBO_H
#define KMYMONEYOCCURRENCECOMBO_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneygeneralcombo.h"

namespace eMyMoney { namespace Schedule { enum class Occurrence; } }

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
