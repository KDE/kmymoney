/***************************************************************************
                          kmymoneyscheduledcalendar.cpp  -  description
                             -------------------
    begin                : Wed Jul 2 2003
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
#include <QPushButton>
#include <qkeysequence.h>

// ----------------------------------------------------------------------------
// KDE Includes
#include <klocale.h>
#include <kmenu.h>
#include <kmessagebox.h>

// ----------------------------------------------------------------------------
// Project Includes
#include "kmymoneyscheduledcalendar.h"
#include "mymoneyfile.h"

kMyMoneyScheduledCalendar::kMyMoneyScheduledCalendar(QWidget *parent, const char *name )
  : kMyMoneyCalendar(parent,name)
{
  QPushButton *pb1 = new QPushButton(i18n("Select Schedules"), this);

  kpopupmenu = new KMenu(this);
  kpopupmenu->addAction(i18n("Bills"), this, SLOT(slotSetViewBills()));
  kpopupmenu->addAction(i18n("Deposits"), this, SLOT(slotSetViewDeposits()));
  kpopupmenu->addAction(i18n("Transfers"), this, SLOT(slotSetViewTransfers()));

  foreach(QAction *a, kpopupmenu->actions()) {
    a->setCheckable(true);
    a->setChecked(true);
  }

  pb1->setMenu(kpopupmenu);

  m_scheduledDateTable = new kMyMoneyScheduledDateTbl(this);
  setDateTable((kMyMoneyDateTbl*)m_scheduledDateTable);

  setUserButton1(true, pb1);

  init( QDate::currentDate() );

  connect(m_scheduledDateTable, SIGNAL(enterClicked(const MyMoneySchedule&, const QDate&)),
    this, SIGNAL(enterClicked(const MyMoneySchedule&, const QDate&)));
  connect(m_scheduledDateTable, SIGNAL(skipClicked(const MyMoneySchedule&, const QDate&)),
    this, SIGNAL(skipClicked(const MyMoneySchedule&, const QDate&)));
}

kMyMoneyScheduledCalendar::~kMyMoneyScheduledCalendar()
{
}

void kMyMoneyScheduledCalendar::slotSetViewBills()
{
  m_scheduledDateTable->filterBills(!kpopupmenu->actions().value(0)->isChecked());
}

void kMyMoneyScheduledCalendar::slotSetViewDeposits()
{
  m_scheduledDateTable->filterDeposits(!kpopupmenu->actions().value(1)->isChecked());
}

void kMyMoneyScheduledCalendar::slotSetViewTransfers()
{
  m_scheduledDateTable->filterTransfers(!kpopupmenu->actions().value(2)->isChecked());
}

