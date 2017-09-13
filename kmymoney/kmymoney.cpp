/***************************************************************************
                          kmymoney.cpp
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes <mte@users.sourceforge.net>
                           (C) 2007 by Thomas Baumgart <ipwizard@users.sourceforge.net>

****************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "config-kmymoney.h"

#include "kmymoney.h"

// for _getpid
#ifdef Q_OS_WIN32                   //krazy:exclude=cpp
#include <process.h>
#else
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#endif

// ----------------------------------------------------------------------------
// Std C++ / STL Includes

#include <typeinfo>
#include <cstdio>
#include <iostream>
#include <memory>

// ----------------------------------------------------------------------------
// QT Includes

#include <QDir>
#include <QPrinter>
#include <QLayout>
#include <QSignalMapper>
#include <QDateTime>         // only for performance tests
#include <QTimer>
#include <QEventLoop>
#include <QByteArray>
#include <QBitArray>
#include <QBoxLayout>
#include <QResizeEvent>
#include <QLabel>
#include <QMenu>
#include <QProgressBar>
#include <QList>
#include <QUrl>
#include <QClipboard>
#include <QKeySequence>
#include <QIcon>
#include <QInputDialog>
#include <QProgressDialog>
#include <QStatusBar>

// ----------------------------------------------------------------------------
// KDE Includes

#include <ktoolbar.h>
#include <kmessagebox.h>
#include <KLocalizedString>
#include <kconfig.h>
#include <kstandardaction.h>
#include <kactioncollection.h>
#include <kactionmenu.h>
#include <ktip.h>
#include <krun.h>
#include <kconfigdialog.h>
#include <kxmlguifactory.h>
#include <krecentfilesaction.h>
#include <KRecentDirs>
#include <ktoolinvocation.h>
#include <KSharedConfig>
#include <KProcess>
#include <KAboutApplicationDialog>
#ifdef KF5Holidays_FOUND
#include <KHolidays/Holiday>
#include <KHolidays/HolidayRegion>
#endif

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyglobalsettings.h"
#include "kmymoneyadaptor.h"

#include "dialogs/settings/ksettingskmymoney.h"
#include "dialogs/kbackupdlg.h"
#include "dialogs/kenterscheduledlg.h"
#include "dialogs/kconfirmmanualenterdlg.h"
#include "dialogs/kmymoneypricedlg.h"
#include "dialogs/kcurrencyeditdlg.h"
#include "dialogs/kequitypriceupdatedlg.h"
#include "dialogs/kmymoneyfileinfodlg.h"
#include "dialogs/kfindtransactiondlg.h"
#include "dialogs/knewbankdlg.h"
#include "wizards/newinvestmentwizard/knewinvestmentwizard.h"
#include "dialogs/knewaccountdlg.h"
#include "dialogs/editpersonaldatadlg.h"
#include "dialogs/kselectdatabasedlg.h"
#include "dialogs/kcurrencycalculator.h"
#include "dialogs/keditscheduledlg.h"
#include "wizards/newloanwizard/knewloanwizard.h"
#include "wizards/newloanwizard/keditloanwizard.h"
#include "dialogs/kpayeereassigndlg.h"
#include "dialogs/ktagreassigndlg.h"
#include "dialogs/kcategoryreassigndlg.h"
#include "dialogs/kmergetransactionsdlg.h"
#include "wizards/endingbalancedlg/kendingbalancedlg.h"
#include "dialogs/kbalancechartdlg.h"
#include "dialogs/kgeneratesqldlg.h"
#include "dialogs/kloadtemplatedlg.h"
#include "dialogs/kgpgkeyselectiondlg.h"
#include "dialogs/ktemplateexportdlg.h"
#include "dialogs/transactionmatcher.h"
#include "wizards/newuserwizard/knewuserwizard.h"
#include "wizards/newaccountwizard/knewaccountwizard.h"
#include "dialogs/kbalancewarning.h"
#include "widgets/onlinejobmessagesview.h"

#include "widgets/kmymoneymvccombo.h"
#include "widgets/kmymoneycompletion.h"

#include "views/kmymoneyview.h"
#include "views/konlinejoboutbox.h"
#include "models/onlinejobmessagesmodel.h"

#include "mymoney/mymoneyutils.h"
#include "mymoney/mymoneystatement.h"
#include "mymoney/storage/mymoneystoragedump.h"
#include "mymoney/mymoneyforecast.h"

#include "mymoney/onlinejob.h"
#include "mymoney/onlinetransfer.h"
#include "mymoney/onlinejobadministration.h"

#include "converter/mymoneystatementreader.h"
#include "converter/mymoneytemplate.h"

#include "plugins/interfaces/kmmviewinterface.h"
#include "plugins/interfaces/kmmstatementinterface.h"
#include "plugins/interfaces/kmmimportinterface.h"
#include "plugins/interfaceloader.h"
#include "plugins/onlinepluginextended.h"
#include "pluginloader.h"

#include "icons/icons.h"

#include <libkgpgfile/kgpgfile.h>

#include <transactioneditor.h>
#include "konlinetransferform.h"
#include <QHBoxLayout>
#include <QFileDialog>

#include "kmymoneyutils.h"
#include "kcreditswindow.h"

#include "ledgerdelegate.h"

using namespace Icons;

static constexpr char recoveryKeyId[] = "59B0F826D2B08440";

// define the default period to warn about an expiring recoverkey to 30 days
// but allows to override this setting during build time
#ifndef RECOVER_KEY_EXPIRATION_WARNING
#define RECOVER_KEY_EXPIRATION_WARNING 30
#endif

const QHash<Action, QString> KMyMoneyApp::s_Actions {
  {Action::FileOpenDatabase, QStringLiteral("open_database")},
  {Action::FileSaveAsDatabase, QStringLiteral("saveas_database")},
  {Action::FileBackup, QStringLiteral("file_backup")},
  {Action::FileImportGNC, QStringLiteral("file_import_gnc")},
  {Action::FileImportStatement, QStringLiteral("file_import_statement")},
  {Action::FileImportTemplate, QStringLiteral("file_import_template")},
  {Action::FileExportTemplate, QStringLiteral("file_export_template")},
  {Action::FilePersonalData, QStringLiteral("view_personal_data")},
  #ifdef KMM_DEBUG
  {Action::FileDump, QStringLiteral("file_dump")},
  #endif
  {Action::FileInformation, QStringLiteral("view_file_info")},
  {Action::EditFindTransaction, QStringLiteral("edit_find_transaction")},
  {Action::ViewTransactionDetail, QStringLiteral("view_show_transaction_detail")},
  {Action::ViewHideReconciled, QStringLiteral("view_hide_reconciled_transactions")},
  {Action::ViewHideCategories, QStringLiteral("view_hide_unused_categories")},
  {Action::ViewShowAll, QStringLiteral("view_show_all_accounts")},
  {Action::InstitutionNew, QStringLiteral("institution_new")},
  {Action::InstitutionEdit, QStringLiteral("institution_edit")},
  {Action::InstitutionDelete, QStringLiteral("institution_delete")},
  {Action::AccountNew, QStringLiteral("account_new")},
  {Action::AccountOpen, QStringLiteral("account_open")},
  {Action::AccountStartReconciliation, QStringLiteral("account_reconcile")},
  {Action::AccountFinishReconciliation, QStringLiteral("account_reconcile_finish")},
  {Action::AccountPostponeReconciliation, QStringLiteral("account_reconcile_postpone")},
  {Action::AccountEdit, QStringLiteral("account_edit")},
  {Action::AccountDelete, QStringLiteral("account_delete")},
  {Action::AccountClose, QStringLiteral("account_close")},
  {Action::AccountReopen, QStringLiteral("account_reopen")},
  {Action::AccountTransactionReport, QStringLiteral("account_transaction_report")},
  {Action::AccountBalanceChart, QStringLiteral("account_chart")},
  {Action::AccountOnlineMap, QStringLiteral("account_online_map")},
  {Action::AccountOnlineUnmap, QStringLiteral("account_online_unmap")},
  {Action::AccountUpdateMenu, QStringLiteral("account_online_update_menu")},
  {Action::AccountUpdate, QStringLiteral("account_online_update")},
  {Action::AccountUpdateAll, QStringLiteral("account_online_update_all")},
  {Action::AccountCreditTransfer, QStringLiteral("account_online_new_credit_transfer")},
  {Action::CategoryNew, QStringLiteral("category_new")},
  {Action::CategoryEdit, QStringLiteral("category_edit")},
  {Action::CategoryDelete, QStringLiteral("category_delete")},
  {Action::ToolCurrencies, QStringLiteral("tools_currency_editor")},
  {Action::ToolPrices, QStringLiteral("tools_price_editor")},
  {Action::ToolUpdatePrices, QStringLiteral("tools_update_prices")},
  {Action::ToolConsistency, QStringLiteral("tools_consistency_check")},
  {Action::ToolPerformance, QStringLiteral("tools_performancetest")},
  {Action::ToolSQL, QStringLiteral("tools_generate_sql")},
  {Action::ToolCalculator, QStringLiteral("tools_kcalc")},
  {Action::SettingsAllMessages, QStringLiteral("settings_enable_messages")},
  {Action::SettingsLanguage, QStringLiteral("settings_language")},
  {Action::HelpShow, QStringLiteral("help_show_tip")},
  {Action::TransactionNew, QStringLiteral("transaction_new")},
  {Action::TransactionEdit, QStringLiteral("transaction_edit")},
  {Action::TransactionEnter, QStringLiteral("transaction_enter")},
  {Action::TransactionEditSplits, QStringLiteral("transaction_editsplits")},
  {Action::TransactionCancel, QStringLiteral("transaction_cancel")},
  {Action::TransactionDelete, QStringLiteral("transaction_delete")},
  {Action::TransactionDuplicate, QStringLiteral("transaction_duplicate")},
  {Action::TransactionMatch, QStringLiteral("transaction_match")},
  {Action::TransactionAccept, QStringLiteral("transaction_accept")},
  {Action::TransactionToggleReconciled, QStringLiteral("transaction_mark_toggle")},
  {Action::TransactionToggleCleared, QStringLiteral("transaction_mark_cleared")},
  {Action::TransactionReconciled, QStringLiteral("transaction_mark_reconciled")},
  {Action::TransactionNotReconciled, QStringLiteral("transaction_mark_notreconciled")},
  {Action::TransactionSelectAll, QStringLiteral("transaction_select_all")},
  {Action::TransactionGoToAccount, QStringLiteral("transaction_goto_account")},
  {Action::TransactionGoToPayee, QStringLiteral("transaction_goto_payee")},
  {Action::TransactionCreateSchedule, QStringLiteral("transaction_create_schedule")},
  {Action::TransactionAssignNumber, QStringLiteral("transaction_assign_number")},
  {Action::TransactionCombine, QStringLiteral("transaction_combine")},
  {Action::TransactionCopySplits, QStringLiteral("transaction_copy_splits")},
  {Action::TransactionMoveMenu, QStringLiteral("transaction_move_menu")},
  {Action::TransactionMarkMenu, QStringLiteral("transaction_mark_menu")},
  {Action::TransactionContextMarkMenu, QStringLiteral("transaction_context_mark_menu")},
  {Action::InvestmentNew, QStringLiteral("investment_new")},
  {Action::InvestmentEdit, QStringLiteral("investment_edit")},
  {Action::InvestmentDelete, QStringLiteral("investment_delete")},
  {Action::InvestmentOnlinePrice, QStringLiteral("investment_online_price_update")},
  {Action::InvestmentManualPrice, QStringLiteral("investment_manual_price_update")},
  {Action::ScheduleNew, QStringLiteral("schedule_new")},
  {Action::ScheduleEdit, QStringLiteral("schedule_edit")},
  {Action::ScheduleDelete, QStringLiteral("schedule_delete")},
  {Action::ScheduleDuplicate, QStringLiteral("schedule_duplicate")},
  {Action::ScheduleEnter, QStringLiteral("schedule_enter")},
  {Action::ScheduleSkip, QStringLiteral("schedule_skip")},
  {Action::PayeeNew, QStringLiteral("payee_new")},
  {Action::PayeeRename, QStringLiteral("payee_rename")},
  {Action::PayeeDelete, QStringLiteral("payee_delete")},
  {Action::PayeeMerge, QStringLiteral("payee_merge")},
  {Action::TagNew, QStringLiteral("tag_new")},
  {Action::TagRename, QStringLiteral("tag_rename")},
  {Action::TagDelete, QStringLiteral("tag_delete")},
  {Action::BudgetNew, QStringLiteral("budget_new")},
  {Action::BudgetRename, QStringLiteral("budget_rename")},
  {Action::BudgetDelete, QStringLiteral("budget_delete")},
  {Action::BudgetCopy, QStringLiteral("budget_copy")},
  {Action::BudgetChangeYear, QStringLiteral("budget_change_year")},
  {Action::BudgetForecast, QStringLiteral("budget_forecast")},
  {Action::CurrencyNew, QStringLiteral("currency_new")},
  {Action::CurrencyRename, QStringLiteral("currency_rename")},
  {Action::CurrencyDelete, QStringLiteral("currency_delete")},
  {Action::CurrencySetBase, QStringLiteral("currency_setbase")},
  {Action::PriceNew, QStringLiteral("price_new")},
  {Action::PriceEdit, QStringLiteral("price_edit")},
  {Action::PriceUpdate, QStringLiteral("price_update")},
  {Action::PriceDelete, QStringLiteral("price_delete")},
  #ifdef KMM_DEBUG
  {Action::WizardNewUser, QStringLiteral("new_user_wizard")},
  {Action::DebugTraces, QStringLiteral("debug_traces")},
  #endif
  {Action::DebugTimers, QStringLiteral("debug_timers")},
  {Action::OnlineJobDelete, QStringLiteral("onlinejob_delete")},
  {Action::OnlineJobEdit, QStringLiteral("onlinejob_edit")},
  {Action::OnlineJobLog, QStringLiteral("onlinejob_log")},
};

enum backupStateE {
  BACKUP_IDLE = 0,
  BACKUP_MOUNTING,
  BACKUP_COPYING,
  BACKUP_UNMOUNTING
};

typedef void(KMyMoneyApp::*KMyMoneyAppFunc)();

class KMyMoneyApp::Private
{
public:
  Private(KMyMoneyApp *app) :
      q(app),
      m_ft(0),
      m_moveToAccountSelector(0),
      m_statementXMLindex(0),
      m_balanceWarning(0),
      m_collectingStatements(false),
      m_pluginLoader(0),
      m_backupResult(0),
      m_backupMount(0),
      m_ignoreBackupExitCode(false),
      m_myMoneyView(0),
      m_progressBar(0),
      m_smtReader(0),
      m_searchDlg(0),
      m_autoSaveTimer(0),
      m_progressTimer(0),
      m_inAutoSaving(false),
      m_transactionEditor(0),
      m_endingBalanceDlg(0),
      m_saveEncrypted(0),
      m_additionalKeyLabel(0),
      m_additionalKeyButton(0),
      m_recentFiles(0),
#ifdef KF5Holidays_FOUND
      m_holidayRegion(0),
#endif
      m_applicationIsReady(true) {
    // since the days of the week are from 1 to 7,
    // and a day of the week is used to index this bit array,
    // resize the array to 8 elements (element 0 is left unused)
    m_processingDays.resize(8);

  }

  void closeFile();
  void unlinkStatementXML();
  void moveInvestmentTransaction(const QString& fromId,
                                 const QString& toId,
                                 const MyMoneyTransaction& t);
  QList<QPair<MyMoneyTransaction, MyMoneySplit> > automaticReconciliation(const MyMoneyAccount &account,
      const QList<QPair<MyMoneyTransaction, MyMoneySplit> > &transactions,
      const MyMoneyMoney &amount);


  /**
    * The public interface.
    */
  KMyMoneyApp * const q;

  MyMoneyFileTransaction*       m_ft;
  kMyMoneyAccountSelector*      m_moveToAccountSelector;
  int                           m_statementXMLindex;
  KBalanceWarning*              m_balanceWarning;

  bool                          m_collectingStatements;
  QStringList                   m_statementResults;
  KMyMoneyPlugin::PluginLoader* m_pluginLoader;
  QString                       m_lastPayeeEnteredId;

  /** the configuration object of the application */
  KSharedConfigPtr m_config;

  /**
   * @brief List of all plugged plugins
   *
   * The key is the file name of the plugin.
   */
  QMap<QString, KMyMoneyPlugin::Plugin*> m_plugins;

  /**
   * @brief List of plugged importer plugins
   *
   * The key is the objectName of the plugin.
   */
  QMap<QString, KMyMoneyPlugin::ImporterPlugin*> m_importerPlugins;

  /**
   * @brief List of plugged online plugins
   *
   * The key is the objectName of the plugin.
   */
  QMap<QString, KMyMoneyPlugin::OnlinePlugin*> m_onlinePlugins;

  /**
    * The following variable represents the state while crafting a backup.
    * It can have the following values
    *
    * - IDLE: the default value if not performing a backup
    * - MOUNTING: when a mount command has been issued
    * - COPYING:  when a copy command has been issued
    * - UNMOUNTING: when an unmount command has been issued
    */
  backupStateE   m_backupState;

  /**
    * This variable keeps the result of the backup operation.
    */
  int     m_backupResult;

  /**
    * This variable is set, when the user selected to mount/unmount
    * the backup volume.
    */
  bool    m_backupMount;

  /**
    * Flag for internal run control
    */
  bool    m_ignoreBackupExitCode;

  KProcess m_proc;

  /// A pointer to the view holding the tabs.
  KMyMoneyView *m_myMoneyView;

  /// The URL of the file currently being edited when open.
  QUrl  m_fileName;

  bool m_startDialog;
  QString m_mountpoint;

  QProgressBar* m_progressBar;
  QTime         m_lastUpdate;
  QLabel*       m_statusLabel;

  MyMoneyStatementReader* m_smtReader;
  // allows multiple imports to be launched trough web connect and to be executed sequentially
  QQueue<QString> m_importUrlsQueue;
  KFindTransactionDlg* m_searchDlg;

  QObject*              m_pluginInterface;

  MyMoneyAccount        m_selectedAccount;
  MyMoneyAccount        m_reconciliationAccount;
  MyMoneyAccount        m_selectedInvestment;
  MyMoneyInstitution    m_selectedInstitution;
  MyMoneySchedule       m_selectedSchedule;
  MyMoneySecurity       m_selectedCurrency;
  MyMoneyPrice          m_selectedPrice;
  QList<MyMoneyPayee>   m_selectedPayees;
  QList<MyMoneyTag>     m_selectedTags;
  QList<MyMoneyBudget>  m_selectedBudgets;
  KMyMoneyRegister::SelectedTransactions m_selectedTransactions;

  // This is Auto Saving related
  bool                  m_autoSaveEnabled;
  QTimer*               m_autoSaveTimer;
  QTimer*               m_progressTimer;
  int                   m_autoSavePeriod;
  bool                  m_inAutoSaving;

  // pointer to the current transaction editor
  TransactionEditor*    m_transactionEditor;

  // Reconciliation dialog
  KEndingBalanceDlg*    m_endingBalanceDlg;

  // Pointer to the combo box used for key selection during
  // File/Save as
  KComboBox*            m_saveEncrypted;

  // id's that need to be remembered
  QString               m_accountGoto, m_payeeGoto;

  QStringList           m_additionalGpgKeys;
  QLabel*               m_additionalKeyLabel;
  QPushButton*          m_additionalKeyButton;

  KRecentFilesAction*   m_recentFiles;

#ifdef KF5Holidays_FOUND
  // used by the calendar interface for schedules
  KHolidays::HolidayRegion* m_holidayRegion;
#endif
  QBitArray             m_processingDays;
  QMap<QDate, bool>     m_holidayMap;
  QStringList           m_consistencyCheckResult;
  bool                  m_applicationIsReady;

  // methods
  void consistencyCheck(bool alwaysDisplayResults);
  void setCustomColors();
  void copyConsistencyCheckResults();
  void saveConsistencyCheckResults();
};

KMyMoneyApp::KMyMoneyApp(QWidget* parent) :
    KXmlGuiWindow(parent),
    d(new Private(this))
{
#ifdef KMM_DBUS
  new KmymoneyAdaptor(this);
  QDBusConnection::sessionBus().registerObject("/KMymoney", this);
  QDBusConnection::sessionBus().interface()->registerService(
    "org.kde.kmymoney", QDBusConnectionInterface::DontQueueService);
#endif
  // Register the main engine types used as meta-objects
  qRegisterMetaType<MyMoneyMoney>("MyMoneyMoney");
  qRegisterMetaType<MyMoneySecurity>("MyMoneySecurity");

  // preset the pointer because we need it during the course of this constructor
  kmymoney = this;
  d->m_config = KSharedConfig::openConfig();

  d->setCustomColors();

  MyMoneyTransactionFilter::setFiscalYearStart(KMyMoneyGlobalSettings::firstFiscalMonth(), KMyMoneyGlobalSettings::firstFiscalDay());

  updateCaption(true);

  QFrame* frame = new QFrame;
  frame->setFrameStyle(QFrame::NoFrame);
  // values for margin (11) and spacing(6) taken from KDialog implementation
  QBoxLayout* layout = new QBoxLayout(QBoxLayout::TopToBottom, frame);
  layout->setContentsMargins(2, 2, 2, 2);
  layout->setSpacing(6);

  {
    QString themeName = KMyMoneySettings::iconsTheme();                 // get theme user wants
    if (!themeName.isEmpty() && themeName != QLatin1Literal("system"))  // if it isn't default theme then set it
      QIcon::setThemeName(themeName);
    Icons::setIconThemeNames(QIcon::themeName());                       // get whatever theme user ends up with and hope our icon names will fit that theme
  }

  initStatusBar();
  initActions();

  initDynamicMenus();

  d->m_myMoneyView = new KMyMoneyView(this/*the global variable kmymoney is not yet assigned. So we pass it here*/);
  layout->addWidget(d->m_myMoneyView, 10);
  connect(d->m_myMoneyView, SIGNAL(aboutToChangeView()), this, SLOT(slotResetSelections()));
  connect(d->m_myMoneyView, SIGNAL(currentPageChanged(KPageWidgetItem*,KPageWidgetItem*)),
          this, SLOT(slotUpdateActions()));

  connectActionsAndViews();

  ///////////////////////////////////////////////////////////////////
  // call inits to invoke all other construction parts
  readOptions();

  // now initialize the plugin structure
  createInterfaces();
  loadPlugins();

  setCentralWidget(frame);

  connect(&d->m_proc, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(slotBackupHandleEvents()));

  // force to show the home page if the file is closed
  connect(actionCollection()->action(s_Actions[Action::ViewTransactionDetail]), &QAction::toggled, d->m_myMoneyView, &KMyMoneyView::slotShowTransactionDetail);

  d->m_backupState = BACKUP_IDLE;

  QLocale locale;
  int weekStart = locale.firstDayOfWeek();
  int weekEnd = weekStart-1;
  if (weekEnd < Qt::Monday) {
    weekEnd = Qt::Sunday;
  }
  bool startFirst = (weekStart < weekEnd);
  for (int i = 0; i < 8; i++) {
    if (startFirst)
      d->m_processingDays.setBit(i, (i >= weekStart && i <= weekEnd));
    else
      d->m_processingDays.setBit(i, (i >= weekStart || i <= weekEnd));
  }
  d->m_autoSaveTimer = new QTimer(this);
  d->m_progressTimer = new QTimer(this);

  connect(d->m_autoSaveTimer, SIGNAL(timeout()), this, SLOT(slotAutoSave()));
  connect(d->m_progressTimer, SIGNAL(timeout()), this, SLOT(slotStatusProgressDone()));

  // make sure, we get a note when the engine changes state
  connect(MyMoneyFile::instance(), SIGNAL(dataChanged()), this, SLOT(slotDataChanged()));

  // make sure we have a balance warning object
  d->m_balanceWarning = new KBalanceWarning(this);

  // setup the initial configuration
  slotUpdateConfiguration();

  // kickstart date change timer
  slotDateChanged();

  connect(this, SIGNAL(fileLoaded(QUrl)), onlineJobAdministration::instance(), SLOT(updateOnlineTaskProperties()));

}

KMyMoneyApp::~KMyMoneyApp()
{
  // delete cached objects since the are in the way
  // when unloading the plugins
  onlineJobAdministration::instance()->clearCaches();

  // we need to unload all plugins before we destroy anything else
  unloadPlugins();

  delete d->m_searchDlg;
  delete d->m_transactionEditor;
  delete d->m_endingBalanceDlg;
  delete d->m_moveToAccountSelector;
#ifdef KF5Holidays_FOUND
  delete d->m_holidayRegion;
#endif
  delete d;
}

QUrl KMyMoneyApp::lastOpenedURL()
{
  QUrl url = d->m_startDialog ? QUrl() : d->m_fileName;

  if (!url.isValid()) {
    url = QUrl::fromUserInput(readLastUsedFile());
  }

  ready();

  return url;
}

void KMyMoneyApp::slotObjectDestroyed(QObject* o)
{
  if (o == d->m_moveToAccountSelector) {
    d->m_moveToAccountSelector = 0;
  }
}

void KMyMoneyApp::slotInstallConsistencyCheckContextMenu()
{
  // this code relies on the implementation of KMessageBox::informationList to add a context menu to that list,
  // please adjust it if it's necessary or rewrite the way the consistency check results are displayed
  if (QWidget* dialog = QApplication::activeModalWidget()) {
    if (QListWidget* widget = dialog->findChild<QListWidget *>()) {
      // give the user a hint that the data can be saved
      widget->setToolTip(i18n("This is the consistency check log, use the context menu to copy or save it."));
      widget->setWhatsThis(widget->toolTip());
      widget->setContextMenuPolicy(Qt::CustomContextMenu);
      connect(widget, SIGNAL(customContextMenuRequested(QPoint)), SLOT(slotShowContextMenuForConsistencyCheck(QPoint)));
    }
  }
}

void KMyMoneyApp::slotShowContextMenuForConsistencyCheck(const QPoint &pos)
{
  // allow the user to save the consistency check results
  if (QWidget* widget = qobject_cast< QWidget* >(sender())) {
    QMenu contextMenu(widget);
    QAction* copy = new QAction(i18n("Copy to clipboard"), widget);
    QAction* save = new QAction(i18n("Save to file"), widget);
    contextMenu.addAction(copy);
    contextMenu.addAction(save);
    QAction *result = contextMenu.exec(widget->mapToGlobal(pos));
    if (result == copy) {
      // copy the consistency check results to the clipboard
      d->copyConsistencyCheckResults();
    } else if (result == save) {
      // save the consistency check results to a file
      d->saveConsistencyCheckResults();
    }
  }
}

void KMyMoneyApp::createTransactionMoveMenu()
{
  if (!d->m_moveToAccountSelector) {
    QWidget* w = factory()->container("transaction_move_menu", this);
    QMenu *menu = dynamic_cast<QMenu*>(w);
    if (menu) {
      QWidgetAction *accountSelectorAction = new QWidgetAction(menu);
      d->m_moveToAccountSelector = new kMyMoneyAccountSelector(menu, 0, false);
      d->m_moveToAccountSelector->setObjectName("transaction_move_menu_selector");
      accountSelectorAction->setDefaultWidget(d->m_moveToAccountSelector);
      menu->addAction(accountSelectorAction);
      connect(d->m_moveToAccountSelector, SIGNAL(destroyed(QObject*)), this, SLOT(slotObjectDestroyed(QObject*)));
      connect(d->m_moveToAccountSelector, SIGNAL(itemSelected(QString)), this, SLOT(slotMoveToAccount(QString)));
    }
  }
}

void KMyMoneyApp::initDynamicMenus()
{
  connect(this, SIGNAL(accountSelected(MyMoneyAccount)), this, SLOT(slotUpdateMoveToAccountMenu()));
  connect(this, SIGNAL(transactionsSelected(KMyMoneyRegister::SelectedTransactions)), this, SLOT(slotUpdateMoveToAccountMenu()));
  connect(MyMoneyFile::instance(), SIGNAL(dataChanged()), this, SLOT(slotUpdateMoveToAccountMenu()));
}

void KMyMoneyApp::initActions()
{
  KActionCollection *aC = actionCollection();

  // *************
  // Adding standard actions
  // *************
  KStandardAction::openNew(this, &KMyMoneyApp::slotFileNew, aC);
  KStandardAction::open(this, &KMyMoneyApp::slotFileOpen, aC);
  d->m_recentFiles = KStandardAction::openRecent(this, &KMyMoneyApp::slotFileOpenRecent, aC);
  KStandardAction::save(this, &KMyMoneyApp::slotFileSave, aC);
  KStandardAction::saveAs(this, &KMyMoneyApp::slotFileSaveAs, aC);
  KStandardAction::close(this, &KMyMoneyApp::slotFileClose, aC);
  KStandardAction::quit(this, &KMyMoneyApp::slotFileQuit, aC);
  KStandardAction::print(this, &KMyMoneyApp::slotPrintView, aC);
  KStandardAction::preferences(this, &KMyMoneyApp::slotSettings, aC);

  /* Look-up table for all custom actions.
  It's required for:
  1) building QList with QActions to be added to ActionCollection
  2) adding custom features to QActions like e.g. overlaid icon or keyboard shortcut
  */
  QHash<Action, QAction *> lutActions;

  // *************
  // Adding all actions
  // *************
  {
    // struct for QAction's vital (except icon) informations
    struct actionInfo {
      Action          action;
      KMyMoneyAppFunc callback;
      QString         text;
      Icon            icon;
    };

    const QVector<actionInfo> actionInfos {
      // *************
      // The File menu
      // *************
      {Action::FileOpenDatabase,              &KMyMoneyApp::slotOpenDatabase,                 i18n("Open database..."),                           Icon::SVNUpdate},
      {Action::FileSaveAsDatabase,            &KMyMoneyApp::slotSaveAsDatabase,               i18n("Save as database..."),                        Icon::FileArchiver},
      {Action::FileBackup,                    &KMyMoneyApp::slotBackupFile,                   i18n("Backup..."),                                  Icon::Empty},
      {Action::FileImportGNC,                 &KMyMoneyApp::slotGncImport,                    i18n("GnuCash..."),                                 Icon::Empty},
      {Action::FileImportStatement,           &KMyMoneyApp::slotStatementImport,              i18n("Statement file..."),                          Icon::Empty},
      {Action::FileImportTemplate,            &KMyMoneyApp::slotLoadAccountTemplates,         i18n("Account Template..."),                        Icon::Empty},
      {Action::FileExportTemplate,            &KMyMoneyApp::slotSaveAccountTemplates,         i18n("Account Template..."),                        Icon::Empty},
      {Action::FilePersonalData,              &KMyMoneyApp::slotFileViewPersonal,             i18n("Personal Data..."),                           Icon::UserProperties},
#ifdef KMM_DEBUG
      {Action::FileDump,                      &KMyMoneyApp::slotFileFileInfo,                 i18n("Dump Memory"),                                Icon::Empty},
#endif
      {Action::FileInformation,               &KMyMoneyApp::slotFileInfoDialog,               i18n("File-Information..."),                        Icon::DocumentProperties},
      // *************
      // The Edit menu
      // *************
      {Action::EditFindTransaction,           &KMyMoneyApp::slotFindTransaction,              i18n("Find transaction..."),                        Icon::Empty},
      // *************
      // The View menu
      // *************
      {Action::ViewTransactionDetail,         &KMyMoneyApp::slotShowTransactionDetail,        i18n("Show Transaction Detail"),                    Icon::ViewTransactionDetail},
      {Action::ViewHideReconciled,            &KMyMoneyApp::slotHideReconciledTransactions,   i18n("Hide reconciled transactions"),               Icon::HideReconciled},
      {Action::ViewHideCategories,            &KMyMoneyApp::slotHideUnusedCategories,         i18n("Hide unused categories"),                     Icon::HideCategories},
      {Action::ViewShowAll,                   &KMyMoneyApp::slotShowAllAccounts,              i18n("Show all accounts"),                          Icon::Empty},
      // *********************
      // The institutions menu
      // *********************
      {Action::InstitutionNew,                &KMyMoneyApp::slotInstitutionNew,               i18n("New institution..."),                         Icon::Empty},
      {Action::InstitutionEdit,               &KMyMoneyApp::slotInstitutionEdit,              i18n("Edit institution..."),                        Icon::Empty},
      {Action::InstitutionDelete,             &KMyMoneyApp::slotInstitutionDelete,            i18n("Delete institution..."),                      Icon::Empty},
      // *****************
      // The accounts menu
      // *****************
      {Action::AccountNew,                    &KMyMoneyApp::slotAccountNew,                   i18n("New account..."),                             Icon::Empty},
      {Action::AccountOpen,                   &KMyMoneyApp::slotAccountOpen,                  i18n("Open ledger"),                                Icon::ViewFinancialList},
      {Action::AccountStartReconciliation,    &KMyMoneyApp::slotAccountReconcileStart,        i18n("Reconcile..."),                               Icon::Reconcile},
      {Action::AccountFinishReconciliation,   &KMyMoneyApp::slotAccountReconcileFinish,       i18nc("Finish reconciliation", "Finish"),           Icon::Empty},
      {Action::AccountPostponeReconciliation, &KMyMoneyApp::slotAccountReconcilePostpone,     i18n("Postpone reconciliation"),                    Icon::MediaPlaybackPause},
      {Action::AccountEdit,                   &KMyMoneyApp::slotAccountEdit,                  i18n("Edit account..."),                            Icon::Empty},
      {Action::AccountDelete,                 &KMyMoneyApp::slotAccountDelete,                i18n("Delete account..."),                          Icon::Empty},
      {Action::AccountClose,                  &KMyMoneyApp::slotAccountClose,                 i18n("Close account"),                              Icon::Empty},
      {Action::AccountReopen,                 &KMyMoneyApp::slotAccountReopen,                i18n("Reopen account"),                             Icon::Empty},
      {Action::AccountTransactionReport,      &KMyMoneyApp::slotAccountTransactionReport,     i18n("Transaction report"),                         Icon::ViewFinancialList},
      {Action::AccountBalanceChart,           &KMyMoneyApp::slotAccountChart,                 i18n("Show balance chart..."),                      Icon::OfficeChartLine},
      {Action::AccountOnlineMap,              &KMyMoneyApp::slotAccountMapOnline,             i18n("Map account..."),                             Icon::NewsSubscribe},
      {Action::AccountOnlineUnmap,            &KMyMoneyApp::slotAccountUnmapOnline,           i18n("Unmap account..."),                           Icon::NewsUnsubscribe},
      {Action::AccountUpdateMenu,             &KMyMoneyApp::slotAccountUpdateOnline,          i18nc("Update online accounts menu", "Update"),     Icon::Empty},
      {Action::AccountUpdate,                 &KMyMoneyApp::slotAccountUpdateOnline,          i18n("Update account..."),                          Icon::Empty},
      {Action::AccountUpdateAll,              &KMyMoneyApp::slotAccountUpdateOnlineAll,       i18n("Update all accounts..."),                     Icon::Empty},
      {Action::AccountCreditTransfer,         &KMyMoneyApp::slotNewOnlineTransfer,            i18n("New credit transfer"),                        Icon::Empty},
      // *******************
      // The categories menu
      // *******************
      {Action::CategoryNew,                   &KMyMoneyApp::slotCategoryNew,                  i18n("New category..."),                            Icon::Empty},
      {Action::CategoryEdit,                  &KMyMoneyApp::slotAccountEdit,                  i18n("Edit category..."),                           Icon::Empty},
      {Action::CategoryDelete,                &KMyMoneyApp::slotAccountDelete,                i18n("Delete category..."),                         Icon::Empty},
      // **************
      // The tools menu
      // **************
      {Action::ToolCurrencies,                &KMyMoneyApp::slotCurrencyDialog,               i18n("Currencies..."),                              Icon::ViewCurrencyList},
      {Action::ToolPrices,                    &KMyMoneyApp::slotPriceDialog,                  i18n("Prices..."),                                  Icon::Empty},
      {Action::ToolUpdatePrices,              &KMyMoneyApp::slotEquityPriceUpdate,            i18n("Update Stock and Currency Prices..."),        Icon::Empty},
      {Action::ToolConsistency,               &KMyMoneyApp::slotFileConsistencyCheck,         i18n("Consistency Check"),                          Icon::Empty},
      {Action::ToolPerformance,               &KMyMoneyApp::slotPerformanceTest,              i18n("Performance-Test"),                           Icon::Fork},
      {Action::ToolSQL,                       &KMyMoneyApp::slotGenerateSql,                  i18n("Generate Database SQL"),                      Icon::Empty},
      {Action::ToolCalculator,                &KMyMoneyApp::slotToolsStartKCalc,              i18n("Calculator..."),                              Icon::AccessoriesCalculator},
      // *****************
      // The settings menu
      // *****************
      {Action::SettingsAllMessages,           &KMyMoneyApp::slotEnableMessages,               i18n("Enable all messages"),                        Icon::Empty},
      {Action::SettingsLanguage,              &KMyMoneyApp::slotKDELanguageSettings,          i18n("KDE language settings..."),                   Icon::Empty},
      // *************
      // The help menu
      // *************
      {Action::HelpShow,                      &KMyMoneyApp::slotShowTipOfTheDay,              i18n("&Show tip of the day"),                       Icon::Tip},
      // ***************************
      // Actions w/o main menu entry
      // ***************************
      {Action::TransactionNew,                &KMyMoneyApp::slotTransactionsNew,              i18nc("New transaction button", "New"),             Icon::Empty},
      {Action::TransactionEdit,               &KMyMoneyApp::slotTransactionsEdit,             i18nc("Edit transaction button", "Edit"),           Icon::Empty},
      {Action::TransactionEnter,              &KMyMoneyApp::slotTransactionsEnter,            i18nc("Enter transaction", "Enter"),                Icon::DialogOK},
      {Action::TransactionEditSplits,         &KMyMoneyApp::slotTransactionsEditSplits,       i18nc("Edit split button", "Edit splits"),          Icon::Split},
      {Action::TransactionCancel,             &KMyMoneyApp::slotTransactionsCancel,           i18nc("Cancel transaction edit", "Cancel"),         Icon::DialogCancel},
      {Action::TransactionDelete,             &KMyMoneyApp::slotTransactionsDelete,           i18nc("Delete transaction", "Delete"),              Icon::EditDelete},
      {Action::TransactionDuplicate,          &KMyMoneyApp::slotTransactionDuplicate,         i18nc("Duplicate transaction", "Duplicate"),        Icon::EditCopy},
      {Action::TransactionMatch,              &KMyMoneyApp::slotTransactionMatch,             i18nc("Button text for match transaction", "Match"),Icon::Empty},
      {Action::TransactionAccept,             &KMyMoneyApp::slotTransactionsAccept,           i18nc("Accept 'imported' and 'matched' transaction", "Accept"), Icon::Empty},
      {Action::TransactionToggleReconciled,   &KMyMoneyApp::slotToggleReconciliationFlag,     i18nc("Toggle reconciliation flag", "Toggle"),      Icon::Empty},
      {Action::TransactionToggleCleared,      &KMyMoneyApp::slotMarkTransactionCleared,       i18nc("Mark transaction cleared", "Cleared"),       Icon::Empty},
      {Action::TransactionReconciled,         &KMyMoneyApp::slotMarkTransactionReconciled,    i18nc("Mark transaction reconciled", "Reconciled"), Icon::Empty},
      {Action::TransactionNotReconciled,      &KMyMoneyApp::slotMarkTransactionNotReconciled, i18nc("Mark transaction not reconciled", "Not reconciled"),     Icon::Empty},
      {Action::TransactionSelectAll,          &KMyMoneyApp::selectAllTransactions,            i18nc("Select all transactions", "Select all"),     Icon::Empty},
      {Action::TransactionGoToAccount,        &KMyMoneyApp::slotTransactionGotoAccount,       i18n("Go to account"),                              Icon::GoJump},
      {Action::TransactionGoToPayee,          &KMyMoneyApp::slotTransactionGotoPayee,         i18n("Go to payee"),                                Icon::GoJump},
      {Action::TransactionCreateSchedule,     &KMyMoneyApp::slotTransactionCreateSchedule,    i18n("Create scheduled transaction..."),            Icon::AppointmentNew},
      {Action::TransactionAssignNumber,       &KMyMoneyApp::slotTransactionAssignNumber,      i18n("Assign next number"),                         Icon::Empty},
      {Action::TransactionCombine,            &KMyMoneyApp::slotTransactionCombine,           i18nc("Combine transactions", "Combine"),           Icon::Empty},
      {Action::TransactionCopySplits,         &KMyMoneyApp::slotTransactionCopySplits,        i18n("Copy splits"),                                Icon::Empty},
      //Investment
      {Action::InvestmentNew,                 &KMyMoneyApp::slotInvestmentNew,                i18n("New investment..."),                          Icon::Empty},
      {Action::InvestmentEdit,                &KMyMoneyApp::slotInvestmentEdit,               i18n("Edit investment..."),                         Icon::Empty},
      {Action::InvestmentDelete,              &KMyMoneyApp::slotInvestmentDelete,             i18n("Delete investment..."),                       Icon::Empty},
      {Action::InvestmentOnlinePrice,         &KMyMoneyApp::slotOnlinePriceUpdate,            i18n("Online price update..."),                     Icon::Empty},
      {Action::InvestmentManualPrice,         &KMyMoneyApp::slotManualPriceUpdate,            i18n("Manual price update..."),                     Icon::Empty},
      //Schedule
      {Action::ScheduleNew,                   &KMyMoneyApp::slotScheduleNew,                  i18n("New scheduled transaction"),                  Icon::AppointmentNew},
      {Action::ScheduleEdit,                  &KMyMoneyApp::slotScheduleEdit,                 i18n("Edit scheduled transaction"),                 Icon::DocumentEdit},
      {Action::ScheduleDelete,                &KMyMoneyApp::slotScheduleDelete,               i18n("Delete scheduled transaction"),               Icon::EditDelete},
      {Action::ScheduleDuplicate,             &KMyMoneyApp::slotScheduleDuplicate,            i18n("Duplicate scheduled transaction"),            Icon::EditCopy},
      {Action::ScheduleEnter,                 &KMyMoneyApp::slotScheduleEnter,                i18n("Enter next transaction..."),                  Icon::KeyEnter},
      {Action::ScheduleSkip,                  &KMyMoneyApp::slotScheduleSkip,                 i18n("Skip next transaction..."),                   Icon::MediaSeekForward},
      //Payees
      {Action::PayeeNew,                      &KMyMoneyApp::slotPayeeNew,                     i18n("New payee"),                                  Icon::ListAddUser},
      {Action::PayeeRename,                   &KMyMoneyApp::payeeRename,                      i18n("Rename payee"),                               Icon::PayeeRename},
      {Action::PayeeDelete,                   &KMyMoneyApp::slotPayeeDelete,                  i18n("Delete payee"),                               Icon::ListRemoveUser},
      {Action::PayeeMerge,                    &KMyMoneyApp::slotPayeeMerge,                   i18n("Merge payees"),                               Icon::PayeeMerge},
      //Tags
      {Action::TagNew,                        &KMyMoneyApp::slotTagNew,                       i18n("New tag"),                                    Icon::ListAddTag},
      {Action::TagRename,                     &KMyMoneyApp::tagRename,                        i18n("Rename tag"),                                 Icon::TagRename},
      {Action::TagDelete,                     &KMyMoneyApp::slotTagDelete,                    i18n("Delete tag"),                                 Icon::ListRemoveTag},
      //Budget
      {Action::BudgetNew,                     &KMyMoneyApp::slotBudgetNew,                    i18n("New budget"),                                 Icon::Empty},
      {Action::BudgetRename,                  &KMyMoneyApp::budgetRename,                     i18n("Rename budget"),                              Icon::Empty},
      {Action::BudgetDelete,                  &KMyMoneyApp::slotBudgetDelete,                 i18n("Delete budget"),                              Icon::Empty},
      {Action::BudgetCopy,                    &KMyMoneyApp::slotBudgetCopy,                   i18n("Copy budget"),                                Icon::Empty},
      {Action::BudgetChangeYear,              &KMyMoneyApp::slotBudgetChangeYear,             i18n("Change budget year"),                         Icon::ViewCalendar},
      {Action::BudgetForecast,                &KMyMoneyApp::slotBudgetForecast,               i18n("Budget based on forecast"),                   Icon::ViewForecast},
      //Currency actions
      {Action::CurrencyNew,                   &KMyMoneyApp::slotCurrencyNew,                  i18n("New currency"),                               Icon::Empty},
      {Action::CurrencyRename,                &KMyMoneyApp::currencyRename,                   i18n("Rename currency"),                            Icon::EditRename},
      {Action::CurrencyDelete,                &KMyMoneyApp::slotCurrencyDelete,               i18n("Delete currency"),                            Icon::EditDelete},
      {Action::CurrencySetBase,               &KMyMoneyApp::slotCurrencySetBase,              i18n("Select as base currency"),                    Icon::KMyMoney},
      //Price actions
      {Action::PriceNew,                      &KMyMoneyApp::priceNew,                         i18n("New price..."),                               Icon::DocumentNew},
      {Action::PriceEdit,                     &KMyMoneyApp::priceEdit,                        i18n("Edit price..."),                              Icon::DocumentEdit},
      {Action::PriceUpdate,                   &KMyMoneyApp::priceOnlineUpdate,                i18n("Online Price Update..."),                     Icon::Empty},
      {Action::PriceDelete,                   &KMyMoneyApp::priceDelete,                      i18n("Delete price..."),                            Icon::EditDelete},
      //debug actions
#ifdef KMM_DEBUG
      {Action::WizardNewUser,                 &KMyMoneyApp::slotNewFeature,                   i18n("Test new feature"),                           Icon::Empty},
      {Action::DebugTraces,                   &KMyMoneyApp::slotToggleTraces,                 i18n("Debug Traces"),                               Icon::Empty},
#endif
      {Action::DebugTimers,                   &KMyMoneyApp::slotToggleTimers,                 i18n("Debug Timers"),                               Icon::Empty},
      // onlineJob actions
      {Action::OnlineJobDelete,               &KMyMoneyApp::slotRemoveJob,                    i18n("Remove credit transfer"),                     Icon::EditDelete},
      {Action::OnlineJobEdit,                 &KMyMoneyApp::slotEditJob,                      i18n("Edit credit transfer"),                       Icon::DocumentEdit},
      {Action::OnlineJobLog,                  &KMyMoneyApp::slotOnlineJobLog,                 i18n("Show log"),                                   Icon::Empty},
    };

    foreach (const auto info, actionInfos) {
      QAction *a = new QAction(0);
      // KActionCollection::addAction by name sets object name anyways,
      // so, as better alternative, set it here right from the start
      a->setObjectName(s_Actions.value(info.action));
      a->setText(info.text);
      if (info.icon != Icon::Empty) // no need to set empty icon
        a->setIcon(QIcon::fromTheme(g_Icons.value(info.icon)));
      connect(a, &QAction::triggered, this, info.callback);
      lutActions.insert(info.action, a);  // store QAction's pointer for later processing
    }
  }

  // *************
  // Setting some of added actions checkable
  // *************
  {
    // Some of acitions schould be checkable,
    // so set them here
    const QVector<Action> checkableActions {
      Action::ViewTransactionDetail, Action::ViewHideReconciled, Action::ViewHideCategories,
    #ifdef KMM_DEBUG
          Action::DebugTraces,
    #endif
          Action::ViewShowAll
    };

    foreach (const auto it, checkableActions)
      lutActions[it]->setCheckable(true);
  }

  // *************
  // Setting custom icons for some of added actions
  // *************
  {
    // struct for QAction's icon overlays
    struct iconOverlayInfo {
      Action action;
      Icon icon;
      Icon overlay;
      Qt::Corner corner;
    };

    const QVector<iconOverlayInfo> actionOverlaidIcons {
      {Action::EditFindTransaction,         Icon::ViewFinancialTransfer,    Icon::EditFind,       Qt::BottomRightCorner},
      {Action::InstitutionNew,              Icon::ViewBank,                 Icon::ListAdd,        Qt::BottomRightCorner},
      {Action::InstitutionEdit,             Icon::ViewBank,                 Icon::DocumentEdit,   Qt::BottomRightCorner},
      {Action::InstitutionDelete,           Icon::ViewBank,                 Icon::EditDelete,     Qt::BottomRightCorner},
      {Action::AccountNew,                  Icon::ViewBankAccount,          Icon::ListAdd,        Qt::TopRightCorner},
      {Action::AccountFinishReconciliation, Icon::Merge,                    Icon::DialogOK,       Qt::BottomRightCorner},
      {Action::AccountEdit,                 Icon::ViewBankAccount,          Icon::DocumentEdit,   Qt::BottomRightCorner},
      {Action::AccountDelete,               Icon::ViewBankAccount,          Icon::EditDelete,     Qt::BottomRightCorner},
      {Action::AccountClose,                Icon::ViewBankAccount,          Icon::DialogClose,    Qt::BottomRightCorner},
      {Action::AccountReopen,               Icon::ViewBankAccount,          Icon::DialogOK,       Qt::BottomRightCorner},
      {Action::AccountUpdateMenu,           Icon::ViewBankAccount,          Icon::Download,       Qt::BottomRightCorner},
      {Action::AccountUpdate,               Icon::ViewBankAccount,          Icon::Download,       Qt::BottomRightCorner},
      {Action::AccountUpdateAll,            Icon::ViewBankAccount,          Icon::Download,       Qt::BottomRightCorner},
      {Action::AccountCreditTransfer,       Icon::ViewBankAccount,          Icon::MailMessageNew, Qt::BottomRightCorner},
      {Action::CategoryNew,                 Icon::ViewFinancialCategories,  Icon::ListAdd,        Qt::TopRightCorner},
      {Action::CategoryEdit,                Icon::ViewFinancialCategories,  Icon::DocumentEdit,   Qt::BottomRightCorner},
      {Action::CategoryDelete,              Icon::ViewFinancialCategories,  Icon::EditDelete,     Qt::BottomRightCorner},
      {Action::ToolUpdatePrices,            Icon::ViewInvestment,           Icon::Download,       Qt::BottomRightCorner},
      {Action::TransactionNew,              Icon::ViewFinancialTransfer,    Icon::ListAdd,        Qt::TopRightCorner},
      {Action::TransactionEdit,             Icon::ViewFinancialTransfer,    Icon::DocumentEdit,   Qt::BottomRightCorner},
      {Action::TransactionMatch,            Icon::ViewFinancialTransfer,    Icon::DocumentImport, Qt::BottomRightCorner},
      {Action::TransactionAccept,           Icon::ViewFinancialTransfer,    Icon::DialogOKApply,  Qt::BottomRightCorner},
      {Action::InvestmentNew,               Icon::ViewInvestment,           Icon::ListAdd,        Qt::TopRightCorner},
      {Action::InvestmentEdit,              Icon::ViewInvestment,           Icon::DocumentEdit,   Qt::BottomRightCorner},
      {Action::InvestmentDelete,            Icon::ViewInvestment,           Icon::EditDelete,     Qt::BottomRightCorner},
      {Action::InvestmentOnlinePrice,       Icon::ViewInvestment,           Icon::Download,       Qt::BottomRightCorner},
      {Action::BudgetNew,                   Icon::ViewTimeScheduleCalculus, Icon::ListAdd,        Qt::TopRightCorner},
      {Action::BudgetRename,                Icon::ViewTimeScheduleCalculus, Icon::DocumentEdit,   Qt::BottomRightCorner},
      {Action::BudgetDelete,                Icon::ViewTimeScheduleCalculus, Icon::EditDelete,     Qt::BottomRightCorner},
      {Action::BudgetCopy,                  Icon::ViewTimeScheduleCalculus, Icon::EditCopy,       Qt::BottomRightCorner},
      {Action::PriceUpdate,                 Icon::ViewCurrencyList,         Icon::Download,       Qt::BottomRightCorner}
    };

    for(const auto& it : actionOverlaidIcons)
      lutActions[it.action]->setIcon(KMyMoneyUtils::overlayIcon(g_Icons[it.icon], g_Icons[it.overlay], it.corner));
  }

  // *************
  // Setting keyboard shortcuts for some of added actions
  // *************
  {
    const QVector<QPair<Action, QKeySequence>> actionShortcuts {
      {qMakePair(Action::EditFindTransaction,         Qt::CTRL + Qt::Key_F)},
      {qMakePair(Action::ViewTransactionDetail,       Qt::CTRL + Qt::Key_T)},
      {qMakePair(Action::ViewHideReconciled,          Qt::CTRL + Qt::Key_R)},
      {qMakePair(Action::ViewHideCategories,          Qt::CTRL + Qt::Key_U)},
      {qMakePair(Action::ViewShowAll,                 Qt::CTRL + Qt::SHIFT + Qt::Key_A)},
      {qMakePair(Action::AccountStartReconciliation,  Qt::CTRL + Qt::SHIFT + Qt::Key_R)},
      {qMakePair(Action::TransactionNew,              Qt::CTRL + Qt::Key_Insert)},
      {qMakePair(Action::TransactionToggleReconciled, Qt::CTRL + Qt::Key_Space)},
      {qMakePair(Action::TransactionToggleCleared,    Qt::CTRL + Qt::Key_Alt + Qt::Key_Space)},
      {qMakePair(Action::TransactionReconciled,       Qt::CTRL + Qt::Key_Shift + Qt::Key_Space)},
      {qMakePair(Action::TransactionSelectAll,        Qt::CTRL + Qt::Key_A)},
#ifdef KMM_DEBUG
      {qMakePair(Action::WizardNewUser,               Qt::CTRL + Qt::Key_G)},
#endif
      {qMakePair(Action::TransactionAssignNumber,     Qt::CTRL + Qt::Key_Shift + Qt::Key_N)}
    };

    for(const auto& it : actionShortcuts)
      aC->setDefaultShortcut(lutActions[it.first], it.second);
  }

  // *************
  // Misc settings
  // *************
  connect(onlineJobAdministration::instance(), &onlineJobAdministration::canSendCreditTransferChanged,  lutActions.value(Action::AccountCreditTransfer), &QAction::setEnabled);

  // Setup transaction detail switch
  lutActions[Action::ViewTransactionDetail]->setChecked(KMyMoneyGlobalSettings::showRegisterDetailed());
  lutActions[Action::ViewHideReconciled]->setChecked(KMyMoneyGlobalSettings::hideReconciledTransactions());
  lutActions[Action::ViewHideCategories]->setChecked(KMyMoneyGlobalSettings::hideUnusedCategory());
  lutActions[Action::ViewShowAll]->setChecked(false);

  // *************
  // Adding actions to ActionCollection
  // *************
  actionCollection()->addActions(lutActions.values());

  // ************************
  // Currently unused actions
  // ************************
#if 0
  new KToolBarPopupAction(i18n("View back"), "go-previous", 0, this, SLOT(slotShowPreviousView()), actionCollection(), "go_back");
  new KToolBarPopupAction(i18n("View forward"), "go-next", 0, this, SLOT(slotShowNextView()), actionCollection(), "go_forward");

  action("go_back")->setEnabled(false);
  action("go_forward")->setEnabled(false);
#endif

  // use the absolute path to your kmymoneyui.rc file for testing purpose in createGUI();
  setupGUI();

  // reconnect about app entry to dialog with full credits information
  QAction *aboutApp = aC->action(QString::fromLatin1(KStandardAction::name(KStandardAction::AboutApp)));
  aboutApp->disconnect();
  connect(aboutApp, &QAction::triggered, this, &KMyMoneyApp::slotShowCredits);

  QMenu *menuContainer;
  menuContainer = static_cast<QMenu*>(factory()->container(QStringLiteral("import"), this));
  menuContainer->setIcon(QIcon::fromTheme(g_Icons[Icon::DocumentImport]));

  menuContainer = static_cast<QMenu*>(factory()->container(QStringLiteral("export"), this));
  menuContainer->setIcon(QIcon::fromTheme(g_Icons[Icon::DocumentExport]));
}

void KMyMoneyApp::connectActionsAndViews()
{
  KOnlineJobOutbox *const outbox = d->m_myMoneyView->getOnlineJobOutbox();
  Q_CHECK_PTR(outbox);

  QAction *const onlineJob_delete = actionCollection()->action(s_Actions[Action::OnlineJobDelete]);
  Q_CHECK_PTR(onlineJob_delete);
  connect(onlineJob_delete, SIGNAL(triggered()), outbox, SLOT(slotRemoveJob()));

  QAction *const onlineJob_edit = actionCollection()->action(s_Actions[Action::OnlineJobEdit]);
  Q_CHECK_PTR(onlineJob_edit);
  connect(onlineJob_edit, SIGNAL(triggered()), outbox, SLOT(slotEditJob()));
}

#ifdef KMM_DEBUG
void KMyMoneyApp::dumpActions() const
{
  const QList<QAction*> list = actionCollection()->actions();
  foreach (const auto it, list)
    std::cout << qPrintable(it->objectName()) << ": " << qPrintable(it->text()) << std::endl;
}
#endif

bool KMyMoneyApp::isActionToggled(const Action _a)
{
  return actionCollection()->action(s_Actions[_a])->isChecked();
}

void KMyMoneyApp::initStatusBar()
{
  ///////////////////////////////////////////////////////////////////
  // STATUSBAR

  d->m_statusLabel = new QLabel;
  statusBar()->addWidget(d->m_statusLabel);
  ready();

  // Initialization of progress bar taken from KDevelop ;-)
  d->m_progressBar = new QProgressBar;
  statusBar()->addWidget(d->m_progressBar);
  d->m_progressBar->setFixedHeight(d->m_progressBar->sizeHint().height() - 8);

  // hide the progress bar for now
  slotStatusProgressBar(-1, -1);
}

void KMyMoneyApp::saveOptions()
{
  KConfigGroup grp = d->m_config->group("General Options");
  grp.writeEntry("Geometry", size());

  grp.writeEntry("Show Statusbar", actionCollection()->action(KStandardAction::name(KStandardAction::ShowStatusbar))->isChecked());

  KConfigGroup toolbarGrp = d->m_config->group("mainToolBar");
  toolBar("mainToolBar")->saveSettings(toolbarGrp);

  d->m_recentFiles->saveEntries(d->m_config->group("Recent Files"));

}


void KMyMoneyApp::readOptions()
{
  KConfigGroup grp = d->m_config->group("General Options");


  actionCollection()->action(s_Actions[Action::ViewHideReconciled])->setChecked(KMyMoneyGlobalSettings::hideReconciledTransactions());
  actionCollection()->action(s_Actions[Action::ViewHideCategories])->setChecked(KMyMoneyGlobalSettings::hideUnusedCategory());

  d->m_recentFiles->loadEntries(d->m_config->group("Recent Files"));

  // Startdialog is written in the settings dialog
  d->m_startDialog = grp.readEntry("StartDialog", true);
}

void KMyMoneyApp::resizeEvent(QResizeEvent* ev)
{
  KMainWindow::resizeEvent(ev);
  updateCaption(true);
}

int KMyMoneyApp::askSaveOnClose()
{
  int ans;
  if (KMyMoneyGlobalSettings::autoSaveOnClose()) {
    ans = KMessageBox::Yes;
  } else {
    ans = KMessageBox::warningYesNoCancel(this, i18n("The file has been changed, save it?"));
  }
  return ans;
}

bool KMyMoneyApp::queryClose()
{
  if (!isReady())
    return false;

  if (d->m_myMoneyView->dirty()) {
    int ans = askSaveOnClose();

    if (ans == KMessageBox::Cancel)
      return false;
    else if (ans == KMessageBox::Yes) {
      bool saved = slotFileSave();
      saveOptions();
      return saved;
    }
  }
  if (d->m_myMoneyView->isDatabase())
    slotFileClose(); // close off the database
  saveOptions();
  return true;
}

/////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATION
/////////////////////////////////////////////////////////////////////
void KMyMoneyApp::slotFileInfoDialog()
{
  QPointer<KMyMoneyFileInfoDlg> dlg = new KMyMoneyFileInfoDlg(0);
  dlg->exec();
  delete dlg;
}

void KMyMoneyApp::slotPerformanceTest()
{
  // dump performance report to stderr

  int measurement[2];
  QTime timer;
  MyMoneyAccount acc;

  qDebug("--- Starting performance tests ---");

  // AccountList
  MyMoneyFile::instance()->preloadCache();
  measurement[0] = measurement[1] = 0;
  timer.start();
  for (int i = 0; i < 1000; ++i) {
    QList<MyMoneyAccount> list;

    MyMoneyFile::instance()->accountList(list);
    measurement[i != 0] = timer.elapsed();
  }
  std::cerr << "accountList()" << std::endl;
  std::cerr << "First time: " << measurement[0] << " msec" << std::endl;
  std::cerr << "Total time: " << (measurement[0] + measurement[1]) << " msec" << std::endl;
  std::cerr << "Average   : " << (measurement[0] + measurement[1]) / 1000 << " msec" << std::endl;

  // Balance of asset account(s)
  MyMoneyFile::instance()->preloadCache();
  measurement[0] = measurement[1] = 0;
  acc = MyMoneyFile::instance()->asset();
  for (int i = 0; i < 1000; ++i) {
    timer.start();
    MyMoneyMoney result = MyMoneyFile::instance()->balance(acc.id());
    measurement[i != 0] += timer.elapsed();
  }
  std::cerr << "balance(Asset)" << std::endl;
  std::cerr << "First time: " << measurement[0] << " msec" << std::endl;
  std::cerr << "Average   : " << (measurement[0] + measurement[1]) / 1000 << " msec" << std::endl;

  // total balance of asset account
  MyMoneyFile::instance()->preloadCache();
  measurement[0] = measurement[1] = 0;
  acc = MyMoneyFile::instance()->asset();
  for (int i = 0; i < 1000; ++i) {
    timer.start();
    MyMoneyMoney result = MyMoneyFile::instance()->totalBalance(acc.id());
    measurement[i != 0] += timer.elapsed();
  }
  std::cerr << "totalBalance(Asset)" << std::endl;
  std::cerr << "First time: " << measurement[0] << " msec" << std::endl;
  std::cerr << "Average   : " << (measurement[0] + measurement[1]) / 1000 << " msec" << std::endl;

  // Balance of expense account(s)
  MyMoneyFile::instance()->preloadCache();
  measurement[0] = measurement[1] = 0;
  acc = MyMoneyFile::instance()->expense();
  for (int i = 0; i < 1000; ++i) {
    timer.start();
    MyMoneyMoney result = MyMoneyFile::instance()->balance(acc.id());
    measurement[i != 0] += timer.elapsed();
  }
  std::cerr << "balance(Expense)" << std::endl;
  std::cerr << "First time: " << measurement[0] << " msec" << std::endl;
  std::cerr << "Average   : " << (measurement[0] + measurement[1]) / 1000 << " msec" << std::endl;

  // total balance of expense account
  MyMoneyFile::instance()->preloadCache();
  measurement[0] = measurement[1] = 0;
  acc = MyMoneyFile::instance()->expense();
  timer.start();
  for (int i = 0; i < 1000; ++i) {
    MyMoneyMoney result = MyMoneyFile::instance()->totalBalance(acc.id());
    measurement[i != 0] = timer.elapsed();
  }
  std::cerr << "totalBalance(Expense)" << std::endl;
  std::cerr << "First time: " << measurement[0] << " msec" << std::endl;
  std::cerr << "Total time: " << (measurement[0] + measurement[1]) << " msec" << std::endl;
  std::cerr << "Average   : " << (measurement[0] + measurement[1]) / 1000 << " msec" << std::endl;

  // transaction list
  MyMoneyFile::instance()->preloadCache();
  measurement[0] = measurement[1] = 0;
  if (MyMoneyFile::instance()->asset().accountCount()) {
    MyMoneyTransactionFilter filter(MyMoneyFile::instance()->asset().accountList()[0]);
    filter.setDateFilter(QDate(), QDate::currentDate());
    QList<MyMoneyTransaction> list;

    timer.start();
    for (int i = 0; i < 100; ++i) {
      list = MyMoneyFile::instance()->transactionList(filter);
      measurement[i != 0] = timer.elapsed();
    }
    std::cerr << "transactionList()" << std::endl;
    std::cerr << "First time: " << measurement[0] << " msec" << std::endl;
    std::cerr << "Total time: " << (measurement[0] + measurement[1]) << " msec" << std::endl;
    std::cerr << "Average   : " << (measurement[0] + measurement[1]) / 100 << " msec" << std::endl;
  }

  // transaction list
  MyMoneyFile::instance()->preloadCache();
  measurement[0] = measurement[1] = 0;
  if (MyMoneyFile::instance()->asset().accountCount()) {
    MyMoneyTransactionFilter filter(MyMoneyFile::instance()->asset().accountList()[0]);
    filter.setDateFilter(QDate(), QDate::currentDate());
    QList<MyMoneyTransaction> list;

    timer.start();
    for (int i = 0; i < 100; ++i) {
      MyMoneyFile::instance()->transactionList(list, filter);
      measurement[i != 0] = timer.elapsed();
    }
    std::cerr << "transactionList(list)" << std::endl;
    std::cerr << "First time: " << measurement[0] << " msec" << std::endl;
    std::cerr << "Total time: " << (measurement[0] + measurement[1]) << " msec" << std::endl;
    std::cerr << "Average   : " << (measurement[0] + measurement[1]) / 100 << " msec" << std::endl;
  }
  MyMoneyFile::instance()->preloadCache();
}

void KMyMoneyApp::slotFileNew()
{
  KMSTATUS(i18n("Creating new document..."));

  slotFileClose();

  if (!d->m_myMoneyView->fileOpen()) {
    // next line required until we move all file handling out of KMyMoneyView
    d->m_myMoneyView->newFile();

    d->m_fileName = QUrl();
    updateCaption();

    NewUserWizard::Wizard *wizard = new NewUserWizard::Wizard();

    if (wizard->exec() == QDialog::Accepted) {
      MyMoneyFileTransaction ft;
      MyMoneyFile* file = MyMoneyFile::instance();
      try {
        // store the user info
        file->setUser(wizard->user());

        // create and setup base currency
        file->addCurrency(wizard->baseCurrency());
        file->setBaseCurrency(wizard->baseCurrency());

        // create a possible institution
        MyMoneyInstitution inst = wizard->institution();
        if (inst.name().length()) {
          file->addInstitution(inst);
        }

        // create a possible checking account
        MyMoneyAccount acc = wizard->account();
        if (acc.name().length()) {
          acc.setInstitutionId(inst.id());
          MyMoneyAccount asset = file->asset();
          file->addAccount(acc, asset);

          // create possible opening balance transaction
          if (!wizard->openingBalance().isZero()) {
            file->createOpeningBalanceTransaction(acc, wizard->openingBalance());
          }
        }

        // import the account templates
        QList<MyMoneyTemplate> templates = wizard->templates();
        QList<MyMoneyTemplate>::iterator it_t;
        for (it_t = templates.begin(); it_t != templates.end(); ++it_t) {
          (*it_t).importTemplate(&progressCallback);
        }

        d->m_fileName = wizard->url();
        ft.commit();
        KMyMoneyGlobalSettings::setFirstTimeRun(false);

        // FIXME This is a bit clumsy. We re-read the freshly
        // created file to be able to run through all the
        // fixup logic and then save it to keep the modified
        // flag off.
        slotFileSave();
        d->m_myMoneyView->readFile(d->m_fileName);
        slotFileSave();

        // now keep the filename in the recent files used list
        //KRecentFilesAction *p = dynamic_cast<KRecentFilesAction*>(action(KStandardAction::name(KStandardAction::OpenRecent)));
        //if(p)
        d->m_recentFiles->addUrl(d->m_fileName);
        writeLastUsedFile(d->m_fileName.url());

      } catch (const MyMoneyException &) {
        // next line required until we move all file handling out of KMyMoneyView
        d->m_myMoneyView->closeFile();
      }
      if (wizard->startSettingsAfterFinished())
        slotSettings();
    } else {
      // next line required until we move all file handling out of KMyMoneyView
      d->m_myMoneyView->closeFile();
    }
    delete wizard;
    updateCaption();

    emit fileLoaded(d->m_fileName);
  }
}

QUrl KMyMoneyApp::selectFile(const QString& title, const QString& _path, const QString& mask, QFileDialog::FileMode mode, QWidget* widget)
{
  QString path(_path);

  // if the path is not specified open the file dialog in the last used directory
  // 'kmymoney' is the keyword that identifies the last used directory in KFileDialog
  if (path.isEmpty()) {
    path = KRecentDirs::dir(":kmymoney-import");
  }

  QPointer<QFileDialog> dialog = new QFileDialog(this, title, path, mask);
  dialog->setFileMode(mode);

  QUrl url;
  if (dialog->exec() == QDialog::Accepted && dialog != 0) {
    QList<QUrl> selectedUrls = dialog->selectedUrls();
    if (!selectedUrls.isEmpty()) {
      url = selectedUrls.first();
      if (_path.isEmpty()) {
        KRecentDirs::add(":kmymoney-import", url.adjusted(QUrl::RemoveFilename | QUrl::StripTrailingSlash).path());
      }
    }
  }

  // in case we have an additional widget, we remove it from the
  // dialog, so that the caller can still access it. Therefore, it is
  // the callers responsibility to delete the object

  if (widget)
    widget->setParent(0);

  delete dialog;

  return url;
}

// General open
void KMyMoneyApp::slotFileOpen()
{
  KMSTATUS(i18n("Open a file."));

  QString prevDir = readLastUsedDir();
  QPointer<QFileDialog> dialog = new QFileDialog(this, QString(), prevDir,
                                                 i18n("KMyMoney files (*.kmy *.xml);;All files"));
  dialog->setFileMode(QFileDialog::ExistingFile);
  dialog->setAcceptMode(QFileDialog::AcceptOpen);

  if (dialog->exec() == QDialog::Accepted && dialog != nullptr) {
    slotFileOpenRecent(dialog->selectedUrls().first());
  }
  delete dialog;
}

void KMyMoneyApp::slotOpenDatabase()
{
  KMSTATUS(i18n("Open a file."));
  QPointer<KSelectDatabaseDlg> dialog = new KSelectDatabaseDlg(QIODevice::ReadWrite);
  if (!dialog->checkDrivers()) {
    delete dialog;
    return;
  }

  if (dialog->exec() == QDialog::Accepted && dialog != 0) {
    slotFileOpenRecent(dialog->selectedURL());
  }
  delete dialog;
}

bool KMyMoneyApp::isImportableFile(const QUrl &url)
{
  bool result = false;

  // Iterate through the plugins and see if there's a loaded plugin who can handle it
  QMap<QString, KMyMoneyPlugin::ImporterPlugin*>::const_iterator it_plugin = d->m_importerPlugins.constBegin();
  while (it_plugin != d->m_importerPlugins.constEnd()) {
    if ((*it_plugin)->isMyFormat(url.path())) {
      result = true;
      break;
    }
    ++it_plugin;
  }

  // If we did not find a match, try importing it as a KMM statement file,
  // which is really just for testing.  the statement file is not exposed
  // to users.
  if (it_plugin == d->m_importerPlugins.constEnd())
    if (MyMoneyStatement::isStatementFile(url.path()))
      result = true;

  // Place code here to test for QIF and other locally-supported formats
  // (i.e. not a plugin). If you add them here, be sure to add it to
  // the webConnect function.

  return result;
}

void KMyMoneyApp::slotFileOpenRecent(const QUrl &url)
{
  KMSTATUS(i18n("Loading file..."));
  QUrl lastFile = d->m_fileName;

  // check if there are other instances which might have this file open
  QList<QString> list = instanceList();
  QList<QString>::ConstIterator it;
  bool duplicate = false;
#ifdef KMM_DBUS
  for (it = list.constBegin(); duplicate == false && it != list.constEnd(); ++it) {
    QDBusInterface remoteApp(*it, "/KMymoney", "org.kde.kmymoney");
    QDBusReply<QString> reply = remoteApp.call("filename");
    if (!reply.isValid()) {
      qDebug("D-Bus error while calling app->filename()");
    } else {
      if (reply.value() == url.url()) {
        duplicate = true;
      }
    }
  }
#endif
  if (!duplicate) {
    QUrl newurl = url;
    if ((newurl.scheme() == QLatin1String("sql"))) {
      const QString key = QLatin1String("driver");
      // take care and convert some old url to their new counterpart
      QUrlQuery query(newurl);
      if (query.queryItemValue(key) == QLatin1String("QMYSQL3")) { // fix any old urls
        query.removeQueryItem(key);
        query.addQueryItem(key, QLatin1String("QMYSQL"));
      }
      if (query.queryItemValue(key) == QLatin1String("QSQLITE3")) {
        query.removeQueryItem(key);
        query.addQueryItem(key, QLatin1String("QSQLITE"));
      }
      newurl.setQuery(query);

      if (query.queryItemValue(key) == QLatin1String("QSQLITE")) {
        newurl.setUserInfo(QString());
        newurl.setHost(QString());
      }
      // check if a password is needed. it may be if the URL came from the last/recent file list
      QPointer<KSelectDatabaseDlg> dialog = new KSelectDatabaseDlg(QIODevice::ReadWrite, newurl);
      if (!dialog->checkDrivers()) {
        delete dialog;
        return;
      }
      // if we need to supply a password, then show the dialog
      // otherwise it isn't needed
      if ((query.queryItemValue("secure").toLower() == QLatin1String("yes")) && newurl.password().isEmpty()) {
        if (dialog->exec() == QDialog::Accepted && dialog != nullptr) {
          newurl = dialog->selectedURL();
        } else {
          delete dialog;
          return;
        }
      }
      delete dialog;
    }
    // TODO: port KF5 (NetAccess)
    if ((newurl.scheme() == QLatin1String("sql")) || (newurl.isValid() /*&& KIO::NetAccess::exists(newurl, KIO::NetAccess::SourceSide, this)*/)) {
      slotFileClose();
      if (!d->m_myMoneyView->fileOpen()) {
        try {
          if (d->m_myMoneyView->readFile(newurl)) {
            if ((d->m_myMoneyView->isNativeFile())) {
              d->m_fileName = newurl;
              updateCaption();
              d->m_recentFiles->addUrl(newurl);
              writeLastUsedFile(newurl.toDisplayString(QUrl::PreferLocalFile));
            } else {
              d->m_fileName = QUrl(); // imported files have no filename
            }
            // Check the schedules
            slotCheckSchedules();
          }
        } catch (const MyMoneyException &e) {
          KMessageBox::sorry(this, i18n("Cannot open file as requested. Error was: %1", e.what()));
        }
        updateCaption();
        emit fileLoaded(d->m_fileName);
      } else {
        /*fileOpen failed - should we do something
         or maybe fileOpen puts out the message... - it does for database*/
      }
    } else { // newurl invalid
      slotFileClose();
      KMessageBox::sorry(this, i18n("<p><b>%1</b> is either an invalid filename or the file does not exist. You can open another file or create a new one.</p>", url.toDisplayString(QUrl::PreferLocalFile)), i18n("File not found"));
    }
  } else { // isDuplicate
    KMessageBox::sorry(this, i18n("<p>File <b>%1</b> is already opened in another instance of KMyMoney</p>", url.toDisplayString(QUrl::PreferLocalFile)), i18n("Duplicate open"));
  }
}

bool KMyMoneyApp::slotFileSave()
{
  // if there's nothing changed, there's no need to save anything
  if (!d->m_myMoneyView->dirty())
    return true;

  bool rc = false;

  KMSTATUS(i18n("Saving file..."));

  if (d->m_fileName.isEmpty())
    return slotFileSaveAs();

  d->consistencyCheck(false);

  /*if (myMoneyView->isDatabase()) {
    rc = myMoneyView->saveDatabase(m_fileName);
    // the 'save' function is no longer relevant for a database*/
  setEnabled(false);
  rc = d->m_myMoneyView->saveFile(d->m_fileName, MyMoneyFile::instance()->value("kmm-encryption-key"));
  setEnabled(true);

  d->m_autoSaveTimer->stop();

  updateCaption();
  return rc;
}

void KMyMoneyApp::slotFileSaveAsFilterChanged(const QString& filter)
{
  if (!d->m_saveEncrypted)
    return;
  if (filter.compare(QLatin1String("*.kmy"), Qt::CaseInsensitive) != 0) {
    d->m_saveEncrypted->setCurrentItem(0);
    d->m_saveEncrypted->setEnabled(false);
  } else {
    d->m_saveEncrypted->setEnabled(true);
  }
}

void KMyMoneyApp::slotManageGpgKeys()
{
  QPointer<KGpgKeySelectionDlg> dlg = new KGpgKeySelectionDlg(this);
  dlg->setKeys(d->m_additionalGpgKeys);
  if (dlg->exec() == QDialog::Accepted && dlg != 0) {
    d->m_additionalGpgKeys = dlg->keys();
    d->m_additionalKeyLabel->setText(i18n("Additional encryption keys to be used: %1", d->m_additionalGpgKeys.count()));
  }
  delete dlg;
}

void KMyMoneyApp::slotKeySelected(int idx)
{
  int cnt = 0;
  if (idx != 0) {
    cnt = d->m_additionalGpgKeys.count();
  }
  d->m_additionalKeyLabel->setEnabled(idx != 0);
  d->m_additionalKeyButton->setEnabled(idx != 0);
  d->m_additionalKeyLabel->setText(i18n("Additional encryption keys to be used: %1", cnt));
}

bool KMyMoneyApp::slotFileSaveAs()
{
  bool rc = false;
  // in event of it being a database, ensure that all data is read into storage for saveas
  if (d->m_myMoneyView->isDatabase())
    dynamic_cast<IMyMoneySerialize*>(MyMoneyFile::instance()->storage())->fillStorage();
  KMSTATUS(i18n("Saving file with a new filename..."));

  // fill the additional key list with the default
  d->m_additionalGpgKeys = KMyMoneyGlobalSettings::gpgRecipientList();

  QWidget* vbox = new QWidget();
  QVBoxLayout *vboxVBoxLayout = new QVBoxLayout(vbox);
  vboxVBoxLayout->setMargin(0);
  if (KGPGFile::GPGAvailable()) {
    QWidget* keyBox = new QWidget(vbox);
    QHBoxLayout *keyBoxHBoxLayout = new QHBoxLayout(keyBox);
    keyBoxHBoxLayout->setMargin(0);
    vboxVBoxLayout->addWidget(keyBox);
    QLabel *keyLabel = new QLabel(i18n("Encryption key to be used"), keyBox);
    keyBoxHBoxLayout->addWidget(keyLabel);
    d->m_saveEncrypted = new KComboBox(keyBox);
    keyBoxHBoxLayout->addWidget(d->m_saveEncrypted);

    QWidget* labelBox = new QWidget(vbox);
    QHBoxLayout *labelBoxHBoxLayout = new QHBoxLayout(labelBox);
    labelBoxHBoxLayout->setMargin(0);
    vboxVBoxLayout->addWidget(labelBox);
    d->m_additionalKeyLabel = new QLabel(i18n("Additional encryption keys to be used: %1", d->m_additionalGpgKeys.count()), labelBox);
    labelBoxHBoxLayout->addWidget(d->m_additionalKeyLabel);
    d->m_additionalKeyButton = new QPushButton(i18n("Manage additional keys"), labelBox);
    labelBoxHBoxLayout->addWidget(d->m_additionalKeyButton);
    connect(d->m_additionalKeyButton, SIGNAL(clicked()), this, SLOT(slotManageGpgKeys()));
    connect(d->m_saveEncrypted, SIGNAL(activated(int)), this, SLOT(slotKeySelected(int)));

    // fill the secret key list and combo box
    QStringList keyList;
    KGPGFile::secretKeyList(keyList);
    d->m_saveEncrypted->addItem(i18n("No encryption"));

    for (QStringList::iterator it = keyList.begin(); it != keyList.end(); ++it) {
      QStringList fields = (*it).split(':', QString::SkipEmptyParts);
      if (fields[0] != recoveryKeyId) {
        // replace parenthesis in name field with brackets
        QString name = fields[1];
        name.replace('(', "[");
        name.replace(')', "]");
        name = QString("%1 (0x%2)").arg(name).arg(fields[0]);
        d->m_saveEncrypted->addItem(name);
        if (name.contains(KMyMoneyGlobalSettings::gpgRecipient())) {
          d->m_saveEncrypted->setCurrentItem(name);
        }
      }
    }
  }

  QString prevDir; // don't prompt file name if not a native file
  if (d->m_myMoneyView->isNativeFile())
    prevDir = readLastUsedDir();

  QPointer<QFileDialog> dlg =
    new QFileDialog(this, i18n("Save As"), prevDir,
                    QString(QLatin1String("%2 (%1);;")).arg(QStringLiteral("*.kmy")).arg(i18nc("KMyMoney (Filefilter)", "KMyMoney files")) +
                    QString(QLatin1String("%2 (%1);;")).arg(QStringLiteral("*.xml")).arg(i18nc("XML (Filefilter)", "XML files")) +
                    QString(QLatin1String("%2 (%1);;")).arg(QStringLiteral("*.anon.xml")).arg(i18nc("Anonymous (Filefilter)", "Anonymous files")) +
                    QString(QLatin1String("%2 (%1);;")).arg(QStringLiteral("*")).arg(i18nc("All files (Filefilter)", "All files")));
  dlg->setAcceptMode(QFileDialog::AcceptSave);
  connect(dlg, SIGNAL(filterChanged(QString)), this, SLOT(slotFileSaveAsFilterChanged(QString)));

  if (dlg->exec() == QDialog::Accepted && dlg != 0) {
    QUrl newURL = dlg->selectedUrls().first();
    if (!newURL.fileName().isEmpty()) {
      d->consistencyCheck(false);
      // deleting the dialog will delete the combobox pointed to by d->m_saveEncrypted so get the key name here
      QString selectedKeyName;
      if (d->m_saveEncrypted && d->m_saveEncrypted->currentIndex() != 0)
        selectedKeyName = d->m_saveEncrypted->currentText();

      d->m_saveEncrypted = 0;

      QString newName = newURL.toDisplayString(QUrl::PreferLocalFile);

      // append extension if not present
      if (!newName.endsWith(QLatin1String(".kmy"), Qt::CaseInsensitive) &&
          !newName.endsWith(QLatin1String(".xml"), Qt::CaseInsensitive))
        newName.append(QLatin1String(".kmy"));
      newURL = QUrl::fromUserInput(newName);
      d->m_recentFiles->addUrl(newURL);

      setEnabled(false);
      // If this is the anonymous file export, just save it, don't actually take the
      // name, or remember it! Don't even try to encrypt it
      if (newName.endsWith(QLatin1String(".anon.xml"), Qt::CaseInsensitive))
        rc = d->m_myMoneyView->saveFile(newURL);
      else {
        d->m_fileName = newURL;
        QString encryptionKeys;
        QRegExp keyExp(".* \\((.*)\\)");
        if (keyExp.indexIn(selectedKeyName) != -1) {
          encryptionKeys = keyExp.cap(1);
        }
        if (!d->m_additionalGpgKeys.isEmpty()) {
          if (!encryptionKeys.isEmpty())
            encryptionKeys.append(QLatin1Char(','));
          encryptionKeys.append(d->m_additionalGpgKeys.join(QLatin1Char(',')));
        }
        rc = d->m_myMoneyView->saveFile(d->m_fileName, encryptionKeys);
        //write the directory used for this file as the default one for next time.
        writeLastUsedDir(newURL.toDisplayString(QUrl::RemoveFilename | QUrl::PreferLocalFile | QUrl::StripTrailingSlash));
        writeLastUsedFile(newName);
      }
      d->m_autoSaveTimer->stop();
      setEnabled(true);
    }
  }

  delete dlg;
  updateCaption();
  return rc;
}

void KMyMoneyApp::slotSaveAsDatabase()
{
  saveAsDatabase();
}

bool KMyMoneyApp::saveAsDatabase()
{
  bool rc = false;
  QUrl oldUrl;
  // in event of it being a database, ensure that all data is read into storage for saveas
  if (d->m_myMoneyView->isDatabase()) {
    dynamic_cast<IMyMoneySerialize*>(MyMoneyFile::instance()->storage())->fillStorage();
    oldUrl = d->m_fileName.isEmpty() ? lastOpenedURL() : d->m_fileName;
  }
  KMSTATUS(i18n("Saving file to database..."));
  QPointer<KSelectDatabaseDlg> dialog = new KSelectDatabaseDlg(QIODevice::WriteOnly);
  QUrl url = oldUrl;
  if (!dialog->checkDrivers()) {
    delete dialog;
    return (false);
  }

  while (oldUrl == url && dialog->exec() == QDialog::Accepted && dialog != 0) {
    url = dialog->selectedURL();
    // If the protocol is SQL for the old and new, and the hostname and database names match
    // Let the user know that the current database cannot be saved on top of itself.
    if (url.scheme() == "sql" && oldUrl.scheme() == "sql"
        && oldUrl.host() == url.host()
        && QUrlQuery(oldUrl).queryItemValue("driver") == QUrlQuery(url).queryItemValue("driver")
        && oldUrl.path().right(oldUrl.path().length() - 1) == url.path().right(url.path().length() - 1)) {
      KMessageBox::sorry(this, i18n("Cannot save to current database."));
    } else {
      try {
        rc = d->m_myMoneyView->saveAsDatabase(url);
      } catch (const MyMoneyException &e) {
        KMessageBox::sorry(this, i18n("Cannot save to current database: %1", e.what()));
      }
    }
  }
  delete dialog;

  if (rc) {
    //KRecentFilesAction *p = dynamic_cast<KRecentFilesAction*>(action("file_open_recent"));
    //if(p)
    d->m_recentFiles->addUrl(url);
    writeLastUsedFile(url.toDisplayString(QUrl::PreferLocalFile));
  }
  d->m_autoSaveTimer->stop();
  updateCaption();
  return rc;
}

void KMyMoneyApp::slotFileCloseWindow()
{
  KMSTATUS(i18n("Closing window..."));

  if (d->m_myMoneyView->dirty()) {
    int answer = askSaveOnClose();
    if (answer == KMessageBox::Cancel)
      return;
    else if (answer == KMessageBox::Yes)
      slotFileSave();
  }
  close();
}

void KMyMoneyApp::slotFileClose()
{
  bool okToSelect = true;

  // check if transaction editor is open and ask user what he wants to do
  slotTransactionsCancelOrEnter(okToSelect);

  if (!okToSelect)
    return;

  // no update status here, as we might delete the status too early.
  if (d->m_myMoneyView->dirty()) {
    int answer = askSaveOnClose();
    if (answer == KMessageBox::Cancel)
      return;
    else if (answer == KMessageBox::Yes)
      slotFileSave();
  }

  d->closeFile();
}

void KMyMoneyApp::slotFileQuit()
{
  // don't modify the status message here as this will prevent quit from working!!
  // See the beginning of queryClose() and isReady() why. Thomas Baumgart 2005-10-17

  bool quitApplication = true;

  QList<KMainWindow*> memberList = KMainWindow::memberList();
  if (!memberList.isEmpty()) {

    QList<KMainWindow*>::const_iterator w_it = memberList.constBegin();
    for (; w_it != memberList.constEnd(); ++w_it) {
      // only close the window if the closeEvent is accepted. If the user presses Cancel on the saveModified() dialog,
      // the window and the application stay open.
      if (!(*w_it)->close()) {
        quitApplication = false;
        break;
      }
    }
  }

  // We will only quit if all windows were processed and not cancelled
  if (quitApplication) {
    QCoreApplication::quit();
  }
}

void KMyMoneyApp::slotShowTransactionDetail()
{

}

void KMyMoneyApp::slotHideReconciledTransactions()
{
  KMyMoneyGlobalSettings::setHideReconciledTransactions(actionCollection()->action(s_Actions[Action::ViewHideReconciled])->isChecked());
  d->m_myMoneyView->slotRefreshViews();
}

void KMyMoneyApp::slotHideUnusedCategories()
{
  KMyMoneyGlobalSettings::setHideUnusedCategory(actionCollection()->action(s_Actions[Action::ViewHideCategories])->isChecked());
  d->m_myMoneyView->slotRefreshViews();
}

void KMyMoneyApp::slotShowAllAccounts()
{
  d->m_myMoneyView->slotRefreshViews();
}

#ifdef KMM_DEBUG
void KMyMoneyApp::slotToggleTraces()
{
  MyMoneyTracer::onOff(actionCollection()->action(s_Actions[Action::DebugTraces])->isChecked() ? 1 : 0);
}
#endif

void KMyMoneyApp::slotToggleTimers()
{
  extern bool timersOn; // main.cpp

  timersOn = actionCollection()->action(s_Actions[Action::DebugTimers])->isChecked();
}

QString KMyMoneyApp::slotStatusMsg(const QString &text)
{
  ///////////////////////////////////////////////////////////////////
  // change status message permanently
  QString previousMessage = d->m_statusLabel->text();
  d->m_applicationIsReady = false;

  QString currentMessage = text;
  if (currentMessage.isEmpty() || currentMessage == i18nc("Application is ready to use", "Ready.")) {
    d->m_applicationIsReady = true;
    currentMessage = i18nc("Application is ready to use", "Ready.");
  }
  statusBar()->clearMessage();
  d->m_statusLabel->setText(currentMessage);
  return previousMessage;
}

void KMyMoneyApp::ready()
{
  slotStatusMsg(QString());
}

bool KMyMoneyApp::isReady()
{
  return d->m_applicationIsReady;
}

void KMyMoneyApp::slotStatusProgressBar(int current, int total)
{
  if (total == -1 && current == -1) {     // reset
    if (d->m_progressTimer) {
      d->m_progressTimer->start(500);     // remove from screen in 500 msec
      d->m_progressBar->setValue(d->m_progressBar->maximum());
    }

  } else if (total != 0) {                // init
    d->m_progressTimer->stop();
    d->m_progressBar->setMaximum(total);
    d->m_progressBar->setValue(0);
    d->m_progressBar->show();

  } else {                                // update
    QTime currentTime = QTime::currentTime();
    // only process painting if last update is at least 250 ms ago
    if (abs(d->m_lastUpdate.msecsTo(currentTime)) > 250) {
      d->m_progressBar->setValue(current);
      d->m_lastUpdate = currentTime;
    }
  }
}

void KMyMoneyApp::slotStatusProgressDone()
{
  d->m_progressTimer->stop();
  d->m_progressBar->reset();
  d->m_progressBar->hide();
  d->m_progressBar->setValue(0);
}

void KMyMoneyApp::progressCallback(int current, int total, const QString& msg)
{
  if (!msg.isEmpty())
    kmymoney->slotStatusMsg(msg);

  kmymoney->slotStatusProgressBar(current, total);
}

void KMyMoneyApp::slotFileViewPersonal()
{
  if (!d->m_myMoneyView->fileOpen()) {
    KMessageBox::information(this, i18n("No KMyMoneyFile open"));
    return;
  }

  KMSTATUS(i18n("Viewing personal data..."));

  MyMoneyFile* file = MyMoneyFile::instance();
  MyMoneyPayee user = file->user();

  QPointer<EditPersonalDataDlg> editPersonalDataDlg = new EditPersonalDataDlg(user.name(), user.address(),
      user.city(), user.state(), user.postcode(), user.telephone(),
      user.email(), this, i18n("Edit Personal Data"));

  if (editPersonalDataDlg->exec() == QDialog::Accepted && editPersonalDataDlg != 0) {
    user.setName(editPersonalDataDlg->userNameText);
    user.setAddress(editPersonalDataDlg->userStreetText);
    user.setCity(editPersonalDataDlg->userTownText);
    user.setState(editPersonalDataDlg->userCountyText);
    user.setPostcode(editPersonalDataDlg->userPostcodeText);
    user.setTelephone(editPersonalDataDlg->userTelephoneText);
    user.setEmail(editPersonalDataDlg->userEmailText);
    MyMoneyFileTransaction ft;
    try {
      file->setUser(user);
      ft.commit();
    } catch (const MyMoneyException &e) {
      KMessageBox::information(this, i18n("Unable to store user information: %1", e.what()));
    }
  }
  delete editPersonalDataDlg;
}

void KMyMoneyApp::slotFileFileInfo()
{
  if (!d->m_myMoneyView->fileOpen()) {
    KMessageBox::information(this, i18n("No KMyMoneyFile open"));
    return;
  }

  QFile g("kmymoney.dump");
  g.open(QIODevice::WriteOnly);
  QDataStream st(&g);
  MyMoneyStorageDump dumper;
  dumper.writeStream(st, dynamic_cast<IMyMoneySerialize*>(MyMoneyFile::instance()->storage()));
  g.close();
}

void KMyMoneyApp::slotLoadAccountTemplates()
{
  KMSTATUS(i18n("Importing account templates."));

  int rc;
  QPointer<KLoadTemplateDlg> dlg = new KLoadTemplateDlg();
  if ((rc = dlg->exec()) == QDialog::Accepted && dlg != 0) {
    MyMoneyFileTransaction ft;
    try {
      // import the account templates
      QList<MyMoneyTemplate> templates = dlg->templates();
      QList<MyMoneyTemplate>::iterator it_t;
      for (it_t = templates.begin(); it_t != templates.end(); ++it_t) {
        (*it_t).importTemplate(&progressCallback);
      }
      ft.commit();
    } catch (const MyMoneyException &e) {
      KMessageBox::detailedSorry(0, i18n("Error"), i18n("Unable to import template(s): %1, thrown in %2:%3", e.what(), e.file(), e.line()));
    }
  }
  delete dlg;
}

void KMyMoneyApp::slotSaveAccountTemplates()
{
  KMSTATUS(i18n("Exporting account templates."));

  QString savePath = QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/templates/" + QLocale().name();
  QDir d(savePath);
  if (!d.exists())
      d.mkpath(savePath);
  QString newName = QFileDialog::getSaveFileName(this, i18n("Save as..."), savePath,
                    i18n("KMyMoney template files (*.kmt);;All files (*)"));

  //
  // If there is no file extension, then append a .kmt at the end of the file name.
  // If there is a file extension, make sure it is .kmt, delete any others.
  //
  if (!newName.isEmpty()) {
    // find last . delimiter
    int nLoc = newName.lastIndexOf('.');
    if (nLoc != -1) {
      QString strExt, strTemp;
      strTemp = newName.left(nLoc + 1);
      strExt = newName.right(newName.length() - (nLoc + 1));
      if ((strExt.indexOf("kmt", 0, Qt::CaseInsensitive) == -1)) {
        strTemp.append("kmt");
        //append to make complete file name
        newName = strTemp;
      }
    } else {
      newName.append(".kmt");
    }

    if (okToWriteFile(QUrl::fromLocalFile(newName))) {
      QPointer <KTemplateExportDlg> dlg = new KTemplateExportDlg(this);
      if (dlg->exec() == QDialog::Accepted && dlg) {
          MyMoneyTemplate templ;
          templ.setTitle(dlg->title());
          templ.setShortDescription(dlg->shortDescription());
          templ.setLongDescription(dlg->longDescription());
          templ.exportTemplate(&progressCallback);
          templ.saveTemplate(QUrl::fromLocalFile(newName));
      }
      delete dlg;
    }
  }
}

void KMyMoneyApp::slotGncImport()
{
  if (d->m_myMoneyView->fileOpen()) {
    switch (KMessageBox::questionYesNoCancel(0,
            i18n("You cannot import GnuCash data into an existing file. Do you wish to save this file?"), PACKAGE)) {
      case KMessageBox::Yes:
        slotFileSave();
        break;
      case KMessageBox::No:
        d->closeFile();
        break;
      default:
        return;
    }
  }

  KMSTATUS(i18n("Importing a GnuCash file."));

  QUrl fileToRead = QFileDialog::getOpenFileUrl(this, QString(), QUrl(), i18n("GnuCash files (*.gnucash *.xac *.gnc);;All files (*)"));

  if (!fileToRead.isEmpty()) {
    // call the importer
    if (d->m_myMoneyView->readFile(fileToRead)) {
      // imported files don't have a name
      d->m_fileName = QUrl();

      updateCaption();
      emit fileLoaded(d->m_fileName);
    }
  }
}

void KMyMoneyApp::slotAccountChart()
{
  if (!d->m_selectedAccount.id().isEmpty()) {
    QPointer<KBalanceChartDlg> dlg = new KBalanceChartDlg(d->m_selectedAccount, this);
    dlg->exec();
    delete dlg;
  }
}


//
// KMyMoneyApp::slotStatementImport() is for testing only.  The MyMoneyStatement
// is not intended to be exposed to users in XML form.
//

void KMyMoneyApp::slotStatementImport()
{
  bool result = false;
  KMSTATUS(i18n("Importing an XML Statement."));

  QList<QUrl> files{QFileDialog::getOpenFileUrls(this, QString(), QUrl(),
      i18n("XML files (*.xml);;All files (*)"))};

  if (!files.isEmpty()) {
    d->m_collectingStatements = (files.count() > 1);

    foreach (const QUrl &url, files) {
      qDebug("Processing '%s'", qPrintable(url.path()));
      result |= slotStatementImport(url.path());
    }
    /*    QFile f( dialog->selectedURL().path() );
        f.open( QIODevice::ReadOnly );
        QString error = "Unable to parse file";
        QDomDocument* doc = new QDomDocument;
        if(doc->setContent(&f, FALSE))
        {
          if ( doc->doctype().name() == "KMYMONEY-STATEMENT" )
          {
            QDomElement rootElement = doc->documentElement();
            if(!rootElement.isNull())
            {
              QDomNode child = rootElement.firstChild();
              if(!child.isNull() && child.isElement())
              {
                MyMoneyStatement s;
                if ( s.read(child.toElement()) )
                  result = slotStatementImport(s);
                else
                  error = "File does not contain any statements";
              }
            }
          }
          else
            error = "File is not a KMyMoney Statement";
        }
        delete doc;

        if ( !result )
        {
          QMessageBox::critical( this, i18n("Critical Error"), i18n("Unable to read file %1: %2").arg( dialog->selectedURL().path()).arg(error), QMessageBox::Ok, 0 );

        }*/
  }

  if (!result) {
    // re-enable all standard widgets
    setEnabled(true);
  }
}

bool KMyMoneyApp::slotStatementImport(const QString& url)
{
  bool result = false;
  MyMoneyStatement s;
  if (MyMoneyStatement::readXMLFile(s, url))
    result = slotStatementImport(s);
  else
    KMessageBox::error(this, i18n("Error importing %1: This file is not a valid KMM statement file.", url), i18n("Invalid Statement"));

  return result;
}

bool KMyMoneyApp::slotStatementImport(const MyMoneyStatement& s, bool silent)
{
  bool result = false;

  // keep a copy of the statement
  if (KMyMoneySettings::logImportedStatements()) {
    QString logFile = QString("%1/kmm-statement-%2.txt").arg(KMyMoneySettings::logPath()).arg(d->m_statementXMLindex++);
    MyMoneyStatement::writeXMLFile(s, logFile);
 }

  // we use an object on the heap here, so that we can check the presence
  // of it during slotUpdateActions() by looking at the pointer.
  d->m_smtReader = new MyMoneyStatementReader;
  d->m_smtReader->setAutoCreatePayee(true);
  d->m_smtReader->setProgressCallback(&progressCallback);

  // disable all standard widgets during the import
  setEnabled(false);

  QStringList messages;
  result = d->m_smtReader->import(s, messages);

  bool transactionAdded = d->m_smtReader->anyTransactionAdded();

  // get rid of the statement reader and tell everyone else
  // about the destruction by setting the pointer to zero
  delete d->m_smtReader;
  d->m_smtReader = 0;

  slotStatusProgressBar(-1, -1);
  ready();

  // re-enable all standard widgets
  setEnabled(true);

  if (!d->m_collectingStatements && !silent)
    KMessageBox::informationList(this, i18n("The statement has been processed with the following results:"), messages, i18n("Statement stats"));
  else if (transactionAdded)
    d->m_statementResults += messages;

  slotUpdateActions();//  Re-enable menu items after import via plugin.
  return result;
}

bool KMyMoneyApp::okToWriteFile(const QUrl &url)
{
  Q_UNUSED(url)

  // check if the file exists and warn the user
  bool reallySaveFile = true;

  // TODO: port KF5 (NetAccess)
  //if (KIO::NetAccess::exists(url, KIO::NetAccess::SourceSide, this)) {
  //  if (KMessageBox::warningYesNo(this, QLatin1String("<qt>") + i18n("The file <b>%1</b> already exists. Do you really want to overwrite it?", url.toDisplayString(QUrl::PreferLocalFile)) + QLatin1String("</qt>"), i18n("File already exists")) != KMessageBox::Yes)
  //    reallySaveFile = false;
  //}
  return reallySaveFile;
}

void KMyMoneyApp::slotSettings()
{
  // if we already have an instance of the settings dialog, then use it
  if (KConfigDialog::showDialog("KMyMoney-Settings"))
    return;

  // otherwise, we have to create it
  KConfigDialog* dlg = new KSettingsKMyMoney(this, "KMyMoney-Settings", KMyMoneyGlobalSettings::self());
  connect(dlg, &KConfigDialog::settingsChanged, this, &KMyMoneyApp::slotUpdateConfiguration);
  dlg->show();
}

void KMyMoneyApp::slotShowCredits()
{
  KAboutData aboutData = initializeCreditsData();
  KAboutApplicationDialog dlg(aboutData, this);
  dlg.exec();
}

void KMyMoneyApp::slotUpdateConfiguration()
{
  MyMoneyTransactionFilter::setFiscalYearStart(KMyMoneyGlobalSettings::firstFiscalMonth(), KMyMoneyGlobalSettings::firstFiscalDay());
  LedgerSeperator::setFirstFiscalDate(KMyMoneyGlobalSettings::firstFiscalMonth(), KMyMoneyGlobalSettings::firstFiscalDay());

  d->m_myMoneyView->updateViewType();

  // update the sql storage module settings
  MyMoneyStorageSql::setStartDate(KMyMoneyGlobalSettings::startDate().date());

  // update the report module settings
  MyMoneyReport::setLineWidth(KMyMoneyGlobalSettings::lineWidth());

  // update the holiday region configuration
  setHolidayRegion(KMyMoneyGlobalSettings::holidayRegion());

  d->m_myMoneyView->slotRefreshViews();

  // re-read autosave configuration
  d->m_autoSaveEnabled = KMyMoneyGlobalSettings::autoSaveFile();
  d->m_autoSavePeriod = KMyMoneyGlobalSettings::autoSavePeriod();

  // stop timer if turned off but running
  if (d->m_autoSaveTimer->isActive() && !d->m_autoSaveEnabled) {
    d->m_autoSaveTimer->stop();
  }
  // start timer if turned on and needed but not running
  if (!d->m_autoSaveTimer->isActive() && d->m_autoSaveEnabled && d->m_myMoneyView->dirty()) {
    d->m_autoSaveTimer->setSingleShot(true);
    d->m_autoSaveTimer->start(d->m_autoSavePeriod * 60 * 1000);
  }

  d->setCustomColors();

  // check if the recovery key is still valid or expires soon

  if (KMyMoneySettings::writeDataEncrypted() && KMyMoneySettings::encryptRecover()) {
    if (KGPGFile::GPGAvailable()) {
      KGPGFile file;
      QDateTime expirationDate = file.keyExpires(QLatin1String(recoveryKeyId));
      if (expirationDate.isValid() && QDateTime::currentDateTime().daysTo(expirationDate) <= RECOVER_KEY_EXPIRATION_WARNING) {
        bool skipMessage = false;

        //get global config object for our app.
        KSharedConfigPtr kconfig = KSharedConfig::openConfig();
        KConfigGroup grp;
        QDate lastWarned;
        if (kconfig) {
          grp = d->m_config->group("General Options");
          lastWarned = grp.readEntry("LastRecoverKeyExpirationWarning", QDate());
          if (QDate::currentDate() == lastWarned) {
            skipMessage = true;
          }
        }
        if (!skipMessage) {
          if (kconfig) {
            grp.writeEntry("LastRecoverKeyExpirationWarning", QDate::currentDate());
          }
          KMessageBox::information(this, i18np("You have configured KMyMoney to use GPG to protect your data and to encrypt your data also with the KMyMoney recover key. This key is about to expire in %1 day. Please update the key from a keyserver using your GPG frontend (e.g. KGPG).", "You have configured KMyMoney to use GPG to protect your data and to encrypt your data also with the KMyMoney recover key. This key is about to expire in %1 days. Please update the key from a keyserver using your GPG frontend (e.g. KGPG).", QDateTime::currentDateTime().daysTo(expirationDate)), i18n("Recover key expires soon"));
        }
      }
    }
  }
}

void KMyMoneyApp::slotBackupFile()
{
  // Save the file first so isLocalFile() works
  if (d->m_myMoneyView && d->m_myMoneyView->dirty())

  {
    if (KMessageBox::questionYesNo(this, i18n("The file must be saved first "
                                   "before it can be backed up.  Do you want to continue?")) == KMessageBox::No) {
      return;

    }

    slotFileSave();
  }



  if (d->m_fileName.isEmpty())
    return;

  if (!d->m_fileName.isLocalFile()) {
    KMessageBox::sorry(this,
                       i18n("The current implementation of the backup functionality only supports local files as source files. Your current source file is '%1'.", d->m_fileName.url()),

                       i18n("Local files only"));
    return;
  }

  QPointer<KBackupDlg> backupDlg = new KBackupDlg(this);
#ifdef Q_OS_WIN
  backupDlg->mountCheckBox->setEnabled(false);
#endif
  int returncode = backupDlg->exec();
  if (returncode == QDialog::Accepted && backupDlg != 0) {

    d->m_backupMount = backupDlg->mountCheckBox->isChecked();
    d->m_proc.clearProgram();
    d->m_backupState = BACKUP_MOUNTING;
    d->m_mountpoint = backupDlg->txtMountPoint->text();

    if (d->m_backupMount) {
      slotBackupMount();
    } else {
      progressCallback(0, 300, "");
#ifdef Q_OS_WIN
      d->m_ignoreBackupExitCode = true;
      QTimer::singleShot(0, this, SLOT(slotBackupHandleEvents()));
#else
      // If we don't have to mount a device, we just issue
      // a dummy command to start the copy operation
      d->m_proc.setProgram("true");
      d->m_proc.start();
#endif
    }

  }

  delete backupDlg;
}

void KMyMoneyApp::slotBackupMount()
{
  progressCallback(0, 300, i18n("Mounting %1", d->m_mountpoint));
  d->m_proc.setProgram("mount");
  d->m_proc << d->m_mountpoint;
  d->m_proc.start();
}

bool KMyMoneyApp::slotBackupWriteFile()
{
  QString today;
  today.sprintf("-%04d-%02d-%02d.kmy",
                QDate::currentDate().year(),
                QDate::currentDate().month(),
                QDate::currentDate().day());
  QString backupfile = d->m_mountpoint + '/' + d->m_fileName.fileName();
  KMyMoneyUtils::appendCorrectFileExt(backupfile, today);

  // check if file already exists and ask what to do
  QFile f(backupfile);
  if (f.exists()) {
    int answer = KMessageBox::warningContinueCancel(this, i18n("Backup file for today exists on that device. Replace?"), i18n("Backup"), KGuiItem(i18n("&Replace")));
    if (answer == KMessageBox::Cancel) {
      return false;
    }
  }

  progressCallback(50, 0, i18n("Writing %1", backupfile));
  d->m_proc.clearProgram();
#ifdef Q_OS_WIN
  d->m_proc << "cmd.exe" << "/c" << "copy" << "/b" << "/y";
  d->m_proc << (QDir::toNativeSeparators(d->m_fileName.toLocalFile()) + "+ nul") << QDir::toNativeSeparators(backupfile);
#else
  d->m_proc << "cp" << "-f";
  d->m_proc << d->m_fileName.toLocalFile() << backupfile;
#endif
  d->m_backupState = BACKUP_COPYING;
  d->m_proc.start();
  return true;
}

void KMyMoneyApp::slotBackupUnmount()
{
  progressCallback(250, 0, i18n("Unmounting %1", d->m_mountpoint));
  d->m_proc.clearProgram();
  d->m_proc.setProgram("umount");
  d->m_proc << d->m_mountpoint;
  d->m_backupState = BACKUP_UNMOUNTING;
  d->m_proc.start();
}

void KMyMoneyApp::slotBackupFinish()
{
  d->m_backupState = BACKUP_IDLE;
  progressCallback(-1, -1, QString());
  ready();
}

void KMyMoneyApp::slotBackupHandleEvents()
{
  switch (d->m_backupState) {
    case BACKUP_MOUNTING:

      if (d->m_ignoreBackupExitCode ||
         (d->m_proc.exitStatus() == QProcess::NormalExit && d->m_proc.exitCode() == 0)) {
        d->m_ignoreBackupExitCode = false;
        d->m_backupResult = 0;
        if (!slotBackupWriteFile()) {
          d->m_backupResult = 1;
          if (d->m_backupMount)
            slotBackupUnmount();
          else
            slotBackupFinish();
        }
      } else {
        KMessageBox::information(this, i18n("Error mounting device"), i18n("Backup"));
        d->m_backupResult = 1;
        if (d->m_backupMount)
          slotBackupUnmount();
        else
          slotBackupFinish();
      }
      break;

    case BACKUP_COPYING:
      if (d->m_proc.exitStatus() == QProcess::NormalExit && d->m_proc.exitCode() == 0) {

        if (d->m_backupMount) {
          slotBackupUnmount();
        } else {
          progressCallback(300, 0, i18nc("Backup done", "Done"));
          KMessageBox::information(this, i18n("File successfully backed up"), i18n("Backup"));
          slotBackupFinish();
        }
      } else {
        qDebug("copy exit code is %d", d->m_proc.exitCode());
        d->m_backupResult = 1;
        KMessageBox::information(this, i18n("Error copying file to device"), i18n("Backup"));
        if (d->m_backupMount)
          slotBackupUnmount();
        else
          slotBackupFinish();
      }
      break;


    case BACKUP_UNMOUNTING:
      if (d->m_proc.exitStatus() == QProcess::NormalExit && d->m_proc.exitCode() == 0) {

        progressCallback(300, 0, i18nc("Backup done", "Done"));
        if (d->m_backupResult == 0)
          KMessageBox::information(this, i18n("File successfully backed up"), i18n("Backup"));
      } else {
        KMessageBox::information(this, i18n("Error unmounting device"), i18n("Backup"));
      }
      slotBackupFinish();
      break;

    default:
      qWarning("Unknown state for backup operation!");
      progressCallback(-1, -1, QString());
      ready();
      break;
  }
}

void KMyMoneyApp::slotShowTipOfTheDay()
{
  KTipDialog::showTip(d->m_myMoneyView, "", true);
}

void KMyMoneyApp::slotShowPreviousView()
{

}

void KMyMoneyApp::slotShowNextView()
{

}

void KMyMoneyApp::slotGenerateSql()
{
  QPointer<KGenerateSqlDlg> editor = new KGenerateSqlDlg(this);
  editor->setObjectName("Generate Database SQL");
  editor->exec();
  delete editor;
}

void KMyMoneyApp::slotToolsStartKCalc()
{
  QString cmd = KMyMoneyGlobalSettings::externalCalculator();
  // if none is present, we fall back to the default
  if (cmd.isEmpty()) {
#if defined(Q_OS_WIN32)
    cmd = QLatin1String("calc");
#elif defined(Q_OS_MAC)
    cmd = QLatin1String("open -a Calculator");
#else
    cmd = QLatin1String("kcalc");
#endif
  }
  KRun::runCommand(cmd, this);
}

void KMyMoneyApp::slotFindTransaction()
{
  if (d->m_searchDlg == 0) {
    d->m_searchDlg = new KFindTransactionDlg(this);
    connect(d->m_searchDlg, SIGNAL(destroyed()), this, SLOT(slotCloseSearchDialog()));
    connect(d->m_searchDlg, SIGNAL(transactionSelected(QString,QString)),
            d->m_myMoneyView, SLOT(slotLedgerSelected(QString,QString)));
  }
  d->m_searchDlg->show();
  d->m_searchDlg->raise();
  d->m_searchDlg->activateWindow();
}

void KMyMoneyApp::slotCloseSearchDialog()
{
  if (d->m_searchDlg)
    d->m_searchDlg->deleteLater();
  d->m_searchDlg = 0;
}

void KMyMoneyApp::createInstitution(MyMoneyInstitution& institution)
{
  MyMoneyFile* file = MyMoneyFile::instance();

  MyMoneyFileTransaction ft;

  try {
    file->addInstitution(institution);
    ft.commit();

  } catch (const MyMoneyException &e) {
    KMessageBox::information(this, i18n("Cannot add institution: %1", e.what()));
  }
}

void KMyMoneyApp::slotInstitutionNew()
{
  MyMoneyInstitution institution;
  slotInstitutionNew(institution);
}

void KMyMoneyApp::slotInstitutionNew(MyMoneyInstitution& institution)
{
  institution.clearId();
  QPointer<KNewBankDlg> dlg = new KNewBankDlg(institution);
  if (dlg->exec() == QDialog::Accepted && dlg != 0) {
    institution = dlg->institution();
    createInstitution(institution);
  }
  delete dlg;
}

void KMyMoneyApp::slotInstitutionEdit()
{
  slotInstitutionEdit(MyMoneyInstitution());
}

void KMyMoneyApp::slotInstitutionEdit(const MyMoneyObject& obj)
{
  if (typeid(obj) != typeid(MyMoneyInstitution))
    return;

  // make sure the selected object has an id
  if (d->m_selectedInstitution.id().isEmpty())
    return;

  try {
    MyMoneyFile* file = MyMoneyFile::instance();

    //grab a pointer to the view, regardless of it being a account or institution view.
    MyMoneyInstitution institution = file->institution(d->m_selectedInstitution.id());

    // bankSuccess is not checked anymore because d->m_file->institution will throw anyway
    QPointer<KNewBankDlg> dlg = new KNewBankDlg(institution);
    if (dlg->exec() == QDialog::Accepted && dlg != 0) {
      MyMoneyFileTransaction ft;
      try {
        file->modifyInstitution(dlg->institution());
        ft.commit();
        slotSelectInstitution(file->institution(dlg->institution().id()));

      } catch (const MyMoneyException &e) {
        KMessageBox::information(this, i18n("Unable to store institution: %1", e.what()));
      }
    }
    delete dlg;

  } catch (const MyMoneyException &e) {
    if (!obj.id().isEmpty())
      KMessageBox::information(this, i18n("Unable to edit institution: %1", e.what()));
  }
}

void KMyMoneyApp::slotInstitutionDelete()
{
  MyMoneyFile *file = MyMoneyFile::instance();
  try {

    MyMoneyInstitution institution = file->institution(d->m_selectedInstitution.id());
    if ((KMessageBox::questionYesNo(this, i18n("<p>Do you really want to delete the institution <b>%1</b>?</p>", institution.name()))) == KMessageBox::No)
      return;
    MyMoneyFileTransaction ft;

    try {
      file->removeInstitution(institution);
      ft.commit();
    } catch (const MyMoneyException &e) {
      KMessageBox::information(this, i18n("Unable to delete institution: %1", e.what()));
    }
  } catch (const MyMoneyException &e) {
    KMessageBox::information(this, i18n("Unable to delete institution: %1", e.what()));
  }
}

void KMyMoneyApp::createAccount(MyMoneyAccount& newAccount, MyMoneyAccount& parentAccount, MyMoneyAccount& brokerageAccount, MyMoneyMoney openingBal)
{
  MyMoneyFile *file = MyMoneyFile::instance();
  try {
    const MyMoneySecurity& sec = file->security(newAccount.currencyId());
    // Check the opening balance
    if (openingBal.isPositive() && newAccount.accountGroup() == MyMoneyAccount::Liability) {
      QString message = i18n("This account is a liability and if the "
                             "opening balance represents money owed, then it should be negative.  "
                             "Negate the amount?\n\n"
                             "Please click Yes to change the opening balance to %1,\n"
                             "Please click No to leave the amount as %2,\n"
                             "Please click Cancel to abort the account creation."
                             , MyMoneyUtils::formatMoney(-openingBal, newAccount, sec)
                             , MyMoneyUtils::formatMoney(openingBal, newAccount, sec));

      int ans = KMessageBox::questionYesNoCancel(this, message);
      if (ans == KMessageBox::Yes) {
        openingBal = -openingBal;

      } else if (ans == KMessageBox::Cancel)
        return;
    }

    file->createAccount(newAccount, parentAccount, brokerageAccount, openingBal);

  } catch (const MyMoneyException &e) {
    KMessageBox::information(this, i18n("Unable to add account: %1", e.what()));
  }
}

void KMyMoneyApp::slotCategoryNew(const QString& name, QString& id)
{
  MyMoneyAccount account;
  account.setName(name);

  slotCategoryNew(account, MyMoneyFile::instance()->expense());

  id = account.id();
}

void KMyMoneyApp::slotCategoryNew(MyMoneyAccount& account, const MyMoneyAccount& parent)
{
  if (KMessageBox::questionYesNo(this,
                                 QString("<qt>%1</qt>").arg(i18n("<p>The category <b>%1</b> currently does not exist. Do you want to create it?</p><p><i>The parent account will default to <b>%2</b> but can be changed in the following dialog</i>.</p>", account.name(), parent.name())), i18n("Create category"),
                                 KStandardGuiItem::yes(), KStandardGuiItem::no(), "CreateNewCategories") == KMessageBox::Yes) {
    createCategory(account, parent);
  } else {
    // we should not keep the 'no' setting because that can confuse people like
    // I have seen in some usability tests. So we just delete it right away.
    KSharedConfigPtr kconfig = KSharedConfig::openConfig();
    if (kconfig) {
      kconfig->group(QLatin1String("Notification Messages")).deleteEntry(QLatin1String("CreateNewCategories"));
    }
  }
}

void KMyMoneyApp::slotCategoryNew()
{
  MyMoneyAccount parent;
  MyMoneyAccount account;

  // Preselect the parent account by looking at the current selected account/category
  if (!d->m_selectedAccount.id().isEmpty() && d->m_selectedAccount.isIncomeExpense()) {
    MyMoneyFile* file = MyMoneyFile::instance();
    try {
      parent = file->account(d->m_selectedAccount.id());
    } catch (const MyMoneyException &) {
    }
  }

  createCategory(account, parent);
}

void KMyMoneyApp::createCategory(MyMoneyAccount& account, const MyMoneyAccount& parent)
{
  if (!parent.id().isEmpty()) {
    try {
      // make sure parent account exists
      MyMoneyFile::instance()->account(parent.id());
      account.setParentAccountId(parent.id());
      account.setAccountType(parent.accountType());
    } catch (const MyMoneyException &) {
    }
  }

  QPointer<KNewAccountDlg> dialog =
    new KNewAccountDlg(account, false, true, 0, i18n("Create a new Category"));

  dialog->setOpeningBalanceShown(false);
  dialog->setOpeningDateShown(false);

  if (dialog->exec() == QDialog::Accepted && dialog != 0) {
    MyMoneyAccount parentAccount, brokerageAccount;
    account = dialog->account();
    parentAccount = dialog->parentAccount();

    MyMoneyFile::instance()->createAccount(account, parentAccount, brokerageAccount, MyMoneyMoney());
  }
  delete dialog;
}

void KMyMoneyApp::slotAccountNew()
{
  MyMoneyAccount acc;
  acc.setInstitutionId(d->m_selectedInstitution.id());
  acc.setOpeningDate(KMyMoneyGlobalSettings::firstFiscalDate());

  slotAccountNew(acc);
}

void KMyMoneyApp::slotAccountNew(MyMoneyAccount& account)
{
  NewAccountWizard::Wizard* wizard = new NewAccountWizard::Wizard();
  connect(wizard, SIGNAL(createInstitution(MyMoneyInstitution&)), this, SLOT(slotInstitutionNew(MyMoneyInstitution&)));
  connect(wizard, SIGNAL(createAccount(MyMoneyAccount&)), this, SLOT(slotAccountNew(MyMoneyAccount&)));
  connect(wizard, SIGNAL(createPayee(QString,QString&)), this, SLOT(slotPayeeNew(QString,QString&)));
  connect(wizard, SIGNAL(createCategory(MyMoneyAccount&,MyMoneyAccount)), this, SLOT(slotCategoryNew(MyMoneyAccount&,MyMoneyAccount)));

  wizard->setAccount(account);

  if (wizard->exec() == QDialog::Accepted) {
    MyMoneyAccount acc = wizard->account();
    if (!(acc == MyMoneyAccount())) {
      MyMoneyFileTransaction ft;
      MyMoneyFile* file = MyMoneyFile::instance();
      try {
        // create the account
        MyMoneyAccount parent = wizard->parentAccount();
        file->addAccount(acc, parent);

        // tell the wizard about the account id which it
        // needs to create a possible schedule and transactions
        wizard->setAccount(acc);

        // store a possible conversion rate for the currency
        if (acc.currencyId() != file->baseCurrency().id()) {
          file->addPrice(wizard->conversionRate());
        }

        // create the opening balance transaction if any
        file->createOpeningBalanceTransaction(acc, wizard->openingBalance());
        // create the payout transaction for loans if any
        MyMoneyTransaction payoutTransaction = wizard->payoutTransaction();
        if (payoutTransaction.splits().count() > 0) {
          file->addTransaction(payoutTransaction);
        }

        // create a brokerage account if selected
        MyMoneyAccount brokerageAccount = wizard->brokerageAccount();
        if (!(brokerageAccount == MyMoneyAccount())) {
          file->addAccount(brokerageAccount, parent);
        }

        // create a possible schedule
        MyMoneySchedule sch = wizard->schedule();
        if (!(sch == MyMoneySchedule())) {
          MyMoneyFile::instance()->addSchedule(sch);
          if (acc.isLoan()) {
            MyMoneyAccountLoan accLoan = MyMoneyFile::instance()->account(acc.id());
            accLoan.setSchedule(sch.id());
            acc = accLoan;
            MyMoneyFile::instance()->modifyAccount(acc);
          }
        }
        ft.commit();
        account = acc;
      } catch (const MyMoneyException &e) {
        KMessageBox::error(this, i18n("Unable to create account: %1", e.what()));
      }
    }
  }
  delete wizard;
}

void KMyMoneyApp::slotInvestmentNew(MyMoneyAccount& account, const MyMoneyAccount& parent)
{
  QString dontShowAgain = "CreateNewInvestments";
  if (KMessageBox::questionYesNo(this,
                                 QString("<qt>") + i18n("The security <b>%1</b> currently does not exist as sub-account of <b>%2</b>. "
                                                        "Do you want to create it?", account.name(), parent.name()) + QString("</qt>"), i18n("Create security"),
                                 KStandardGuiItem::yes(), KStandardGuiItem::no(), dontShowAgain) == KMessageBox::Yes) {
    KNewInvestmentWizard dlg;
    dlg.setName(account.name());
    if (dlg.exec() == QDialog::Accepted) {
      dlg.createObjects(parent.id());
      account = dlg.account();
    }
  } else {
    // in case the user said no but turned on the don't show again selection, we will enable
    // the message no matter what. Otherwise, the user is not able to use this feature
    // in the future anymore.
    KMessageBox::enableMessage(dontShowAgain);
  }
}

void KMyMoneyApp::slotInvestmentNew()
{
  QPointer<KNewInvestmentWizard> dlg = new KNewInvestmentWizard(this);
  if (dlg->exec() == QDialog::Accepted)
    dlg->createObjects(d->m_selectedAccount.id());
  delete dlg;
}

void KMyMoneyApp::slotInvestmentEdit()
{
  QPointer<KNewInvestmentWizard> dlg = new KNewInvestmentWizard(d->m_selectedInvestment);
  if (dlg->exec() == QDialog::Accepted)
    dlg->createObjects(d->m_selectedAccount.id());
  delete dlg;
}

void KMyMoneyApp::slotInvestmentDelete()
{
  if (KMessageBox::questionYesNo(this, i18n("<p>Do you really want to delete the investment <b>%1</b>?</p>", d->m_selectedInvestment.name()), i18n("Delete investment"), KStandardGuiItem::yes(), KStandardGuiItem::no(), "DeleteInvestment") == KMessageBox::Yes) {
    MyMoneyFile* file = MyMoneyFile::instance();
    MyMoneyFileTransaction ft;
    try {
      d->m_selectedAccount = MyMoneyAccount(); // CAUTION: deleting equity from investments view needs this, if ID of the equity to be deleted is the smallest from all
      file->removeAccount(d->m_selectedInvestment);
      ft.commit();
    } catch (const MyMoneyException &e) {
      KMessageBox::information(this, i18n("Unable to delete investment: %1", e.what()));
    }
  } else {
    // we should not keep the 'no' setting because that can confuse people like
    // I have seen in some usability tests. So we just delete it right away.
    KSharedConfigPtr kconfig = KSharedConfig::openConfig();
    if (kconfig) {
      kconfig->group(QLatin1String("Notification Messages")).deleteEntry(QLatin1String("DeleteInvestment"));
    }
  }
}

void KMyMoneyApp::slotOnlinePriceUpdate()
{
  if (!d->m_selectedInvestment.id().isEmpty()) {
    QPointer<KEquityPriceUpdateDlg> dlg =
      new KEquityPriceUpdateDlg(0, d->m_selectedInvestment.currencyId());
    if (dlg->exec() == QDialog::Accepted && dlg != 0) {
      dlg->storePrices();
    }
    delete dlg;
  }
}

void KMyMoneyApp::slotManualPriceUpdate()
{
  if (!d->m_selectedInvestment.id().isEmpty()) {
    try {
      MyMoneySecurity security = MyMoneyFile::instance()->security(d->m_selectedInvestment.currencyId());
      MyMoneySecurity currency = MyMoneyFile::instance()->security(security.tradingCurrency());
      const MyMoneyPrice &price = MyMoneyFile::instance()->price(security.id(), currency.id());

      QPointer<KCurrencyCalculator> calc =
        new KCurrencyCalculator(security, currency, MyMoneyMoney::ONE, price.rate(currency.id()), price.date(), MyMoneyMoney::precToDenom(security.pricePrecision()));
      calc->setupPriceEditor();

      // The dialog takes care of adding the price if necessary
      calc->exec();
      delete calc;
    } catch (const MyMoneyException &e) {
      qDebug("Error in price update: %s", qPrintable(e.what()));
    }
  }
}

void KMyMoneyApp::createSchedule(MyMoneySchedule newSchedule, MyMoneyAccount& newAccount)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  // Add the schedule only if one exists
  //
  // Remember to modify the first split to reference the newly created account
  if (!newSchedule.name().isEmpty()) {
    try {
      // We assume at least 2 splits in the transaction
      MyMoneyTransaction t = newSchedule.transaction();
      if (t.splitCount() < 2) {
        throw MYMONEYEXCEPTION("Transaction for schedule has less than 2 splits!");
      }

      MyMoneyFileTransaction ft;
      try {
        file->addSchedule(newSchedule);

        // in case of a loan account, we keep a reference to this
        // schedule in the account
        if (newAccount.accountType() == MyMoneyAccount::Loan
            || newAccount.accountType() == MyMoneyAccount::AssetLoan) {
          newAccount.setValue("schedule", newSchedule.id());
          file->modifyAccount(newAccount);
        }
        ft.commit();
      } catch (const MyMoneyException &e) {
        KMessageBox::information(this, i18n("Unable to add scheduled transaction: %1", e.what()));
      }
    } catch (const MyMoneyException &e) {
      KMessageBox::information(this, i18n("Unable to add scheduled transaction: %1", e.what()));
    }
  }
}

void KMyMoneyApp::slotAccountDelete()
{
  if (d->m_selectedAccount.id().isEmpty())
    return;  // need an account ID

  MyMoneyFile* file = MyMoneyFile::instance();
  // can't delete standard accounts or account which still have transactions assigned
  if (file->isStandardAccount(d->m_selectedAccount.id()))
    return;

  // check if the account is referenced by a transaction or schedule
  MyMoneyFileBitArray skip(IMyMoneyStorage::MaxRefCheckBits);
  skip.fill(false);
  skip.setBit(IMyMoneyStorage::RefCheckAccount);
  skip.setBit(IMyMoneyStorage::RefCheckInstitution);
  skip.setBit(IMyMoneyStorage::RefCheckPayee);
  skip.setBit(IMyMoneyStorage::RefCheckTag);
  skip.setBit(IMyMoneyStorage::RefCheckSecurity);
  skip.setBit(IMyMoneyStorage::RefCheckCurrency);
  skip.setBit(IMyMoneyStorage::RefCheckPrice);
  bool hasReference = file->isReferenced(d->m_selectedAccount, skip);

  // make sure we only allow transactions in a 'category' (income/expense account)
  switch (d->m_selectedAccount.accountType()) {
    case MyMoneyAccount::Income:
    case MyMoneyAccount::Expense:
      break;

    default:
      // if the account is still referenced
      if (hasReference) {
        return;
      }
      break;
  }

  // if we get here and still have transactions referencing the account, we
  // need to check with the user to possibly re-assign them to a different account
  bool needAskUser = true;
  bool exit = false;

  MyMoneyFileTransaction ft;

  if (hasReference) {
    // show transaction reassignment dialog

    needAskUser = false;
    KCategoryReassignDlg* dlg = new KCategoryReassignDlg(this);
    QString categoryId = dlg->show(d->m_selectedAccount);
    delete dlg; // and kill the dialog
    if (categoryId.isEmpty())
      return; // the user aborted the dialog, so let's abort as well

    MyMoneyAccount newCategory = file->account(categoryId);
    try {
      {
        KMSTATUS(i18n("Adjusting transactions..."));
        /*
          d->m_selectedAccount.id() is the old id, categoryId the new one
          Now search all transactions and schedules that reference d->m_selectedAccount.id()
          and replace that with categoryId.
        */
        // get the list of all transactions that reference the old account
        MyMoneyTransactionFilter filter(d->m_selectedAccount.id());
        filter.setReportAllSplits(false);
        QList<MyMoneyTransaction> tlist;
        QList<MyMoneyTransaction>::iterator it_t;
        file->transactionList(tlist, filter);

        slotStatusProgressBar(0, tlist.count());
        int cnt = 0;
        for (it_t = tlist.begin(); it_t != tlist.end(); ++it_t) {
          slotStatusProgressBar(++cnt, 0);
          MyMoneyTransaction t = (*it_t);
          if (t.replaceId(categoryId, d->m_selectedAccount.id()))
            file->modifyTransaction(t);
        }
        slotStatusProgressBar(tlist.count(), 0);
      }
      // now fix all schedules
      {
        KMSTATUS(i18n("Adjusting scheduled transactions..."));
        QList<MyMoneySchedule> slist = file->scheduleList(d->m_selectedAccount.id());
        QList<MyMoneySchedule>::iterator it_s;

        int cnt = 0;
        slotStatusProgressBar(0, slist.count());
        for (it_s = slist.begin(); it_s != slist.end(); ++it_s) {
          slotStatusProgressBar(++cnt, 0);
          MyMoneySchedule sch = (*it_s);
          if (sch.replaceId(categoryId, d->m_selectedAccount.id())) {
            file->modifySchedule(sch);
          }
        }
        slotStatusProgressBar(slist.count(), 0);
      }
      // now fix all budgets
      {
        KMSTATUS(i18n("Adjusting budgets..."));
        QList<MyMoneyBudget> blist = file->budgetList();
        QList<MyMoneyBudget>::const_iterator it_b;
        for (it_b = blist.constBegin(); it_b != blist.constEnd(); ++it_b) {
          if ((*it_b).hasReferenceTo(d->m_selectedAccount.id())) {
            MyMoneyBudget b = (*it_b);
            MyMoneyBudget::AccountGroup fromBudget = b.account(d->m_selectedAccount.id());
            MyMoneyBudget::AccountGroup toBudget = b.account(categoryId);
            toBudget += fromBudget;
            b.setAccount(toBudget, categoryId);
            b.removeReference(d->m_selectedAccount.id());
            file->modifyBudget(b);

          }
        }
        slotStatusProgressBar(blist.count(), 0);
      }
    } catch (MyMoneyException &e) {
      KMessageBox::error(this, i18n("Unable to exchange category <b>%1</b> with category <b>%2</b>. Reason: %3", d->m_selectedAccount.name(), newCategory.name(), e.what()));
      exit = true;
    }
    slotStatusProgressBar(-1, -1);
  }

  if (exit)
    return;

  // retain the account name for a possible later usage in the error message box
  // since the account removal notifies the views the selected account can be changed
  // so we make sure by doing this that we display the correct name in the error message
  QString selectedAccountName = d->m_selectedAccount.name();

  // at this point, we must not have a reference to the account
  // to be deleted anymore
  switch (d->m_selectedAccount.accountGroup()) {
      // special handling for categories to allow deleting of empty subcategories
    case MyMoneyAccount::Income:
    case MyMoneyAccount::Expense: { // open a compound statement here to be able to declare variables
        // which would otherwise not work within a case label.

        // case A - only a single, unused category without subcats selected
        if (d->m_selectedAccount.accountList().isEmpty()) {
          if (!needAskUser || (KMessageBox::questionYesNo(this, QString("<qt>") + i18n("Do you really want to delete category <b>%1</b>?", selectedAccountName) + QString("</qt>")) == KMessageBox::Yes)) {
            try {
              file->removeAccount(d->m_selectedAccount);
              d->m_selectedAccount.clearId();
              slotUpdateActions();
              ft.commit();
            } catch (const MyMoneyException &e) {
              KMessageBox::error(this, QString("<qt>") + i18n("Unable to delete category <b>%1</b>. Cause: %2", selectedAccountName, e.what()) + QString("</qt>"));
            }
          }
          return;
        }
        // case B - we have some subcategories, maybe the user does not want to
        //          delete them all, but just the category itself?
        MyMoneyAccount parentAccount = file->account(d->m_selectedAccount.parentAccountId());

        QStringList accountsToReparent;
        int result = KMessageBox::questionYesNoCancel(this, QString("<qt>") +
                     i18n("Do you want to delete category <b>%1</b> with all its sub-categories or only "
                          "the category itself? If you only delete the category itself, all its sub-categories "
                          "will be made sub-categories of <b>%2</b>.", selectedAccountName, parentAccount.name()) + QString("</qt>"),
                     QString(),
                     KGuiItem(i18n("Delete all")),
                     KGuiItem(i18n("Just the category")));
        if (result == KMessageBox::Cancel)
          return; // cancel pressed? ok, no delete then...
        // "No" means "Just the category" and that means we need to reparent all subaccounts
        bool need_confirmation = false;
        // case C - User only wants to delete the category itself
        if (result == KMessageBox::No)
          accountsToReparent = d->m_selectedAccount.accountList();
        else {
          // case D - User wants to delete all subcategories, now check all subcats of
          //          d->m_selectedAccount and remember all that cannot be deleted and
          //          must be "reparented"
          for (QStringList::const_iterator it = d->m_selectedAccount.accountList().begin();
               it != d->m_selectedAccount.accountList().end(); ++it) {
            // reparent account if a transaction is assigned
            if (file->transactionCount(*it) != 0)
              accountsToReparent.push_back(*it);
            else if (!file->account(*it).accountList().isEmpty()) {
              // or if we have at least one sub-account that is used for transactions
              if (!file->hasOnlyUnusedAccounts(file->account(*it).accountList())) {
                accountsToReparent.push_back(*it);
                //qDebug() << "subaccount not empty";
              }
            }
          }
          if (!accountsToReparent.isEmpty())
            need_confirmation = true;
        }
        if (!accountsToReparent.isEmpty() && need_confirmation) {
          if (KMessageBox::questionYesNo(this, i18n("<p>Some sub-categories of category <b>%1</b> cannot "
                                         "be deleted, because they are still used. They will be made sub-categories of <b>%2</b>. Proceed?</p>", selectedAccountName, parentAccount.name())) != KMessageBox::Yes) {
            return; // user gets wet feet...
          }
        }
        // all good, now first reparent selected sub-categories
        try {
          MyMoneyAccount parent = file->account(d->m_selectedAccount.parentAccountId());
          for (QStringList::const_iterator it = accountsToReparent.constBegin(); it != accountsToReparent.constEnd(); ++it) {
            MyMoneyAccount child = file->account(*it);
            file->reparentAccount(child, parent);
          }
          // reload the account because the sub-account list might have changed
          d->m_selectedAccount = file->account(d->m_selectedAccount.id());
          // now recursively delete remaining sub-categories
          file->removeAccountList(d->m_selectedAccount.accountList());
          // don't forget to update d->m_selectedAccount, because we still have a copy of
          // the old account list, which is no longer valid
          d->m_selectedAccount = file->account(d->m_selectedAccount.id());
        } catch (const MyMoneyException &e) {
          KMessageBox::error(this, QString("<qt>") + i18n("Unable to delete a sub-category of category <b>%1</b>. Reason: %2", selectedAccountName, e.what()) + QString("</qt>"));
          return;
        }
      }
      break; // the category/account is deleted after the switch

    default:
      if (!d->m_selectedAccount.accountList().isEmpty())
        return; // can't delete accounts which still have subaccounts

      if (KMessageBox::questionYesNo(this, i18n("<p>Do you really want to "
                                     "delete account <b>%1</b>?</p>", selectedAccountName)) != KMessageBox::Yes) {
        return; // ok, you don't want to? why did you click then, hmm?
      }
  } // switch;

  try {
    file->removeAccount(d->m_selectedAccount);
    d->m_selectedAccount.clearId();
    slotUpdateActions();
    ft.commit();
  } catch (const MyMoneyException &e) {
    KMessageBox::error(this, i18n("Unable to delete account '%1'. Cause: %2", selectedAccountName, e.what()));
  }
}

void KMyMoneyApp::slotAccountEdit()
{
  MyMoneyFile* file = MyMoneyFile::instance();
  if (!d->m_selectedAccount.id().isEmpty()) {
    if (!file->isStandardAccount(d->m_selectedAccount.id())) {
      if (d->m_selectedAccount.accountType() != MyMoneyAccount::Loan
          && d->m_selectedAccount.accountType() != MyMoneyAccount::AssetLoan) {
        QString caption;
        bool category = false;
        switch (d->m_selectedAccount.accountGroup()) {
          default:
            caption = i18n("Edit account '%1'", d->m_selectedAccount.name());
            break;

          case MyMoneyAccount::Expense:
          case MyMoneyAccount::Income:
            caption = i18n("Edit category '%1'", d->m_selectedAccount.name());
            category = true;
            break;
        }

        // set a status message so that the application can't be closed until the editing is done
        slotStatusMsg(caption);

        QString tid = file->openingBalanceTransaction(d->m_selectedAccount);
        MyMoneyTransaction t;
        MyMoneySplit s0, s1;
        QPointer<KNewAccountDlg> dlg =
          new KNewAccountDlg(d->m_selectedAccount, true, category, 0, caption);

        if (category) {
          dlg->setOpeningBalanceShown(false);
          dlg->setOpeningDateShown(false);
          tid.clear();
        } else {
          if (!tid.isEmpty()) {
            try {
              t = file->transaction(tid);
              s0 = t.splitByAccount(d->m_selectedAccount.id());
              s1 = t.splitByAccount(d->m_selectedAccount.id(), false);
              dlg->setOpeningBalance(s0.shares());
              if (d->m_selectedAccount.accountGroup() == MyMoneyAccount::Liability) {
                dlg->setOpeningBalance(-s0.shares());
              }
            } catch (const MyMoneyException &e) {
              qDebug() << "Error retrieving opening balance transaction " << tid << ": " << e.what() << "\n";
              tid.clear();
            }
          }
        }

        // check for online modules
        QMap<QString, KMyMoneyPlugin::OnlinePlugin *>::const_iterator it_plugin = d->m_onlinePlugins.constEnd();
        const MyMoneyKeyValueContainer& kvp = d->m_selectedAccount.onlineBankingSettings();
        if (!kvp["provider"].isEmpty()) {
          // if we have an online provider for this account, we need to check
          // that we have the corresponding plugin. If that exists, we ask it
          // to provide an additional tab for the account editor.
          it_plugin = d->m_onlinePlugins.constFind(kvp["provider"]);
          if (it_plugin != d->m_onlinePlugins.constEnd()) {
            QString name;
            QWidget *w = (*it_plugin)->accountConfigTab(d->m_selectedAccount, name);
            dlg->addTab(w, name);
          }
        }

        if (dlg != 0 && dlg->exec() == QDialog::Accepted) {
          try {
            MyMoneyFileTransaction ft;

            MyMoneyAccount account = dlg->account();
            MyMoneyAccount parent = dlg->parentAccount();
            if (it_plugin != d->m_onlinePlugins.constEnd()) {
              account.setOnlineBankingSettings((*it_plugin)->onlineBankingSettings(account.onlineBankingSettings()));
            }
            MyMoneyMoney bal = dlg->openingBalance();
            if (d->m_selectedAccount.accountGroup() == MyMoneyAccount::Liability) {
              bal = -bal;
            }

            // we need to modify first, as reparent would override all other changes
            file->modifyAccount(account);
            if (account.parentAccountId() != parent.id()) {
              file->reparentAccount(account, parent);
            }
            if (!tid.isEmpty() && dlg->openingBalance().isZero()) {
              file->removeTransaction(t);

            } else if (!tid.isEmpty() && !dlg->openingBalance().isZero()) {
              s0.setShares(bal);
              s0.setValue(bal);
              t.modifySplit(s0);
              s1.setShares(-bal);
              s1.setValue(-bal);
              t.modifySplit(s1);
              t.setPostDate(account.openingDate());
              file->modifyTransaction(t);

            } else if (tid.isEmpty() && !dlg->openingBalance().isZero()) {
              file->createOpeningBalanceTransaction(d->m_selectedAccount, bal);
            }

            ft.commit();

            // reload the account object as it might have changed in the meantime
            slotSelectAccount(file->account(account.id()));

          } catch (const MyMoneyException &e) {
            KMessageBox::error(this, i18n("Unable to modify account '%1'. Cause: %2", d->m_selectedAccount.name(), e.what()));
          }
        }

        delete dlg;
        ready();
      } else {
        QPointer<KEditLoanWizard> wizard = new KEditLoanWizard(d->m_selectedAccount);
        connect(wizard, SIGNAL(newCategory(MyMoneyAccount&)), this, SLOT(slotCategoryNew(MyMoneyAccount&)));
        connect(wizard, SIGNAL(createPayee(QString,QString&)), this, SLOT(slotPayeeNew(QString,QString&)));
        if (wizard->exec() == QDialog::Accepted && wizard != 0) {
          MyMoneySchedule sch;
          try {
            MyMoneySchedule sch = file->schedule(d->m_selectedAccount.value("schedule").toLatin1());
          } catch (const MyMoneyException &e) {
            qDebug() << "schedule" << d->m_selectedAccount.value("schedule").toLatin1() << "not found";
          }
          if (!(d->m_selectedAccount == wizard->account())
              || !(sch == wizard->schedule())) {
            MyMoneyFileTransaction ft;
            try {
              file->modifyAccount(wizard->account());
              if (!sch.id().isEmpty()) {
                sch = wizard->schedule();
              }
              try {
                file->schedule(sch.id());
                file->modifySchedule(sch);
                ft.commit();
              } catch (const MyMoneyException &) {
                try {
                  if(sch.transaction().splitCount() >= 2) {
                    file->addSchedule(sch);
                  }
                  ft.commit();
                } catch (const MyMoneyException &e) {
                  qDebug("Cannot add schedule: '%s'", qPrintable(e.what()));
                }
              }
            } catch (const MyMoneyException &e) {
              qDebug("Unable to modify account %s: '%s'", qPrintable(d->m_selectedAccount.name()),
                     qPrintable(e.what()));
            }
          }
        }
        delete wizard;
      }
    }
  }
}

QList<QPair<MyMoneyTransaction, MyMoneySplit> > KMyMoneyApp::Private::automaticReconciliation(const MyMoneyAccount &account,
    const QList<QPair<MyMoneyTransaction, MyMoneySplit> > &transactions,
    const MyMoneyMoney &amount)
{
  static const int NR_OF_STEPS_LIMIT = 300000;
  static const int PROGRESSBAR_STEPS = 1000;
  QList<QPair<MyMoneyTransaction, MyMoneySplit> > result = transactions;

  KMSTATUS(i18n("Running automatic reconciliation"));
  int progressBarIndex = 0;
  kmymoney->slotStatusProgressBar(progressBarIndex, NR_OF_STEPS_LIMIT / PROGRESSBAR_STEPS);

  // optimize the most common case - all transactions should be cleared
  QListIterator<QPair<MyMoneyTransaction, MyMoneySplit> > itTransactionSplitResult(result);
  MyMoneyMoney transactionsBalance;
  while (itTransactionSplitResult.hasNext()) {
    const QPair<MyMoneyTransaction, MyMoneySplit> &transactionSplit = itTransactionSplitResult.next();
    transactionsBalance += transactionSplit.second.shares();
  }
  if (amount == transactionsBalance) {
    result = transactions;
    return result;
  }
  kmymoney->slotStatusProgressBar(progressBarIndex++, 0);
  // only one transaction is uncleared
  itTransactionSplitResult.toFront();
  int index = 0;
  while (itTransactionSplitResult.hasNext()) {
    const QPair<MyMoneyTransaction, MyMoneySplit> &transactionSplit = itTransactionSplitResult.next();
    if (transactionsBalance - transactionSplit.second.shares() == amount) {
      result.removeAt(index);
      return result;
    }
    index++;
  }
  kmymoney->slotStatusProgressBar(progressBarIndex++, 0);

  // more than one transaction is uncleared - apply the algorithm
  result.clear();

  const MyMoneySecurity &security = MyMoneyFile::instance()->security(account.currencyId());
  double precision = 0.1 / account.fraction(security);

  QList<MyMoneyMoney> sumList;
  sumList << MyMoneyMoney();

  QMap<MyMoneyMoney, QList<QPair<QString, QString> > > sumToComponentsMap;

  // compute the possible matches
  QListIterator<QPair<MyMoneyTransaction, MyMoneySplit> > itTransactionSplit(transactions);
  while (itTransactionSplit.hasNext()) {
    const QPair<MyMoneyTransaction, MyMoneySplit> &transactionSplit = itTransactionSplit.next();
    QListIterator<MyMoneyMoney> itSum(sumList);
    QList<MyMoneyMoney> tempList;
    while (itSum.hasNext()) {
      const MyMoneyMoney &sum = itSum.next();
      QList<QPair<QString, QString> > splitIds;
      splitIds << qMakePair<QString, QString>(transactionSplit.first.id(), transactionSplit.second.id());
      if (sumToComponentsMap.contains(sum)) {
        if (sumToComponentsMap.value(sum).contains(qMakePair<QString, QString>(transactionSplit.first.id(), transactionSplit.second.id()))) {
          continue;
        }
        splitIds.append(sumToComponentsMap.value(sum));
      }
      tempList << transactionSplit.second.shares() + sum;
      sumToComponentsMap[transactionSplit.second.shares() + sum] = splitIds;
      int size = sumToComponentsMap.size();
      if (size % PROGRESSBAR_STEPS == 0) {
        kmymoney->slotStatusProgressBar(progressBarIndex++, 0);
      }
      if (size > NR_OF_STEPS_LIMIT) {
        return result; // it's taking too much resources abort the algorithm
      }
    }
    QList<MyMoneyMoney> unionList;
    unionList.append(tempList);
    unionList.append(sumList);
    qSort(unionList);
    sumList.clear();
    MyMoneyMoney smallestSumFromUnion = unionList.first();
    sumList.append(smallestSumFromUnion);
    QListIterator<MyMoneyMoney> itUnion(unionList);
    while (itUnion.hasNext()) {
      MyMoneyMoney sumFromUnion = itUnion.next();
      if (smallestSumFromUnion < MyMoneyMoney(1 - precision / transactions.size())*sumFromUnion) {
        smallestSumFromUnion = sumFromUnion;
        sumList.append(sumFromUnion);
      }
    }
  }

  kmymoney->slotStatusProgressBar(NR_OF_STEPS_LIMIT / PROGRESSBAR_STEPS, 0);
  if (sumToComponentsMap.contains(amount)) {
    QListIterator<QPair<MyMoneyTransaction, MyMoneySplit> > itTransactionSplit(transactions);
    while (itTransactionSplit.hasNext()) {
      const QPair<MyMoneyTransaction, MyMoneySplit> &transactionSplit = itTransactionSplit.next();
      const QList<QPair<QString, QString> > &splitIds = sumToComponentsMap.value(amount);
      if (splitIds.contains(qMakePair<QString, QString>(transactionSplit.first.id(), transactionSplit.second.id()))) {
        result.append(transactionSplit);
      }
    }
  }

#ifdef KMM_DEBUG
  qDebug("For the amount %s a number of %d possible sums where computed from the set of %d transactions: ",
         qPrintable(MyMoneyUtils::formatMoney(amount, security)), sumToComponentsMap.size(), transactions.size());
#endif

  kmymoney->slotStatusProgressBar(-1, -1);
  return result;
}

void KMyMoneyApp::slotAccountReconcileStart()
{
  MyMoneyFile* file = MyMoneyFile::instance();
  MyMoneyAccount account;

  // we cannot reconcile standard accounts
  if (!file->isStandardAccount(d->m_selectedAccount.id())) {
    // check if we can reconcile this account
    // it make's sense for asset and liability accounts
    try {
      // check if we have overdue schedules for this account
      QList<MyMoneySchedule> schedules = file->scheduleList(d->m_selectedAccount.id(), MyMoneySchedule::TYPE_ANY, MyMoneySchedule::OCCUR_ANY, MyMoneySchedule::STYPE_ANY, QDate(), QDate(), true);
      if (schedules.count() > 0) {
        if (KMessageBox::questionYesNo(this, i18n("KMyMoney has detected some overdue scheduled transactions for this account. Do you want to enter those scheduled transactions now?"), i18n("Scheduled transactions found")) == KMessageBox::Yes) {

          QMap<QString, bool> skipMap;
          bool processedOne;
          KMyMoneyUtils::EnterScheduleResultCodeE rc = KMyMoneyUtils::Enter;
          do {
            processedOne = false;
            QList<MyMoneySchedule>::const_iterator it_sch;
            for (it_sch = schedules.constBegin(); (rc != KMyMoneyUtils::Cancel) && (it_sch != schedules.constEnd()); ++it_sch) {
              MyMoneySchedule sch(*(it_sch));

              // and enter it if it is not on the skip list
              if (skipMap.find((*it_sch).id()) == skipMap.end()) {
                rc = enterSchedule(sch, false, true);
                if (rc == KMyMoneyUtils::Ignore) {
                  skipMap[(*it_sch).id()] = true;
                }
              }
            }

            // reload list (maybe this schedule needs to be added again)
            schedules = file->scheduleList(d->m_selectedAccount.id(), MyMoneySchedule::TYPE_ANY, MyMoneySchedule::OCCUR_ANY, MyMoneySchedule::STYPE_ANY, QDate(), QDate(), true);
          } while (processedOne);
        }
      }

      account = file->account(d->m_selectedAccount.id());
      // get rid of previous run.
      delete d->m_endingBalanceDlg;
      d->m_endingBalanceDlg = new KEndingBalanceDlg(account, this);
      if (account.isAssetLiability()) {
        connect(d->m_endingBalanceDlg, SIGNAL(createPayee(QString,QString&)), this, SLOT(slotPayeeNew(QString,QString&)));
        connect(d->m_endingBalanceDlg, SIGNAL(createCategory(MyMoneyAccount&,MyMoneyAccount)), this, SLOT(slotCategoryNew(MyMoneyAccount&,MyMoneyAccount)));

        if (d->m_endingBalanceDlg->exec() == QDialog::Accepted) {
          if (KMyMoneyGlobalSettings::autoReconciliation()) {
            MyMoneyMoney startBalance = d->m_endingBalanceDlg->previousBalance();
            MyMoneyMoney endBalance = d->m_endingBalanceDlg->endingBalance();
            QDate endDate = d->m_endingBalanceDlg->statementDate();

            QList<QPair<MyMoneyTransaction, MyMoneySplit> > transactionList;
            MyMoneyTransactionFilter filter(account.id());
            filter.addState(MyMoneyTransactionFilter::cleared);
            filter.addState(MyMoneyTransactionFilter::notReconciled);
            filter.setDateFilter(QDate(), endDate);
            filter.setConsiderCategory(false);
            filter.setReportAllSplits(true);
            file->transactionList(transactionList, filter);
            QList<QPair<MyMoneyTransaction, MyMoneySplit> > result = d->automaticReconciliation(account, transactionList, endBalance - startBalance);

            if (!result.empty()) {
              QString message = i18n("KMyMoney has detected transactions matching your reconciliation data.\nWould you like KMyMoney to clear these transactions for you?");
              if (KMessageBox::questionYesNo(this,
                                             message,
                                             i18n("Automatic reconciliation"),
                                             KStandardGuiItem::yes(),
                                             KStandardGuiItem::no(),
                                             "AcceptAutomaticReconciliation") == KMessageBox::Yes) {
                // mark the transactions cleared
                KMyMoneyRegister::SelectedTransactions oldSelection = d->m_selectedTransactions;
                d->m_selectedTransactions.clear();
                QListIterator<QPair<MyMoneyTransaction, MyMoneySplit> > itTransactionSplitResult(result);
                while (itTransactionSplitResult.hasNext()) {
                  const QPair<MyMoneyTransaction, MyMoneySplit> &transactionSplit = itTransactionSplitResult.next();
                  d->m_selectedTransactions.append(KMyMoneyRegister::SelectedTransaction(transactionSplit.first, transactionSplit.second));
                }
                // mark all transactions in d->m_selectedTransactions as 'Cleared'
                markTransaction(MyMoneySplit::Cleared);
                d->m_selectedTransactions = oldSelection;
              }
            }
          }

          if (d->m_myMoneyView->startReconciliation(account, d->m_endingBalanceDlg->statementDate(), d->m_endingBalanceDlg->endingBalance())) {

            // check if the user requests us to create interest
            // or charge transactions.
            MyMoneyTransaction ti = d->m_endingBalanceDlg->interestTransaction();
            MyMoneyTransaction tc = d->m_endingBalanceDlg->chargeTransaction();
            MyMoneyFileTransaction ft;
            try {
              if (ti != MyMoneyTransaction()) {
                MyMoneyFile::instance()->addTransaction(ti);
              }
              if (tc != MyMoneyTransaction()) {
                MyMoneyFile::instance()->addTransaction(tc);
              }
              ft.commit();

            } catch (const MyMoneyException &e) {
              qWarning("interest transaction not stored: '%s'", qPrintable(e.what()));
            }

            // reload the account object as it might have changed in the meantime
            d->m_reconciliationAccount = file->account(account.id());
            slotUpdateActions();
          }
        }
      }
    } catch (const MyMoneyException &) {
    }
  }
}

void KMyMoneyApp::slotAccountReconcileFinish()
{
  MyMoneyFile* file = MyMoneyFile::instance();

  if (!d->m_reconciliationAccount.id().isEmpty()) {
    // retrieve list of all transactions that are not reconciled or cleared
    QList<QPair<MyMoneyTransaction, MyMoneySplit> > transactionList;
    MyMoneyTransactionFilter filter(d->m_reconciliationAccount.id());
    filter.addState(MyMoneyTransactionFilter::cleared);
    filter.addState(MyMoneyTransactionFilter::notReconciled);
    filter.setDateFilter(QDate(), d->m_endingBalanceDlg->statementDate());
    filter.setConsiderCategory(false);
    filter.setReportAllSplits(true);
    file->transactionList(transactionList, filter);

    MyMoneyMoney balance = MyMoneyFile::instance()->balance(d->m_reconciliationAccount.id(), d->m_endingBalanceDlg->statementDate());
    MyMoneyMoney actBalance, clearedBalance;
    actBalance = clearedBalance = balance;

    // walk the list of transactions to figure out the balance(s)
    QList<QPair<MyMoneyTransaction, MyMoneySplit> >::const_iterator it;
    for (it = transactionList.constBegin(); it != transactionList.constEnd(); ++it) {
      if ((*it).second.reconcileFlag() == MyMoneySplit::NotReconciled) {
        clearedBalance -= (*it).second.shares();
      }
    }

    if (d->m_endingBalanceDlg->endingBalance() != clearedBalance) {
      QString message = i18n("You are about to finish the reconciliation of this account with a difference between your bank statement and the transactions marked as cleared.\n"
                             "Are you sure you want to finish the reconciliation?");
      if (KMessageBox::questionYesNo(this, message, i18n("Confirm end of reconciliation"), KStandardGuiItem::yes(), KStandardGuiItem::no()) == KMessageBox::No)
        return;
    }

    MyMoneyFileTransaction ft;

    // refresh object
    d->m_reconciliationAccount = file->account(d->m_reconciliationAccount.id());

    // Turn off reconciliation mode
    d->m_myMoneyView->finishReconciliation(d->m_reconciliationAccount);

    // only update the last statement balance here, if we haven't a newer one due
    // to download of online statements.
    if (d->m_reconciliationAccount.value("lastImportedTransactionDate").isEmpty()
        || QDate::fromString(d->m_reconciliationAccount.value("lastImportedTransactionDate"), Qt::ISODate) < d->m_endingBalanceDlg->statementDate()) {
      d->m_reconciliationAccount.setValue("lastStatementBalance", d->m_endingBalanceDlg->endingBalance().toString());
      // in case we override the last statement balance here, we have to make sure
      // that we don't show the online balance anymore, as it might be different
      d->m_reconciliationAccount.deletePair("lastImportedTransactionDate");
    }
    d->m_reconciliationAccount.setLastReconciliationDate(d->m_endingBalanceDlg->statementDate());

    // keep a record of this reconciliation
    d->m_reconciliationAccount.addReconciliation(d->m_endingBalanceDlg->statementDate(), d->m_endingBalanceDlg->endingBalance());

    d->m_reconciliationAccount.deletePair("lastReconciledBalance");
    d->m_reconciliationAccount.deletePair("statementBalance");
    d->m_reconciliationAccount.deletePair("statementDate");

    try {
      // update the account data
      file->modifyAccount(d->m_reconciliationAccount);

      /*
      // collect the list of cleared splits for this account
      filter.clear();
      filter.addAccount(d->m_reconciliationAccount.id());
      filter.addState(MyMoneyTransactionFilter::cleared);
      filter.setConsiderCategory(false);
      filter.setReportAllSplits(true);
      file->transactionList(transactionList, filter);
      */

      // walk the list of transactions/splits and mark the cleared ones as reconciled
      QList<QPair<MyMoneyTransaction, MyMoneySplit> >::iterator it;

      for (it = transactionList.begin(); it != transactionList.end(); ++it) {
        MyMoneySplit sp = (*it).second;
        // skip the ones that are not marked cleared
        if (sp.reconcileFlag() != MyMoneySplit::Cleared)
          continue;

        // always retrieve a fresh copy of the transaction because we
        // might have changed it already with another split
        MyMoneyTransaction t = file->transaction((*it).first.id());
        sp.setReconcileFlag(MyMoneySplit::Reconciled);
        sp.setReconcileDate(d->m_endingBalanceDlg->statementDate());
        t.modifySplit(sp);

        // update the engine ...
        file->modifyTransaction(t);

        // ... and the list
        (*it) = qMakePair(t, sp);
      }
      ft.commit();

      // reload account data from engine as the data might have changed in the meantime
      d->m_reconciliationAccount = file->account(d->m_reconciliationAccount.id());
      emit accountReconciled(d->m_reconciliationAccount,
                             d->m_endingBalanceDlg->statementDate(),
                             d->m_endingBalanceDlg->previousBalance(),
                             d->m_endingBalanceDlg->endingBalance(),
                             transactionList);

    } catch (const MyMoneyException &) {
      qDebug("Unexpected exception when setting cleared to reconcile");
    }
  }
  // Turn off reconciliation mode
  d->m_reconciliationAccount = MyMoneyAccount();
  slotUpdateActions();
}

void KMyMoneyApp::slotAccountReconcilePostpone()
{
  MyMoneyFileTransaction ft;
  MyMoneyFile* file = MyMoneyFile::instance();

  if (!d->m_reconciliationAccount.id().isEmpty()) {
    // refresh object
    d->m_reconciliationAccount = file->account(d->m_reconciliationAccount.id());

    // Turn off reconciliation mode
    d->m_myMoneyView->finishReconciliation(d->m_reconciliationAccount);

    d->m_reconciliationAccount.setValue("lastReconciledBalance", d->m_endingBalanceDlg->previousBalance().toString());
    d->m_reconciliationAccount.setValue("statementBalance", d->m_endingBalanceDlg->endingBalance().toString());
    d->m_reconciliationAccount.setValue("statementDate", d->m_endingBalanceDlg->statementDate().toString(Qt::ISODate));

    try {
      file->modifyAccount(d->m_reconciliationAccount);
      ft.commit();
      d->m_reconciliationAccount = MyMoneyAccount();
      slotUpdateActions();
    } catch (const MyMoneyException &) {
      qDebug("Unexpected exception when setting last reconcile info into account");
      ft.rollback();
      d->m_reconciliationAccount = file->account(d->m_reconciliationAccount.id());
    }
  }
}

void KMyMoneyApp::slotAccountOpen()
{
  slotAccountOpen(MyMoneyAccount());
}

void KMyMoneyApp::slotAccountOpen(const MyMoneyObject& obj)
{
  if (typeid(obj) != typeid(MyMoneyAccount))
    return;

  MyMoneyFile* file = MyMoneyFile::instance();
  QString id = d->m_selectedAccount.id();

  // if the caller passed a non-empty object, we need to select that
  if (!obj.id().isEmpty()) {
    id = obj.id();
  }

  // we cannot reconcile standard accounts
  if (!file->isStandardAccount(id)) {
    // check if we can open this account
    // currently it make's sense for asset and liability accounts
    try {
      MyMoneyAccount account = file->account(id);
      d->m_myMoneyView->slotLedgerSelected(account.id());
    } catch (const MyMoneyException &) {
    }
  }
}

void KMyMoneyApp::enableCloseAccountAction(const MyMoneyAccount& acc)
{
  QAction *a = actionCollection()->action(s_Actions[Action::AccountClose]);
  switch (canCloseAccount(acc)) {
    case KMyMoneyUtils::AccountCanClose: {
        a->setEnabled(true);
        break;
      }
    case KMyMoneyUtils::AccountBalanceNonZero: {
        a->setEnabled(false);
        a->setToolTip(i18n("The balance of the account must be zero before the account can be closed"));
        break;
      }
    case KMyMoneyUtils::AccountChildrenOpen: {
        a->setEnabled(false);
        a->setToolTip(i18n("All subaccounts must be closed before the account can be closed"));
        break;
      }
    case KMyMoneyUtils::AccountScheduleReference: {
        a->setEnabled(false);
        a->setToolTip(i18n("This account is still included in an active schedule"));
        break;
      }
  }
}


KMyMoneyUtils::CanCloseAccountCodeE KMyMoneyApp::canCloseAccount(const MyMoneyAccount& acc) const
{
  // balance must be zero
  if (!acc.balance().isZero())
    return KMyMoneyUtils::AccountBalanceNonZero;

  // all children must be already closed
  QStringList::const_iterator it_a;
  for (it_a = acc.accountList().constBegin(); it_a != acc.accountList().constEnd(); ++it_a) {
    MyMoneyAccount a = MyMoneyFile::instance()->account(*it_a);
    if (!a.isClosed()) {
      return KMyMoneyUtils::AccountChildrenOpen;
    }
  }

  // there must be no unfinished schedule referencing the account
  QList<MyMoneySchedule> list = MyMoneyFile::instance()->scheduleList();
  QList<MyMoneySchedule>::const_iterator it_l;
  for (it_l = list.constBegin(); it_l != list.constEnd(); ++it_l) {
    if ((*it_l).isFinished())
      continue;
    if ((*it_l).hasReferenceTo(acc.id()))
      return KMyMoneyUtils::AccountScheduleReference;
  }
  return KMyMoneyUtils::AccountCanClose;
}

void KMyMoneyApp::slotAccountClose()
{
  MyMoneyAccount a;
  if (!d->m_selectedInvestment.id().isEmpty())
    a = d->m_selectedInvestment;
  else if (!d->m_selectedAccount.id().isEmpty())
    a = d->m_selectedAccount;
  if (a.id().isEmpty())
    return;  // need an account ID

  MyMoneyFileTransaction ft;
  try {
    a.setClosed(true);
    MyMoneyFile::instance()->modifyAccount(a);
    ft.commit();
    if (KMyMoneyGlobalSettings::hideClosedAccounts()) {
      KMessageBox::information(this, QString("<qt>") + i18n("You have closed this account. It remains in the system because you have transactions which still refer to it, but it is not shown in the views. You can make it visible again by going to the View menu and selecting <b>Show all accounts</b> or by deselecting the <b>Do not show closed accounts</b> setting.") + QString("</qt>"), i18n("Information"), "CloseAccountInfo");
    }
  } catch (const MyMoneyException &) {
  }
}

void KMyMoneyApp::slotAccountReopen()
{
  MyMoneyAccount a;
  if (!d->m_selectedInvestment.id().isEmpty())
    a = d->m_selectedInvestment;
  else if (!d->m_selectedAccount.id().isEmpty())
    a = d->m_selectedAccount;
  if (a.id().isEmpty())
    return;  // need an account ID

  MyMoneyFile* file = MyMoneyFile::instance();
  MyMoneyFileTransaction ft;
  try {
    while (a.isClosed()) {
      a.setClosed(false);
      file->modifyAccount(a);
      a = file->account(a.parentAccountId());
    }
    ft.commit();
  } catch (const MyMoneyException &) {
  }
}

void KMyMoneyApp::slotReparentAccount(const MyMoneyAccount& _src, const MyMoneyInstitution& _dst)
{
  MyMoneyAccount src(_src);
  src.setInstitutionId(_dst.id());
  MyMoneyFileTransaction ft;
  try {
    MyMoneyFile::instance()->modifyAccount(src);
    ft.commit();
  } catch (const MyMoneyException &e) {
    KMessageBox::sorry(this, i18n("<p><b>%1</b> cannot be moved to institution <b>%2</b>. Reason: %3</p>", src.name(), _dst.name(), e.what()));
  }
}

void KMyMoneyApp::slotReparentAccount(const MyMoneyAccount& _src, const MyMoneyAccount& _dst)
{
  MyMoneyAccount src(_src);
  MyMoneyAccount dst(_dst);
  MyMoneyFileTransaction ft;
  try {
    MyMoneyFile::instance()->reparentAccount(src, dst);
    ft.commit();
  } catch (const MyMoneyException &e) {
    KMessageBox::sorry(this, i18n("<p><b>%1</b> cannot be moved to <b>%2</b>. Reason: %3</p>", src.name(), dst.name(), e.what()));
  }
}

void KMyMoneyApp::slotAccountTransactionReport()
{
  // Generate a transaction report that contains transactions for only the
  // currently selected account.
  if (!d->m_selectedAccount.id().isEmpty()) {
    MyMoneyReport report(
      MyMoneyReport::eAccount,
      MyMoneyReport::eQCnumber | MyMoneyReport::eQCpayee | MyMoneyReport::eQCcategory,
      MyMoneyTransactionFilter::yearToDate,
      MyMoneyReport::eDetailAll,
      i18n("%1 YTD Account Transactions", d->m_selectedAccount.name()),
      i18n("Generated Report")
    );
    report.setGroup(i18n("Transactions"));
    report.addAccount(d->m_selectedAccount.id());

    d->m_myMoneyView->slotShowReport(report);
  }
}

void KMyMoneyApp::slotScheduleNew()
{
  slotScheduleNew(MyMoneyTransaction());
}

void KMyMoneyApp::slotScheduleNew(const MyMoneyTransaction& _t, MyMoneySchedule::occurrenceE occurrence)
{
  MyMoneySchedule schedule;
  schedule.setOccurrence(occurrence);

  // if the schedule is based on an existing transaction,
  // we take the post date and project it to the next
  // schedule in a month.
  if (_t != MyMoneyTransaction()) {
    MyMoneyTransaction t(_t);
    schedule.setTransaction(t);
    if (occurrence != MyMoneySchedule::OCCUR_ONCE)
      schedule.setNextDueDate(schedule.nextPayment(t.postDate()));
  }

  QPointer<KEditScheduleDlg> dlg = new KEditScheduleDlg(schedule, this);
  TransactionEditor* transactionEditor = dlg->startEdit();
  if (transactionEditor) {
    KMyMoneyMVCCombo::setSubstringSearchForChildren(dlg, !KMyMoneySettings::stringMatchFromStart());
    if (dlg->exec() == QDialog::Accepted && dlg != 0) {
      MyMoneyFileTransaction ft;
      try {
        schedule = dlg->schedule();
        MyMoneyFile::instance()->addSchedule(schedule);
        ft.commit();

      } catch (const MyMoneyException &e) {
        KMessageBox::error(this, i18n("Unable to add scheduled transaction: %1", e.what()), i18n("Add scheduled transaction"));
      }
    }
  }
  delete transactionEditor;
  delete dlg;
}

void KMyMoneyApp::slotScheduleEdit()
{
  if (!d->m_selectedSchedule.id().isEmpty()) {
    try {
      MyMoneySchedule schedule = MyMoneyFile::instance()->schedule(d->m_selectedSchedule.id());

      KEditScheduleDlg* sched_dlg = 0;
      KEditLoanWizard* loan_wiz = 0;

      switch (schedule.type()) {
        case MyMoneySchedule::TYPE_BILL:
        case MyMoneySchedule::TYPE_DEPOSIT:
        case MyMoneySchedule::TYPE_TRANSFER:
          sched_dlg = new KEditScheduleDlg(schedule, this);
          d->m_transactionEditor = sched_dlg->startEdit();
          if (d->m_transactionEditor) {
            KMyMoneyMVCCombo::setSubstringSearchForChildren(sched_dlg, !KMyMoneySettings::stringMatchFromStart());
            if (sched_dlg->exec() == QDialog::Accepted) {
              MyMoneyFileTransaction ft;
              try {
                MyMoneySchedule sched = sched_dlg->schedule();
                // Check whether the new Schedule Date
                // is at or before the lastPaymentDate
                // If it is, ask the user whether to clear the
                // lastPaymentDate
                const QDate& next = sched.nextDueDate();
                const QDate& last = sched.lastPayment();
                if (next.isValid() && last.isValid() && next <= last) {
                  // Entered a date effectively no later
                  // than previous payment.  Date would be
                  // updated automatically so we probably
                  // want to clear it.  Let's ask the user.
                  if (KMessageBox::questionYesNo(this, QString("<qt>") + i18n("You have entered a scheduled transaction date of <b>%1</b>.  Because the scheduled transaction was last paid on <b>%2</b>, KMyMoney will automatically adjust the scheduled transaction date to the next date unless the last payment date is reset.  Do you want to reset the last payment date?", QLocale().toString(next, QLocale::ShortFormat), QLocale().toString(last, QLocale::ShortFormat)) + QString("</qt>"), i18n("Reset Last Payment Date"), KStandardGuiItem::yes(), KStandardGuiItem::no()) == KMessageBox::Yes) {
                    sched.setLastPayment(QDate());
                  }
                }
                MyMoneyFile::instance()->modifySchedule(sched);
                // delete the editor before we emit the dataChanged() signal from the
                // engine. Calling this twice in a row does not hurt.
                deleteTransactionEditor();
                ft.commit();
              } catch (const MyMoneyException &e) {
                KMessageBox::detailedSorry(this, i18n("Unable to modify scheduled transaction '%1'", d->m_selectedSchedule.name()), e.what());
              }
            }
            deleteTransactionEditor();
          }
          delete sched_dlg;
          break;

        case MyMoneySchedule::TYPE_LOANPAYMENT:
          loan_wiz = new KEditLoanWizard(schedule.account(2));
          connect(loan_wiz, SIGNAL(newCategory(MyMoneyAccount&)), this, SLOT(slotCategoryNew(MyMoneyAccount&)));
          connect(loan_wiz, SIGNAL(createPayee(QString,QString&)), this, SLOT(slotPayeeNew(QString,QString&)));
          if (loan_wiz->exec() == QDialog::Accepted) {
            MyMoneyFileTransaction ft;
            try {
              MyMoneyFile::instance()->modifySchedule(loan_wiz->schedule());
              MyMoneyFile::instance()->modifyAccount(loan_wiz->account());
              ft.commit();
            } catch (const MyMoneyException &e) {
              KMessageBox::detailedSorry(this, i18n("Unable to modify scheduled transaction '%1'", d->m_selectedSchedule.name()), e.what());
            }
          }
          delete loan_wiz;
          break;

        case MyMoneySchedule::TYPE_ANY:
          break;
      }

    } catch (const MyMoneyException &e) {
      KMessageBox::detailedSorry(this, i18n("Unable to modify scheduled transaction '%1'", d->m_selectedSchedule.name()), e.what());
    }
  }
}

void KMyMoneyApp::slotScheduleDelete()
{
  if (!d->m_selectedSchedule.id().isEmpty()) {
    MyMoneyFileTransaction ft;
    try {
      MyMoneySchedule sched = MyMoneyFile::instance()->schedule(d->m_selectedSchedule.id());
      QString msg = i18n("<p>Are you sure you want to delete the scheduled transaction <b>%1</b>?</p>", d->m_selectedSchedule.name());
      if (sched.type() == MyMoneySchedule::TYPE_LOANPAYMENT) {
        msg += QString(" ");
        msg += i18n("In case of loan payments it is currently not possible to recreate the scheduled transaction.");
      }
      if (KMessageBox::questionYesNo(this, msg) == KMessageBox::No)
        return;

      MyMoneyFile::instance()->removeSchedule(sched);
      ft.commit();

    } catch (const MyMoneyException &e) {
      KMessageBox::detailedSorry(this, i18n("Unable to remove scheduled transaction '%1'", d->m_selectedSchedule.name()), e.what());
    }
  }
}

void KMyMoneyApp::slotScheduleDuplicate()
{
  // since we may jump here via code, we have to make sure to react only
  // if the action is enabled
  if (kmymoney->actionCollection()->action(s_Actions[Action::ScheduleDuplicate])->isEnabled()) {
    MyMoneySchedule sch = d->m_selectedSchedule;
    sch.clearId();
    sch.setLastPayment(QDate());
    sch.setName(i18nc("Copy of scheduled transaction name", "Copy of %1", sch.name()));
    // make sure that we set a valid next due date if the original next due date is invalid
    if (!sch.nextDueDate().isValid())
      sch.setNextDueDate(QDate::currentDate());

    MyMoneyFileTransaction ft;
    try {
      MyMoneyFile::instance()->addSchedule(sch);
      ft.commit();

      // select the new schedule in the view
      if (!d->m_selectedSchedule.id().isEmpty())
        d->m_myMoneyView->slotScheduleSelected(sch.id());

    } catch (const MyMoneyException &e) {
      KMessageBox::detailedSorry(0, i18n("Unable to duplicate scheduled transaction: '%1'", d->m_selectedSchedule.name()), e.what());
    }
  }
}

void KMyMoneyApp::slotScheduleSkip()
{
  if (!d->m_selectedSchedule.id().isEmpty()) {
    try {
      MyMoneySchedule schedule = MyMoneyFile::instance()->schedule(d->m_selectedSchedule.id());
      skipSchedule(schedule);
    } catch (const MyMoneyException &e) {
      KMessageBox::detailedSorry(this, i18n("Unknown scheduled transaction '%1'", d->m_selectedSchedule.name()), e.what());
    }
  }
}

void KMyMoneyApp::skipSchedule(MyMoneySchedule& schedule)
{
  if (!schedule.id().isEmpty()) {
    try {
      schedule = MyMoneyFile::instance()->schedule(schedule.id());
      if (!schedule.isFinished()) {
        if (schedule.occurrence() != MyMoneySchedule::OCCUR_ONCE) {
          QDate next = schedule.nextDueDate();
          if (!schedule.isFinished() && (KMessageBox::questionYesNo(this, QString("<qt>") + i18n("Do you really want to skip the <b>%1</b> transaction scheduled for <b>%2</b>?", schedule.name(), QLocale().toString(next, QLocale::ShortFormat)) + QString("</qt>"))) == KMessageBox::Yes) {
            MyMoneyFileTransaction ft;
            schedule.setLastPayment(next);
            schedule.setNextDueDate(schedule.nextPayment(next));
            MyMoneyFile::instance()->modifySchedule(schedule);
            ft.commit();
          }
        }
      }
    } catch (const MyMoneyException &e) {
      KMessageBox::detailedSorry(this, QString("<qt>") + i18n("Unable to skip scheduled transaction <b>%1</b>.", schedule.name()) + QString("</qt>"), e.what());
    }
  }
}

void KMyMoneyApp::slotScheduleEnter()
{
  if (!d->m_selectedSchedule.id().isEmpty()) {
    try {
      MyMoneySchedule schedule = MyMoneyFile::instance()->schedule(d->m_selectedSchedule.id());
      enterSchedule(schedule);
    } catch (const MyMoneyException &e) {
      KMessageBox::detailedSorry(this, i18n("Unknown scheduled transaction '%1'", d->m_selectedSchedule.name()), e.what());
    }
  }
}

KMyMoneyUtils::EnterScheduleResultCodeE KMyMoneyApp::enterSchedule(MyMoneySchedule& schedule, bool autoEnter, bool extendedKeys)
{
  KMyMoneyUtils::EnterScheduleResultCodeE rc = KMyMoneyUtils::Cancel;
  if (!schedule.id().isEmpty()) {
    try {
      schedule = MyMoneyFile::instance()->schedule(schedule.id());
    } catch (const MyMoneyException &e) {
      KMessageBox::detailedSorry(this, i18n("Unable to enter scheduled transaction '%1'", schedule.name()), e.what());
      return rc;
    }

    QPointer<KEnterScheduleDlg> dlg = new KEnterScheduleDlg(this, schedule);

    try {
      QDate origDueDate = schedule.nextDueDate();

      dlg->showExtendedKeys(extendedKeys);

      d->m_transactionEditor = dlg->startEdit();
      if (d->m_transactionEditor) {
        KMyMoneyMVCCombo::setSubstringSearchForChildren(dlg, !KMyMoneySettings::stringMatchFromStart());
        MyMoneyTransaction torig, taccepted;
        d->m_transactionEditor->createTransaction(torig, dlg->transaction(),
            schedule.transaction().splits().isEmpty() ? MyMoneySplit() : schedule.transaction().splits().front(), true);
        // force actions to be available no matter what (will be updated according to the state during
        // slotTransactionsEnter or slotTransactionsCancel)
        kmymoney->actionCollection()->action(s_Actions[Action::TransactionCancel])->setEnabled(true);
        kmymoney->actionCollection()->action(s_Actions[Action::TransactionEnter])->setEnabled(true);

        KConfirmManualEnterDlg::Action action = KConfirmManualEnterDlg::ModifyOnce;
        if (!autoEnter || !schedule.isFixed()) {
          for (; dlg != 0;) {
            rc = KMyMoneyUtils::Cancel;
            if (dlg->exec() == QDialog::Accepted && dlg != 0) {
              rc = dlg->resultCode();
              if (rc == KMyMoneyUtils::Enter) {
                d->m_transactionEditor->createTransaction(taccepted, torig, torig.splits().isEmpty() ? MyMoneySplit() : torig.splits().front(), true);
                // make sure to suppress comparison of some data: postDate
                torig.setPostDate(taccepted.postDate());
                if (torig != taccepted) {
                  QPointer<KConfirmManualEnterDlg> cdlg =
                    new KConfirmManualEnterDlg(schedule, this);
                  cdlg->loadTransactions(torig, taccepted);
                  if (cdlg->exec() == QDialog::Accepted) {
                    action = cdlg->action();
                    delete cdlg;
                    break;
                  }
                  delete cdlg;
                  // the user has chosen 'cancel' during confirmation,
                  // we go back to the editor
                  continue;
                }
              } else if (rc == KMyMoneyUtils::Skip) {
                slotTransactionsCancel();
                skipSchedule(schedule);
              } else {
                slotTransactionsCancel();
              }
            } else {
              if (autoEnter) {
                if (KMessageBox::warningYesNo(this, i18n("Are you sure you wish to stop this scheduled transaction from being entered into the register?\n\nKMyMoney will prompt you again next time it starts unless you manually enter it later.")) == KMessageBox::No) {
                  // the user has chosen 'No' for the above question,
                  // we go back to the editor
                  continue;
                }
              }
              slotTransactionsCancel();
            }
            break;
          }
        }

        // if we still have the editor around here, the user did not cancel
        if ((d->m_transactionEditor != 0) && (dlg != 0)) {
          MyMoneyFileTransaction ft;
          try {
            MyMoneyTransaction t;
            // add the new transaction
            switch (action) {
              case KConfirmManualEnterDlg::UseOriginal:
                // setup widgets with original transaction data
                d->m_transactionEditor->setTransaction(dlg->transaction(), dlg->transaction().splits().isEmpty() ? MyMoneySplit() : dlg->transaction().splits().front());
                // and create a transaction based on that data
                taccepted = MyMoneyTransaction();
                d->m_transactionEditor->createTransaction(taccepted, dlg->transaction(),
                    dlg->transaction().splits().isEmpty() ? MyMoneySplit() : dlg->transaction().splits().front(), true);
                break;

              case KConfirmManualEnterDlg::ModifyAlways:
                torig = taccepted;
                torig.setPostDate(origDueDate);
                schedule.setTransaction(torig);
                break;

              case KConfirmManualEnterDlg::ModifyOnce:
                break;
            }

            QString newId;
            connect(d->m_transactionEditor, SIGNAL(balanceWarning(QWidget*,MyMoneyAccount,QString)), d->m_balanceWarning, SLOT(slotShowMessage(QWidget*,MyMoneyAccount,QString)));
            if (d->m_transactionEditor->enterTransactions(newId, false)) {
              if (!newId.isEmpty()) {
                MyMoneyTransaction t = MyMoneyFile::instance()->transaction(newId);
                schedule.setLastPayment(t.postDate());
              }
              // in case the next due date is invalid, the schedule is finished
              // we mark it as such by setting the next due date to one day past the end
              QDate nextDueDate = schedule.nextPayment(origDueDate);
              if (!nextDueDate.isValid()) {
                schedule.setNextDueDate(schedule.endDate().addDays(1));
              } else {
                schedule.setNextDueDate(nextDueDate);
              }
              MyMoneyFile::instance()->modifySchedule(schedule);
              rc = KMyMoneyUtils::Enter;

              // delete the editor before we emit the dataChanged() signal from the
              // engine. Calling this twice in a row does not hurt.
              deleteTransactionEditor();
              ft.commit();
            }
          } catch (const MyMoneyException &e) {
            KMessageBox::detailedSorry(this, i18n("Unable to enter scheduled transaction '%1'", schedule.name()), e.what());
          }
          deleteTransactionEditor();
        }
      }
    } catch (const MyMoneyException &e) {
      KMessageBox::detailedSorry(this, i18n("Unable to enter scheduled transaction '%1'", schedule.name()), e.what());
    }
    delete dlg;
  }
  return rc;
}

bool KMyMoneyApp::slotPayeeNew(const QString& newnameBase, QString& id)
{
  bool doit = true;

  if (newnameBase != i18n("New Payee")) {
    // Ask the user if that is what he intended to do?
    QString msg = QLatin1String("<qt>") + i18n("Do you want to add <b>%1</b> as payer/receiver?", newnameBase) + QLatin1String("</qt>");

    if (KMessageBox::questionYesNo(this, msg, i18n("New payee/receiver"), KStandardGuiItem::yes(), KStandardGuiItem::no(), "NewPayee") == KMessageBox::No) {
      doit = false;
      // we should not keep the 'no' setting because that can confuse people like
      // I have seen in some usability tests. So we just delete it right away.
      KSharedConfigPtr kconfig = KSharedConfig::openConfig();
      if (kconfig) {
        kconfig->group(QLatin1String("Notification Messages")).deleteEntry(QLatin1String("NewPayee"));
      }
    }
  }

  if (doit) {
    MyMoneyFileTransaction ft;
    try {
      QString newname(newnameBase);
      // adjust name until a unique name has been created
      int count = 0;
      for (;;) {
        try {
          MyMoneyFile::instance()->payeeByName(newname);
          newname = QString("%1 [%2]").arg(newnameBase).arg(++count);
        } catch (const MyMoneyException &) {
          break;
        }
      }

      MyMoneyPayee p;
      p.setName(newname);
      MyMoneyFile::instance()->addPayee(p);
      id = p.id();
      ft.commit();
    } catch (const MyMoneyException &e) {
      KMessageBox::detailedSorry(this, i18n("Unable to add payee"),
                                 i18n("%1 thrown in %2:%3", e.what(), e.file(), e.line()));
      doit = false;
    }
  }
  return doit;
}

void KMyMoneyApp::slotPayeeNew()
{
  QString id;
  slotPayeeNew(i18n("New Payee"), id);

  // the callbacks should have made sure, that the payees view has been
  // updated already. So we search for the id in the list of items
  // and select it.
  emit payeeCreated(id);
}

bool KMyMoneyApp::payeeInList(const QList<MyMoneyPayee>& list, const QString& id) const
{
  bool rc = false;
  QList<MyMoneyPayee>::const_iterator it_p = list.begin();
  while (it_p != list.end()) {
    if ((*it_p).id() == id) {
      rc = true;
      break;
    }
    ++it_p;
  }
  return rc;
}

void KMyMoneyApp::slotPayeeDelete()
{
  if (d->m_selectedPayees.isEmpty())
    return; // shouldn't happen

  // get confirmation from user
  QString prompt;
  if (d->m_selectedPayees.size() == 1)
    prompt = i18n("<p>Do you really want to remove the payee <b>%1</b>?</p>", d->m_selectedPayees.front().name());
  else
    prompt = i18n("Do you really want to remove all selected payees?");

  if (KMessageBox::questionYesNo(this, prompt, i18n("Remove Payee")) == KMessageBox::No)
    return;

  payeeReassign(KPayeeReassignDlg::TypeDelete);
}

void KMyMoneyApp::slotPayeeMerge()
{
  if (d->m_selectedPayees.size() < 1)
    return; // shouldn't happen

  if (KMessageBox::questionYesNo(this, i18n("<p>Do you really want to merge the selected payees?"),
                                 i18n("Merge Payees")) == KMessageBox::No)
    return;

  if (payeeReassign(KPayeeReassignDlg::TypeMerge))
    // clean selection since we just deleted the selected payees
    slotSelectPayees(QList<MyMoneyPayee>());
}

bool KMyMoneyApp::payeeReassign(int type)
{
  if (!(type >= 0 && type < KPayeeReassignDlg::TypeCount))
    return false;

  MyMoneyFile * file = MyMoneyFile::instance();

  MyMoneyFileTransaction ft;
  try {
    // create a transaction filter that contains all payees selected for removal
    MyMoneyTransactionFilter f = MyMoneyTransactionFilter();
    for (QList<MyMoneyPayee>::const_iterator it = d->m_selectedPayees.constBegin();
         it != d->m_selectedPayees.constEnd(); ++it) {
      f.addPayee((*it).id());
    }
    // request a list of all transactions that still use the payees in question
    QList<MyMoneyTransaction> translist = file->transactionList(f);
//     qDebug() << "[KPayeesView::slotDeletePayee]  " << translist.count() << " transaction still assigned to payees";

    // now get a list of all schedules that make use of one of the payees
    QList<MyMoneySchedule> all_schedules = file->scheduleList();
    QList<MyMoneySchedule> used_schedules;
    for (QList<MyMoneySchedule>::ConstIterator it = all_schedules.constBegin();
         it != all_schedules.constEnd(); ++it) {
      // loop over all splits in the transaction of the schedule
      for (QList<MyMoneySplit>::ConstIterator s_it = (*it).transaction().splits().constBegin();
           s_it != (*it).transaction().splits().constEnd(); ++s_it) {
        // is the payee in the split to be deleted?
        if (payeeInList(d->m_selectedPayees, (*s_it).payeeId())) {
          used_schedules.push_back(*it); // remember this schedule
          break;
        }
      }
    }
//     qDebug() << "[KPayeesView::slotDeletePayee]  " << used_schedules.count() << " schedules use one of the selected payees";

    // and a list of all loan accounts that references one of the payees
    QList<MyMoneyAccount> allAccounts;
    QList<MyMoneyAccount> usedAccounts;
    file->accountList(allAccounts);
    foreach (const MyMoneyAccount &account, allAccounts) {
      if (account.isLoan()) {
        MyMoneyAccountLoan loanAccount(account);
        foreach (const MyMoneyPayee &payee, d->m_selectedPayees) {
          if (loanAccount.hasReferenceTo(payee.id())) {
            usedAccounts.append(account);
          }
        }
      }
    }


    MyMoneyPayee newPayee;
    bool addToMatchList = false;
    // if at least one payee is still referenced, we need to reassign its transactions first
    if (!translist.isEmpty() || !used_schedules.isEmpty() || !usedAccounts.isEmpty()) {

      // first create list with all non-selected payees
      QList<MyMoneyPayee> remainingPayees;
      if (type == KPayeeReassignDlg::TypeMerge) {
        remainingPayees = d->m_selectedPayees;
      } else {
        remainingPayees = file->payeeList();
        QList<MyMoneyPayee>::iterator it_p;
        for (it_p = remainingPayees.begin(); it_p != remainingPayees.end();) {
          if (d->m_selectedPayees.contains(*it_p)) {
            it_p = remainingPayees.erase(it_p);
          } else {
            ++it_p;
          }
        }
      }

      // show error message if no payees remain
      if (remainingPayees.isEmpty()) {
        KMessageBox::sorry(this, i18n("At least one transaction/scheduled transaction or loan account is still referenced by a payee. "
                                      "Currently you have all payees selected. However, at least one payee must remain so "
                                      "that the transaction/scheduled transaction or loan account can be reassigned."));
        return false;
      }

      // show transaction reassignment dialog
      KPayeeReassignDlg * dlg = new KPayeeReassignDlg(static_cast<KPayeeReassignDlg::OperationType>(type), this);
      KMyMoneyMVCCombo::setSubstringSearchForChildren(dlg, !KMyMoneySettings::stringMatchFromStart());
      QString payee_id = dlg->show(remainingPayees);
      addToMatchList = dlg->addToMatchList();
      delete dlg; // and kill the dialog
      if (payee_id.isEmpty())
        return false; // the user aborted the dialog, so let's abort as well

      // try to get selected payee. If not possible and we are merging payees,
      // then we create a new one
      try {
        newPayee = file->payee(payee_id);
      } catch (const MyMoneyException &e) {
        if (type == KPayeeReassignDlg::TypeMerge) {
          // it's ok to use payee_id for both arguments since the first is const,
          // so it's garantee not to change its content
          if (!slotPayeeNew(payee_id, payee_id))
            return false; // the user aborted the dialog, so let's abort as well
          newPayee = file->payee(payee_id);
        } else {
          return false;
        }
      }

      // TODO : check if we have a report that explicitively uses one of our payees
      //        and issue an appropriate warning
      try {
        QList<MyMoneySplit>::iterator s_it;
        // now loop over all transactions and reassign payee
        for (QList<MyMoneyTransaction>::iterator it = translist.begin(); it != translist.end(); ++it) {
          // create a copy of the splits list in the transaction
          QList<MyMoneySplit> splits = (*it).splits();
          // loop over all splits
          for (s_it = splits.begin(); s_it != splits.end(); ++s_it) {
            // if the split is assigned to one of the selected payees, we need to modify it
            if (payeeInList(d->m_selectedPayees, (*s_it).payeeId())) {
              (*s_it).setPayeeId(payee_id); // first modify payee in current split
              // then modify the split in our local copy of the transaction list
              (*it).modifySplit(*s_it); // this does not modify the list object 'splits'!
            }
          } // for - Splits
          file->modifyTransaction(*it);  // modify the transaction in the MyMoney object
        } // for - Transactions

        // now loop over all schedules and reassign payees
        for (QList<MyMoneySchedule>::iterator it = used_schedules.begin();
             it != used_schedules.end(); ++it) {
          // create copy of transaction in current schedule
          MyMoneyTransaction trans = (*it).transaction();
          // create copy of lists of splits
          QList<MyMoneySplit> splits = trans.splits();
          for (s_it = splits.begin(); s_it != splits.end(); ++s_it) {
            if (payeeInList(d->m_selectedPayees, (*s_it).payeeId())) {
              (*s_it).setPayeeId(payee_id);
              trans.modifySplit(*s_it); // does not modify the list object 'splits'!
            }
          } // for - Splits
          // store transaction in current schedule
          (*it).setTransaction(trans);
          file->modifySchedule(*it);  // modify the schedule in the MyMoney engine
        } // for - Schedules

        // reassign the payees in the loans that reference the deleted payees
        foreach (const MyMoneyAccount &account, usedAccounts) {
          MyMoneyAccountLoan loanAccount(account);
          loanAccount.setPayee(payee_id);
          file->modifyAccount(loanAccount);
        }

      } catch (const MyMoneyException &e) {
        KMessageBox::detailedSorry(0, i18n("Unable to reassign payee of transaction/split"),
                                   i18n("%1 thrown in %2:%3", e.what(), e.file(), e.line()));
      }
    } else { // if !translist.isEmpty()
      if (type == KPayeeReassignDlg::TypeMerge) {
        KMessageBox::sorry(this, i18n("Nothing to merge."), i18n("Merge Payees"));
        return false;
      }
    }

    bool ignorecase;
    QStringList payeeNames;
    MyMoneyPayee::payeeMatchType matchType = newPayee.matchData(ignorecase, payeeNames);
    QStringList deletedPayeeNames;

    // now loop over all selected payees and remove them
    for (QList<MyMoneyPayee>::iterator it = d->m_selectedPayees.begin();
         it != d->m_selectedPayees.end(); ++it) {
      if (newPayee.id() != (*it).id()) {
        if (addToMatchList) {
          deletedPayeeNames << (*it).name();
        }
        file->removePayee(*it);
      }
    }

    // if we initially have no matching turned on, we just ignore the case (default)
    if (matchType == MyMoneyPayee::matchDisabled)
      ignorecase = true;

    // update the destination payee if this was requested by the user
    if (addToMatchList && deletedPayeeNames.count() > 0) {
      // add new names to the list
      // TODO: it would be cool to somehow shrink the list to make better use
      //       of regular expressions at this point. For now, we leave this task
      //       to the user himeself.
      QStringList::const_iterator it_n;
      for (it_n = deletedPayeeNames.constBegin(); it_n != deletedPayeeNames.constEnd(); ++it_n) {
        if (matchType == MyMoneyPayee::matchKey) {
          // make sure we really need it and it is not caught by an existing regexp
          QStringList::const_iterator it_k;
          for (it_k = payeeNames.constBegin(); it_k != payeeNames.constEnd(); ++it_k) {
            QRegExp exp(*it_k, ignorecase ? Qt::CaseInsensitive : Qt::CaseSensitive);
            if (exp.indexIn(*it_n) != -1)
              break;
          }
          if (it_k == payeeNames.constEnd())
            payeeNames << QRegExp::escape(*it_n);
        } else if (payeeNames.contains(*it_n) == 0)
          payeeNames << QRegExp::escape(*it_n);
      }

      // and update the payee in the engine context
      // make sure to turn on matching for this payee in the right mode
      newPayee.setMatchData(MyMoneyPayee::matchKey, ignorecase, payeeNames);
      file->modifyPayee(newPayee);
    }
    ft.commit();

    // If we just deleted the payees, they sure don't exist anymore
    slotSelectPayees(QList<MyMoneyPayee>());

  } catch (const MyMoneyException &e) {
    KMessageBox::detailedSorry(0, i18n("Unable to remove payee(s)"),
                               i18n("%1 thrown in %2:%3", e.what(), e.file(), e.line()));
  }

  return true;
}

void KMyMoneyApp::slotTagNew(const QString& newnameBase, QString& id)
{
  bool doit = true;

  if (newnameBase != i18n("New Tag")) {
    // Ask the user if that is what he intended to do?
    QString msg = QString("<qt>") + i18n("Do you want to add <b>%1</b> as tag?", newnameBase) + QString("</qt>");

    if (KMessageBox::questionYesNo(this, msg, i18n("New tag"), KStandardGuiItem::yes(), KStandardGuiItem::no(), "NewTag") == KMessageBox::No) {
      doit = false;
      // we should not keep the 'no' setting because that can confuse people like
      // I have seen in some usability tests. So we just delete it right away.
      KSharedConfigPtr kconfig = KSharedConfig::openConfig();
      if (kconfig) {
        kconfig->group(QLatin1String("Notification Messages")).deleteEntry(QLatin1String("NewTag"));
      }
    }
  }

  if (doit) {
    MyMoneyFileTransaction ft;
    try {
      QString newname(newnameBase);
      // adjust name until a unique name has been created
      int count = 0;
      for (;;) {
        try {
          MyMoneyFile::instance()->tagByName(newname);
          newname = QString("%1 [%2]").arg(newnameBase).arg(++count);
        } catch (const MyMoneyException &) {
          break;
        }
      }

      MyMoneyTag ta;
      ta.setName(newname);
      MyMoneyFile::instance()->addTag(ta);
      id = ta.id();
      ft.commit();
    } catch (const MyMoneyException &e) {
      KMessageBox::detailedSorry(this, i18n("Unable to add tag"),
                                 i18n("%1 thrown in %2:%3", e.what(), e.file(), e.line()));
    }
  }
}

void KMyMoneyApp::slotTagNew()
{
  QString id;
  slotTagNew(i18n("New Tag"), id);

  // the callbacks should have made sure, that the tags view has been
  // updated already. So we search for the id in the list of items
  // and select it.
  emit tagCreated(id);
}

bool KMyMoneyApp::tagInList(const QList<MyMoneyTag>& list, const QString& id) const
{
  bool rc = false;
  QList<MyMoneyTag>::const_iterator it_p = list.begin();
  while (it_p != list.end()) {
    if ((*it_p).id() == id) {
      rc = true;
      break;
    }
    ++it_p;
  }
  return rc;
}

void KMyMoneyApp::slotTagDelete()
{
  if (d->m_selectedTags.isEmpty())
    return; // shouldn't happen

  MyMoneyFile * file = MyMoneyFile::instance();

  // first create list with all non-selected tags
  QList<MyMoneyTag> remainingTags = file->tagList();
  QList<MyMoneyTag>::iterator it_ta;
  for (it_ta = remainingTags.begin(); it_ta != remainingTags.end();) {
    if (d->m_selectedTags.contains(*it_ta)) {
      it_ta = remainingTags.erase(it_ta);
    } else {
      ++it_ta;
    }
  }

  // get confirmation from user
  QString prompt;
  if (d->m_selectedTags.size() == 1)
    prompt = i18n("<p>Do you really want to remove the tag <b>%1</b>?</p>", d->m_selectedTags.front().name());
  else
    prompt = i18n("Do you really want to remove all selected tags?");

  if (KMessageBox::questionYesNo(this, prompt, i18n("Remove Tag")) == KMessageBox::No)
    return;

  MyMoneyFileTransaction ft;
  try {
    // create a transaction filter that contains all tags selected for removal
    MyMoneyTransactionFilter f = MyMoneyTransactionFilter();
    for (QList<MyMoneyTag>::const_iterator it = d->m_selectedTags.constBegin();
         it != d->m_selectedTags.constEnd(); ++it) {
      f.addTag((*it).id());
    }
    // request a list of all transactions that still use the tags in question
    QList<MyMoneyTransaction> translist = file->transactionList(f);
//     qDebug() << "[KTagsView::slotDeleteTag]  " << translist.count() << " transaction still assigned to tags";

    // now get a list of all schedules that make use of one of the tags
    QList<MyMoneySchedule> all_schedules = file->scheduleList();
    QList<MyMoneySchedule> used_schedules;
    for (QList<MyMoneySchedule>::ConstIterator it = all_schedules.constBegin();
         it != all_schedules.constEnd(); ++it) {
      // loop over all splits in the transaction of the schedule
      for (QList<MyMoneySplit>::ConstIterator s_it = (*it).transaction().splits().constBegin();
           s_it != (*it).transaction().splits().constEnd(); ++s_it) {
        for (int i = 0; i < (*s_it).tagIdList().size(); i++) {
          // is the tag in the split to be deleted?
          if (tagInList(d->m_selectedTags, (*s_it).tagIdList()[i])) {
            used_schedules.push_back(*it); // remember this schedule
            break;
          }
        }
      }
    }
//     qDebug() << "[KTagsView::slotDeleteTag]  " << used_schedules.count() << " schedules use one of the selected tags";

    MyMoneyTag newTag;
    // if at least one tag is still referenced, we need to reassign its transactions first
    if (!translist.isEmpty() || !used_schedules.isEmpty()) {
      // show error message if no tags remain
      //FIXME-ALEX Tags are optional so we can delete all of them and simply delete every tagId from every transaction
      if (remainingTags.isEmpty()) {
        KMessageBox::sorry(this, i18n("At least one transaction/scheduled transaction is still referenced by a tag. "
                                      "Currently you have all tags selected. However, at least one tag must remain so "
                                      "that the transaction/scheduled transaction can be reassigned."));
        return;
      }

      // show transaction reassignment dialog
      KTagReassignDlg * dlg = new KTagReassignDlg(this);
      KMyMoneyMVCCombo::setSubstringSearchForChildren(dlg, !KMyMoneySettings::stringMatchFromStart());
      QString tag_id = dlg->show(remainingTags);
      delete dlg; // and kill the dialog
      if (tag_id.isEmpty())  //FIXME-ALEX Let the user choose to not reassign a to-be deleted tag to another one.
        return; // the user aborted the dialog, so let's abort as well

      newTag = file->tag(tag_id);

      // TODO : check if we have a report that explicitively uses one of our tags
      //        and issue an appropriate warning
      try {
        QList<MyMoneySplit>::iterator s_it;
        // now loop over all transactions and reassign tag
        for (QList<MyMoneyTransaction>::iterator it = translist.begin(); it != translist.end(); ++it) {
          // create a copy of the splits list in the transaction
          QList<MyMoneySplit> splits = (*it).splits();
          // loop over all splits
          for (s_it = splits.begin(); s_it != splits.end(); ++s_it) {
            QList<QString> tagIdList = (*s_it).tagIdList();
            for (int i = 0; i < tagIdList.size(); i++) {
              // if the split is assigned to one of the selected tags, we need to modify it
              if (tagInList(d->m_selectedTags, tagIdList[i])) {
                tagIdList.removeAt(i);
                if (tagIdList.indexOf(tag_id) == -1)
                  tagIdList.append(tag_id);
                i = -1; // restart from the first element
              }
            }
            (*s_it).setTagIdList(tagIdList); // first modify tag list in current split
            // then modify the split in our local copy of the transaction list
            (*it).modifySplit(*s_it); // this does not modify the list object 'splits'!
          } // for - Splits
          file->modifyTransaction(*it);  // modify the transaction in the MyMoney object
        } // for - Transactions

        // now loop over all schedules and reassign tags
        for (QList<MyMoneySchedule>::iterator it = used_schedules.begin();
             it != used_schedules.end(); ++it) {
          // create copy of transaction in current schedule
          MyMoneyTransaction trans = (*it).transaction();
          // create copy of lists of splits
          QList<MyMoneySplit> splits = trans.splits();
          for (s_it = splits.begin(); s_it != splits.end(); ++s_it) {
            QList<QString> tagIdList = (*s_it).tagIdList();
            for (int i = 0; i < tagIdList.size(); i++) {
              if (tagInList(d->m_selectedTags, tagIdList[i])) {
                tagIdList.removeAt(i);
                if (tagIdList.indexOf(tag_id) == -1)
                  tagIdList.append(tag_id);
                i = -1; // restart from the first element
              }
            }
            (*s_it).setTagIdList(tagIdList);
            trans.modifySplit(*s_it); // does not modify the list object 'splits'!
          } // for - Splits
          // store transaction in current schedule
          (*it).setTransaction(trans);
          file->modifySchedule(*it);  // modify the schedule in the MyMoney engine
        } // for - Schedules

      } catch (const MyMoneyException &e) {
        KMessageBox::detailedSorry(0, i18n("Unable to reassign tag of transaction/split"),
                                   i18n("%1 thrown in %2:%3", e.what(), e.file(), e.line()));
      }
    } // if !translist.isEmpty()

    // now loop over all selected tags and remove them
    for (QList<MyMoneyTag>::iterator it = d->m_selectedTags.begin();
         it != d->m_selectedTags.end(); ++it) {
      file->removeTag(*it);
    }

    ft.commit();

    // If we just deleted the tags, they sure don't exist anymore
    slotSelectTags(QList<MyMoneyTag>());

  } catch (const MyMoneyException &e) {
    KMessageBox::detailedSorry(0, i18n("Unable to remove tag(s)"),
                               i18n("%1 thrown in %2:%3", e.what(), e.file(), e.line()));
  }
}


void KMyMoneyApp::slotCurrencyNew()
{
  QString sid = QInputDialog::getText(0, i18n("New currency"), i18n("Enter ISO 4217 code for the new currency"));
  if (!sid.isEmpty()) {
    QString id(sid);
    MyMoneySecurity currency(id, i18n("New currency"));
    MyMoneyFileTransaction ft;
    try {
      MyMoneyFile::instance()->addCurrency(currency);
      ft.commit();
    } catch (const MyMoneyException &e) {
      KMessageBox::sorry(this, i18n("Cannot create new currency. %1", e.what()), i18n("New currency"));
    }
    emit currencyCreated(id);
  }
}

void KMyMoneyApp::slotCurrencyUpdate(const QString &currencyId, const QString& currencyName, const QString& currencyTradingSymbol)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  try {
    if (currencyName != d->m_selectedCurrency.name() || currencyTradingSymbol != d->m_selectedCurrency.tradingSymbol()) {
      MyMoneySecurity currency = file->currency(currencyId);
      currency.setName(currencyName);
      currency.setTradingSymbol(currencyTradingSymbol);
      MyMoneyFileTransaction ft;
      try {
        file->modifyCurrency(currency);
        d->m_selectedCurrency = currency;
        ft.commit();
      } catch (const MyMoneyException &e) {
        KMessageBox::sorry(this, i18n("Cannot update currency. %1", e.what()), i18n("Update currency"));
      }
    }
  } catch (const MyMoneyException &e) {
    KMessageBox::sorry(this, i18n("Cannot update currency. %1", e.what()), i18n("Update currency"));
  }
}

void KMyMoneyApp::slotCurrencyDelete()
{
  if (!d->m_selectedCurrency.id().isEmpty()) {
    MyMoneyFileTransaction ft;
    try {
      MyMoneyFile::instance()->removeCurrency(d->m_selectedCurrency);
      ft.commit();
    } catch (const MyMoneyException &e) {
      KMessageBox::sorry(this, i18n("Cannot delete currency %1. %2", d->m_selectedCurrency.name(), e.what()), i18n("Delete currency"));
    }
  }
}

void KMyMoneyApp::slotCurrencySetBase()
{
  if (!d->m_selectedCurrency.id().isEmpty()) {
    if (d->m_selectedCurrency.id() != MyMoneyFile::instance()->baseCurrency().id()) {
      MyMoneyFileTransaction ft;
      try {
        MyMoneyFile::instance()->setBaseCurrency(d->m_selectedCurrency);
        ft.commit();
      } catch (const MyMoneyException &e) {
        KMessageBox::sorry(this, i18n("Cannot set %1 as base currency: %2", d->m_selectedCurrency.name(), e.what()), i18n("Set base currency"));
      }
    }
  }
}

void KMyMoneyApp::slotBudgetNew()
{
  QDate date = QDate::currentDate();
  date.setDate(date.year(), KMyMoneyGlobalSettings::firstFiscalMonth(), KMyMoneyGlobalSettings::firstFiscalDay());
  QString newname = i18n("Budget %1", date.year());

  MyMoneyBudget budget;

  // make sure we have a unique name
  try {
    int i = 1;
    // Exception thrown when the name is not found
    while (1) {
      MyMoneyFile::instance()->budgetByName(newname);
      newname = i18n("Budget %1 %2", date.year(), i++);
    }
  } catch (const MyMoneyException &) {
    // all ok, the name is unique
  }

  MyMoneyFileTransaction ft;
  try {
    budget.setName(newname);
    budget.setBudgetStart(date);

    MyMoneyFile::instance()->addBudget(budget);
    ft.commit();
  } catch (const MyMoneyException &e) {
    KMessageBox::detailedSorry(0, i18n("Error"), i18n("Unable to add budget: %1, thrown in %2:%3", e.what(), e.file(), e.line()));
  }
}

void KMyMoneyApp::slotBudgetDelete()
{
  if (d->m_selectedBudgets.isEmpty())
    return; // shouldn't happen

  MyMoneyFile * file = MyMoneyFile::instance();

  // get confirmation from user
  QString prompt;
  if (d->m_selectedBudgets.size() == 1)
    prompt = i18n("<p>Do you really want to remove the budget <b>%1</b>?</p>", d->m_selectedBudgets.front().name());
  else
    prompt = i18n("Do you really want to remove all selected budgets?");

  if (KMessageBox::questionYesNo(this, prompt, i18n("Remove Budget")) == KMessageBox::No)
    return;

  MyMoneyFileTransaction ft;
  try {
    // now loop over all selected budgets and remove them
    for (QList<MyMoneyBudget>::iterator it = d->m_selectedBudgets.begin();
         it != d->m_selectedBudgets.end(); ++it) {
      file->removeBudget(*it);
    }
    ft.commit();

  } catch (const MyMoneyException &e) {
    KMessageBox::detailedSorry(0, i18n("Error"), i18n("Unable to remove budget: %1, thrown in %2:%3", e.what(), e.file(), e.line()));
  }
}

void KMyMoneyApp::slotBudgetCopy()
{
  if (d->m_selectedBudgets.size() == 1) {
    MyMoneyFileTransaction ft;
    try {
      MyMoneyBudget budget = d->m_selectedBudgets.first();
      budget.clearId();
      budget.setName(i18n("Copy of %1", budget.name()));

      MyMoneyFile::instance()->addBudget(budget);
      ft.commit();
    } catch (const MyMoneyException &e) {
      KMessageBox::detailedSorry(0, i18n("Error"), i18n("Unable to add budget: %1, thrown in %2:%3", e.what(), e.file(), e.line()));
    }
  }
}

void KMyMoneyApp::slotBudgetChangeYear()
{
  if (d->m_selectedBudgets.size() == 1) {
    QStringList years;
    int current = 0;
    bool haveCurrent = false;
    MyMoneyBudget budget = *(d->m_selectedBudgets.begin());
    for (int i = (QDate::currentDate().year() - 3); i < (QDate::currentDate().year() + 5); ++i) {
      years << QString("%1").arg(i);
      if (i == budget.budgetStart().year()) {
        haveCurrent = true;
      }
      if (!haveCurrent)
        ++current;
    }
    if (!haveCurrent)
      current = 0;
    bool ok = false;

    QString yearString = QInputDialog::getItem(this, i18n("Select year"), i18n("Budget year"), years, current, false, &ok);

    if (ok) {
      int year = yearString.toInt(0, 0);
      QDate newYear = QDate(year, budget.budgetStart().month(), budget.budgetStart().day());
      if (newYear != budget.budgetStart()) {
        MyMoneyFileTransaction ft;
        try {
          budget.setBudgetStart(newYear);
          MyMoneyFile::instance()->modifyBudget(budget);
          ft.commit();
        } catch (const MyMoneyException &e) {
          KMessageBox::detailedSorry(0, i18n("Error"), i18n("Unable to modify budget: %1, thrown in %2:%3", e.what(), e.file(), e.line()));
        }
      }
    }
  }
}

void KMyMoneyApp::slotBudgetForecast()
{
  if (d->m_selectedBudgets.size() == 1) {
    MyMoneyFileTransaction ft;
    try {
      MyMoneyBudget budget = d->m_selectedBudgets.first();
      bool calcBudget = budget.getaccounts().count() == 0;
      if (!calcBudget) {
        if (KMessageBox::warningContinueCancel(0, i18n("The current budget already contains data. Continuing will replace all current values of this budget."), i18nc("Warning message box", "Warning")) == KMessageBox::Continue)
          calcBudget = true;
      }

      if (calcBudget) {
        QDate historyStart;
        QDate historyEnd;
        QDate budgetStart;
        QDate budgetEnd;

        budgetStart = budget.budgetStart();
        budgetEnd = budgetStart.addYears(1).addDays(-1);
        historyStart = budgetStart.addYears(-1);
        historyEnd = budgetEnd.addYears(-1);

        MyMoneyForecast forecast = KMyMoneyGlobalSettings::forecast();
        forecast.createBudget(budget, historyStart, historyEnd, budgetStart, budgetEnd, true);

        MyMoneyFile::instance()->modifyBudget(budget);
        ft.commit();
      }
    } catch (const MyMoneyException &e) {
      KMessageBox::detailedSorry(0, i18n("Error"), i18n("Unable to modify budget: %1, thrown in %2:%3", e.what(), e.file(), e.line()));
    }
  }
}

void KMyMoneyApp::slotKDELanguageSettings()
{
  KMessageBox::information(this, i18n("Please be aware that changes made in the following dialog affect all KDE applications not only KMyMoney."), i18nc("Warning message box", "Warning"), "LanguageSettingsWarning");

  QStringList args;
  args << "language";
  QString error;
  int pid;

  KToolInvocation::kdeinitExec("kcmshell4", args, &error, &pid);
}

void KMyMoneyApp::slotNewFeature()
{
}

void KMyMoneyApp::slotTransactionsDelete()
{
  // since we may jump here via code, we have to make sure to react only
  // if the action is enabled
  if (!kmymoney->actionCollection()->action(s_Actions[Action::TransactionDelete])->isEnabled())
    return;
  if (d->m_selectedTransactions.isEmpty())
    return;
  if (d->m_selectedTransactions.warnLevel() == 1) {
    if (KMessageBox::warningContinueCancel(0,
                                           i18n("At least one split of the selected transactions has been reconciled. "
                                                "Do you wish to delete the transactions anyway?"),
                                           i18n("Transaction already reconciled")) == KMessageBox::Cancel)
      return;
  }
  QString msg =
    i18np("Do you really want to delete the selected transaction?",
          "Do you really want to delete all %1 selected transactions?",
          d->m_selectedTransactions.count());

  if (KMessageBox::questionYesNo(this, msg, i18n("Delete transaction")) == KMessageBox::Yes) {
    KMSTATUS(i18n("Deleting transactions"));
    doDeleteTransactions();
  }
}

void KMyMoneyApp::slotTransactionDuplicate()
{
  // since we may jump here via code, we have to make sure to react only
  // if the action is enabled
  if (kmymoney->actionCollection()->action(s_Actions[Action::TransactionDuplicate])->isEnabled()) {
    KMyMoneyRegister::SelectedTransactions list = d->m_selectedTransactions;
    KMyMoneyRegister::SelectedTransactions::iterator it_t;

    int i = 0;
    int cnt = d->m_selectedTransactions.count();
    KMSTATUS(i18n("Duplicating transactions"));
    slotStatusProgressBar(0, cnt);
    MyMoneyFileTransaction ft;
    MyMoneyTransaction lt;
    try {
      for (it_t = list.begin(); it_t != list.end(); ++it_t) {
        MyMoneyTransaction t = (*it_t).transaction();
        QList<MyMoneySplit>::iterator it_s;
        // wipe out any reconciliation information
        for (it_s = t.splits().begin(); it_s != t.splits().end(); ++it_s) {
          (*it_s).setReconcileFlag(MyMoneySplit::NotReconciled);
          (*it_s).setReconcileDate(QDate());
          (*it_s).setBankID(QString());
        }
        // clear invalid data
        t.setEntryDate(QDate());
        t.clearId();
        // and set the post date to today
        t.setPostDate(QDate::currentDate());

        MyMoneyFile::instance()->addTransaction(t);
        lt = t;
        slotStatusProgressBar(i++, 0);
      }
      ft.commit();

      // select the new transaction in the ledger
      if (!d->m_selectedAccount.id().isEmpty())
        d->m_myMoneyView->slotLedgerSelected(d->m_selectedAccount.id(), lt.id());

    } catch (const MyMoneyException &e) {
      KMessageBox::detailedSorry(0, i18n("Error"), i18n("Unable to duplicate transaction(s): %1, thrown in %2:%3", e.what(), e.file(), e.line()));
    }
    // switch off the progress bar
    slotStatusProgressBar(-1, -1);
  }
}

void KMyMoneyApp::doDeleteTransactions()
{
  KMyMoneyRegister::SelectedTransactions list = d->m_selectedTransactions;
  KMyMoneyRegister::SelectedTransactions::iterator it_t;
  int cnt = list.count();
  int i = 0;
  slotStatusProgressBar(0, cnt);
  MyMoneyFileTransaction ft;
  MyMoneyFile* file = MyMoneyFile::instance();
  try {
    it_t = list.begin();
    while (it_t != list.end()) {
      // only remove those transactions that do not reference a closed account
      if (!file->referencesClosedAccount((*it_t).transaction())) {
        file->removeTransaction((*it_t).transaction());
        // remove all those references in the list of selected transactions
        // that refer to the same transaction we just removed so that we
        // will not be caught by an exception later on (see bko #285310)
        KMyMoneyRegister::SelectedTransactions::iterator it_td = it_t;
        ++it_td;
        while (it_td != list.end()) {
          if ((*it_t).transaction().id() == (*it_td).transaction().id()) {
            it_td = list.erase(it_td);
            i++; // bump count of deleted transactions
          } else {
            ++it_td;
          }
        }
      }
      // need to ensure "nextCheckNumber" is still correct
      MyMoneyAccount acc = file->account((*it_t).split().accountId());
      // the "lastNumberUsed" might have been the txn number deleted
      // so adjust it
      QString deletedNum = (*it_t).split().number();
      // decrement deletedNum and set new "lastNumberUsed"
      QString num = KMyMoneyUtils::getAdjacentNumber(deletedNum, -1);
      acc.setValue("lastNumberUsed", num);

      file->modifyAccount(acc);
      list.erase(it_t);
      it_t = list.begin();
      slotStatusProgressBar(i++, 0);
    }
    ft.commit();
  } catch (const MyMoneyException &e) {
    KMessageBox::detailedSorry(0, i18n("Error"), i18n("Unable to delete transaction(s): %1, thrown in %2:%3", e.what(), e.file(), e.line()));
  }
  slotStatusProgressBar(-1, -1);
}

void KMyMoneyApp::slotTransactionsNew()
{
  // since we jump here via code, we have to make sure to react only
  // if the action is enabled
  if (kmymoney->actionCollection()->action(s_Actions[Action::TransactionNew])->isEnabled()) {
    if (d->m_myMoneyView->createNewTransaction()) {
      d->m_transactionEditor = d->m_myMoneyView->startEdit(d->m_selectedTransactions);
      if (d->m_transactionEditor) {
        KMyMoneyMVCCombo::setSubstringSearchForChildren(d->m_myMoneyView, !KMyMoneySettings::stringMatchFromStart());
        KMyMoneyPayeeCombo* payeeEdit = dynamic_cast<KMyMoneyPayeeCombo*>(d->m_transactionEditor->haveWidget("payee"));
        if (payeeEdit && !d->m_lastPayeeEnteredId.isEmpty()) {
          // in case we entered a new transaction before and used a payee,
          // we reuse it here. Save the text to the edit widget, select it
          // so that hitting any character will start entering another payee.
          payeeEdit->setSelectedItem(d->m_lastPayeeEnteredId);
          payeeEdit->lineEdit()->selectAll();
        }
        if (d->m_transactionEditor) {
          connect(d->m_transactionEditor, SIGNAL(statusProgress(int,int)), this, SLOT(slotStatusProgressBar(int,int)));
          connect(d->m_transactionEditor, SIGNAL(statusMsg(QString)), this, SLOT(slotStatusMsg(QString)));
          connect(d->m_transactionEditor, SIGNAL(scheduleTransaction(MyMoneyTransaction,MyMoneySchedule::occurrenceE)), this, SLOT(slotScheduleNew(MyMoneyTransaction,MyMoneySchedule::occurrenceE)));
        }
        slotUpdateActions();
      }
    }
  }
}

void KMyMoneyApp::slotTransactionsEdit()
{
  // qDebug("KMyMoneyApp::slotTransactionsEdit()");
  // since we jump here via code, we have to make sure to react only
  // if the action is enabled
  if (kmymoney->actionCollection()->action(s_Actions[Action::TransactionEdit])->isEnabled()) {
    // as soon as we edit a transaction, we don't remember the last payee entered
    d->m_lastPayeeEnteredId.clear();
    d->m_transactionEditor = d->m_myMoneyView->startEdit(d->m_selectedTransactions);
    KMyMoneyMVCCombo::setSubstringSearchForChildren(d->m_myMoneyView, !KMyMoneySettings::stringMatchFromStart());
    slotUpdateActions();
  }
}

void KMyMoneyApp::deleteTransactionEditor()
{
  // make sure, we don't use the transaction editor pointer
  // anymore from now on
  TransactionEditor* p = d->m_transactionEditor;
  d->m_transactionEditor = 0;
  delete p;
}

void KMyMoneyApp::slotTransactionsEditSplits()
{
  // since we jump here via code, we have to make sure to react only
  // if the action is enabled
  if (kmymoney->actionCollection()->action(s_Actions[Action::TransactionEditSplits])->isEnabled()) {
    // as soon as we edit a transaction, we don't remember the last payee entered
    d->m_lastPayeeEnteredId.clear();
    d->m_transactionEditor = d->m_myMoneyView->startEdit(d->m_selectedTransactions);
    slotUpdateActions();

    if (d->m_transactionEditor) {
      KMyMoneyMVCCombo::setSubstringSearchForChildren(d->m_myMoneyView, !KMyMoneySettings::stringMatchFromStart());
      if (d->m_transactionEditor->slotEditSplits() == QDialog::Accepted) {
        MyMoneyFileTransaction ft;
        try {
          QString id;
          connect(d->m_transactionEditor, SIGNAL(balanceWarning(QWidget*,MyMoneyAccount,QString)), d->m_balanceWarning, SLOT(slotShowMessage(QWidget*,MyMoneyAccount,QString)));
          d->m_transactionEditor->enterTransactions(id);
          ft.commit();
        } catch (const MyMoneyException &e) {
          KMessageBox::detailedSorry(0, i18n("Error"), i18n("Unable to modify transaction: %1, thrown in %2:%3", e.what(), e.file(), e.line()));
        }
      }
    }
    deleteTransactionEditor();
    slotUpdateActions();
  }
}

void KMyMoneyApp::slotTransactionsCancel()
{
  // since we jump here via code, we have to make sure to react only
  // if the action is enabled
  if (kmymoney->actionCollection()->action(s_Actions[Action::TransactionCancel])->isEnabled()) {
    // make sure, we block the enter function
    actionCollection()->action(s_Actions[Action::TransactionEnter])->setEnabled(false);
    // qDebug("KMyMoneyApp::slotTransactionsCancel");
    deleteTransactionEditor();
    slotUpdateActions();
  }
}

void KMyMoneyApp::slotTransactionsEnter()
{
  // since we jump here via code, we have to make sure to react only
  // if the action is enabled
  if (kmymoney->actionCollection()->action(s_Actions[Action::TransactionEnter])->isEnabled()) {
    // disable the action while we process it to make sure it's processed only once since
    // d->m_transactionEditor->enterTransactions(newId) will run QCoreApplication::processEvents
    // we could end up here twice which will cause a crash slotUpdateActions() will enable the action again
    kmymoney->actionCollection()->action(s_Actions[Action::TransactionEnter])->setEnabled(false);
    if (d->m_transactionEditor) {
      QString accountId = d->m_selectedAccount.id();
      QString newId;
      connect(d->m_transactionEditor, SIGNAL(balanceWarning(QWidget*,MyMoneyAccount,QString)), d->m_balanceWarning, SLOT(slotShowMessage(QWidget*,MyMoneyAccount,QString)));
      if (d->m_transactionEditor->enterTransactions(newId)) {
        KMyMoneyPayeeCombo* payeeEdit = dynamic_cast<KMyMoneyPayeeCombo*>(d->m_transactionEditor->haveWidget("payee"));
        if (payeeEdit && !newId.isEmpty()) {
          d->m_lastPayeeEnteredId = payeeEdit->selectedItem();
        }
        deleteTransactionEditor();
      }
      if (!newId.isEmpty()) {
        d->m_myMoneyView->slotLedgerSelected(accountId, newId);
      }
    }
    slotUpdateActions();
  }
}

void KMyMoneyApp::slotTransactionsCancelOrEnter(bool& okToSelect)
{
  static bool oneTime = false;
  if (!oneTime) {
    oneTime = true;
    QString dontShowAgain = "CancelOrEditTransaction";
    // qDebug("KMyMoneyApp::slotCancelOrEndEdit");
    if (d->m_transactionEditor) {
      if (KMyMoneyGlobalSettings::focusChangeIsEnter() && kmymoney->actionCollection()->action(s_Actions[Action::TransactionEnter])->isEnabled()) {
        slotTransactionsEnter();
        if (d->m_transactionEditor) {
          // if at this stage the editor is still there that means that entering the transaction was cancelled
          // for example by pressing cancel on the exchange rate editor so we must stay in edit mode
          okToSelect = false;
        }
      } else {
        // okToSelect is preset to true if a cancel of the dialog is useful and false if it is not
        int rc;
        KGuiItem noGuiItem = KStandardGuiItem::save();
        KGuiItem yesGuiItem = KStandardGuiItem::discard();
        KGuiItem cancelGuiItem = KStandardGuiItem::cont();

        // if the transaction can't be entered make sure that it can't be entered by pressing no either
        if (!kmymoney->actionCollection()->action(s_Actions[Action::TransactionEnter])->isEnabled()) {
          noGuiItem.setEnabled(false);
          noGuiItem.setToolTip(kmymoney->actionCollection()->action(s_Actions[Action::TransactionEnter])->toolTip());
        }
        if (okToSelect == true) {
          rc = KMessageBox::warningYesNoCancel(0, i18n("<p>Please select what you want to do: discard the changes, save the changes or continue to edit the transaction.</p><p>You can also set an option to save the transaction automatically when e.g. selecting another transaction.</p>"), i18n("End transaction edit"), yesGuiItem, noGuiItem, cancelGuiItem, dontShowAgain);

        } else {
          rc = KMessageBox::warningYesNo(0, i18n("<p>Please select what you want to do: discard the changes, save the changes or continue to edit the transaction.</p><p>You can also set an option to save the transaction automatically when e.g. selecting another transaction.</p>"), i18n("End transaction edit"), yesGuiItem, noGuiItem, dontShowAgain);
        }

        switch (rc) {
          case KMessageBox::Yes:
            slotTransactionsCancel();
            break;
          case KMessageBox::No:
            slotTransactionsEnter();
            // make sure that we'll see this message the next time no matter
            // if the user has chosen the 'Don't show again' checkbox
            KMessageBox::enableMessage(dontShowAgain);
            if (d->m_transactionEditor) {
              // if at this stage the editor is still there that means that entering the transaction was cancelled
              // for example by pressing cancel on the exchange rate editor so we must stay in edit mode
              okToSelect = false;
            }
            break;
          case KMessageBox::Cancel:
            // make sure that we'll see this message the next time no matter
            // if the user has chosen the 'Don't show again' checkbox
            KMessageBox::enableMessage(dontShowAgain);
            okToSelect = false;
            break;
        }
      }
    }
    oneTime = false;
  }
}

void KMyMoneyApp::slotToggleReconciliationFlag()
{
  markTransaction(MyMoneySplit::Unknown);
}

void KMyMoneyApp::slotMarkTransactionCleared()
{
  markTransaction(MyMoneySplit::Cleared);
}

void KMyMoneyApp::slotMarkTransactionReconciled()
{
  markTransaction(MyMoneySplit::Reconciled);
}

void KMyMoneyApp::slotMarkTransactionNotReconciled()
{
  markTransaction(MyMoneySplit::NotReconciled);
}

void KMyMoneyApp::markTransaction(MyMoneySplit::reconcileFlagE flag)
{
  KMyMoneyRegister::SelectedTransactions list = d->m_selectedTransactions;
  KMyMoneyRegister::SelectedTransactions::const_iterator it_t;
  int cnt = list.count();
  int i = 0;
  slotStatusProgressBar(0, cnt);
  MyMoneyFileTransaction ft;
  try {
    for (it_t = list.constBegin(); it_t != list.constEnd(); ++it_t) {
      // turn on signals before we modify the last entry in the list
      cnt--;
      MyMoneyFile::instance()->blockSignals(cnt != 0);

      // get a fresh copy
      MyMoneyTransaction t = MyMoneyFile::instance()->transaction((*it_t).transaction().id());
      MyMoneySplit sp = t.splitById((*it_t).split().id());
      if (sp.reconcileFlag() != flag) {
        if (flag == MyMoneySplit::Unknown) {
          if (d->m_reconciliationAccount.id().isEmpty()) {
            // in normal mode we cycle through all states
            switch (sp.reconcileFlag()) {
              case MyMoneySplit::NotReconciled:
                sp.setReconcileFlag(MyMoneySplit::Cleared);
                break;
              case MyMoneySplit::Cleared:
                sp.setReconcileFlag(MyMoneySplit::Reconciled);
                break;
              case MyMoneySplit::Reconciled:
                sp.setReconcileFlag(MyMoneySplit::NotReconciled);
                break;
              default:
                break;
            }
          } else {
            // in reconciliation mode we skip the reconciled state
            switch (sp.reconcileFlag()) {
              case MyMoneySplit::NotReconciled:
                sp.setReconcileFlag(MyMoneySplit::Cleared);
                break;
              case MyMoneySplit::Cleared:
                sp.setReconcileFlag(MyMoneySplit::NotReconciled);
                break;
              default:
                break;
            }
          }
        } else {
          sp.setReconcileFlag(flag);
        }

        t.modifySplit(sp);
        MyMoneyFile::instance()->modifyTransaction(t);
      }
      slotStatusProgressBar(i++, 0);
    }
    slotStatusProgressBar(-1, -1);
    ft.commit();
  } catch (const MyMoneyException &e) {
    KMessageBox::detailedSorry(0, i18n("Error"), i18n("Unable to modify transaction: %1, thrown in %2:%3", e.what(), e.file(), e.line()));
  }
}

void KMyMoneyApp::slotTransactionsAccept()
{
  KMyMoneyRegister::SelectedTransactions list = d->m_selectedTransactions;
  KMyMoneyRegister::SelectedTransactions::const_iterator it_t;
  int cnt = list.count();
  int i = 0;
  slotStatusProgressBar(0, cnt);
  MyMoneyFileTransaction ft;
  try {
    for (it_t = list.constBegin(); it_t != list.constEnd(); ++it_t) {
      // reload transaction in case it got changed during the course of this loop
      MyMoneyTransaction t = MyMoneyFile::instance()->transaction((*it_t).transaction().id());
      if (t.isImported()) {
        t.setImported(false);
        if (!d->m_selectedAccount.id().isEmpty()) {
          QList<MyMoneySplit> list = t.splits();
          QList<MyMoneySplit>::const_iterator it_s;
          for (it_s = list.constBegin(); it_s != list.constEnd(); ++it_s) {
            if ((*it_s).accountId() == d->m_selectedAccount.id()) {
              if ((*it_s).reconcileFlag() == MyMoneySplit::NotReconciled) {
                MyMoneySplit s = (*it_s);
                s.setReconcileFlag(MyMoneySplit::Cleared);
                t.modifySplit(s);
              }
            }
          }
        }
        MyMoneyFile::instance()->modifyTransaction(t);
      }
      if ((*it_t).split().isMatched()) {
        // reload split in case it got changed during the course of this loop
        MyMoneySplit s = t.splitById((*it_t).split().id());
        TransactionMatcher matcher(d->m_selectedAccount);
        matcher.accept(t, s);
      }
      slotStatusProgressBar(i++, 0);
    }
    slotStatusProgressBar(-1, -1);
    ft.commit();
  } catch (const MyMoneyException &e) {
    KMessageBox::detailedSorry(0, i18n("Error"), i18n("Unable to accept transaction: %1, thrown in %2:%3", e.what(), e.file(), e.line()));
  }
}

void KMyMoneyApp::slotTransactionGotoAccount()
{
  if (!d->m_accountGoto.isEmpty()) {
    try {
      QString transactionId;
      if (d->m_selectedTransactions.count() == 1) {
        transactionId = d->m_selectedTransactions[0].transaction().id();
      }
      // make sure to pass a copy, as d->myMoneyView->slotLedgerSelected() overrides
      // d->m_accountGoto while calling slotUpdateActions()
      QString accountId = d->m_accountGoto;
      d->m_myMoneyView->slotLedgerSelected(accountId, transactionId);
    } catch (const MyMoneyException &) {
    }
  }
}

void KMyMoneyApp::slotTransactionGotoPayee()
{
  if (!d->m_payeeGoto.isEmpty()) {
    try {
      QString transactionId;
      if (d->m_selectedTransactions.count() == 1) {
        transactionId = d->m_selectedTransactions[0].transaction().id();
      }
      // make sure to pass copies, as d->myMoneyView->slotPayeeSelected() overrides
      // d->m_payeeGoto and d->m_selectedAccount while calling slotUpdateActions()
      QString payeeId = d->m_payeeGoto;
      QString accountId = d->m_selectedAccount.id();
      d->m_myMoneyView->slotPayeeSelected(payeeId, accountId, transactionId);
    } catch (const MyMoneyException &) {
    }
  }
}

void KMyMoneyApp::slotTransactionCreateSchedule()
{
  if (d->m_selectedTransactions.count() == 1) {
    // make sure to have the current selected split as first split in the schedule
    MyMoneyTransaction t = d->m_selectedTransactions[0].transaction();
    MyMoneySplit s = d->m_selectedTransactions[0].split();
    QString splitId = s.id();
    s.clearId();
    s.setReconcileFlag(MyMoneySplit::NotReconciled);
    s.setReconcileDate(QDate());
    t.removeSplits();
    t.addSplit(s);
    const QList<MyMoneySplit>& splits = d->m_selectedTransactions[0].transaction().splits();
    QList<MyMoneySplit>::const_iterator it_s;
    for (it_s = splits.begin(); it_s != splits.end(); ++it_s) {
      if ((*it_s).id() != splitId) {
        MyMoneySplit s0 = (*it_s);
        s0.clearId();
        s0.setReconcileFlag(MyMoneySplit::NotReconciled);
        s0.setReconcileDate(QDate());
        t.addSplit(s0);
      }
    }
    slotScheduleNew(t);
  }
}

void KMyMoneyApp::slotTransactionAssignNumber()
{
  if (d->m_transactionEditor)
    d->m_transactionEditor->assignNextNumber();
}

void KMyMoneyApp::slotTransactionCombine()
{
  qDebug("slotTransactionCombine() not implemented yet");
}

void KMyMoneyApp::slotTransactionCopySplits()
{
  MyMoneyFile* file = MyMoneyFile::instance();

  if (d->m_selectedTransactions.count() >= 2) {
    int singleSplitTransactions = 0;
    int multipleSplitTransactions = 0;
    KMyMoneyRegister::SelectedTransaction selectedSourceTransaction;
    foreach (const KMyMoneyRegister::SelectedTransaction& st, d->m_selectedTransactions) {
      switch (st.transaction().splitCount()) {
        case 0:
          break;
        case 1:
          singleSplitTransactions++;
          break;
        default:
          selectedSourceTransaction = st;
          multipleSplitTransactions++;
          break;
      }
    }
    if (singleSplitTransactions > 0 && multipleSplitTransactions == 1) {
      MyMoneyFileTransaction ft;
      try {
        const MyMoneyTransaction& sourceTransaction = selectedSourceTransaction.transaction();
        const MyMoneySplit& sourceSplit = selectedSourceTransaction.split();
        foreach (const KMyMoneyRegister::SelectedTransaction& st, d->m_selectedTransactions) {
          MyMoneyTransaction t = st.transaction();

          // don't process the source transaction
          if (sourceTransaction.id() == t.id()) {
            continue;
          }

          const MyMoneySplit& baseSplit = st.split();

          if (t.splitCount() == 1) {
            foreach (const MyMoneySplit& split, sourceTransaction.splits()) {
              // Don't copy the source split, as we already have that
              // as part of the destination transaction
              if (split.id() == sourceSplit.id()) {
                continue;
              }

              MyMoneySplit sp(split);
              // clear the ID and reconciliation state
              sp.clearId();
              sp.setReconcileFlag(MyMoneySplit::NotReconciled);
              sp.setReconcileDate(QDate());

              // in case it is a simple transaction consisting of two splits,
              // we can adjust the share and value part of the second split we
              // just created. We need to keep a possible price in mind in case
              // of different currencies
              if (sourceTransaction.splitCount() == 2) {
                sp.setValue(-baseSplit.value());
                sp.setShares(-(baseSplit.shares() * baseSplit.price()));
              }
              t.addSplit(sp);
            }
            file->modifyTransaction(t);
          }
        }
        ft.commit();
      } catch (const MyMoneyException &) {
        qDebug() << "transactionCopySplits() failed";
      }
    }
  }
}

void KMyMoneyApp::slotMoveToAccount(const QString& id)
{
  // close the menu, if it is still open
  QWidget* w = factory()->container("transaction_context_menu", this);
  if (w && w->isVisible()) {
    w->close();
  }

  if (!d->m_selectedTransactions.isEmpty()) {
    MyMoneyFileTransaction ft;
    try {
      KMyMoneyRegister::SelectedTransactions::const_iterator it_t;
      for (it_t = d->m_selectedTransactions.constBegin(); it_t != d->m_selectedTransactions.constEnd(); ++it_t) {
        if (d->m_selectedAccount.accountType() == MyMoneyAccount::Investment) {
          d->moveInvestmentTransaction(d->m_selectedAccount.id(), id, (*it_t).transaction());
        } else {
          QList<MyMoneySplit>::const_iterator it_s;
          bool changed = false;
          MyMoneyTransaction t = (*it_t).transaction();
          for (it_s = (*it_t).transaction().splits().constBegin(); it_s != (*it_t).transaction().splits().constEnd(); ++it_s) {
            if ((*it_s).accountId() == d->m_selectedAccount.id()) {
              MyMoneySplit s = (*it_s);
              s.setAccountId(id);
              t.modifySplit(s);
              changed = true;
            }
          }
          if (changed) {
            MyMoneyFile::instance()->modifyTransaction(t);
          }
        }
      }
      ft.commit();
    } catch (const MyMoneyException &) {
    }
  }
}

// move a stock transaction from one investment account to another
void KMyMoneyApp::Private::moveInvestmentTransaction(const QString& /*fromId*/,
    const QString& toId,
    const MyMoneyTransaction& tx)
{
  MyMoneyAccount toInvAcc = MyMoneyFile::instance()->account(toId);
  MyMoneyTransaction t(tx);
  // first determine which stock we are dealing with.
  // fortunately, investment transactions have only one stock involved
  QString stockAccountId;
  QString stockSecurityId;
  MyMoneySplit s;
  for (QList<MyMoneySplit>::const_iterator it_s = t.splits().constBegin(); it_s != t.splits().constEnd(); ++it_s) {
    stockAccountId = (*it_s).accountId();
    stockSecurityId =
      MyMoneyFile::instance()->account(stockAccountId).currencyId();
    if (!MyMoneyFile::instance()->security(stockSecurityId).isCurrency()) {
      s = *it_s;
      break;
    }
  }
  // Now check the target investment account to see if it
  // contains a stock with this id
  QString newStockAccountId;
  QStringList accountList = toInvAcc.accountList();
  for (QStringList::const_iterator it_a = accountList.constBegin(); it_a != accountList.constEnd(); ++it_a) {
    if (MyMoneyFile::instance()->account((*it_a)).currencyId() ==
        stockSecurityId) {
      newStockAccountId = (*it_a);
      break;
    }
  }
  // if it doesn't exist, we need to add it as a copy of the old one
  // no 'copyAccount()' function??
  if (newStockAccountId.isEmpty()) {
    MyMoneyAccount stockAccount =
      MyMoneyFile::instance()->account(stockAccountId);
    MyMoneyAccount newStock;
    newStock.setName(stockAccount.name());
    newStock.setNumber(stockAccount.number());
    newStock.setDescription(stockAccount.description());
    newStock.setInstitutionId(stockAccount.institutionId());
    newStock.setOpeningDate(stockAccount.openingDate());
    newStock.setAccountType(stockAccount.accountType());
    newStock.setCurrencyId(stockAccount.currencyId());
    newStock.setClosed(stockAccount.isClosed());
    MyMoneyFile::instance()->addAccount(newStock, toInvAcc);
    newStockAccountId = newStock.id();
  }
  // now update the split and the transaction
  s.setAccountId(newStockAccountId);
  t.modifySplit(s);
  MyMoneyFile::instance()->modifyTransaction(t);
}

void KMyMoneyApp::slotUpdateMoveToAccountMenu()
{
  createTransactionMoveMenu();

  // in case we were not able to create the selector, we
  // better get out of here. Anything else would cause
  // a crash later on (accountSet.load)
  if (!d->m_moveToAccountSelector)
    return;

  if (!d->m_selectedAccount.id().isEmpty()) {
    AccountSet accountSet;
    if (d->m_selectedAccount.accountType() == MyMoneyAccount::Investment) {
      accountSet.addAccountType(MyMoneyAccount::Investment);
    } else if (d->m_selectedAccount.isAssetLiability()) {

      accountSet.addAccountType(MyMoneyAccount::Checkings);
      accountSet.addAccountType(MyMoneyAccount::Savings);
      accountSet.addAccountType(MyMoneyAccount::Cash);
      accountSet.addAccountType(MyMoneyAccount::AssetLoan);
      accountSet.addAccountType(MyMoneyAccount::CertificateDep);
      accountSet.addAccountType(MyMoneyAccount::MoneyMarket);
      accountSet.addAccountType(MyMoneyAccount::Asset);
      accountSet.addAccountType(MyMoneyAccount::Currency);
      accountSet.addAccountType(MyMoneyAccount::CreditCard);
      accountSet.addAccountType(MyMoneyAccount::Loan);
      accountSet.addAccountType(MyMoneyAccount::Liability);
    } else if (d->m_selectedAccount.isIncomeExpense()) {
      accountSet.addAccountType(MyMoneyAccount::Income);
      accountSet.addAccountType(MyMoneyAccount::Expense);
    }

    accountSet.load(d->m_moveToAccountSelector);
    // remove those accounts that we currently reference
    KMyMoneyRegister::SelectedTransactions::const_iterator it_t;
    for (it_t = d->m_selectedTransactions.constBegin(); it_t != d->m_selectedTransactions.constEnd(); ++it_t) {
      QList<MyMoneySplit>::const_iterator it_s;
      for (it_s = (*it_t).transaction().splits().constBegin(); it_s != (*it_t).transaction().splits().constEnd(); ++it_s) {
        d->m_moveToAccountSelector->removeItem((*it_s).accountId());
      }
    }
    // remove those accounts from the list that are denominated
    // in a different currency
    QStringList list = d->m_moveToAccountSelector->accountList();
    QList<QString>::const_iterator it_a;
    for (it_a = list.constBegin(); it_a != list.constEnd(); ++it_a) {
      MyMoneyAccount acc = MyMoneyFile::instance()->account(*it_a);
      if (acc.currencyId() != d->m_selectedAccount.currencyId())
        d->m_moveToAccountSelector->removeItem((*it_a));
    }
  }
}

void KMyMoneyApp::slotTransactionMatch()
{
  // if the menu action is retrieved it can contain an '&' character for the accelerator causing the comparison to fail if not removed
  QString transactionActionText = actionCollection()->action(s_Actions[Action::TransactionMatch])->text();
  transactionActionText.remove('&');
  if (transactionActionText == i18nc("Button text for match transaction", "Match"))
    transactionMatch();
  else
    transactionUnmatch();
}

void KMyMoneyApp::transactionUnmatch()
{
  KMyMoneyRegister::SelectedTransactions::const_iterator it;
  MyMoneyFileTransaction ft;
  try {
    for (it = d->m_selectedTransactions.constBegin(); it != d->m_selectedTransactions.constEnd(); ++it) {
      if ((*it).split().isMatched()) {
        TransactionMatcher matcher(d->m_selectedAccount);
        matcher.unmatch((*it).transaction(), (*it).split());
      }
    }
    ft.commit();

  } catch (const MyMoneyException &e) {
    KMessageBox::detailedSorry(0, i18n("Unable to unmatch the selected transactions"), e.what());
  }
}

void KMyMoneyApp::transactionMatch()
{
  if (d->m_selectedTransactions.count() != 2)
    return;

  MyMoneyTransaction startMatchTransaction;
  MyMoneyTransaction endMatchTransaction;
  MyMoneySplit startSplit;
  MyMoneySplit endSplit;

  KMyMoneyRegister::SelectedTransactions::const_iterator it;
  KMyMoneyRegister::SelectedTransactions toBeDeleted;
  for (it = d->m_selectedTransactions.constBegin(); it != d->m_selectedTransactions.constEnd(); ++it) {
    if ((*it).transaction().isImported()) {
      if (endMatchTransaction.id().isEmpty()) {
        endMatchTransaction = (*it).transaction();
        endSplit = (*it).split();
        toBeDeleted << *it;
      } else {
        //This is a second imported transaction, we still want to merge
        startMatchTransaction = (*it).transaction();
        startSplit = (*it).split();
      }
    } else if (!(*it).split().isMatched()) {
      if (startMatchTransaction.id().isEmpty()) {
        startMatchTransaction = (*it).transaction();
        startSplit = (*it).split();
      } else {
        endMatchTransaction = (*it).transaction();
        endSplit = (*it).split();
        toBeDeleted << *it;
      }
    }
  }

#if 0
  KMergeTransactionsDlg dlg(d->m_selectedAccount);
  dlg.addTransaction(startMatchTransaction);
  dlg.addTransaction(endMatchTransaction);
  if (dlg.exec() == QDialog::Accepted)
#endif
  {
    MyMoneyFileTransaction ft;
    try {
      if (startMatchTransaction.id().isEmpty())
        throw MYMONEYEXCEPTION(i18n("No manually entered transaction selected for matching"));
      if (endMatchTransaction.id().isEmpty())
        throw MYMONEYEXCEPTION(i18n("No imported transaction selected for matching"));

      TransactionMatcher matcher(d->m_selectedAccount);
      matcher.match(startMatchTransaction, startSplit, endMatchTransaction, endSplit, true);
      ft.commit();
    } catch (const MyMoneyException &e) {
      KMessageBox::detailedSorry(0, i18n("Unable to match the selected transactions"), e.what());
    }
  }
}

void KMyMoneyApp::showContextMenu(const QString& containerName)
{
  QWidget* w = factory()->container(containerName, this);
  QMenu *menu = dynamic_cast<QMenu*>(w);
  if (menu)
    menu->exec(QCursor::pos());
  else
    qDebug("menu '%s' not found: w = %p, menu = %p", qPrintable(containerName), w, menu);
}

void KMyMoneyApp::slotShowTransactionContextMenu()
{
  if (d->m_selectedTransactions.isEmpty() && d->m_selectedSchedule != MyMoneySchedule()) {
    showContextMenu("schedule_context_menu");
  } else {
    showContextMenu("transaction_context_menu");
  }
}

void KMyMoneyApp::slotShowInvestmentContextMenu()
{
  showContextMenu("investment_context_menu");
}

void KMyMoneyApp::slotShowScheduleContextMenu()
{
  showContextMenu("schedule_context_menu");
}

void KMyMoneyApp::slotShowAccountContextMenu(const MyMoneyObject& obj)
{
//  qDebug("KMyMoneyApp::slotShowAccountContextMenu");
  if (typeid(obj) != typeid(MyMoneyAccount))
    return;

  const MyMoneyAccount& acc = dynamic_cast<const MyMoneyAccount&>(obj);

  // if the selected account is actually a stock account, we
  // call the right slot instead
  if (acc.isInvest()) {
    showContextMenu("investment_context_menu");
  } else if (acc.isIncomeExpense()) {
    showContextMenu("category_context_menu");
  } else {
    showContextMenu("account_context_menu");
  }
}

void KMyMoneyApp::slotShowInstitutionContextMenu(const MyMoneyObject& obj)
{
  if (typeid(obj) != typeid(MyMoneyInstitution))
    return;

  showContextMenu("institution_context_menu");
}

void KMyMoneyApp::slotShowPayeeContextMenu()
{
  showContextMenu("payee_context_menu");
}

void KMyMoneyApp::slotShowTagContextMenu()
{
  showContextMenu("tag_context_menu");
}

void KMyMoneyApp::slotShowBudgetContextMenu()
{
  showContextMenu("budget_context_menu");
}

void KMyMoneyApp::slotShowCurrencyContextMenu()
{
  showContextMenu("currency_context_menu");
}

void KMyMoneyApp::slotShowPriceContextMenu()
{
  showContextMenu("price_context_menu");
}

void KMyMoneyApp::slotShowOnlineJobContextMenu()
{
  showContextMenu("onlinejob_context_menu");
}

void KMyMoneyApp::slotPrintView()
{
  d->m_myMoneyView->slotPrintView();
}

void KMyMoneyApp::updateCaption(bool skipActions)
{
  QString caption;

  caption = d->m_fileName.fileName();

  if (caption.isEmpty() && d->m_myMoneyView && d->m_myMoneyView->fileOpen())
    caption = i18n("Untitled");

  // MyMoneyFile::instance()->dirty() throws an exception, if
  // there's no storage object available. In this case, we
  // assume that the storage object is not changed. Actually,
  // this can only happen if we are newly created early on.
  bool modified;
  try {
    modified = MyMoneyFile::instance()->dirty();
  } catch (const MyMoneyException &) {
    modified = false;
    skipActions = true;
  }

#ifdef KMM_DEBUG
  caption += QString(" (%1 x %2)").arg(width()).arg(height());
#endif

  setCaption(caption, modified);

  if (!skipActions) {
    d->m_myMoneyView->enableViewsIfFileOpen();
    slotUpdateActions();
  }
}

void KMyMoneyApp::slotUpdateActions()
{
  MyMoneyFile* file = MyMoneyFile::instance();
  const bool fileOpen = d->m_myMoneyView->fileOpen();
  const bool modified = file->dirty();
  const bool importRunning = (d->m_smtReader != 0);
  QWidget* w;
  KActionCollection *aC = actionCollection();

  // *************
  // Disabling actions to be disabled at this point
  // *************
  {
    static const QVector<Action> disabledActions {
          Action::AccountStartReconciliation, Action::AccountFinishReconciliation, Action::AccountPostponeReconciliation,
          Action::AccountEdit, Action::AccountDelete, Action::AccountOpen, Action::AccountClose, Action::AccountReopen,
          Action::AccountTransactionReport, Action::AccountOnlineMap, Action::AccountOnlineUnmap,
          Action::AccountUpdate, Action::AccountUpdateAll, Action::AccountBalanceChart,
          Action::CategoryEdit, Action::CategoryDelete, Action::InstitutionEdit, Action::InstitutionDelete,
          Action::InvestmentEdit, Action::InvestmentNew, Action::InvestmentDelete, Action::InvestmentOnlinePrice, Action::InvestmentManualPrice,
          Action::ScheduleEdit, Action::ScheduleDelete, Action::ScheduleEnter, Action::ScheduleSkip,
          Action::PayeeDelete, Action::PayeeRename, Action::PayeeMerge, Action::TagDelete, Action::TagRename,
          Action::BudgetDelete, Action::BudgetRename, Action::BudgetChangeYear, Action::BudgetNew, Action::BudgetCopy, Action::BudgetForecast,
          Action::TransactionEdit, Action::TransactionEditSplits, Action::TransactionEnter,
          Action::TransactionCancel, Action::TransactionDelete, Action::TransactionMatch,
          Action::TransactionAccept, Action::TransactionDuplicate, Action::TransactionToggleReconciled, Action::TransactionToggleCleared,
          Action::TransactionGoToAccount, Action::TransactionGoToPayee, Action::TransactionAssignNumber, Action::TransactionCreateSchedule,
          Action::TransactionCombine, Action::TransactionSelectAll, Action::TransactionCopySplits,
          Action::ScheduleEdit, Action::ScheduleDelete, Action::ScheduleDuplicate, Action::ScheduleEnter, Action::ScheduleSkip,
          Action::CurrencyRename, Action::CurrencyDelete, Action::CurrencySetBase,
          Action::PriceEdit, Action::PriceDelete, Action::PriceUpdate
    };

    foreach (const auto a, disabledActions)
      aC->action(s_Actions.value(a))->setEnabled(false);
  }

  // *************
  // Disabling actions based on conditions
  // *************
  {
    QString tooltip = i18n("Create a new transaction");
    const QVector<QPair<Action, bool>> actionStates {
      {qMakePair(Action::FileOpenDatabase, true)},
      {qMakePair(Action::FileSaveAsDatabase, fileOpen)},
      {qMakePair(Action::FilePersonalData, fileOpen)},
      {qMakePair(Action::FileBackup, (fileOpen && !d->m_myMoneyView->isDatabase()))},
      {qMakePair(Action::FileInformation, fileOpen)},
      {qMakePair(Action::FileImportGNC, !importRunning)},
      {qMakePair(Action::FileImportTemplate, fileOpen && !importRunning)},
      {qMakePair(Action::FileExportTemplate, fileOpen && !importRunning)},
#ifdef KMM_DEBUG
      {qMakePair(Action::FileDump, fileOpen)},
#endif
      {qMakePair(Action::EditFindTransaction, fileOpen)},
      {qMakePair(Action::ToolCurrencies, fileOpen)},
      {qMakePair(Action::ToolPrices, fileOpen)},
      {qMakePair(Action::ToolUpdatePrices, fileOpen)},
      {qMakePair(Action::ToolConsistency, fileOpen)},
      {qMakePair(Action::AccountNew, fileOpen)},
      {qMakePair(Action::AccountCreditTransfer, onlineJobAdministration::instance()->canSendCreditTransfer())},
      {qMakePair(Action::CategoryDelete, fileOpen)},
      {qMakePair(Action::InstitutionNew, fileOpen)},
      {qMakePair(Action::TransactionNew, (fileOpen && d->m_myMoneyView->canCreateTransactions(KMyMoneyRegister::SelectedTransactions(), tooltip)))},
      {qMakePair(Action::ScheduleNew, fileOpen)},
      {qMakePair(Action::CurrencyNew, fileOpen)},
      {qMakePair(Action::PriceNew, fileOpen)},
    };

    foreach (const auto a, actionStates)
      aC->action(s_Actions.value(a.first))->setEnabled(a.second);
  }

  // *************
  // Disabling standard actions based on conditions
  // *************
  aC->action(QString::fromLatin1(KStandardAction::name(KStandardAction::Save)))->setEnabled(modified && !d->m_myMoneyView->isDatabase());
  aC->action(QString::fromLatin1(KStandardAction::name(KStandardAction::SaveAs)))->setEnabled(fileOpen);
  aC->action(QString::fromLatin1(KStandardAction::name(KStandardAction::Close)))->setEnabled(fileOpen);
  aC->action(QString::fromLatin1(KStandardAction::name(KStandardAction::Print)))->setEnabled(fileOpen && d->m_myMoneyView->canPrint());

  aC->action(s_Actions[Action::TransactionMatch])->setText(i18nc("Button text for match transaction", "Match"));
  aC->action(s_Actions[Action::TransactionNew])->setToolTip(i18n("Create a new transaction"));

  w = factory()->container(s_Actions[Action::TransactionMoveMenu], this);
  if (w)
    w->setEnabled(false);

  w = factory()->container(s_Actions[Action::TransactionMarkMenu], this);
  if (w)
    w->setEnabled(false);

  w = factory()->container(s_Actions[Action::TransactionContextMarkMenu], this);
  if (w)
    w->setEnabled(false);

  // *************
  // Enabling actions based on conditions
  // *************

  // FIXME for now it's always on, but we should only allow it, if we
  //       can select at least a single transaction
  aC->action(s_Actions[Action::TransactionSelectAll])->setEnabled(true);
  if (!d->m_selectedTransactions.isEmpty()) {
    // enable 'delete transaction' only if at least one of the
    // selected transactions does not reference a closed account
    bool enable = false;
    KMyMoneyRegister::SelectedTransactions::const_iterator it_t;
    for (it_t = d->m_selectedTransactions.constBegin(); (enable == false) && (it_t != d->m_selectedTransactions.constEnd()); ++it_t) {
      enable = !(*it_t).transaction().id().isEmpty() && !file->referencesClosedAccount((*it_t).transaction());
    }
    aC->action(s_Actions[Action::TransactionDelete])->setEnabled(enable);

    if (!d->m_transactionEditor) {
      QString tooltip = i18n("Duplicate the current selected transactions");
      aC->action(s_Actions[Action::TransactionDuplicate])->setEnabled(d->m_myMoneyView->canDuplicateTransactions(d->m_selectedTransactions, tooltip) && !d->m_selectedTransactions[0].transaction().id().isEmpty());
      aC->action(s_Actions[Action::TransactionDuplicate])->setToolTip(tooltip);

      if (d->m_myMoneyView->canEditTransactions(d->m_selectedTransactions, tooltip)) {
        aC->action(s_Actions[Action::TransactionEdit])->setEnabled(true);
        // editing splits is allowed only if we have one transaction selected
        if (d->m_selectedTransactions.count() == 1) {
          aC->action(s_Actions[Action::TransactionEditSplits])->setEnabled(true);
        }
        if (d->m_selectedAccount.isAssetLiability() && d->m_selectedAccount.accountType() != MyMoneyAccount::Investment) {
          aC->action(s_Actions[Action::TransactionCreateSchedule])->setEnabled(d->m_selectedTransactions.count() == 1);
        }
      }
      aC->action(s_Actions[Action::TransactionEdit])->setToolTip(tooltip);

      if (!d->m_selectedAccount.isClosed()) {
        w = factory()->container(s_Actions[Action::TransactionMoveMenu], this);
        if (w)
          w->setEnabled(true);
      }

      w = factory()->container(s_Actions[Action::TransactionMarkMenu], this);
      if (w)
        w->setEnabled(true);

      w = factory()->container(s_Actions[Action::TransactionContextMarkMenu], this);
      if (w)
        w->setEnabled(true);

      // Allow marking the transaction if at least one is selected
      aC->action(s_Actions[Action::TransactionToggleCleared])->setEnabled(true);
      aC->action(s_Actions[Action::TransactionReconciled])->setEnabled(true);
      aC->action(s_Actions[Action::TransactionNotReconciled])->setEnabled(true);
      aC->action(s_Actions[Action::TransactionToggleReconciled])->setEnabled(true);

      if (!d->m_accountGoto.isEmpty())
        aC->action(s_Actions[Action::TransactionGoToAccount])->setEnabled(true);
      if (!d->m_payeeGoto.isEmpty())
        aC->action(s_Actions[Action::TransactionGoToPayee])->setEnabled(true);

      // Matching is enabled as soon as one regular and one imported transaction is selected
      int matchedCount = 0;
      int importedCount = 0;
      KMyMoneyRegister::SelectedTransactions::const_iterator it;
      for (it = d->m_selectedTransactions.constBegin(); it != d->m_selectedTransactions.constEnd(); ++it) {
        if ((*it).transaction().isImported())
          ++importedCount;
        if ((*it).split().isMatched())
          ++matchedCount;
      }

      if (d->m_selectedTransactions.count() == 2 /* && aC->action(s_Actions[Action::TransactionEdit])->isEnabled() */) {
        aC->action(s_Actions[Action::TransactionMatch])->setEnabled(true);
      }
      if (importedCount != 0 || matchedCount != 0)
        aC->action(s_Actions[Action::TransactionAccept])->setEnabled(true);
      if (matchedCount != 0) {
        aC->action(s_Actions[Action::TransactionMatch])->setEnabled(true);
        aC->action(s_Actions[Action::TransactionMatch])->setText(i18nc("Button text for unmatch transaction", "Unmatch"));
        aC->action(s_Actions[Action::TransactionMatch])->setIcon(QIcon("process-stop"));
      }

      if (d->m_selectedTransactions.count() > 1) {
        aC->action(s_Actions[Action::TransactionCombine])->setEnabled(true);
      }
      if (d->m_selectedTransactions.count() >= 2) {
        int singleSplitTransactions = 0;
        int multipleSplitTransactions = 0;
        foreach (const KMyMoneyRegister::SelectedTransaction& st, d->m_selectedTransactions) {
          switch (st.transaction().splitCount()) {
            case 0:
              break;
            case 1:
              singleSplitTransactions++;
              break;
            default:
              multipleSplitTransactions++;
              break;
          }
        }
        if (singleSplitTransactions > 0 && multipleSplitTransactions == 1) {
          aC->action(s_Actions[Action::TransactionCopySplits])->setEnabled(true);
        }
      }
      if (d->m_selectedTransactions.count() >= 2) {
        int singleSplitTransactions = 0;
        int multipleSplitTransactions = 0;
        foreach(const KMyMoneyRegister::SelectedTransaction& st, d->m_selectedTransactions) {
          switch(st.transaction().splitCount()) {
            case 0:
              break;
            case 1:
              singleSplitTransactions++;
              break;
            default:
              multipleSplitTransactions++;
              break;
          }
        }
        if(singleSplitTransactions > 0 && multipleSplitTransactions == 1) {
          aC->action(s_Actions[Action::TransactionCopySplits])->setEnabled(true);
        }
      }
    } else {
      aC->action(s_Actions[Action::TransactionAssignNumber])->setEnabled(d->m_transactionEditor->canAssignNumber());
      aC->action(s_Actions[Action::TransactionNew])->setEnabled(false);
      aC->action(s_Actions[Action::TransactionDelete])->setEnabled(false);
      QString reason;
      aC->action(s_Actions[Action::TransactionEnter])->setEnabled(d->m_transactionEditor->isComplete(reason));
      //FIXME: Port to KDE4
      // the next line somehow worked in KDE3 but does not have
      // any influence under KDE4
      ///  Works for me when 'reason' is set. Allan
      aC->action(s_Actions[Action::TransactionEnter])->setToolTip(reason);
      aC->action(s_Actions[Action::TransactionCancel])->setEnabled(true);
    }
  }

  QList<MyMoneyAccount> accList;
  file->accountList(accList);
  QList<MyMoneyAccount>::const_iterator it_a;
  QMap<QString, KMyMoneyPlugin::OnlinePlugin*>::const_iterator it_p = d->m_onlinePlugins.constEnd();
  for (it_a = accList.constBegin(); (it_p == d->m_onlinePlugins.constEnd()) && (it_a != accList.constEnd()); ++it_a) {
    if (!(*it_a).onlineBankingSettings().value("provider").isEmpty()) {
      // check if provider is available
      it_p = d->m_onlinePlugins.constFind((*it_a).onlineBankingSettings().value("provider"));
      if (it_p != d->m_onlinePlugins.constEnd()) {
        QStringList protocols;
        (*it_p)->protocols(protocols);
        if (protocols.count() > 0) {
          aC->action(s_Actions[Action::AccountUpdateAll])->setEnabled(true);
          aC->action(s_Actions[Action::AccountUpdateMenu])->setEnabled(true);
        }
      }
    }
  }
  MyMoneyFileBitArray skip(IMyMoneyStorage::MaxRefCheckBits);
  if (!d->m_selectedAccount.id().isEmpty()) {
    if (!file->isStandardAccount(d->m_selectedAccount.id())) {
      switch (d->m_selectedAccount.accountGroup()) {
        case MyMoneyAccount::Asset:
        case MyMoneyAccount::Liability:
        case MyMoneyAccount::Equity:
          aC->action(s_Actions[Action::AccountTransactionReport])->setEnabled(true);
          aC->action(s_Actions[Action::AccountEdit])->setEnabled(true);
          aC->action(s_Actions[Action::AccountDelete])->setEnabled(!file->isReferenced(d->m_selectedAccount));
          aC->action(s_Actions[Action::AccountOpen])->setEnabled(true);
          if (d->m_selectedAccount.accountGroup() != MyMoneyAccount::Equity) {
            if (d->m_reconciliationAccount.id().isEmpty()) {
              aC->action(s_Actions[Action::AccountStartReconciliation])->setEnabled(true);
              aC->action(s_Actions[Action::AccountStartReconciliation])->setToolTip(i18n("Reconcile"));
            } else {
              QString tip;
              tip = i18n("Reconcile - disabled because you are currently reconciling <b>%1</b>", d->m_reconciliationAccount.name());
              aC->action(s_Actions[Action::AccountStartReconciliation])->setToolTip(tip);
              if (!d->m_transactionEditor) {
                aC->action(s_Actions[Action::AccountFinishReconciliation])->setEnabled(d->m_selectedAccount.id() == d->m_reconciliationAccount.id());
                aC->action(s_Actions[Action::AccountPostponeReconciliation])->setEnabled(d->m_selectedAccount.id() == d->m_reconciliationAccount.id());
              }
            }
          }

          if (d->m_selectedAccount.accountType() == MyMoneyAccount::Investment)
            aC->action(s_Actions[Action::InvestmentNew])->setEnabled(true);

          if (d->m_selectedAccount.isClosed())
            aC->action(s_Actions[Action::AccountReopen])->setEnabled(true);
          else enableCloseAccountAction(d->m_selectedAccount);

          if (!d->m_selectedAccount.onlineBankingSettings().value("provider").isEmpty()) {
            aC->action(s_Actions[Action::AccountOnlineUnmap])->setEnabled(true);
            // check if provider is available
            QMap<QString, KMyMoneyPlugin::OnlinePlugin*>::const_iterator it_p;
            it_p = d->m_onlinePlugins.constFind(d->m_selectedAccount.onlineBankingSettings().value("provider"));
            if (it_p != d->m_onlinePlugins.constEnd()) {
              QStringList protocols;
              (*it_p)->protocols(protocols);
              if (protocols.count() > 0) {
                aC->action(s_Actions[Action::AccountUpdate])->setEnabled(true);
                aC->action(s_Actions[Action::AccountUpdateMenu])->setEnabled(true);
              }
            }
          } else {
            aC->action(s_Actions[Action::AccountOnlineMap])->setEnabled(d->m_onlinePlugins.count() > 0);
          }

          aC->action(s_Actions[Action::AccountBalanceChart])->setEnabled(true);
          break;

        case MyMoneyAccount::Income :
        case MyMoneyAccount::Expense :
          aC->action(s_Actions[Action::CategoryEdit])->setEnabled(true);
          // enable delete action, if category/account itself is not referenced
          // by any object except accounts, because we want to allow
          // deleting of sub-categories. Also, we allow transactions, schedules and budgets
          // to be present because we can re-assign them during the delete process
          skip.fill(false);
          skip.setBit(IMyMoneyStorage::RefCheckTransaction);
          skip.setBit(IMyMoneyStorage::RefCheckAccount);
          skip.setBit(IMyMoneyStorage::RefCheckSchedule);
          skip.setBit(IMyMoneyStorage::RefCheckBudget);
          aC->action(s_Actions[Action::CategoryDelete])->setEnabled(!file->isReferenced(d->m_selectedAccount, skip));
          aC->action(s_Actions[Action::AccountOpen])->setEnabled(true);
          break;

        default:
          break;
      }
    }
  }

  if (!d->m_selectedInstitution.id().isEmpty()) {
    aC->action(s_Actions[Action::InstitutionEdit])->setEnabled(true);
    aC->action(s_Actions[Action::InstitutionDelete])->setEnabled(!file->isReferenced(d->m_selectedInstitution));
  }

  if (!d->m_selectedInvestment.id().isEmpty()) {
    aC->action(s_Actions[Action::InvestmentEdit])->setEnabled(true);
    aC->action(s_Actions[Action::InvestmentDelete])->setEnabled(!file->isReferenced(d->m_selectedInvestment));
    aC->action(s_Actions[Action::InvestmentManualPrice])->setEnabled(true);
    try {
      MyMoneySecurity security = MyMoneyFile::instance()->security(d->m_selectedInvestment.currencyId());
      if (!security.value("kmm-online-source").isEmpty())
        aC->action(s_Actions[Action::InvestmentOnlinePrice])->setEnabled(true);

    } catch (const MyMoneyException &e) {
      qDebug("Error retrieving security for investment %s: %s", qPrintable(d->m_selectedInvestment.name()), qPrintable(e.what()));
    }
    if (d->m_selectedInvestment.isClosed())
      aC->action(s_Actions[Action::AccountReopen])->setEnabled(true);
    else enableCloseAccountAction(d->m_selectedInvestment);
  }

  if (!d->m_selectedSchedule.id().isEmpty()) {
    aC->action(s_Actions[Action::ScheduleEdit])->setEnabled(true);
    aC->action(s_Actions[Action::ScheduleDuplicate])->setEnabled(true);
    aC->action(s_Actions[Action::ScheduleDelete])->setEnabled(!file->isReferenced(d->m_selectedSchedule));
    if (!d->m_selectedSchedule.isFinished()) {
      aC->action(s_Actions[Action::ScheduleEnter])->setEnabled(true);
      // a schedule with a single occurrence cannot be skipped
      if (d->m_selectedSchedule.occurrence() != MyMoneySchedule::OCCUR_ONCE) {
        aC->action(s_Actions[Action::ScheduleSkip])->setEnabled(true);
      }
    }
  }

  if (d->m_selectedPayees.count() >= 1) {
    aC->action(s_Actions[Action::PayeeRename])->setEnabled(d->m_selectedPayees.count() == 1);
    aC->action(s_Actions[Action::PayeeMerge])->setEnabled(d->m_selectedPayees.count() > 1);
    aC->action(s_Actions[Action::PayeeDelete])->setEnabled(true);
  }

  if (d->m_selectedTags.count() >= 1) {
    aC->action(s_Actions[Action::TagRename])->setEnabled(d->m_selectedTags.count() == 1);
    aC->action(s_Actions[Action::TagDelete])->setEnabled(true);
  }

  if (d->m_selectedBudgets.count() >= 1) {
    aC->action(s_Actions[Action::BudgetDelete])->setEnabled(true);
    if (d->m_selectedBudgets.count() == 1) {
      aC->action(s_Actions[Action::BudgetChangeYear])->setEnabled(true);
      aC->action(s_Actions[Action::BudgetCopy])->setEnabled(true);
      aC->action(s_Actions[Action::BudgetRename])->setEnabled(true);
      aC->action(s_Actions[Action::BudgetForecast])->setEnabled(true);
    }
  }

  if (!d->m_selectedCurrency.id().isEmpty()) {
    aC->action(s_Actions[Action::CurrencyRename])->setEnabled(true);
    // no need to check each transaction. accounts are enough in this case
    skip.fill(false);
    skip.setBit(IMyMoneyStorage::RefCheckTransaction);
    aC->action(s_Actions[Action::CurrencyDelete])->setEnabled(!file->isReferenced(d->m_selectedCurrency, skip));
    if (d->m_selectedCurrency.id() != file->baseCurrency().id())
      aC->action(s_Actions[Action::CurrencySetBase])->setEnabled(true);
  }

  if (!d->m_selectedPrice.from().isEmpty() && d->m_selectedPrice.source() != QLatin1String("KMyMoney")) {
    aC->action(s_Actions[Action::PriceEdit])->setEnabled(true);
    aC->action(s_Actions[Action::PriceDelete])->setEnabled(true);

    //enable online update if it is a currency
    MyMoneySecurity security = MyMoneyFile::instance()->security(d->m_selectedPrice.from());
    aC->action(s_Actions[Action::PriceUpdate])->setEnabled(security.isCurrency());
  }
}

void KMyMoneyApp::slotResetSelections()
{
  slotSelectAccount();
  slotSelectInstitution();
  slotSelectInvestment();
  slotSelectSchedule();
  slotSelectCurrency();
  slotSelectPrice();
  slotSelectPayees(QList<MyMoneyPayee>());
  slotSelectTags(QList<MyMoneyTag>());
  slotSelectBudget(QList<MyMoneyBudget>());
  slotSelectTransactions(KMyMoneyRegister::SelectedTransactions());
  slotUpdateActions();
}

void KMyMoneyApp::slotSelectCurrency()
{
  slotSelectCurrency(MyMoneySecurity());
}

void KMyMoneyApp::slotSelectCurrency(const MyMoneySecurity& currency)
{
  d->m_selectedCurrency = currency;
  slotUpdateActions();
  emit currencySelected(d->m_selectedCurrency);
}

void KMyMoneyApp::slotSelectPrice()
{
  slotSelectPrice(MyMoneyPrice());
}

void KMyMoneyApp::slotSelectPrice(const MyMoneyPrice& price)
{
  d->m_selectedPrice = price;
  slotUpdateActions();
  emit priceSelected(d->m_selectedPrice);
}

void KMyMoneyApp::slotSelectBudget(const QList<MyMoneyBudget>& list)
{
  d->m_selectedBudgets = list;
  slotUpdateActions();
  emit budgetSelected(d->m_selectedBudgets);
}

void KMyMoneyApp::slotSelectPayees(const QList<MyMoneyPayee>& list)
{
  d->m_selectedPayees = list;
  slotUpdateActions();
  emit payeesSelected(d->m_selectedPayees);
}

void KMyMoneyApp::slotSelectTags(const QList<MyMoneyTag>& list)
{
  d->m_selectedTags = list;
  slotUpdateActions();
  emit tagsSelected(d->m_selectedTags);
}

void KMyMoneyApp::slotSelectTransactions(const KMyMoneyRegister::SelectedTransactions& list)
{
  // list can either contain a list of transactions or a single selected scheduled transaction
  // in the latter case, the transaction id is actually the one of the schedule. In order
  // to differentiate between the two, we just ask for the schedule. If we don't find one - because
  // we passed the id of a real transaction - then we know that fact.  We use the schedule here,
  // because the list of schedules is kept in a cache by MyMoneyFile. This way, we save some trips
  // to the backend which we would have to do if we check for the transaction.
  d->m_selectedTransactions.clear();
  d->m_selectedSchedule = MyMoneySchedule();

  d->m_accountGoto.clear();
  d->m_payeeGoto.clear();
  if (!list.isEmpty() && !list.first().isScheduled()) {
    d->m_selectedTransactions = list;
    if (list.count() == 1) {
      const MyMoneySplit& sp = d->m_selectedTransactions[0].split();
      if (!sp.payeeId().isEmpty()) {
        try {
          MyMoneyPayee payee = MyMoneyFile::instance()->payee(sp.payeeId());
          if (!payee.name().isEmpty()) {
            d->m_payeeGoto = payee.id();
            QString name = payee.name();
            name.replace(QRegExp("&(?!&)"), "&&");
            actionCollection()->action(s_Actions[Action::TransactionGoToPayee])->setText(i18n("Go to '%1'", name));
          }
        } catch (const MyMoneyException &) {
        }
      }
      try {
        QList<MyMoneySplit>::const_iterator it_s;
        const MyMoneyTransaction& t = d->m_selectedTransactions[0].transaction();
        // search the first non-income/non-expense accunt and use it for the 'goto account'
        const MyMoneySplit& sp = d->m_selectedTransactions[0].split();
        for (it_s = t.splits().begin(); it_s != t.splits().end(); ++it_s) {
          if ((*it_s).id() != sp.id()) {
            MyMoneyAccount acc = MyMoneyFile::instance()->account((*it_s).accountId());
            if (!acc.isIncomeExpense()) {
              // for stock accounts we show the portfolio account
              if (acc.isInvest()) {
                acc = MyMoneyFile::instance()->account(acc.parentAccountId());
              }
              d->m_accountGoto = acc.id();
              QString name = acc.name();
              name.replace(QRegExp("&(?!&)"), "&&");
              actionCollection()->action(s_Actions[Action::TransactionGoToAccount])->setText(i18n("Go to '%1'", name));
              break;
            }
          }
        }
      } catch (const MyMoneyException &) {
      }
    }

    slotUpdateActions();
    emit transactionsSelected(d->m_selectedTransactions);

  } else if (!list.isEmpty()) {
    slotSelectSchedule(MyMoneyFile::instance()->schedule(list.first().scheduleId()));

  } else {
    slotUpdateActions();
  }

  // make sure, we show some neutral menu entry if we don't have an object
  if (d->m_payeeGoto.isEmpty())
    actionCollection()->action(s_Actions[Action::TransactionGoToPayee])->setText(i18n("Go to payee"));
  if (d->m_accountGoto.isEmpty())
    actionCollection()->action(s_Actions[Action::TransactionGoToAccount])->setText(i18n("Go to account"));
}

void KMyMoneyApp::slotSelectInstitution()
{
  slotSelectInstitution(MyMoneyInstitution());
}

void KMyMoneyApp::slotSelectInstitution(const MyMoneyObject& institution)
{
  if (typeid(institution) != typeid(MyMoneyInstitution))
    return;

  d->m_selectedInstitution = dynamic_cast<const MyMoneyInstitution&>(institution);
  // qDebug("slotSelectInstitution('%s')", d->m_selectedInstitution.name().data());
  slotUpdateActions();
  emit institutionSelected(d->m_selectedInstitution);
}

void KMyMoneyApp::slotSelectAccount()
{
  slotSelectAccount(MyMoneyAccount());
}

void KMyMoneyApp::slotSelectAccount(const MyMoneyObject& obj)
{
  if (typeid(obj) != typeid(MyMoneyAccount))
    return;

  d->m_selectedAccount = MyMoneyAccount();
  const MyMoneyAccount& acc = dynamic_cast<const MyMoneyAccount&>(obj);
  if (!acc.isInvest())
    d->m_selectedAccount = acc;

  // qDebug("slotSelectAccount('%s')", d->m_selectedAccount.name().data());
  slotUpdateActions();
  emit accountSelected(d->m_selectedAccount);
}

void KMyMoneyApp::slotSelectInvestment()
{
  slotSelectInvestment(MyMoneyAccount());
}

void KMyMoneyApp::slotSelectInvestment(const MyMoneyObject& obj)
{
  if (typeid(obj) != typeid(MyMoneyAccount))
    return;

  // qDebug("slotSelectInvestment('%s')", account.name().data());
  d->m_selectedInvestment = MyMoneyAccount();
  const MyMoneyAccount& acc = dynamic_cast<const MyMoneyAccount&>(obj);
  if (acc.isInvest())
    d->m_selectedInvestment = acc;

  slotUpdateActions();
  emit investmentSelected(d->m_selectedInvestment);
}

void KMyMoneyApp::slotSelectSchedule()
{
  slotSelectSchedule(MyMoneySchedule());
}

void KMyMoneyApp::slotSelectSchedule(const MyMoneySchedule& schedule)
{
  // qDebug("slotSelectSchedule('%s')", schedule.name().data());
  d->m_selectedSchedule = schedule;
  slotUpdateActions();
  emit scheduleSelected(d->m_selectedSchedule);
}

void KMyMoneyApp::slotDataChanged()
{
  // As this method is called every time the MyMoneyFile instance
  // notifies a modification, it's the perfect place to start the timer if needed
  if (d->m_autoSaveEnabled && !d->m_autoSaveTimer->isActive()) {
    d->m_autoSaveTimer->setSingleShot(true);
    d->m_autoSaveTimer->start(d->m_autoSavePeriod * 60 * 1000);  //miliseconds
  }
  updateCaption();
}

void KMyMoneyApp::slotCurrencyDialog()
{
  QPointer<KCurrencyEditDlg> dlg = new KCurrencyEditDlg(this);
  connect(dlg, SIGNAL(selectObject(MyMoneySecurity)), this, SLOT(slotSelectCurrency(MyMoneySecurity)));
  connect(dlg, SIGNAL(openContextMenu(MyMoneySecurity)), this, SLOT(slotShowCurrencyContextMenu()));
  connect(this, SIGNAL(currencyRename()), dlg, SLOT(slotStartRename()));
  connect(dlg, SIGNAL(updateCurrency(QString,QString,QString)), this, SLOT(slotCurrencyUpdate(QString,QString,QString)));
  connect(this, SIGNAL(currencyCreated(QString)), dlg, SLOT(slotSelectCurrency(QString)));
  connect(dlg, SIGNAL(selectBaseCurrency(MyMoneySecurity)), this, SLOT(slotCurrencySetBase()));

  dlg->exec();
  delete dlg;

  slotSelectCurrency(MyMoneySecurity());
}

void KMyMoneyApp::slotPriceDialog()
{
  QPointer<KMyMoneyPriceDlg> dlg = new KMyMoneyPriceDlg(this);
  connect(dlg, SIGNAL(selectObject(MyMoneyPrice)), this, SLOT(slotSelectPrice(MyMoneyPrice)));
  connect(dlg, SIGNAL(openContextMenu(MyMoneyPrice)), this, SLOT(slotShowPriceContextMenu()));
  connect(this, SIGNAL(priceNew()), dlg, SLOT(slotNewPrice()));
  connect(this, SIGNAL(priceEdit()), dlg, SLOT(slotEditPrice()));
  connect(this, SIGNAL(priceDelete()), dlg, SLOT(slotDeletePrice()));
  connect(this, SIGNAL(priceOnlineUpdate()), dlg, SLOT(slotOnlinePriceUpdate()));
  dlg->exec();
}

void KMyMoneyApp::slotFileConsistencyCheck()
{
  d->consistencyCheck(true);
  updateCaption();
}

void KMyMoneyApp::Private::consistencyCheck(bool alwaysDisplayResult)
{
  KMSTATUS(i18n("Running consistency check..."));

  MyMoneyFileTransaction ft;
  try {
    m_consistencyCheckResult = MyMoneyFile::instance()->consistencyCheck();
    ft.commit();
  } catch (const MyMoneyException &e) {
    m_consistencyCheckResult.append(i18n("Consistency check failed: %1", e.what()));
    // always display the result if the check failed
    alwaysDisplayResult = true;
  }

  // in case the consistency check was OK, we get a single line as result
  // in all errneous cases, we get more than one line and force the
  // display of them.

  if (alwaysDisplayResult || m_consistencyCheckResult.size() > 1) {
    QString msg = i18n("The consistency check has found no issues in your data. Details are presented below.");
    if (m_consistencyCheckResult.size() > 1)
      msg = i18n("The consistency check has found some issues in your data. Details are presented below. Those issues that could not be corrected automatically need to be solved by the user.");
    // install a context menu for the list after the dialog is displayed
    QTimer::singleShot(500, q, SLOT(slotInstallConsistencyCheckContextMenu()));
    KMessageBox::informationList(0, msg, m_consistencyCheckResult, i18n("Consistency check result"));
  }
  // this data is no longer needed
  m_consistencyCheckResult.clear();
}

void KMyMoneyApp::Private::copyConsistencyCheckResults()
{
  QClipboard *clipboard = QApplication::clipboard();
  clipboard->setText(m_consistencyCheckResult.join(QLatin1String("\n")));
}

void KMyMoneyApp::Private::saveConsistencyCheckResults()
{
  QUrl fileUrl = QFileDialog::getSaveFileUrl(q);

  if (!fileUrl.isEmpty()) {
    QFile file(fileUrl.toLocalFile());
    if (file.open(QFile::WriteOnly | QFile::Append | QFile::Text)) {
      QTextStream out(&file);
      out << m_consistencyCheckResult.join(QLatin1String("\n"));
      file.close();
    }
  }
}

void KMyMoneyApp::Private::setCustomColors()
{
  // setup the kmymoney custom colors if needed
  if (KMyMoneyGlobalSettings::useSystemColors()) {
    qApp->setStyleSheet(QString());
  } else {
    qApp->setStyleSheet("QTreeView, QTableView#register, QTableView#m_register, QTableView#splittable, QListView { background-color: " +
                        KMyMoneyGlobalSettings::listBGColor().name() + ';' +
                        "alternate-background-color: " + KMyMoneyGlobalSettings::listColor().name() + ';' +
                        "background-clip: content;}");
  }
}

void KMyMoneyApp::slotCheckSchedules()
{
  if (KMyMoneyGlobalSettings::checkSchedule() == true) {

    KMSTATUS(i18n("Checking for overdue scheduled transactions..."));
    MyMoneyFile *file = MyMoneyFile::instance();
    QDate checkDate = QDate::currentDate().addDays(KMyMoneyGlobalSettings::checkSchedulePreview());

    QList<MyMoneySchedule> scheduleList =  file->scheduleList();
    QList<MyMoneySchedule>::Iterator it;

    KMyMoneyUtils::EnterScheduleResultCodeE rc = KMyMoneyUtils::Enter;
    for (it = scheduleList.begin(); (it != scheduleList.end()) && (rc != KMyMoneyUtils::Cancel); ++it) {
      // Get the copy in the file because it might be modified by commitTransaction
      MyMoneySchedule schedule = file->schedule((*it).id());

      if (schedule.autoEnter()) {
        try {
          while (!schedule.isFinished() && (schedule.adjustedNextDueDate() <= checkDate)
                 && rc != KMyMoneyUtils::Ignore
                 && rc != KMyMoneyUtils::Cancel) {
            rc = enterSchedule(schedule, true, true);
            schedule = file->schedule((*it).id()); // get a copy of the modified schedule
          }
        } catch (const MyMoneyException &) {
        }
      }
      if (rc == KMyMoneyUtils::Ignore) {
        // if the current schedule was ignored then we must make sure that the user can still enter the next scheduled transaction
        rc = KMyMoneyUtils::Enter;
      }
    }
    updateCaption();
  }
}

void KMyMoneyApp::writeLastUsedDir(const QString& directory)
{
  //get global config object for our app.
  KSharedConfigPtr kconfig = KSharedConfig::openConfig();
  if (kconfig) {
    KConfigGroup grp = kconfig->group("General Options");

    //write path entry, no error handling since its void.
    grp.writeEntry("LastUsedDirectory", directory);
  }
}

void KMyMoneyApp::writeLastUsedFile(const QString& fileName)
{
  //get global config object for our app.
  KSharedConfigPtr kconfig = KSharedConfig::openConfig();
  if (kconfig) {
    KConfigGroup grp = d->m_config->group("General Options");

    // write path entry, no error handling since its void.
    // use a standard string, as fileName could contain a protocol
    // e.g. file:/home/thb/....
    grp.writeEntry("LastUsedFile", fileName);
  }
}

QString KMyMoneyApp::readLastUsedDir() const
{
  QString str;

  //get global config object for our app.
  KSharedConfigPtr kconfig = KSharedConfig::openConfig();
  if (kconfig) {
    KConfigGroup grp = d->m_config->group("General Options");

    //read path entry.  Second parameter is the default if the setting is not found, which will be the default document path.
    str = grp.readEntry("LastUsedDirectory", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
    // if the path stored is empty, we use the default nevertheless
    if (str.isEmpty())
      str = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
  }

  return str;
}

QString KMyMoneyApp::readLastUsedFile() const
{
  QString str;

  // get global config object for our app.
  KSharedConfigPtr kconfig = KSharedConfig::openConfig();
  if (kconfig) {
    KConfigGroup grp = d->m_config->group("General Options");

    // read filename entry.
    str = grp.readEntry("LastUsedFile", "");
  }

  return str;
}

QString KMyMoneyApp::filename() const
{
  return d->m_fileName.url();
}

QList<QString> KMyMoneyApp::instanceList() const
{
  QList<QString> list;
#ifdef KMM_DBUS
  QDBusReply<QStringList> reply = QDBusConnection::sessionBus().interface()->registeredServiceNames();

  if (reply.isValid()) {
    QStringList apps = reply.value();
    QStringList::ConstIterator it;

    // build a list of service names of all running kmymoney applications without this one
    for (it = apps.constBegin(); it != apps.constEnd(); ++it) {
      // please shange this method of creating a list of 'all the other kmymoney instances that are running on the system'
      // since assuming that D-Bus creates service names with org.kde.kmymoney-PID is an observation I don't think that it's documented somwhere
      if ((*it).indexOf("org.kde.kmymoney-") == 0) {
#ifdef _MSC_VER
        uint thisProcPid = _getpid();
#else
        uint thisProcPid = getpid();
#endif
        if ((*it).indexOf(QString("org.kde.kmymoney-%1").arg(thisProcPid)) != 0)
          list += (*it);
      }
    }
  } else {
    qDebug("D-Bus returned the following error while obtaining instances: %s", qPrintable(reply.error().message()));
  }
#endif
  return list;
}

void KMyMoneyApp::slotEquityPriceUpdate()
{
  QPointer<KEquityPriceUpdateDlg> dlg = new KEquityPriceUpdateDlg(this);
  if (dlg->exec() == QDialog::Accepted && dlg != 0)
    dlg->storePrices();
  delete dlg;
}

void KMyMoneyApp::webConnect(const QString& sourceUrl, const QByteArray& asn_id)
{
  //
  // Web connect attempts to go through the known importers and see if the file
  // can be importing using that method.  If so, it will import it using that
  // plugin
  //

  Q_UNUSED(asn_id)

  d->m_importUrlsQueue.enqueue(sourceUrl);

  // only start processing if this is the only import so far
  if (d->m_importUrlsQueue.count() == 1) {
    while (!d->m_importUrlsQueue.isEmpty()) {
      // get the value of the next item from the queue
      // but leave it on the queue for now
      QString url = d->m_importUrlsQueue.head();

      // Bring this window to the forefront.  This method was suggested by
      // Lubos Lunak <l.lunak@suse.cz> of the KDE core development team.
      // TODO: port KF5
      //KStartupInfo::setNewStartupId(this, asn_id);

      // Make sure we have an open file
      if (! d->m_myMoneyView->fileOpen() &&
          KMessageBox::warningContinueCancel(kmymoney, i18n("You must first select a KMyMoney file before you can import a statement.")) == KMessageBox::Continue)
        kmymoney->slotFileOpen();

      // only continue if the user really did open a file.
      if (d->m_myMoneyView->fileOpen()) {
        KMSTATUS(i18n("Importing a statement via Web Connect"));

        // remove the statement files
        d->unlinkStatementXML();

        QMap<QString, KMyMoneyPlugin::ImporterPlugin*>::const_iterator it_plugin = d->m_importerPlugins.constBegin();
        while (it_plugin != d->m_importerPlugins.constEnd()) {
          if ((*it_plugin)->isMyFormat(url)) {
            QList<MyMoneyStatement> statements;
            if (!(*it_plugin)->import(url)) {
              KMessageBox::error(this, i18n("Unable to import %1 using %2 plugin.  The plugin returned the following error: %3", url, (*it_plugin)->formatName(), (*it_plugin)->lastError()), i18n("Importing error"));
            }

            break;
          }
          ++it_plugin;
        }

        // If we did not find a match, try importing it as a KMM statement file,
        // which is really just for testing.  the statement file is not exposed
        // to users.
        if (it_plugin == d->m_importerPlugins.constEnd())
          if (MyMoneyStatement::isStatementFile(url))
            slotStatementImport(url);

      }
      // remove the current processed item from the queue
      d->m_importUrlsQueue.dequeue();
    }
  }
}

void KMyMoneyApp::slotEnableMessages()
{
  KMessageBox::enableAllMessages();
  KMessageBox::information(this, i18n("All messages have been enabled."), i18n("All messages"));
}

void KMyMoneyApp::createInterfaces()
{
  // Sets up the plugin interface
  KMyMoneyPlugin::pluginInterfaces().importInterface = new KMyMoneyPlugin::KMMImportInterface(this, this);
  KMyMoneyPlugin::pluginInterfaces().statementInterface = new KMyMoneyPlugin::KMMStatementInterface(this, this);
  KMyMoneyPlugin::pluginInterfaces().viewInterface = new KMyMoneyPlugin::KMMViewInterface(this, d->m_myMoneyView, this);

  // setup the calendar interface for schedules
  MyMoneySchedule::setProcessingCalendar(this);
}

void KMyMoneyApp::loadPlugins()
{
  Q_ASSERT(!d->m_pluginLoader);
  d->m_pluginLoader = new KMyMoneyPlugin::PluginLoader(this);

  //! @todo Junior Job: Improve the config read system
  KSharedConfigPtr config = KSharedConfig::openConfig();
  KConfigGroup group{ config->group("Plugins") };

  const auto plugins = KPluginLoader::findPlugins("kmymoney");
  d->m_pluginLoader->addPluginInfo(plugins);

  for (const KPluginMetaData & pluginData : plugins) {
    // Only load plugins which are enabled and have the right serviceType. Other serviceTypes are loaded on demand.
    if (isPluginEnabled(pluginData, group))
      slotPluginLoad(pluginData);
  }

  connect(d->m_pluginLoader, &KMyMoneyPlugin::PluginLoader::pluginEnabled, this, &KMyMoneyApp::slotPluginLoad);
  connect(d->m_pluginLoader, &KMyMoneyPlugin::PluginLoader::pluginDisabled, this, &KMyMoneyApp::slotPluginUnload);
}

void KMyMoneyApp::unloadPlugins()
{
  Q_ASSERT(d->m_pluginLoader);
  delete d->m_pluginLoader;
}

inline bool KMyMoneyApp::isPluginEnabled(const KPluginMetaData& metaData, const KConfigGroup& configGroup)
{
  //! @fixme: there is a function in KMyMoneyPlugin::PluginLoader which has to have the same content
  if (metaData.serviceTypes().contains("KMyMoney/Plugin")) {
    const QString keyName{metaData.name() + "Enabled"};
    if (configGroup.hasKey(keyName))
      return configGroup.readEntry(keyName, true);
    return metaData.isEnabledByDefault();
  }
  return false;
}

void KMyMoneyApp::slotPluginLoad(const KPluginMetaData& metaData)
{
  std::unique_ptr<QPluginLoader> loader{new QPluginLoader{metaData.fileName()}};
  QObject* plugin = loader->instance();
  if (!plugin) {
    qWarning("Could not load plugin '%s', error: %s", qPrintable(metaData.fileName()), qPrintable(loader->errorString()));
    return;
  }

  if ( d->m_plugins.contains(metaData.fileName()) ) {
    /** @fixme Handle a reload e.g. objectNames are equal but the object are different (plugin != d->m_plugins[plugin->objectName()])
     *  Also it could be usefull to drop the dependence on objectName()
     */

    /* Note: there is nothing to delete here because if the plugin was loaded already,
       plugin points to the same object as the previously loaded one. */
    return;
  }

  KMyMoneyPlugin::Plugin* kmmPlugin = qobject_cast<KMyMoneyPlugin::Plugin*>(plugin);
  if (!kmmPlugin) {
    qWarning("Could not load plugin '%s'.", qPrintable(metaData.fileName()));
    return;
  }

  // check for online plugin
  KMyMoneyPlugin::OnlinePlugin* op = dynamic_cast<KMyMoneyPlugin::OnlinePlugin *>(plugin);
  // check for extended online plugin
  KMyMoneyPlugin::OnlinePluginExtended* ope = dynamic_cast<KMyMoneyPlugin::OnlinePluginExtended*>(plugin);
  // check for importer plugin
  KMyMoneyPlugin::ImporterPlugin* ip = dynamic_cast<KMyMoneyPlugin::ImporterPlugin *>(plugin);

  // Tell the plugin it is about to get plugged
  kmmPlugin->plug();

  // plug the plugin
  guiFactory()->addClient(kmmPlugin);

  d->m_plugins[metaData.fileName()] = kmmPlugin;

  if (op)
    d->m_onlinePlugins[plugin->objectName()] = op;

  if (ope)
    onlineJobAdministration::instance()->addPlugin(plugin->objectName(), ope);

  if (ip)
    d->m_importerPlugins[plugin->objectName()] = ip;

  slotUpdateActions();
}

void KMyMoneyApp::slotPluginUnload(const KPluginMetaData& metaData)
{
  KMyMoneyPlugin::Plugin* plugin = d->m_plugins[metaData.fileName()];

  // Remove and test if the plugin was actually loaded
  if (!d->m_plugins.remove(metaData.fileName()) || plugin == nullptr)
    return;

  // check for online plugin
  KMyMoneyPlugin::OnlinePlugin* op = dynamic_cast<KMyMoneyPlugin::OnlinePlugin *>(plugin);
  // check for importer plugin
  KMyMoneyPlugin::ImporterPlugin* ip = dynamic_cast<KMyMoneyPlugin::ImporterPlugin *>(plugin);

  // unplug the plugin
  guiFactory()->removeClient(plugin);

  if (op)
    d->m_onlinePlugins.remove(plugin->objectName());

  if (ip)
    d->m_importerPlugins.remove(plugin->objectName());

  plugin->unplug();
  slotUpdateActions();

}

void KMyMoneyApp::slotAutoSave()
{
  if (!d->m_inAutoSaving) {
    // store the focus widget so we can restore it after save
    QPointer<QWidget> focusWidget = qApp->focusWidget();
    d->m_inAutoSaving = true;
    KMSTATUS(i18n("Auto saving..."));

    //calls slotFileSave if needed, and restart the timer
    //it the file is not saved, reinitializes the countdown.
    if (d->m_myMoneyView->dirty() && d->m_autoSaveEnabled) {
      if (!slotFileSave() && d->m_autoSavePeriod > 0) {
        d->m_autoSaveTimer->setSingleShot(true);
        d->m_autoSaveTimer->start(d->m_autoSavePeriod * 60 * 1000);
      }
    }

    d->m_inAutoSaving = false;
    if (focusWidget && focusWidget != qApp->focusWidget()) {
      // we have a valid focus widget so restore it
      focusWidget->setFocus();
    }
  }
}

void KMyMoneyApp::slotDateChanged()
{
  QDateTime dt = QDateTime::currentDateTime();
  QDateTime nextDay(QDate(dt.date().addDays(1)), QTime(0, 0, 0));

  // +1 is to make sure that we're already in the next day when the
  // signal is sent (this way we also avoid setting the timer to 0)
  QTimer::singleShot((dt.secsTo(nextDay) + 1)*1000, this, SLOT(slotDateChanged()));
  d->m_myMoneyView->slotRefreshViews();
}

const MyMoneyAccount& KMyMoneyApp::account(const QString& key, const QString& value) const
{
  QList<MyMoneyAccount> list;
  QList<MyMoneyAccount>::const_iterator it_a;
  MyMoneyFile::instance()->accountList(list);
  QString accId;
  for (it_a = list.constBegin(); it_a != list.constEnd(); ++it_a) {
    // search in the account's kvp container
    const QString& accountKvpValue = (*it_a).value(key);
    // search in the account's online settings kvp container
    const QString& onlineSettingsKvpValue = (*it_a).onlineBankingSettings().value(key);
    if (accountKvpValue.contains(value) || onlineSettingsKvpValue.contains(value)) {
      if(accId.isEmpty()) {
        accId = (*it_a).id();
      }
    }
    if (accountKvpValue == value || onlineSettingsKvpValue == value) {
      accId = (*it_a).id();
      break;
    }
  }

  // return the account found or an empty element
  return MyMoneyFile::instance()->account(accId);
}

void KMyMoneyApp::setAccountOnlineParameters(const MyMoneyAccount& _acc, const MyMoneyKeyValueContainer& kvps)
{
  MyMoneyFileTransaction ft;
  try {
    MyMoneyAccount acc = MyMoneyFile::instance()->account(_acc.id());
    acc.setOnlineBankingSettings(kvps);
    MyMoneyFile::instance()->modifyAccount(acc);
    ft.commit();

  } catch (const MyMoneyException &e) {
    KMessageBox::detailedSorry(0, i18n("Unable to setup online parameters for account '%1'", _acc.name()), e.what());
  }
}

void KMyMoneyApp::slotAccountUnmapOnline()
{
  // no account selected
  if (d->m_selectedAccount.id().isEmpty())
    return;

  // not a mapped account
  if (d->m_selectedAccount.onlineBankingSettings().value("provider").isEmpty())
    return;

  if (KMessageBox::warningYesNo(this, QString("<qt>%1</qt>").arg(i18n("Do you really want to remove the mapping of account <b>%1</b> to an online account? Depending on the details of the online banking method used, this action cannot be reverted.", d->m_selectedAccount.name())), i18n("Remove mapping to online account")) == KMessageBox::Yes) {
    MyMoneyFileTransaction ft;
    try {
      d->m_selectedAccount.setOnlineBankingSettings(MyMoneyKeyValueContainer());
      // delete the kvp that is used in MyMoneyStatementReader too
      // we should really get rid of it, but since I don't know what it
      // is good for, I'll keep it around. (ipwizard)
      d->m_selectedAccount.deletePair("StatementKey");
      MyMoneyFile::instance()->modifyAccount(d->m_selectedAccount);
      ft.commit();
      // The mapping could disable the online task system
      onlineJobAdministration::instance()->updateOnlineTaskProperties();
    } catch (const MyMoneyException &e) {
      KMessageBox::error(this, i18n("Unable to unmap account from online account: %1", e.what()));
    }
  }
}

void KMyMoneyApp::slotAccountMapOnline()
{
  // no account selected
  if (d->m_selectedAccount.id().isEmpty())
    return;

  // already an account mapped
  if (!d->m_selectedAccount.onlineBankingSettings().value("provider").isEmpty())
    return;

  // check if user tries to map a brokerageAccount
  if (d->m_selectedAccount.name().contains(i18n(" (Brokerage)"))) {
    if (KMessageBox::warningContinueCancel(this, i18n("You try to map a brokerage account to an online account. This is usually not advisable. In general, the investment account should be mapped to the online account. Please cancel if you intended to map the investment account, continue otherwise"), i18n("Mapping brokerage account")) == KMessageBox::Cancel) {
      return;
    }
  }

  // if we have more than one provider let the user select the current provider
  QString provider;
  QMap<QString, KMyMoneyPlugin::OnlinePlugin*>::const_iterator it_p;
  switch (d->m_onlinePlugins.count()) {
    case 0:
      break;
    case 1:
      provider = d->m_onlinePlugins.begin().key();
      break;
    default: {
        QMenu popup(this);
        popup.setTitle(i18n("Select online banking plugin"));

        // Populate the pick list with all the provider
        for (it_p = d->m_onlinePlugins.constBegin(); it_p != d->m_onlinePlugins.constEnd(); ++it_p) {
          popup.addAction(it_p.key())->setData(it_p.key());
        }

        QAction *item = popup.actions()[0];
        if (item) {
          popup.setActiveAction(item);
        }

        // cancelled
        if ((item = popup.exec(QCursor::pos(), item)) == 0) {
          return;
        }

        provider = item->data().toString();
      }
      break;
  }

  if (provider.isEmpty())
    return;

  // find the provider
  it_p = d->m_onlinePlugins.constFind(provider);
  if (it_p != d->m_onlinePlugins.constEnd()) {
    // plugin found, call it
    MyMoneyKeyValueContainer settings;
    if ((*it_p)->mapAccount(d->m_selectedAccount, settings)) {
      settings["provider"] = provider;
      MyMoneyAccount acc(d->m_selectedAccount);
      acc.setOnlineBankingSettings(settings);
      MyMoneyFileTransaction ft;
      try {
        MyMoneyFile::instance()->modifyAccount(acc);
        ft.commit();
        // The mapping could enable the online task system
        onlineJobAdministration::instance()->updateOnlineTaskProperties();
      } catch (const MyMoneyException &e) {
        KMessageBox::error(this, i18n("Unable to map account to online account: %1", e.what()));
      }
    }
  }
}

void KMyMoneyApp::slotAccountUpdateOnlineAll()
{
  QList<MyMoneyAccount> accList;
  MyMoneyFile::instance()->accountList(accList);
  QList<MyMoneyAccount>::iterator it_a;
  QMap<QString, KMyMoneyPlugin::OnlinePlugin*>::const_iterator it_p;
  d->m_statementResults.clear();
  d->m_collectingStatements = true;

  // remove all those from the list, that don't have a 'provider' or the
  // provider is not currently present
  for (it_a = accList.begin(); it_a != accList.end();) {
    if ((*it_a).onlineBankingSettings().value("provider").isEmpty()
        || d->m_onlinePlugins.find((*it_a).onlineBankingSettings().value("provider")) == d->m_onlinePlugins.end()) {
      it_a = accList.erase(it_a);
    } else
      ++it_a;
  }
  QVector<Action> disabledActions {Action::AccountUpdate, Action::AccountUpdateMenu, Action::AccountUpdateAll};
  foreach (const auto a, disabledActions)
    actionCollection()->action(s_Actions.value(a))->setEnabled(false);

  // now work on the remaining list of accounts
  int cnt = accList.count() - 1;
  for (it_a = accList.begin(); it_a != accList.end(); ++it_a) {
    it_p = d->m_onlinePlugins.constFind((*it_a).onlineBankingSettings().value("provider"));
    (*it_p)->updateAccount(*it_a, cnt != 0);
    --cnt;
  }

  d->m_collectingStatements = false;
  if (!d->m_statementResults.isEmpty())
    KMessageBox::informationList(this, i18n("The statements have been processed with the following results:"), d->m_statementResults, i18n("Statement stats"));

  // re-enable the disabled actions
  slotUpdateActions();
}

void KMyMoneyApp::slotAccountUpdateOnline()
{
  // no account selected
  if (d->m_selectedAccount.id().isEmpty())
    return;

  // no online account mapped
  if (d->m_selectedAccount.onlineBankingSettings().value("provider").isEmpty())
    return;

  QVector<Action> disabledActions {Action::AccountUpdate, Action::AccountUpdateMenu, Action::AccountUpdateAll};
  foreach (const auto a, disabledActions)
    actionCollection()->action(s_Actions.value(a))->setEnabled(false);

  // find the provider
  QMap<QString, KMyMoneyPlugin::OnlinePlugin*>::const_iterator it_p;
  it_p = d->m_onlinePlugins.constFind(d->m_selectedAccount.onlineBankingSettings().value("provider"));
  if (it_p != d->m_onlinePlugins.constEnd()) {
    // plugin found, call it
    d->m_collectingStatements = true;
    d->m_statementResults.clear();
    (*it_p)->updateAccount(d->m_selectedAccount);
    d->m_collectingStatements = false;
    if (!d->m_statementResults.isEmpty())
      KMessageBox::informationList(this, i18n("The statements have been processed with the following results:"), d->m_statementResults, i18n("Statement stats"));
  }

  // re-enable the disabled actions
  slotUpdateActions();
}

void KMyMoneyApp::slotNewOnlineTransfer()
{
  kOnlineTransferForm *transferForm = new kOnlineTransferForm(this);
  if (!d->m_selectedAccount.id().isEmpty()) {
    transferForm->setCurrentAccount(d->m_selectedAccount.id());
  }
  connect(transferForm, SIGNAL(rejected()), transferForm, SLOT(deleteLater()));
  connect(transferForm, SIGNAL(acceptedForSave(onlineJob)), this, SLOT(slotOnlineJobSave(onlineJob)));
  connect(transferForm, SIGNAL(acceptedForSend(onlineJob)), this, SLOT(slotOnlineJobSend(onlineJob)));
  connect(transferForm, SIGNAL(accepted()), transferForm, SLOT(deleteLater()));
  transferForm->show();
}

void KMyMoneyApp::slotEditOnlineJob(const QString jobId)
{
  try {
    const onlineJob constJob = MyMoneyFile::instance()->getOnlineJob(jobId);
    slotEditOnlineJob(constJob);
  } catch (MyMoneyException&) {
    // Prevent a crash in very rare cases
  }
}

void KMyMoneyApp::slotEditOnlineJob(onlineJob job)
{
  try {
    slotEditOnlineJob(onlineJobTyped<creditTransfer>(job));
  } catch (MyMoneyException&) {
  }
}

void KMyMoneyApp::slotEditOnlineJob(const onlineJobTyped<creditTransfer> job)
{
  kOnlineTransferForm *transferForm = new kOnlineTransferForm(this);
  transferForm->setOnlineJob(job);
  connect(transferForm, SIGNAL(rejected()), transferForm, SLOT(deleteLater()));
  connect(transferForm, SIGNAL(acceptedForSave(onlineJob)), this, SLOT(slotOnlineJobSave(onlineJob)));
  connect(transferForm, SIGNAL(acceptedForSend(onlineJob)), this, SLOT(slotOnlineJobSend(onlineJob)));
  connect(transferForm, SIGNAL(accepted()), transferForm, SLOT(deleteLater()));
  transferForm->show();
}

void KMyMoneyApp::slotOnlineJobSave(onlineJob job)
{
  MyMoneyFileTransaction fileTransaction;
  if (job.id() == MyMoneyObject::emptyId())
    MyMoneyFile::instance()->addOnlineJob(job);
  else
    MyMoneyFile::instance()->modifyOnlineJob(job);
  fileTransaction.commit();
}

/** @todo when onlineJob queue is used, continue here */
void KMyMoneyApp::slotOnlineJobSend(onlineJob job)
{
  MyMoneyFileTransaction fileTransaction;
  if (job.id() == MyMoneyObject::emptyId())
    MyMoneyFile::instance()->addOnlineJob(job);
  else
    MyMoneyFile::instance()->modifyOnlineJob(job);
  fileTransaction.commit();

  QList<onlineJob> jobList;
  jobList.append(job);
  slotOnlineJobSend(jobList);
}

void KMyMoneyApp::slotOnlineJobSend(QList<onlineJob> jobs)
{
  MyMoneyFile *const kmmFile = MyMoneyFile::instance();
  QMultiMap<QString, onlineJob> jobsByPlugin;

  // Sort jobs by online plugin & lock them
  foreach (onlineJob job, jobs) {
    Q_ASSERT(job.id() != MyMoneyObject::emptyId());
    // find the provider
    const MyMoneyAccount originAcc = job.responsibleMyMoneyAccount();
    job.setLock();
    job.addJobMessage(onlineJobMessage(onlineJobMessage::debug, "KMyMoneyApp::slotOnlineJobSend", "Added to queue for plugin '" + originAcc.onlineBankingSettings().value("provider") + '\''));
    MyMoneyFileTransaction fileTransaction;
    kmmFile->modifyOnlineJob(job);
    fileTransaction.commit();
    jobsByPlugin.insert(originAcc.onlineBankingSettings().value("provider"), job);
  }

  // Send onlineJobs to plugins
  QList<QString> usedPlugins = jobsByPlugin.keys();
  std::sort(usedPlugins.begin(), usedPlugins.end());
  const QList<QString>::iterator newEnd = std::unique(usedPlugins.begin(), usedPlugins.end());
  usedPlugins.erase(newEnd, usedPlugins.end());

  foreach (const QString& pluginKey, usedPlugins) {
    QMap<QString, KMyMoneyPlugin::OnlinePlugin*>::const_iterator it_p = d->m_onlinePlugins.constFind(pluginKey);

    if (it_p != d->m_onlinePlugins.constEnd()) {
      // plugin found, call it
      KMyMoneyPlugin::OnlinePluginExtended *pluginExt = dynamic_cast< KMyMoneyPlugin::OnlinePluginExtended* >(*it_p);
      if (pluginExt == 0) {
        qWarning("Job given for plugin which is not an extended plugin");
        continue;
      }
      //! @fixme remove debug message
      qDebug() << "Sending " << jobsByPlugin.count(pluginKey) << " job(s) to online plugin " << pluginKey;
      QList<onlineJob> jobsToExecute = jobsByPlugin.values(pluginKey);
      QList<onlineJob> executedJobs = jobsToExecute;
      pluginExt->sendOnlineJob(executedJobs);

      // Save possible changes of the online job and remove lock
      MyMoneyFileTransaction fileTransaction;
      foreach (onlineJob job, executedJobs) {
        fileTransaction.restart();
        job.setLock(false);
        kmmFile->modifyOnlineJob(job);
        fileTransaction.commit();
      }

      if (Q_UNLIKELY(executedJobs.size() != jobsToExecute.size())) {
        // OnlinePlugin did not return all jobs
        qWarning() << "Error saving send online tasks. After restart you should see at minimum all successfully executed jobs marked send. Imperfect plugin: " << pluginExt->objectName();
      }

    } else {
      qWarning() << "Error, got onlineJob for an account without online plugin.";
      /** @FIXME can this actually happen? */
    }
  }
}

void KMyMoneyApp::slotRemoveJob()
{
}

void KMyMoneyApp::slotEditJob()
{
}

void KMyMoneyApp::slotOnlineJobLog()
{
  QStringList jobIds = d->m_myMoneyView->getOnlineJobOutbox()->selectedOnlineJobs();
  slotOnlineJobLog(jobIds);
}

void KMyMoneyApp::slotOnlineJobLog(const QStringList& onlineJobIds)
{
  onlineJobMessagesView *const dialog = new onlineJobMessagesView();
  onlineJobMessagesModel *const model = new onlineJobMessagesModel(dialog);
  model->setOnlineJob(MyMoneyFile::instance()->getOnlineJob(onlineJobIds.first()));
  dialog->setModel(model);
  dialog->setAttribute(Qt::WA_DeleteOnClose);
  dialog->show();
  // Note: Objects are not deleted here, Qt's parent-child system has to do that.
}

void KMyMoneyApp::setHolidayRegion(const QString& holidayRegion)
{
#ifdef KF5Holidays_FOUND
  //since the cost of updating the cache is now not negligible
  //check whether the region has been modified
  if (!d->m_holidayRegion || d->m_holidayRegion->regionCode() != holidayRegion) {
    // Delete the previous holidayRegion before creating a new one.
    delete d->m_holidayRegion;
    // Create a new holidayRegion.
    d->m_holidayRegion = new KHolidays::HolidayRegion(holidayRegion);

    //clear and update the holiday cache
    preloadHolidays();
  }
#else
  Q_UNUSED(holidayRegion);
#endif
}

bool KMyMoneyApp::isProcessingDate(const QDate& date) const
{
#ifdef KF5Holidays_FOUND
  if (!d->m_processingDays.testBit(date.dayOfWeek()))
    return false;
  if (!d->m_holidayRegion || !d->m_holidayRegion->isValid())
    return true;

  //check first whether it's already in cache
  if (d->m_holidayMap.contains(date)) {
    return d->m_holidayMap.value(date, true);
  } else {
    bool processingDay = !d->m_holidayRegion->isHoliday(date);
    d->m_holidayMap.insert(date, processingDay);
    return processingDay;
  }
#else
  Q_UNUSED(date);
  return true;
#endif
}

void KMyMoneyApp::preloadHolidays()
{
#ifdef KF5Holidays_FOUND
  //clear the cache before loading
  d->m_holidayMap.clear();
  //only do this if it is a valid region
  if (d->m_holidayRegion && d->m_holidayRegion->isValid()) {
    //load holidays for the forecast days plus 1 cycle, to be on the safe side
    int forecastDays = KMyMoneyGlobalSettings::forecastDays() + KMyMoneyGlobalSettings::forecastAccountCycle();
    QDate endDate = QDate::currentDate().addDays(forecastDays);

    //look for holidays for the next 2 years as a minimum. That should give a good margin for the cache
    if (endDate < QDate::currentDate().addYears(2))
      endDate = QDate::currentDate().addYears(2);

    KHolidays::Holiday::List holidayList = d->m_holidayRegion->holidays(QDate::currentDate(), endDate);
    KHolidays::Holiday::List::const_iterator holiday_it;
    for (holiday_it = holidayList.constBegin(); holiday_it != holidayList.constEnd(); ++holiday_it) {
      for (QDate holidayDate = (*holiday_it).observedStartDate(); holidayDate <= (*holiday_it).observedEndDate(); holidayDate = holidayDate.addDays(1))
        d->m_holidayMap.insert(holidayDate, false);
    }

    for (QDate date = QDate::currentDate(); date <= endDate; date = date.addDays(1)) {
      //if it is not a processing day, set it to false
      if (!d->m_processingDays.testBit(date.dayOfWeek())) {
        d->m_holidayMap.insert(date, false);
      } else if (!d->m_holidayMap.contains(date)) {
        //if it is not a holiday nor a weekend, it is a processing day
        d->m_holidayMap.insert(date, true);
      }
    }
  }
#endif
}

KMStatus::KMStatus(const QString &text)
{
  m_prevText = kmymoney->slotStatusMsg(text);
}

KMStatus::~KMStatus()
{
  kmymoney->slotStatusMsg(m_prevText);
}

void KMyMoneyApp::Private::unlinkStatementXML()
{
  QDir d(KMyMoneySettings::logPath(), "kmm-statement*");
  for (uint i = 0; i < d.count(); ++i) {
    qDebug("Remove %s", qPrintable(d[i]));
    d.remove(KMyMoneySettings::logPath() + QString("/%1").arg(d[i]));
  }
  m_statementXMLindex = 0;
}

void KMyMoneyApp::Private::closeFile()
{
  q->slotSelectAccount();
  q->slotSelectInstitution();
  q->slotSelectInvestment();
  q->slotSelectSchedule();
  q->slotSelectCurrency();
  q->slotSelectBudget(QList<MyMoneyBudget>());
  q->slotSelectPayees(QList<MyMoneyPayee>());
  q->slotSelectTags(QList<MyMoneyTag>());
  q->slotSelectTransactions(KMyMoneyRegister::SelectedTransactions());

  m_reconciliationAccount = MyMoneyAccount();
  m_myMoneyView->finishReconciliation(m_reconciliationAccount);

  m_myMoneyView->closeFile();
  m_fileName = QUrl();
  q->updateCaption();

  // just create a new balance warning object
  delete m_balanceWarning;
  m_balanceWarning = new KBalanceWarning(q);

  emit q->fileLoaded(m_fileName);
}
