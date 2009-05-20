/***************************************************************************
                          kmymoneyscheduleddatetbl.cpp  -  description
                             -------------------
    begin                : Thu Jul 3 2003
    copyright            : (C) 2000-2003 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
 ***************************************************************************/
 /****************************************************************************
 Contains code from the KDateTable class ala kdelibs-3.1.2.  Original license:

    This file is part of the KDE libraries
    Copyright (C) 1997 Tim D. Gilman (tdgilman@best.org)
              (C) 1998-2001 Mirko Boehm (mirko@kde.org)
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
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
#include <qstring.h>
#include <qpen.h>
#include <qpainter.h>
#include <qdialog.h>
#include <qdrawutil.h>
#include <qcursor.h>
#include <qapplication.h>
//Added by qt3to4:
#include <Q3ValueList>
#include <QMouseEvent>
#include <QDesktopWidget>
// ----------------------------------------------------------------------------
// KDE Includes
#include "kdecompat.h"
#include <kglobal.h>
#include <kglobalsettings.h>
#include <klocale.h>

// ----------------------------------------------------------------------------
// Project Includes
#include "kmymoneyscheduleddatetbl.h"
#include "../mymoney/mymoneyfile.h"

kMyMoneyScheduledDateTbl::kMyMoneyScheduledDateTbl(QWidget *parent, QDate date_, const char* name, Qt::WFlags f )
  : kMyMoneyDateTbl(parent, date_, name, f),
  m_filterBills(false), m_filterDeposits(false), m_filterTransfers(false)
{
  connect(&briefWidget, SIGNAL(enterClicked(const MyMoneySchedule&, const QDate&)), this, SIGNAL(enterClicked(const MyMoneySchedule&, const QDate&)));
  connect(&briefWidget, SIGNAL(skipClicked(const MyMoneySchedule&, const QDate&)), this, SIGNAL(skipClicked(const MyMoneySchedule&, const QDate&)));
}

kMyMoneyScheduledDateTbl::~kMyMoneyScheduledDateTbl()
{
}

void kMyMoneyScheduledDateTbl::drawCellContents(QPainter *painter, int /*row*/, int /*col*/, const QDate& theDate)
{
  QRect rect;
  QString text;
  int w=cellWidth();
  int h=cellHeight();
  QPen pen;
  QBrush brushBlue(KGlobalSettings::activeTitleColor());
  QBrush brushLightblue(KGlobalSettings::baseColor());
  QFont font=KGlobalSettings::generalFont();
  MyMoneyFile *file = MyMoneyFile::instance();

  // -----
  font.setPointSize(fontsize);
  QFont fontLarge(font);
  QFont fontSmall(font);
  fontLarge.setPointSize(fontsize*2);
  fontSmall.setPointSize(fontsize-1);

  painter->setFont(font);


  if (m_type == MONTHLY)
  {
    if (theDate.month() != date.month())
    {
      painter->setFont(fontSmall);
      pen = Qt::lightGray;
    }
    else
    {
      pen = Qt::gray;
    }

    if (theDate == date)
    {
      if (hasFocus())
      { // draw the currently selected date
        painter->setPen(KGlobalSettings::highlightColor());
        painter->setBrush(KGlobalSettings::highlightColor());
        pen=Qt::white;
      } else {
        painter->setPen(KGlobalSettings::calculateAlternateBackgroundColor(KGlobalSettings::highlightColor()));
        painter->setBrush(KGlobalSettings::calculateAlternateBackgroundColor(KGlobalSettings::highlightColor()));
        pen=Qt::white;
      }
    } else {
      painter->setBrush(KGlobalSettings::baseColor());
      painter->setPen(KGlobalSettings::baseColor());
    }
    painter->drawRect(0, 0, w, h);
    painter->setPen(pen);
    text = QString::number(theDate.day());
    addDayPostfix(text);
    painter->drawText(0, 0, w-2, h, Qt::AlignRight, text, -1, &rect);

    MyMoneyFile *file = MyMoneyFile::instance();
    Q3ValueList<MyMoneySchedule> schedules;
    try
    {

      // Honour the filter.
      int scheduleTypes=0;
      int scheduleOcurrences=0;
      int schedulePaymentTypes=0;

      scheduleOcurrences |= MyMoneySchedule::OCCUR_ANY;
      schedulePaymentTypes |= MyMoneySchedule::STYPE_ANY;

      if (!m_filterBills)
      {
        scheduleTypes |= MyMoneySchedule::TYPE_BILL;
      }
      if (!m_filterDeposits)
      {
        scheduleTypes |= MyMoneySchedule::TYPE_DEPOSIT;
      }
      if (!m_filterTransfers)
      {
        scheduleTypes |= MyMoneySchedule::TYPE_TRANSFER;
      }

      schedules = file->scheduleListEx( scheduleTypes,
                                        scheduleOcurrences,
                                        schedulePaymentTypes,
                                        theDate,
                                        m_filterAccounts);
    }
    catch ( MyMoneyException* e)
    {
      // SAfe to ignore here, cause no schedules might exist
      // for the selected account
      delete e;
    }

    if (schedules.count() >= 1)
    {
      Q3ValueList<MyMoneySchedule>::Iterator iter;
      bool anyOverdue=false;
      for (iter=schedules.begin(); iter!=schedules.end(); ++iter)
      {
        MyMoneySchedule schedule = *iter;
        if (theDate < QDate::currentDate())
        {
          if (schedule.isOverdue())
          {
            anyOverdue = true;
            break; // out early
          }
        }
      }

      if (anyOverdue)
        painter->setPen(Qt::red);
      else
        painter->setPen(Qt::darkGray);

      painter->setFont(fontLarge);
      painter->drawText(0, 0, w, h, Qt::AlignCenter, QString::number(schedules.count()),
          -1, &rect);
    }

    painter->setPen(Qt::lightGray);
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(0, 0, w, h);
  }
  else if (m_type == WEEKLY)
  {
    // TODO: Handle other start weekdays than Monday
    if (theDate == date)
    {
      painter->setBrush(KGlobalSettings::highlightColor());
    }
    else
    {
      painter->setBrush(KGlobalSettings::baseColor());
      painter->setPen(KGlobalSettings::baseColor());
    }

    painter->setPen(Qt::lightGray);
    painter->drawRect(0, 0, w, h);

    text = QString::number(theDate.day());
    addDayPostfix(text);

    painter->drawText(0, 0, w-2, h, Qt::AlignRight, QDate::shortDayName(theDate.dayOfWeek()) + " " + text, -1, &rect);

    Q3ValueList<MyMoneySchedule> billSchedules;
    Q3ValueList<MyMoneySchedule> depositSchedules;
    Q3ValueList<MyMoneySchedule> transferSchedules;
    try
    {
      text = QString();

      if (!m_filterBills)
      {
        billSchedules = file->scheduleListEx( MyMoneySchedule::TYPE_BILL,
                                          MyMoneySchedule::OCCUR_ANY,
                                          MyMoneySchedule::STYPE_ANY,
                                          theDate,
                                          m_filterAccounts);

        if (billSchedules.count() >= 1)
        {
          text += i18n("%1 Bills.").arg(QString::number(billSchedules.count()));
        }
      }

      if (!m_filterDeposits)
      {
        depositSchedules = file->scheduleListEx( MyMoneySchedule::TYPE_DEPOSIT,
                                          MyMoneySchedule::OCCUR_ANY,
                                          MyMoneySchedule::STYPE_ANY,
                                          theDate,
                                          m_filterAccounts);

        if (depositSchedules.count() >= 1)
        {
          if(!text.isEmpty())
            text += "  ";
          text += i18n("%1 Deposits.").arg(QString::number(depositSchedules.count()));
        }
      }

      if (!m_filterTransfers)
      {
        transferSchedules = file->scheduleListEx( MyMoneySchedule::TYPE_TRANSFER,
                                          MyMoneySchedule::OCCUR_ANY,
                                          MyMoneySchedule::STYPE_ANY,
                                          theDate,
                                          m_filterAccounts);

        if (transferSchedules.count() >= 1)
        {
          if(!text.isEmpty())
            text += "  ";
          text += i18n("%1 Transfers.").arg(QString::number(transferSchedules.count()));
        }
      }
    }
    catch (MyMoneyException* e)
    {
      // SAfe to ignore here, cause no schedules might exist
      // for the selected account
      delete e;
    }

    bool anyOverdue=false;
    Q3ValueList<MyMoneySchedule>::Iterator iter;
    for (iter=transferSchedules.begin(); iter!=transferSchedules.end(); ++iter)
    {
      MyMoneySchedule schedule = *iter;
      if (theDate < QDate::currentDate())
      {
        if (schedule.isOverdue())
        {
          anyOverdue = true;
          break; // out early
        }
      }
    }

    if (!anyOverdue)
    {
      for (iter=depositSchedules.begin(); iter!=depositSchedules.end(); ++iter)
      {
        MyMoneySchedule schedule = *iter;
        if (theDate < QDate::currentDate())
        {
          if (schedule.isOverdue())
          {
            anyOverdue = true;
            break; // out early
          }
        }
      }

      if (!anyOverdue)
      {
        for (iter=billSchedules.begin(); iter!=billSchedules.end(); ++iter)
        {
          MyMoneySchedule schedule = *iter;
          if (theDate < QDate::currentDate())
          {
            if (schedule.isOverdue())
            {
              anyOverdue = true;
              break; // out early
            }
          }
        }
      }
    }

    if (anyOverdue)
      painter->setPen(Qt::red);
    else
      painter->setPen(Qt::darkGray);

    painter->setFont(fontLarge);
    painter->drawText(0, 0, w, h, Qt::AlignCenter, text,
          -1, &rect);
  }
  else if (m_type == QUARTERLY)
  {
    painter->setBrush(KGlobalSettings::baseColor());

    painter->setPen(Qt::lightGray);
    painter->drawRect(0, 0, w, h);
  }
}

void kMyMoneyScheduledDateTbl::addDayPostfix(QString& text)
{
  int d = text.toInt();

  if (d >= 1 && d <= 31)
  {
    QStringList postfixList = QStringList::split("-", i18n("st-nd-rd-th-th-th-th-th-th-th-th-th-th-th-th-th-th-th-th-th-st-nd-rd-th-th-th-th-th-th-th-st"), true);
    text += postfixList[d-1];
  }
}

void kMyMoneyScheduledDateTbl::refresh()
{
  repaintContents(false);
}

void kMyMoneyScheduledDateTbl::contentsMouseMoveEvent(QMouseEvent* e)
{
  int row, col, pos;
  QPoint mouseCoord;

  if (isActiveWindow() || briefWidget.isVisible())
  {
    mouseCoord = e->pos();
    row = rowAt(mouseCoord.y());
    col = columnAt(mouseCoord.x());
    if (row<1 || col<0)
    {
      return;
    }

  #if KDE_VERSION < 310
    int firstWeekDay = KGlobal::locale()->weekStartsMonday() ? 1 : 0;
  #else
    int firstWeekDay = KGlobal::locale()->weekStartDay();
  #endif

    QDate drawDate(date);
    QString text;

    if (m_type == MONTHLY)
    {
      pos=7*(row-1)+col;
      if ( firstWeekDay < 4 )
        pos += firstWeekDay;
      else
        pos += firstWeekDay - 7;

      if (pos<firstday || (firstday+numdays<=pos))
      { // we are either
        //  painting a day of the previous month or
        //  painting a day of the following month

        if (pos<firstday)
        { // previous month
          drawDate = drawDate.addMonths(-1);
          text.setNum(numDaysPrevMonth+pos-firstday+1);
          drawDate.setYMD(drawDate.year(), drawDate.month(), text.toInt());
        } else { // following month
          drawDate = drawDate.addMonths(1);
          text.setNum(pos-firstday-numdays+1);
          drawDate.setYMD(drawDate.year(), drawDate.month(), text.toInt());
        }
      } else { // paint a day of the current month
        text.setNum(pos-firstday+1);
        drawDate.setYMD(drawDate.year(), drawDate.month(), text.toInt());
      }
    }
    else if (m_type == WEEKLY)
    {
      // TODO: Handle other start weekdays than Monday
      text = QDate::shortDayName(row);
      text += " ";

      int dayOfWeek = date.dayOfWeek();
      int diff;

      if (row < dayOfWeek)
      {
        diff = -(dayOfWeek - row);
      }
      else
      {
        diff = row - dayOfWeek;
      }

      drawDate = date.addDays(diff);
    }
    else if (m_type == QUARTERLY)
    {
    }

    m_drawDateOrig = drawDate;
    MyMoneyFile *file = MyMoneyFile::instance();
    Q3ValueList<MyMoneySchedule> schedules;

    try
    {
      int types=0;

      if (!m_filterBills)
      {
        types |= MyMoneySchedule::TYPE_BILL;
      }

      if (!m_filterDeposits)
      {
        types |= MyMoneySchedule::TYPE_DEPOSIT;
      }

      if (!m_filterTransfers)
      {
        types |= MyMoneySchedule::TYPE_TRANSFER;
      }


      schedules = file->scheduleListEx( types,
                                        MyMoneySchedule::OCCUR_ANY,
                                        MyMoneySchedule::STYPE_ANY,
                                        drawDate,
                                        m_filterAccounts);
    }
    catch ( MyMoneyException* e)
    {
      // SAfe to ignore here, cause no schedules might exist
      // for the selected account
      delete e;
    }

    if (schedules.count() >= 1)
    {
      briefWidget.setSchedules(schedules, drawDate);

      int h = briefWidget.height();
      int w = briefWidget.width();

      // Take off five pixels so the mouse cursor
      // will be over the widget
      QPoint p = QCursor::pos();
      if (p.y() + h > QApplication::desktop()->height())
      {
        p.setY(p.y() - (h-5));
      }
      else
        p.setY(p.y() - 5);

      if (p.x() + w > QApplication::desktop()->width())
      {
        p.setX(p.x() - (w-5));
      }
      else
        p.setX(p.x() - 5);

      briefWidget.move(p);
      briefWidget.show();
    }
    else
    {
      briefWidget.hide();
    }
  }
}

void kMyMoneyScheduledDateTbl::filterBills(bool enable)
{
  m_filterBills = enable;
  repaintContents(false);
}

void kMyMoneyScheduledDateTbl::filterDeposits(bool enable)
{
  m_filterDeposits = enable;
  repaintContents(false);
}

void kMyMoneyScheduledDateTbl::filterTransfers(bool enable)
{
  m_filterTransfers = enable;
  repaintContents(false);
}
