/*
 * SPDX-FileCopyrightText: 2007-2018 Thomas Baumgart <tbaumgart@kde.org>
 * SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KEDITSCHEDULEDLG_H
#define KEDITSCHEDULEDLG_H

#include "kmm_oldregister_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDialog>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class MyMoneySchedule;
class MyMoneyAccount;
class MyMoneyTransaction;
class TransactionEditor;

namespace eMyMoney { namespace Schedule { enum class Occurrence; } }

/**
  * @author Thomas Baumgart
  */

class KEditScheduleDlgPrivate;
class KMM_OLDREGISTER_EXPORT KEditScheduleDlg : public QDialog
{
  Q_OBJECT
  Q_DISABLE_COPY(KEditScheduleDlg)
  
public:
  explicit KEditScheduleDlg(const MyMoneySchedule& schedule, QWidget *parent = nullptr);
  ~KEditScheduleDlg();

  TransactionEditor* startEdit();

  /**
    * Returns the edited schedule.
    *
    * @return MyMoneySchedule The schedule details.
    **/
  const MyMoneySchedule& schedule();

  static void newSchedule(const MyMoneyTransaction& _t, eMyMoney::Schedule::Occurrence occurrence);
  static void editSchedule(const MyMoneySchedule& inputSchedule);

protected:
  /**
    * This method adjusts @a _date according to the rules specified by
    * the schedule's weekend option.
    */
  QDate adjustDate(const QDate& _date) const;

  /// Overridden for internal reasons. No API changes.
  bool focusNextPrevChild(bool next) override;

  /// Overridden for internal reasons. No API changes.
  void resizeEvent(QResizeEvent* ev) override;

private Q_SLOTS:
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
  void accept() override;

private:
  KEditScheduleDlgPrivate * const d_ptr;
  Q_DECLARE_PRIVATE(KEditScheduleDlg)  
};

#endif
