/*
    SPDX-FileCopyrightText: 2002-2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "daterangedlg.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneytransactionfilter.h"
#include "mymoneyenums.h"
#include "kmymoneydateinput.h"

#include "ui_daterangedlg.h"

using namespace eMyMoney;

class DateRangeDlgPrivate
{
  Q_DISABLE_COPY(DateRangeDlgPrivate)
  Q_DECLARE_PUBLIC(DateRangeDlg)

public:
  explicit DateRangeDlgPrivate(DateRangeDlg *qq) :
    q_ptr(qq),
    ui(new Ui::DateRangeDlg)
  {
  }

  ~DateRangeDlgPrivate()
  {
    delete ui;
  }

  void setupDatePage()
  {
    Q_Q(DateRangeDlg);
    for (auto i = (int)TransactionFilter::Date::All; i < (int)TransactionFilter::Date::LastDateItem; ++i) {
      MyMoneyTransactionFilter::translateDateRange(static_cast<TransactionFilter::Date>(i), m_startDates[i], m_endDates[i]);
    }

    q->connect(ui->m_dateRange, static_cast<void (KMyMoneyPeriodCombo::*)(int)>(&KMyMoneyPeriodCombo::currentIndexChanged), q, &DateRangeDlg::slotDateRangeSelectedByUser);
    q->connect(ui->m_fromDate, &KMyMoneyDateInput::dateChanged, q, &DateRangeDlg::slotDateChanged);
    q->connect(ui->m_toDate, &KMyMoneyDateInput::dateChanged, q, &DateRangeDlg::slotDateChanged);

    q->setDateRange(TransactionFilter::Date::All);
  }

  DateRangeDlg     *q_ptr;
  Ui::DateRangeDlg *ui;
  QDate             m_startDates[(int)eMyMoney::TransactionFilter::Date::LastDateItem];
  QDate             m_endDates[(int)eMyMoney::TransactionFilter::Date::LastDateItem];
};

DateRangeDlg::DateRangeDlg(QWidget *parent) :
  QWidget(parent),
  d_ptr(new DateRangeDlgPrivate(this))
{
  Q_D(DateRangeDlg);
  d->ui->setupUi(this);
  d->setupDatePage();
}

DateRangeDlg::~DateRangeDlg()
{
  Q_D(DateRangeDlg);
  delete d;
}

void DateRangeDlg::slotReset()
{
  Q_D(DateRangeDlg);
  d->ui->m_dateRange->setCurrentItem(TransactionFilter::Date::All);
  setDateRange(TransactionFilter::Date::All);
}

void DateRangeDlg::slotDateRangeSelectedByUser()
{
  Q_D(DateRangeDlg);
  setDateRange(d->ui->m_dateRange->currentItem());
}

void DateRangeDlg::setDateRange(const QDate& from, const QDate& to)
{
  Q_D(DateRangeDlg);
  d->ui->m_fromDate->loadDate(from);
  d->ui->m_toDate->loadDate(to);
  d->ui->m_dateRange->setCurrentItem(TransactionFilter::Date::UserDefined);
  emit rangeChanged();
}

void DateRangeDlg::setDateRange(TransactionFilter::Date idx)
{
  Q_D(DateRangeDlg);
  d->ui->m_dateRange->setCurrentItem(idx);
  switch (idx) {
    case TransactionFilter::Date::All:
      d->ui->m_fromDate->loadDate(QDate());
      d->ui->m_toDate->loadDate(QDate());
      break;
    case TransactionFilter::Date::UserDefined:
      break;
    default:
      d->ui->m_fromDate->blockSignals(true);
      d->ui->m_toDate->blockSignals(true);
      d->ui->m_fromDate->loadDate(d->m_startDates[(int)idx]);
      d->ui->m_toDate->loadDate(d->m_endDates[(int)idx]);
      d->ui->m_fromDate->blockSignals(false);
      d->ui->m_toDate->blockSignals(false);
      break;
  }
  emit rangeChanged();
}

TransactionFilter::Date DateRangeDlg::dateRange() const
{
  Q_D(const DateRangeDlg);
  return d->ui->m_dateRange->currentItem();
}

void DateRangeDlg::slotDateChanged()
{
  Q_D(DateRangeDlg);
  d->ui->m_dateRange->blockSignals(true);
  d->ui->m_dateRange->setCurrentItem(TransactionFilter::Date::UserDefined);
  d->ui->m_dateRange->blockSignals(false);
}

QDate DateRangeDlg::fromDate() const
{
  Q_D(const DateRangeDlg);
  return d->ui->m_fromDate->date();
}

QDate DateRangeDlg::toDate() const
{
  Q_D(const DateRangeDlg);
  return d->ui->m_toDate->date();
}
