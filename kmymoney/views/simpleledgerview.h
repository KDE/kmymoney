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
   * This slot creates tabs for the favorite ledgers if not already open
   */
  void openFavoriteLedgers();

  void showEvent(QShowEvent* event) override;

  void slotSettingsChanged();

protected:

private Q_SLOTS:
  void tabSelected(int idx);
  void openNewLedger(QString accountId);
  void updateModels();
  void closeLedger(int idx);
  void checkTabOrder(int from, int to);
  void setupCornerWidget();

Q_SIGNALS:
  void showForms(bool show);

private:
  Q_DECLARE_PRIVATE(SimpleLedgerView)
};

#endif // SIMPLELEDGERVIEW_H

