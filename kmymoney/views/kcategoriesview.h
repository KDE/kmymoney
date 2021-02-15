/*
    SPDX-FileCopyrightText: 2000-2002 Michael Edwardes <mte@users.sourceforge.net>
    SPDX-FileCopyrightText: 2005 Thomas Baumgart <ipwizard@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2002 Javier Campos Morales <javi_c@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2002 Felix Rodriguez <frodriguez@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2002 John C <thetacoturtle@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2002 Kevin Tambascio <ktambascio@users.sourceforge.net>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KCATEGORIESVIEW_H
#define KCATEGORIESVIEW_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyviewbase.h"

class MyMoneyMoney;
class MyMoneyAccount;
class SelectedObjects;

/**
  * @brief  This class contains the implementation of the categories view.
  * @author Michael Edwardes, Thomas Baumgart
  *
  * While named "categories view", this view actually displays all accounts
  * that are children of the global "Income" and "Expense" accounts. Even though
  * categories are internally just accounts as well, the distinction between
  * categories and accounts in the user interface is done for better
  * usability and clarity.
  *
  * The main functionality in the categories view is actually implemented
  * in the KMyMoneyAccountTreeView. Signals from user actions are connect to
  * other signals/slots in KCategoriesView and relayed to KMyMoneyView.
  * A typical example is the selectObject() signal that eventually results
  * in enabling/disabling the user actions for the categories view.
  *
  * For the categories view three user actions are important (all created in
  * kmymoney.cpp): category_new, category_edit and category_delete. They are
  * accessible from either the main menu or the context menu.
  */
class KCategoriesViewPrivate;
class KCategoriesView : public KMyMoneyViewBase
{
  Q_OBJECT

public:
  explicit KCategoriesView(QWidget *parent = nullptr);
  ~KCategoriesView();

  void executeCustomAction(eView::Action action) override;

public Q_SLOTS:
  void slotProfitLossChanged(const MyMoneyMoney &profit, bool isApproximate);

  void slotSettingsChanged() override;
  void updateActions(const SelectedObjects& selections) override;

protected Q_SLOTS:
  void slotUnusedIncomeExpenseAccountHidden();

private:
  Q_DECLARE_PRIVATE(KCategoriesView)

private Q_SLOTS:
  void slotNewCategory();
  void slotEditCategory();
  void slotDeleteCategory();
};

#endif
