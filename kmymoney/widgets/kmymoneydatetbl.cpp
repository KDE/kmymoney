/***************************************************************************
                          kmymoneydatetbl.cpp  -  description
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
   the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA
*/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kmymoneydatetbl.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDateTime>
#include <QString>
#include <QPen>
#include <QPainter>
#include <QDialog>
#include <QWheelEvent>
#include <QResizeEvent>
#include <QFocusEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QEvent>
#include <QHeaderView>
#include <QDebug>
#include <QFontDatabase>

// ----------------------------------------------------------------------------
// KDE Includes

//#include <kdatetable.h> // Use the classes available for maximum re-use
#include <kapplication.h>
#include <klocale.h>
#include <knotification.h>
#include <kcalendarsystem.h>
#include <KColorScheme>

// ----------------------------------------------------------------------------
// Project Includes

#define WEEK_DAY_NAME(a,b)  KLocale::global()->calendar()->weekDayName(a,b)

KMyMoneyDateTbDelegate::KMyMoneyDateTbDelegate(kMyMoneyDateTbl* parent): QStyledItemDelegate(parent), m_parent(parent)
{
}

KMyMoneyDateTbDelegate::~KMyMoneyDateTbDelegate()
{
}

void KMyMoneyDateTbDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  QStyleOptionViewItemV4 opt = option;
  initStyleOption(&opt, index);
  const QStyle *style = QApplication::style();

  QFont font = QFontDatabase::systemFont(QFontDatabase::GeneralFont);
  font.setPointSize(m_parent->fontsize);

  int firstWeekDay = KLocale::global()->weekStartDay();

  if (index.row() == 0) { // we are drawing the headline
    QStyledItemDelegate::paint(painter, opt, index);

    if (m_parent->m_type == kMyMoneyDateTbl::MONTHLY) {
      font.setBold(true);
      painter->setFont(font);
      bool normalday = true;
      QString daystr;

      if (index.column() + firstWeekDay < 8)
        daystr = WEEK_DAY_NAME(index.column() + firstWeekDay, KCalendarSystem::ShortDayName);
      else
        daystr = WEEK_DAY_NAME(index.column() + firstWeekDay - 7, KCalendarSystem::ShortDayName);

      if (daystr == i18nc("Sunday", "Sun") || daystr == i18nc("Saturday", "Sat"))
        normalday = false;

      painter->fillRect(opt.rect, normalday ? QBrush(opt.palette.color(QPalette::Highlight)) : QBrush(opt.palette.color(QPalette::Base)));
      style->drawItemText(painter, opt.rect, Qt::AlignCenter, opt.palette, true, daystr, normalday ? QPalette::HighlightedText : QPalette::Text);
    } else if (m_parent->m_type == kMyMoneyDateTbl::WEEKLY) {
      int year = m_parent->date.year();
      QString headerText;
      QString weekStr = QString::number(m_parent->date.weekNumber(&year));
      QString yearStr = QString::number(year);
      headerText = i18n("Week %1 for year %2.", weekStr, yearStr);
      painter->fillRect(opt.rect, QBrush(opt.palette.color(QPalette::Highlight)));
      style->drawItemText(painter, opt.rect, Qt::AlignCenter, opt.palette, true, headerText, QPalette::HighlightedText);
    }
  } else {
    int pos = 0;
    QString text;
    QDate drawDate(m_parent->date);

    if (m_parent->m_type == kMyMoneyDateTbl::MONTHLY) {
      pos = 7 * (index.row() - 1) + index.column();
      if (firstWeekDay < 4)
        pos += firstWeekDay;
      else
        pos += firstWeekDay - 7;

      if (pos < m_parent->firstday || (m_parent->firstday + m_parent->numdays <= pos)) { // we are either
        //  painting a day of the previous month or
        //  painting a day of the following month

        if (pos < m_parent->firstday) { // previous month
          drawDate = drawDate.addMonths(-1);
          text.setNum(m_parent->numDaysPrevMonth + pos - m_parent->firstday + 1);
          drawDate.setYMD(drawDate.year(), drawDate.month(), text.toInt());
        } else { // following month
          drawDate = drawDate.addMonths(1);
          text.setNum(pos - m_parent->firstday - m_parent->numdays + 1);
          drawDate.setYMD(drawDate.year(), drawDate.month(), text.toInt());
        }
      } else { // paint a day of the current month
        text.setNum(pos - m_parent->firstday + 1);
        drawDate.setYMD(drawDate.year(), drawDate.month(), text.toInt());
      }
    } else if (m_parent->m_type == kMyMoneyDateTbl::WEEKLY) {
      // TODO: Handle other start weekdays than Monday
      text = QDate::shortDayName(index.row());
      text += ' ';

      int dayOfWeek = m_parent->date.dayOfWeek();
      int diff;

      if (index.row() < dayOfWeek) {
        diff = -(dayOfWeek - index.row());
      } else {
        diff = index.row() - dayOfWeek;
      }

      drawDate = m_parent->date.addDays(diff);
    }
    if (drawDate == m_parent->date) {
      opt.state |= QStyle::State_Selected;
      opt.state |= QStyle::State_HasFocus;
    } else {
      opt.state &= ~QStyle::State_Selected;
      opt.state &= ~QStyle::State_HasFocus;
    }
    QStyledItemDelegate::paint(painter, opt, index);
    m_parent->drawCellContents(painter, opt, index, drawDate);
  }
}

kMyMoneyDateTbl::kMyMoneyDateTbl(QWidget *parent, QDate date_)
    : QTableWidget(parent), m_colCount(0), m_rowCount(0), m_itemDelegate(new KMyMoneyDateTbDelegate(this))
{
  // call this first to make sure that member variables are initialized
  setType(MONTHLY);

  setFontSize(10);

  if (!date_.isValid()) {
    qDebug() << "kMyMoneyDateTbl ctor: WARNING: Given date is invalid, using current date.";
    date_ = QDate::currentDate();
  }
  setFocusPolicy(Qt::StrongFocus);

  setDate(date_); // this initializes firstday, numdays, numDaysPrevMonth

  viewport()->setMouseTracking(true);

  horizontalHeader()->setResizeMode(QHeaderView::Fixed);
  horizontalHeader()->hide();
  verticalHeader()->setResizeMode(QHeaderView::Fixed);
  verticalHeader()->hide();
}

void kMyMoneyDateTbl::keyPressEvent(QKeyEvent *e)
{
  if (e->key() == Qt::Key_PageUp) {
    setDate(date.addMonths(-1));
    return;
  }
  if (e->key() == Qt::Key_PageDown) {
    setDate(date.addMonths(1));
    return;
  }

  if (e->key() == Qt::Key_Up) {
    if (date.day() > 7) {
      setDate(date.addDays(-7));
      return;
    }
  }
  if (e->key() == Qt::Key_Down) {
    if (date.day() <= date.daysInMonth() - 7) {
      setDate(date.addDays(7));
      return;
    }
  }
  if (e->key() == Qt::Key_Left) {
    if (date.day() > 1) {
      setDate(date.addDays(-1));
      return;
    }
  }
  if (e->key() == Qt::Key_Right) {
    if (date.day() < date.daysInMonth()) {
      setDate(date.addDays(1));
      return;
    }
  }

  if (e->key() == Qt::Key_Minus) {
    setDate(date.addDays(-1));
    return;
  }
  if (e->key() == Qt::Key_Plus) {
    setDate(date.addDays(1));
    return;
  }
  if (e->key() == Qt::Key_N) {
    setDate(QDate::currentDate());
    return;
  }
  KNotification::beep();
}

void kMyMoneyDateTbl::resizeEvent(QResizeEvent * e)
{
  if (e)
    QTableWidget::resizeEvent(e);

  if (m_colCount > 0) {
    horizontalHeader()->setDefaultSectionSize(viewport()->width() / m_colCount + 1);
    horizontalHeader()->setStretchLastSection(true);
  }
  if (m_rowCount > 0) {
    verticalHeader()->setDefaultSectionSize(viewport()->height() / m_rowCount + 1);
    verticalHeader()->setStretchLastSection(true);
  }
}

void kMyMoneyDateTbl::setFontSize(int size)
{
  fontsize = size;
}

void kMyMoneyDateTbl::wheelEvent(QWheelEvent * e)
{
  setDate(date.addMonths(-(int)(e->delta() / 120)));
  e->accept();
}

void kMyMoneyDateTbl::mouseReleaseEvent(QMouseEvent *e)
{
  QAbstractItemView::mouseReleaseEvent(e);

  if (e->type() != QEvent::MouseButtonRelease) { // the KDatePicker only reacts on mouse press events:
    return;
  }

  if (!isEnabled()) {
    KNotification::beep();
    return;
  }

  int dayoff = KLocale::global()->weekStartDay();

  // -----
  int row, col, pos;
  QPoint mouseCoord;

  // -----
  mouseCoord = e->pos();
  row = rowAt(mouseCoord.y());
  col = columnAt(mouseCoord.x());
  if (row < 1 || col < 0) { // the user clicked on the frame of the table
    return;
  }

  if (m_type == MONTHLY) {
    // Rows and columns are zero indexed.  The (row - 1) below is to avoid counting
    // the row with the days of the week in the calculation.  We however want pos
    // to be "1 indexed", hence the "+ 1" at the end of the sum.
    pos = (7 * (row - 1)) + col + 1;

    // This gets pretty nasty below.  firstday is a locale independent index for
    // the first day of the week.  dayoff is the day of the week that the week
    // starts with for the selected locale.  Monday = 1 .. Sunday = 7
    // Strangely, in some cases dayoff is in fact set to 8, hence all of the
    // "dayoff % 7" sections below.

    if (pos + dayoff % 7 <= firstday) { // this day is in the previous month
      setDate(date.addDays(-1 *(date.day() + firstday - pos - dayoff % 7)));
      return;
    }

    if (firstday + numdays < pos + dayoff % 7) { // this date is in the next month
      setDate(date.addDays(pos - firstday - date.day() + dayoff % 7));
      return;
    }

    setDate(QDate(date.year(), date.month(), pos - firstday + dayoff % 7));
  } else if (m_type == WEEKLY) {
    int dayOfWeek = date.dayOfWeek();
    int diff;

    if (row < dayOfWeek) {
      diff = -(dayOfWeek - row);
    } else {
      diff = row - dayOfWeek;
    }

    setDate(date.addDays(diff));
  }

  emit(tableClicked());
}

bool kMyMoneyDateTbl::setDate(const QDate& date_)
{
  bool changed = false;
  QDate temp;
  // -----
  if (!date_.isValid()) {
    qDebug() << "kMyMoneyDateTbl::setDate: refusing to set invalid date.";
    return false;
  }

  if (date != date_) {
    date = date_;
    changed = true;
  }

  temp.setYMD(date.year(), date.month(), 1);
  firstday = temp.dayOfWeek();

  if (firstday == 1)
    firstday = 8;

  numdays = date.daysInMonth();

  if (date.month() == 1) { // set to december of previous year
    temp.setYMD(date.year() - 1, 12, 1);
  } else { // set to previous month
    temp.setYMD(date.year(), date.month() - 1, 1);
  }

  numDaysPrevMonth = temp.daysInMonth();

  if (changed) {
    viewport()->update();
  }

  emit(dateChanged(date));
  return true;
}

const QDate& kMyMoneyDateTbl::getDate() const
{
  return date;
}

// what are those repaintContents() good for? (pfeiffer)
void kMyMoneyDateTbl::focusInEvent(QFocusEvent *e)
{
//    repaintContents(false);
  QTableWidget::focusInEvent(e);
}

void kMyMoneyDateTbl::focusOutEvent(QFocusEvent *e)
{
//    repaintContents(false);
  QTableWidget::focusOutEvent(e);
}

void kMyMoneyDateTbl::setType(calendarType type)
{
  if (type == WEEKLY) {
    m_rowCount = 8;
    m_colCount = 1;
    m_type = WEEKLY;
  } else { // default to monthly
    m_rowCount = m_colCount = 7;
    m_type = MONTHLY;
  }

  setRowCount(m_rowCount);
  setColumnCount(m_colCount);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  // setup the item delegate for all the rows
  for (int i = 0; i < rowCount(); ++i) {
    setItemDelegateForRow(i, m_itemDelegate);
  }

  resizeEvent(0);
}

void kMyMoneyDateTbl::mouseMoveEvent(QMouseEvent* e)
{
  int row, col, pos;
  QPoint mouseCoord;

  mouseCoord = e->pos();
  row = rowAt(mouseCoord.y());
  col = columnAt(mouseCoord.x());
  if (row < 1 || col < 0) {
    return;
  }

  int firstWeekDay = KLocale::global()->weekStartDay();

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

  if (m_drawDateOrig != drawDate) {
    m_drawDateOrig = drawDate;
    emit hoverDate(drawDate);
  }

  QTableWidget::mouseMoveEvent(e);
}
