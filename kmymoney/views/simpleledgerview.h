/*
 * Copyright 2015-2019  Thomas Baumgart <tbaumgart@kde.org>
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


#ifndef SIMPLELEDGERVIEW_H
#define SIMPLELEDGERVIEW_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyviewbase.h"

class SimpleLedgerViewPrivate;
class SimpleLedgerView : public KMyMoneyViewBase
{
  Q_OBJECT

public:
  explicit SimpleLedgerView(QWidget *parent = nullptr);
  ~SimpleLedgerView() override;

  virtual void showTransactionForm(bool = true);

public Q_SLOTS:
  /**
   * This method closes all open ledgers
   */
  void closeLedgers();

  /**
   * This slot creates tabs for the initial ledgers on file open
   */
  void openLedgersAfterOpen();

  void showEvent(QShowEvent* event) override;

  void slotSettingsChanged() override;

protected:

private Q_SLOTS:
  void tabSelected(int idx);
  void tabClicked(int idx);
  void openLedger(QString accountId, bool makeCurrentLedger);
  /**
   * A convenience method for openLedger() that sets @a makeCurrentLedger to @c true.
   */
  void openNewLedger(QString accountId);
  void closeLedger(int idx);
  void checkTabOrder(int from, int to);
  void setupCornerWidget();

protected:
  bool eventFilter(QObject* o, QEvent* e) override;

Q_SIGNALS:
  void showForms(bool show);
  void settingsChanged();

private:
  Q_DECLARE_PRIVATE(SimpleLedgerView)
};

#endif // SIMPLELEDGERVIEW_H

