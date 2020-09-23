/*
 * Copyright 2010-2018  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2010-2016  Cristian Oneț <onet.cristian@gmail.com>
 * Copyright 2010       Alvaro Soliverez <asoliverez@gmail.com>
 * Copyright 2017-2018  Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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

#ifndef KMYMONEYFREQUENCYCOMBO_H
#define KMYMONEYFREQUENCYCOMBO_H

#include "kmm_base_widgets_export.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyoccurrencecombo.h"

/**
 * This class implements a payment frequency selector
 * @author Thomas Baumgart
 */
class KMM_BASE_WIDGETS_EXPORT KMyMoneyFrequencyCombo : public KMyMoneyOccurrenceCombo
{
  Q_OBJECT
  Q_DISABLE_COPY(KMyMoneyFrequencyCombo)
  Q_PROPERTY(QVariant data READ currentData WRITE setCurrentData STORED false)

public:
  explicit KMyMoneyFrequencyCombo(QWidget* parent = nullptr);
  ~KMyMoneyFrequencyCombo() override;

  /**
   * This method returns the number of events for the selected payment
   * frequency (eg for yearly the return value is 1 and for monthly it
   * is 12). In case, the frequency cannot be converted (once, every other year, etc.)
   * the method returns 0.
   */
  int eventsPerYear() const;
  /**
   * This method returns the number of days between two events of
   * the selected frequency. The return value for months is based
   * on 30 days and the year is 360 days long.
   */
  int daysBetweenEvents() const;

  QVariant currentData() const;

  void setCurrentData(QVariant datavar);

Q_SIGNALS:
  void currentDataChanged(QVariant data);

protected Q_SLOTS:
  void slotCurrentDataChanged();

private:
  QVariant data;

};

#endif
