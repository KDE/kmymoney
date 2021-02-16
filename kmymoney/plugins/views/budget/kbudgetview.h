/*
    SPDX-FileCopyrightText: 2006 Darren Gould <darren_gould@gmx.de>
    SPDX-FileCopyrightText: 2006 Alvaro Soliverez <asoliverez@gmail.com>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KBUDGETVIEW_H
#define KBUDGETVIEW_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyaccountsviewbase.h"

class QTreeWidgetItem;

class MyMoneyObject;
class MyMoneyBudget;
class MyMoneyMoney;

/**
  * @author Darren Gould
  * @author Thomas Baumgart
  */
class KBudgetViewPrivate;
class KBudgetView : public KMyMoneyAccountsViewBase
{
  Q_OBJECT

public:
  explicit KBudgetView(QWidget *parent = nullptr);
  ~KBudgetView() override;

  void executeCustomAction(eView::Action action) override;
  void refresh();

protected:
  KBudgetView(KBudgetViewPrivate &dd, QWidget *parent);
  void showEvent(QShowEvent * event) override;

private:
  Q_DECLARE_PRIVATE(KBudgetView)

private Q_SLOTS:
  void slotNewBudget();
  void slotDeleteBudget();
  void slotCopyBudget();
  void slotChangeBudgetYear();
  void slotBudgetForecast();
  void slotResetBudget();
  void slotUpdateBudget();
  void slotStartRename();

  /**
    * This slot receives the signal from the listview control that an
    * item was right-clicked,
    * If @p p points to a real budget item, emits openContextMenu().
    *
    * @param p position of the pointing device
    */
  void slotOpenContextMenu(const QPoint&);
  void slotItemChanged(QTreeWidgetItem* p, int col);
  void slotSelectAccount(const MyMoneyObject &obj, eView::Intent intent);
  void slotBudgetedAmountChanged();
  /**
    *This is to update the information about the checkbox "budget amount integrates subaccounts" into the file, when the user clicks the check box
   */
  void cb_includesSubaccounts_clicked();
  void slotBudgetBalanceChanged(const MyMoneyMoney &balance);
  void slotSelectBudget();
  void slotHideUnused(bool toggled);

};

#endif
