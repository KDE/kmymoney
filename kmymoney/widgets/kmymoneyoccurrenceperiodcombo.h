/*
 * Copyright 2009-2016  Cristian Oneț <onet.cristian@gmail.com>
 * Copyright 2009-2010  Alvaro Soliverez <asoliverez@gmail.com>
 * Copyright 2010-2016  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2017       Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
