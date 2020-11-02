/***************************************************************************
                          kbudgetview.h
                          -------------
    begin                : Thu Jan 10 2006
    copyright            : (C) 2006 by Darren Gould
    email                : darren_gould@gmx.de
                           Alvaro Soliverez <asoliverez@gmail.com>
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
                           (C) 2019 Thomas Baumgart <tbaumgart@kde.org>
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KBUDGETVIEW_H
#define KBUDGETVIEW_H

// ----------------------------------------------------------------------------
// QT Includes

class QTreeWidgetItem;
class QMenu;

// ----------------------------------------------------------------------------
// KDE Includes

class KActionCollection;
class KXMLGUIClient;
class KXMLGUIFactory;

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyviewbase.h"


class MyMoneyObject;
class MyMoneyBudget;
class MyMoneyMoney;
class SelectedObjects;

namespace eMenu {
  enum class BudgetAction {
    NewBudget, RenameBudget, DeleteBudget,
    CopyBudget, BudgetForecast,
  };
  inline uint qHash(const BudgetAction key, uint seed) { return ::qHash(static_cast<uint>(key), seed); }
};

/**
  * @author Darren Gould
  * @author Thomas Baumgart
  */
class KBudgetViewPrivate;
class KBudgetView : public KMyMoneyViewBase
{
  Q_OBJECT

public:
  explicit KBudgetView(QWidget *parent = nullptr);
  ~KBudgetView() override;

  void executeCustomAction(eView::Action action) override;

  void createActions(KXMLGUIClient* guiClient);
  void removeActions();

public Q_SLOTS:
  void slotSettingsChanged() override;
  void updateActions(const SelectedObjects& selections) override;

protected:
  void showEvent(QShowEvent * event) override;

private:
  Q_DECLARE_PRIVATE(KBudgetView)

private Q_SLOTS:
  void slotNewBudget();
  void slotDeleteBudget();
  void slotCopyBudget();
  void slotStartRename();
  void slotBudgetForecast();
  void slotResetBudget();
  void slotUpdateBudget();

  void slotSelectAccount(const MyMoneyObject &obj, eView::Intent intent);
  void slotBudgetedAmountChanged();
  /**
    *This is to update the information about the checkbox "budget amount integrates subaccounts" into the file, when the user clicks the check box
   */
  void cb_includesSubaccounts_clicked();
  void slotBudgetBalanceChanged(const MyMoneyMoney &balance);
  void slotSelectBudget();
  void slotHideUnused(bool toggled);

  void slotAccountSelectionChanged (const SelectedObjects& selections);
};

#endif
