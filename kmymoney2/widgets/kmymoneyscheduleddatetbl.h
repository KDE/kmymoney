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

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "../widgets/kmymoneydatetbl.h"
#include "../widgets/kmymoneybriefschedule.h"
#include "../mymoney/mymoneyscheduled.h"
//Added by qt3to4:
#include <QMouseEvent>

/**
  * @author Michael Edwardes
  */

class kMyMoneyScheduledDateTbl : public kMyMoneyDateTbl
{
  Q_OBJECT
public:
  kMyMoneyScheduledDateTbl(QWidget *parent=0,
         QDate date=QDate::currentDate(),
         const char* name=0, Qt::WFlags f=0);

  ~kMyMoneyScheduledDateTbl();
  void refresh();
  void filterBills(bool enable);
  void filterDeposits(bool enable);
  void filterTransfers(bool enable);
  void setFilterAccounts(const QStringList& list) { m_filterAccounts = list; repaintContents(false); }

signals:
  void enterClicked(const MyMoneySchedule&, const QDate&);
  void skipClicked(const MyMoneySchedule&, const QDate&);

protected:
  void drawCellContents(QPainter *painter, int row, int col, const QDate& theDate);
  void addDayPostfix(QString& text);
  void contentsMouseMoveEvent(QMouseEvent* e);

private:
  bool m_filterBills, m_filterDeposits, m_filterTransfers;
  QStringList m_filterAccounts;
  KMyMoneyBriefSchedule briefWidget;
};

#endif
