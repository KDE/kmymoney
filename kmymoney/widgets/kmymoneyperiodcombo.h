/***************************************************************************
                          kmymoneyperiodcombo.h  -  description
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

#ifndef KMYMONEYPERIODCOMBO_H
#define KMYMONEYPERIODCOMBO_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneygeneralcombo.h"

namespace eMyMoney { namespace TransactionFilter { enum class Date; } }

/**
 * This class implements a time period selector
 * @author Thomas Baumgart
 */
class KMM_WIDGETS_EXPORT KMyMoneyPeriodCombo : public KMyMoneyGeneralCombo
{
  Q_OBJECT
  Q_DISABLE_COPY(KMyMoneyPeriodCombo)

public:
  explicit KMyMoneyPeriodCombo(QWidget* parent = nullptr);
  ~KMyMoneyPeriodCombo() override;

  eMyMoney::TransactionFilter::Date currentItem() const;
  void setCurrentItem(eMyMoney::TransactionFilter::Date id);

  /**
   * This function returns the actual start date for the given
   * period definition given by @p id. For user defined periods
   * the returned value is QDate()
   */
  static QDate start(eMyMoney::TransactionFilter::Date id);

  /**
   * This function returns the actual end date for the given
   * period definition given by @p id. For user defined periods
   * the returned value is QDate()
   */
  static QDate end(eMyMoney::TransactionFilter::Date id);

  // static void dates(QDate& start, QDate& end, MyMoneyTransactionFilter::dateOptionE id);
};

#endif
