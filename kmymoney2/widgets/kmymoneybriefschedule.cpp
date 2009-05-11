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

// ----------------------------------------------------------------------------
// QT Includes

#include <qlabel.h>
#include <qlineedit.h>
#include <q3textedit.h>
#include <qtoolbutton.h>
//Added by qt3to4:
#include <Q3ValueList>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kglobal.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kpushbutton.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/mymoneyscheduled.h>
#include "kmymoneybriefschedule.h"
#include "../kmymoneyutils.h"

KMyMoneyBriefSchedule::KMyMoneyBriefSchedule(QWidget *parent, const char *name )
  : kScheduleBriefWidget(parent,name, Qt::WStyle_Customize | Qt::WStyle_NoBorder)
{
  KIconLoader *ic = KIconLoader::global();
  m_nextButton->setPixmap(BarIcon(QString::fromLatin1("1rightarrow")));
  m_prevButton->setPixmap(BarIcon(QString::fromLatin1("1leftarrow")));

  connect(m_prevButton, SIGNAL(clicked()), this, SLOT(slotPrevClicked()));
  connect(m_nextButton, SIGNAL(clicked()), this, SLOT(slotNextClicked()));
  connect(m_closeButton, SIGNAL(clicked()), this, SLOT(hide()));
  connect(m_skipButton, SIGNAL(clicked()), this, SLOT(slotSkipClicked()));
  connect(m_buttonEnter, SIGNAL(clicked()), this, SLOT(slotEnterClicked()));

  KGuiItem skipGuiItem(  i18n("&Skip"),
                          QIcon(ic->loadIcon("player_fwd", KIcon::Small, KIconLoader::SizeSmall)),
                          i18n("Skip this transaction"),
                          i18n("Use this button to skip this transaction"));
  m_skipButton->setGuiItem(skipGuiItem);

  KGuiItem enterGuiItem(  i18n("&Enter"),
                          QIcon(ic->loadIcon("key_enter", KIcon::Small, KIconLoader::SizeSmall)),
                          i18n("Record this transaction into the register"),
                          i18n("Use this button to record this transaction"));
  m_buttonEnter->setGuiItem(enterGuiItem);
}

KMyMoneyBriefSchedule::~KMyMoneyBriefSchedule()
{
}

void KMyMoneyBriefSchedule::setSchedules(Q3ValueList<MyMoneySchedule> list, const QDate& date)
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

      m_indexLabel->setText(i18n("%1 of %2")
                              .arg(QString::number(m_index+1))
                              .arg(QString::number(m_scheduleList.count())));
      m_name->setText(sched.name());
      m_type->setText(KMyMoneyUtils::scheduleTypeToString(sched.type()));
      m_account->setText(sched.account().name());
      QString text;
      MyMoneyMoney amount = sched.transaction().splitByAccount(sched.account().id()).value();
      amount = amount.abs();

      if (sched.willEnd())
      {
        int transactions = sched.paymentDates(m_date, sched.endDate()).count()-1;
        text = i18n("Payment on %1 for %2 with %3 transactions remaining occuring %4.")
                .arg(KGlobal::locale()->formatDate(m_date, true))
                .arg(amount.formatMoney(sched.account().fraction()))
                .arg(QString::number(transactions))
                .arg(i18n(sched.occurenceToString()));
      } else {
        text = i18n("Payment on %1 for %2 occuring %4.")
                .arg(KGlobal::locale()->formatDate(m_date, true))
                .arg(amount.formatMoney(sched.account().fraction()))
                .arg(i18n(sched.occurenceToString()));
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
          text += i18n("%1 days overdue (%2 occurences).")
                      .arg(QString::number(days))
                      .arg(QString::number(transactions));
          text += "</color>";
        }
      }

      m_details->setText(text);

      m_prevButton->setEnabled(true);
      m_nextButton->setEnabled(true);
      m_skipButton->setEnabled(sched.occurencePeriod() != MyMoneySchedule::OCCUR_ONCE);

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

#include "kmymoneybriefschedule.moc"
