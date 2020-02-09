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
class MyMoneySecurity;

/**
 * This dialog allows the user to modify the splits of a transaction.
 * The splits are passed in form of a SplitModel via setModel(). The
 * total amount of the transaction is passed as @a mainAmount. If
 * @a mainAmount equals to MyMoneyMoney::autoCalc then the total
 * will automatically be adjusted.
 */
class SplitDialog : public QDialog
{
  Q_OBJECT
public:
  explicit SplitDialog(const MyMoneyAccount& account, const MyMoneySecurity& commodity, const MyMoneyMoney& mainAmount, QWidget* parent, Qt::WindowFlags f = 0);
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
