/*
 * SPDX-FileCopyrightText: 2006 Darren Gould <darren_gould@gmx.de>
 * SPDX-FileCopyrightText: 2009-2014 Alvaro Soliverez <asoliverez@gmail.com>
 * SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef BUDGETVIEWPROXYMODEL_H
#define BUDGETVIEWPROXYMODEL_H

#include "kmm_widgets_export.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "accountsviewproxymodel.h"

class MyMoneyAccount;
class MyMoneyBudget;
class MyMoneyMoney;

/**
  * This proxy model implements all the functionality needed by the budgets
  * account tree based on the @ref AccountsModel. One such functionality is
  * obtaining the account balance and value base on the budget.
  *
  * @author Cristian Oneț
  */
class BudgetViewProxyModelPrivate;
class KMM_WIDGETS_EXPORT BudgetViewProxyModel : public AccountsViewProxyModel
{
  Q_OBJECT
  Q_DISABLE_COPY(BudgetViewProxyModel)

public:
  explicit BudgetViewProxyModel(QObject *parent = nullptr);
  ~BudgetViewProxyModel() override;

  virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
  Qt::ItemFlags flags(const QModelIndex &index) const override;

  void setBudget(const MyMoneyBudget& budget);

Q_SIGNALS:
  /**
    * Emit this signal when the balance of the budget is changed.
    */
  void balanceChanged(const MyMoneyMoney &);

protected:
  bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
  MyMoneyMoney accountBalance(const QString &accountId) const;
  MyMoneyMoney accountValue(const MyMoneyAccount &account, const MyMoneyMoney &balance) const;
  MyMoneyMoney computeTotalValue(const QModelIndex &source_index) const;

private:
  Q_DECLARE_PRIVATE(BudgetViewProxyModel)

  void checkBalance();
};

#endif
