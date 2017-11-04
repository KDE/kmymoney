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
#ifndef KMYMONEY_H
#define KMYMONEY_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QList>
#include <QByteArray>
#include <QFileDialog>
#include <QUrl>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KXmlGuiWindow>

// ----------------------------------------------------------------------------
// Project Includes

#include <imymoneyprocessingcalendar.h>

#include "kmymoneyutils.h"

#include "mymoneyaccount.h"
#include "mymoney/onlinejob.h"
#include "onlinejobtyped.h"
#include "mymoneykeyvaluecontainer.h"
#include "mymoneymoney.h"
#include "selectedtransaction.h"

class QResizeEvent;
class KPluginMetaData;
class MyMoneyObject;
class MyMoneyInstitution;
class MyMoneyAccount;
class MyMoneySecurity;
class MyMoneyBudget;
class MyMoneyPayee;
class MyMoneyPrice;
class MyMoneyStatement;
class MyMoneyTag;
class MyMoneySplit;
class MyMoneyTransaction;
class WebConnect;
class creditTransfer;

template <class T> class onlineJobTyped;

enum class Action {
  FileOpenDatabase, FileSaveAsDatabase, FileBackup,
  FileImportGNC,
  FileImportStatement,
  FileImportTemplate, FileExportTemplate,
  #ifdef KMM_DEBUG
  FileDump,
  #endif
  FilePersonalData, FileInformation,
  EditFindTransaction,
  ViewTransactionDetail, ViewHideReconciled,
  ViewHideCategories, ViewShowAll,
  InstitutionNew, InstitutionEdit,
  InstitutionDelete,
  AccountNew, AccountOpen,
  AccountStartReconciliation, AccountFinishReconciliation,
  AccountPostponeReconciliation, AccountEdit,
  AccountDelete, AccountClose, AccountReopen,
  AccountTransactionReport, AccountBalanceChart,
  AccountUpdateMenu,
  AccountOnlineMap, AccountOnlineUnmap,
  AccountUpdate, AccountUpdateAll,
  AccountCreditTransfer,
  CategoryNew, CategoryEdit,
  CategoryDelete,
  ToolCurrencies,
  ToolPrices, ToolUpdatePrices,
  ToolConsistency, ToolPerformance,
  ToolSQL, ToolCalculator,
  SettingsAllMessages,
  HelpShow,
  TransactionNew, TransactionEdit,
  TransactionEnter, TransactionEditSplits,
  TransactionCancel, TransactionDelete,
  TransactionDuplicate, TransactionMatch,
  TransactionAccept, TransactionToggleReconciled,
  TransactionToggleCleared, TransactionReconciled,
  TransactionNotReconciled, TransactionSelectAll,
  TransactionGoToAccount, TransactionGoToPayee,
  TransactionCreateSchedule, TransactionAssignNumber,
  TransactionCombine, TransactionCopySplits,
  TransactionMoveMenu, TransactionMarkMenu,
  TransactionContextMarkMenu,
  InvestmentNew, InvestmentEdit,
  InvestmentDelete, InvestmentOnlinePrice,
  InvestmentManualPrice,
  ScheduleNew, ScheduleEdit,
  ScheduleDelete, ScheduleDuplicate,
  ScheduleEnter, ScheduleSkip,
  PayeeNew, PayeeRename, PayeeDelete,
  PayeeMerge,
  TagNew, TagRename, TagDelete,
  BudgetNew, BudgetRename, BudgetDelete,
  BudgetCopy, BudgetChangeYear, BudgetForecast,
  CurrencyNew, CurrencyRename, CurrencyDelete,
  CurrencySetBase,
  PriceNew, PriceDelete,
  PriceUpdate, PriceEdit,
  #ifdef KMM_DEBUG
  WizardNewUser, DebugTraces,
  #endif
  DebugTimers,
  OnlineJobDelete, OnlineJobEdit, OnlineJobLog
};

inline uint qHash(const Action key, uint seed)
{
  return ::qHash(static_cast<uint>(key), seed);
}

/*! \mainpage KMyMoney Main Page for API documentation.
 *
 * \section intro Introduction
 *
 * This is the API documentation for KMyMoney.  It should be used as a reference
 * for KMyMoney developers and users who wish to see how KMyMoney works.  This
 * documentation will be kept up-to-date as development progresses and should be
 * read for new features that have been developed in KMyMoney.
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
class KMyMoneyApp : public KXmlGuiWindow, public IMyMoneyProcessingCalendar
{
  Q_OBJECT

private slots:
  /**
    * Keep track of objects that are destroyed by external events
    */
  void slotObjectDestroyed(QObject* o);

  /**
    * Add a context menu to the list used by KMessageBox::informationList to display the consistency check results.
    */
  void slotInstallConsistencyCheckContextMenu();

  /**
    * Handle the context menu of the list used by KMessageBox::informationList to display the consistency check results.
    */
  void slotShowContextMenuForConsistencyCheck(const QPoint &);

protected slots:
  void slotFileSaveAsFilterChanged(const QString& filter);

  /**
    * This slot is intended to be used as part of auto saving. This is used when the
    * QTimer emits the timeout signal and simply checks that the file is dirty (has
    * received modifications to its contents), and call the appropriate method to
    * save the file. Furthermore, re-starts the timer (possibly not needed).
    * @author mvillarino 2005
    * @see KMyMoneyApp::slotDataChanged()
    */
  void slotAutoSave();

  /**
    * This slot re-enables all message for which the "Don't show again"
    * option had been selected.
    */
  void slotEnableMessages();

  /**
    * Called when the user asks for file information.
    */
  void slotFileFileInfo();

  /**
    * Called to run performance test.
    */
  void slotPerformanceTest();

  /**
    * Called to generate the sql to create kmymoney database tables etc.
    */
  void slotGenerateSql();

#ifdef KMM_DEBUG
  /**
    * Debugging only: turn on/off traces
    */
  void slotToggleTraces();
#endif

  /**
    * Debugging only: turn on/off timers
    */
  void slotToggleTimers();

  /**
    * Called when the user asks for the personal information.
    */
  void slotFileViewPersonal();

  /**
    * Opens a file selector dialog for the user to choose an existing OFX
    * file from the file system to be imported.  This slot is expected to
    * be called from the UI.
    */
  void slotGncImport();

  /**
   * Open a dialog with a chart of the balance for the currently selected
   * account (m_selectedAccount). Return once the dialog is closed. Don't do
   * anything if no account is selected or charts are not available.
   */
  void slotAccountChart();

  /**
    * Opens a file selector dialog for the user to choose an existing KMM
    * statement file from the file system to be imported.  This is for testing
    * only.  KMM statement files are not designed to be exposed to the user.
    */
  void slotStatementImport();

  void slotLoadAccountTemplates();
  void slotSaveAccountTemplates();

  /**
    * Open up the application wide settings dialog.
    *
    * @see KSettingsDlg
    */
  void slotSettings();

  /**
    * Called to show credits window.
    */
  void slotShowCredits();

  /**
    * Called when the user wishes to backup the current file
    */
  void slotBackupFile();

  /**
    * Perform mount operation before making a backup of the current file
    */
  void slotBackupMount();

  /**
    * Perform the backup write operation
    */
  bool slotBackupWriteFile();

  /**
    * Perform unmount operation after making a backup of the current file
    */
  void slotBackupUnmount();

  /**
    * Finish backup of the current file
    */
  void slotBackupFinish();

  /**
    * Handle events on making a backup of the current file
    */
  void slotBackupHandleEvents();

  void slotShowTipOfTheDay();

  void slotShowPreviousView();

  void slotShowNextView();

  /**
    * Brings up a dialog to let the user search for specific transaction(s).  It then
    * opens a results window to display those transactions.
    */
  void slotFindTransaction();

  /**
    * Destroys a possibly open the search dialog
    */
  void slotCloseSearchDialog();

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
  void slotPrintView();

  /**
    * Create a new investment
    */
  void slotInvestmentNew();

  /**
    * Create a new investment in a given @p parent investment account
    */
  void slotInvestmentNew(MyMoneyAccount& account, const MyMoneyAccount& parent);

  /**
    * This slot opens the investment editor to edit the currently
    * selected investment if possible
    */
  void slotInvestmentEdit();

  /**
    * Deletes the current selected investment.
    */
  void slotInvestmentDelete();

  /**
    * Performs online update for currently selected investment
    */
  void slotOnlinePriceUpdate();

  /**
    * Performs manual update for currently selected investment
    */
  void slotManualPriceUpdate();

  /**
    * Call this slot, if any configuration parameter has changed
    */
  void slotUpdateConfiguration();

  /**
    */
  bool slotPayeeNew(const QString& newnameBase, QString& id);
  void slotPayeeNew();

  /**
    */
  void slotPayeeDelete();

  /**
    * Slot that merges two or more selected payess into a new payee
    */
  void slotPayeeMerge();

  /**
    */
  void slotBudgetNew();

  /**
    */
  void slotBudgetDelete();

  /**
   */
  void slotBudgetCopy();

  /**
    */
  void slotBudgetChangeYear();

  /**
    */
  void slotBudgetForecast();

  /**
    */
  void slotCurrencyNew();

  /**
    */
  void slotCurrencyUpdate(const QString &currencyId, const QString& currencyName, const QString& currencyTradingSymbol);

  /**
    */
  void slotCurrencyDelete();

  /**
    */
  void slotCurrencySetBase();

  /**
    * This slot is used to start new features during the development cycle
    */
  void slotNewFeature();

  /**
    */
  void slotTransactionsNew();

  /**
    */
  void slotTransactionsEdit();

  /**
    */
  void slotTransactionsEditSplits();

  /**
    */
  void slotTransactionsDelete();

  /**
    */
  void slotTransactionsEnter();

  /**
    */
  void slotTransactionsCancel();

  /**
    */
  void slotTransactionsCancelOrEnter(bool& okToSelect);

  /**
    */
  void slotTransactionDuplicate();

  /**
    */
  void slotToggleReconciliationFlag();

  /**
    */
  void slotMarkTransactionCleared();

  /**
    */
  void slotMarkTransactionReconciled();

  /**
    */
  void slotMarkTransactionNotReconciled();

  /**
    */
  void slotTransactionGotoAccount();

  /**
    */
  void slotTransactionGotoPayee();

  /**
    */
  void slotTransactionCreateSchedule();

  /**
    */
  void slotTransactionAssignNumber();

  /**
    */
  void slotTransactionCombine();

  /**
   * This method takes the selected splits and checks that only one transaction (src)
   * has more than one split and all others have only a single one. It then copies the
   * splits of the @b src transaction to all others.
   */
  void slotTransactionCopySplits();

  /**
    * Accept the selected transactions that are marked as 'imported' and remove the flag
    */
  void slotTransactionsAccept();

  /**
    * This slot triggers an update of all views and restarts
    * a single shot timer to call itself again at beginning of
    * the next day.
    */
  void slotDateChanged();

  /**
    * This slot will be called when the engine data changed
    * and the application object needs to update its state.
    */
  void slotDataChanged();

  void slotMoveToAccount(const QString& id);

  void slotUpdateMoveToAccountMenu();

  /**
    * This slot collects information for a new scheduled transaction
    * based on transaction @a t and @a occurrence and saves it in the engine.
    */
  void slotScheduleNew(const MyMoneyTransaction& t, eMyMoney::Schedule::Occurrence occurrence = eMyMoney::Schedule::Occurrence::Monthly);

  /**
    */
  void slotScheduleDuplicate();

  void slotAccountMapOnline();
  void slotAccountUnmapOnline();
  void slotAccountUpdateOnline();
  void slotAccountUpdateOnlineAll();

  /**
   * @brief Start dialog for an online banking transfer
   */
  void slotNewOnlineTransfer();

  /**
   * @brief Start dialog to edit onlineJob if possible
   * @param onlineJob id to edit
   */
  void slotEditOnlineJob(const QString);

  /**
   * @brief Start dialog to edit onlineJob if possible
   */
  void slotEditOnlineJob(const onlineJob);

  /**
   * @brief Start dialog to edit onlineJob if possible
   */
  void slotEditOnlineJob(const onlineJobTyped<creditTransfer>);

  /**
   * @brief Saves an online banking job
   */
  void slotOnlineJobSave(onlineJob job);

  /**
   * @brief Queue an online banking job
   */
  void slotOnlineJobSend(onlineJob job);

  /**
   * @brief Send a list of onlineJobs
   */
  void slotOnlineJobSend(QList<onlineJob> jobs);

  /**
    * dummy method needed just for initialization
    */
  void slotRemoveJob();
  void slotEditJob();

  /**
   * @brief Show the log currently selected online job
   */
  void slotOnlineJobLog();
  void slotOnlineJobLog(const QStringList& onlineJobIds);

  void slotManageGpgKeys();
  void slotKeySelected(int idx);

  void slotStatusProgressDone();

public:
  /**
    * This method checks if there is at least one asset or liability account
    * in the current storage object. If not, it starts the new account wizard.
    */
  void createInitialAccount();

  /**
    * This method returns the last URL used or an empty URL
    * depending on the option setting if the last file should
    * be opened during startup or the open file dialog should
    * be displayed.
    *
    * @return URL of last opened file or empty if the program
    *         should start with the open file dialog
    */
  QUrl lastOpenedURL();

  /**
    * construtor of KMyMoneyApp, calls all init functions to create the application.
    */
  explicit KMyMoneyApp(QWidget* parent = 0);

  /**
    * Destructor
    */
  ~KMyMoneyApp();

  static void progressCallback(int current, int total, const QString&);

  void writeLastUsedDir(const QString& directory);
  QString readLastUsedDir() const;
  void writeLastUsedFile(const QString& fileName);
  QString readLastUsedFile() const;

  /**
    * Returns whether there is an importer available that can handle this file
    */
  bool isImportableFile(const QUrl &url);

  /**
    * This method is used to update the caption of the application window.
    * It sets the caption to "filename [modified] - KMyMoney".
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
  QList<QString> instanceList() const;

#ifdef KMM_DEBUG
  /**
    * Dump a list of the names of all defined KActions to stdout.
    */
  void dumpActions() const;
#endif

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
   * is returned. The @a key - @a value pair can be in the account's kvp
   * container or the account's online settings kvp container.
   */
  const MyMoneyAccount& account(const QString& key, const QString& value) const;

  /**
   * This method set the online parameters stored in @a kvps with the
   * account referenced by @a acc.
   */
  void setAccountOnlineParameters(const MyMoneyAccount& acc, const MyMoneyKeyValueContainer& kvps);

  QUrl selectFile(const QString& title, const QString& path, const QString& mask, QFileDialog::FileMode, QWidget *widget);

  void createAccount(MyMoneyAccount& newAccount, MyMoneyAccount& parentAccount, MyMoneyAccount& brokerageAccount, MyMoneyMoney openingBal);

  QString filename() const;

  /**
    * Checks if the file with the @a url already exists. If so,
    * the user is asked if he/she wants to override the file.
    * If the user's answer is negative, @p false will be returned.
    * @p true will be returned in all other cases.
    */
  bool okToWriteFile(const QUrl &url);

  /**
   * Return pointer to the WebConnect object
   */
  WebConnect* webConnect() const;

protected:
  /** save general Options like all bar positions and status as well as the geometry and the recent file list to the configuration
   * file
   */
  void saveOptions();

  /**
    * Creates the interfaces necessary for the plugins to work. Therefore,
    * this method must be called prior to loadPlugins().
    */
  void createInterfaces();

  /**
    * load all available plugins. Make sure you have called createInterfaces()
    * before you call this one.
    */
  void loadPlugins();

  /**
    * unload all available plugins. Make sure you have called loadPlugins()
    * before you call this one.
    */
  void unloadPlugins();

  /**
   * @brief Checks if the given plugin is loaded on start up
   *
   * This filters plugins which are loaded on demand only and deactivated plugins.
   * The configGroup must point to the correct group already.
   */
  bool isPluginEnabled(const KPluginMetaData& metaData, const KConfigGroup& configGroup);

  /**
   * read general options again and initialize all variables like the recent file list
   */
  void readOptions();

  /** initializes the KActions of the application */
  void initActions();

  /** initializes the dynamic menus (account selectors) */
  void initDynamicMenus();

  /**
   * sets up the statusbar for the main window by initialzing a statuslabel.
   */
  void initStatusBar();

  /**
   * @brief Establish connections between actions and views
   *
   * Must be called after creation of actions and views.
   */
  void connectActionsAndViews();

  /** queryClose is called by KMainWindow on each closeEvent of a window. Against the
   * default implementation (only returns true), this calls saveModified() on the document object to ask if the document shall
   * be saved if Modified; on cancel the closeEvent is rejected.
   * The settings are saved using saveOptions() if we are about to close.
   * @see KMainWindow#queryClose
   * @see QWidget#closeEvent
   */
  virtual bool queryClose();

  void slotCheckSchedules();

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
  KMyMoneyUtils::CanCloseAccountCodeE canCloseAccount(const MyMoneyAccount& acc) const;

  /**
   * This method checks if an account can be closed and enables/disables
   * the close account action
   * If disabled, it sets a tooltip explaning why it cannot be closed
   * @brief enableCloseAccountAction
   * @param acc reference to MyMoneyAccount object in question
   */
  void enableCloseAccountAction(const MyMoneyAccount& acc);

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
    * Check if a list contains a tag with a given id
    *
    * @param list const reference to value list
    * @param id const reference to id
    *
    * @retval true object has been found
    * @retval false object is not in list
    */
  bool tagInList(const QList<MyMoneyTag>& list, const QString& id) const;

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
  void markTransaction(eMyMoney::Split::State flag);

  /**
    * This method allows to skip the next scheduled transaction of
    * the given schedule @a s.
    *
    */
  void skipSchedule(MyMoneySchedule& s);

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
  void transactionUnmatch();

  /**
   * This method matches the currently selected transactions
   */
  void transactionMatch();

  /**
    * This method preloads the holidays for the duration of the default forecast period
    */
  void preloadHolidays();

public slots:
  void slotFileInfoDialog();

  /** */
  void slotFileNew();

  /** open a file and load it into the document*/
  void slotFileOpen();

  /** opens a file from the recent files menu */

  void slotFileOpenRecent(const QUrl &url);

  /** open a SQL database */
  void slotOpenDatabase();

  /**
    * saves the current document. If it has no name yet, the user
    * will be queried for it.
    *
    * @retval false save operation failed
    * @retval true save operation was successful
    */
  bool slotFileSave();

  /**
    * ask the user for the filename and save the current document
    *
    * @retval false save operation failed
    * @retval true save operation was successful
    */
  bool slotFileSaveAs();

  /**
   * ask the user to select a database and save the current document
   *
   * @retval false save operation failed
   * @retval true save operation was successful
   */
  bool saveAsDatabase();
  void slotSaveAsDatabase();

  /** asks for saving if the file is modified, then closes the actual file and window */
  void slotFileCloseWindow();

  /** asks for saving if the file is modified, then closes the actual file */
  void slotFileClose();

  /**
    * closes all open windows by calling close() on each memberList item
    * until the list is empty, then quits the application.
    * If queryClose() returns false because the user canceled the
    * saveModified() dialog, the closing breaks.
    */
  void slotFileQuit();

  void slotFileConsistencyCheck();

  /**
    * fires up the price table editor
    */
  void slotPriceDialog();

  /**
    * fires up the currency table editor
    */
  void slotCurrencyDialog();

  /**
    * dummy method needed just for initialization
    */
  void slotShowTransactionDetail();

  /**
    * Toggles the hide reconciled transactions setting
    */
  void slotHideReconciledTransactions();

  /**
    * Toggles the hide unused categories setting
    */
  void slotHideUnusedCategories();

  /**
    * Toggles the show all accounts setting
    */
  void slotShowAllAccounts();


  /**
    * changes the statusbar contents for the standard label permanently,
    * used to indicate current actions. Returns the previous value for
    * 'stacked' usage.
    *
    * @param text the text that is displayed in the statusbar
    */
  QString slotStatusMsg(const QString &text);

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

  /**
    * Called to update stock and currency prices from the user menu
    */
  void slotEquityPriceUpdate();

  /**
    * Imports a KMM statement into the engine, triggering the appropriate
    * UI to handle account matching, payee creation, and someday
    * payee and transaction matching.
    */
  bool slotStatementImport(const MyMoneyStatement& s, bool silent = false);

  /**
    * Essentially similar to the above slot, except this will load the file
    * from disk first, given the URL.
    */
  bool slotStatementImport(const QString& url);

  /**
    * This slot starts the reconciliation of the currently selected account
    */
  void slotAccountReconcileStart();

  /**
    * This slot finishes a previously started reconciliation
    */
  void slotAccountReconcileFinish();

  /**
    * This slot postpones a previously started reconciliations
    */
  void slotAccountReconcilePostpone();

  /**
    * This slot deletes the currently selected account if possible
    */
  void slotAccountDelete();

  /**
    * This slot opens the account editor to edit the currently
    * selected account if possible
    */
  void slotAccountEdit();

  /**
    * This slot opens the selected account in the ledger view
    */
  void slotAccountOpenEmpty();
  void slotAccountOpen(const MyMoneyObject&);

  /**
    * Preloads the input dialog with the data of the current
    * selected institution and brings up the input dialog
    * and saves the information entered.
    */
  void slotInstitutionEditEmpty();
  void slotInstitutionEdit(const MyMoneyObject &obj);

  /**
    * Deletes the current selected institution.
    */
  void slotInstitutionDelete();

  /**
    * This slot closes the currently selected account if possible
    */
  void slotAccountClose();

  /**
    * This slot re-openes the currently selected account if possible
    */
  void slotAccountReopen();

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
  void slotAccountTransactionReport();

  /**
    * This slot opens the account options menu at the current cursor
    * position.
    */
  void slotShowAccountContextMenu(const MyMoneyObject&);

  /**
    * This slot opens the schedule options menu at the current cursor
    * position.
    */
  void slotShowScheduleContextMenu();

  /**
    * This slot opens the institution options menu at the current cursor
    * position.
    */
  void slotShowInstitutionContextMenu(const MyMoneyObject&);

  /**
    * This slot opens the investment options menu at the current cursor
    * position.
    */
  void slotShowInvestmentContextMenu();

  /**
    * This slot opens the payee options menu at the current cursor
    * position.
    */
  void slotShowPayeeContextMenu();

  /**
    * This slot opens the tag options menu at the current cursor
    * position.
    */
  void slotShowTagContextMenu();

  /**
    * This slot opens the budget options menu at the current cursor
    * position.
    */
  void slotShowBudgetContextMenu();

  /**
    * This slot opens the transaction options menu at the current cursor
    * position.
    */
  void slotShowTransactionContextMenu();

  /**
    * This slot opens the currency options menu at the current cursor
    * position.
    */
  void slotShowCurrencyContextMenu();

  /**
    * This slot opens the price options menu at the current cursor
    * position.
    */
  void slotShowPriceContextMenu();

  /**
   * Open onlineJob options menu at current cursor position.
   */
  void slotShowOnlineJobContextMenu();

  /**
    * This slot collects information for a new scheduled transaction
    * and saves it in the engine. @sa slotScheduleNew(const MyMoneyTransaction&)
    */
  void slotScheduleNew();

  /**
    * This slot allows to edit information the currently selected schedule
    */
  void slotScheduleEdit();

  /**
    * This slot allows to delete the currently selected schedule
    */
  void slotScheduleDelete();

  /**
    * This slot allows to enter the next scheduled transaction of
    * the currently selected schedule
    */
  void slotScheduleEnter();

  /**
   * This slot allows to skip the next scheduled transaction of
   * the currently selected schedule
   */
  void slotScheduleSkip();

  /**
    */
  void slotTagNew(const QString& newnameBase, QString& id);
  void slotTagNew();

  /**
    */
  void slotTagDelete();

  /**
    * This slot fires up the KCalc application
    */
  void slotToolsStartKCalc();

  void slotResetSelections();

  void slotSelectAccount(const MyMoneyObject& account);

  void slotSelectInstitution(const MyMoneyObject& institution);

  void slotSelectInvestment(const MyMoneyObject& account);

  void slotSelectSchedule();
  void slotSelectSchedule(const MyMoneySchedule& schedule);

  void slotSelectPayees(const QList<MyMoneyPayee>& list);

  void slotSelectTags(const QList<MyMoneyTag>& list);

  void slotSelectBudget(const QList<MyMoneyBudget>& list);

  void slotSelectTransactions(const KMyMoneyRegister::SelectedTransactions& list);

  void slotSelectCurrency();
  void slotSelectCurrency(const MyMoneySecurity& currency);

  void slotSelectPrice();
  void slotSelectPrice(const MyMoneyPrice& price);

  void slotTransactionMatch();

  /**
    * Brings up the new account wizard and saves the information.
    */
  void slotAccountNew();
  void slotAccountNew(MyMoneyAccount&);

  /**
    * Brings up the new category editor and saves the information.
    */
  void slotCategoryNew();

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
  void slotCategoryNew(MyMoneyAccount& account, const MyMoneyAccount& parent);
  void slotCategoryNew(MyMoneyAccount& account);

  /**
    * This method updates all KAction items to the current state.
    */
  void slotUpdateActions();

  /**
    * Brings up the input dialog and saves the information.
    */
  void slotInstitutionNew();

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
   * Loads a plugin
   */
  void slotPluginLoad(const KPluginMetaData& metaData);

  /**
   * Unloads a plugin
   */
  void slotPluginUnload(const KPluginMetaData& metaData);

  void webConnect(const QString& sourceUrl, const QByteArray &asn_id);
  void webConnect(const QUrl url) { webConnect(url.path(), QByteArray()); }

private:
  /**
    * Create the transaction move menu and setup necessary connections.
    */
  void createTransactionMoveMenu();

  /**
    * This method sets the holidayRegion for use by the processing calendar.
    */
  void setHolidayRegion(const QString& holidayRegion);

  /**
    * Load the status bar with the 'ready' message. This is hold in a single
    * place, so that is consistent with isReady().
    */
  void ready();

  /**
    * Check if the status bar contains the 'ready' message. The return
    * value is used e.g. to detect if a quit operation is allowed or not.
    *
    * @retval true application is idle
    * @retval false application is active working on a longer operation
    */
  bool isReady();

  /**
    * Delete a possibly existing transaction editor but make sure to remove
    * any reference to it so that we avoid using a half-dead object
    */
  void deleteTransactionEditor();

  /**
    * delete all selected transactions w/o further questions
    */
  void doDeleteTransactions();

  /**
    * Re-implemented from IMyMoneyProcessingCalendar
    */
  bool isProcessingDate(const QDate& date) const;

  /**
    * Depending on the setting of AutoSaveOnQuit, this method
    * asks the user to save the file or not.
    *
    * @returns see return values of KMessageBox::warningYesNoCancel()
    */
  int askSaveOnClose();

  /**
    * Implement common task when deleting or merging payees
    */
  bool payeeReassign(int type);

signals:
  /**
    * This signal is emitted when a new file is loaded. In the case file
    * is closed, this signal is also emitted with an empty url.
    */
  void fileLoaded(const QUrl &url);

  /**
    * This signal is emitted when a payee/list of payees has been selected by
    * the GUI. If no payee is selected or the selection is removed,
    * @p payees is identical to an empty QList. This signal is used
    * by plugins to get information about changes.
    */
  void payeesSelected(const QList<MyMoneyPayee>& payees);

  /**
    * This signal is emitted when a tag/list of tags has been selected by
    * the GUI. If no tag is selected or the selection is removed,
    * @p tags is identical to an empty QList. This signal is used
    * by plugins to get information about changes.
    */
  void tagsSelected(const QList<MyMoneyTag>& tags);

  /**
    * This signal is emitted when a transaction/list of transactions has been selected by
    * the GUI. If no transaction is selected or the selection is removed,
    * @p transactions is identical to an empty QList. This signal is used
    * by plugins to get information about changes.
    */
  void transactionsSelected(const KMyMoneyRegister::SelectedTransactions& transactions);

  /**
    * This signal is sent out, when the user presses Ctrl+A or activates
    * the Select all transactions action.
    */
  void selectAllTransactions();

  /**
    * This signal is emitted when a list of budgets has been selected by
    * the GUI. If no budget is selected or the selection is removed,
    * @a budget is identical to an empty QList. This signal is used
    * by plugins to get information about changes.
    */
  void budgetSelected(const QList<MyMoneyBudget>& budget);
  void budgetRename();

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

  /**
    * This signal is emitted when a new price has been selected by
    * the GUI. If no price is selected or the selection is removed,
    * @a price is identical to MyMoneyPrice().
    */
  void priceSelected(const MyMoneyPrice& price);

  void payeeRename();
  void payeeCreated(const QString& id);

  void slotTagRename();
  void tagCreated(const QString& id);

  void currencyRename();
  void currencyCreated(const QString& id);

  void priceEdit();
  void priceNew();
  void priceDelete();
  void priceOnlineUpdate();

  void startMatchTransaction(const MyMoneyTransaction& t);
  void cancelMatchTransaction();

  /**
    * This signal is emitted when an account has been successfully reconciled
    * and all transactions are updated in the engine. It can be used by plugins
    * to create reconciliation reports.
    *
    * @param account the account data
    * @param date the reconciliation date as provided through the dialog
    * @param startingBalance the starting balance as provided through the dialog
    * @param endingBalance the ending balance as provided through the dialog
    * @param transactionList reference to QList of QPair containing all
    *        transaction/split pairs processed by the reconciliation.
    */
  void accountReconciled(const MyMoneyAccount& account, const QDate& date, const MyMoneyMoney& startingBalance, const MyMoneyMoney& endingBalance, const QList<QPair<MyMoneyTransaction, MyMoneySplit> >& transactionList);

public:

  bool isActionToggled(const Action _a);
  static const QHash<Action, QString> s_Actions;

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

extern KMyMoneyApp *kmymoney;
typedef void(KMyMoneyApp::*KMyMoneyAppFunc)();

class KMStatus
{
public:
  KMStatus(const QString &text);
  ~KMStatus();
private:
  QString m_prevText;
};

#define KMSTATUS(msg) KMStatus _thisStatus(msg)

#endif // KMYMONEY_H
