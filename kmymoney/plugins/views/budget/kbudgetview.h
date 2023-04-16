/*
    SPDX-FileCopyrightText: 2006 Darren Gould <darren_gould@gmx.de>
    SPDX-FileCopyrightText: 2006 Alvaro Soliverez <asoliverez@gmail.com>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
    NewBudget,
    RenameBudget,
    DeleteBudget,
    CopyBudget,
    BudgetForecast,
    TreatAsIncome,
    TreatAsExpense,
};

inline uint qHash(const BudgetAction key, uint seed) {
    return ::qHash(static_cast<uint>(key), seed);
}
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

    void createActions(KXMLGUIFactory* guiFactory, KXMLGUIClient* guiClient);
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

    void slotSelectAccount(const MyMoneyObject& obj);
    void slotBudgetedAmountChanged();
    /**
      *This is to update the information about the checkbox "budget amount integrates subaccounts" into the file, when the user clicks the check box
     */
    void cb_includesSubaccounts_clicked();
    void slotBudgetBalanceChanged(const MyMoneyMoney &balance);
    void slotSelectBudget();
    void slotHideUnused(bool toggled);

    void slotAccountSelectionChanged (const SelectedObjects& selections);

    void slotOpenAccountContextMenu(eMenu::Menu type, const QPoint& p);

    void slotTreatAsIncome();
    void slotTreatAsExpense();
};

#endif
