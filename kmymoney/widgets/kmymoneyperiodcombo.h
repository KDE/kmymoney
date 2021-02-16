/*
    SPDX-FileCopyrightText: 2010-2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2010-2016 Cristian One ț <onet.cristian@gmail.com>
    SPDX-FileCopyrightText: 2010 Alvaro Soliverez <asoliverez@gmail.com>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
