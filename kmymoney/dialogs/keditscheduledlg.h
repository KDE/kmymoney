/***************************************************************************
                          keditscheduledlg.h  -  description
                             -------------------
    begin                : Mon Sep  3 2007
    copyright            : (C) 2007 by Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KEDITSCHEDULEDLG_H
#define KEDITSCHEDULEDLG_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QDialog>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_keditscheduledlgdecl.h"

class MyMoneySchedule;
class MyMoneyTransaction;
class TransactionEditor;

/**
  * @author Thomas Baumgart
  */
class KEditScheduleDlgDecl : public QDialog, public Ui::KEditScheduleDlgDecl
{
public:
  explicit KEditScheduleDlgDecl(QWidget *parent) : QDialog(parent) {
    setupUi(this);
  }
};
class KEditScheduleDlg : public KEditScheduleDlgDecl
{
  Q_OBJECT
public:
  /**
    * Standard QWidget constructor.
    **/
  explicit KEditScheduleDlg(const MyMoneySchedule& schedule, QWidget *parent = 0);

  /**
    * Standard destructor.
    **/
  ~KEditScheduleDlg();

  TransactionEditor* startEdit();

  /**
    * Returns the edited schedule.
    *
    * @return MyMoneySchedule The schedule details.
    **/
  const MyMoneySchedule& schedule() const;

protected:
  MyMoneyTransaction transaction() const;
  /**
    * This method adjusts @a _date according to the rules specified by
    * the schedule's weekend option.
    */
  QDate adjustDate(const QDate& _date) const;

  /// Overridden for internal reasons. No API changes.
  bool focusNextPrevChild(bool next);

  /// Overridden for internal reasons. No API changes.
  void resizeEvent(QResizeEvent* ev);

private slots:
  void slotSetupSize();
  void slotRemainingChanged(int);
  void slotEndDateChanged(const QDate& date);
  void slotPostDateChanged(const QDate& date);
  void slotSetPaymentMethod(int);
  void slotFrequencyChanged(int item);
  void slotShowHelp();
  void slotOccurrenceMultiplierChanged(int mult);
  void slotFilterPaymentType(int index);

  /// Overridden for internal reasons. No API changes.
  void accept();

private:
  /**
    * Helper method to recalculate and update Transactions Remaining
    * when other values are changed
    */
  void updateTransactionsRemaining();

  /// \internal d-pointer class.
  class Private;
  /// \internal d-pointer instance.
  Private* const d;
};

#endif
