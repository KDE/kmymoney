/***************************************************************************
                          kmymoney.h
                             -------------------
    copyright            : (C) 2000-2001 by Michael Edwardes <mte@users.sourceforge.net>

***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef KMYMONEY2_H
#define KMYMONEY2_H

#ifdef HAVE_CONFIG_H
#include <config-kmymoney.h>
#endif

// ----------------------------------------------------------------------------
// QT Includes

#include <QList>
#include <QByteArray>
#include <QResizeEvent>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kxmlguiwindow.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <mymoneyaccount.h>
#include <mymoneyscheduled.h>
#include <mymoneyinstitution.h>
#include <mymoneypayee.h>
#include <mymoneybudget.h>
#include <kmymoneyplugin.h>
#include <register.h>
#include <kmymoneyutils.h>

class QResizeEvent;
class Q3ListViewItem;
class KPluginInfo;

/*! \mainpage KMyMoney Main Page for API documentation.
 *
 * \section intro Introduction
 *
 * This is the API documentation for KMyMoney.  It should be used as a reference
 * for KMyMoney developers and users who wish to see how KMyMoney works.  This
 * documentation will be kept up-to-date as development progresses and should be
 * read for new features that have been developed in KMyMoney.
 *
 * The latest version of this document is available from the project's web-site
 * at http://kmymoney2.sourceforge.net/ and is generated daily by doxygen reading
 * the header files found in the CVS main branch.
 */

/**
  * The base class for KMyMoney application windows. It sets up the main
  * window and reads the config file as well as providing a menubar, toolbar
  * and statusbar.
  *
  * @see KMyMoneyView
  *
  * @author Michael Edwardes 2000-2001
  * @author Thomas Baumgart 2006-2008
  *
  * @short Main application class.
  */
class KMyMoney2App : public KXmlGuiWindow
{
  Q_OBJECT

protected slots:
  void slotFileSaveAsFilterChanged(const QString& filter);

  /**
    * This slot is intended to be used as part of auto saving. This is used when the
    * QTimer emits the timeout signal and simply checks that the file is dirty (has
    * received modifications to it's contents), and call the appropriate method to
    * save the file. Furthermore, re-starts the timer (possibly not needed).
    * @author mvillarino 2005
    * @see KMyMoney2App::slotDataChanged()
    */
  void slotAutoSave(void);

  /**
    * This slot re-enables all message for which the "Don't show again"
    * option had been selected.
    */
  void slotEnableMessages(void);

  /**
    * Called when the user asks for file information.
    */
  void slotFileFileInfo(void);

  void slotPerformanceTest(void);

  /**
    * Debugging only: turn on/off traces
    */
  void slotToggleTraces(void);

  /**
    * Debugging only: turn on/off timers
    */
  void slotToggleTimers(void);

  /**
    * Called when the user asks for the personal information.
    */
  void slotFileViewPersonal(void);

  /**
    * Called when the user wishes to import tab delimeted transactions
    * into the current account.  An account must be open for this to
    * work.  Calls KMyMoneyView::slotAccountImportAscii.
    *
    * @see MyMoneyAccount
    */
  void slotQifImport(void);

  /**
    * Called when a QIF import is finished.
    */
  void slotQifImportFinished(void);

  /**
    * Opens a file selector dialog for the user to choose an existing OFX
    * file from the file system to be imported.  This slot is expected to
    * be called from the UI.
    */
  void slotGncImport(void);

  /**
   * Open a dialog with a chart of the balance for the currently selected
   * account (m_selectedAccount). Return once the dialog is closed. Don't do
   * anything if no account is selected or charts are not available.
   */
  void slotAccountChart(void);

  /**
    * Opens a file selector dialog for the user to choose an existing KMM
    * statement file from the file system to be imported.  This is for testing
    * only.  KMM statement files are not designed to be exposed to the user.
    */
  void slotStatementImport(void);

  void slotLoadAccountTemplates(void);
  void slotSaveAccountTemplates(void);

  /**
    * Called when the user wishes to export some transaction to a
    * QIF formatted file. An account must be open for this to work.
    * Uses MyMoneyQifWriter() for the actual output.
    */
  void slotQifExport(void);

  /**
    * Open up the application wide settings dialog.
    *
    * @see KSettingsDlg
    */
  void slotSettings(void);

  /** No descriptions */
  void slotFileBackup(void);

  void slotShowTipOfTheDay(void);

  void slotQifProfileEditor(void);

  void slotShowPreviousView(void);

  void slotShowNextView(void);

  void slotSecurityEditor(void);

  /**
    * Brings up a dialog to let the user search for specific transaction(s).  It then
    * opens a results window to display those transactions.
    */
  void slotFindTransaction(void);

  /**
    * Destroys a possibly open the search dialog
    */
  void slotCloseSearchDialog(void);

  /**
    * Preloads the input dialog with the data of the current
    * selected institution and brings up the input dialog
    * and saves the information entered.
    */
  void slotInstitutionEdit(const MyMoneyObject& obj = MyMoneyInstitution());

  /**
    * Deletes the current selected institution.
    */
  void slotInstitutionDelete(void);

  /**
    * Brings up the new category editor and saves the information.
    * The dialog will be preset with the name. The parent defaults to
    * MyMoneyFile::expense()
    *
    * @param name Name of the account to be created. Could include a full hierarchy
    * @param id reference to storage which will receive the id after successful creation
    *
    * @note Typically, this slot can be connected to the
    *       StdTransactionEditor::createCategory(const QString&, QString&) or
    *       KMyMoneyCombo::createItem(const QString&, QString&) signal.
    */
  void slotCategoryNew(const QString& name, QString& id);

  /**
    * Calls the print logic for the current view
    */
  void slotPrintView(void);

  /**
    * Create a new investment
    */
  void slotInvestmentNew(void);

  /**
    * Create a new investment in a given @p parent investment account
    */
  void slotInvestmentNew(MyMoneyAccount& account, const MyMoneyAccount& parent);

  /**
    * This slot opens the investment editor to edit the currently
    * selected investment if possible
    */
  void slotInvestmentEdit(void);

  /**
    * Deletes the current selected investment.
    */
  void slotInvestmentDelete(void);

  /**
    * Performs online update for currently selected investment
    */
  void slotOnlinePriceUpdate(void);

  /**
    * Performs manual update for currently selected investment
    */
  void slotManualPriceUpdate(void);

  /**
    * Call this slot, if any configuration parameter has changed
    */
  void slotUpdateConfiguration(void);

  /**
    */
  void slotPayeeNew(const QString& newnameBase, QString& id);
  void slotPayeeNew(void);

  /**
    */
  void slotPayeeDelete(void);

  /**
    */
  void slotBudgetNew(void);

  /**
    */
  void slotBudgetDelete(void);

  /**
   */
  void slotBudgetCopy(void);

  /**
    */
  void slotBudgetChangeYear(void);

  /**
    */
  void slotBudgetForecast(void);

  /**
    */
  void slotCurrencyNew(void);

  /**
    */
  void slotCurrencyRename(Q3ListViewItem* item, int, const QString& txt);

  /**
    */
  void slotCurrencyDelete(void);

  /**
    */
  void slotCurrencySetBase(void);

  /**
    * This slot is used to start new features during the development cycle
    */
  void slotNewFeature(void);

  /**
    */
  void slotTransactionsNew(void);

  /**
    */
  void slotTransactionsEdit(void);

  /**
    */
  void slotTransactionsEditSplits(void);

  /**
    */
  void slotTransactionsDelete(void);

  /**
    */
  void slotTransactionsEnter(void);

  /**
    */
  void slotTransactionsCancel(void);

  /**
    */
  void slotTransactionsCancelOrEnter(bool& okToSelect);

  /**
    */
  void slotTransactionDuplicate(void);

  /**
    */
  void slotToggleReconciliationFlag(void);

  /**
    */
  void slotMarkTransactionCleared(void);

  /**
    */
  void slotMarkTransactionReconciled(void);

  /**
    */
  void slotMarkTransactionNotReconciled(void);

  /**
    */
  void slotTransactionGotoAccount(void);

  /**
    */
  void slotTransactionGotoPayee(void);

  /**
    */
  void slotTransactionCreateSchedule(void);

  /**
    */
  void slotTransactionAssignNumber(void);

  /**
    */
  void slotTransactionCombine(void);

  /**
    * Accept the selected transactions that are marked as 'imported' and remove the flag
    */
  void slotTransactionsAccept(void);

  /**
    * This slot triggers an update of all views and restarts
    * a single shot timer to call itself again at beginning of
    * the next day.
    */
  void slotDateChanged(void);

  /**
    * This slot will be called when the engine data changed
    * and the application object needs to update it's state.
    */
  void slotDataChanged(void);

  void slotMoveToAccount(const QString& id);

  void slotUpdateMoveToAccountMenu(void);

  /**
    * This slot collects information for a new scheduled transaction
    * based on transaction @a t and @a occurrence and saves it in the engine.
    */
  void slotScheduleNew(const MyMoneyTransaction& t, MyMoneySchedule::occurrenceE occurrence = MyMoneySchedule::OCCUR_MONTHLY);

  /**
    */
  void slotScheduleDuplicate(void);

  void slotKDELanguageSettings(void);

  void slotAccountMapOnline(void);
  void slotAccountUnmapOnline(void);
  void slotAccountUpdateOnline(void);
  void slotAccountUpdateOnlineAll(void);

  void slotManageGpgKeys(void);
  void slotKeySelected(int idx);

public:
  /**
    * This method checks if there is at least one asset or liability account
    * in the current storage object. If not, it starts the new account wizard.
    */
  void createInitialAccount(void);

  /**
    * This method returns the last URL used or an empty URL
    * depending on the option setting if the last file should
    * be opened during startup or the open file dialog should
    * be displayed.
    *
    * @return URL of last opened file or empty if the program
    *         should start with the open file dialog
    */
  const KUrl lastOpenedURL(void);

  /**
    * construtor of KMyMoney2App, calls all init functions to create the application.
    */
  explicit KMyMoney2App(QWidget* parent=0);

  /**
    * Destructor
    */
  ~KMyMoney2App();

  /** Init wizard dialog */
  bool initWizard(void);

  static void progressCallback(int current, int total, const QString&);

  void writeLastUsedDir(const QString& directory);
  QString readLastUsedDir(void) const;
  void writeLastUsedFile(const QString& fileName);
  QString readLastUsedFile(void) const;

  /**
    * Returns whether there is an importer available that can handle this file
    */
  bool isImportableFile( const KUrl& url );

  /**
    * This method is used to update the caption of the application window.
    * It set's the caption to "filename [modified] - KMyMoney".
    *
    * @param skipActions if true, the actions will not be updated. This
    *                    is usually onyl required by some early calls when
    *                    these widgets are not yet created (the default is false).
    */
  void updateCaption(bool skipActions = false);

  /**
    * This method returns a list of all 'other' dcop registered kmymoney processes.
    * It's a subset of the return of DCOPclient()->registeredApplications().
    *
    * @retval QStringList of process ids
    */
  const QList<QString> instanceList(void) const;

  /**
    * Dump a list of the names of all defined KActions to stdout.
    */
  void dumpActions(void) const;

  /**
    * Popup the context menu with the respective @p containerName.
    * Valid container names are defined in kmymoneyui.rc
    */
  void showContextMenu(const QString& containerName);

  /**
   * This method opens the category editor with the data found in @a account. The
   * parent account is preset to @a parent but can be modified. If the user
   * acknowledges, the category is created.
   */
  void createCategory(MyMoneyAccount& account, const MyMoneyAccount& parent);

  /**
   * This method returns the account for a given @a key - @a value pair.
   * If the account is not found in the list of accounts, MyMoneyAccount()
   * is returned.
   */
  const MyMoneyAccount& account(const QString& key, const QString& value) const;

  /**
   * This method set the online parameters stored in @a kvps with the
   * account referenced by @a acc.
   */
  void setAccountOnlineParameters(const MyMoneyAccount& acc, const MyMoneyKeyValueContainer& kvps);

  KUrl selectFile(const QString& title, const QString& path, const QString& mask, KFile::Mode mode);

  const MyMoneyAccount& findAccount(const MyMoneyAccount& acc, const MyMoneyAccount& parent) const;

  void createAccount(MyMoneyAccount& newAccount, MyMoneyAccount& parentAccount, MyMoneyAccount& brokerageAccount, MyMoneyMoney openingBal);

  // Note: Don't use e.g. filename(void) but use filename() because
  // otherwise the kidl compiler produces uncompilable results.
  const QString filename() const;

  void webConnect(const QString&, const QByteArray& asn_id);

  /**
    * Checks if the file with the @a url already exists. If so,
    * the user is asked if he/she wants to override the file.
    * If the user's answer is negative, @p false will be returned.
    * @p true will be returned in all other cases.
    */
  bool okToWriteFile(const KUrl& url);

  // QValueList<MyMoneyAccount> accountList() const;

protected:
  /** save general Options like all bar positions and status as well as the geometry and the recent file list to the configuration
   * file
   */
  void saveOptions(void);

  /**
    * Creates the interfaces necessary for the plugins to work. Therefore,
    * this method must be called prior to loadPlugins().
    */
  void createInterfaces(void);

  /**
    * load all available plugins. Make sure you have called createInterfaces()
    * before you call this one.
    */
  void loadPlugins(void);

  /** read general Options again and initialize all variables like the recent file list
   */
  void readOptions(void);

  /** initializes the KActions of the application */
  void initActions(void);

  /** initializes the dynamic menus (account selectors) */
  void initDynamicMenus(void);

  /** sets up the statusbar for the main window by initialzing a statuslabel.
   */
  void initStatusBar(void);

  /** queryClose is called by KTMainWindow on each closeEvent of a window. Against the
   * default implementation (only returns true), this calles saveModified() on the document object to ask if the document shall
   * be saved if Modified; on cancel the closeEvent is rejected.
   * @see KTMainWindow#queryClose
   * @see KTMainWindow#closeEvent
   */
  virtual bool queryClose(void);

  /** queryExit is called by KTMainWindow when the last window of the application is going to be closed during the closeEvent().
   * Against the default implementation that just returns true, this calls saveOptions() to save the settings of the last window's
   * properties.
   * @see KTMainWindow#queryExit
   * @see KTMainWindow#closeEvent
   */
  virtual bool queryExit(void);

  void slotCheckSchedules(void);

  virtual void resizeEvent(QResizeEvent*);

  void createSchedule(MyMoneySchedule newSchedule, MyMoneyAccount& newAccount);

  /**
    * This method checks, if an account can be closed or not. An account
    * can be closed if:
    *
    * - the balance is zero and
    * - all children are already closed and
    * - there is no unfinished schedule referencing the account
    *
    * @param acc reference to MyMoneyAccount object in question
    * @retval true account can be closed
    * @retval false account cannot be closed
    */
  bool canCloseAccount(const MyMoneyAccount& acc) const;

  /**
    * Check if a list contains a payee with a given id
    *
    * @param list const reference to value list
    * @param id const reference to id
    *
    * @retval true object has been found
    * @retval false object is not in list
    */
  bool payeeInList(const QList<MyMoneyPayee>& list, const QString& id) const;

  /**
    * Mark the selected transactions as provided by @a flag. If
    * flag is @a MyMoneySplit::Unknown, the future state depends
    * on the current stat of the split's flag accoring to the
    * following table:
    *
    * - NotReconciled --> Cleared
    * - Cleared --> Reconciled
    * - Reconciled --> NotReconciled
    */
  void markTransaction(MyMoneySplit::reconcileFlagE flag);

  /**
    * This method allows to enter the next scheduled transaction of
    * the given schedule @a s. In case @a extendedKeys is @a true,
    * the given schedule can also be skipped or ignored.
    * If @a autoEnter is @a true and the schedule does not contain
    * an estimated value, the schedule is entered as is without further
    * interaction with the user. In all other cases, the user will
    * be presented a dialog and allowed to adjust the values for this
    * instance of the schedule.
    *
    * The transaction will be created and entered into the ledger
    * and the schedule updated.
    */
  KMyMoneyUtils::EnterScheduleResultCodeE enterSchedule(MyMoneySchedule& s, bool autoEnter = false, bool extendedKeys = false);

  /**
    * Creates a new institution entry in the MyMoneyFile engine
    *
    * @param institution MyMoneyInstitution object containing the data of
    *                    the institution to be created.
    */
  void createInstitution(MyMoneyInstitution& institution);

  /**
   * This method unmatches the currently selected transactions
   */
  void transactionUnmatch(void);

  /**
   * This method matches the currently selected transactions
   */
  void transactionMatch(void);

public slots:
  void slotFileInfoDialog(void);

  /** */
  void slotFileNew(void);

  /** Open a new window */
  void slotFileNewWindow(void);

  /** open a file and load it into the document*/
  void slotFileOpen(void);

  /** opens a file from the recent files menu */

  void slotFileOpenRecent(const KUrl& url);

  /** open a SQL database */
  void slotOpenDatabase(void);

  /**
    * saves the current document. If it has no name yet, the user
    * will be queried for it.
    *
    * @retval false save operation failed
    * @retval true save operation was successful
    */
  bool slotFileSave(void);

  /**
    * ask the user for the filename and save the current document
    *
    * @retval false save operation failed
    * @retval true save operation was successful
    */
  bool slotFileSaveAs(void);

  /**
   * ask the user to select a database and save the current document
   *
   * @retval false save operation failed
   * @retval true save operation was successful
   */
  bool slotSaveAsDatabase(void);

  /** asks for saving if the file is modified, then closes the actual file and window */
  void slotFileCloseWindow(void);

  /** asks for saving if the file is modified, then closes the actual file */
  void slotFileClose(void);

  /**
    * closes all open windows by calling close() on each memberList item
    * until the list is empty, then quits the application.
    * If queryClose() returns false because the user canceled the
    * saveModified() dialog, the closing breaks.
    */
  void slotFileQuit(void);

  void slotFileConsitencyCheck(void);

  /**
    * fires up the price table editor
    */
  void slotPriceDialog(void);

  /**
    * fires up the currency table editor
    */
  void slotCurrencyDialog(void);

  /**
    * Toggles the hide reconciled transactions setting
    */
  void slotHideReconciledTransactions(void);

  /**
    * Toggles the hide unused categories setting
    */
  void slotHideUnusedCategories(void);

  /**
    * Toggles the show all accounts setting
    */
  void slotShowAllAccounts(void);


  /**
    * changes the statusbar contents for the standard label permanently,
    * used to indicate current actions. Returns the previous value for
    * 'stacked' usage.
    *
    * @param text the text that is displayed in the statusbar
    */
  const QString slotStatusMsg(const QString &text);

  /**
    * This method changes the progress bar in the status line according
    * to the parameters @p current and @p total. The following special
    * cases exist:
    *
    * - current = -1 and total = -1  will reset the progress bar
    * - current = ?? and total != 0  will setup the 100% mark to @p total
    * - current = xx and total == 0  will set the percentage
    *
    * @param current the current value with respect to the initialised
    *                 100% mark
    * @param total the total value (100%)
    */
  void slotStatusProgressBar(int current, int total = 0);

  /** No descriptions */
  void slotProcessExited(void);

  /**
    * Called to update stock and currency prices from the user menu
    */
  void slotEquityPriceUpdate(void);

  /**
    * Imports a KMM statement into the engine, triggering the appropriate
    * UI to handle account matching, payee creation, and someday
    * payee and transaction matching.
    */
  bool slotStatementImport(const MyMoneyStatement& s);

  /**
    * Essentially similar to the above slot, except this will load the file
    * from disk first, given the URL.
    */
  bool slotStatementImport(const QString& url);

  /**
    * This slot starts the reconciliation of the currently selected account
    */
  void slotAccountReconcileStart(void);

  /**
    * This slot finishes a previously started reconciliation
    */
  void slotAccountReconcileFinish(void);

  /**
    * This slot postpones a previously started reconciliations
    */
  void slotAccountReconcilePostpone(void);

  /**
    * This slot deletes the currently selected account if possible
    */
  void slotAccountDelete(void);

  /**
    * This slot opens the account editor to edit the currently
    * selected account if possible
    */
  void slotAccountEdit(void);

  /**
    * This slot opens the selected account in the ledger view
    */
  void slotAccountOpen(const MyMoneyObject& = MyMoneyAccount());

  /**
    * This slot closes the currently selected account if possible
    */
  void slotAccountClose(void);

  /**
    * This slot re-openes the currently selected account if possible
    */
  void slotAccountReopen(void);

  /**
    * This slot reparents account @p src to be a child of account @p dest
    *
    * @param src account to be reparented
    * @param dest new parent
    */
  void slotReparentAccount(const MyMoneyAccount& src, const MyMoneyAccount& dest);

  /**
    * This slot reparents account @p src to be a held at institution @p dest
    *
    * @param src account to be reparented
    * @param dest new parent institution
    */
  void slotReparentAccount(const MyMoneyAccount& src, const MyMoneyInstitution& dest);

  /**
    * This slot creates a transaction report for the selected account
    * and opens it in the reports view.
    */
  void slotAccountTransactionReport(void);

  /**
    * This slot opens the account options menu at the current cursor
    * position.
    */
  void slotShowAccountContextMenu(const MyMoneyObject&);

  /**
    * This slot opens the schedule options menu at the current cursor
    * position.
    */
  void slotShowScheduleContextMenu(void);

  /**
    * This slot opens the institution options menu at the current cursor
    * position.
    */
  void slotShowInstitutionContextMenu(const MyMoneyObject&);

  /**
    * This slot opens the investment options menu at the current cursor
    * position.
    */
  void slotShowInvestmentContextMenu(void);

  /**
    * This slot opens the payee options menu at the current cursor
    * position.
    */
  void slotShowPayeeContextMenu(void);

  /**
    * This slot opens the budget options menu at the current cursor
    * position.
    */
  void slotShowBudgetContextMenu(void);

  /**
    * This slot opens the transaction options menu at the current cursor
    * position.
    */
  void slotShowTransactionContextMenu(void);

  /**
    * This slot opens the currency options menu at the current cursor
    * position.
    */
  void slotShowCurrencyContextMenu(void);

  /**
    * This slot collects information for a new scheduled transaction
    * and saves it in the engine. @sa slotScheduleNew(const MyMoneyTransaction&)
    */
  void slotScheduleNew(void);

  /**
    * This slot allows to edit information the currently selected schedule
    */
  void slotScheduleEdit(void);

  /**
    * This slot allows to delete the currently selected schedule
    */
  void slotScheduleDelete(void);

  /**
    * This slot allows to enter the next scheduled transaction of
    * the currently selected schedule
    */
  void slotScheduleEnter(void);

  /**
   * This slot allows to skip the next scheduled transaction of
   * the currently selected schedule
   */
  void slotScheduleSkip(void);

  /**
    * This slot fires up the KCalc application
    */
  void slotToolsStartKCalc(void);

  void slotResetSelections(void);

  void slotSelectAccount(const MyMoneyObject& account = MyMoneyAccount());

  void slotSelectInstitution(const MyMoneyObject& institution = MyMoneyInstitution());

  void slotSelectInvestment(const MyMoneyObject& account = MyMoneyAccount());

  void slotSelectSchedule(const MyMoneySchedule& schedule = MyMoneySchedule());

  void slotSelectPayees(const QList<MyMoneyPayee>& list);

  void slotSelectBudget(const QList<MyMoneyBudget>& list);

  void slotSelectTransactions(const KMyMoneyRegister::SelectedTransactions& list);

  void slotSelectCurrency(const MyMoneySecurity& currency = MyMoneySecurity());

  void slotTransactionMatch(void);

  /**
    * Brings up the new account wizard and saves the information.
    */
  void slotAccountNew(void);
  void slotAccountNew(MyMoneyAccount&);

  /**
    * Brings up the new category editor and saves the information.
    */
  void slotCategoryNew(void);

  /**
    * Brings up the new category editor and saves the information.
    * The dialog will be preset with the name and parent account.
    *
    * @param account reference of category to be created. The @p name member
    *                should be filled by the caller. The object will be filled
    *                with additional information during the creation process
    *                esp. the @p id member.
    * @param parent reference to parent account (defaults to none)
    */
  void slotCategoryNew(MyMoneyAccount& account, const MyMoneyAccount& parent = MyMoneyAccount());

  /**
    * This method updates all KAction items to the current state.
    */
  void slotUpdateActions(void);

  /**
    * Brings up the input dialog and saves the information.
    */
  void slotInstitutionNew(void);

  /**
    * Brings up the input dialog and saves the information. If
    * the institution has been created, the @a id member is filled,
    * otherwise it is empty.
    *
    * @param institution reference to data to be used to create the
    *                    institution. id member will be updated.
    */
  void slotInstitutionNew(MyMoneyInstitution& institution);

  /**
    * Called whenever a plugin is plugged in
    */
  void slotPluginPlug(KPluginInfo*);

  /**
    * Called whenever a plugin is unplugged
    */
  void slotPluginUnplug(KPluginInfo*);

private:
  // bool verifyImportedData(const MyMoneyAccount& account);

  /**
    * Load the status bar with the 'ready' message. This is hold in a single
    * place, so that is consistent with isReady().
    */
  void ready(void);

  /**
    * Check if the status bar contains the 'ready' message. The return
    * value is used e.g. to detect if a quit operation is allowed or not.
    *
    * @retval true application is idle
    * @retval false application is active working on a longer operation
    */
  bool isReady(void);

  /**
    * Delete a possibly existing transaction editor but make sure to remove
    * any reference to it so that we avoid using a half-dead object
    */
  void deleteTransactionEditor(void);

  /**
    * delete all selected transactions w/o further questions
    */
  void doDeleteTransactions(void);

  /**
    * Exchanges all references in transaction @a _t to account @a fromId
    * into references to account @a toId. Returns @a true if at least
    * one split has been changed, @a false otherwise.
    */
  bool exchangeAccountInTransaction(MyMoneyTransaction& _t, const QString& fromId, const QString& toId);

signals:
  /**
    * This signal is emitted when a new file is loaded. In the case file
    * is closed, this signal is also emitted with an empty url.
    */
  void fileLoaded(const KUrl& url);

  /**
    * This signal is emitted when a payee/list of payees has been selected by
    * the GUI. If no payee is selected or the selection is removed,
    * @p payees is identical to an empty QValueList. This signal is used
    * by plugins to get information about changes.
    */
  void payeesSelected(const QList<MyMoneyPayee>& payees);

  /**
    * This signal is emitted when a transaction/list of transactions has been selected by
    * the GUI. If no transaction is selected or the selection is removed,
    * @p transactions is identical to an empty QValueList. This signal is used
    * by plugins to get information about changes.
    */
  void transactionsSelected(const KMyMoneyRegister::SelectedTransactions& transactions);

  /**
    * This signal is sent out, when the user presses Ctrl+A or activates
    * the Select all transactions action.
    */
  void selectAllTransactions(void);

  /**
    * This signal is emitted when a list of budgets has been selected by
    * the GUI. If no budget is selected or the selection is removed,
    * @a budget is identical to an empty QValueList. This signal is used
    * by plugins to get information about changes.
    */
  void budgetSelected(const QList<MyMoneyBudget>& budget);
  void budgetRename(void);

  /**
    * This signal is emitted when a new account has been selected by
    * the GUI. If no account is selected or the selection is removed,
    * @a account is identical to MyMoneyAccount(). This signal is used
    * by plugins to get information about changes.
    */
  void accountSelected(const MyMoneyAccount& account);
  void investmentSelected(const MyMoneyAccount& account);

  /**
    * This signal is emitted when a new institution has been selected by
    * the GUI. If no institution is selected or the selection is removed,
    * @a institution is identical to MyMoneyInstitution(). This signal is used
    * by plugins to get information about changes.
    */
  void institutionSelected(const MyMoneyInstitution& institution);

  /**
    * This signal is emitted when a new schedule has been selected by
    * the GUI. If no schedule is selected or the selection is removed,
    * @a schedule is identical to MyMoneySchedule(). This signal is used
    * by plugins to get information about changes.
    */
  void scheduleSelected(const MyMoneySchedule& schedule);

  /**
    * This signal is emitted when a new currency has been selected by
    * the GUI. If no currency is selected or the selection is removed,
    * @a currency is identical to MyMoneySecurity(). This signal is used
    * by plugins to get information about changes.
    */
  void currencySelected(const MyMoneySecurity& currency);

  void payeeRename(void);
  void payeeCreated(const QString& id);

  void currencyRename(void);
  void currencyCreated(const QString& id);

  void startMatchTransaction(const MyMoneyTransaction& t);
  void cancelMatchTransaction(void);

  /**
    * This signal is emitted when an account has been successfully reconciled
    * and all transactions are updated in the engine. It can be used by plugins
    * to create reconciliation reports.
    *
    * @param account the account data
    * @param date the reconciliation date as provided through the dialog
    * @param startingBalance the starting balance as provided through the dialog
    * @param endingBalance the ending balance as provided through the dialog
    * @param transactionList reference to QValueList of QPair containing all
    *        transaction/split pairs processed by the reconciliation.
    */
  void accountReconciled(const MyMoneyAccount& account, const QDate& date, const MyMoneyMoney& startingBalance, const MyMoneyMoney& endingBalance, const QList<QPair<MyMoneyTransaction, MyMoneySplit> >& transactionList);

public:
  /**
    * This method retrieves a pointer to a QAction object from actionCollection().
    * If the action with the name @p actionName is not found, a pointer to
    * a static non-configured QAction object is returned and a warning is
    * printed to stderr.
    *
    * @param actionName name of the action to be retrieved
    * @return pointer to QAction object (or derivative)
    */
  QAction* action(const QString& actionName) const;

  /**
    * This method is implemented for convenience. It returns a dynamic_cast-ed
    * pointer to an action found in actionCollection().
    * If the action with the name @p actionName is not found or the object
    * is not of type KToggleAction, a pointer to a static non-configured
    * KToggleAction object is returned and a warning is printed to stderr.
    */
  KToggleAction* toggleAction(const QString& actionName) const;


private:
  /// \internal d-pointer class.
  class Private;
  /*
   * Actually, one should write "Private * const d" but that confuses the KIDL
   * compiler in this context. It complains about the const keyword. So we leave
   * it out here
   */
  /// \internal d-pointer instance.
  Private* d;
};

extern  KMyMoney2App *kmymoney2;

class KMStatus
{
public:
  KMStatus (const QString &text);
  ~KMStatus();
private:
  QString m_prevText;
};

#define KMSTATUS(msg) KMStatus _thisStatus(msg)

#endif // KMYMONEY2_H
