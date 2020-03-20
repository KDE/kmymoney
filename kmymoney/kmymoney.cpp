/***************************************************************************
                          kmymoney.cpp
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes <mte@users.sourceforge.net>
                           (C) 2002-2019 by Thomas Baumgart <tbaumgart@kde.org>
                           (C) 2017-2018 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>

****************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include <config-kmymoney.h>

#include "kmymoney.h"

// ----------------------------------------------------------------------------
// Std C++ / STL Includes

#include <typeinfo>
#include <iostream>
#include <memory>

// ----------------------------------------------------------------------------
// QT Includes

#include <QDir>
#include <QDateTime>         // only for performance tests
#include <QTimer>
#include <QByteArray>
#include <QBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QProgressBar>
#include <QList>
#include <QUrl>
#include <QClipboard>
#include <QKeySequence>
#include <QIcon>
#include <QInputDialog>
#include <QStatusBar>
#include <QPushButton>
#include <QListWidget>
#include <QApplication>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KToolBar>
#include <KMessageBox>
#include <KLocalizedString>
#include <KConfig>
#include <KStandardAction>
#include <KActionCollection>
#include <KTipDialog>
#include <KRun>
#include <KConfigDialog>
#include <KXMLGUIFactory>
#include <KRecentFilesAction>
#include <KRecentDirs>
#include <KProcess>
#include <KAboutApplicationDialog>
#include <KBackup>
#ifdef ENABLE_HOLIDAYS
#include <KHolidays/Holiday>
#include <KHolidays/HolidayRegion>
#endif
#ifdef ENABLE_ACTIVITIES
#include <KActivities/ResourceInstance>
#endif

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneysettings.h"
#include "kmymoneyadaptor.h"

#include "dialogs/settings/ksettingskmymoney.h"
#include "dialogs/kbackupdlg.h"
#include "dialogs/kconfirmmanualenterdlg.h"
#include "dialogs/kmymoneypricedlg.h"
#include "dialogs/kcurrencyeditdlg.h"
#include "dialogs/kequitypriceupdatedlg.h"
#include "dialogs/kmymoneyfileinfodlg.h"
#include "dialogs/knewbankdlg.h"
#include "dialogs/ksaveasquestion.h"
#include "wizards/newinvestmentwizard/knewinvestmentwizard.h"
#include "dialogs/knewaccountdlg.h"
#include "dialogs/editpersonaldatadlg.h"
#include "dialogs/kcurrencycalculator.h"
#include "dialogs/keditscheduledlg.h"
#include "wizards/newloanwizard/keditloanwizard.h"
#include "dialogs/kpayeereassigndlg.h"
#include "dialogs/kcategoryreassigndlg.h"
#include "wizards/endingbalancedlg/kendingbalancedlg.h"
#include "dialogs/kloadtemplatedlg.h"
#include "dialogs/ktemplateexportdlg.h"
#include "dialogs/transactionmatcher.h"
#include "wizards/newuserwizard/knewuserwizard.h"
#include "wizards/newaccountwizard/knewaccountwizard.h"
#include "dialogs/kbalancewarning.h"
#include "widgets/kmymoneyaccountselector.h"
#include "widgets/kmymoneypayeecombo.h"
#include "widgets/amountedit.h"
#include "widgets/kmymoneymvccombo.h"

#include "views/kmymoneyview.h"
#include "models/models.h"
#include "models/accountsmodel.h"
#include "models/equitiesmodel.h"
#include "models/securitiesmodel.h"
#ifdef ENABLE_UNFINISHEDFEATURES
#include "models/ledgermodel.h"
#endif

#include "mymoney/mymoneyobject.h"
#include "mymoney/mymoneyfile.h"
#include "mymoney/mymoneyinstitution.h"
#include "mymoney/mymoneyaccount.h"
#include "mymoney/mymoneyaccountloan.h"
#include "mymoney/mymoneysecurity.h"
#include "mymoney/mymoneypayee.h"
#include "mymoney/mymoneyprice.h"
#include "mymoney/mymoneytag.h"
#include "mymoney/mymoneybudget.h"
#include "mymoney/mymoneyreport.h"
#include "mymoney/mymoneysplit.h"
#include "mymoney/mymoneyutils.h"
#include "mymoney/mymoneystatement.h"
#include "mymoney/mymoneyforecast.h"
#include "mymoney/mymoneytransactionfilter.h"
#include "mymoneyexception.h"


#include "converter/mymoneystatementreader.h"
#include "converter/mymoneytemplate.h"

#include "plugins/interfaces/kmmappinterface.h"
#include "plugins/interfaces/kmmviewinterface.h"
#include "plugins/interfaces/kmmstatementinterface.h"
#include "plugins/interfaces/kmmimportinterface.h"
#include "plugins/interfaceloader.h"
#include "plugins/onlinepluginextended.h"
#include "pluginloader.h"
#include "kmymoneyplugin.h"

#include "tasks/credittransfer.h"

#include "icons/icons.h"

#include "misc/webconnect.h"

#include "storage/mymoneystoragemgr.h"
#include "imymoneystorageformat.h"

#include "transactioneditor.h"
#include <QHBoxLayout>
#include <QFileDialog>

#include "kmymoneyutils.h"
#include "kcreditswindow.h"

#include "ledgerdelegate.h"
#include "storageenums.h"
#include "mymoneyenums.h"
#include "dialogenums.h"
#include "viewenums.h"
#include "menuenums.h"
#include "kmymoneyenums.h"

#include "platformtools.h"
#include "kmm_printer.h"

#ifdef ENABLE_SQLCIPHER
#include "sqlcipher/sqlite3.h"
#endif

#ifdef KMM_DEBUG
#include "mymoney/storage/mymoneystoragedump.h"
#include "mymoneytracer.h"
#endif

using namespace Icons;
using namespace eMenu;

enum backupStateE {
  BACKUP_IDLE = 0,
  BACKUP_MOUNTING,
  BACKUP_COPYING,
  BACKUP_UNMOUNTING
};

class KMyMoneyApp::Private
{
public:
  explicit Private(KMyMoneyApp *app) :
      q(app),
      m_backupState(backupStateE::BACKUP_IDLE),
      m_backupResult(0),
      m_backupMount(0),
      m_ignoreBackupExitCode(false),
      m_myMoneyView(nullptr),
      m_startDialog(false),
      m_progressBar(nullptr),
      m_statusLabel(nullptr),
      m_autoSaveEnabled(true),
      m_autoSaveTimer(nullptr),
      m_progressTimer(nullptr),
      m_autoSavePeriod(0),
      m_inAutoSaving(false),
      m_recentFiles(nullptr),
#ifdef ENABLE_HOLIDAYS
      m_holidayRegion(nullptr),
#endif
#ifdef ENABLE_ACTIVITIES
      m_activityResourceInstance(nullptr),
#endif
      m_applicationIsReady(true),
      m_webConnect(new WebConnect(app)) {
    // since the days of the week are from 1 to 7,
    // and a day of the week is used to index this bit array,
    // resize the array to 8 elements (element 0 is left unused)
    m_processingDays.resize(8);

  }

  void unlinkStatementXML();
  void moveInvestmentTransaction(const QString& fromId,
                                 const QString& toId,
                                 const MyMoneyTransaction& t);

  struct storageInfo {
    eKMyMoney::StorageType type {eKMyMoney::StorageType::None};
    bool isOpened {false};
    QUrl url;
  };

  storageInfo m_storageInfo;
  /**
    * The public interface.
    */
  KMyMoneyApp * const q;

  /** the configuration object of the application */
  KSharedConfigPtr m_config;

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

  bool m_startDialog;
  QString m_mountpoint;

  QProgressBar* m_progressBar;
  QTime         m_lastUpdate;
  QLabel*       m_statusLabel;

  // allows multiple imports to be launched trough web connect and to be executed sequentially
  QQueue<QString> m_importUrlsQueue;

  // This is Auto Saving related
  bool                  m_autoSaveEnabled;
  QTimer*               m_autoSaveTimer;
  QTimer*               m_progressTimer;
  int                   m_autoSavePeriod;
  bool                  m_inAutoSaving;

  // id's that need to be remembered
  QString               m_accountGoto, m_payeeGoto;

  KRecentFilesAction*   m_recentFiles;

#ifdef ENABLE_HOLIDAYS
  // used by the calendar interface for schedules
  KHolidays::HolidayRegion* m_holidayRegion;
#endif

#ifdef ENABLE_ACTIVITIES
  KActivities::ResourceInstance * m_activityResourceInstance;
#endif

  QBitArray             m_processingDays;
  QMap<QDate, bool>     m_holidayMap;
  QStringList           m_consistencyCheckResult;
  bool                  m_applicationIsReady;

  WebConnect*           m_webConnect;

  // methods
  void consistencyCheck(bool alwaysDisplayResults);
  static void setThemedCSS();
  void copyConsistencyCheckResults();
  void saveConsistencyCheckResults();

  void checkAccountName(const MyMoneyAccount& _acc, const QString& name) const
  {
    auto file = MyMoneyFile::instance();
    if (_acc.name() != name) {
      MyMoneyAccount acc(_acc);
      acc.setName(name);
      file->modifyAccount(acc);
    }
  }

  /**
    * This method updates names of currencies from file to localized names
    */
  void updateCurrencyNames()
  {
    auto file = MyMoneyFile::instance();
    MyMoneyFileTransaction ft;

    QList<MyMoneySecurity> storedCurrencies = MyMoneyFile::instance()->currencyList();
    QList<MyMoneySecurity> availableCurrencies = MyMoneyFile::instance()->availableCurrencyList();
    QStringList currencyIDs;

    foreach (auto currency, availableCurrencies)
      currencyIDs.append(currency.id());

    try {
      foreach (auto currency, storedCurrencies) {
        int i = currencyIDs.indexOf(currency.id());
        if (i != -1 && availableCurrencies.at(i).name() != currency.name()) {
          currency.setName(availableCurrencies.at(i).name());
          file->modifyCurrency(currency);
        }
      }
      ft.commit();
    } catch (const MyMoneyException &e) {
      qDebug("Error %s updating currency names", e.what());
    }
  }

  void updateAccountNames()
  {
    // make sure we setup the name of the base accounts in translated form
    try {
      MyMoneyFileTransaction ft;
      const auto file = MyMoneyFile::instance();
      checkAccountName(file->asset(), i18n("Asset"));
      checkAccountName(file->liability(), i18n("Liability"));
      checkAccountName(file->income(), i18n("Income"));
      checkAccountName(file->expense(), i18n("Expense"));
      checkAccountName(file->equity(), i18n("Equity"));
      ft.commit();
    } catch (const MyMoneyException &) {
    }
  }

  void ungetString(QIODevice *qfile, char *buf, int len)
  {
    buf = &buf[len-1];
    while (len--) {
      qfile->ungetChar(*buf--);
    }
  }

  bool applyFileFixes()
  {
    const auto blocked = MyMoneyFile::instance()->blockSignals(true);
    KSharedConfigPtr config = KSharedConfig::openConfig();

    KConfigGroup grp = config->group("General Options");

    // For debugging purposes, we can turn off the automatic fix manually
    // by setting the entry in kmymoneyrc to true
    if (grp.readEntry("SkipFix", false) != true) {
      MyMoneyFileTransaction ft;
      try {
        // Check if we have to modify the file before we allow to work with it
        auto s = MyMoneyFile::instance()->storage();
        while (s->fileFixVersion() < s->currentFixVersion()) {
          qDebug("%s", qPrintable((QString("testing fileFixVersion %1 < %2").arg(s->fileFixVersion()).arg(s->currentFixVersion()))));
          switch (s->fileFixVersion()) {
            case 0:
              fixFile_0();
              s->setFileFixVersion(1);
              break;

            case 1:
              fixFile_1();
              s->setFileFixVersion(2);
              break;

            case 2:
              fixFile_2();
              s->setFileFixVersion(3);
              break;

            case 3:
              fixFile_3();
              s->setFileFixVersion(4);
              break;

            case 4:
              fixFile_4();
              s->setFileFixVersion(5);
              break;

              // add new levels above. Don't forget to increase currentFixVersion() for all
              // the storage backends this fix applies to
            default:
              throw MYMONEYEXCEPTION(QString::fromLatin1("Unknown fix level in input file"));
          }
        }
        ft.commit();
      } catch (const MyMoneyException &) {
        MyMoneyFile::instance()->blockSignals(blocked);
        return false;
      }
    } else {
      qDebug("Skipping automatic transaction fix!");
    }
    MyMoneyFile::instance()->blockSignals(blocked);
    return true;
  }

  void connectStorageToModels()
  {
    const auto file = MyMoneyFile::instance();

    const auto accountsModel = Models::instance()->accountsModel();
    q->connect(file, &MyMoneyFile::objectAdded,    accountsModel, &AccountsModel::slotObjectAdded);
    q->connect(file, &MyMoneyFile::objectModified, accountsModel, &AccountsModel::slotObjectModified);
    q->connect(file, &MyMoneyFile::objectRemoved,  accountsModel, &AccountsModel::slotObjectRemoved);
    q->connect(file, &MyMoneyFile::balanceChanged, accountsModel, &AccountsModel::slotBalanceOrValueChanged);
    q->connect(file, &MyMoneyFile::valueChanged,   accountsModel, &AccountsModel::slotBalanceOrValueChanged);

    const auto institutionsModel = Models::instance()->institutionsModel();
    q->connect(file, &MyMoneyFile::objectAdded,    institutionsModel, &InstitutionsModel::slotObjectAdded);
    q->connect(file, &MyMoneyFile::objectModified, institutionsModel, &InstitutionsModel::slotObjectModified);
    q->connect(file, &MyMoneyFile::objectRemoved,  institutionsModel, &InstitutionsModel::slotObjectRemoved);
    q->connect(file, &MyMoneyFile::balanceChanged, institutionsModel, &AccountsModel::slotBalanceOrValueChanged);
    q->connect(file, &MyMoneyFile::valueChanged,   institutionsModel, &AccountsModel::slotBalanceOrValueChanged);

    const auto equitiesModel = Models::instance()->equitiesModel();
    q->connect(file, &MyMoneyFile::objectAdded,    equitiesModel, &EquitiesModel::slotObjectAdded);
    q->connect(file, &MyMoneyFile::objectModified, equitiesModel, &EquitiesModel::slotObjectModified);
    q->connect(file, &MyMoneyFile::objectRemoved,  equitiesModel, &EquitiesModel::slotObjectRemoved);
    q->connect(file, &MyMoneyFile::balanceChanged, equitiesModel, &EquitiesModel::slotBalanceOrValueChanged);
    q->connect(file, &MyMoneyFile::valueChanged,   equitiesModel, &EquitiesModel::slotBalanceOrValueChanged);

    const auto securitiesModel = Models::instance()->securitiesModel();
    q->connect(file, &MyMoneyFile::objectAdded,    securitiesModel, &SecuritiesModel::slotObjectAdded);
    q->connect(file, &MyMoneyFile::objectModified, securitiesModel, &SecuritiesModel::slotObjectModified);
    q->connect(file, &MyMoneyFile::objectRemoved,  securitiesModel, &SecuritiesModel::slotObjectRemoved);

#ifdef ENABLE_UNFINISHEDFEATURES
    const auto ledgerModel = Models::instance()->ledgerModel();
    q->connect(file, &MyMoneyFile::objectAdded,    ledgerModel, &LedgerModel::slotAddTransaction);
    q->connect(file, &MyMoneyFile::objectModified, ledgerModel, &LedgerModel::slotModifyTransaction);
    q->connect(file, &MyMoneyFile::objectRemoved,  ledgerModel, &LedgerModel::slotRemoveTransaction);

    q->connect(file, &MyMoneyFile::objectAdded,    ledgerModel, &LedgerModel::slotAddSchedule);
    q->connect(file, &MyMoneyFile::objectModified, ledgerModel, &LedgerModel::slotModifySchedule);
    q->connect(file, &MyMoneyFile::objectRemoved,  ledgerModel, &LedgerModel::slotRemoveSchedule);
#endif
  }

  void disconnectStorageFromModels()
  {
    const auto file = MyMoneyFile::instance();
    q->disconnect(file, nullptr, Models::instance()->accountsModel(), nullptr);
    q->disconnect(file, nullptr, Models::instance()->institutionsModel(), nullptr);
    q->disconnect(file, nullptr, Models::instance()->equitiesModel(), nullptr);
    q->disconnect(file, nullptr, Models::instance()->securitiesModel(), nullptr);

#ifdef ENABLE_UNFINISHEDFEATURES
    q->disconnect(file, nullptr, Models::instance()->ledgerModel(), nullptr);
#endif
  }

  bool askAboutSaving()
  {
    const auto isFileNotSaved = q->actionCollection()->action(QString::fromLatin1(KStandardAction::name(KStandardAction::Save)))->isEnabled();
    const auto isNewFileNotSaved = m_storageInfo.isOpened && m_storageInfo.url.isEmpty();
    auto fileNeedsToBeSaved = false;

    if (isFileNotSaved && KMyMoneySettings::autoSaveOnClose()) {
      fileNeedsToBeSaved = true;
    } else if (isFileNotSaved || isNewFileNotSaved) {
      switch (KMessageBox::warningYesNoCancel(q, i18n("The file has been changed, save it?"))) {
        case KMessageBox::ButtonCode::Yes:
          fileNeedsToBeSaved = true;
          break;
        case KMessageBox::ButtonCode::No:
          fileNeedsToBeSaved = false;
          break;
        case KMessageBox::ButtonCode::Cancel:
        default:
          return false;
          break;
      }
    }
    if (fileNeedsToBeSaved) {
      if (isFileNotSaved)
        return q->slotFileSave();
      else if (isNewFileNotSaved)
        return q->slotFileSaveAs();
    }
    return true;
  }

  /**
    * This method attaches an empty storage object to the MyMoneyFile
    * object. It calls removeStorage() to remove a possibly attached
    * storage object.
    */
  void newStorage()
  {
    removeStorage();
    auto file = MyMoneyFile::instance();
    file->attachStorage(new MyMoneyStorageMgr);
  }

  /**
    * This method removes an attached storage from the MyMoneyFile
    * object.
    */
  void removeStorage()
  {
    auto file = MyMoneyFile::instance();
    auto p = file->storage();
    if (p) {
      file->detachStorage(p);
      delete p;
    }
  }

  /**
    * if no base currency is defined, start the dialog and force it to be set
    */
  void selectBaseCurrency()
  {
    auto file = MyMoneyFile::instance();

    // check if we have a base currency. If not, we need to select one
    QString baseId;
    try {
      baseId = MyMoneyFile::instance()->baseCurrency().id();
    } catch (const MyMoneyException &e) {
      qDebug("%s", e.what());
    }

    if (baseId.isEmpty()) {
      QPointer<KCurrencyEditDlg> dlg = new KCurrencyEditDlg(q);
  //    connect(dlg, SIGNAL(selectBaseCurrency(MyMoneySecurity)), this, SLOT(slotSetBaseCurrency(MyMoneySecurity)));
      dlg->exec();
      delete dlg;
    }

    try {
      baseId = MyMoneyFile::instance()->baseCurrency().id();
    } catch (const MyMoneyException &e) {
      qDebug("%s", e.what());
    }

    if (!baseId.isEmpty()) {
      // check that all accounts have a currency
      QList<MyMoneyAccount> list;
      file->accountList(list);
      QList<MyMoneyAccount>::Iterator it;

      // don't forget those standard accounts
      list << file->asset();
      list << file->liability();
      list << file->income();
      list << file->expense();
      list << file->equity();


      for (it = list.begin(); it != list.end(); ++it) {
        QString cid;
        try {
          if (!(*it).currencyId().isEmpty() || (*it).currencyId().length() != 0)
            cid = MyMoneyFile::instance()->currency((*it).currencyId()).id();
        } catch (const MyMoneyException &e) {
          qDebug() << QLatin1String("Account") << (*it).id() << (*it).name() << e.what();
        }

        if (cid.isEmpty()) {
          (*it).setCurrencyId(baseId);
          MyMoneyFileTransaction ft;
          try {
            file->modifyAccount(*it);
            ft.commit();
          } catch (const MyMoneyException &e) {
            qDebug("Unable to setup base currency in account %s (%s): %s", qPrintable((*it).name()), qPrintable((*it).id()), e.what());
          }
        }
      }
    }
  }

  /**
    * Call this to see if the MyMoneyFile contains any unsaved data.
    *
    * @retval true if any data has been modified but not saved
    * @retval false otherwise
    */
  bool dirty()
  {
    if (!m_storageInfo.isOpened)
      return false;

    return MyMoneyFile::instance()->dirty();
  }


  /* DO NOT ADD code to this function or any of it's called ones.
     Instead, create a new function, fixFile_n, and modify the initializeStorage()
     logic above to call it */

  void fixFile_4()
  {
    auto file = MyMoneyFile::instance();
    QList<MyMoneySecurity> currencies = file->currencyList();
    static const QStringList symbols = {  QStringLiteral("XAU"),
                                          QStringLiteral("XPD"),
                                          QStringLiteral("XPT"),
                                          QStringLiteral("XAG") };


    foreach(auto currency, currencies) {
      if (symbols.contains(currency.id())) {
        if (currency.smallestAccountFraction() != currency.smallestCashFraction()) {
          currency.setSmallestAccountFraction(currency.smallestCashFraction());
          file->modifyCurrency(currency);
        }
      }
    }
  }

  void fixFile_3()
  {
    // make sure each storage object contains a (unique) id
    MyMoneyFile::instance()->storageId();
  }

  void fixFile_2()
  {
    auto file = MyMoneyFile::instance();
    MyMoneyTransactionFilter filter;
    filter.setReportAllSplits(false);
    QList<MyMoneyTransaction> transactionList;
    file->transactionList(transactionList, filter);

    // scan the transactions and modify transactions with two splits
    // which reference an account and a category to have the memo text
    // of the account.
    auto count = 0;
    foreach (const auto transaction, transactionList) {
      if (transaction.splitCount() == 2) {
        QString accountId;
        QString categoryId;
        QString accountMemo;
        QString categoryMemo;
        foreach (const auto split, transaction.splits()) {
          auto acc = file->account(split.accountId());
          if (acc.isIncomeExpense()) {
            categoryId = split.id();
            categoryMemo = split.memo();
          } else {
            accountId = split.id();
            accountMemo = split.memo();
          }
        }

        if (!accountId.isEmpty() && !categoryId.isEmpty()
            && accountMemo != categoryMemo) {
          MyMoneyTransaction t(transaction);
          MyMoneySplit s(t.splitById(categoryId));
          s.setMemo(accountMemo);
          t.modifySplit(s);
          file->modifyTransaction(t);
          ++count;
        }
      }
    }
    qDebug("%d transactions fixed in fixFile_2", count);
  }

  void fixFile_1()
  {
    // we need to fix reports. If the account filter list contains
    // investment accounts, we need to add the stock accounts to the list
    // as well if we don't have the expert mode enabled
    if (!KMyMoneySettings::expertMode()) {
      try {
        QList<MyMoneyReport> reports = MyMoneyFile::instance()->reportList();
        QList<MyMoneyReport>::iterator it_r;
        for (it_r = reports.begin(); it_r != reports.end(); ++it_r) {
          QStringList list;
          (*it_r).accounts(list);
          QStringList missing;
          QStringList::const_iterator it_a, it_b;
          for (it_a = list.constBegin(); it_a != list.constEnd(); ++it_a) {
            auto acc = MyMoneyFile::instance()->account(*it_a);
            if (acc.accountType() == eMyMoney::Account::Type::Investment) {
              foreach (const auto accountID, acc.accountList()) {
                if (!list.contains(accountID)) {
                  missing.append(accountID);
                }
              }
            }
          }
          if (!missing.isEmpty()) {
            (*it_r).addAccount(missing);
            MyMoneyFile::instance()->modifyReport(*it_r);
          }
        }
      } catch (const MyMoneyException &) {
      }
    }
  }

  #if 0
  if (!m_accountsView->allItemsSelected())
  {
    // retrieve a list of selected accounts
    QStringList list;
    m_accountsView->selectedItems(list);

    // if we're not in expert mode, we need to make sure
    // that all stock accounts for the selected investment
    // account are also selected
    if (!KMyMoneySettings::expertMode()) {
      QStringList missing;
      QStringList::const_iterator it_a, it_b;
      for (it_a = list.begin(); it_a != list.end(); ++it_a) {
        auto acc = MyMoneyFile::instance()->account(*it_a);
        if (acc.accountType() == Account::Type::Investment) {
          foreach (const auto accountID, acc.accountList()) {
            if (!list.contains(accountID)) {
              missing.append(accountID);
            }
          }
        }
      }
      list += missing;
    }

    m_filter.addAccount(list);
  }

  #endif





  void fixFile_0()
  {
    /* (Ace) I am on a crusade against file fixups.  Whenever we have to fix the
     * file, it is really a warning.  So I'm going to print a debug warning, and
     * then go track them down when I see them to figure out how they got saved
     * out needing fixing anyway.
     */

    auto file = MyMoneyFile::instance();
    QList<MyMoneyAccount> accountList;
    file->accountList(accountList);
    QList<MyMoneyAccount>::Iterator it_a;
    QList<MyMoneySchedule> scheduleList = file->scheduleList();
    QList<MyMoneySchedule>::Iterator it_s;

    MyMoneyAccount equity = file->equity();
    MyMoneyAccount asset = file->asset();
    bool equityListEmpty = equity.accountList().count() == 0;

    for (it_a = accountList.begin(); it_a != accountList.end(); ++it_a) {
      if ((*it_a).accountType() == eMyMoney::Account::Type::Loan
          || (*it_a).accountType() == eMyMoney::Account::Type::AssetLoan) {
        fixLoanAccount_0(*it_a);
      }
      // until early before 0.8 release, the equity account was not saved to
      // the file. If we have an equity account with no sub-accounts but
      // find and equity account that has equity() as it's parent, we reparent
      // this account. Need to move it to asset() first, because otherwise
      // MyMoneyFile::reparent would act as NOP.
      if (equityListEmpty && (*it_a).accountType() == eMyMoney::Account::Type::Equity) {
        if ((*it_a).parentAccountId() == equity.id()) {
          auto acc = *it_a;
          // tricky, force parent account to be empty so that we really
          // can re-parent it
          acc.setParentAccountId(QString());
          file->reparentAccount(acc, equity);
          qDebug() << Q_FUNC_INFO << " fixed account " << acc.id() << " reparented to " << equity.id();
        }
      }
    }

    for (it_s = scheduleList.begin(); it_s != scheduleList.end(); ++it_s) {
      fixSchedule_0(*it_s);
    }

    fixTransactions_0();
  }

  void fixSchedule_0(MyMoneySchedule sched)
  {
    MyMoneyTransaction t = sched.transaction();
    QList<MyMoneySplit> splitList = t.splits();
    QList<MyMoneySplit>::ConstIterator it_s;

    try {
      bool updated = false;
      // Check if the splits contain valid data and set it to
      // be valid.
      for (it_s = splitList.constBegin(); it_s != splitList.constEnd(); ++it_s) {
        // the first split is always the account on which this transaction operates
        // and if the transaction commodity is not set, we take this
        if (it_s == splitList.constBegin() && t.commodity().isEmpty()) {
          qDebug() << Q_FUNC_INFO << " " << t.id() << " has no commodity";
          try {
            auto acc = MyMoneyFile::instance()->account((*it_s).accountId());
            t.setCommodity(acc.currencyId());
            updated = true;
          } catch (const MyMoneyException &) {
          }
        }
        // make sure the account exists. If not, remove the split
        try {
          MyMoneyFile::instance()->account((*it_s).accountId());
        } catch (const MyMoneyException &) {
          qDebug() << Q_FUNC_INFO << " " << sched.id() << " " << (*it_s).id() << " removed, because account '" << (*it_s).accountId() << "' does not exist.";
          t.removeSplit(*it_s);
          updated = true;
        }
        if ((*it_s).reconcileFlag() != eMyMoney::Split::State::NotReconciled) {
          qDebug() << Q_FUNC_INFO << " " << sched.id() << " " << (*it_s).id() << " should be 'not reconciled'";
          MyMoneySplit split = *it_s;
          split.setReconcileDate(QDate());
          split.setReconcileFlag(eMyMoney::Split::State::NotReconciled);
          t.modifySplit(split);
          updated = true;
        }
        // the schedule logic used to operate only on the value field.
        // This is now obsolete.
        if ((*it_s).shares().isZero() && !(*it_s).value().isZero()) {
          MyMoneySplit split = *it_s;
          split.setShares(split.value());
          t.modifySplit(split);
          updated = true;
        }
      }

      // If there have been changes, update the schedule and
      // the engine data.
      if (updated) {
        sched.setTransaction(t);
        MyMoneyFile::instance()->modifySchedule(sched);
      }
    } catch (const MyMoneyException &e) {
      qWarning("Unable to update broken schedule: %s", e.what());
    }
  }

  void fixLoanAccount_0(MyMoneyAccount acc)
  {
    if (acc.value("final-payment").isEmpty()
        || acc.value("term").isEmpty()
        || acc.value("periodic-payment").isEmpty()
        || acc.value("loan-amount").isEmpty()
        || acc.value("interest-calculation").isEmpty()
        || acc.value("schedule").isEmpty()
        || acc.value("fixed-interest").isEmpty()) {
      KMessageBox::information(q,
                               i18n("<p>The account \"%1\" was previously created as loan account but some information is missing.</p><p>The new loan wizard will be started to collect all relevant information.</p><p>Please use KMyMoney version 0.8.7 or later and earlier than version 0.9 to correct the problem.</p>"
                                    , acc.name()),
                               i18n("Account problem"));

      throw MYMONEYEXCEPTION_CSTRING("Fix LoanAccount0 not supported anymore");
    }
  }

  void fixTransactions_0()
  {
    auto file = MyMoneyFile::instance();

    QList<MyMoneySchedule> scheduleList = file->scheduleList();
    MyMoneyTransactionFilter filter;
    filter.setReportAllSplits(false);
    QList<MyMoneyTransaction> transactionList;
    file->transactionList(transactionList, filter);

    QList<MyMoneySchedule>::Iterator it_x;
    QStringList interestAccounts;

    KMSTATUS(i18n("Fix transactions"));
    q->slotStatusProgressBar(0, scheduleList.count() + transactionList.count());

    int cnt = 0;
    // scan the schedules to find interest accounts
    for (it_x = scheduleList.begin(); it_x != scheduleList.end(); ++it_x) {
      MyMoneyTransaction t = (*it_x).transaction();
      QList<MyMoneySplit>::ConstIterator it_s;
      QStringList accounts;
      bool hasDuplicateAccounts = false;

      foreach (const auto split, t.splits()) {
        if (accounts.contains(split.accountId())) {
          hasDuplicateAccounts = true;
          qDebug() << Q_FUNC_INFO << " " << t.id() << " has multiple splits with account " << split.accountId();
        } else {
          accounts << split.accountId();
        }

        if (split.action() == MyMoneySplit::actionName(eMyMoney::Split::Action::Interest)) {
          if (interestAccounts.contains(split.accountId()) == 0) {
            interestAccounts << split.accountId();
          }
        }
      }
      if (hasDuplicateAccounts) {
        fixDuplicateAccounts_0(t);
      }
      ++cnt;
      if (!(cnt % 10))
        q->slotStatusProgressBar(cnt);
    }

    // scan the transactions and modify loan transactions
    for (auto& transaction : transactionList) {
      QString defaultAction;
      QList<MyMoneySplit> splits = transaction.splits();
      QStringList accounts;

      // check if base commodity is set. if not, set baseCurrency
      if (transaction.commodity().isEmpty()) {
        qDebug() << Q_FUNC_INFO << " " << transaction.id() << " has no base currency";
        transaction.setCommodity(file->baseCurrency().id());
        file->modifyTransaction(transaction);
      }

      bool isLoan = false;
      // Determine default action
      if (transaction.splitCount() == 2) {
        // check for transfer
        int accountCount = 0;
        MyMoneyMoney val;
        foreach (const auto split, splits) {
          auto acc = file->account(split.accountId());
          if (acc.accountGroup() == eMyMoney::Account::Type::Asset
              || acc.accountGroup() == eMyMoney::Account::Type::Liability) {
            val = split.value();
            accountCount++;
            if (acc.accountType() == eMyMoney::Account::Type::Loan
                || acc.accountType() == eMyMoney::Account::Type::AssetLoan)
              isLoan = true;
          } else
            break;
        }
        if (accountCount == 2) {
          if (isLoan)
            defaultAction = MyMoneySplit::actionName(eMyMoney::Split::Action::Amortization);
          else
            defaultAction = MyMoneySplit::actionName(eMyMoney::Split::Action::Transfer);
        } else {
          if (val.isNegative())
            defaultAction = MyMoneySplit::actionName(eMyMoney::Split::Action::Withdrawal);
          else
            defaultAction = MyMoneySplit::actionName(eMyMoney::Split::Action::Deposit);
        }
      }

      isLoan = false;
      foreach (const auto split, splits) {
        auto acc = file->account(split.accountId());
        MyMoneyMoney val = split.value();
        if (acc.accountGroup() == eMyMoney::Account::Type::Asset
            || acc.accountGroup() == eMyMoney::Account::Type::Liability) {
          if (!val.isPositive()) {
            defaultAction = MyMoneySplit::actionName(eMyMoney::Split::Action::Withdrawal);
            break;
          } else {
            defaultAction = MyMoneySplit::actionName(eMyMoney::Split::Action::Deposit);
            break;
          }
        }
      }

  #if 0
      // Check for correct actions in transactions referencing credit cards
      bool needModify = false;
      // The action fields are actually not used anymore in the ledger view logic
      // so we might as well skip this whole thing here!
      for (it_s = splits.begin(); needModify == false && it_s != splits.end(); ++it_s) {
        auto acc = file->account((*it_s).accountId());
        MyMoneyMoney val = (*it_s).value();
        if (acc.accountType() == Account::Type::CreditCard) {
          if (val < 0 && (*it_s).action() != MyMoneySplit::actionName(eMyMoney::Split::Action::Withdrawal) && (*it_s).action() != MyMoneySplit::actionName(eMyMoney::Split::Action::Transfer))
            needModify = true;
          if (val >= 0 && (*it_s).action() != MyMoneySplit::actionName(eMyMoney::Split::Action::Deposit) && (*it_s).action() != MyMoneySplit::actionName(eMyMoney::Split::Action::Transfer))
            needModify = true;
        }
      }

      // (Ace) Extended the #endif down to cover this conditional, because as-written
      // it will ALWAYS be skipped.

      if (needModify == true) {
        for (it_s = splits.begin(); it_s != splits.end(); ++it_s) {
          (*it_s).setAction(defaultAction);
          transaction.modifySplit(*it_s);
          file->modifyTransaction(transaction);
        }
        splits = transaction.splits();    // update local copy
        qDebug("Fixed credit card assignment in %s", transaction.id().data());
      }
  #endif

      // Check for correct assignment of ActionInterest in all splits
      // and check if there are any duplicates in this transactions
      for (auto& split : splits) {
        MyMoneyAccount splitAccount = file->account(split.accountId());
        if (!accounts.contains(split.accountId())) {
          accounts << split.accountId();
        }
        // if this split references an interest account, the action
        // must be of type ActionInterest
        if (interestAccounts.contains(split.accountId())) {
          if (split.action() != MyMoneySplit::actionName(eMyMoney::Split::Action::Interest)) {
            qDebug() << Q_FUNC_INFO << " " << transaction.id() << " contains an interest account (" << split.accountId() << ") but does not have ActionInterest";
            split.setAction(MyMoneySplit::actionName(eMyMoney::Split::Action::Interest));
            transaction.modifySplit(split);
            file->modifyTransaction(transaction);
            qDebug("Fixed interest action in %s", qPrintable(transaction.id()));
          }
          // if it does not reference an interest account, it must not be
          // of type ActionInterest
        } else {
          if (split.action() == MyMoneySplit::actionName(eMyMoney::Split::Action::Interest)) {
            qDebug() << Q_FUNC_INFO << " " << transaction.id() << " does not contain an interest account so it should not have ActionInterest";
            split.setAction(defaultAction);
            transaction.modifySplit(split);
            file->modifyTransaction(transaction);
            qDebug("Fixed interest action in %s", qPrintable(transaction.id()));
          }
        }

        // check that for splits referencing an account that has
        // the same currency as the transactions commodity the value
        // and shares field are the same.
        if (transaction.commodity() == splitAccount.currencyId()
            && split.value() != split.shares()) {
          qDebug() << Q_FUNC_INFO << " " << transaction.id() << " " << split.id() << " uses the transaction currency, but shares != value";
          split.setShares(split.value());
          transaction.modifySplit(split);
          file->modifyTransaction(transaction);
        }

        // fix the shares and values to have the correct fraction
        if (!splitAccount.isInvest()) {
          try {
            int fract = splitAccount.fraction();
            if (split.shares() != split.shares().convert(fract)) {
              qDebug("adjusting fraction in %s,%s", qPrintable(transaction.id()), qPrintable(split.id()));
              split.setShares(split.shares().convert(fract));
              split.setValue(split.value().convert(fract));
              transaction.modifySplit(split);
              file->modifyTransaction(transaction);
            }
          } catch (const MyMoneyException &) {
            qDebug("Missing security '%s', split not altered", qPrintable(splitAccount.currencyId()));
          }
        }
      }

      ++cnt;
      if (!(cnt % 10))
        q->slotStatusProgressBar(cnt);
    }

    q->slotStatusProgressBar(-1, -1);
  }

  void fixDuplicateAccounts_0(MyMoneyTransaction& t)
  {
    qDebug("Duplicate account in transaction %s", qPrintable(t.id()));
  }

  /**
    * This method is used to update the caption of the application window.
    * It sets the caption to "filename [modified] - KMyMoney".
    *
    * @param skipActions if true, the actions will not be updated. This
    *                    is usually onyl required by some early calls when
    *                    these widgets are not yet created (the default is false).
    */
  void updateCaption();
  void updateActions();
  bool canFileSaveAs() const;
  bool canUpdateAllAccounts() const;
  void fileAction(eKMyMoney::FileAction action);
};

KMyMoneyApp::KMyMoneyApp(QWidget* parent) :
    KXmlGuiWindow(parent),
    d(new Private(this))
{
#ifdef KMM_DBUS
  new KmymoneyAdaptor(this);
  QDBusConnection::sessionBus().registerObject("/KMymoney", this);
  QDBusConnection::sessionBus().interface()->registerService(
    "org.kde.kmymoney-" + QString::number(platformTools::processId()), QDBusConnectionInterface::DontQueueService);
#endif
  // Register the main engine types used as meta-objects
  qRegisterMetaType<MyMoneyMoney>("MyMoneyMoney");
  qRegisterMetaType<MyMoneySecurity>("MyMoneySecurity");

#ifdef ENABLE_SQLCIPHER
  /* Issues:
   * 1) libsqlite3 loads implicitly before libsqlcipher
   *  thus making the second one loaded but non-functional,
   * 2) libsqlite3 gets linked into kmymoney target implicitly
   *  and it's not possible to unload or unlink it explicitly
   *
   * Solution:
   * Use e.g. dummy sqlite3_key call, so that libsqlcipher gets loaded implicitly before libsqlite3
   * thus making the first one functional.
   *
   * Additional info:
   * 1) loading libsqlcipher explicitly doesn't solve the issue,
   * 2) using sqlite3_key only in sqlstorage plugin doesn't solve the issue,
   * 3) in a separate, minimal test case, loading libsqlite3 explicitly
   *  with QLibrary::ExportExternalSymbolsHint makes libsqlcipher non-functional
  */
  sqlite3_key(nullptr, nullptr, 0);
#endif

  // preset the pointer because we need it during the course of this constructor
  kmymoney = this;
  d->m_config = KSharedConfig::openConfig();

  d->setThemedCSS();

  MyMoneyTransactionFilter::setFiscalYearStart(KMyMoneySettings::firstFiscalMonth(), KMyMoneySettings::firstFiscalDay());

  QFrame* frame = new QFrame;
  frame->setFrameStyle(QFrame::NoFrame);
  // values for margin (11) and spacing(6) taken from KDialog implementation
  QBoxLayout* layout = new QBoxLayout(QBoxLayout::TopToBottom, frame);
  layout->setContentsMargins(2, 2, 2, 2);
  layout->setSpacing(6);

  {
    const auto customIconRelativePath = QString(QStringLiteral("icons/hicolor/16x16/actions/account-add.png"));
#ifndef IS_APPIMAGE
    // find where our custom icons were installed based on an custom icon that we know should exist after installation
    auto customIconAbsolutePath = QStandardPaths::locate(QStandardPaths::AppDataLocation, customIconRelativePath);
    if (customIconAbsolutePath.isEmpty()) {
      qWarning("Custom icons were not found in any of the following QStandardPaths::AppDataLocation:");
      for (const auto &standardPath : QStandardPaths::standardLocations(QStandardPaths::AppDataLocation))
        qWarning() << standardPath;
    }
#else
    // according to https://docs.appimage.org/packaging-guide/ingredients.html#open-source-applications
    // QStandardPaths::AppDataLocation is unreliable on AppImages, so apply workaround here in case we fail to find icons
    QString customIconAbsolutePath;
    const auto appImageAppDataLocation = QString("%1%2%3").arg(QCoreApplication::applicationDirPath(), QString("/../share/kmymoney/"), customIconRelativePath);
    if (QFile::exists(appImageAppDataLocation )) {
      customIconAbsolutePath = appImageAppDataLocation ;
    } else {
      qWarning("Custom icons were not found in the following location:");
      qWarning() << appImageAppDataLocation ;
    }
#endif

    // add our custom icons path to icons search path
    if (!customIconAbsolutePath.isEmpty()) {
      customIconAbsolutePath.chop(customIconRelativePath.length());
      customIconAbsolutePath.append(QLatin1String("icons"));
      auto paths = QIcon::themeSearchPaths();
      paths.append(customIconAbsolutePath);
      QIcon::setThemeSearchPaths(paths);
    }

    #if defined(Q_OS_WIN) || defined(Q_OS_MACOS)
    QString themeName = QLatin1Literal("system");                       // using QIcon::setThemeName on Craft build system causes icons to disappear
    #else
    QString themeName = KMyMoneySettings::iconsTheme();                 // get theme user wants
    #endif
    if (!themeName.isEmpty() && themeName != QLatin1Literal("system"))  // if it isn't default theme then set it
      QIcon::setThemeName(themeName);
    Icons::setIconThemeNames(QIcon::themeName());                       // get whatever theme user ends up with and hope our icon names will fit that theme
  }

  initStatusBar();
  pActions = initActions();
  pMenus = initMenus();

  d->m_myMoneyView = new KMyMoneyView;
  layout->addWidget(d->m_myMoneyView, 10);
  connect(d->m_myMoneyView, &KMyMoneyView::statusMsg, this, &KMyMoneyApp::slotStatusMsg);
  connect(d->m_myMoneyView, &KMyMoneyView::statusProgress, this, &KMyMoneyApp::slotStatusProgressBar);

  // Initialize kactivities resource instance
#ifdef ENABLE_ACTIVITIES
  d->m_activityResourceInstance = new KActivities::ResourceInstance(window()->winId(), this);
#endif

  const auto viewActions = d->m_myMoneyView->actionsToBeConnected();
  actionCollection()->addActions(viewActions.values());
  for (auto it = viewActions.cbegin(); it != viewActions.cend(); ++it)
    pActions.insert(it.key(), it.value());

  ///////////////////////////////////////////////////////////////////
  // call inits to invoke all other construction parts
  readOptions();

  // now initialize the plugin structure
  createInterfaces();
  KMyMoneyPlugin::pluginHandling(KMyMoneyPlugin::Action::Load, pPlugins, this, guiFactory());
  onlineJobAdministration::instance()->setOnlinePlugins(pPlugins.extended);
  d->m_myMoneyView->setOnlinePlugins(pPlugins.online);

  setCentralWidget(frame);

  connect(&d->m_proc, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(slotBackupHandleEvents()));

  // force to show the home page if the file is closed
  connect(pActions[Action::ViewTransactionDetail], &QAction::toggled, d->m_myMoneyView, &KMyMoneyView::slotShowTransactionDetail);

  d->m_backupState = BACKUP_IDLE;

  QLocale locale;
  for (auto const& weekDay: locale.weekdays())
  {
    d->m_processingDays.setBit(static_cast<int>(weekDay));
  }
  d->m_autoSaveTimer = new QTimer(this);
  d->m_progressTimer = new QTimer(this);

  connect(d->m_autoSaveTimer, SIGNAL(timeout()), this, SLOT(slotAutoSave()));
  connect(d->m_progressTimer, SIGNAL(timeout()), this, SLOT(slotStatusProgressDone()));

  // connect the WebConnect server
  connect(d->m_webConnect, &WebConnect::gotUrl, this, &KMyMoneyApp::webConnectUrl);

  // setup the initial configuration
  slotUpdateConfiguration(QString());

  // kickstart date change timer
  slotDateChanged();
  d->fileAction(eKMyMoney::FileAction::Closed);
}

KMyMoneyApp::~KMyMoneyApp()
{
  // delete cached objects since they are in the way
  // when unloading the plugins
  onlineJobAdministration::instance()->clearCaches();

  // we need to unload all plugins before we destroy anything else
  KMyMoneyPlugin::pluginHandling(KMyMoneyPlugin::Action::Unload, pPlugins, this, guiFactory());
  d->removeStorage();

#ifdef ENABLE_HOLIDAYS
  delete d->m_holidayRegion;
#endif

#ifdef ENABLE_ACTIVITIES
  delete d->m_activityResourceInstance;
#endif

  // destroy printer object
  KMyMoneyPrinter::cleanup();

  // make sure all settings are written to disk
  KMyMoneySettings::self()->save();
  delete d;
}

QUrl KMyMoneyApp::lastOpenedURL()
{
  QUrl url = d->m_startDialog ? QUrl() : d->m_storageInfo.url;

  if (!url.isValid()) {
    url = QUrl::fromUserInput(readLastUsedFile());
  }

  ready();

  return url;
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

QHash<eMenu::Menu, QMenu *> KMyMoneyApp::initMenus()
{
  QHash<Menu, QMenu *> lutMenus;
  const QHash<Menu, QString> menuNames {
    {Menu::Institution,             QStringLiteral("institution_context_menu")},
    {Menu::Account,                 QStringLiteral("account_context_menu")},
    {Menu::Schedule,                QStringLiteral("schedule_context_menu")},
    {Menu::Category,                QStringLiteral("category_context_menu")},
    {Menu::Tag,                     QStringLiteral("tag_context_menu")},
    {Menu::Payee,                   QStringLiteral("payee_context_menu")},
    {Menu::Investment,              QStringLiteral("investment_context_menu")},
    {Menu::Transaction,             QStringLiteral("transaction_context_menu")},
    {Menu::MoveTransaction,         QStringLiteral("transaction_move_menu")},
    {Menu::MarkTransaction,         QStringLiteral("transaction_mark_menu")},
    {Menu::MarkTransactionContext,  QStringLiteral("transaction_context_mark_menu")},
    {Menu::OnlineJob,               QStringLiteral("onlinejob_context_menu")}
  };

  for (auto it = menuNames.cbegin(); it != menuNames.cend(); ++it)
    lutMenus.insert(it.key(), qobject_cast<QMenu*>(factory()->container(it.value(), this)));
  return lutMenus;
}

QHash<Action, QAction *> KMyMoneyApp::initActions()
{
  auto aC = actionCollection();

  /* Look-up table for all custom and standard actions.
  It's required for:
  1) building QList with QActions to be added to ActionCollection
  2) adding custom features to QActions like e.g. keyboard shortcut
  */
  QHash<Action, QAction *> lutActions;

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
  lutActions.insert(Action::Print, KStandardAction::print(this, &KMyMoneyApp::slotPrintView, aC));
  KStandardAction::preferences(this, &KMyMoneyApp::slotSettings, aC);

  // *************
  // Adding all actions
  // *************
  {
    // struct for creating useless (unconnected) QAction
    struct actionInfo {
      Action  action;
      QString name;
      QString text;
      Icon    icon;
    };

    const QVector<actionInfo> actionInfos {
      // *************
      // The File menu
      // *************
      {Action::FileBackup,                    QStringLiteral("file_backup"),                    i18n("Backup..."),                                  Icon::Empty},
      {Action::FileImportStatement,           QStringLiteral("file_import_statement"),          i18n("Statement file..."),                          Icon::Empty},
      {Action::FileImportTemplate,            QStringLiteral("file_import_template"),           i18n("Account Template..."),                        Icon::Empty},
      {Action::FileExportTemplate,            QStringLiteral("file_export_template"),           i18n("Account Template..."),                        Icon::Empty},
      {Action::FilePersonalData,              QStringLiteral("view_personal_data"),             i18n("Personal Data..."),                           Icon::UserProperties},
#ifdef KMM_DEBUG
      {Action::FileDump,                      QStringLiteral("file_dump"),                      i18n("Dump Memory"),                                Icon::Empty},
#endif
      {Action::FileInformation,               QStringLiteral("view_file_info"),                 i18n("File-Information..."),                        Icon::DocumentProperties},
      // *************
      // The Edit menu
      // *************
      {Action::EditFindTransaction,           QStringLiteral("edit_find_transaction"),          i18n("Find transaction..."),                        Icon::EditFindTransaction},
      // *************
      // The View menu
      // *************
      {Action::ViewTransactionDetail,         QStringLiteral("view_show_transaction_detail"),   i18n("Show Transaction Detail"),                    Icon::ViewTransactionDetail},
      {Action::ViewHideReconciled,            QStringLiteral("view_hide_reconciled_transactions"), i18n("Hide reconciled transactions"),            Icon::HideReconciled},
      {Action::ViewHideCategories,            QStringLiteral("view_hide_unused_categories"),    i18n("Hide unused categories"),                     Icon::HideCategories},
      {Action::ViewShowAll,                   QStringLiteral("view_show_all_accounts"),         i18n("Show all accounts"),                          Icon::Empty},
      // *********************
      // The institutions menu
      // *********************
      {Action::NewInstitution,                QStringLiteral("institution_new"),                i18n("New institution..."),                         Icon::InstitutionNew},
      {Action::EditInstitution,               QStringLiteral("institution_edit"),               i18n("Edit institution..."),                        Icon::InstitutionEdit},
      {Action::DeleteInstitution,             QStringLiteral("institution_delete"),             i18n("Delete institution..."),                      Icon::InstitutionDelete},
      // *****************
      // The accounts menu
      // *****************
      {Action::NewAccount,                    QStringLiteral("account_new"),                    i18n("New account..."),                             Icon::AccountNew},
      {Action::OpenAccount,                   QStringLiteral("account_open"),                   i18n("Open ledger"),                                Icon::ViewFinancialList},
      {Action::StartReconciliation,           QStringLiteral("account_reconcile"),              i18n("Reconcile..."),                               Icon::Reconcile},
      {Action::FinishReconciliation,          QStringLiteral("account_reconcile_finish"),       i18nc("Finish reconciliation", "Finish"),           Icon::AccountFinishReconciliation},
      {Action::PostponeReconciliation,        QStringLiteral("account_reconcile_postpone"),     i18n("Postpone reconciliation"),                    Icon::MediaPlaybackPause},
      {Action::EditAccount,                   QStringLiteral("account_edit"),                   i18n("Edit account..."),                            Icon::AccountEdit},
      {Action::DeleteAccount,                 QStringLiteral("account_delete"),                 i18n("Delete account..."),                          Icon::AccountDelete},
      {Action::CloseAccount,                  QStringLiteral("account_close"),                  i18n("Close account"),                              Icon::AccountClose},
      {Action::ReopenAccount,                 QStringLiteral("account_reopen"),                 i18n("Reopen account"),                             Icon::AccountReopen},
      {Action::ReportAccountTransactions,     QStringLiteral("account_transaction_report"),     i18n("Transaction report"),                         Icon::ViewFinancialList},
      {Action::ChartAccountBalance,           QStringLiteral("account_chart"),                  i18n("Show balance chart..."),                      Icon::OfficeChartLine},
      {Action::MapOnlineAccount,              QStringLiteral("account_online_map"),             i18n("Map account..."),                             Icon::NewsSubscribe},
      {Action::UnmapOnlineAccount,            QStringLiteral("account_online_unmap"),           i18n("Unmap account..."),                           Icon::NewsUnsubscribe},
      {Action::UpdateAccount,                 QStringLiteral("account_online_update"),          i18n("Update account..."),                          Icon::AccountUpdate},
      {Action::UpdateAllAccounts,             QStringLiteral("account_online_update_all"),      i18n("Update all accounts..."),                     Icon::AccountUpdateAll},
      {Action::AccountCreditTransfer,         QStringLiteral("account_online_new_credit_transfer"), i18n("New credit transfer"),                        Icon::AccountCreditTransfer},
      // *******************
      // The categories menu
      // *******************
      {Action::NewCategory,                   QStringLiteral("category_new"),                   i18n("New category..."),                            Icon::CategoryNew},
      {Action::EditCategory,                  QStringLiteral("category_edit"),                  i18n("Edit category..."),                           Icon::CategoryEdit},
      {Action::DeleteCategory,                QStringLiteral("category_delete"),                i18n("Delete category..."),                         Icon::CategoryDelete},
      // **************
      // The tools menu
      // **************
      {Action::ToolCurrencies,                QStringLiteral("tools_currency_editor"),          i18n("Currencies..."),                              Icon::ViewCurrencyList},
      {Action::ToolPrices,                    QStringLiteral("tools_price_editor"),             i18n("Prices..."),                                  Icon::Empty},
      {Action::ToolUpdatePrices,              QStringLiteral("tools_update_prices"),            i18n("Update Stock and Currency Prices..."),        Icon::ToolUpdatePrices},
      {Action::ToolConsistency,               QStringLiteral("tools_consistency_check"),        i18n("Consistency Check"),                          Icon::Empty},
      {Action::ToolPerformance,               QStringLiteral("tools_performancetest"),          i18n("Performance-Test"),                           Icon::Fork},
      {Action::ToolCalculator,                QStringLiteral("tools_kcalc"),                    i18n("Calculator..."),                              Icon::AccessoriesCalculator},
      // *****************
      // The settings menu
      // *****************
      {Action::SettingsAllMessages,           QStringLiteral("settings_enable_messages"),       i18n("Enable all messages"),                        Icon::Empty},
      // *************
      // The help menu
      // *************
      {Action::HelpShow,                      QStringLiteral("help_show_tip"),                  i18n("&Show tip of the day"),                       Icon::Tip},
      // ***************************
      // Actions w/o main menu entry
      // ***************************
      {Action::NewTransaction,                QStringLiteral("transaction_new"),                i18nc("New transaction button", "New"),             Icon::TransactionNew},
      {Action::EditTransaction,               QStringLiteral("transaction_edit"),               i18nc("Edit transaction button", "Edit"),           Icon::TransactionEdit},
      {Action::EnterTransaction,              QStringLiteral("transaction_enter"),              i18nc("Enter transaction", "Enter"),                Icon::DialogOK},
      {Action::EditSplits,                    QStringLiteral("transaction_editsplits"),         i18nc("Edit split button", "Edit splits"),          Icon::Split},
      {Action::CancelTransaction,             QStringLiteral("transaction_cancel"),             i18nc("Cancel transaction edit", "Cancel"),         Icon::DialogCancel},
      {Action::DeleteTransaction,             QStringLiteral("transaction_delete"),             i18nc("Delete transaction", "Delete"),              Icon::EditDelete},
      {Action::DuplicateTransaction,          QStringLiteral("transaction_duplicate"),          i18nc("Duplicate transaction", "Duplicate"),        Icon::EditCopy},
      {Action::AddReversingTransaction,       QStringLiteral("transaction_add_reversing"),      i18nc("Add reversing transaction", "Add reversing"),Icon::Reverse},
      {Action::MatchTransaction,              QStringLiteral("transaction_match"),              i18nc("Button text for match transaction", "Match"),Icon::TransactionMatch},
      {Action::AcceptTransaction,             QStringLiteral("transaction_accept"),             i18nc("Accept 'imported' and 'matched' transaction", "Accept"), Icon::TransactionAccept},
      {Action::ToggleReconciliationFlag,      QStringLiteral("transaction_mark_toggle"),        i18nc("Toggle reconciliation flag", "Toggle"),     Icon::Empty},
      {Action::MarkCleared,                   QStringLiteral("transaction_mark_cleared"),       i18nc("Mark transaction cleared", "Cleared"),       Icon::Empty},
      {Action::MarkReconciled,                QStringLiteral("transaction_mark_reconciled"),    i18nc("Mark transaction reconciled", "Reconciled"), Icon::Empty},
      {Action::MarkNotReconciled,             QStringLiteral("transaction_mark_notreconciled"), i18nc("Mark transaction not reconciled", "Not reconciled"),     Icon::Empty},
      {Action::SelectAllTransactions,         QStringLiteral("transaction_select_all"),         i18nc("Select all transactions", "Select all"),     Icon::Empty},
      {Action::GoToAccount,                   QStringLiteral("transaction_goto_account"),       i18n("Go to account"),                              Icon::GoJump},
      {Action::GoToPayee,                     QStringLiteral("transaction_goto_payee"),         i18n("Go to payee"),                                Icon::GoJump},
      {Action::NewScheduledTransaction,       QStringLiteral("transaction_create_schedule"),    i18n("Create scheduled transaction..."),            Icon::AppointmentNew},
      {Action::AssignTransactionsNumber,      QStringLiteral("transaction_assign_number"),      i18n("Assign next number"),                         Icon::Empty},
      {Action::CombineTransactions,           QStringLiteral("transaction_combine"),            i18nc("Combine transactions", "Combine"),           Icon::Empty},
      {Action::CopySplits,                    QStringLiteral("transaction_copy_splits"),        i18n("Copy splits"),                                Icon::Empty},
      //Investment
      {Action::NewInvestment,                 QStringLiteral("investment_new"),                 i18n("New investment..."),                          Icon::InvestmentNew},
      {Action::EditInvestment,                QStringLiteral("investment_edit"),                i18n("Edit investment..."),                         Icon::InvestmentEdit},
      {Action::DeleteInvestment,              QStringLiteral("investment_delete"),              i18n("Delete investment..."),                       Icon::InvestmentDelete},
      {Action::UpdatePriceOnline,             QStringLiteral("investment_online_price_update"), i18n("Online price update..."),                     Icon::InvestmentOnlinePrice},
      {Action::UpdatePriceManually,           QStringLiteral("investment_manual_price_update"), i18n("Manual price update..."),                     Icon::Empty},
      //Schedule
      {Action::NewSchedule,                   QStringLiteral("schedule_new"),                   i18n("New scheduled transaction"),                  Icon::AppointmentNew},
      {Action::EditSchedule,                  QStringLiteral("schedule_edit"),                  i18n("Edit scheduled transaction"),                 Icon::DocumentEdit},
      {Action::DeleteSchedule,                QStringLiteral("schedule_delete"),                i18n("Delete scheduled transaction"),               Icon::EditDelete},
      {Action::DuplicateSchedule,             QStringLiteral("schedule_duplicate"),             i18n("Duplicate scheduled transaction"),            Icon::EditCopy},
      {Action::EnterSchedule,                 QStringLiteral("schedule_enter"),                 i18n("Enter next transaction..."),                  Icon::KeyEnter},
      {Action::SkipSchedule,                  QStringLiteral("schedule_skip"),                  i18n("Skip next transaction..."),                   Icon::MediaSeekForward},
      //Payees
      {Action::NewPayee,                      QStringLiteral("payee_new"),                      i18n("New payee"),                                  Icon::ListAddUser},
      {Action::RenamePayee,                   QStringLiteral("payee_rename"),                   i18n("Rename payee"),                               Icon::PayeeRename},
      {Action::DeletePayee,                   QStringLiteral("payee_delete"),                   i18n("Delete payee"),                               Icon::ListRemoveUser},
      {Action::MergePayee,                    QStringLiteral("payee_merge"),                    i18n("Merge payees"),                               Icon::PayeeMerge},
      //Tags
      {Action::NewTag,                        QStringLiteral("tag_new"),                        i18n("New tag"),                                    Icon::ListAddTag},
      {Action::RenameTag,                     QStringLiteral("tag_rename"),                     i18n("Rename tag"),                                 Icon::TagRename},
      {Action::DeleteTag,                     QStringLiteral("tag_delete"),                     i18n("Delete tag"),                                 Icon::ListRemoveTag},
      //debug actions
#ifdef KMM_DEBUG
      {Action::WizardNewUser,                 QStringLiteral("new_user_wizard"),                i18n("Test new feature"),                           Icon::Empty},
      {Action::DebugTraces,                   QStringLiteral("debug_traces"),                   i18n("Debug Traces"),                               Icon::Empty},
#endif
      {Action::DebugTimers,                   QStringLiteral("debug_timers"),                   i18n("Debug Timers"),                               Icon::Empty},
      // onlineJob actions
      {Action::DeleteOnlineJob,               QStringLiteral("onlinejob_delete"),               i18n("Remove credit transfer"),                     Icon::EditDelete},
      {Action::EditOnlineJob,                 QStringLiteral("onlinejob_edit"),                 i18n("Edit credit transfer"),                       Icon::DocumentEdit},
      {Action::LogOnlineJob,                  QStringLiteral("onlinejob_log"),                  i18n("Show log"),                                   Icon::Empty},
    };

    for (const auto& info : actionInfos) {
      auto a = new QAction(this);
      // KActionCollection::addAction by name sets object name anyways,
      // so, as better alternative, set it here right from the start
      a->setObjectName(info.name);
      a->setText(info.text);
      if (info.icon != Icon::Empty) // no need to set empty icon
        a->setIcon(Icons::get(info.icon));
      a->setEnabled(false);
      lutActions.insert(info.action, a);  // store QAction's pointer for later processing
    }
  }

  {
    // List with slots that get connected here. Other slots get connected in e.g. appropriate views
    typedef void(KMyMoneyApp::*KMyMoneyAppFunc)();
    const QHash<eMenu::Action, KMyMoneyAppFunc> actionConnections {
      // *************
      // The File menu
      // *************
//      {Action::FileOpenDatabase,              &KMyMoneyApp::slotOpenDatabase},
//      {Action::FileSaveAsDatabase,            &KMyMoneyApp::slotSaveAsDatabase},
      {Action::FileBackup,                    &KMyMoneyApp::slotBackupFile},
      {Action::FileImportTemplate,            &KMyMoneyApp::slotLoadAccountTemplates},
      {Action::FileExportTemplate,            &KMyMoneyApp::slotSaveAccountTemplates},
      {Action::FilePersonalData,              &KMyMoneyApp::slotFileViewPersonal},
#ifdef KMM_DEBUG
      {Action::FileDump,                      &KMyMoneyApp::slotFileFileInfo},
#endif
      {Action::FileInformation,               &KMyMoneyApp::slotFileInfoDialog},
      // *************
      // The View menu
      // *************
      {Action::ViewTransactionDetail,         &KMyMoneyApp::slotShowTransactionDetail},
      {Action::ViewHideReconciled,            &KMyMoneyApp::slotHideReconciledTransactions},
      {Action::ViewHideCategories,            &KMyMoneyApp::slotHideUnusedCategories},
      {Action::ViewShowAll,                   &KMyMoneyApp::slotShowAllAccounts},
      // **************
      // The tools menu
      // **************
      {Action::ToolCurrencies,                &KMyMoneyApp::slotCurrencyDialog},
      {Action::ToolPrices,                    &KMyMoneyApp::slotPriceDialog},
      {Action::ToolUpdatePrices,              &KMyMoneyApp::slotEquityPriceUpdate},
      {Action::ToolConsistency,               &KMyMoneyApp::slotFileConsistencyCheck},
      {Action::ToolPerformance,               &KMyMoneyApp::slotPerformanceTest},
//      {Action::ToolSQL,                       &KMyMoneyApp::slotGenerateSql},
      {Action::ToolCalculator,                &KMyMoneyApp::slotToolsStartKCalc},
      // *****************
      // The settings menu
      // *****************
      {Action::SettingsAllMessages,           &KMyMoneyApp::slotEnableMessages},
      // *************
      // The help menu
      // *************
      {Action::HelpShow,                      &KMyMoneyApp::slotShowTipOfTheDay},
      // ***************************
      // Actions w/o main menu entry
      // ***************************
      //debug actions
#ifdef KMM_DEBUG
      {Action::WizardNewUser,                 &KMyMoneyApp::slotNewFeature},
      {Action::DebugTraces,                   &KMyMoneyApp::slotToggleTraces},
#endif
      {Action::DebugTimers,                   &KMyMoneyApp::slotToggleTimers},
    };

    for (auto connection = actionConnections.cbegin(); connection != actionConnections.cend(); ++connection)
      connect(lutActions[connection.key()], &QAction::triggered, this, connection.value());
  }

  // *************
  // Setting some of added actions checkable
  // *************
  {
    // Some actions are checkable,
    // so set them here
    const QVector<Action> checkableActions {
      Action::ViewTransactionDetail, Action::ViewHideReconciled, Action::ViewHideCategories,
    #ifdef KMM_DEBUG
          Action::DebugTraces,
          Action::DebugTimers,
    #endif
          Action::ViewShowAll
    };

    for (const auto& it : checkableActions) {
      lutActions[it]->setCheckable(true);
      lutActions[it]->setEnabled(true);
    }
  }

  // *************
  // Setting actions that are always enabled
  // *************
  {
    const QVector<eMenu::Action> alwaysEnabled {
      Action::HelpShow,
      Action::SettingsAllMessages,
      Action::ToolPerformance,
      Action::ToolCalculator
    };
    for (const auto& action : alwaysEnabled) {
      lutActions[action]->setEnabled(true);
    }
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
      {qMakePair(Action::StartReconciliation,         Qt::CTRL + Qt::SHIFT + Qt::Key_R)},
      {qMakePair(Action::NewTransaction,              Qt::CTRL + Qt::Key_Insert)},
      {qMakePair(Action::ToggleReconciliationFlag,    Qt::CTRL + Qt::Key_Space)},
      {qMakePair(Action::MarkCleared,                 Qt::CTRL + Qt::ALT+ Qt::Key_Space)},
      {qMakePair(Action::MarkReconciled,              Qt::CTRL + Qt::SHIFT + Qt::Key_Space)},
      {qMakePair(Action::SelectAllTransactions,       Qt::CTRL + Qt::Key_A)},
#ifdef KMM_DEBUG
      {qMakePair(Action::WizardNewUser,               Qt::CTRL + Qt::Key_G)},
#endif
      {qMakePair(Action::AssignTransactionsNumber,    Qt::CTRL + Qt::SHIFT + Qt::Key_N)}
    };

    for(const auto& it : actionShortcuts)
      aC->setDefaultShortcut(lutActions[it.first], it.second);
  }

  // *************
  // Misc settings
  // *************
  connect(onlineJobAdministration::instance(), &onlineJobAdministration::canSendCreditTransferChanged,  lutActions.value(Action::AccountCreditTransfer), &QAction::setEnabled);

  // Setup transaction detail switch
  lutActions[Action::ViewTransactionDetail]->setChecked(KMyMoneySettings::showRegisterDetailed());
  lutActions[Action::ViewHideReconciled]->setChecked(KMyMoneySettings::hideReconciledTransactions());
  lutActions[Action::ViewHideCategories]->setChecked(KMyMoneySettings::hideUnusedCategory());
  lutActions[Action::ViewShowAll]->setChecked(KMyMoneySettings::showAllAccounts());

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
  auto aboutApp = aC->action(QString::fromLatin1(KStandardAction::name(KStandardAction::AboutApp)));
  aboutApp->disconnect();
  connect(aboutApp, &QAction::triggered, this, &KMyMoneyApp::slotShowCredits);

  QMenu *menuContainer;
  menuContainer = static_cast<QMenu*>(factory()->container(QStringLiteral("import"), this));
  menuContainer->setIcon(Icons::get(Icon::DocumentImport));

  menuContainer = static_cast<QMenu*>(factory()->container(QStringLiteral("export"), this));
  menuContainer->setIcon(Icons::get(Icon::DocumentExport));
  return lutActions;
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
  return pActions[_a]->isChecked();
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


  pActions[Action::ViewHideReconciled]->setChecked(KMyMoneySettings::hideReconciledTransactions());
  pActions[Action::ViewHideCategories]->setChecked(KMyMoneySettings::hideUnusedCategory());

  d->m_recentFiles->loadEntries(d->m_config->group("Recent Files"));

  // Startdialog is written in the settings dialog
  d->m_startDialog = grp.readEntry("StartDialog", true);
}

#ifdef KMM_DEBUG
void KMyMoneyApp::resizeEvent(QResizeEvent* ev)
{
  KMainWindow::resizeEvent(ev);
  d->updateCaption();
}
#endif

bool KMyMoneyApp::queryClose()
{
  if (!isReady())
    return false;

  if (!slotFileClose())
    return false;

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
//  MyMoneyFile::instance()->preloadCache();
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
//  MyMoneyFile::instance()->preloadCache();
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
//  MyMoneyFile::instance()->preloadCache();
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
//  MyMoneyFile::instance()->preloadCache();
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
//  MyMoneyFile::instance()->preloadCache();
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
//  MyMoneyFile::instance()->preloadCache();
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
//  MyMoneyFile::instance()->preloadCache();
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
//  MyMoneyFile::instance()->preloadCache();
}

bool KMyMoneyApp::isDatabase()
{
  return (d->m_storageInfo.isOpened && ((d->m_storageInfo.type == eKMyMoney::StorageType::SQL)));
}

bool KMyMoneyApp::isNativeFile()
{
  return (d->m_storageInfo.isOpened && (d->m_storageInfo.type == eKMyMoney::StorageType::SQL || d->m_storageInfo.type == eKMyMoney::StorageType::XML));
}

bool KMyMoneyApp::fileOpen() const
{
  return d->m_storageInfo.isOpened;
}

KMyMoneyAppCallback KMyMoneyApp::progressCallback()
{
  return &KMyMoneyApp::progressCallback;
}

void KMyMoneyApp::consistencyCheck(bool alwaysDisplayResult)
{
  d->consistencyCheck(alwaysDisplayResult);
}

bool KMyMoneyApp::isImportableFile(const QUrl &url)
{
  bool result = false;

  // Iterate through the plugins and see if there's a loaded plugin who can handle it
  QMap<QString, KMyMoneyPlugin::ImporterPlugin*>::const_iterator it_plugin = pPlugins.importer.constBegin();
  while (it_plugin != pPlugins.importer.constEnd()) {
    if ((*it_plugin)->isMyFormat(url.toLocalFile())) {
      result = true;
      break;
    }
    ++it_plugin;
  }

  // If we did not find a match, try importing it as a KMM statement file,
  // which is really just for testing.  the statement file is not exposed
  // to users.
  if (it_plugin == pPlugins.importer.constEnd())
    if (MyMoneyStatement::isStatementFile(url.path()))
      result = true;

  // Place code here to test for QIF and other locally-supported formats
  // (i.e. not a plugin). If you add them here, be sure to add it to
  // the webConnect function.

  return result;
}

bool KMyMoneyApp::isFileOpenedInAnotherInstance(const QUrl &url)
{
  const auto instances = instanceList();
#ifdef KMM_DBUS
  // check if there are other instances which might have this file open
  for (const auto& instance : instances) {
    QDBusInterface remoteApp(instance, "/KMymoney", "org.kde.kmymoney");
    QDBusReply<QString> reply = remoteApp.call("filename");
    if (!reply.isValid())
      qDebug("D-Bus error while calling app->filename()");
    else if (reply.value() == url.url())
      return true;
  }
#else
  Q_UNUSED(url)
#endif
  return false;
}

void KMyMoneyApp::slotShowTransactionDetail()
{

}

void KMyMoneyApp::slotHideReconciledTransactions()
{
  KMyMoneySettings::setHideReconciledTransactions(pActions[Action::ViewHideReconciled]->isChecked());
  d->m_myMoneyView->slotRefreshViews();
}

void KMyMoneyApp::slotHideUnusedCategories()
{
  KMyMoneySettings::setHideUnusedCategory(pActions[Action::ViewHideCategories]->isChecked());
  d->m_myMoneyView->slotRefreshViews();
}

void KMyMoneyApp::slotShowAllAccounts()
{
  KMyMoneySettings::setShowAllAccounts(pActions[Action::ViewShowAll]->isChecked());
  d->m_myMoneyView->slotRefreshViews();
}

#ifdef KMM_DEBUG
void KMyMoneyApp::slotFileFileInfo()
{
  if (!d->m_storageInfo.isOpened) {
    KMessageBox::information(this, i18n("No KMyMoneyFile open"));
    return;
  }

  QFile g("kmymoney.dump");
  g.open(QIODevice::WriteOnly);
  QDataStream st(&g);
  MyMoneyStorageDump dumper;
  dumper.writeStream(st, MyMoneyFile::instance()->storage());
  g.close();
}

void KMyMoneyApp::slotToggleTraces()
{
  MyMoneyTracer::onOff(pActions[Action::DebugTraces]->isChecked() ? 1 : 0);
}
#endif

void KMyMoneyApp::slotToggleTimers()
{
  extern bool timersOn; // main.cpp

  timersOn = pActions[Action::DebugTimers]->isChecked();
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
    d->m_lastUpdate = QTime::currentTime();

  } else {                                // update
    const auto currentTime = QTime::currentTime();
    // only process painting if last update is at least 200 ms ago
    if (abs(d->m_lastUpdate.msecsTo(currentTime)) > 200) {
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
  if (!d->m_storageInfo.isOpened) {
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
    user.setName(editPersonalDataDlg->userName());
    user.setAddress(editPersonalDataDlg->userStreet());
    user.setCity(editPersonalDataDlg->userTown());
    user.setState(editPersonalDataDlg->userCountry());
    user.setPostcode(editPersonalDataDlg->userPostcode());
    user.setTelephone(editPersonalDataDlg->userTelephone());
    user.setEmail(editPersonalDataDlg->userEmail());
    MyMoneyFileTransaction ft;
    try {
      file->setUser(user);
      ft.commit();
    } catch (const MyMoneyException &e) {
      KMessageBox::information(this, i18n("Unable to store user information: %1", QString::fromLatin1(e.what())));
    }
  }
  delete editPersonalDataDlg;
}

void KMyMoneyApp::slotLoadAccountTemplates()
{
  KMSTATUS(i18n("Importing account templates."));

  QPointer<KLoadTemplateDlg> dlg = new KLoadTemplateDlg();
  if (dlg->exec() == QDialog::Accepted && dlg != 0) {
    MyMoneyFileTransaction ft;
    try {
      // import the account templates
      QList<MyMoneyTemplate> templates = dlg->templates();
      QList<MyMoneyTemplate>::iterator it_t;
      for (it_t = templates.begin(); it_t != templates.end(); ++it_t) {
        (*it_t).importTemplate(progressCallback);
      }
      ft.commit();
    } catch (const MyMoneyException &e) {
      KMessageBox::detailedSorry(this, i18n("Unable to import template(s)"), QString::fromLatin1(e.what()));
    }
  }
  delete dlg;
}

void KMyMoneyApp::slotSaveAccountTemplates()
{
  KMSTATUS(i18n("Exporting account templates."));

  QString savePath = QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/templates/" + QLocale().name();
  QDir templatesDir(savePath);
  if (!templatesDir.exists())
      templatesDir.mkpath(savePath);
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
          templ.exportTemplate(progressCallback);
          templ.saveTemplate(QUrl::fromLocalFile(newName));
      }
      delete dlg;
    }
  }
}

bool KMyMoneyApp::okToWriteFile(const QUrl &url)
{
  Q_UNUSED(url)

  // check if the file exists and warn the user
  bool reallySaveFile = true;

  if (KMyMoneyUtils::fileExists(url)) {
    if (KMessageBox::warningYesNo(this, QLatin1String("<qt>") + i18n("The file <b>%1</b> already exists. Do you really want to overwrite it?", url.toDisplayString(QUrl::PreferLocalFile)) + QLatin1String("</qt>"), i18n("File already exists")) != KMessageBox::Yes)
    reallySaveFile = false;
  }
  return reallySaveFile;
}

void KMyMoneyApp::slotSettings()
{
  // if we already have an instance of the settings dialog, then use it
  if (KConfigDialog::showDialog("KMyMoney-Settings"))
    return;

  // otherwise, we have to create it
  auto dlg = new KSettingsKMyMoney(this, "KMyMoney-Settings", KMyMoneySettings::self());
  connect(dlg, &KSettingsKMyMoney::settingsChanged, this, &KMyMoneyApp::slotUpdateConfiguration);
  dlg->show();
}

void KMyMoneyApp::slotShowCredits()
{
  KAboutData aboutData = initializeCreditsData();
  KAboutApplicationDialog dlg(aboutData, this);
  dlg.exec();
}

void KMyMoneyApp::slotUpdateConfiguration(const QString &dialogName)
{
  if(dialogName.compare(QLatin1String("Plugins")) == 0) {
    KMyMoneyPlugin::pluginHandling(KMyMoneyPlugin::Action::Reorganize, pPlugins, this, guiFactory());
    actionCollection()->action(QString::fromLatin1(KStandardAction::name(KStandardAction::SaveAs)))->setEnabled(d->canFileSaveAs());
    onlineJobAdministration::instance()->updateActions();
    onlineJobAdministration::instance()->setOnlinePlugins(pPlugins.extended);
    d->m_myMoneyView->setOnlinePlugins(pPlugins.online);
    d->updateActions();
    d->m_myMoneyView->slotRefreshViews();
    return;
  }
  MyMoneyTransactionFilter::setFiscalYearStart(KMyMoneySettings::firstFiscalMonth(), KMyMoneySettings::firstFiscalDay());

#ifdef ENABLE_UNFINISHEDFEATURES
  LedgerSeparator::setFirstFiscalDate(KMyMoneySettings::firstFiscalMonth(), KMyMoneySettings::firstFiscalDay());
#endif

  d->m_myMoneyView->updateViewType();

  // update the sql storage module settings
//  MyMoneyStorageSql::setStartDate(KMyMoneySettings::startDate().date());

  // update the report module settings
  MyMoneyReport::setLineWidth(KMyMoneySettings::lineWidth());

  // update the holiday region configuration
  setHolidayRegion(KMyMoneySettings::holidayRegion());

  d->m_myMoneyView->slotRefreshViews();

  // re-read autosave configuration
  d->m_autoSaveEnabled = KMyMoneySettings::autoSaveFile();
  d->m_autoSavePeriod = KMyMoneySettings::autoSavePeriod();

  // stop timer if turned off but running
  if (d->m_autoSaveTimer->isActive() && !d->m_autoSaveEnabled) {
    d->m_autoSaveTimer->stop();
  }
  // start timer if turned on and needed but not running
  if (!d->m_autoSaveTimer->isActive() && d->m_autoSaveEnabled && d->dirty()) {
    d->m_autoSaveTimer->setSingleShot(true);
    d->m_autoSaveTimer->start(d->m_autoSavePeriod * 60 * 1000);
  }

  d->setThemedCSS();
}

void KMyMoneyApp::slotBackupFile()
{
  // Save the file first so isLocalFile() works
  if (d->m_myMoneyView && d->dirty())

  {
    if (KMessageBox::questionYesNo(this, i18n("The file must be saved first "
                                   "before it can be backed up.  Do you want to continue?")) == KMessageBox::No) {
      return;

    }

    slotFileSave();
  }



  if (d->m_storageInfo.url.isEmpty())
    return;

  if (!d->m_storageInfo.url.isLocalFile()) {
    KMessageBox::sorry(this,
                       i18n("The current implementation of the backup functionality only supports local files as source files. Your current source file is '%1'.", d->m_storageInfo.url.url()),

                       i18n("Local files only"));
    return;
  }

  QPointer<KBackupDlg> backupDlg = new KBackupDlg(this);
  int returncode = backupDlg->exec();
  if (returncode == QDialog::Accepted && backupDlg != 0) {


    d->m_backupMount = backupDlg->mountCheckBoxChecked();
    d->m_proc.clearProgram();
    d->m_backupState = BACKUP_MOUNTING;
    d->m_mountpoint = backupDlg->mountPoint();

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
  QFileInfo fi(d->m_storageInfo.url.fileName());
  QString today = QDate::currentDate().toString("-yyyy-MM-dd.") + fi.suffix();
  QString backupfile = d->m_mountpoint + '/' + d->m_storageInfo.url.fileName();
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
  d->m_proc << QDir::toNativeSeparators(d->m_storageInfo.url.toLocalFile()) << "+" << "nul" << QDir::toNativeSeparators(backupfile);
#else
  d->m_proc << "cp" << "-f";
  d->m_proc << d->m_storageInfo.url.toLocalFile() << backupfile;
#endif
  d->m_backupState = BACKUP_COPYING;
  qDebug() << "Backup cmd:" << d->m_proc.program();
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

void KMyMoneyApp::slotViewSelected(View view)
{
  KMyMoneySettings::setLastViewSelected(static_cast<int>(view));
}

void KMyMoneyApp::slotGenerateSql()
{
//  QPointer<KGenerateSqlDlg> editor = new KGenerateSqlDlg(this);
//  editor->setObjectName("Generate Database SQL");
//  editor->exec();
//  delete editor;
}

void KMyMoneyApp::slotToolsStartKCalc()
{
  QString cmd = KMyMoneySettings::externalCalculator();
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

void KMyMoneyApp::createAccount(MyMoneyAccount& newAccount, MyMoneyAccount& parentAccount, MyMoneyAccount& brokerageAccount, MyMoneyMoney openingBal)
{
  MyMoneyFile *file = MyMoneyFile::instance();
  try {
    const MyMoneySecurity& sec = file->security(newAccount.currencyId());
    // Check the opening balance
    if (openingBal.isPositive() && newAccount.accountGroup() == eMyMoney::Account::Type::Liability) {
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
    KMessageBox::information(this, i18n("Unable to add account: %1", QString::fromLatin1(e.what())));
  }
}

void KMyMoneyApp::slotInvestmentNew(MyMoneyAccount& account, const MyMoneyAccount& parent)
{
  KNewInvestmentWizard::newInvestment(account, parent);
}

void KMyMoneyApp::slotCategoryNew(MyMoneyAccount& account, const MyMoneyAccount& parent)
{
  KNewAccountDlg::newCategory(account, parent);
}

void KMyMoneyApp::slotCategoryNew(MyMoneyAccount& account)
{
  KNewAccountDlg::newCategory(account, MyMoneyAccount());
}

void KMyMoneyApp::slotAccountNew(MyMoneyAccount& account)
{
  NewAccountWizard::Wizard::newAccount(account);
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
        throw MYMONEYEXCEPTION_CSTRING("Transaction for schedule has less than 2 splits!");
      }

      MyMoneyFileTransaction ft;
      try {
        file->addSchedule(newSchedule);

        // in case of a loan account, we keep a reference to this
        // schedule in the account
        if (newAccount.accountType() == eMyMoney::Account::Type::Loan
            || newAccount.accountType() == eMyMoney::Account::Type::AssetLoan) {
          newAccount.setValue("schedule", newSchedule.id());
          file->modifyAccount(newAccount);
        }
        ft.commit();
      } catch (const MyMoneyException &e) {
        KMessageBox::information(this, i18n("Unable to add scheduled transaction: %1", QString::fromLatin1(e.what())));
      }
    } catch (const MyMoneyException &e) {
      KMessageBox::information(this, i18n("Unable to add scheduled transaction: %1", QString::fromLatin1(e.what())));
    }
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
    KMessageBox::sorry(this, i18n("<p><b>%1</b> cannot be moved to institution <b>%2</b>. Reason: %3</p>", src.name(), _dst.name(), QString::fromLatin1(e.what())));
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
    KMessageBox::sorry(this, i18n("<p><b>%1</b> cannot be moved to <b>%2</b>. Reason: %3</p>", src.name(), dst.name(), QString::fromLatin1(e.what())));
  }
}

void KMyMoneyApp::slotScheduleNew(const MyMoneyTransaction& _t, eMyMoney::Schedule::Occurrence occurrence)
{
  KEditScheduleDlg::newSchedule(_t, occurrence);
}

void KMyMoneyApp::slotPayeeNew(const QString& newnameBase, QString& id)
{
  KMyMoneyUtils::newPayee(newnameBase, id);
}

void KMyMoneyApp::slotNewFeature()
{
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
  foreach (const auto split, t.splits()) {
    stockAccountId = split.accountId();
    stockSecurityId =
      MyMoneyFile::instance()->account(stockAccountId).currencyId();
    if (!MyMoneyFile::instance()->security(stockSecurityId).isCurrency()) {
      s = split;
      break;
    }
  }
  // Now check the target investment account to see if it
  // contains a stock with this id
  QString newStockAccountId;
  foreach (const auto sAccount, toInvAcc.accountList()) {
    if (MyMoneyFile::instance()->account(sAccount).currencyId() ==
        stockSecurityId) {
      newStockAccountId = sAccount;
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

void KMyMoneyApp::showContextMenu(const QString& containerName)
{
  QWidget* w = factory()->container(containerName, this);
  if (auto menu = dynamic_cast<QMenu*>(w))
    menu->exec(QCursor::pos());
  else
    qDebug("menu '%s' not found: w = %p, menu = %p", qPrintable(containerName), w, menu);
}

void KMyMoneyApp::slotPrintView()
{
  d->m_myMoneyView->slotPrintView();
}

void KMyMoneyApp::Private::updateCaption()
{
  auto caption = m_storageInfo.url.isEmpty() && m_myMoneyView && m_storageInfo.isOpened  ?
        i18n("Untitled") :
        m_storageInfo.url.fileName();

#ifdef KMM_DEBUG
  caption += QString(" (%1 x %2)").arg(q->width()).arg(q->height());
#endif

  q->setCaption(caption, MyMoneyFile::instance()->dirty());
}

void KMyMoneyApp::Private::updateActions()
{
  const QVector<Action> actions
  {
    Action::FilePersonalData, Action::FileInformation, Action::FileImportTemplate, Action::FileExportTemplate,
#ifdef KMM_DEBUG
    Action::FileDump,
#endif
    Action::EditFindTransaction, Action::NewCategory, Action::ToolCurrencies, Action::ToolPrices, Action::ToolUpdatePrices,
    Action::ToolConsistency, Action::ToolPerformance, Action::NewAccount, Action::NewInstitution, Action::NewSchedule
  };

  for (const auto &action : actions)
    pActions[action]->setEnabled(m_storageInfo.isOpened);
  pActions[Action::FileBackup]->setEnabled(m_storageInfo.isOpened && m_storageInfo.type == eKMyMoney::StorageType::XML);

  auto aC = q->actionCollection();
  aC->action(QString::fromLatin1(KStandardAction::name(KStandardAction::SaveAs)))->setEnabled(canFileSaveAs());
  aC->action(QString::fromLatin1(KStandardAction::name(KStandardAction::Close)))->setEnabled(m_storageInfo.isOpened);
  pActions[eMenu::Action::UpdateAllAccounts]->setEnabled(KMyMoneyUtils::canUpdateAllAccounts());
}

bool KMyMoneyApp::Private::canFileSaveAs() const
{
  return (m_storageInfo.isOpened &&
          (!pPlugins.storage.isEmpty() &&
           !(pPlugins.storage.count() == 1 && pPlugins.storage.first()->storageType() == eKMyMoney::StorageType::GNC)));
}

void KMyMoneyApp::slotDataChanged()
{
  d->fileAction(eKMyMoney::FileAction::Changed);
}

void KMyMoneyApp::slotCurrencyDialog()
{
  QPointer<KCurrencyEditDlg> dlg = new KCurrencyEditDlg(this);
  dlg->exec();
  delete dlg;
}

void KMyMoneyApp::slotPriceDialog()
{
  QPointer<KMyMoneyPriceDlg> dlg = new KMyMoneyPriceDlg(this);
  dlg->exec();
  delete dlg;
}

void KMyMoneyApp::slotFileConsistencyCheck()
{
  d->consistencyCheck(true);
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
  // in all erroneous cases, we get more than one line and force the
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

void KMyMoneyApp::Private::setThemedCSS()
{
  const QStringList CSSnames {QStringLiteral("kmymoney.css"), QStringLiteral("welcome.css")};
  const QString rcDir("/html/");
  QStringList defaultCSSDirs;
#ifndef IS_APPIMAGE
  defaultCSSDirs = QStandardPaths::locateAll(QStandardPaths::AppDataLocation, rcDir, QStandardPaths::LocateDirectory);
#else
  // according to https://docs.appimage.org/packaging-guide/ingredients.html#open-source-applications
  // QStandardPaths::AppDataLocation is unreliable on AppImages, so apply workaround here in case we fail to find icons
  // watch out for QStringBuilder here; for yet unknown reason it causes segmentation fault on startup
  const auto appImageAppDataLocation = QString("%1%2%3").arg(QCoreApplication::applicationDirPath(), QString("/../share/kmymoney"), rcDir);
  if (QFile::exists(appImageAppDataLocation + CSSnames.first())) {
    defaultCSSDirs.append(appImageAppDataLocation);
  } else {
    qWarning("CSS file was not found in the following location:");
    qWarning() << appImageAppDataLocation;
  }
#endif

  // scan the list of directories to find the ones that really
  // contains all files we look for
  QString defaultCSSDir;
  foreach (const auto dir, defaultCSSDirs) {
    defaultCSSDir = dir;
    foreach (const auto CSSname, CSSnames) {
      QFileInfo fileInfo(defaultCSSDir + CSSname);
      if (!fileInfo.exists()) {
        defaultCSSDir.clear();
        break;
      }
    }
    if (!defaultCSSDir.isEmpty()) {
      break;
    }
  }

  // make sure we have the local directory where the themed version is stored
  const QString themedCSSDir  = QStandardPaths::standardLocations(QStandardPaths::AppConfigLocation).first() + rcDir;
  QDir().mkpath(themedCSSDir);

  foreach (const auto CSSname, CSSnames) {
    const QString defaultCSSFilename = defaultCSSDir + CSSname;
    QFileInfo fileInfo(defaultCSSFilename);
    if (fileInfo.exists()) {
      const QString themedCSSFilename = themedCSSDir + CSSname;
      QFile::remove(themedCSSFilename);
      if (QFile::copy(defaultCSSFilename, themedCSSFilename)) {
        QFile cssFile (themedCSSFilename);
        if (cssFile.open(QIODevice::ReadWrite)) {
          QTextStream cssStream(&cssFile);
          auto cssText = cssStream.readAll();
          cssText.replace(QLatin1String("./"), defaultCSSDir, Qt::CaseSensitive);
          cssText.replace(QLatin1String("WindowText"),    KMyMoneySettings::schemeColor(SchemeColor::WindowText).name(),        Qt::CaseSensitive);
          cssText.replace(QLatin1String("Window"),        KMyMoneySettings::schemeColor(SchemeColor::WindowBackground).name(),  Qt::CaseSensitive);
          cssText.replace(QLatin1String("HighlightText"), KMyMoneySettings::schemeColor(SchemeColor::ListHighlightText).name(), Qt::CaseSensitive);
          cssText.replace(QLatin1String("Highlight"),     KMyMoneySettings::schemeColor(SchemeColor::ListHighlight).name(),     Qt::CaseSensitive);
          cssText.replace(QLatin1String("black"),         KMyMoneySettings::schemeColor(SchemeColor::ListGrid).name(),          Qt::CaseSensitive);
          cssStream.seek(0);
          cssStream << cssText;
          cssFile.close();
        }
      }
    }
  }
}

void KMyMoneyApp::slotCheckSchedules()
{
  if (KMyMoneySettings::checkSchedule() == true) {

    KMSTATUS(i18n("Checking for overdue scheduled transactions..."));
    MyMoneyFile *file = MyMoneyFile::instance();
    QDate checkDate = QDate::currentDate().addDays(KMyMoneySettings::checkSchedulePreview());

    QList<MyMoneySchedule> scheduleList =  file->scheduleList();
    QList<MyMoneySchedule>::Iterator it;

    eDialogs::ScheduleResultCode rc = eDialogs::ScheduleResultCode::Enter;
    for (it = scheduleList.begin(); (it != scheduleList.end()) && (rc != eDialogs::ScheduleResultCode::Cancel); ++it) {
      // Get the copy in the file because it might be modified by commitTransaction
      MyMoneySchedule schedule = file->schedule((*it).id());

      if (schedule.autoEnter()) {
        try {
          while (!schedule.isFinished() && (schedule.adjustedNextDueDate() <= checkDate)
                 && rc != eDialogs::ScheduleResultCode::Ignore
                 && rc != eDialogs::ScheduleResultCode::Cancel) {
            rc = d->m_myMoneyView->enterSchedule(schedule, true, true);
            schedule = file->schedule((*it).id()); // get a copy of the modified schedule
          }
        } catch (const MyMoneyException &) {
        }
      }
      if (rc == eDialogs::ScheduleResultCode::Ignore) {
        // if the current schedule was ignored then we must make sure that the user can still enter the next scheduled transaction
        rc = eDialogs::ScheduleResultCode::Enter;
      }
    }
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
  return d->m_storageInfo.url.url();
}

QUrl KMyMoneyApp::filenameURL() const
{
  return d->m_storageInfo.url;
}

void KMyMoneyApp::writeFilenameURL(const QUrl &url)
{
  d->m_storageInfo.url = url;
}

void KMyMoneyApp::addToRecentFiles(const QUrl& url)
{
  d->m_recentFiles->addUrl(url);
}

QTimer* KMyMoneyApp::autosaveTimer()
{
  return d->m_autoSaveTimer;
}

WebConnect* KMyMoneyApp::webConnect() const
{
  return d->m_webConnect;
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
      // please change this method of creating a list of 'all the other kmymoney instances that are running on the system'
      // since assuming that D-Bus creates service names with org.kde.kmymoney-PID is an observation I don't think that it's documented somewhere
      if ((*it).indexOf("org.kde.kmymoney-") == 0) {
        uint thisProcPid = platformTools::processId();
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

void KMyMoneyApp::webConnectUrl(const QUrl url)
{
  QMetaObject::invokeMethod(this, "webConnect", Qt::QueuedConnection, Q_ARG(QString, url.toLocalFile()), Q_ARG(QByteArray, QByteArray()));
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
    MyMoneyStatementReader::clearResultMessages();
    auto statementCount = 0;
    while (!d->m_importUrlsQueue.isEmpty()) {
      ++statementCount;
      // get the value of the next item from the queue
      // but leave it on the queue for now
      QString url = d->m_importUrlsQueue.head();

      // Bring this window to the forefront.  This method was suggested by
      // Lubos Lunak <l.lunak@suse.cz> of the KDE core development team.
      //KStartupInfo::setNewStartupId(this, asn_id);

      // Make sure we have an open file
      if (! d->m_storageInfo.isOpened &&
          KMessageBox::warningContinueCancel(this, i18n("You must first select a KMyMoney file before you can import a statement.")) == KMessageBox::Continue)
        slotFileOpen();

      // only continue if the user really did open a file.
      if (d->m_storageInfo.isOpened) {
        KMSTATUS(i18n("Importing a statement via Web Connect"));

        // remove the statement files
        d->unlinkStatementXML();

        QMap<QString, KMyMoneyPlugin::ImporterPlugin*>::const_iterator it_plugin = pPlugins.importer.constBegin();
        while (it_plugin != pPlugins.importer.constEnd()) {
          if ((*it_plugin)->isMyFormat(url)) {
            if (!(*it_plugin)->import(url) && !(*it_plugin)->lastError().isEmpty()) {
              KMessageBox::error(this, i18n("Unable to import %1 using %2 plugin.  The plugin returned the following error: %3", url, (*it_plugin)->formatName(), (*it_plugin)->lastError()), i18n("Importing error"));
            }

            break;
          }
          ++it_plugin;
        }

        // If we did not find a match, try importing it as a KMM statement file,
        // which is really just for testing.  the statement file is not exposed
        // to users.
        if (it_plugin == pPlugins.importer.constEnd())
          if (MyMoneyStatement::isStatementFile(url))
            MyMoneyStatementReader::importStatement(url, false, progressCallback);

      }
      // remove the current processed item from the queue
      d->m_importUrlsQueue.dequeue();
    }

    KMyMoneyUtils::showStatementImportResult(MyMoneyStatementReader::resultMessages(), statementCount);
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
  KMyMoneyPlugin::pluginInterfaces().appInterface = new KMyMoneyPlugin::KMMAppInterface(this, this);
  KMyMoneyPlugin::pluginInterfaces().importInterface = new KMyMoneyPlugin::KMMImportInterface(this);
  KMyMoneyPlugin::pluginInterfaces().statementInterface = new KMyMoneyPlugin::KMMStatementInterface(this);
  KMyMoneyPlugin::pluginInterfaces().viewInterface = new KMyMoneyPlugin::KMMViewInterface(d->m_myMoneyView, this);

  // setup the calendar interface for schedules
  MyMoneySchedule::setProcessingCalendar(this);
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
    if (d->dirty() && d->m_autoSaveEnabled) {
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
  QTimer::singleShot((static_cast<int>(dt.secsTo(nextDay)) + 1)*1000, this, SLOT(slotDateChanged()));
  d->m_myMoneyView->slotRefreshViews();
}

void KMyMoneyApp::setHolidayRegion(const QString& holidayRegion)
{
#ifdef ENABLE_HOLIDAYS
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
  if (!d->m_processingDays.testBit(date.dayOfWeek()))
    return false;
#ifdef ENABLE_HOLIDAYS
  if (!d->m_holidayRegion || !d->m_holidayRegion->isValid())
    return true;

  //check first whether it's already in cache
  if (d->m_holidayMap.contains(date)) {
    return d->m_holidayMap.value(date);
  } else {
    bool processingDay = !d->m_holidayRegion->isHoliday(date);
    d->m_holidayMap.insert(date, processingDay);
    return processingDay;
  }
#else
  return true;
#endif
}

void KMyMoneyApp::preloadHolidays()
{
#ifdef ENABLE_HOLIDAYS
  //clear the cache before loading
  d->m_holidayMap.clear();
  // only do this if it is a valid region
  if (d->m_holidayRegion && d->m_holidayRegion->isValid()) {
    // load holidays for the forecast days plus 1 cycle, to be on the safe side
    auto forecastDays = KMyMoneySettings::forecastDays() + KMyMoneySettings::forecastAccountCycle();
    QDate endDate = QDate::currentDate().addDays(forecastDays);

    // look for holidays for the next 2 years as a minimum. That should give a good margin for the cache
    if (endDate < QDate::currentDate().addYears(2))
      endDate = QDate::currentDate().addYears(2);

    KHolidays::Holiday::List holidayList = d->m_holidayRegion->holidays(QDate::currentDate(), endDate);
    KHolidays::Holiday::List::const_iterator holiday_it;
    for (holiday_it = holidayList.constBegin(); holiday_it != holidayList.constEnd(); ++holiday_it) {
      for (QDate holidayDate = (*holiday_it).observedStartDate(); holidayDate <= (*holiday_it).observedEndDate(); holidayDate = holidayDate.addDays(1))
        d->m_holidayMap.insert(holidayDate, (*holiday_it).dayType() == KHolidays::Holiday::Workday);
    }

    // prefill cache with all values of the forecast period
    for (QDate date = QDate::currentDate(); date <= endDate; date = date.addDays(1)) {
      // if it is not a processing day, set it to false
      if (!d->m_processingDays.testBit(date.dayOfWeek())) {
        d->m_holidayMap.insert(date, false);
      } else if (!d->m_holidayMap.contains(date)) {
        // if it is not a holiday nor a weekend, it is a processing day
        d->m_holidayMap.insert(date, true);
      }
    }
  }
#endif
}

bool KMyMoneyApp::slotFileNew()
{
  KMSTATUS(i18n("Creating new document..."));

  if (!slotFileClose())
    return false;

  NewUserWizard::Wizard wizard;
  if (wizard.exec() != QDialog::Accepted)
    return false;

  d->m_storageInfo.isOpened = true;
  d->m_storageInfo.type = eKMyMoney::StorageType::None;
  d->m_storageInfo.url = QUrl();

  try {
    auto storage = new MyMoneyStorageMgr;
    MyMoneyFile::instance()->attachStorage(storage);

    MyMoneyFileTransaction ft;
    auto file = MyMoneyFile::instance();
    // store the user info
    file->setUser(wizard.user());

    // create and setup base currency
    file->addCurrency(wizard.baseCurrency());
    file->setBaseCurrency(wizard.baseCurrency());

    // create a possible institution
    MyMoneyInstitution inst = wizard.institution();
    if (inst.name().length()) {
      file->addInstitution(inst);
    }

    // create a possible checking account
    auto acc = wizard.account();
    if (acc.name().length()) {
      acc.setInstitutionId(inst.id());
      MyMoneyAccount asset = file->asset();
      file->addAccount(acc, asset);

      // create possible opening balance transaction
      if (!wizard.openingBalance().isZero()) {
        file->createOpeningBalanceTransaction(acc, wizard.openingBalance());
      }
    }

    // import the account templates
    for (auto &tmpl : wizard.templates())
      tmpl.importTemplate(progressCallback);

    ft.commit();
    KMyMoneySettings::setFirstTimeRun(false);

    d->fileAction(eKMyMoney::FileAction::Opened);
    if (actionCollection()->action(QString::fromLatin1(KStandardAction::name(KStandardAction::SaveAs)))->isEnabled())
      slotFileSaveAs();
  } catch (const MyMoneyException & e) {
    slotFileClose();
    d->removeStorage();
    KMessageBox::detailedError(this, i18n("Couldn't create a new file."), e.what());
    return false;
  }

  if (wizard.startSettingsAfterFinished())
    slotSettings();
  return true;
}

void KMyMoneyApp::slotFileOpen()
{
  KMSTATUS(i18n("Open a file."));

  const QVector<eKMyMoney::StorageType> desiredFileExtensions {eKMyMoney::StorageType::XML, eKMyMoney::StorageType::GNC};
  QString fileExtensions;
  for (const auto &extension : desiredFileExtensions) {
    for (const auto &plugin : pPlugins.storage) {
      if (plugin->storageType() == extension) {
        fileExtensions += plugin->fileExtension() + QLatin1String(";;");
        break;
      }
    }
  }

  if (fileExtensions.isEmpty()) {
    KMessageBox::error(this, i18n("Couldn't find any plugin for opening storage."));
    return;
  }

  fileExtensions.append(i18n("All files (*)"));

  QPointer<QFileDialog> dialog = new QFileDialog(this, QString(), readLastUsedDir(), fileExtensions);
  dialog->setFileMode(QFileDialog::ExistingFile);
  dialog->setAcceptMode(QFileDialog::AcceptOpen);

  if (dialog->exec() == QDialog::Accepted && dialog != nullptr)
    slotFileOpenRecent(dialog->selectedUrls().first());
  delete dialog;
}

bool KMyMoneyApp::slotFileOpenRecent(const QUrl &url)
{
  KMSTATUS(i18n("Loading file..."));

  if (!url.isValid())
    throw MYMONEYEXCEPTION(QString::fromLatin1("Invalid URL %1").arg(qPrintable(url.url())));

  if (isFileOpenedInAnotherInstance(url)) {
    KMessageBox::sorry(this, i18n("<p>File <b>%1</b> is already opened in another instance of KMyMoney</p>", url.toDisplayString(QUrl::PreferLocalFile)), i18n("Duplicate open"));
    return false;
  }

  if (url.scheme() != QLatin1String("sql") && !KMyMoneyUtils::fileExists(url)) {
    KMessageBox::sorry(this, i18n("<p><b>%1</b> is either an invalid filename or the file does not exist. You can open another file or create a new one.</p>", url.toDisplayString(QUrl::PreferLocalFile)), i18n("File not found"));
    return false;
  }

  if (d->m_storageInfo.isOpened)
    if (!slotFileClose())
      return false;

  // open the database
  d->m_storageInfo.type = eKMyMoney::StorageType::None;
  for (auto &plugin : pPlugins.storage) {
    try {
      if (auto pStorage = plugin->open(url)) {
        MyMoneyFile::instance()->attachStorage(pStorage);
        d->m_storageInfo.type = plugin->storageType();
        if (plugin->storageType() != eKMyMoney::StorageType::GNC) {
          d->m_storageInfo.url = plugin->openUrl();
          writeLastUsedFile(url.toDisplayString(QUrl::PreferLocalFile));
          /* Don't use url variable after KRecentFilesAction::addUrl
         * as it might delete it.
         * More in API reference to this method
         */
          d->m_recentFiles->addUrl(url);
        }
        d->m_storageInfo.isOpened = true;
        break;
      }
    } catch (const MyMoneyException &e) {
      KMessageBox::detailedError(this, i18n("Cannot open file as requested."), QString::fromLatin1(e.what()));
      return false;
    }
  }

  if(d->m_storageInfo.type == eKMyMoney::StorageType::None) {
    KMessageBox::error(this, i18n("Could not read your data source. Please check the KMyMoney settings that the necessary plugin is enabled."));
    return false;
  }

  d->fileAction(eKMyMoney::FileAction::Opened);
  return true;
}

bool KMyMoneyApp::slotFileSave()
{
  KMSTATUS(i18n("Saving file..."));

  for (const auto& plugin : pPlugins.storage) {
    if (plugin->storageType() == d->m_storageInfo.type) {
      d->consistencyCheck(false);
      try {
        if (plugin->save(d->m_storageInfo.url)) {
          d->fileAction(eKMyMoney::FileAction::Saved);
          return true;
        }
        return false;
      } catch (const MyMoneyException &e) {
        KMessageBox::detailedError(this, i18n("Failed to save your storage."), e.what());
        return false;
      }
    }
  }

  KMessageBox::error(this, i18n("Couldn't find suitable plugin to save your storage."));
  return false;
}

bool KMyMoneyApp::slotFileSaveAs()
{
  KMSTATUS(i18n("Saving file as...."));

  QVector<eKMyMoney::StorageType> availableFileTypes;
  for (const auto& plugin : pPlugins.storage) {
    switch (plugin->storageType()) {
      case eKMyMoney::StorageType::GNC:
        break;
      default:
        availableFileTypes.append(plugin->storageType());
        break;
    }
  }

  auto chosenFileType = eKMyMoney::StorageType::None;
  switch (availableFileTypes.count()) {
    case 0:
      KMessageBox::error(this, i18n("Couldn't find any plugin for saving storage."));
      return false;
    case 1:
      chosenFileType = availableFileTypes.first();
      break;
    default:
      {
        QPointer<KSaveAsQuestion> dlg = new KSaveAsQuestion(availableFileTypes, this);
        auto rc = dlg->exec();
        if (dlg) {
          auto fileType = dlg->fileType();
          delete dlg;
          if (rc != QDialog::Accepted)
            return false;
          chosenFileType = fileType;
        }
      }
  }

  for (const auto &plugin : pPlugins.storage) {
    if (chosenFileType == plugin->storageType()) {
      try {
        d->consistencyCheck(false);
        if (plugin->saveAs()) {
          d->fileAction(eKMyMoney::FileAction::Saved);
          d->m_storageInfo.type = plugin->storageType();
          return true;
        }
      } catch (const MyMoneyException &e) {
        KMessageBox::detailedError(this, i18n("Failed to save your storage."), e.what());
      }
    }
  }
  return false;
}

bool KMyMoneyApp::slotFileClose()
{
  if (!d->m_storageInfo.isOpened)
    return true;

  if (!d->askAboutSaving())
    return false;

  d->fileAction(eKMyMoney::FileAction::Closing);

  d->removeStorage();

  d->m_storageInfo = KMyMoneyApp::Private::storageInfo();

  d->fileAction(eKMyMoney::FileAction::Closed);
  return true;
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

void KMyMoneyApp::Private::fileAction(eKMyMoney::FileAction action)
{
  switch(action) {
    case eKMyMoney::FileAction::Opened:
      q->actionCollection()->action(QString::fromLatin1(KStandardAction::name(KStandardAction::Save)))->setEnabled(false);
      updateAccountNames();
      updateCurrencyNames();
      selectBaseCurrency();

      // setup the standard precision
      AmountEdit::setStandardPrecision(MyMoneyMoney::denomToPrec(MyMoneyFile::instance()->baseCurrency().smallestAccountFraction()));

      applyFileFixes();
      Models::instance()->fileOpened();
      connectStorageToModels();
      // inform everyone about new data
      MyMoneyFile::instance()->forceDataChanged();
      // Enable save in case the fix changed the contents
      q->actionCollection()->action(QString::fromLatin1(KStandardAction::name(KStandardAction::Save)))->setEnabled(dirty());
      updateActions();
      m_myMoneyView->slotFileOpened();
      onlineJobAdministration::instance()->updateActions();
      m_myMoneyView->enableViewsIfFileOpen(m_storageInfo.isOpened);
      m_myMoneyView->slotRefreshViews();
      onlineJobAdministration::instance()->updateOnlineTaskProperties();
      q->connect(MyMoneyFile::instance(), &MyMoneyFile::dataChanged, q, &KMyMoneyApp::slotDataChanged);

#ifdef ENABLE_ACTIVITIES
      {
        // make sure that we don't store the DB password in activity
        QUrl url(m_storageInfo.url);
        url.setPassword(QString());
        m_activityResourceInstance->setUri(url);
      }
#endif
      // start the check for scheduled transactions that need to be
      // entered as soon as the event loop becomes active.
      QMetaObject::invokeMethod(q, "slotCheckSchedules",  Qt::QueuedConnection);

      // make sure to catch view activations
      connect(m_myMoneyView, &KMyMoneyView::viewActivated, q, &KMyMoneyApp::slotViewSelected);
      break;

    case eKMyMoney::FileAction::Saved:
      q->connect(MyMoneyFile::instance(), &MyMoneyFile::dataChanged, q, &KMyMoneyApp::slotDataChanged);
      q->actionCollection()->action(QString::fromLatin1(KStandardAction::name(KStandardAction::Save)))->setEnabled(false);
      m_autoSaveTimer->stop();
      break;

    case eKMyMoney::FileAction::Closing:
      disconnect(MyMoneyFile::instance(), &MyMoneyFile::dataChanged, q, &KMyMoneyApp::slotDataChanged);
      // make sure to not catch view activations anymore
      disconnect(m_myMoneyView, &KMyMoneyView::viewActivated, q, &KMyMoneyApp::slotViewSelected);
      m_myMoneyView->slotFileClosed();
      // notify the models that the file is going to be closed (we should have something like dataChanged that reaches the models first)
      Models::instance()->fileClosed();
      break;

    case eKMyMoney::FileAction::Closed:
      q->disconnect(MyMoneyFile::instance(), &MyMoneyFile::dataChanged, q, &KMyMoneyApp::slotDataChanged);
      disconnectStorageFromModels();
      q->actionCollection()->action(QString::fromLatin1(KStandardAction::name(KStandardAction::Save)))->setEnabled(false);
      m_myMoneyView->enableViewsIfFileOpen(m_storageInfo.isOpened);
      updateActions();
      break;

    case eKMyMoney::FileAction::Changed:
      q->disconnect(MyMoneyFile::instance(), &MyMoneyFile::dataChanged, q, &KMyMoneyApp::slotDataChanged);
      q->actionCollection()->action(QString::fromLatin1(KStandardAction::name(KStandardAction::Save)))->setEnabled(true && !m_storageInfo.url.isEmpty());
      // As this method is called every time the MyMoneyFile instance
      // notifies a modification, it's the perfect place to start the timer if needed
      if (m_autoSaveEnabled && !m_autoSaveTimer->isActive()) {
        m_autoSaveTimer->setSingleShot(true);
        m_autoSaveTimer->start(m_autoSavePeriod * 60 * 1000);  //miliseconds
      }
      pActions[eMenu::Action::UpdateAllAccounts]->setEnabled(KMyMoneyUtils::canUpdateAllAccounts());
      break;

    default:
      break;
  }

  updateCaption();
}

KMStatus::KMStatus(const QString &text)
  : m_prevText(kmymoney->slotStatusMsg(text))
{
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
}
