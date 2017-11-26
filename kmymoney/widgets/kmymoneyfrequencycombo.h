/***************************************************************************
                          kmymoneyfrequencycombo.h  -  description
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

#ifndef KMYMONEYFREQUENCYCOMBO_H
#define KMYMONEYFREQUENCYCOMBO_H

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
class KMM_WIDGETS_EXPORT KMyMoneyFrequencyCombo : public KMyMoneyOccurrenceCombo
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

  void setCurrentData(QVariant data);

Q_SIGNALS:
  void currentDataChanged(QVariant data);

protected Q_SLOTS:
  void slotCurrentDataChanged();

private:
  QVariant data;

};

#endif
