/***************************************************************************
                          kmymoneycalendar.h  -  description
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

/***************************************************************************
 * Contains code from KDatePicker in kdelibs-3.1.2.
 * Original license message:
 *
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
 ****************************************************************************/


#ifndef KMYMONEYCALENDAR_H
#define KMYMONEYCALENDAR_H

// ----------------------------------------------------------------------------
// QT Includes
#include <q3frame.h>
#include <QDateTime>
//Added by qt3to4:
#include <QResizeEvent>
#include <QEvent>
#include <QLineEdit>

// ----------------------------------------------------------------------------
// KDE Includes



// ----------------------------------------------------------------------------
// Project Includes
#include "kmymoneydatetbl.h"

class QToolButton;
class KDateValidator;
class kMyMoneyDateTbl;
class QPushButton;
class QIntValidator;
class KCalendarSystem;

/**
  * A representation of a calendar.
  *
  * @author Michael Edwardes 2003
  *
**/
class kMyMoneyCalendar : public Q3Frame  {
   Q_OBJECT
public:

public:
  /**
    * Standard constructor.
  **/
  explicit kMyMoneyCalendar(QWidget *parent=0);

  /**
    * Standard destructor.
  **/
  ~kMyMoneyCalendar();

  /**
    * Sets the calendar type.
  **/
  void setType(kMyMoneyDateTbl::calendarType type) { table->setType(type); }

  /**
    * Get the calendar type.
  **/
  kMyMoneyDateTbl::calendarType type(void) const { return table->type(); }

  /** The size hint for date pickers. The size hint recommends the
   *   minimum size of the widget so that all elements may be placed
   *  without clipping. This sometimes looks ugly, so when using the
   *  size hint, try adding 28 to each of the reported numbers of
   *  pixels.
   **/
  QSize sizeHint() const;

  /**
   * Sets the date.
   *
   *  @returns @p false and does not change anything
   *      if the date given is invalid.
   **/
  bool setDate(const QDate&);

  /**
   * Returns the selected date.
   * @deprecated
   **/
  const QDate& getDate() const;

  /**
   * @returns the selected date.
   */
  const QDate &date() const;

  /**
   * Enables or disables the widget.
   **/
  void setEnabled(bool);

  /**
   * Sets the font size of the widgets elements.
   **/
  void setFontSize(int);
  /**
   * Returns the font size of the widget elements.
   */
  int fontSize() const
    { return fontsize; }

  /**
   * By calling this method with @p enable = true, KDatePicker will show
   * a little close-button in the upper button-row. Clicking the
   * close-button will cause the KDatePicker's topLevelWidget()'s close()
   * method being called. This is mostly useful for toplevel datepickers
   * without a window manager decoration.
   * @see #hasCloseButton
   * @since 3.1
   */
  void setCloseButton( bool enable );

  /**
   * @returns true if a KDatePicker shows a close-button.
   * @see #setCloseButton
   * @since 3.1
   */
  bool hasCloseButton() const;

  /**
    * Dynamically set the Date Table
  **/
  virtual void setDateTable(kMyMoneyDateTbl *tbl) = 0;

  void setUserButton1(bool enable, QPushButton* pb);
  void setUserButton2(bool enable, QPushButton* pb);

protected:
  /// to catch move keyEvents when QLineEdit has keyFocus
  virtual bool eventFilter(QObject *o, QEvent *e );
  /// the resize event
  virtual void resizeEvent(QResizeEvent*);
  /// the style control button
  QPushButton *styleControl;
  /// the year forward button
  QToolButton *yearForward;
  /// the year backward button
  QToolButton *yearBackward;
  /// the month forward button
  QToolButton *monthForward;
  /// the month backward button
  QToolButton *monthBackward;
  /// the button for selecting the month directly
  QToolButton *selectMonth;
  /// the button for selecting the year directly
  QToolButton *selectYear;
  /// the line edit to enter the date directly
  QLineEdit *line;
  /// the validator for the line edit:
  KDateValidator *val;
  /// the date table
  kMyMoneyDateTbl *table;
  /// the size calculated during resize events
    //  QSize sizehint;
  /// the widest month string in pixels:
  QSize maxMonthRect;

protected slots:
  void dateChangedSlot(QDate);
  void tableClickedSlot();
  void monthForwardClicked();
  void monthBackwardClicked();
  void yearForwardClicked();
  void yearBackwardClicked();
  /// @since 3.1
  void selectWeekClicked();
  void selectMonthClicked();
  void selectYearClicked();
  void lineEnterPressed();

  void slotSetStyleWeekly();
  void slotSetStyleMonthly();
  void slotSetStyleQuarterly();

signals:
  /** This signal is emitted each time the selected date is changed.
   *  Usually, this does not mean that the date has been entered,
   *  since the date also changes, for example, when another month is
   *  selected.
   *  @see dateSelected
   */
  void dateChanged(QDate);
  /** This signal is emitted each time a day has been selected by
   *  clicking on the table (hitting a day in the current month). It
   *  has the same meaning as dateSelected() in older versions of
   *  KDatePicker.
   */
  void dateSelected(QDate);
  /** This signal is emitted when enter is pressed and a VALID date
   *  has been entered before into the line edit. Connect to both
   *  dateEntered() and dateSelected() to receive all events where the
   *  user really enters a date.
   */
  void dateEntered(QDate);
  /** This signal is emitted when the day has been selected by
   *  clicking on it in the table.
   */
  void tableClicked();

private:
  /// the font size for the widget
  int fontsize;

protected:
  virtual void virtual_hook( int id, void* data );
  void init( const QDate &dt );

private:
  class kMyMoneyCalendarPrivate;
  kMyMoneyCalendarPrivate* const d;
  // calculate ISO 8601 week number
  int weekOfYear(QDate);

#define MONTH_NAME(a,b,c)  KGlobal::locale()->calendar()->monthName(a,b,c)
};

//taken from kdatepicker_p.h until kmymoneycalendar is ported to not duplicate KDE code
class KDatePickerPrivateYearSelector : public QLineEdit
{
  Q_OBJECT

public:
  KDatePickerPrivateYearSelector(const KCalendarSystem *calendar, const QDate &currentDate, QWidget *parent = 0);
  int year();
  void setYear(int year);

public slots:
  void yearEnteredSlot();

signals:
  void closeMe(int);

protected:
  QIntValidator *val;
  int result;

private:
  const KCalendarSystem *calendar;
  QDate oldDate;

  Q_DISABLE_COPY(KDatePickerPrivateYearSelector)
};

class KDatePickerPrivateWeekSelector : public QLineEdit
{
  Q_OBJECT

public:
  KDatePickerPrivateWeekSelector(const KCalendarSystem *calendar, const QDate &currentDate, QWidget *parent = 0);
  int week();
  void setWeek(int week);

public slots:
  void weekEnteredSlot();

signals:
  void closeMe(int);

protected:
  QIntValidator *val;
  int result;

private:
  const KCalendarSystem *calendar;
  QDate oldDate;

  Q_DISABLE_COPY(KDatePickerPrivateWeekSelector)
};

#endif
