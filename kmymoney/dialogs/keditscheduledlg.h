/*
 * Copyright 2007-2018  Thomas Baumgart <tbaumgart@kde.org>
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

#ifndef KEDITSCHEDULEDLG_H
#define KEDITSCHEDULEDLG_H

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
class KEditScheduleDlg : public QDialog
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
