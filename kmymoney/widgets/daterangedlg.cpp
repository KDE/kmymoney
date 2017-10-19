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
  m_ui->m_dateRange->setCurrentItem(MyMoneyTransactionFilter::allDates);
  setDateRange(MyMoneyTransactionFilter::allDates);
}

void DateRangeDlg::setupDatePage()
{
  int i;
  for (i = MyMoneyTransactionFilter::allDates; i < MyMoneyTransactionFilter::dateOptionCount; ++i) {
    MyMoneyTransactionFilter::translateDateRange(static_cast<MyMoneyTransactionFilter::dateOptionE>(i), m_startDates[i], m_endDates[i]);
  }

  connect(m_ui->m_dateRange, SIGNAL(currentIndexChanged(int)), this, SLOT(slotDateRangeSelectedByUser()));
  connect(m_ui->m_fromDate, SIGNAL(dateChanged(QDate)), this, SLOT(slotDateChanged()));
  connect(m_ui->m_toDate, SIGNAL(dateChanged(QDate)), this, SLOT(slotDateChanged()));

  setDateRange(MyMoneyTransactionFilter::allDates);
}

void DateRangeDlg::slotDateRangeSelectedByUser()
{
  setDateRange(m_ui->m_dateRange->currentItem());
}

void DateRangeDlg::setDateRange(const QDate& from, const QDate& to)
{
    m_ui->m_fromDate->loadDate(from);
    m_ui->m_toDate->loadDate(to);
    m_ui->m_dateRange->setCurrentItem(MyMoneyTransactionFilter::userDefined);
    emit rangeChanged();
}

void DateRangeDlg::setDateRange(MyMoneyTransactionFilter::dateOptionE idx)
{
  m_ui->m_dateRange->setCurrentItem(idx);
  switch (idx) {
    case MyMoneyTransactionFilter::allDates:
      m_ui->m_fromDate->loadDate(QDate());
      m_ui->m_toDate->loadDate(QDate());
      break;
    case MyMoneyTransactionFilter::userDefined:
      break;
    default:
      m_ui->m_fromDate->blockSignals(true);
      m_ui->m_toDate->blockSignals(true);
      m_ui->m_fromDate->loadDate(m_startDates[idx]);
      m_ui->m_toDate->loadDate(m_endDates[idx]);
      m_ui->m_fromDate->blockSignals(false);
      m_ui->m_toDate->blockSignals(false);
      break;
  }
  emit rangeChanged();
}

MyMoneyTransactionFilter::dateOptionE DateRangeDlg::dateRange() const
{
    return m_ui->m_dateRange->currentItem();
}

void DateRangeDlg::slotDateChanged()
{
  m_ui->m_dateRange->blockSignals(true);
  m_ui->m_dateRange->setCurrentItem(MyMoneyTransactionFilter::userDefined);
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
