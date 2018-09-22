/***************************************************************************
                          kmymoneyview.h
                             -------------------
    copyright            : (C) 2000-2001 by Michael Edwardes <mte@users.sourceforge.net>
                               2004 by Thomas Baumgart <ipwizard@users.sourceforge.net>
                           (C) 2017, 2018 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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

#ifdef KF5Activities_FOUND
namespace KActivities
{
class ResourceInstance;
}
#endif

namespace eAccountsModel { enum class Column; }
namespace eMenu { enum class Action; }
namespace KMyMoneyPlugin { class OnlinePlugin; }
namespace eDialogs { enum class ScheduleResultCode; }
namespace eView { enum class Intent; }
namespace eView { enum class Action; }
namespace Icons { enum class Icon; }

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
class KMyMoneyTitleLabel;
class MyMoneyAccount;
class MyMoneyMoney;
class MyMoneyObject;
class QLabel;
class KMyMoneyViewBase;

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
class KMyMoneyView : public KPageWidget
{
  Q_OBJECT
private:

  KPageWidgetModel* m_model;

  QHash<View, KPageWidgetItem*> viewFrames;
  QHash<View, KMyMoneyViewBase*> viewBases;

  KMyMoneyTitleLabel* m_header;

  void viewAccountList(const QString& selectAccount); // Show the accounts view

  void createSchedule(MyMoneySchedule s, MyMoneyAccount& a);

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
    * check if the current view allows to print something
    *
    * @retval true Yes, view allows to print
    * @retval false No, view cannot print
    */
  bool canPrint();

  void finishReconciliation(const MyMoneyAccount& account);

  void showTitleBar(bool show);

  /**
    * This method changes the view type according to the settings.
    */
  void updateViewType();

  void slotAccountTreeViewChanged(const eAccountsModel::Column column, const bool show);

  void setOnlinePlugins(QMap<QString, KMyMoneyPlugin::OnlinePlugin*>& plugins);

  // TODO: remove that function
  /**
   * ugly proxy function
   */
  eDialogs::ScheduleResultCode enterSchedule(MyMoneySchedule& schedule, bool autoEnter, bool extendedKeys);

  void addView(KMyMoneyViewBase* view, const QString& name, View idView);
  void removeView(View idView);

  /**
   * @brief actionsToBeConnected are actions that need ActionCollection
   * which is available in KMyMoneyApp
   * @return QHash of action id and QAction itself
   */
  QHash<eMenu::Action, QAction *> actionsToBeConnected();

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
    * @param current QModelIndex of the current page item
    * @param previous QModelIndex of the previous page item
    */
  void slotCurrentPageChanged(const QModelIndex current, const QModelIndex previous);

  /**
    * Brings up a dialog to change the list(s) settings and saves them into the
    * class KMyMoneySettings (a singleton).
    *
    * @see KListSettingsDlg
    * Refreshes all views. Used e.g. after settings have been changed or
    * data has been loaded from external sources (QIF import).
    **/
  void slotRefreshViews();

  /**
    * Called, whenever the payees view should pop up and a specific
    * transaction in an account should be shown.
    *
    * @param payeeId The ID of the payee to be shown
    * @param accountId The ID of the account to be shown
    * @param transactionId The ID of the transaction to be selected
    */
  void slotPayeeSelected(const QString& payeeId, const QString& accountId, const QString& transactionId);

  /**
    * Called, whenever the tags view should pop up and a specific
    * transaction in an account should be shown.
    *
    * @param tagId The ID of the tag to be shown
    * @param accountId The ID of the account to be shown
    * @param transactionId The ID of the transaction to be selected
    */
  void slotTagSelected(const QString& tagId, const QString& accountId, const QString& transactionId);

  /**
    * This slot prints the current view.
    */
  void slotPrintView();

  /**
    * Called when the user changes the detail
    * setting of the transaction register
    *
    * @param detailed if true, the register is shown with all details
    */
  void slotShowTransactionDetail(bool detailed);

  /**
   * Informs respective views about selected object, so they can
   * update action states and current object.
   * @param obj Account, Category, Investment, Stock, Institution
   */
  void slotObjectSelected(const MyMoneyObject& obj);

  void slotSelectByObject(const MyMoneyObject& obj, eView::Intent intent);
  void slotSelectByVariant(const QVariantList& variant, eView::Intent intent);
  void slotCustomActionRequested(View view, eView::Action action);

  void slotFileOpened();
  void slotFileClosed();

private Q_SLOTS:
  /**
    * This slots switches the view to the specific page
    */
  void slotShowHomePage();
  void slotShowInstitutionsPage();
  void slotShowAccountsPage();
  void slotShowSchedulesPage();
  void slotShowCategoriesPage();
  void slotShowTagsPage();
  void slotShowPayeesPage();
  void slotShowLedgersPage();
  void slotShowInvestmentsPage();
  void slotShowReportsPage();
  void slotShowBudgetPage();
  void slotShowForecastPage();
  void slotShowOutboxPage();
  void switchToDefaultView();

  /**
   * Opens object in ledgers or edits in case of institution
   * @param obj Account, Category, Investment, Stock, Institution
   */
  void slotOpenObjectRequested(const MyMoneyObject& obj);

  /**
   * Opens context menu based on objects's type
   * @param obj Account, Category, Investment, Stock, Institution
   */
  void slotContextMenuRequested(const MyMoneyObject& obj);

private:

  /**
    * Internal method used by slotAccountNew() and slotAccountCategory().
    */
  void accountNew(const bool createCategory);

  void resetViewSelection();

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

  void accountReconciled(const MyMoneyAccount& account, const QDate& date, const MyMoneyMoney& startingBalance, const MyMoneyMoney& endingBalance, const QList<QPair<MyMoneyTransaction, MyMoneySplit> >& transactionList);

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
};

#endif
