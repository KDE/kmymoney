/***************************************************************************
                          splitdialog.h
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

#ifndef SPLITDIALOG_H
#define SPLITDIALOG_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QDialog>
#include <QScopedPointer>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneymoney.h"

class QAbstractItemModel;
class MyMoneyAccount;
class NewTransactionEditor;

class SplitDialog : public QDialog
{
  Q_OBJECT
public:
  explicit SplitDialog(const MyMoneyAccount& account, const MyMoneyMoney& mainAmount, NewTransactionEditor* parent, Qt::WindowFlags f = 0);
  virtual ~SplitDialog();


  void setModel(QAbstractItemModel* model);
  void setAccountId(const QString& id);

  /**
   * Returns the amount for the transaction.
   */
  MyMoneyMoney transactionAmount() const;

public Q_SLOTS:
  void accept() final override;
  int exec() final override;

private Q_SLOTS:
  void adjustSummary();

  void disableButtons();
  void enableButtons();

  void newSplit();

protected Q_SLOTS:
  void deleteSelectedSplits();
  void deleteAllSplits();
  void deleteZeroSplits();
  void mergeSplits();
  void selectionChanged();
  void updateButtonState();

protected:
  void resizeEvent(QResizeEvent* ev) final override;
  void adjustSummaryWidth();

private:
  class Private;
  QScopedPointer<Private> d;
};
#endif // SPLITDIALOG_H
