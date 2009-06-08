/***************************************************************************
                          kmymoneydatetbl.h  -  description
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
#ifndef KMYMONEYDATETBL_H
#define KMYMONEYDATETBL_H

// ----------------------------------------------------------------------------
// QT Includes
#include <q3gridview.h>
#include <QDateTime>
//Added by qt3to4:
#include <QResizeEvent>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QFocusEvent>

// ----------------------------------------------------------------------------
// KDE Includes



// ----------------------------------------------------------------------------
// Project Includes



/**
  * @author Michael Edwardes
  */
class kMyMoneyDateTbl : public Q3GridView  {
   Q_OBJECT
public:
  enum calendarType { WEEKLY,
                      MONTHLY,
                      QUARTERLY };

public:
    /**
     * The constructor.
     */
    kMyMoneyDateTbl(QWidget *parent=0,
         QDate date=QDate::currentDate(),
         const char* name=0, Qt::WFlags f=0);
    /**
     * Returns a recommended size for the widget.
     * To save some time, the size of the largest used cell content is
     * calculated in each paintCell() call, since all calculations have
     * to be done there anyway. The size is stored in maxCell. The
     * sizeHint() simply returns a multiple of maxCell.
     */
    virtual QSize sizeHint() const;
    /**
     * Set the font size of the date table.
     */
    virtual void setFontSize(int size);
    /**
     * Select and display this date.
     */
    virtual bool setDate(const QDate&);
    virtual const QDate& getDate() const;

    virtual void setType(calendarType type);
    virtual calendarType type(void) const { return m_type; }

signals:
    /**
     * The selected date changed.
     */
    void dateChanged(QDate);
    /**
     * A date has been selected by clicking on the table.
     */
    void tableClicked();

    /**
      *
    **/
    virtual void hoverDate(QDate);

protected:
    /**
     * Paint a cell.
     */
    virtual void paintCell(QPainter*, int, int);
    /**
     * Handle the resize events.
     */
    virtual void viewportResizeEvent(QResizeEvent *);
    /**
     * React on mouse clicks that select a date.
     */
    virtual void contentsMouseReleaseEvent(QMouseEvent *);
    virtual void wheelEvent( QWheelEvent * e );
    virtual void keyPressEvent( QKeyEvent *e );
    virtual void focusInEvent( QFocusEvent *e );
    virtual void focusOutEvent( QFocusEvent *e );

    virtual void drawCellContents(QPainter *painter, int row, int col, const QDate& theDate) = 0;

    virtual void contentsMouseMoveEvent(QMouseEvent* e);

    /**
     * The font size of the displayed text.
     */
    int fontsize;
    /**
     * The currently selected date.
     */
    QDate date;
    /**
     * The day of the first day in the month [1..7].
     */
    int firstday;
    /**
     * The number of days in the current month.
     */
    int numdays;
    /**
     * The number of days in the previous month.
     */
    int numDaysPrevMonth;
    /**
     * unused
     * ### remove in KDE 4.0
     */
    bool unused_hasSelection;
    /**
     * Save the size of the largest used cell content.
     */
    QRect maxCell;

    /**
      * Type related variables
    **/
    calendarType m_type;
    int m_colCount;
    int m_rowCount;

    ///
    QDate m_drawDateOrig;

private:
  #define WEEK_DAY_NAME(a,b)  KGlobal::locale()->calendar()->weekDayName(a,b)
};

#endif
