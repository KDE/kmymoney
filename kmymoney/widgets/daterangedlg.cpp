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

#include <QLabel>
#include <QButtonGroup>
#include <QRadioButton>
#include <QCheckBox>
#include <QTimer>
#include <QTabWidget>
#include <QLayout>
#include <QKeyEvent>
#include <QList>
#include <QResizeEvent>
#include <QEvent>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QVBoxLayout>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klineedit.h>
#include <kcombobox.h>
#include <kstandardguiitem.h>
#include <khelpclient.h>
#include <KGuiItem>
#include <KStandardGuiItem>
#include <KConfigGroup>
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoneydateinput.h>
#include <kmymoneyedit.h>
#include <kmymoneylineedit.h>
#include <kmymoneyaccountselector.h>
#include <mymoneyfile.h>
#include <kmymoneyglobalsettings.h>
#include <transaction.h>

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

void DateRangeDlg::slotUpdateSelections(QString &txt)
{
  if (m_ui->m_dateRange->currentItem() != 0) {
    if (!txt.isEmpty())
      txt += ", ";
    txt += i18n("Date");
  }
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
  setDateRange(static_cast<MyMoneyTransactionFilter::dateOptionE>(m_ui->m_dateRange->currentData().toInt()));
}

void DateRangeDlg::setDateRange(MyMoneyTransactionFilter::dateOptionE idx)
{
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
//  slotUpdateSelections();
}

void DateRangeDlg::slotDateChanged()
{
  m_ui->m_dateRange->blockSignals(true);
  m_ui->m_dateRange->setCurrentItem(static_cast<MyMoneyTransactionFilter::dateOptionE>(MyMoneyTransactionFilter::userDefined));
  m_ui->m_dateRange->blockSignals(false);
//  slotUpdateSelections();
}
