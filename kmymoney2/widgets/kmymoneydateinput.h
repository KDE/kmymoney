/***************************************************************************
                          kmymoneydateinput.h
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes
    email                : mte@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KMYMONEYDATEINPUT_H
#define KMYMONEYDATEINPUT_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qwidget.h>
#include <qlineedit.h>
#include <qdatetime.h>
#include <q3datetimeedit.h>
#include <q3vbox.h>
//Added by qt3to4:
#include <QResizeEvent>
#include <QKeyEvent>
#include <QEvent>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kdatepicker.h>
class KPushButton;
class KPassivePopup;

// ----------------------------------------------------------------------------
// Project Includes

// Ideas neatly taken from korganizer
// Respective authors are credited.
// Some ideas/code have been borrowed from Calendar-0.13 (phoenix.bmedesign.com/~qt)

/**
  * Provided to be able to catch the focusOut events before the contents gets changed
  */
class KMyMoneyDateEdit : public Q3DateEdit
{
  Q_OBJECT
public:
  KMyMoneyDateEdit(const QDate& date, QWidget *parent=0, const char *name=0) : Q3DateEdit(date, parent, name) {}

protected:
  /** reimplemented for internal reasons */
  bool event(QEvent* e);
};

/**
  * This class provides the general widget used for date selection
  * throughout the KMyMoney project. It provides an QDateEdit widget
  * which is based on an edit field with spin boxes and adds a QPushButton
  * to open a KDatePicker.
  */
class kMyMoneyDateInput : public Q3HBox
{
  Q_OBJECT

public:
  kMyMoneyDateInput(QWidget *parent=0, const char *name=0, Qt::AlignmentFlags flags=Qt::AlignLeft);
  ~kMyMoneyDateInput();

  // Replace calls to this with the new date() method
  // QDate getQDate(void) KDE_DEPRECATED;

  QDate date(void) const;
  void setDate(QDate date);
  void loadDate(const QDate& date);
  void resetDate(void);
  QWidget* focusWidget(void) const;
  virtual void setRange(const QDate & min, const QDate & max) { dateEdit->setRange(min, max); }

public slots:
  virtual void show(void);

signals:
  void dateChanged(const QDate& date);

protected:
  /**
    * - increments/decrements the date upon +/- key input
    * - increments/decrements the date upon Up/Down key input
    * - sets the date to current date when the 'T' key is pressed.
    *   The actual key for this to happen might be overridden through
    *   an i18n package. The 'T'-key is always possible.
    */
  void keyPressEvent(QKeyEvent * k);
  void resizeEvent(QResizeEvent*);

  /** To intercept events sent to focusWidget() */
  bool eventFilter(QObject *o, QEvent *e);

protected slots:
  void slotDateChosen(QDate date);
  void toggleDatePicker();

private slots:
  void slotDateChosenRef(const QDate& date);
  void fixSize(void);

private:
  Q3DateEdit *dateEdit;
  KDatePicker *m_datePicker;
  QDate m_date;  // The date !
  QDate m_prevDate;
  Qt::AlignmentFlags m_qtalignment;
  Q3VBox *m_dateFrame;
  KPushButton *m_dateButton;
  KPassivePopup *m_datePopup;
  int m_focusDatePart;
};

#endif

