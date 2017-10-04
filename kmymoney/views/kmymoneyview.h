/***************************************************************************
                          kmymoneyview.h
                             -------------------
    copyright            : (C) 2000-2001 by Michael Edwardes <mte@users.sourceforge.net>
                               2004 by Thomas Baumgart <ipwizard@users.sourceforge.net>
                               2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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

#include "config-kmymoney.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QFileDevice>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KPageWidget>

// ----------------------------------------------------------------------------
// Project Includes
#include "selectedtransaction.h"
#include "mymoneyreport.h"

#ifdef KF5Activities_FOUND
namespace KActivities
{
class ResourceInstance;
}
#endif

namespace eAccountsModel {
  enum class Column;
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
class IMyMoneyStorageFormat;
class MyMoneyTransaction;
class KInvestmentView;
class KReportsView;
class KMyMoneyViewBase;
class MyMoneyReport;
class TransactionEditor;
class KForecastView;
class KOnlineJobOutbox;
class KMyMoneyTitleLabel;
class MyMoneyAccount;
class QLabel;
class QVBoxLayout;

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
public:
  // file actions for plugin
  enum fileActions {
    preOpen, postOpen, preSave, postSave, preClose, postClose
  };

  KOnlineJobOutbox* getOnlineJobOutbox() const {
    return m_onlineJobOutboxView;
  }

private:
  enum menuID {
    AccountNew = 1,
    AccountOpen,
    AccountReconcile,
    AccountEdit,
    AccountDelete,
    AccountOnlineMap,
    AccountOnlineUpdate,
    AccountOfxConnect,
    CategoryNew
  };

  enum storageTypeE {
    Memory = 0,
    Database
  } _storageType;

  KPageWidgetModel* m_model;

  KHomeView *m_homeView;
  KAccountsView *m_accountsView;
  KInstitutionsView *m_institutionsView;
  KCategoriesView *m_categoriesView;
  KPayeesView *m_payeesView;
  KTagsView *m_tagsView;
  KBudgetView *m_budgetView;
  KScheduledView *m_scheduledView;
  KGlobalLedgerView *m_ledgerView;
  KInvestmentView *m_investmentView;
  KReportsView* m_reportsView;
  KForecastView* m_forecastView;
  KOnlineJobOutbox* m_onlineJobOutboxView;

  QHash<View,KPageWidgetItem*> viewFrames;

  KMyMoneyTitleLabel* m_header;
  bool m_inConstructor;
  bool m_fileOpen;
  QFileDevice::Permissions m_fmode;
  int m_lastViewSelected;

  // Keep a note of the file type
  typedef enum _fileTypeE {
    KmmBinary = 0,  // native, binary
    KmmXML,        // native, XML
    KmmDb,         //  SQL database
    /* insert new native file types above this line */
    MaxNativeFileType,
    /* and non-native types below */
    GncXML         // Gnucash XML
  } fileTypeE;
  fileTypeE m_fileType;

#ifdef KF5Activities_FOUND
private:
  KActivities::ResourceInstance * m_activityResourceInstance;
#endif

private:
  void ungetString(QIODevice *qfile, char * buf, int len);

  /**
    * if no base currency is defined, start the dialog and force it to be set
    */
  void selectBaseCurrency();

  /**
    * This method attaches an empty storage object to the MyMoneyFile
    * object. It calls removeStorage() to remove a possibly attached
    * storage object.
    */
  void newStorage(storageTypeE = Memory);

  /**
    * This method removes an attached storage from the MyMoneyFile
    * object.
    */
  void removeStorage();

  void viewAccountList(const QString& selectAccount); // Show the accounts view

  static void progressCallback(int current, int total, const QString&);

  /**
    */
  void fixFile_0();
  void fixFile_1();
  void fixFile_2();
  void fixFile_3();

  /**
    */
  void fixLoanAccount_0(MyMoneyAccount acc);

  /**
    */
  void fixTransactions_0();
  void fixSchedule_0(MyMoneySchedule sched);
  void fixDuplicateAccounts_0(MyMoneyTransaction& t);

  void createSchedule(MyMoneySchedule s, MyMoneyAccount& a);

  void checkAccountName(const MyMoneyAccount& acc, const QString& name) const;

public:
  /**
    * The constructor for KMyMoneyView. Just creates all the tabs for the
    * different aspects of the MyMoneyFile.
    */
  explicit KMyMoneyView(KMyMoneyApp *kmymoney);

  /**
    * Destructor
    */
  ~KMyMoneyView();

  /**
    * Makes sure that a MyMoneyFile is open and has been created successfully.
    *
    * @return Whether the file is open and initialised
    */
  bool fileOpen();

  /**
    * Closes the open MyMoneyFile and frees all the allocated memory, I hope !
    */
  void closeFile();


  /**
    * Calls MyMoneyFile::readAllData which reads a MyMoneyFile into appropriate
    * data structures in memory.  The return result is examined to make sure no
    * errors occurred whilst parsing.
    *
    * @param url The URL to read from.
    *            If no protocol is specified, file:// is assumed.
    *
    * @return Whether the read was successful.
    */
  bool readFile(const QUrl &url);

  /**
    * Saves the data into permanent storage using the XML format.
    *
    * @param url The URL to save into.
    *            If no protocol is specified, file:// is assumed.
    * @param keyList QString containing a comma separated list of keys
    *            to be used for encryption. If @p keyList is empty,
    *            the file will be saved unencrypted (the default)
    *
    * @retval false save operation failed
    * @retval true save operation was successful
    */
  bool saveFile(const QUrl &url, const QString& keyList = QString());
  /**
   * Saves the data into permanent storage on a new or empty SQL database.
   *
   * @param url The pseudo of tyhe database
   *
   * @retval false save operation failed
   * @retval true save operation was successful
   */
  //const bool saveDatabase(const QUrl &url); This no longer relevant
  /**
   * Saves the data into permanent storage on a new or empty SQL database.
   *
   * @param url The pseudo URL of the database
   *
   * @retval false save operation failed
   * @retval true save operation was successful
   */
  bool saveAsDatabase(const QUrl &url);

  /**
    * Call this to find out if the currently open file is native KMM
    *
    * @retval true file is native
    * @retval false file is foreign
    */
  bool isNativeFile() {
    return (m_fileOpen && (m_fileType < MaxNativeFileType));
  }

  /**
   * Call this to find out if the currently open file is a sql database
   *
   * @retval true file is database
   * @retval false file is serial
   */
  bool isDatabase() {
    return (m_fileOpen && ((m_fileType == KmmDb)));
  }

  /**
    * Call this to see if the MyMoneyFile contains any unsaved data.
    *
    * @retval true if any data has been modified but not saved
    * @retval false otherwise
    */
  bool dirty();

  /**
    * Close the currently opened file and create an empty new file.
    *
    * @see MyMoneyFile
    */
  void newFile();

  /**
    * This method enables the state of all views (except home view) according
    * to an open file.
    */
  void enableViewsIfFileOpen();

  void addWidget(QWidget* w);

  void showPage(KPageWidgetItem* pageItem);

  /**
    * check if the current view allows to create a transaction
    *
    * @param list list of selected transactions
    * @param tooltip reference to string receiving the tooltip text
    *        which explains why the modify function is not available (in case
    *        of returning @c false)
    *
    * @retval true Yes, view allows to create a transaction (tooltip is not changed)
    * @retval false No, view cannot to create a transaction (tooltip is updated with message)
    */
  bool canCreateTransactions(const KMyMoneyRegister::SelectedTransactions& list, QString& tooltip) const;

  /**
    * check if the current view allows to modify (edit/delete) the selected transactions
    *
    * @param list list of selected transactions
    * @param tooltip reference to string receiving the tooltip text
    *        which explains why the modify function is not available (in case
    *        of returning @c false)
    *
    * @retval true Yes, view allows to edit/delete transactions (tooltip is not changed)
    * @retval false No, view cannot edit/delete transactions (tooltip is updated with message)
    */
  bool canModifyTransactions(const KMyMoneyRegister::SelectedTransactions& list, QString& tooltip) const;

  bool canDuplicateTransactions(const KMyMoneyRegister::SelectedTransactions& list, QString& tooltip) const;

  /**
  * check if the current view allows to edit the selected transactions
  *
  * @param list list of selected transactions
  * @param tooltip reference to string receiving the tooltip text
  *        which explains why the edit function is not available (in case
  *        of returning @c false)
  *
  * @retval true Yes, view allows to enter/edit transactions
  * @retval false No, view cannot enter/edit transactions
  */
  bool canEditTransactions(const KMyMoneyRegister::SelectedTransactions& list, QString& tooltip) const;

  /**
    * check if the current view allows to print something
    *
    * @retval true Yes, view allows to print
    * @retval false No, view cannot print
    */
  bool canPrint();

  TransactionEditor* startEdit(const KMyMoneyRegister::SelectedTransactions&);

  bool createNewTransaction();

  /**
    * Used to start reconciliation of account @a account. It switches the
    * ledger view into reconciliation mode and updates the view.
    *
    * @param account account which should be reconciled
    * @param reconciliationDate the statement date
    * @param endingBalance the ending balance entered for this account
    *
    * @retval true Reconciliation started
    * @retval false Account cannot be reconciled
    */
  bool startReconciliation(const MyMoneyAccount& account, const QDate& reconciliationDate, const MyMoneyMoney& endingBalance);

  /**
    * Used to finish reconciliation of account @a account. It switches the
    * ledger view to normal mode and updates the view.
    *
    * @param account account which should be reconciled
    */
  void finishReconciliation(const MyMoneyAccount& account);

  /**
    * This method updates names of currencies from file to localized names
    */
  void updateCurrencyNames();

  /**
    * This method loads all known currencies and saves them to the storage
    */
  void loadAllCurrencies();

  void showTitleBar(bool show);

  /**
    * This method changes the view type according to the settings.
    */
  void updateViewType();

  void slotAccountTreeViewChanged(const eAccountsModel::Column column, const bool show);

  void slotNetBalProChanged(const MyMoneyMoney &val, QLabel *label, const View view);

protected:
  /**
    * Overwritten because KMyMoney has it's custom header.
    */
  virtual bool showPageHeader() const;


public slots:
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
    * Called, whenever the ledger view should pop up and a specific
    * transaction in an account should be shown. If @p transaction
    * is empty, the last transaction should be selected
    *
    * @param acc The ID of the account to be shown
    * @param transaction The ID of the transaction to be selected
    */
  void slotLedgerSelected(const QString& acc, const QString& transaction = QString());

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
    * Called, whenever the schedule view should pop up and a specific
    * schedule should be shown.
    *
    * @param schedule The ID of the schedule to be shown
    */
  void slotScheduleSelected(const QString& schedule);

  /**
    * Called, whenever the report view should pop up and a specific
    * report should be shown.
    *
    * @param reportid The ID of the report to be shown
    */
  void slotShowReport(const QString& reportid);

  /**
    * Same as the above, but the caller passes in an actual report
    * definition to be shown.
    *
    * @param report The report to be shown
    */
  void slotShowReport(const MyMoneyReport& report);

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

private slots:
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

protected slots:
  /**
   * eventually replace this with KMyMoneyApp::slotCurrencySetBase().
   * it contains the same code
   *
   * @deprecated
   */
  void slotSetBaseCurrency(const MyMoneySecurity& baseCurrency);

private:
  /**
   * This method is called from readFile to open a database file which
   * is to be processed in 'proper' database mode, i.e. in-place updates
   *
   * @param dbaseURL pseudo-QUrl representation of database
   *
   * @retval true Database opened successfully
   * @retval false Could not open or read database
   */
  bool openDatabase(const QUrl &dbaseURL);
  /**
   * This method is used after a file or database has been
   * read into storage, and performs various initialization tasks
   *
   * @retval true all went okay
   * @retval false an exception occurred during this process
   */
  bool initializeStorage();
  /**
    * This method is used by saveFile() to store the data
    * either directly in the destination file if it is on
    * the local file system or in a temporary file when
    * the final destination is reached over a network
    * protocol (e.g. FTP)
    *
    * @param localFile the name of the local file
    * @param writer pointer to the formatter
    * @param plaintext whether to override any compression & encryption settings
    * @param keyList QString containing a comma separated list of keys to be used for encryption
    *            If @p keyList is empty, the file will be saved unencrypted
    *
    * @note This method will close the file when it is written.
    */
  void saveToLocalFile(const QString& localFile, IMyMoneyStorageFormat* writer, bool plaintext = false, const QString& keyList = QString());

  /**
    * Internal method used by slotAccountNew() and slotAccountCategory().
    */
  void accountNew(const bool createCategory);

  void resetViewSelection(const View);
  void connectView(const View);

signals:
  /**
    * This signal is emitted whenever a view is selected.
    * The parameter @p view is identified as one of KMyMoneyView::viewID.
    */
  void viewActivated(int view);

  /**
    * This signal is emitted whenever a new view is about to be selected.
    */
  void aboutToChangeView();

  void accountSelectedForContextMenu(const MyMoneyAccount& acc);

  void viewStateChanged(bool enabled);
  /**
    * This signal is emitted to inform the kmmFile plugin when various file actions
    * occur. The Action parameter distinguishes between them.
    */
  void kmmFilePlugin(unsigned int action);

  /**
    * Signal is emitted when reconciliation starts or ends. In case of end,
    * @a account is MyMoneyAccount()
    *
    * @param account account for which reconciliation starts or MyMoneyAccount()
    *                if reconciliation ends.
    * @param reconciliationDate the statement date
    * @param endingBalance collected ending balance when reconciliation starts
    *                0 otherwise
    */
  void reconciliationStarts(const MyMoneyAccount& account, const QDate& reconciliationDate, const MyMoneyMoney& endingBalance);

  /**
   * This signal is emitted after a data source has been closed
   */
  void fileClosed();

  /**
   * This signal is emitted after a data source has been opened
   */
  void fileOpened();
};

#endif
