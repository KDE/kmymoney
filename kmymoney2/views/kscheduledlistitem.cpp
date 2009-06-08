/***************************************************************************
                          kscheduledlistitem.cpp  -  description
                             -------------------
    begin                : Sun Jan 27 2002
    copyright            : (C) 2000-2002 by Michael Edwardes
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

#include <QPainter>
#include <QStyle>
//Added by qt3to4:
#include <QList>
#include <QPixmap>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kconfig.h>
#include <klocale.h>
#include <kglobal.h>
#include <kglobalsettings.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kscheduledlistitem.h"
#include "mymoneyfile.h"
#include "kmymoneyglobalsettings.h"
#include "kmymoneyutils.h"

KScheduledListItem::KScheduledListItem(K3ListView *parent, const QString& name, const QPixmap& pixmap, const QString& sortKey) :
  K3ListViewItem(parent, name),
  m_sortKey(sortKey)
{
  setPixmap(0, pixmap);
  if(m_sortKey.isEmpty())
    m_sortKey = name;
}

KScheduledListItem::KScheduledListItem(KScheduledListItem *parent, const MyMoneySchedule& schedule/*, bool even*/)
 : K3ListViewItem(parent)
{
  m_schedule = schedule;
  m_sortKey = schedule.name();
  setPixmap(0, KMyMoneyUtils::scheduleIcon(KIconLoader::Small));

  try
  {
    MyMoneyTransaction transaction = schedule.transaction();
    MyMoneySplit s1 = transaction.splits()[0];
    MyMoneySplit s2 = transaction.splits()[1];
    QList<MyMoneySplit>::ConstIterator it_s;
    MyMoneySplit split;
    MyMoneyAccount acc;

    switch(schedule.type()) {
      case MyMoneySchedule::TYPE_DEPOSIT:
        if (s1.value().isNegative())
          split = s2;
        else
          split = s1;
        break;

      case MyMoneySchedule::TYPE_LOANPAYMENT:
        for(it_s = transaction.splits().begin(); it_s != transaction.splits().end(); ++it_s) {
          acc = MyMoneyFile::instance()->account((*it_s).accountId());
          if(acc.accountGroup() == MyMoneyAccount::Asset
          || acc.accountGroup() == MyMoneyAccount::Liability) {
            if(acc.accountType() != MyMoneyAccount::Loan
            && acc.accountType() != MyMoneyAccount::AssetLoan) {
              split = *it_s;
              break;
            }
          }
        }
        if(it_s == transaction.splits().end()) {
          qFatal("Split for payment account not found in %s:%d.", __FILE__, __LINE__);
        }
        break;

      default:
        if (!s1.value().isPositive())
          split = s1;
        else
          split = s2;
        break;
    }
    acc = MyMoneyFile::instance()->account(split.accountId());

/*
    if (schedule.type() == MyMoneySchedule::TYPE_DEPOSIT)
    {
      if (s1.value() >= 0)
        split = s1;
      else
        split = s2;
    }
    else if(schedule.type() == MyMoneySchedule::TYPE_LOANPAYMENT)
    {

    }
    else
    {
      if (s1.value() < 0)
        split = s1;
      else
        split = s2;
    }
*/
    setText(0, schedule.name());
    MyMoneySecurity currency = MyMoneyFile::instance()->currency(acc.currencyId());

    setText(1, acc.name());
    if(!s1.payeeId().isEmpty())
      setText(2, MyMoneyFile::instance()->payee(s1.payeeId()).name());
    else
      setText(2, "---");
    m_amount = split.shares().abs();
    setText(3, QString("%1  ").arg(m_amount.formatMoney(acc, currency)));
    // Do the real next payment like ms-money etc
    if (schedule.isFinished())
    {
      setText(4, i18n("Finished"));
    }
    else
      setText(4, KGlobal::locale()->formatDate(schedule.nextDueDate()));

    setText(5, i18n(schedule.occurenceToString().toLatin1()));
    setText(6, KMyMoneyUtils::paymentMethodToString(schedule.paymentType()));
  }
  catch (MyMoneyException *e)
  {
    setText(0, "Error:");
    setText(1, e->what());
    delete e;
  }
}

KScheduledListItem::~KScheduledListItem()
{
}

void KScheduledListItem::paintCell(QPainter* p, const QColorGroup& cg, int column, int width, int align)
{
  QColorGroup cg2(cg);

  QColor textColour;
#warning "port to kde4";  
  //QColor textColour = KGlobalSettings::textColor();
  QFont cellFont = KMyMoneyGlobalSettings::listCellFont();

  // avoid colorizing lines that do not contain a schedule
  if(!m_schedule.id().isEmpty()) {
    if (m_schedule.isFinished())
      textColour = Qt::darkGreen;
    else if (m_schedule.isOverdue())
      textColour = Qt::red;
  }

  cg2.setColor(QColorGroup::Text, textColour);

  // display group items in bold
  if (!parent())
    cellFont.setBold(true);

  p->setFont(cellFont);

  if (isAlternate())
    cg2.setColor(QColorGroup::Base, KMyMoneyGlobalSettings::listColor());
  else
    cg2.setColor(QColorGroup::Base, KMyMoneyGlobalSettings::listBGColor());

  Q3ListViewItem::paintCell(p, cg2, column, width, align);
}

int KScheduledListItem::compare(Q3ListViewItem* i, int col, bool ascending) const
{
  KScheduledListItem* item = dynamic_cast<KScheduledListItem*>(i);
  int rc;
  // do special sorting only if
  // a) date
  // b) amount
  // c) name/group
  // d) occurence
  // in all other cases use the standard sorting
  MyMoneyMoney diff;
  switch(col) {
    case 0:   // type and name
      rc = m_sortKey.compare(item->m_sortKey);
      break;

    case 3:   // amount
      diff = m_amount - item->m_amount;
      if(diff.isZero())
        rc = 0;
      else if(diff.isPositive())
        rc = 1;
      else
        rc = -1;
      break;

    case 4:   // date
      rc = item->m_schedule.nextDueDate().daysTo(m_schedule.nextDueDate());
      break;

    case 5:   // occurence
      rc = (m_schedule.occurence() - item->m_schedule.occurence());
      break;

    default:
      rc = K3ListViewItem::compare(i, col, ascending);
      break;
  }
  // adjust to [-1..1]
  if(rc != 0) {
    rc = (rc > 0) ? 1 : -1;
  }
  return rc;
}
