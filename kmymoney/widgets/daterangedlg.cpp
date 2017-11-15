/***************************************************************************
                          daterange.cpp
                             -------------------
    copyright            : (C) 2003, 2007 by Thomas Baumgart
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

#include <config-kmymoney.h>

#include "daterangedlg.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneydateinput.h"
#include "kmymoneymvccombo.h"

#include "ui_daterangedlgdecl.h"

using namespace eMyMoney;

DateRangeDlg::DateRangeDlg(QWidget *parent) :
    QWidget(parent),
    m_ui(new Ui::DateRangeDlgDecl)
{
  m_ui->setupUi(this);
  setupDatePage();
}

DateRangeDlg::~DateRangeDlg()
{
  delete m_ui;
}

void DateRangeDlg::slotReset()
{
  m_ui->m_dateRange->setCurrentItem(TransactionFilter::Date::All);
  setDateRange(TransactionFilter::Date::All);
}

void DateRangeDlg::setupDatePage()
{
  for (auto i = (int)TransactionFilter::Date::All; i < (int)TransactionFilter::Date::LastDateItem; ++i) {
    MyMoneyTransactionFilter::translateDateRange(static_cast<TransactionFilter::Date>(i), m_startDates[i], m_endDates[i]);
  }

  connect(m_ui->m_dateRange, SIGNAL(currentIndexChanged(int)), this, SLOT(slotDateRangeSelectedByUser()));
  connect(m_ui->m_fromDate, SIGNAL(dateChanged(QDate)), this, SLOT(slotDateChanged()));
  connect(m_ui->m_toDate, SIGNAL(dateChanged(QDate)), this, SLOT(slotDateChanged()));

  setDateRange(TransactionFilter::Date::All);
}

void DateRangeDlg::slotDateRangeSelectedByUser()
{
  setDateRange(m_ui->m_dateRange->currentItem());
}

void DateRangeDlg::setDateRange(const QDate& from, const QDate& to)
{
    m_ui->m_fromDate->loadDate(from);
    m_ui->m_toDate->loadDate(to);
    m_ui->m_dateRange->setCurrentItem(TransactionFilter::Date::UserDefined);
    emit rangeChanged();
}

void DateRangeDlg::setDateRange(TransactionFilter::Date idx)
{
  m_ui->m_dateRange->setCurrentItem(idx);
  switch (idx) {
    case TransactionFilter::Date::All:
      m_ui->m_fromDate->loadDate(QDate());
      m_ui->m_toDate->loadDate(QDate());
      break;
    case TransactionFilter::Date::UserDefined:
      break;
    default:
      m_ui->m_fromDate->blockSignals(true);
      m_ui->m_toDate->blockSignals(true);
      m_ui->m_fromDate->loadDate(m_startDates[(int)idx]);
      m_ui->m_toDate->loadDate(m_endDates[(int)idx]);
      m_ui->m_fromDate->blockSignals(false);
      m_ui->m_toDate->blockSignals(false);
      break;
  }
  emit rangeChanged();
}

TransactionFilter::Date DateRangeDlg::dateRange() const
{
    return m_ui->m_dateRange->currentItem();
}

void DateRangeDlg::slotDateChanged()
{
  m_ui->m_dateRange->blockSignals(true);
  m_ui->m_dateRange->setCurrentItem(TransactionFilter::Date::UserDefined);
  m_ui->m_dateRange->blockSignals(false);
}

QDate DateRangeDlg::fromDate() const
{
    return m_ui->m_fromDate->date();
}

QDate DateRangeDlg::toDate() const
{
    return m_ui->m_toDate->date();
}
