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

#include <QWidget>
#include <QDateTime>
#include <QDateEdit>

// ----------------------------------------------------------------------------
// KDE Includes

#include <khbox.h>

// ----------------------------------------------------------------------------
// Project Includes

// Ideas neatly taken from korganizer
// Respective authors are credited.
// Some ideas/code have been borrowed from Calendar-0.13 (phoenix.bmedesign.com/~qt)

/**
  * Provided to be able to catch the focusOut events before the contents gets changed
  */
class KMyMoneyDateEdit : public QDateEdit
{
  Q_OBJECT
public:
  explicit KMyMoneyDateEdit(const QDate& date, QWidget *parent=0) : QDateEdit(date, parent) {}

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
class kMyMoneyDateInput : public KHBox
{
  Q_OBJECT

public:
  explicit kMyMoneyDateInput(QWidget *parent=0, Qt::AlignmentFlag flags=Qt::AlignLeft);
  ~kMyMoneyDateInput();

  QDate date(void) const;
  void setDate(QDate date);
  void loadDate(const QDate& date);
  void resetDate(void);
  QWidget* focusWidget(void) const;
  void setRange(const QDate & min, const QDate & max);
  void markAsBadDate(bool bad = false, const QColor& = QColor());

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
  void keyPressEvent(QKeyEvent* k);
  void resizeEvent(QResizeEvent*);
  void showEvent(QShowEvent* event);

  /** To intercept events sent to focusWidget() */
  bool eventFilter(QObject *o, QEvent *e);

protected slots:
  void slotDateChosen(QDate date);
  void toggleDatePicker();

private slots:
  void slotDateChosenRef(const QDate& date);
  void fixSize(void);

private:
  struct Private;
  Private * const d;
};

#endif

