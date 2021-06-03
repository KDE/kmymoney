/*
    SPDX-FileCopyrightText: 2000-2001 Michael Edwardes <mte@users.sourceforge.net>
    SPDX-FileCopyrightText: 2017, 2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/
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
#include "selectedtransactions.h"
#include "mymoneysplit.h"
#include "mymoneytransaction.h"
#include "mymoneyenums.h"
#include "viewenums.h"

class QResizeEvent;
class MyMoneyObject;
class MyMoneyInstitution;
class MyMoneyAccount;
class MyMoneySecurity;
class MyMoneyPayee;
class MyMoneyPrice;
class MyMoneyTag;
class MyMoneySplit;
class MyMoneyTransaction;
class WebConnect;
class creditTransfer;
class IMyMoneyOperationsFormat;
class SelectedObjects;

template <class T> class onlineJobTyped;
typedef  void (*KMyMoneyAppCallback)(int, int, const QString &);

namespace eKMyMoney {
enum class FileAction;
}
namespace eDialogs {
enum class ScheduleResultCode;
}
namespace eMenu {
enum class Action;
enum class Menu;
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

private Q_SLOTS:
    /**
      * Add a context menu to the list used by KMessageBox::informationList to display the consistency check results.
      */
    void slotInstallConsistencyCheckContextMenu();

    /**
      * Handle the context menu of the list used by KMessageBox::informationList to display the consistency check results.
      */
    void slotShowContextMenuForConsistencyCheck(const QPoint &);

    void slotSelectionChanged(const SelectedObjects& selection);

    void slotAddSharedAction(eMenu::Action action, QAction* defaultAction);

protected Q_SLOTS:
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
      * Called to run performance test.
      */
    void slotPerformanceTest();

    /**
      * Called to generate the sql to create kmymoney database tables etc.
      */
    void slotGenerateSql();

#ifdef KMM_DEBUG
    /**
      * Called when the user asks for file information.
      */
    void slotFileFileInfo();

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

    /**
      * Call this slot, if any configuration parameter has changed
      */
    void slotUpdateConfiguration(const QString &dialogName);

    /**
      * This slot is used to start new features during the development cycle
      */
    void slotNewFeature();

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

    /**
      * This slot collects information for a new scheduled transaction
      * based on transaction @a t and @a occurrence and saves it in the engine.
      */
    void slotScheduleNew(const MyMoneyTransaction& t, eMyMoney::Schedule::Occurrence occurrence = eMyMoney::Schedule::Occurrence::Monthly);

    void slotStatusProgressDone();

    void slotCheckSchedules();

    void slotDeleteTransactions();
    void slotDuplicateTransactions();
    void slotCopySplits();
    void slotMarkTransactions();
    void slotMoveTransactionTo();
    void slotMatchTransaction();
    void slotAcceptTransaction();
    void slotCreateScheduledTransaction();
    void slotStartReconciliation();
    void slotReportReconciliation();
    void slotEnterOverdueSchedules();
    void slotFindTransaction();

    void slotExecuteAction();
    void slotExecuteActionWithData();

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
      * constructor of KMyMoneyApp, calls all init functions to create the application.
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

    void createAccount(MyMoneyAccount& newAccount, MyMoneyAccount& parentAccount, MyMoneyAccount& brokerageAccount, MyMoneyMoney openingBal);

    QString filename() const;
    QUrl filenameURL() const;
    void writeFilenameURL(const QUrl &url);

    void addToRecentFiles(const QUrl &url);
    QTimer* autosaveTimer();

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

    /**
     * Call this to find out if the currently open file is a sql database
     *
     * @retval true file is database
     * @retval false file is serial
     */
    bool isDatabase();

    /**
      * Call this to find out if the currently open file is native KMM
      *
      * @retval true file is native
      * @retval false file is foreign
      */
    bool isNativeFile();

    bool fileOpen() const;

    KMyMoneyAppCallback progressCallback();

    void consistencyCheck(bool alwaysDisplayResult);

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
     * read general options again and initialize all variables like the recent file list
     */
    void readOptions();

    /**
     * Gets pointers for menus preset by KXMLGUIFactory
     * @return pointers for menus
     */
    QHash<eMenu::Menu, QMenu *> initMenus();

    /**
     * Initializes QActions (names, object names, icons, some connections, shortcuts)
     * @return pointers for actions
     */
    QHash<eMenu::Action, QAction *> initActions();

    /** initializes the dynamic menus (account selectors) */
    void initDynamicMenus();

    /**
     * sets up the statusbar for the main window by initializing a statuslabel.
     */
    void initStatusBar();

    /** sets up the application icons */
    void initIcons();

    /** queryClose is called by KMainWindow on each closeEvent of a window. Against the
     * default implementation (only returns true), this calls saveModified() on the document object to ask if the document shall
     * be saved if Modified; on cancel the closeEvent is rejected.
     * The settings are saved using saveOptions() if we are about to close.
     * @see KMainWindow#queryClose
     * @see QWidget#closeEvent
     */
    bool queryClose() final override;

#ifdef KMM_DEBUG
    void resizeEvent(QResizeEvent*) final override;
#endif

    /**
      * This method preloads the holidays for the duration of the default forecast period
      */
    void preloadHolidays();

    bool event(QEvent * event) override;

public Q_SLOTS:

    void slotFileInfoDialog();

    bool isFileOpenedInAnotherInstance(const QUrl &url);

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
      * Create a new investment in a given @p parent investment account
      */
    void slotInvestmentNew(MyMoneyAccount& account, const MyMoneyAccount& parent);

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
      */
    void slotPayeeNew(const QString& newnameBase, QString& id);

    /**
      * This slot fires up the KCalc application
      */
    void slotToolsStartKCalc();

    /**
      * Brings up the new account wizard and saves the information.
      */
    void slotAccountNew(MyMoneyAccount&);

    void webConnect(const QString& sourceUrl, const QByteArray &asn_id);
    void webConnectUrl(const QUrl url);

private:
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
      * Re-implemented from IMyMoneyProcessingCalendar
      */
    bool isProcessingDate(const QDate& date) const final override;

Q_SIGNALS:
    /**
      * This signal is emitted when a new file is loaded. In the case file
      * is closed, this signal is also emitted with an empty url.
      */
    void fileLoaded(const QUrl &url);

    /**
      * This signal is emitted when a transaction/list of transactions has been selected by
      * the GUI. If no transaction is selected or the selection is removed,
      * @p transactions is identical to an empty QList. This signal is used
      * by plugins to get information about changes.
      */
    void transactionsSelected(const KMyMoneyRegister::SelectedTransactions& transactions);

    /**
     * This signal is emitted if a specific transaction with @a transactionId in account
     * @a accountId is selected
     */
    void transactionSelected(const QString accountId, const QString& transactionId);

    /**
      * This signal is sent out, when the user presses Ctrl+A or activates
      * the Select all transactions action.
      */
    void selectAllTransactions();

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

    void startMatchTransaction(const MyMoneyTransaction& t);
    void cancelMatchTransaction();

    void kmmFilePlugin(unsigned int);

    void selectionChanged(const SelectedObjects& selection);

public:

    bool isActionToggled(const eMenu::Action _a);
    static const QHash<eMenu::Action, QString> s_Actions;

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

public Q_SLOTS:
    bool slotFileNew();
    void slotFileOpen();
    bool slotFileOpenRecent(const QUrl &url);
    bool slotFileSave();
    bool slotFileSaveAs();
    bool slotFileClose();
    /**
      * closes all open windows by calling close() on each memberList item
      * until the list is empty, then quits the application.
      * If queryClose() returns false because the user canceled the
      * saveModified() dialog, the closing breaks.
      */
    void slotFileQuit();
};

extern KMyMoneyApp *kmymoney;

class KMStatus
{
public:
    explicit KMStatus(const QString &text);
    ~KMStatus();
private:
    QString m_prevText;
};

#define KMSTATUS(msg) KMStatus _thisStatus(msg)

#endif // KMYMONEY_H
