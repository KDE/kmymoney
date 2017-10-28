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
#include <QToolButton>
#include <QList>
#include <QPushButton>
#include <QIcon>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>
#include <KGuiItem>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneymoney.h"
#include "mymoneyaccount.h"
#include "mymoneyschedule.h"
#include "mymoneysplit.h"
#include "mymoneytransaction.h"
#include "kmymoneyutils.h"
#include "icons/icons.h"

using namespace Icons;

KMyMoneyBriefSchedule::KMyMoneyBriefSchedule(QWidget *parent)
    : kScheduleBriefWidget(parent/*,name, Qt::WStyle_Customize | Qt::WStyle_NoBorder*/),
      m_index(0)
{
  m_nextButton->setIcon(QIcon::fromTheme(g_Icons[Icon::ArrowRight]));
  m_prevButton->setIcon(QIcon::fromTheme(g_Icons[Icon::ArrowLeft]));

  connect(m_prevButton, SIGNAL(clicked()), this, SLOT(slotPrevClicked()));
  connect(m_nextButton, SIGNAL(clicked()), this, SLOT(slotNextClicked()));
  connect(m_closeButton, SIGNAL(clicked()), this, SLOT(hide()));
  connect(m_skipButton, SIGNAL(clicked()), this, SLOT(slotSkipClicked()));
  connect(m_buttonEnter, SIGNAL(clicked()), this, SLOT(slotEnterClicked()));

  KGuiItem skipGuiItem(i18n("&Skip"),
                       QIcon::fromTheme(g_Icons[Icon::MediaSeekForward]),
                       i18n("Skip this transaction"),
                       i18n("Use this button to skip this transaction"));
  KGuiItem::assign(m_skipButton, skipGuiItem);

  KGuiItem enterGuiItem(i18n("&Enter"),
                        QIcon::fromTheme(g_Icons[Icon::KeyEnter]),
                        i18n("Record this transaction into the register"),
                        i18n("Use this button to record this transaction"));
  KGuiItem::assign(m_buttonEnter, enterGuiItem);
}

KMyMoneyBriefSchedule::~KMyMoneyBriefSchedule()
{
}

void KMyMoneyBriefSchedule::setSchedules(QList<MyMoneySchedule> list, const QDate& date)
{
  m_scheduleList = list;
  m_date = date;

  m_index = 0;
  if (list.count() >= 1) {
    loadSchedule();
  }
}

void KMyMoneyBriefSchedule::loadSchedule()
{
  try {
    if (m_index < m_scheduleList.count()) {
      MyMoneySchedule sched = m_scheduleList[m_index];

      m_indexLabel->setText(i18n("%1 of %2", m_index + 1, m_scheduleList.count()));
      m_name->setText(sched.name());
      m_type->setText(KMyMoneyUtils::scheduleTypeToString(sched.type()));
      m_account->setText(sched.account().name());
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

      m_details->setText(text);

      m_prevButton->setEnabled(true);
      m_nextButton->setEnabled(true);
      m_skipButton->setEnabled(sched.occurrencePeriod() != eMyMoney::Schedule::Occurrence::Once);

      if (m_index == 0)
        m_prevButton->setEnabled(false);
      if (m_index == (m_scheduleList.count() - 1))
        m_nextButton->setEnabled(false);
    }
  } catch (const MyMoneyException &) {
  }
}

void KMyMoneyBriefSchedule::slotPrevClicked()
{
  if (m_index >= 1) {
    --m_index;
    loadSchedule();
  }
}

void KMyMoneyBriefSchedule::slotNextClicked()
{
  if (m_index < (m_scheduleList.count() - 1)) {
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

