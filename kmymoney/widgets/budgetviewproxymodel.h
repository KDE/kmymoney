/*
 * Copyright 2006       Darren Gould <darren_gould@gmx.de>
 * Copyright 2009-2014  Alvaro Soliverez <asoliverez@gmail.com>
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

#ifndef BUDGETVIEWPROXYMODEL_H
#define BUDGETVIEWPROXYMODEL_H

#include "kmm_widgets_export.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

// #include "accountsviewproxymodel.h"

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
// class BudgetViewProxyModelPrivate;
// class KMM_WIDGETS_EXPORT BudgetViewProxyModel : public AccountsViewProxyModel
// {
//   Q_OBJECT
//   Q_DISABLE_COPY(BudgetViewProxyModel)
//
// public:
//   explicit BudgetViewProxyModel(QObject *parent = nullptr);
//   ~BudgetViewProxyModel() override;
//
//   virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
//   Qt::ItemFlags flags(const QModelIndex &index) const override;
//
//   void setBudget(const MyMoneyBudget& budget);
//
// Q_SIGNALS:
//   /**
//     * Emit this signal when the balance of the budget is changed.
//     */
//   void balanceChanged(const MyMoneyMoney &);
//
// protected:
//   bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
//   MyMoneyMoney accountBalance(const QString &accountId) const;
//   MyMoneyMoney accountValue(const MyMoneyAccount &account, const MyMoneyMoney &balance) const;
//   MyMoneyMoney computeTotalValue(const QModelIndex &source_index) const;
//
// private:
//   Q_DECLARE_PRIVATE(BudgetViewProxyModel)
//
//   void checkBalance();
// };

#endif
