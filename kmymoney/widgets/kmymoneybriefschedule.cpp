/***************************************************************************
                          kmymoneybriefschedule.cpp  -  description
                             -------------------
    begin                : Sun Jul 6 2003
    copyright            : (C) 2000-2003 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
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

#include "kmymoneybriefschedule.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDate>
#include <QLabel>
#include <QToolButton>
#include <QList>
#include <QPushButton>
#include <QIcon>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kmymoneybriefschedule.h"

#include "mymoneyexception.h"
#include "mymoneymoney.h"
#include "mymoneyaccount.h"
#include "mymoneyschedule.h"
#include "mymoneysplit.h"
#include "mymoneytransaction.h"
#include "kmymoneyutils.h"
#include "icons/icons.h"
#include "mymoneyenums.h"

using namespace Icons;

class KMyMoneyBriefSchedulePrivate
{
  Q_DISABLE_COPY(KMyMoneyBriefSchedulePrivate)

public:
  KMyMoneyBriefSchedulePrivate() :
    ui(new Ui::KMyMoneyBriefSchedule),
    m_index(0)
  {
  }

  ~KMyMoneyBriefSchedulePrivate()
  {
    delete ui;
  }

  void loadSchedule()
  {
    try {
      if (m_index < m_scheduleList.count()) {
        MyMoneySchedule sched = m_scheduleList[m_index];

        ui->m_indexLabel->setText(i18n("%1 of %2", m_index + 1, m_scheduleList.count()));
        ui->m_name->setText(sched.name());
        ui->m_type->setText(KMyMoneyUtils::scheduleTypeToString(sched.type()));
        ui->m_account->setText(sched.account().name());
        QString text;
        MyMoneyMoney amount = sched.transaction().splitByAccount(sched.account().id()).value();
        amount = amount.abs();

        if (sched.willEnd()) {
          int transactions = sched.paymentDates(m_date, sched.endDate()).count() - 1;
          text = i18np("Payment on %2 for %3 with %1 transaction remaining occurring %4.",
                       "Payment on %2 for %3 with %1 transactions remaining occurring %4.",
                       transactions,
                       QLocale().toString(m_date, QLocale::ShortFormat),
                       amount.formatMoney(sched.account().fraction()),
                       i18n(sched.occurrenceToString().toLatin1()));
        } else {
          text = i18n("Payment on %1 for %2 occurring %3.",
                      QLocale().toString(m_date, QLocale::ShortFormat),
                      amount.formatMoney(sched.account().fraction()),
                      i18n(sched.occurrenceToString().toLatin1()));
        }

        if (m_date < QDate::currentDate()) {
          if (sched.isOverdue()) {
            QDate startD = (sched.lastPayment().isValid()) ?
                           sched.lastPayment() :
                           sched.startDate();

            if (m_date.isValid())
              startD = m_date;

            int days = startD.daysTo(QDate::currentDate());
            int transactions = sched.paymentDates(startD, QDate::currentDate()).count();

            text += "<br><font color=red>";
            text += i18np("%1 day overdue", "%1 days overdue", days);
            text += QString(" ");
            text += i18np("(%1 occurrence.)", "(%1 occurrences.)", transactions);
            text += "</color>";
          }
        }

        ui->m_details->setText(text);

        ui->m_prevButton->setEnabled(true);
        ui->m_nextButton->setEnabled(true);
        ui->m_skipButton->setEnabled(sched.occurrencePeriod() != eMyMoney::Schedule::Occurrence::Once);

        if (m_index == 0)
          ui->m_prevButton->setEnabled(false);
        if (m_index == (m_scheduleList.count() - 1))
          ui->m_nextButton->setEnabled(false);
      }
    } catch (const MyMoneyException &) {
    }
  }


  Ui::KMyMoneyBriefSchedule *ui;
  QList<MyMoneySchedule> m_scheduleList;
  int m_index;
  QDate m_date;

};

KMyMoneyBriefSchedule::KMyMoneyBriefSchedule(QWidget *parent) :
  QWidget(parent),
  d_ptr(new KMyMoneyBriefSchedulePrivate)
{
  Q_D(KMyMoneyBriefSchedule);
  d->ui->setupUi(this);
  d->ui->m_nextButton->setIcon(Icons::get(Icon::ArrowRight));
  d->ui->m_prevButton->setIcon(Icons::get(Icon::ArrowLeft));
  d->ui->m_skipButton->setIcon(Icons::get(Icon::MediaSeekForward));
  d->ui->m_buttonEnter->setIcon(Icons::get(Icon::KeyEnter));

  connect(d->ui->m_prevButton, &QAbstractButton::clicked, this, &KMyMoneyBriefSchedule::slotPrevClicked);
  connect(d->ui->m_nextButton, &QAbstractButton::clicked, this, &KMyMoneyBriefSchedule::slotNextClicked);
  connect(d->ui->m_closeButton, &QAbstractButton::clicked, this, &QWidget::hide);
  connect(d->ui->m_skipButton, &QAbstractButton::clicked, this, &KMyMoneyBriefSchedule::slotSkipClicked);
  connect(d->ui->m_buttonEnter, &QAbstractButton::clicked, this, &KMyMoneyBriefSchedule::slotEnterClicked);
}

KMyMoneyBriefSchedule::~KMyMoneyBriefSchedule()
{
  Q_D(KMyMoneyBriefSchedule);
  delete d;
}

void KMyMoneyBriefSchedule::setSchedules(QList<MyMoneySchedule> list, const QDate& date)
{
  Q_D(KMyMoneyBriefSchedule);
  d->m_scheduleList = list;
  d->m_date = date;

  d->m_index = 0;
  if (list.count() >= 1) {
    d->loadSchedule();
  }
}

void KMyMoneyBriefSchedule::slotPrevClicked()
{
  Q_D(KMyMoneyBriefSchedule);
  if (d->m_index >= 1) {
    --d->m_index;
    d->loadSchedule();
  }
}

void KMyMoneyBriefSchedule::slotNextClicked()
{
  Q_D(KMyMoneyBriefSchedule);
  if (d->m_index < (d->m_scheduleList.count() - 1)) {
    d->m_index++;
    d->loadSchedule();
  }
}

void KMyMoneyBriefSchedule::slotEnterClicked()
{
  Q_D(KMyMoneyBriefSchedule);
  hide();
  emit enterClicked(d->m_scheduleList[d->m_index], d->m_date);
}

void KMyMoneyBriefSchedule::slotSkipClicked()
{
  Q_D(KMyMoneyBriefSchedule);
  hide();
  emit skipClicked(d->m_scheduleList[d->m_index], d->m_date);
}

