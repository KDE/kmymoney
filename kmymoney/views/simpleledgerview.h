/***************************************************************************
                          simpleledgerview.h
                             -------------------
    begin                : Sat Aug 8 2015
    copyright            : (C) 2015 by Thomas Baumgart
    email                : Thomas Baumgart <tbaumgart@kde.org>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

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

