/***************************************************************************
                          daterange.h
                             -------------------
    copyright            : (C) 2003 by Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DATERANGEDLG_H
#define DATERANGEDLG_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QDate>

// ----------------------------------------------------------------------------
// KDE Includes

#include <QWidget>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneytransactionfilter.h"
#include "mymoneyenums.h"

namespace Ui
{
class DateRangeDlgDecl;
}

class DateRangeDlg : public QWidget
{
  Q_OBJECT

public:
  DateRangeDlg(QWidget *parent = 0);
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

public slots:
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
  void setupDatePage();

  Ui::DateRangeDlgDecl*  m_ui;
  QDate                  m_startDates[(int)eMyMoney::TransactionFilter::Date::LastDateItem];
  QDate                  m_endDates[(int)eMyMoney::TransactionFilter::Date::LastDateItem];
};

#endif
