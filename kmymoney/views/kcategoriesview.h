/***************************************************************************
                          kcategoriesview.h
                             -------------------
    begin                : Sun Jan 20 2002
    copyright            : (C) 2000-2002 by Michael Edwardes <mte@users.sourceforge.net>
                           (C) 2005 by Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KCATEGORIESVIEW_H
#define KCATEGORIESVIEW_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyaccountsviewbase.h"

class MyMoneyMoney;
class MyMoneyAccount;

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
class KCategoriesView : public KMyMoneyAccountsViewBase
{
  Q_OBJECT

public:
  explicit KCategoriesView(QWidget *parent = nullptr);
  ~KCategoriesView();

  void setDefaultFocus() override;
  void refresh() override;
  void updateActions(const MyMoneyObject& obj) override;

public Q_SLOTS:
  void slotProfitChanged(const MyMoneyMoney &);

  void slotShowCategoriesMenu(const MyMoneyAccount& acc);

protected:
  void showEvent(QShowEvent * event) override;

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
