/*
 * Copyright 2000-2003  Michael Edwardes <mte@users.sourceforge.net>
 * Copyright 2001       Felix Rodriguez <frodriguez@users.sourceforge.net>
 * Copyright 2017-2018  Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef KMYMONEYDATEINPUT_H
#define KMYMONEYDATEINPUT_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWidget>
#include <QDate>
#include <QDateEdit>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmm_base_widgets_export.h"

// Ideas neatly taken from korganizer
// Respective authors are credited.
// Some ideas/code have been borrowed from Calendar-0.13 (phoenix.bmedesign.com/~qt)

namespace KMyMoney {
  /**
    * Provided to be able to catch the focusOut events before the contents gets changed
    */
  class OldDateEdit : public QDateEdit
  {
    Q_OBJECT
  public:
    explicit OldDateEdit(const QDate& date, QWidget* parent = nullptr);
    void setInitialSection(Section section) { m_initialSection = section; }

  protected:
    /** if the date was cleared (a state which is not supported by QDateEdit)
      * make sure that a date can be entered again
      */
    void keyPressEvent(QKeyEvent* k) final override;

    /** reimplemented for internal reasons */
    bool event(QEvent* e) final override;

    /** reimplemented for internal reasons */
    bool focusNextPrevChild(bool next) final override;

    /** reimplemented for internal reasons */
    void focusInEvent(QFocusEvent *event) final override;

  private:
    QDateEdit::Section  m_initialSection;
  };
}; // namespace

/**
  * This class provides the general widget used for date selection
  * throughout the KMyMoney project. It provides an QDateEdit widget
  * which is based on an edit field with spin boxes and adds a QPushButton
  * to open a KDatePicker.
  */
class KMM_BASE_WIDGETS_EXPORT KMyMoneyDateInput : public QWidget
{
  Q_OBJECT
  Q_PROPERTY(QDate date READ date WRITE setDate STORED false)

public:
  explicit KMyMoneyDateInput(QWidget* parent = nullptr, Qt::AlignmentFlag flags = Qt::AlignLeft);
  ~KMyMoneyDateInput();

  /**
    * Returns the selected date in the widget. If the widget is not
    * showing a date, a QDate() object is returned which has an invalid date.
    */
  QDate date() const;

  /**
    * Set the date shown in the widget to @a date. If @a date is invalid,
    * no text will be shown. The internal widget will use 1.1.1800 for this
    * special case, as the standard QDateEdit widget does not support an
    * invalid date as of Qt4 anymore, but we need it anyway for multi transaction
    * edit.
    */
  void setDate(QDate date);

  void setMaximumDate(const QDate& max);

  /**
    * Setup the widget with @a date. This date is stored internally and
    * can be reloaded using resetDate().
    *
    * @sa setDate, resetDate
    */
  void loadDate(const QDate& date);

  /**
    * Setup the widget with the date loaded using loadDate().
    *
    *  @sa loadDate
    */
  void resetDate();

  QWidget* focusWidget() const;
  void setRange(const QDate & min, const QDate & max);
  void markAsBadDate(bool bad = false, const QColor& = QColor());

Q_SIGNALS:
  void dateChanged(const QDate& date);

protected:
  /**
    * - increments/decrements the date upon +/- key input
    * - increments/decrements the date upon Up/Down key input
    * - sets the date to current date when the 'T' key is pressed.
    *   The actual key for this to happen might be overridden through
    *   an i18n package. The 'T'-key is always possible.
    */
  void keyPressEvent(QKeyEvent* k) override;
  void showEvent(QShowEvent* event) override;

  /** To intercept events sent to focusWidget() */
  bool eventFilter(QObject *o, QEvent *e) override;


protected Q_SLOTS:
  void slotDateChosen(QDate date);
  void toggleDatePicker();

private Q_SLOTS:
  void slotDateChosenRef(const QDate& date);
  void fixSize();

private:
  struct Private;
  Private * const d;
};

#endif

