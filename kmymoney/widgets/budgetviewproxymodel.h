/***************************************************************************
                          budgetviewproxymodel.h
                             -------------------
    Copyright (C) 2006 by Darren Gould <darren_gould@gmx.de>
    Copyright (C) 2006 by Alvaro Soliverez <asoliverez@gmail.com>
    Copyright (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>

***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef BUDGETVIEWPROXYMODEL_H
#define BUDGETVIEWPROXYMODEL_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "accountsviewproxymodel.h"

#include "mymoneybudget.h"

/**
  * This proxy model implements all the functionality needed by the budgets
  * account tree based on the @ref AccountsModel. One such functionality is
  * obtaining the account balance and value base on the budget.
  *
  * @author Cristin Oneț
  */
class BudgetViewProxyModel : public AccountsViewProxyModel
{
  Q_OBJECT

public:
  BudgetViewProxyModel(QObject *parent = 0);

  virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
  Qt::ItemFlags flags(const QModelIndex &index) const;

  void setBudget(const MyMoneyBudget& budget);

signals:
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
  void checkBalance();

private:
  MyMoneyBudget m_budget;
  MyMoneyMoney m_lastBalance;
};

#endif
