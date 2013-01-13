/***************************************************************************
                          kmymoneyscheduleddatetbl.h  -  description
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

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef KMYMONEYSCHEDULEDDATETBL_H
#define KMYMONEYSCHEDULEDDATETBL_H


// ----------------------------------------------------------------------------
// QT Includes

#include <QMouseEvent>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneydatetbl.h"
#include "kmymoneybriefschedule.h"
#include "mymoneyschedule.h"

/**
  * @author Michael Edwardes
  */

class kMyMoneyScheduledDateTbl : public kMyMoneyDateTbl
{
  Q_OBJECT
public:
  explicit kMyMoneyScheduledDateTbl(QWidget *parent = 0,
                                    QDate date = QDate::currentDate());

  ~kMyMoneyScheduledDateTbl();
  void refresh();
  void filterBills(bool enable);
  void filterDeposits(bool enable);
  void filterTransfers(bool enable);
  void setFilterAccounts(const QStringList& list) {
    m_filterAccounts = list; update();
  }

signals:
  void enterClicked(const MyMoneySchedule&, const QDate&);
  void skipClicked(const MyMoneySchedule&, const QDate&);

protected:
  void drawCellContents(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index, const QDate& theDate);
  void addDayPostfix(QString& text);
  void mouseMoveEvent(QMouseEvent* e);

private:
  bool m_filterBills, m_filterDeposits, m_filterTransfers;
  QStringList m_filterAccounts;
  KMyMoneyBriefSchedule briefWidget;
};

#endif
