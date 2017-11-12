/***************************************************************************
                          kmymoneyoccurrenceperiodcombo.h  -  description
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

#ifndef KMYMONEYOCCURRENCEPERIODCOMBO_H
#define KMYMONEYOCCURRENCEPERIODCOMBO_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyoccurrencecombo.h"

/**
 * This class implements an occurrence period selector
 *
 * @author Colin Wright
 */
class KMM_WIDGETS_EXPORT KMyMoneyOccurrencePeriodCombo : public KMyMoneyOccurrenceCombo
{
  Q_OBJECT
  Q_DISABLE_COPY(KMyMoneyOccurrencePeriodCombo)

public:
  explicit KMyMoneyOccurrencePeriodCombo(QWidget* parent = nullptr);
  ~KMyMoneyOccurrencePeriodCombo() override;

};

#endif
