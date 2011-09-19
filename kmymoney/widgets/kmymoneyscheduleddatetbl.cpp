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

#include "kmymoneyscheduleddatetbl.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>
#include <QPen>
#include <QPainter>
#include <QDialog>
#include <QCursor>
#include <QApplication>
#include <QList>
#include <QMouseEvent>
#include <QDesktopWidget>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kglobal.h>
#include <kglobalsettings.h>
#include <kcolorscheme.h>
#include <klocale.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyfile.h"

kMyMoneyScheduledDateTbl::kMyMoneyScheduledDateTbl(QWidget *parent, QDate date_)
    : kMyMoneyDateTbl(parent, date_),
    m_filterBills(false), m_filterDeposits(false), m_filterTransfers(false)
{
  connect(&briefWidget, SIGNAL(enterClicked(MyMoneySchedule,QDate)), this, SIGNAL(enterClicked(MyMoneySchedule,QDate)));
  connect(&briefWidget, SIGNAL(skipClicked(MyMoneySchedule,QDate)), this, SIGNAL(skipClicked(MyMoneySchedule,QDate)));
}

kMyMoneyScheduledDateTbl::~kMyMoneyScheduledDateTbl()
{
}

void kMyMoneyScheduledDateTbl::drawCellContents(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index, const QDate& theDate)
{
  Q_UNUSED(index)
  QString text;
  QPen pen;
  QFont font = KGlobalSettings::generalFont();
  MyMoneyFile *file = MyMoneyFile::instance();

  const QStyle *style = QApplication::style();

  // -----
  font.setPointSize(fontsize);
  QFont fontLarge(font);
  QFont fontSmall(font);
  fontLarge.setPointSize(fontsize*2);
  fontSmall.setPointSize(fontsize - 1);

  painter->save();

  painter->setFont(font);

  if (m_type == MONTHLY) {
    text = QString::number(theDate.day());
    addDayPostfix(text);
    style->drawItemText(painter, option.rect, Qt::AlignRight, option.palette, true, text);

    MyMoneyFile *file = MyMoneyFile::instance();
    QList<MyMoneySchedule> schedules;
    try {
      // Honour the filter.
      int scheduleTypes = 0;
      int scheduleOcurrences = 0;
      int schedulePaymentTypes = 0;

      scheduleOcurrences |= MyMoneySchedule::OCCUR_ANY;
      schedulePaymentTypes |= MyMoneySchedule::STYPE_ANY;

      if (!m_filterBills) {
        scheduleTypes |= MyMoneySchedule::TYPE_BILL;
      }
      if (!m_filterDeposits) {
        scheduleTypes |= MyMoneySchedule::TYPE_DEPOSIT;
      }
      if (!m_filterTransfers) {
        scheduleTypes |= MyMoneySchedule::TYPE_TRANSFER;
      }

      schedules = file->scheduleListEx(scheduleTypes,
                                       scheduleOcurrences,
                                       schedulePaymentTypes,
                                       theDate,
                                       m_filterAccounts);
    } catch (MyMoneyException* e) {
      // SAfe to ignore here, cause no schedules might exist
      // for the selected account
      delete e;
    }

    if (schedules.count() >= 1) {
      QList<MyMoneySchedule>::Iterator iter;
      bool anyOverdue = false;
      for (iter = schedules.begin(); iter != schedules.end(); ++iter) {
        MyMoneySchedule schedule = *iter;
        if (theDate < QDate::currentDate()) {
          if (schedule.isOverdue()) {
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
      style->drawItemText(painter, option.rect, Qt::AlignCenter, option.palette, true, QString::number(schedules.count()));
    }
  } else if (m_type == WEEKLY) {
    text = QString::number(theDate.day());
    addDayPostfix(text);
    style->drawItemText(painter, option.rect, Qt::AlignRight, option.palette, true, QDate::shortDayName(theDate.dayOfWeek()) + ' ' + text);

    QList<MyMoneySchedule> billSchedules;
    QList<MyMoneySchedule> depositSchedules;
    QList<MyMoneySchedule> transferSchedules;
    try {
      text.clear();

      if (!m_filterBills) {
        billSchedules = file->scheduleListEx(MyMoneySchedule::TYPE_BILL,
                                             MyMoneySchedule::OCCUR_ANY,
                                             MyMoneySchedule::STYPE_ANY,
                                             theDate,
                                             m_filterAccounts);

        if (billSchedules.count() >= 1) {
          text += i18n("%1 Bills.", billSchedules.count());
        }
      }

      if (!m_filterDeposits) {
        depositSchedules = file->scheduleListEx(MyMoneySchedule::TYPE_DEPOSIT,
                                                MyMoneySchedule::OCCUR_ANY,
                                                MyMoneySchedule::STYPE_ANY,
                                                theDate,
                                                m_filterAccounts);

        if (depositSchedules.count() >= 1) {
          if (!text.isEmpty())
            text += "  ";
          text += i18n("%1 Deposits.", depositSchedules.count());
        }
      }

      if (!m_filterTransfers) {
        transferSchedules = file->scheduleListEx(MyMoneySchedule::TYPE_TRANSFER,
                            MyMoneySchedule::OCCUR_ANY,
                            MyMoneySchedule::STYPE_ANY,
                            theDate,
                            m_filterAccounts);

        if (transferSchedules.count() >= 1) {
          if (!text.isEmpty())
            text += "  ";
          text += i18n("%1 Transfers.", transferSchedules.count());
        }
      }
    } catch (MyMoneyException* e) {
      // SAfe to ignore here, cause no schedules might exist
      // for the selected account
      delete e;
    }

    bool anyOverdue = false;
    QList<MyMoneySchedule>::Iterator iter;
    for (iter = transferSchedules.begin(); iter != transferSchedules.end(); ++iter) {
      MyMoneySchedule schedule = *iter;
      if (theDate < QDate::currentDate()) {
        if (schedule.isOverdue()) {
          anyOverdue = true;
          break; // out early
        }
      }
    }

    if (!anyOverdue) {
      for (iter = depositSchedules.begin(); iter != depositSchedules.end(); ++iter) {
        MyMoneySchedule schedule = *iter;
        if (theDate < QDate::currentDate()) {
          if (schedule.isOverdue()) {
            anyOverdue = true;
            break; // out early
          }
        }
      }

      if (!anyOverdue) {
        for (iter = billSchedules.begin(); iter != billSchedules.end(); ++iter) {
          MyMoneySchedule schedule = *iter;
          if (theDate < QDate::currentDate()) {
            if (schedule.isOverdue()) {
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
    style->drawItemText(painter, option.rect, Qt::AlignCenter, option.palette, true, text);
  }

  painter->restore();
}

void kMyMoneyScheduledDateTbl::addDayPostfix(QString& text)
{
  int d = text.toInt();

  if (d >= 1 && d <= 31) {
    QStringList postfixList =
      i18nc("These are the suffix strings of the days in the calendar view; please make sure that you keep all the 30 separators (the '-' character) when translating",
            "st-nd-rd-th-th-th-th-th-th-th-th-th-th-th-th-th-th-th-th-th-st-nd-rd-th-th-th-th-th-th-th-st").split('-', QString::KeepEmptyParts);
    if (d <= postfixList.size()) // make sure that an improper translation will not cause a crash
      text += postfixList[d-1];
  }
}

void kMyMoneyScheduledDateTbl::refresh()
{
  update();
}

void kMyMoneyScheduledDateTbl::mouseMoveEvent(QMouseEvent* e)
{
  int row, col, pos;
  QPoint mouseCoord;

  if (isActiveWindow() || briefWidget.isVisible()) {
    mouseCoord = e->pos();
    row = rowAt(mouseCoord.y());
    col = columnAt(mouseCoord.x());
    if (row < 1 || col < 0) {
      return;
    }

    int firstWeekDay = KGlobal::locale()->weekStartDay();

    QDate drawDate(date);
    QString text;

    if (m_type == MONTHLY) {
      pos = 7 * (row - 1) + col;
      if (firstWeekDay < 4)
        pos += firstWeekDay;
      else
        pos += firstWeekDay - 7;

      if (pos < firstday || (firstday + numdays <= pos)) { // we are either
        //  painting a day of the previous month or
        //  painting a day of the following month

        if (pos < firstday) { // previous month
          drawDate = drawDate.addMonths(-1);
          text.setNum(numDaysPrevMonth + pos - firstday + 1);
          drawDate.setYMD(drawDate.year(), drawDate.month(), text.toInt());
        } else { // following month
          drawDate = drawDate.addMonths(1);
          text.setNum(pos - firstday - numdays + 1);
          drawDate.setYMD(drawDate.year(), drawDate.month(), text.toInt());
        }
      } else { // paint a day of the current month
        text.setNum(pos - firstday + 1);
        drawDate.setYMD(drawDate.year(), drawDate.month(), text.toInt());
      }
    } else if (m_type == WEEKLY) {
      // TODO: Handle other start weekdays than Monday
      text = QDate::shortDayName(row);
      text += ' ';

      int dayOfWeek = date.dayOfWeek();
      int diff;

      if (row < dayOfWeek) {
        diff = -(dayOfWeek - row);
      } else {
        diff = row - dayOfWeek;
      }

      drawDate = date.addDays(diff);
    }

    m_drawDateOrig = drawDate;
    MyMoneyFile *file = MyMoneyFile::instance();
    QList<MyMoneySchedule> schedules;

    try {
      int types = 0;

      if (!m_filterBills) {
        types |= MyMoneySchedule::TYPE_BILL;
      }

      if (!m_filterDeposits) {
        types |= MyMoneySchedule::TYPE_DEPOSIT;
      }

      if (!m_filterTransfers) {
        types |= MyMoneySchedule::TYPE_TRANSFER;
      }


      schedules = file->scheduleListEx(types,
                                       MyMoneySchedule::OCCUR_ANY,
                                       MyMoneySchedule::STYPE_ANY,
                                       drawDate,
                                       m_filterAccounts);
    } catch (MyMoneyException* e) {
      // SAfe to ignore here, cause no schedules might exist
      // for the selected account
      delete e;
    }

    if (schedules.count() >= 1) {
      briefWidget.setSchedules(schedules, drawDate);

      int h = briefWidget.height();
      int w = briefWidget.width();

      // Take off five pixels so the mouse cursor
      // will be over the widget
      QPoint p = QCursor::pos();
      if (p.y() + h > QApplication::desktop()->height()) {
        p.setY(p.y() - (h - 5));
      } else
        p.setY(p.y() - 5);

      if (p.x() + w > QApplication::desktop()->width()) {
        p.setX(p.x() - (w - 5));
      } else
        p.setX(p.x() - 5);

      briefWidget.move(p);
      briefWidget.show();
    } else {
      briefWidget.hide();
    }
  }
}

void kMyMoneyScheduledDateTbl::filterBills(bool enable)
{
  m_filterBills = enable;
  update();
}

void kMyMoneyScheduledDateTbl::filterDeposits(bool enable)
{
  m_filterDeposits = enable;
  update();
}

void kMyMoneyScheduledDateTbl::filterTransfers(bool enable)
{
  m_filterTransfers = enable;
  update();
}
