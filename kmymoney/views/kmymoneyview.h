/*
    SPDX-FileCopyrightText: 2000-2001 Michael Edwardes <mte@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004 Thomas Baumgart <ipwizard@users.sourceforge.net>
    SPDX-FileCopyrightText: 2017, 2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KMYMONEYVIEW_H
#define KMYMONEYVIEW_H

#include <config-kmymoney.h>

// ----------------------------------------------------------------------------
// QT Includes

#include <QFileDevice>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KPageWidget>

// ----------------------------------------------------------------------------
// Project Includes
#include "selectedtransactions.h"

#ifdef ENABLE_ACTIVITIES
namespace KActivities
{
class ResourceInstance;
}
#endif

namespace eAccountsModel {
enum class Column;
}
namespace eMenu {
enum class Action;
enum class Menu;
}
namespace KMyMoneyPlugin {
class OnlinePlugin;
}
namespace eDialogs {
enum class ScheduleResultCode;
}
namespace eView {
enum class Intent;
}
namespace eView {
enum class Action;
}
namespace Icons {
enum class Icon;
}

class KMyMoneyApp;
class KHomeView;
class KAccountsView;
class KCategoriesView;
class KInstitutionsView;
class KPayeesView;
class KTagsView;
class KBudgetView;
class KScheduledView;
class KGlobalLedgerView;
class IMyMoneyOperationsFormat;
class MyMoneyTransaction;
class KInvestmentView;
class KReportsView;
class SimpleLedgerView;
class MyMoneySchedule;
class MyMoneySecurity;
class MyMoneyReport;
class TransactionEditor;
class KOnlineJobOutbox;
class MyMoneyAccount;
class MyMoneyMoney;
class MyMoneyObject;
class QLabel;
class KMyMoneyViewBase;
class SelectedObjects;

/**
  * This class represents the view of the MyMoneyFile which contains
  * Banks/Accounts/Transactions, Recurring transactions (or Bills & Deposits)
  * and scripts (yet to be implemented).  Each different aspect of the file
  * is represented by a tab within the view.
  *
  * @author Michael Edwardes 2001 Copyright 2000-2001
  *
  * @short Handles the view of the MyMoneyFile.
  */
enum class View;
class KMyMoneyViewPrivate;
class KMyMoneyView : public KPageWidget
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(KMyMoneyView)

private:
    const QScopedPointer<KMyMoneyViewPrivate> d_ptr;

public:
    /**
      * The constructor for KMyMoneyView. Just creates all the tabs for the
      * different aspects of the MyMoneyFile.
      */
    KMyMoneyView();

    /**
      * Destructor
      */
    ~KMyMoneyView();

    /**
      * This method enables the state of all views (except home view) according
      * to an open file.
      */
    void enableViewsIfFileOpen(bool fileOpen);
    void switchToHomeView();

    void addWidget(QWidget* w);

    void showPageAndFocus(View idView);
    void showPage(View idView);

    /**
      * This method changes the view type according to the settings.
      */
    void updateViewType();

    void setOnlinePlugins(QMap<QString, KMyMoneyPlugin::OnlinePlugin*>& plugins);

    // TODO: remove that function
    /**
     * ugly proxy function
     */
    eDialogs::ScheduleResultCode enterSchedule(MyMoneySchedule& schedule, bool autoEnter, bool extendedKeys);

    void addView(KMyMoneyViewBase* view, const QString& name, View idView, Icons::Icon icon);
    void removeView(View idView);

    /**
     * @brief actionsToBeConnected are actions that need ActionCollection
     * which is available in KMyMoneyApp
     * @return QHash of action id and QAction itself
     */
    QHash<eMenu::Action, QAction *> actionsToBeConnected();

    /**
     * Execute the @a action using the selected objects of @a selections
     */
    void executeAction(eMenu::Action action, const SelectedObjects& selections);

    void setupSharedActions();

protected:
    /**
      * Overwritten because KMyMoney has it's custom header.
      */
    bool showPageHeader() const final override;


public Q_SLOTS:
    /**
      * This slot writes information about the page passed as argument @a current
      * in the kmymoney.rc file so that it can be selected automatically when
      * the application is started again.
      *
      * @param current KPageWidgetItem pointer to the current page item
      * @param previous KPageWidgetItem pointer to the previous page item
      */
    void slotSwitchView(KPageWidgetItem* current, KPageWidgetItem* previous);

    /**
     * Informs respective views about selected object, so they can
     * update action states and current object.
     * @param obj Account, Category, Investment, Stock, Institution
     */
    void slotObjectSelected(const MyMoneyObject& obj);

    void slotSelectByObject(const MyMoneyObject& obj, eView::Intent intent);
    void slotSettingsChanged();

    void updateActions(const SelectedObjects& selections);

private Q_SLOTS:
    void switchToDefaultView();

    /**
     * Opens object in ledgers or edits in case of institution
     * @param obj Account, Category, Investment, Stock, Institution
     */
    void slotOpenObjectRequested(const MyMoneyObject& obj);

    void slotRememberLastView(View view);

Q_SIGNALS:
    /**
     * This signal is emitted whenever a view is selected.
     * The parameter @p view is identified as one of KMyMoneyView::viewID.
     */
    void viewActivated(View view);

    void accountSelectedForContextMenu(const MyMoneyAccount& acc);

    void viewStateChanged(bool enabled);
    /**
      * This signal is emitted to inform the kmmFile plugin when various file actions
      * occur. The Action parameter distinguishes between them.
      */
    void kmmFilePlugin(unsigned int action);

    /**
     * @brief proxy signal
     */
    void statusMsg(const QString& txt);

    /**
     * @brief proxy signal
     */
    void statusProgress(int cnt, int base);

    void accountReconciled(const MyMoneyAccount& account,
                           const QDate& date,
                           const MyMoneyMoney& startingBalance,
                           const MyMoneyMoney& endingBalance,
                           const QStringList& transactionList);

    /**
      * This signal is emitted when a transaction/list of transactions has been selected by
      * the GUI. If no transaction is selected or the selection is removed,
      * @p transactions is identical to an empty QList. This signal is used
      * by plugins to get information about changes.
      */
    void transactionsSelected(const KMyMoneyRegister::SelectedTransactions& transactions);

    /**
      * This signal is emitted when a new account has been selected by
      * the GUI. If no account is selected or the selection is removed,
      * @a account is identical to MyMoneyAccount(). This signal is used
      * by plugins to get information about changes.
      */
    void accountSelected(const MyMoneyAccount& account);

    // these signals request a change by the application
    void requestSelectionChange (const SelectedObjects& selection);
    void requestCustomContextMenu(eMenu::Menu type, const QPoint& pos) const;
    void requestActionTrigger(eMenu::Action action);

    void settingsChanged();

    void addSharedActionButton(eMenu::Action action, QAction* defaultAction);
    void selectSharedActionButton(eMenu::Action action, QAction* defaultAction);
};

#endif
