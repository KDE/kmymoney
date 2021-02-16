/*
    SPDX-FileCopyrightText: 2002-2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef DATERANGEDLG_H
#define DATERANGEDLG_H

#include "kmm_widgets_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QWidget>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

namespace eMyMoney { namespace TransactionFilter { enum class Date; } }

class DateRangeDlgPrivate;
class KMM_WIDGETS_EXPORT DateRangeDlg : public QWidget
{
  Q_OBJECT
  Q_DISABLE_COPY(DateRangeDlg)

public:
  explicit DateRangeDlg(QWidget* parent = nullptr);
  ~DateRangeDlg();

  /*!
   * Setup a sliding date range which is relative to the
   * current date (sliding date range)
   */
  void setDateRange(eMyMoney::TransactionFilter::Date);

  /*!
   * Setup a fixed user selected date range (does not slide)
   */
  void setDateRange(const QDate& from, const QDate& to);

  /*!
   * Return the currently selected date range option
   */
  eMyMoney::TransactionFilter::Date dateRange() const;

  QDate fromDate() const;
  QDate toDate() const;

public Q_SLOTS:
  void slotReset();
  void slotDateRangeSelectedByUser();
  void slotDateChanged();

Q_SIGNALS:
  /*!
   * The rangeChanged() signal is emitted whenever a range
   * is changed (user interaction or call to setDateRange() )
   */
  void rangeChanged();

private:
  DateRangeDlgPrivate * const d_ptr;
  Q_DECLARE_PRIVATE(DateRangeDlg)
};

#endif
