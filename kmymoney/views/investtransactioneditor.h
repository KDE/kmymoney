/*
 * Copyright 2019-2020  Thomas Baumgart <tbaumgart@kde.org>
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

#ifndef INVESTTRANSACTIONEDITOR_H
#define INVESTTRANSACTIONEDITOR_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QFrame>
class QWidget;

// ----------------------------------------------------------------------------
// Project Includes

#include "transactioneditorbase.h"
#include "mymoneymoney.h"

class InvestTransactionEditor : public TransactionEditorBase
{
  Q_OBJECT

public:
  explicit InvestTransactionEditor(QWidget* parent = 0, const QString& accountId = QString());
  virtual ~InvestTransactionEditor();

  /**
   * This method returns true if the user pressed the enter button.
   * It remains false, in case the user pressed the cancel button.
   */
  virtual bool accepted() const override;

  /**
   * Returns the currently entered amount
   */
  MyMoneyMoney transactionAmount() const;

  /**
   */
  void loadTransaction(const QModelIndex& index) override;
  void saveTransaction() override;

  /**
   * Reimplemented to suppress some events in certain conditions
   */
  bool eventFilter(QObject* o, QEvent* e) override;

  /**
   * Returns the transaction amount
   */
  MyMoneyMoney totalAmount() const;

protected:
  virtual void keyPressEvent(QKeyEvent* e) override;

protected Q_SLOTS:
  virtual void reject();
  virtual void acceptEdit();

  virtual void editFeeSplits();
  virtual void editInterestSplits();

  virtual void categoryChanged(const QString& accountId);
  virtual void postdateChanged(const QDate& date);
  virtual void securityChanged(int index);
  virtual void feesChanged(int index);
  virtual void feesChanged(const QString& txt);
  virtual void interestChanged(int index);
  virtual void interestChanged(const QString& txt);


  void valueChanged();

  void setupActivity(int index);

private:
  class Private;
  QScopedPointer<Private> const d;
  static QDate  m_lastPostDateUsed;
};

#endif // INVESTTRANSACTIONEDITOR_H

