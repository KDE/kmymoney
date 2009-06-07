/***************************************************************************
                         kmymoneyaccounttreebudget.cpp  -  description
                            -------------------
   begin                : Tue Feb 21 2006
   copyright            : (C) 2005 by Darren Gould
   email                : Darren Gould <darren_gould@gmx.de>
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// ----------------------------------------------------------------------------
// Project Includes
#include <kmymoneyaccounttreebudget.h>
//Added by qt3to4:
#include <QList>

KMyMoneyAccountTreeBudget::KMyMoneyAccountTreeBudget(QWidget* parent, const char* name) :
  KMyMoneyAccountTreeBase::KMyMoneyAccountTreeBase(parent, name)
{
  showType();
  showValue();
}

void KMyMoneyAccountTreeBudget::slotSelectObject(const Q3ListViewItem* i)
{
  emit selectObject(MyMoneyInstitution());
  emit selectObject(MyMoneyAccount());

  const KMyMoneyAccountTreeBaseItem* item = dynamic_cast<const KMyMoneyAccountTreeBaseItem*>(i);
  if(item) {
    emit openObject(item->itemObject());
  }
}

KMyMoneyAccountTreeBudgetItem::KMyMoneyAccountTreeBudgetItem(K3ListView *parent, const MyMoneyAccount& account, const MyMoneyBudget  &budget, const MyMoneySecurity& security, const QString& name) :
  KMyMoneyAccountTreeBaseItem(parent, account, security, name),
  m_budget(budget)
{
  updateAccount(true);
}

KMyMoneyAccountTreeBudgetItem::KMyMoneyAccountTreeBudgetItem(KMyMoneyAccountTreeBudgetItem *parent, const MyMoneyAccount& account, const MyMoneyBudget& budget, const QList<MyMoneyPrice>& price, const MyMoneySecurity& security) :
  KMyMoneyAccountTreeBaseItem(parent, account, price, security),
  m_budget(budget)
{
  updateAccount(true);
}


KMyMoneyAccountTreeBudgetItem::~KMyMoneyAccountTreeBudgetItem()
{
}

void KMyMoneyAccountTreeBudgetItem::setBudget(const MyMoneyBudget& budget)
{
  m_budget = budget;
  updateAccount();
}

MyMoneyMoney KMyMoneyAccountTreeBudgetItem::balance() const
{
  MyMoneyMoney result = MyMoneyMoney();
  // find out if the account is budgeted
  MyMoneyBudget::AccountGroup budgetAccount = m_budget.account( m_account.id() );
  if ( budgetAccount.id() == m_account.id() ) {
    result = budgetAccount.balance();
    switch(budgetAccount.budgetLevel()) {
      case MyMoneyBudget::AccountGroup::eMonthly:
        result = result * 12;
        break;

      default:
        break;
    }
  }
  return result;
}

#include "kmymoneyaccounttreebudget.moc"
