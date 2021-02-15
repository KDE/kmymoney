/*
    SPDX-FileCopyrightText: 2019-2020 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
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
#include "investactivities.h"

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
  void editFeeSplits();
  void editInterestSplits();

  void activityChanged (int index);
  void sharesChanged();

  void feeCategoryChanged(const QString& accountId);
  void interestCategoryChanged(const QString& accountId);
  void feesValueChanged();
  void interestValueChanged();
  void postdateChanged(const QDate& date);
  void securityAccountChanged (int index);

  void assetAccountChanged (const QString& accountId);

  void updateWidgets();
  void updateTotalAmount();

private:
  class Private;
  QScopedPointer<Private> const d;
};

#endif // INVESTTRANSACTIONEDITOR_H

