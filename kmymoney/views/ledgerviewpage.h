/***************************************************************************
                          ledgerviewpage.h
                             -------------------
    begin                : Sat Aug 8 2015
    copyright            : (C) 2015 by Thomas Baumgart
    email                : Thomas Baumgart <tbaumgart@kde.org>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef LEDGERVIEWPAGE_H
#define LEDGERVIEWPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWidget>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class MyMoneyAccount;

class LedgerViewPage : public QWidget
{
  Q_OBJECT
public:
  explicit LedgerViewPage(QWidget* parent = 0);
  virtual ~LedgerViewPage();

  virtual void setAccount(const MyMoneyAccount& id);
  virtual QString accountId() const;

  /**
   * This method is used to modify the visibility of the
   * empty entry at the end of the ledger. The default
   * for the parameter @a show is @c true.
   */
  void setShowEntryForNewTransaction(bool show = true);

protected:

public Q_SLOTS:
  void showTransactionForm(bool show);
  void splitterChanged(int pos, int index);
  void slotSettingsChanged();

protected Q_SLOTS:
  void startEdit();
  void finishEdit();

Q_SIGNALS:
  void transactionSelected(const QModelIndex& idx);
  void aboutToStartEdit();
  void aboutToFinishEdit();

private:
  class Private;
  Private * const d;
};

#endif // LEDGERVIEWPAGE_H

