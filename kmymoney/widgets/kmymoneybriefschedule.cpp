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

#include <QLabel>
#include <QLineEdit>
#include <q3textedit.h>
#include <QToolButton>
#include <QList>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kglobal.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kpushbutton.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyscheduled.h"
#include "kmymoneyutils.h"

KMyMoneyBriefSchedule::KMyMoneyBriefSchedule(QWidget *parent)
    : kScheduleBriefWidget(parent/*,name, Qt::WStyle_Customize | Qt::WStyle_NoBorder*/)
{
  m_nextButton->setPixmap(BarIcon(QString::fromLatin1("arrow-right")));
  m_prevButton->setPixmap(BarIcon(QString::fromLatin1("arrow-left")));

  connect(m_prevButton, SIGNAL(clicked()), this, SLOT(slotPrevClicked()));
  connect(m_nextButton, SIGNAL(clicked()), this, SLOT(slotNextClicked()));
  connect(m_closeButton, SIGNAL(clicked()), this, SLOT(hide()));
  connect(m_skipButton, SIGNAL(clicked()), this, SLOT(slotSkipClicked()));
  connect(m_buttonEnter, SIGNAL(clicked()), this, SLOT(slotEnterClicked()));

  KGuiItem skipGuiItem(  i18n("&Skip"),
                          KIcon("media-seek-forward"),
                          i18n("Skip this transaction"),
                          i18n("Use this button to skip this transaction"));
  m_skipButton->setGuiItem(skipGuiItem);

  KGuiItem enterGuiItem(  i18n("&Enter"),
                          KIcon("go-jump-locationbar"),
                          i18n("Record this transaction into the register"),
                          i18n("Use this button to record this transaction"));
  m_buttonEnter->setGuiItem(enterGuiItem);
}

KMyMoneyBriefSchedule::~KMyMoneyBriefSchedule()
{
}

void KMyMoneyBriefSchedule::setSchedules(QList<MyMoneySchedule> list, const QDate& date)
{
  m_scheduleList = list;
  m_date = date;

  m_index = 0;
  if (list.count() >= 1)
  {
    loadSchedule();
  }
}

void KMyMoneyBriefSchedule::loadSchedule()
{
  try
  {
    if (m_index < m_scheduleList.count())
    {
      MyMoneySchedule sched = m_scheduleList[m_index];

      m_indexLabel->setText(i18n("%1 of %2", m_index+1, m_scheduleList.count()));
      m_name->setText(sched.name());
      m_type->setText(KMyMoneyUtils::scheduleTypeToString(sched.type()));
      m_account->setText(sched.account().name());
      QString text;
      MyMoneyMoney amount = sched.transaction().splitByAccount(sched.account().id()).value();
      amount = amount.abs();

      if (sched.willEnd())
      {
        int transactions = sched.paymentDates(m_date, sched.endDate()).count()-1;
        text = i18n("Payment on %1 for %2 with %3 transactions remaining occurring %4.",
                KGlobal::locale()->formatDate(m_date),
                amount.formatMoney(sched.account().fraction()),
                QString::number(transactions),
                i18n(sched.occurrenceToString().toLatin1()));
      } else {
        text = i18n("Payment on %1 for %2 occurring %3.",
                KGlobal::locale()->formatDate(m_date),
                amount.formatMoney(sched.account().fraction()),
                i18n(sched.occurrenceToString().toLatin1()));
      }

      if (m_date < QDate::currentDate())
      {
        if (sched.isOverdue())
        {
          QDate startD = (sched.lastPayment().isValid()) ?
            sched.lastPayment() :
            sched.startDate();

          if (m_date.isValid())
            startD = m_date;

          int days = startD.daysTo(QDate::currentDate());
          int transactions = sched.paymentDates(startD, QDate::currentDate()).count();

          text += "<br><font color=red>";
          text += i18n("%1 days overdue (%2 occurrences).",
                      QString::number(days),
                      QString::number(transactions));
          text += "</color>";
        }
      }

      m_details->setText(text);

      m_prevButton->setEnabled(true);
      m_nextButton->setEnabled(true);
      m_skipButton->setEnabled(sched.occurrencePeriod() != MyMoneySchedule::OCCUR_ONCE);

      if (m_index == 0)
        m_prevButton->setEnabled(false);
      if (m_index == (m_scheduleList.count()-1))
        m_nextButton->setEnabled(false);
    }
  }
  catch (MyMoneyException *e)
  {
    delete e;
  }
}

void KMyMoneyBriefSchedule::slotPrevClicked()
{
  if (m_index >= 1)
  {
    --m_index;
    loadSchedule();
  }
}

void KMyMoneyBriefSchedule::slotNextClicked()
{
  if (m_index < (m_scheduleList.count()-1))
  {
    m_index++;
    loadSchedule();
  }
}

void KMyMoneyBriefSchedule::slotEnterClicked()
{
  hide();
  emit enterClicked(m_scheduleList[m_index], m_date);
}

void KMyMoneyBriefSchedule::slotSkipClicked()
{
  hide();
  emit skipClicked(m_scheduleList[m_index], m_date);
}

