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

#include <QDateTime>
#include <QMap>
#include <QResizeEvent>
#include <QEvent>
#include <QList>

// ----------------------------------------------------------------------------
// KDE Includes

#include <QWidget>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyutils.h"
#include "mymoneytransactionfilter.h"

namespace Ui
{
class DateRangeDlgDecl;
}

class DateRangeDlg : public QWidget
{
  Q_OBJECT

public:

  /*
  // Make sure to keep the following enum valus in sync with the values
  // used by the GUI in daterangedlgdecl.ui
  enum dateOptionE {
    allDates = 0,
    asOfToday,
    currentMonth,
    currentYear,
    monthToDate,
    yearToDate,
    yearToMonth,
    lastMonth,
    lastYear,
    last7Days,
    last30Days,
    last3Months,
    last6Months,
    last12Months,
    next7Days,
    next30Days,
    next3Months,
    next6Months,
    next12Months,
    userDefined,
    last3ToNext3Months,
    last11Months,
    next18Months,
    // insert new constants above of this line
    dateOptionCount
  };
  */
  DateRangeDlg(QWidget *parent = 0);
  ~DateRangeDlg();
  Ui::DateRangeDlgDecl*    m_ui;

public slots:
  void slotReset();
  void slotUpdateSelections(QString &txt);
  void slotDateRangeChanged(int);
  void slotDateChanged();

private:
  void setupDatePage();

  QDate                m_startDates[MyMoneyTransactionFilter::dateOptionCount];
  QDate                m_endDates[MyMoneyTransactionFilter::dateOptionCount];
};

#endif
