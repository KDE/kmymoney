/*
    SPDX-FileCopyrightText: 2000 Michael Edwardes <mte@users.sourceforge.net>
    SPDX-FileCopyrightText: 2002-2020 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2021 Dawid Wróbel <me@dawidwrobel.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#include <config-kmymoney.h>

#include "kmymoney.h"
#include "reportsmodel.h"

// ----------------------------------------------------------------------------
// Std C++ / STL Includes

#include <iostream>

// ----------------------------------------------------------------------------
// QT Includes

#include <QApplication>
#include <QBitArray>
#include <QBoxLayout>
#include <QByteArray>
#include <QClipboard>
#include <QDateTime> // only for performance tests
#include <QDesktopServices>
#include <QDir>
#include <QElapsedTimer>
#include <QFileDialog>
#include <QFlags>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QInputDialog>
#include <QKeyEvent>
#include <QKeySequence>
#include <QLabel>
#include <QList>
#include <QListWidget>
#include <QMenu>
#include <QMenuBar>
#include <QProgressBar>
#include <QPushButton>
#include <QQueue>
#include <QStatusBar>
#include <QTextBrowser>
#include <QTimer>
#include <QUrl>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KAboutApplicationDialog>
#include <KActionCollection>
#include <KBackup>
#include <KConfig>
#include <KConfigDialog>
#include <KConfigDialogManager>
#include <KDualAction>
#include <KLocalizedString>
#include <KMessageBox>
#include <KProcess>
#include <KRecentDirs>
#include <KRecentFilesAction>
#include <KStandardAction>
#include <KToolBar>
#include <KUndoActions>
#include <KXMLGUIFactory>
#include <kio_version.h>

#ifdef ENABLE_HOLIDAYS
#include <KHolidays/Holiday>
#include <KHolidays/HolidayRegion>
#include <kholidays_version.h>
#endif

#ifdef ENABLE_ACTIVITIES
#include <KActivities/ResourceInstance>
#endif

#include <KDialogJobUiDelegate>
#include <KIO/CommandLauncherJob>

// ----------------------------------------------------------------------------
// Project Includes

#include <alkimia/alkenvironment.h>

#include "accountsmodel.h"
#include "budgetsmodel.h"
#include "dialogs/editpersonaldatadlg.h"
#include "dialogs/kbackupdlg.h"
#include "dialogs/kbalancewarning.h"
#include "dialogs/kcategoryreassigndlg.h"
#include "dialogs/kconfirmmanualenterdlg.h"
#include "dialogs/kcurrencycalculator.h"
#include "dialogs/kcurrencyeditdlg.h"
#include "dialogs/kequitypriceupdatedlg.h"
#include "dialogs/kmymoneyfileinfodlg.h"
#include "dialogs/kmymoneypricedlg.h"
#include "dialogs/knewaccountdlg.h"
#include "dialogs/knewinstitutiondlg.h"
#include "dialogs/kpayeereassigndlg.h"
#include "dialogs/ksaveasquestion.h"
#include "dialogs/settings/ksettingskmymoney.h"
#include "dialogs/transactionmatcher.h"
#include "equitiesmodel.h"
#include "importsummarydlg.h"
#include "journalmodel.h"
#include "keditscheduledlg.h"
#include "kmm_menuactionexchanger.h"
#ifdef KMM_DBUS
#include "kmymoneyadaptor.h"
#endif
#include "kmymoneysettings.h"
#include "kmymoneyutils.h"
#include "kmymoneyview.h"
#include "ksearchtransactiondlg.h"
#include "ktransactionselectdlg.h"
#include "ledgerviewsettings.h"
#include "schedulesjournalmodel.h"
#include "specialdatesmodel.h"
#include "widgets/amountedit.h"
#include "widgets/kmymoneyaccountselector.h"
#include "widgets/kmymoneydateedit.h"
#include "widgets/kmymoneypayeecombo.h"
#include "wizards/endingbalancedlg/kendingbalancedlg.h"
#include "wizards/newaccountwizard/knewaccountwizard.h"
#include "wizards/newinvestmentwizard/knewinvestmentwizard.h"
#include "wizards/newuserwizard/knewuserwizard.h"

#include "mymoney/mymoneyaccount.h"
#include "mymoney/mymoneyaccountloan.h"
#include "mymoney/mymoneybudget.h"
#include "mymoney/mymoneyfile.h"
#include "mymoney/mymoneyforecast.h"
#include "mymoney/mymoneyinstitution.h"
#include "mymoney/mymoneyobject.h"
#include "mymoney/mymoneypayee.h"
#include "mymoney/mymoneyprice.h"
#include "mymoney/mymoneyreport.h"
#include "mymoney/mymoneysecurity.h"
#include "mymoney/mymoneysplit.h"
#include "mymoney/mymoneystatement.h"
#include "mymoney/mymoneytag.h"
#include "mymoney/mymoneytransactionfilter.h"
#include "mymoney/mymoneyutils.h"
#include "mymoneyexception.h"
#include "mymoneyreconciliationreport.h"

#include "mymoneytemplate.h"
#include "templateloader.h"
#include "templatewriter.h"
#include "kloadtemplatedlg.h"
#include "ktemplateexportdlg.h"

#include "converter/mymoneystatementreader.h"

#include "interfaces/interfaceloader.h"
#include "kmymoneyplugin.h"
#include "pluginloader.h"
#include "plugins/plugin-interfaces/kmmappinterface.h"
#include "plugins/plugin-interfaces/kmmimportinterface.h"
#include "plugins/plugin-interfaces/kmmstatementinterface.h"
#include "plugins/plugin-interfaces/kmmviewinterface.h"

#include "tasks/credittransfer.h"

#include "icons/icons.h"

#include "misc/webconnect.h"

#include "imymoneystorageformat.h"

#include "kmymoneyutils.h"
#include "kcreditswindow.h"

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

#include "selectedobjects.h"

#include "kmmyesno.h"

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#define QT6_SKIP_FROM_LATIN_STRING(a) a
#else
#define QT6_SKIP_FROM_LATIN_STRING(a) QString::fromLatin1(a)
#endif

using namespace Icons;
using namespace eMenu;

enum backupStateE {
    BACKUP_IDLE = 0,
    BACKUP_MOUNTING,
    BACKUP_COPYING,
    BACKUP_UNMOUNTING,
};

class ShortCutActionFilter : public QObject
{
    Q_OBJECT
public:
    ShortCutActionFilter(QAction* parent)
        : QObject(parent)
    {
    }

    bool eventFilter(QObject* watched, QEvent* event) override
    {
        Q_UNUSED(watched)

        if (event->type() == QEvent::ShortcutOverride) {
            const auto kev = static_cast<QKeyEvent*>(event);
            const auto keySeq = QKeySequence(kev->modifiers() | kev->key());
            const auto action = static_cast<QAction*>(parent());
            if (keySeq == action->shortcut()) {
                Q_EMIT shortCutDetected();
                event->accept();
                return true;
            }
        }
        return false;
    }

Q_SIGNALS:
    void shortCutDetected();
};

class KMyMoneyApp::Private
{
public:
    Q_DISABLE_COPY_MOVE(Private)

    explicit Private(KMyMoneyApp* app)
        : q(app)
        , m_backupState(backupStateE::BACKUP_IDLE)
        , m_backupResult(0)
        , m_backupMount(0)
        , m_ignoreBackupExitCode(false)
        , m_myMoneyView(nullptr)
        , m_startDialog(false)
        , m_progressBar(nullptr)
        , m_statusLabel(nullptr)
        , m_autoSaveEnabled(true)
        , m_autoSaveTimer(nullptr)
        , m_progressTimer(nullptr)
        , m_autoSavePeriod(0)
        , m_inAutoSaving(false)
        , m_recentFiles(nullptr)
#ifdef ENABLE_HOLIDAYS
        , m_holidayRegion(nullptr)
#endif
#ifdef ENABLE_ACTIVITIES
        , m_activityResourceInstance(nullptr)
#endif
        , m_applicationIsReady(true)
        , m_webConnect(new WebConnect(app))
        , m_searchDlg(nullptr)
        , m_actionExchanger(new KMenuActionExchanger(q))
    {
        // since the days of the week are from 1 to 7,
        // and a day of the week is used to index this bit array,
        // resize the array to 8 elements (element 0 is left unused)
        m_processingDays.resize(8);

        m_actionCollectorTimer.setSingleShot(true);
        m_actionCollectorTimer.setInterval(100);
    }

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

    // allows multiple imports to be launched through web connect and to be executed sequentially
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

    SelectedObjects       m_selections;
    QTimer                m_actionCollectorTimer;

    typedef struct SharedActionButtonInfo {
        QToolButton* button = nullptr;
        QAction* defaultAction = nullptr;
    } SharedActionButtonInfo;

    QHash<eMenu::Action, SharedActionButtonInfo> m_sharedActionButtons;

    KSearchTransactionDlg* m_searchDlg;

    KMenuActionExchanger* m_actionExchanger;

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
        const QList<MyMoneySecurity> availableCurrencies = MyMoneyFile::instance()->availableCurrencyList();
        QStringList currencyIDs;

        for (const auto& currency : availableCurrencies)
            currencyIDs.append(currency.id());

        try {
            for (auto currency : qAsConst(storedCurrencies)) {
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
        const auto file = MyMoneyFile::instance();
        QSignalBlocker blocked(file);
        KSharedConfigPtr config = KSharedConfig::openConfig();
        KConfigGroup grp = config->group("General Options");
        bool result = true;

        // For debugging purposes, we can turn off the automatic fix manually
        // by setting the entry in kmymoneyrc to true
        if (grp.readEntry("SkipFix", false) != true) {
            try {
                result = file->applyFileFixes(KMyMoneySettings::expertMode());
            } catch (const MyMoneyException &) {
                return false;
            }
        } else {
            qDebug() << "Skipping automatic transaction fix!";
        }
        return result;
    }

    bool askAboutSaving()
    {
        const auto isFileNotSaved = q->actionCollection()->action(QT6_SKIP_FROM_LATIN_STRING(KStandardAction::name(KStandardAction::Save)))->isEnabled();
        const auto isNewFileNotSaved = m_storageInfo.isOpened && m_storageInfo.url.isEmpty();
        auto fileNeedsToBeSaved = false;

        if (isFileNotSaved && KMyMoneySettings::autoSaveOnClose()) {
            fileNeedsToBeSaved = true;
        } else if (isFileNotSaved || isNewFileNotSaved) {
            switch (KMessageBox::warningTwoActionsCancel(q,
                                                         i18n("The file has been changed, save it?"),
                                                         i18nc("@title:window", "Save file"),
                                                         KMMYesNo::yes(),
                                                         KMMYesNo::no())) {
            case KMessageBox::ButtonCode::PrimaryAction:
                fileNeedsToBeSaved = true;
                break;
            case KMessageBox::ButtonCode::SecondaryAction:
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
      * This method removes all data from the MyMoneyFile object
      * and resets the dirty flag(s).
      */
    void removeStorage()
    {
        MyMoneyFile::instance()->unload();
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
                        cid = MyMoneyFile::instance()->security((*it).currencyId()).id();
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

    void updateActions(const SelectedObjects& selections)
    {
        static const QVector<Action> actions = {
            Action::FilePersonalData,
            Action::FileInformation,
            Action::FileImportTemplate,
            Action::FileExportTemplate,
            Action::EditTabOrder,
#ifdef KMM_DEBUG
            Action::FileDump,
            Action::NewFeature,
#endif
            Action::EditFindTransaction,
            Action::NewCategory,
            Action::ToolCurrencies,
            Action::ToolPrices,
            Action::ToolUpdatePrices,
            Action::ToolConsistency,
            Action::ToolPerformance,
            Action::NewAccount,
            Action::NewInstitution,
            Action::NewSchedule,
            Action::ShowFilterWidget,
            Action::NewPayee,
            Action::NewTag,
            Action::Print,
            Action::PrintPreview,
        };

        for (const auto& action : actions)
            pActions[action]->setEnabled(m_storageInfo.isOpened);

        // make sure all shared actions of the New button have the right state
        if (m_sharedActionButtons[Action::FileNew].button) {
            for (const auto action : m_sharedActionButtons[Action::FileNew].button->actions()) {
                if (action) {
                    action->setEnabled(m_storageInfo.isOpened);
                }
            }
        }
        // except the New File/Book which is always enabled
        pActions[Action::FileNew]->setEnabled(true);

        pActions[Action::FileBackup]->setEnabled(m_storageInfo.isOpened && m_storageInfo.type == eKMyMoney::StorageType::XML);

        auto aC = q->actionCollection();
        aC->action(QT6_SKIP_FROM_LATIN_STRING(KStandardAction::name(KStandardAction::SaveAs)))->setEnabled(canFileSaveAs());
        aC->action(QT6_SKIP_FROM_LATIN_STRING(KStandardAction::name(KStandardAction::Close)))->setEnabled(m_storageInfo.isOpened);
        pActions[Action::UpdateAllAccounts]->setEnabled(m_storageInfo.isOpened && KMyMoneyUtils::canUpdateAllAccounts());

        updateReconciliationTools(selections);

        // update actions in views and plugins
        m_myMoneyView->updateActions(selections);
        KMyMoneyPlugin::updateActions(pPlugins, selections);
    }

    void updateReconciliationTools(const SelectedObjects& selections)
    {
        // update reconciliation toolbar
        const bool inReconciliation = !selections.firstSelection(SelectedObjects::ReconciliationAccount).isEmpty();
        q->toolBar("reconcileToolBar")->setVisible(inReconciliation);
    }

    bool canFileSaveAs() const
    {
        return (m_storageInfo.isOpened
                && (!pPlugins.storage.isEmpty() && !(pPlugins.storage.count() == 1 && pPlugins.storage.first()->storageType() == eKMyMoney::StorageType::GNC)));
    }

    /**
      * This method is used to update the caption of the application window.
      * It sets the caption to "filename [modified] - KMyMoney".
      */
    void updateCaption()
    {
        auto caption = m_storageInfo.url.isEmpty() && m_myMoneyView && m_storageInfo.isOpened ? i18n("Untitled") : m_storageInfo.url.fileName();

#ifdef KMM_DEBUG
        caption += QString(" (%1 x %2)").arg(q->width()).arg(q->height());
#endif

        q->setCaption(caption, MyMoneyFile::instance()->dirty());
    }

    // bool canUpdateAllAccounts() const;
    void fileAction(eKMyMoney::FileAction action)
    {
        AmountEdit amountEdit;

        switch (action) {
        case eKMyMoney::FileAction::Opened:
            q->actionCollection()->action(QT6_SKIP_FROM_LATIN_STRING(KStandardAction::name(KStandardAction::Save)))->setEnabled(false);
            updateAccountNames();
            updateCurrencyNames();
            selectBaseCurrency();

            // setup the standard precision
            amountEdit.setStandardPrecision(MyMoneyMoney::denomToPrec(MyMoneyFile::instance()->baseCurrency().smallestAccountFraction()));

            applyFileFixes();
            // setup internal data for which we need all models loaded
            MyMoneyFile::instance()->accountsModel()->setupAccountFractions();

            // clean undostack
            MyMoneyFile::instance()->undoStack()->clear();

            // inform everyone about new data
            MyMoneyFile::instance()->modelsReadyToUse();
            MyMoneyFile::instance()->forceDataChanged();
            // Enable save in case the fix changed the contents
            q->actionCollection()->action(QT6_SKIP_FROM_LATIN_STRING(KStandardAction::name(KStandardAction::Save)))->setEnabled(dirty());
            updateActions(SelectedObjects());
            // inform views about new data source
            m_myMoneyView->executeAction(Action::FileNew, SelectedObjects());

            onlineJobAdministration::instance()->updateActions();
            m_myMoneyView->enableViewsIfFileOpen(m_storageInfo.isOpened);
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
            QMetaObject::invokeMethod(q, "slotCheckSchedules", Qt::QueuedConnection);
            break;

        case eKMyMoney::FileAction::Saved:
            // clear the dirty flag
            MyMoneyFile::instance()->setDirty(false);
            q->connect(MyMoneyFile::instance(), &MyMoneyFile::dataChanged, q, &KMyMoneyApp::slotDataChanged);
            q->actionCollection()->action(QT6_SKIP_FROM_LATIN_STRING(KStandardAction::name(KStandardAction::Save)))->setEnabled(false);
            m_autoSaveTimer->stop();
            break;

        case eKMyMoney::FileAction::Closing:
            disconnect(MyMoneyFile::instance(), &MyMoneyFile::dataChanged, q, &KMyMoneyApp::slotDataChanged);
            // make sure to not catch view activations anymore
            m_myMoneyView->executeAction(Action::FileClose, SelectedObjects());
            // notify the plugin that we close the file
            for (auto& plugin : pPlugins.storage) {
                try {
                    if (m_storageInfo.type == plugin->storageType()) {
                        plugin->close();
                        break;
                    }
                } catch (const MyMoneyException& e) {
                    KMessageBox::detailedError(q, i18n("Plugin responded with error during close."), QString::fromLatin1(e.what()));
                }
            }
            // notify the models that the file is going to be closed (we should have something like dataChanged that reaches the models first)
            MyMoneyFile::instance()->unload();
            break;

        case eKMyMoney::FileAction::Closed:
            q->disconnect(MyMoneyFile::instance(), &MyMoneyFile::dataChanged, q, &KMyMoneyApp::slotDataChanged);
            q->actionCollection()->action(QT6_SKIP_FROM_LATIN_STRING(KStandardAction::name(KStandardAction::Save)))->setEnabled(false);
            m_myMoneyView->enableViewsIfFileOpen(m_storageInfo.isOpened);
            updateActions(SelectedObjects());
            break;

        case eKMyMoney::FileAction::Changed:
            q->disconnect(MyMoneyFile::instance(), &MyMoneyFile::dataChanged, q, &KMyMoneyApp::slotDataChanged);
            q->actionCollection()
                ->action(QT6_SKIP_FROM_LATIN_STRING(KStandardAction::name(KStandardAction::Save)))
                ->setEnabled(true && !m_storageInfo.url.isEmpty());
            // As this method is called every time the MyMoneyFile instance
            // notifies a modification, it's the perfect place to start the timer if needed
            if (m_autoSaveEnabled && !m_autoSaveTimer->isActive()) {
                m_autoSaveTimer->setSingleShot(true);
                m_autoSaveTimer->start(m_autoSavePeriod * 60 * 1000); // milliseconds
            }
            pActions[eMenu::Action::UpdateAllAccounts]->setEnabled(KMyMoneyUtils::canUpdateAllAccounts());
            break;

        default:
            break;
        }

        updateCaption();
    }

    eMenu::Action qActionToId(const QAction* action)
    {
        if (action) {
            auto it = pActions.cbegin();
            for (; it != pActions.cend(); ++it) {
                if (*it == action) {
                    return it.key();
                }
            }
        }
        return eMenu::Action::None;
    }

    void matchTransaction()
    {
        if (m_selections.selection(SelectedObjects::JournalEntry).count() != 2)
            return;

        const auto priorSelection(m_selections);

        const auto file(MyMoneyFile::instance());
        QString toBeDeletedId;
        QString remainingId;

        for (const auto& journalEntryId : m_selections.selection(SelectedObjects::JournalEntry)) {
            const auto idx = file->journalModel()->indexById(journalEntryId);
            if (idx.data(eMyMoney::Model::TransactionIsImportedRole).toBool()) {
                if (toBeDeletedId.isEmpty()) {
                    toBeDeletedId = journalEntryId;
                } else {
                    // This is a second imported transaction, we still want to merge
                    remainingId = journalEntryId;
                }
            } else if (!idx.data(eMyMoney::Model::JournalSplitIsMatchedRole).toBool()) {
                if (remainingId.isEmpty()) {
                    remainingId = journalEntryId;
                } else {
                    toBeDeletedId = journalEntryId;
                }
            }
        }

        // the user selected two transactions but they might be
        // selected in the wrong order. We check here, if the
        // other way around makes more sense and simply exchange
        // the two. See bko #435512
        auto remaining = file->journalModel()->itemById(remainingId);
        auto toBeDeleted = file->journalModel()->itemById(toBeDeletedId);

        if ((remaining.transaction().splitCount() == 1) && (toBeDeleted.transaction().splitCount() > 1)) {
            swap(remaining, toBeDeleted);
        }

        QPointer<KTransactionMergeDlg> dlg(new KTransactionMergeDlg(q));
        dlg->addTransaction(remainingId);
        dlg->addTransaction(toBeDeletedId);
        const auto doMatch = ((dlg->exec() == QDialog::Accepted) && (dlg != nullptr));

        if (doMatch && (toBeDeleted.split().accountId() == remaining.split().accountId())) {
            remaining = file->journalModel()->itemById(dlg->remainingTransactionId());
            toBeDeleted = file->journalModel()->itemById(dlg->mergedTransactionId());

            MyMoneyFileTransaction ft;
            try {
                if (remaining.transaction().id().isEmpty())
                    throw MYMONEYEXCEPTION(QString::fromLatin1("No manually entered transaction selected for matching"));
                if (toBeDeleted.transaction().id().isEmpty())
                    throw MYMONEYEXCEPTION(QString::fromLatin1("No imported transaction selected for matching"));

                TransactionMatcher matcher;
                matcher.match(remaining.transaction(), remaining.split(), toBeDeleted.transaction(), toBeDeleted.split(), true);
                ft.commit();
            } catch (const MyMoneyException& e) {
                KMessageBox::detailedError(q, i18n("Unable to match the selected transactions"), e.what());
            }

            // inform views about the match (they may have to reselect some items)
            m_myMoneyView->executeAction(Action::MatchTransaction, priorSelection);
        }

        delete dlg;
    }

    void unmatchTransaction()
    {
        const auto file(MyMoneyFile::instance());

        MyMoneyFileTransaction ft;
        try {
            for (const auto& journalEntryId : m_selections.selection(SelectedObjects::JournalEntry)) {
                const auto idx = file->journalModel()->indexById(journalEntryId);
                if (idx.data(eMyMoney::Model::JournalSplitIsMatchedRole).toBool()) {
                    const auto journalEntry = file->journalModel()->itemByIndex(idx);
                    TransactionMatcher matcher;
                    matcher.unmatch(journalEntry.transaction(), journalEntry.split());
                }
            }
            ft.commit();

        } catch (const MyMoneyException& e) {
            KMessageBox::detailedError(q, i18n("Unable to unmatch the selected transactions"), e.what());
        }
    }

    void executeAction(eMenu::Action actionId)
    {
        if (actionId != eMenu::Action::None) {
            // make sure to work on the current state
            const auto selections = m_selections;
            m_myMoneyView->executeAction(actionId, selections);
        }
    }

    void executeCustomAction(eView::Action actionId)
    {
        if (actionId != eView::Action::None) {
            m_myMoneyView->executeCustomAction(actionId);
        }
    }

    void closeSubAccounts(const MyMoneyAccount& account)
    {
        MyMoneyFile* file = MyMoneyFile::instance();
        const auto accountList = account.accountList();

        for (const auto& subAccountId : accountList) {
            auto subAccount = file->account(subAccountId);
            closeSubAccounts(subAccount);
            subAccount.setClosed(true);
            file->modifyAccount(subAccount);
        }
    }

    LedgerViewSettings::ReconciliationHeader showReconciliationMarker() const
    {
        switch (KMyMoneySettings::showReconciliationMarker()) {
        case KMyMoneySettings::Off:
            return LedgerViewSettings::DontShowReconciliationHeader;
        case KMyMoneySettings::Last:
            return LedgerViewSettings::ShowLastReconciliationHeader;
        default:
            return LedgerViewSettings::ShowAllReconciliationHeader;
        }
    }

    /**
     * Create a new stock account as a copy of @a sourceStockAccountId in
     * the invesment account identified by @a parentInvestAccount.
     */
    QString createNewStockAccount(MyMoneyAccount& parentInvestAccount, const QString& sourceStockAccountId)
    {
        MyMoneyAccount stockAccount = MyMoneyFile::instance()->account(sourceStockAccountId);
        MyMoneyAccount newStock;
        newStock.setName(stockAccount.name());
        newStock.setNumber(stockAccount.number());
        newStock.setDescription(stockAccount.description());
        newStock.setInstitutionId(stockAccount.institutionId());
        newStock.setOpeningDate(stockAccount.openingDate());
        newStock.setAccountType(stockAccount.accountType());
        newStock.setCurrencyId(stockAccount.currencyId());
        newStock.setClosed(stockAccount.isClosed());
        MyMoneyFile::instance()->addAccount(newStock, parentInvestAccount);
        return newStock.id();
    }

    // move a stock transaction from one investment account to another
    void moveInvestmentTransaction(const QString& /*fromId*/, const QString& toId, const MyMoneyTransaction& tx)
    {
        MyMoneyAccount toInvAcc = MyMoneyFile::instance()->account(toId);
        MyMoneyTransaction t(tx);
        // first determine which stock we are dealing with.
        // fortunately, investment transactions have only one stock involved
        QString stockAccountId;
        QString stockSecurityId;
        MyMoneySplit s;
        const auto splits = t.splits();
        for (const auto& split : splits) {
            stockAccountId = split.accountId();
            stockSecurityId = MyMoneyFile::instance()->account(stockAccountId).currencyId();
            if (!MyMoneyFile::instance()->security(stockSecurityId).isCurrency()) {
                s = split;
                break;
            }
        }
        // Now check the target investment account to see if it
        // contains a stock with this id
        QString newStockAccountId;
        const auto accountList = toInvAcc.accountList();
        for (const auto& sAccount : accountList) {
            if (MyMoneyFile::instance()->account(sAccount).currencyId() == stockSecurityId) {
                newStockAccountId = sAccount;
                break;
            }
        }
        // if it doesn't exist, we need to add it as a copy of the old one
        // no 'copyAccount()' function??
        if (newStockAccountId.isEmpty()) {
            newStockAccountId = createNewStockAccount(toInvAcc, stockAccountId);
        }
        // now update the split and the transaction
        s.setAccountId(newStockAccountId);
        t.modifySplit(s);
        MyMoneyFile::instance()->modifyTransaction(t);
    }

    void unlinkStatementXML()
    {
        QDir dir(KMyMoneySettings::logPath(), "kmm-statement*");
        for (uint i = 0; i < dir.count(); ++i) {
            qDebug("Remove %s", qPrintable(dir[i]));
            dir.remove(KMyMoneySettings::logPath() + QString("/%1").arg(dir[i]));
        }
    }
};

KMyMoneyApp::KMyMoneyApp(QWidget* parent)
    : KXmlGuiWindow(parent)
    , MyMoneyFactory(this)
    , d(new Private(this))
{
#ifdef KMM_DBUS
    new KmymoneyAdaptor(this);
    QDBusConnection::sessionBus().registerObject("/KMymoney", this);
    QDBusConnection::sessionBus().interface()->registerService(
        QStringLiteral("org.kde.kmymoney-%1").arg(QString::number(platformTools::processId()), QDBusConnectionInterface::DontQueueService));
#endif
    // Register the main engine types used as meta-objects
    qRegisterMetaType<MyMoneyMoney>("MyMoneyMoney");
    qRegisterMetaType<MyMoneySecurity>("MyMoneySecurity");

    registerCreator<KCurrencyCalculator, QWidget>(&KCurrencyCalculator::createObject);
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

    MyMoneyTransactionFilter::setFiscalYearStart(KMyMoneySettings::firstFiscalMonth(), KMyMoneySettings::firstFiscalDay());

    QFrame* frame = new QFrame;
    frame->setFrameStyle(QFrame::NoFrame);
    // values for margin (11) and spacing(6) taken from KDialog implementation
    QBoxLayout* layout = new QBoxLayout(QBoxLayout::TopToBottom, frame);
    layout->setContentsMargins(2, 2, 2, 2);
    layout->setSpacing(6);

    initIcons();
    initStatusBar();
    pActions = initActions();
    pMenus = initMenus();

    // overwrite the default text of the new file button
    pActions[Action::FileNew]->setText(i18nc("@action New file/database", "New book..."));

    d->m_myMoneyView = new KMyMoneyView;
    // we need to setup this connection so that setupSharedActions() has a callback
    connect(d->m_myMoneyView, &KMyMoneyView::addSharedActionButton, this, &KMyMoneyApp::slotAddSharedAction);
    d->m_myMoneyView->setupSharedActions();

    layout->addWidget(d->m_myMoneyView, 10);
    connect(d->m_myMoneyView, &KMyMoneyView::statusMsg, this, &KMyMoneyApp::slotStatusMsg);
    connect(d->m_myMoneyView, &KMyMoneyView::statusProgress, this, &KMyMoneyApp::slotStatusProgressBar);
    // connect view requests
    connect(d->m_myMoneyView, &KMyMoneyView::requestSelectionChange, this, &KMyMoneyApp::slotSelectionChanged);

    connect(d->m_myMoneyView, &KMyMoneyView::requestCustomContextMenu, this, [&](eMenu::Menu type, const QPoint& pos) {
        if (pMenus.contains(type)) {
            if (type == eMenu::Menu::Schedule) {
                const auto scheduleId = d->m_selections.firstSelection(SelectedObjects::Schedule);
                pActions[eMenu::Action::EnterSchedule]->setData(scheduleId);
                pActions[eMenu::Action::EditSchedule]->setData(scheduleId);
                pActions[eMenu::Action::DeleteSchedule]->setData(scheduleId);
                pActions[eMenu::Action::SkipSchedule]->setData(scheduleId);
                pActions[eMenu::Action::DuplicateSchedule]->setData(scheduleId);
            }
            pMenus[type]->exec(pos);
        } else
            qDebug() << "Context menu for type" << static_cast<int>(type) << " not found";
    });

    connect(d->m_myMoneyView, &KMyMoneyView::requestActionTrigger, this, [&](eMenu::Action action) {
        if (pActions.contains(action)) {
            const bool enabled = pActions[action]->isEnabled();
            pActions[action]->setEnabled(true);
            pActions[action]->trigger();
            pActions[action]->setEnabled(enabled);
        }
    });

    // and setup the shared action dynamics
    connect(d->m_myMoneyView, &KMyMoneyView::selectSharedActionButton, this, [&](eMenu::Action action, QAction* newAction) {
        if (d->m_sharedActionButtons.contains(action) && d->m_sharedActionButtons.value(action).button && newAction) {
            d->m_sharedActionButtons.value(action).button->setDefaultAction(newAction);
        } else {
            for (auto buttonInfo : d->m_sharedActionButtons) {
                if (buttonInfo.button) {
                    buttonInfo.button->setDefaultAction(buttonInfo.defaultAction);
                }
            }
        }
    });

    // Initialize kactivities resource instance
#ifdef ENABLE_ACTIVITIES
    d->m_activityResourceInstance = new KActivities::ResourceInstance(window()->winId(), this);
#endif

    const auto viewActions = d->m_myMoneyView->actionsToBeConnected();
    actionCollection()->addActions(viewActions.values());
    for (auto it = viewActions.cbegin(); it != viewActions.cend(); ++it) {
        pActions.insert(it.key(), it.value());
        // set the shortcut again through the actionCollection
        if (it.value()->shortcut() != QKeySequence()) {
            actionCollection()->setDefaultShortcut(it.value(), it.value()->shortcut());
        }
    }

    ///////////////////////////////////////////////////////////////////
    // call inits to invoke all other construction parts
    readOptions();

    // now initialize the plugin structure
    createInterfaces();
    KMyMoneyPlugin::pluginHandling(KMyMoneyPlugin::Action::Load, pPlugins, this, guiFactory());
    onlineJobAdministration::instance()->setOnlinePlugins(pPlugins.extended);
    d->m_myMoneyView->setOnlinePlugins(&pPlugins.online);

    setCentralWidget(frame);

    connect(&d->m_proc, QOverload<int,QProcess::ExitStatus>::of(&KProcess::finished), this, &KMyMoneyApp::slotBackupHandleEvents);

    d->m_backupState = BACKUP_IDLE;

    QLocale locale;
    for (auto const& weekDay: locale.weekdays())
    {
        d->m_processingDays.setBit(static_cast<int>(weekDay));
    }
    d->m_autoSaveTimer = new QTimer(this);
    d->m_progressTimer = new QTimer(this);

    connect(d->m_autoSaveTimer, &QTimer::timeout, this, &KMyMoneyApp::slotAutoSave);
    connect(d->m_progressTimer, &QTimer::timeout, this, &KMyMoneyApp::slotStatusProgressDone);

    // connect the WebConnect server
    connect(d->m_webConnect, &WebConnect::gotUrl, this, &KMyMoneyApp::webConnectUrl);

    // connection to update caption
    connect(MyMoneyFile::instance(), &MyMoneyFile::dataChanged, this, [&]() {
        d->updateCaption();
    });

    // setup the initial configuration
    slotUpdateConfiguration(QString());

    // kickstart date change timer
    slotDateChanged();
    d->fileAction(eKMyMoney::FileAction::Closed);

    connect(&d->m_actionCollectorTimer, &QTimer::timeout, this, [&]() {
        // update the actions in the views
        d->updateActions(d->m_selections);
    });
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

    // destroy find transaction dialog
    delete d->m_searchDlg;

    // make sure all settings are written to disk
    KMyMoneySettings::self()->save();

    delete d;

    // clear the pointer because it could still be used in connected lambdas
    // (see KMyMoneyApp::slotAddSharedAction)
    d = nullptr;
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
        // allow the user to resize the dialog, since the contents might be large
        dialog->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
        dialog->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        if (QListWidget* widget = dialog->findChild<QListWidget *>()) {
            // give the user a hint that the data can be saved
            widget->setToolTip(i18n("This is the consistency check log, use the context menu to copy or save it."));
            widget->setWhatsThis(widget->toolTip());
            widget->setContextMenuPolicy(Qt::CustomContextMenu);
            connect(widget, &QListWidget::customContextMenuRequested, this, &KMyMoneyApp::slotShowContextMenuForConsistencyCheck);
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
    const QHash<Menu, QString> menuNames{
        {Menu::Institution, QStringLiteral("institution_context_menu")},
        {Menu::Account, QStringLiteral("account_context_menu")},
        {Menu::Schedule, QStringLiteral("schedule_context_menu")},
        {Menu::Category, QStringLiteral("category_context_menu")},
        {Menu::Tag, QStringLiteral("tag_context_menu")},
        {Menu::Payee, QStringLiteral("payee_context_menu")},
        {Menu::Investment, QStringLiteral("investment_context_menu")},
        {Menu::Security, QStringLiteral("security_context_menu")},
        {Menu::Transaction, QStringLiteral("transaction_context_menu")},
        {Menu::MoveTransaction, QStringLiteral("transaction_move_menu")},
        {Menu::MarkTransaction, QStringLiteral("transaction_mark_menu")},
        {Menu::MarkTransactionContext, QStringLiteral("transaction_context_mark_menu")},
    };

    for (auto it = menuNames.cbegin(); it != menuNames.cend(); ++it) {
        lutMenus.insert(it.key(), qobject_cast<QMenu*>(factory()->container(it.value(), this)));
    }

    d->m_actionExchanger->addExchange(lutMenus[Menu::Schedule], Qt::Key_Shift, pActions[Action::EditSchedule], pActions[Action::EditScheduleForce]);
    return lutMenus;
}

void KMyMoneyApp::slotSelectionChanged(const SelectedObjects& selections)
{
    if (!selections.isEmpty()) {
        qDebug() << "current selection";

        if (!selections.isEmpty(SelectedObjects::Institution))
            qDebug() << "Institutions:" << selections.selection(SelectedObjects::Institution);
        if (!selections.isEmpty(SelectedObjects::Account))
            qDebug() << "Accounts:" << selections.selection(SelectedObjects::Account);
        if (!selections.isEmpty(SelectedObjects::ReconciliationAccount))
            qDebug() << "ReconciliationAccounts:" << selections.selection(SelectedObjects::ReconciliationAccount);
        if (!selections.isEmpty(SelectedObjects::JournalEntry))
            qDebug() << "JournalEntries:" << selections.selection(SelectedObjects::JournalEntry);
        if (!selections.isEmpty(SelectedObjects::Payee))
            qDebug() << "Payees:" << selections.selection(SelectedObjects::Payee);
        if (!selections.isEmpty(SelectedObjects::Schedule))
            qDebug() << "Schedules:" << selections.selection(SelectedObjects::Schedule);
        if (!selections.isEmpty(SelectedObjects::Budget))
            qDebug() << "Budgets:" << selections.selection(SelectedObjects::Budget);
        if (!selections.isEmpty(SelectedObjects::OnlineJob))
            qDebug() << "OnlineJobs:" << selections.selection(SelectedObjects::OnlineJob);
        if (!selections.isEmpty(SelectedObjects::Tag))
            qDebug() << "Tags:" << selections.selection(SelectedObjects::Tag);
        if (!selections.isEmpty(SelectedObjects::Security))
            qDebug() << "Securities:" << selections.selection(SelectedObjects::Security);
        if (!selections.isEmpty(SelectedObjects::Report))
            qDebug() << "Reports:" << selections.selection(SelectedObjects::Report);
    } else {
        qDebug() << "No selections";
    }

    d->m_selections = selections;

    d->m_actionCollectorTimer.start();
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
    lutActions.insert(Action::FileNew, KStandardAction::openNew(this, &KMyMoneyApp::slotFileNew, aC));
    KStandardAction::open(this, &KMyMoneyApp::slotFileOpen, aC);
    d->m_recentFiles = KStandardAction::openRecent(this, &KMyMoneyApp::slotFileOpenRecent, aC);
    KStandardAction::save(this, &KMyMoneyApp::slotFileSave, aC);
    KStandardAction::saveAs(this, &KMyMoneyApp::slotFileSaveAs, aC);
    lutActions.insert(Action::FileClose, KStandardAction::close(this, &KMyMoneyApp::slotFileClose, aC));
    auto actionFilter = new ShortCutActionFilter(lutActions[Action::FileClose]);
    connect(actionFilter, &ShortCutActionFilter::shortCutDetected, this, &KMyMoneyApp::slotCloseViewOrFile, Qt::QueuedConnection);
    for (auto* w : qApp->topLevelWidgets()) {
        const auto mw = qobject_cast<QMainWindow*>(w);
        if (mw) {
            mw->installEventFilter(actionFilter);
        }
    }
    KStandardAction::quit(this, &KMyMoneyApp::slotFileQuit, aC);
    lutActions.insert(Action::Print, KStandardAction::print(this, &KMyMoneyApp::slotExecuteAction, aC));
    lutActions.insert(Action::PrintPreview, KStandardAction::printPreview(this, &KMyMoneyApp::slotExecuteAction, aC));
    KStandardAction::preferences(this, &KMyMoneyApp::slotSettings, aC);

    KStandardAction::showMenubar(this, &KMyMoneyApp::slotToggleMenuBar, aC);

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

        // clang-format off
        const QVector<actionInfo> actionInfos {
            // *************
            // The File menu
            // *************
            {Action::FileBackup,                    QStringLiteral("file_backup"),                    i18n("Backup..."),                                  Icon::Backup},
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
            {Action::EditFindTransaction,           QStringLiteral("edit_find_transaction"),          i18n("Find transaction..."),                        Icon::Find},
            // *************
            // The View menu
            // *************
            {Action::ViewTransactionDetail,         QStringLiteral("view_show_transaction_detail"),   i18n("Show Transaction Detail"),                    Icon::TransactionDetails},
            {Action::ViewHideReconciled,            QStringLiteral("view_hide_reconciled_transactions"), i18n("Hide reconciled transactions"),            Icon::HideReconciled},
            {Action::ViewHideCategories,            QStringLiteral("view_hide_unused_categories"),    i18n("Hide unused categories"),                     Icon::HideCategories},
            {Action::ViewShowAll,                   QStringLiteral("view_show_all_accounts"),         i18n("Show all accounts"),                          Icon::Empty},
            // *********************
            // The institutions menu
            // *********************
            {Action::NewInstitution,                QStringLiteral("institution_new"),                i18n("New institution..."),                         Icon::InstitutionNew},
            {Action::EditInstitution,               QStringLiteral("institution_edit"),               i18n("Edit institution..."),                        Icon::InstitutionEdit},
            {Action::DeleteInstitution,             QStringLiteral("institution_delete"),             i18n("Delete institution..."),                      Icon::InstitutionRemove},
            // *****************
            // The accounts menu
            // *****************
            {Action::NewAccount,                    QStringLiteral("account_new"),                    i18n("New account..."),                             Icon::AccountNew},
            {Action::OpenAccount,                   QStringLiteral("account_open"),                   i18n("Open ledger"),                                Icon::Ledger},
            {Action::StartReconciliation,           QStringLiteral("account_reconcile"),              i18n("Reconcile..."),                               Icon::Reconcile},
            {Action::FinishReconciliation,          QStringLiteral("account_reconcile_finish"),       i18nc("Finish reconciliation", "Finish"),           Icon::Reconciled},
            {Action::PostponeReconciliation,        QStringLiteral("account_reconcile_postpone"),     i18nc("Postpone reconciliation", "Postpone"),       Icon::Pause},
            {Action::CancelReconciliation,          QStringLiteral("account_reconcile_cancel"),       i18nc("Cancel reconciliation", "Cancel"),           Icon::DialogCancel},
            {Action::ReconciliationReport,          QStringLiteral("account_reconcile_report"),       i18n("Report reconciliation"),                      Icon::Empty},
            {Action::EditAccount,                   QStringLiteral("account_edit"),                   i18n("Edit account..."),                            Icon::AccountEdit},
            {Action::DeleteAccount,                 QStringLiteral("account_delete"),                 i18n("Delete account..."),                          Icon::AccountRemove},
            {Action::CloseAccount,                  QStringLiteral("account_close"),                  i18n("Close account"),                              Icon::AccountClose},
            {Action::ReopenAccount,                 QStringLiteral("account_reopen"),                 i18n("Reopen account"),                             Icon::AccountReopen},
            {Action::ReportAccountTransactions,     QStringLiteral("account_transaction_report"),     i18n("Transaction report"),                         Icon::Report},
            {Action::ChartAccountBalance,           QStringLiteral("account_chart"),                  i18n("Show balance chart..."),                      Icon::OfficeCharBar},
            {Action::MapOnlineAccount,              QStringLiteral("account_online_map"),             i18n("Map account..."),                             Icon::MapOnlineAccount},
            {Action::UnmapOnlineAccount,            QStringLiteral("account_online_unmap"),           i18n("Unmap account..."),                           Icon::UnmapOnlineAccount},
            {Action::UpdateAccount,                 QStringLiteral("account_online_update"),          i18n("Update account..."),                          Icon::AccountUpdate},
            {Action::UpdateAllAccounts,             QStringLiteral("account_online_update_all"),      i18n("Update all accounts..."),                     Icon::AccountUpdateAll},
            // *******************
            // The categories menu
            // *******************
            {Action::NewCategory,                   QStringLiteral("category_new"),                   i18n("New category..."),                            Icon::FinancialCategoryNew},
            {Action::EditCategory,                  QStringLiteral("category_edit"),                  i18n("Edit category..."),                           Icon::FinancialCategoryEdit},
            {Action::DeleteCategory,                QStringLiteral("category_delete"),                i18n("Delete category..."),                         Icon::FinancialCategoryRemove},
            // **************
            // The tools menu
            // **************
            {Action::ToolCurrencies,                QStringLiteral("tools_currency_editor"),          i18n("Currencies..."),                              Icon::Currencies},
            {Action::ToolPrices,                    QStringLiteral("tools_price_editor"),             i18n("Prices..."),                                  Icon::Empty},
            {Action::ToolUpdatePrices,              QStringLiteral("tools_update_prices"),            i18n("Update Stock and Currency Prices..."),        Icon::OnlinePriceUpdate},
            {Action::ToolConsistency,               QStringLiteral("tools_consistency_check"),        i18n("Consistency Check"),                          Icon::Empty},
            {Action::ToolPerformance,               QStringLiteral("tools_performancetest"),          i18n("Performance-Test"),                           Icon::PerformanceTest},
            {Action::ToolCalculator,                QStringLiteral("tools_kcalc"),                    i18n("Calculator..."),                              Icon::Calculator},
            // *****************
            // The settings menu
            // *****************
            {Action::SettingsAllMessages,           QStringLiteral("settings_enable_messages"),       i18n("Enable all messages"),                        Icon::Empty},
            // *************
            // The help menu
            // *************
            {Action::GetOnlineHelp,                 QStringLiteral("help_get_online_help"),           i18n("Get help from our community"),                Icon::Community},
            {Action::WhatsNew,                      QStringLiteral("help_whats_new"),                 i18n("See what's new in this version"),             Icon::DialogInformation},
            {Action::VisitWebsite,                  QStringLiteral("help_visit_website"),             i18n("Visit our website"),                          Icon::Globe},
            // ***************************
            // Actions w/o main menu entry
            // ***************************
            {Action::NewTransaction,                QStringLiteral("transaction_new"),                i18nc("New transaction button", "New transaction"), Icon::DocumentNew},
            {Action::EditTransaction,               QStringLiteral("transaction_edit"),               i18nc("Edit transaction button", "Edit"),           Icon::DocumentEdit},
            {Action::EnterTransaction,              QStringLiteral("transaction_enter"),              i18nc("Enter transaction", "Enter"),                Icon::DialogOK},
            {Action::EditSplits,                    QStringLiteral("transaction_editsplits"),         i18nc("Edit split button", "Edit splits"),          Icon::Split},
            {Action::CancelTransaction,             QStringLiteral("transaction_cancel"),             i18nc("Cancel transaction edit", "Cancel"),         Icon::DialogCancel},
            {Action::DeleteTransaction,             QStringLiteral("transaction_delete"),             i18nc("Delete transaction", "Delete"),              Icon::EditRemove},
            {Action::DuplicateTransaction,          QStringLiteral("transaction_duplicate"),          i18nc("Duplicate transaction", "Duplicate"),        Icon::EditCopy},
            {Action::AddReversingTransaction,       QStringLiteral("transaction_add_reversing"),      i18nc("Add reversing transaction", "Add reversing"),Icon::Reverse},
            {Action::AcceptTransaction,             QStringLiteral("transaction_accept"),             i18nc("Accept 'imported' and 'matched' transaction", "Accept"), Icon::DialogOK},
            {Action::ToggleReconciliationFlag,      QStringLiteral("transaction_mark_toggle"),        i18nc("Toggle reconciliation flag", "Toggle"),      Icon::Empty},
            {Action::MarkCleared,                   QStringLiteral("transaction_mark_cleared"),       i18nc("Mark transaction cleared", "Cleared"),       Icon::Empty},
            {Action::MarkReconciled,                QStringLiteral("transaction_mark_reconciled"),    i18nc("Mark transaction reconciled", "Reconciled"), Icon::Empty},
            {Action::MarkNotReconciled,             QStringLiteral("transaction_mark_notreconciled"), i18nc("Mark transaction not reconciled", "Not reconciled"),     Icon::Empty},
            {Action::MoveTransactionTo,             QStringLiteral("transaction_move"),               i18nc("Move transaction", "Move transaction"),      Icon::Empty},     // not directly available in UI
            {Action::ShowTransaction,               QStringLiteral("transaction_show"),               i18nc("Show transaction", "Show transaction"),      Icon::Empty},     // not directly available in UI
            {Action::DisplayTransactionDetails,     QStringLiteral("transaction_display_details"),    i18nc("Display transaction details", "Show transaction details"),   Icon::DocumentProperties},

            {Action::TransactionOpenURL,            QStringLiteral("transaction_open_url"),           i18nc("Open URL", "Open URL"),                      Icon::Empty},     // not directly available in UI
            {Action::SelectAllTransactions,         QStringLiteral("transaction_select_all"),         i18nc("Select all transactions", "Select all"),     Icon::SelectAll},
            {Action::GoToAccount,                   QStringLiteral("transaction_goto_account"),       i18n("Go to account"),                              Icon::BankAccount},
            {Action::GoToPayee,                     QStringLiteral("transaction_goto_payee"),         i18n("Go to payee"),                                Icon::Payee},
            {Action::NewScheduledTransaction,       QStringLiteral("transaction_create_schedule"),    i18n("Create scheduled transaction..."),            Icon::NewSchedule},
            {Action::AssignTransactionsNumber,      QStringLiteral("transaction_assign_number"),      i18n("Assign next number"),                         Icon::Empty},
            {Action::CombineTransactions,           QStringLiteral("transaction_combine"),            i18nc("Combine transactions", "Combine"),           Icon::Empty},
            {Action::MoveToToday,                   QStringLiteral("transaction_move_to_today"),      i18n("Move to today"),                              Icon::Empty},
            {Action::CopySplits,                    QStringLiteral("transaction_copy_splits"),        i18n("Copy splits"),                                Icon::Empty},
            {Action::ShowFilterWidget,              QStringLiteral("filter_show_widget"),             i18n("Show filter widget"),                         Icon::Empty},
            //Investment
            {Action::NewInvestment,                 QStringLiteral("investment_new"),                 i18n("New investment..."),                          Icon::InvestmentNew},
            {Action::EditInvestment,                QStringLiteral("investment_edit"),                i18n("Edit investment..."),                         Icon::InvestmentEdit},
            {Action::DeleteInvestment,              QStringLiteral("investment_delete"),              i18n("Delete investment..."),                       Icon::InvestmentRemove},
            {Action::UpdatePriceOnline,             QStringLiteral("investment_online_price_update"), i18n("Online price update..."),                     Icon::OnlinePriceUpdate},
            {Action::UpdatePriceManually,           QStringLiteral("investment_manual_price_update"), i18n("Manual price update..."),                     Icon::Empty},
            {Action::EditSecurity,                  QStringLiteral("security_edit"),                  i18n("Edit security..."),                           Icon::InvestmentEdit},
            {Action::DeleteSecurity,                QStringLiteral("security_delete"),                i18n("Delete security..."),                         Icon::InvestmentRemove},

            //Schedule
            {Action::NewSchedule,                   QStringLiteral("schedule_new"),                   i18n("New schedule..."),                            Icon::NewSchedule},
            {Action::EditSchedule,                  QStringLiteral("schedule_edit"),                  i18n("Edit scheduled transaction"),                 Icon::DocumentEdit},
            {Action::EditScheduleForce,             QStringLiteral("schedule_edit_force"),            i18n("Edit scheduled transaction"),                 Icon::DocumentEdit},
            {Action::DeleteSchedule,                QStringLiteral("schedule_delete"),                i18n("Delete scheduled transaction"),               Icon::EditRemove},
            {Action::DuplicateSchedule,             QStringLiteral("schedule_duplicate"),             i18n("Duplicate scheduled transaction"),            Icon::EditCopy},
            {Action::EnterSchedule,                 QStringLiteral("schedule_enter"),                 i18n("Enter next transaction..."),                  Icon::KeyEnter},
            {Action::SkipSchedule,                  QStringLiteral("schedule_skip"),                  i18n("Skip next transaction..."),                   Icon::SeekForward},
            //Payees
            {Action::NewPayee,                      QStringLiteral("payee_new"),                      i18n("New payee..."),                               Icon::PayeeNew},
            {Action::RenamePayee,                   QStringLiteral("payee_rename"),                   i18n("Rename payee"),                               Icon::PayeeRename},
            {Action::DeletePayee,                   QStringLiteral("payee_delete"),                   i18n("Delete payee"),                               Icon::PayeeRemove},
            {Action::MergePayee,                    QStringLiteral("payee_merge"),                    i18n("Merge payees"),                               Icon::PayeeMerge},
            //Tags
            {Action::NewTag,                        QStringLiteral("tag_new"),                        i18n("New tag..."),                                 Icon::TagNew},
            {Action::RenameTag,                     QStringLiteral("tag_rename"),                     i18n("Rename tag"),                                 Icon::TagRename},
            {Action::DeleteTag,                     QStringLiteral("tag_delete"),                     i18n("Delete tag"),                                 Icon::TagRemove},
            //Reports
            {Action::ReportOpen,                    QStringLiteral("report_open"),                    i18n("Open report"),                                Icon::Report},
            {Action::ReportNew,                     QStringLiteral("report_new"),                     i18n("New report"),                                 Icon::DocumentNew},
            {Action::ReportCopy,                    QStringLiteral("report_copy"),                    i18n("Copy report"),                                Icon::EditCopy},
            {Action::ReportConfigure,               QStringLiteral("report_configure"),               i18n("Configure report"),                           Icon::DocumentProperties},
            {Action::ReportExport,                  QStringLiteral("report_export"),                  i18n("Export report"),                              Icon::Empty},
            {Action::ReportDelete,                  QStringLiteral("report_delete"),                  i18n("Delete report"),                              Icon::EditRemove},
            {Action::ReportClose,                   QStringLiteral("report_close"),                   i18n("Close report"),                               Icon::Close},
            {Action::ReportToggleChart,             QStringLiteral("report_toggle"),                  i18n("Toggle chart"),                               Icon::Close},
            {Action::EditTabOrder,                  QStringLiteral("edit_taborder"),                  i18n("Edit tab order"),                             Icon::Empty},
            //debug actions
            {Action::LedgerQuickOpen,               QStringLiteral("ledger_quick_open"),              i18n("Ledger Quick Open"),                          Icon::Empty},
#ifdef KMM_DEBUG
            {Action::NewFeature,                    QStringLiteral("new feature"),                    i18n("Test new feature"),                           Icon::Empty},
            {Action::DebugTraces,                   QStringLiteral("debug_traces"),                   i18n("Debug Traces"),                               Icon::Empty},
#endif
            {Action::DebugTimers,                   QStringLiteral("debug_timers"),                   i18n("Debug Timers"),                               Icon::Empty},
            // onlineJob actions
        };
        // clang-format on

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

        auto a = new KDualAction(this);
        a->setObjectName(QStringLiteral("transaction_match"));
        a->setActiveText(i18nc("Match transactions", "Match"));
        a->setInactiveText(i18nc("Unmatch transactions", "Unmatch"));
        a->setActiveIcon(Icons::get(Icon::Link));
        a->setInactiveIcon(Icons::get(Icon::Unlink));
        a->setActive(true);
        a->setAutoToggle(false);
        a->setEnabled(false);
        lutActions.insert(Action::MatchTransaction, a);
    }

    lutActions.insert(Action::EditUndo, KUndoActions::createUndoAction(MyMoneyFile::instance()->undoStack(), aC));
    lutActions.insert(Action::EditRedo, KUndoActions::createRedoAction(MyMoneyFile::instance()->undoStack(), aC));

    {
        // List with slots that get connected here. Other slots get connected in e.g. appropriate views
        // clang-format off
        typedef void(KMyMoneyApp::*KMyMoneyAppFunc)();
        const QHash<eMenu::Action, KMyMoneyAppFunc> actionConnections{
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
            // *************
            // The Account menu
            // *************
            {Action::CloseAccount,                  &KMyMoneyApp::slotCloseAccount},
            {Action::ReopenAccount,                 &KMyMoneyApp::slotReopenAccount},

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
            {Action::EditTabOrder,                  &KMyMoneyApp::slotEditTabOrder},
            // *****************
            // The help menu
            // *****************
            {Action::GetOnlineHelp,                 &KMyMoneyApp::slotGetOnlineHelp},
            {Action::WhatsNew,                      &KMyMoneyApp::slotWhatsNew},
            {Action::VisitWebsite,                  &KMyMoneyApp::slotVisitWebsite},
            // ***************************
            // Actions w/o main menu entry
            // ***************************
            //debug actions
#ifdef KMM_DEBUG
            {Action::NewFeature,                    &KMyMoneyApp::slotNewFeature},
            {Action::DebugTraces,                   &KMyMoneyApp::slotToggleTraces},
#endif
            {Action::DebugTimers,                   &KMyMoneyApp::slotToggleTimers},

            {Action::OpenAccount,                   &KMyMoneyApp::slotExecuteAction},
            {Action::NewTransaction,                &KMyMoneyApp::slotExecuteAction},
            {Action::EditTransaction,               &KMyMoneyApp::slotExecuteAction},
            {Action::EditSplits,                    &KMyMoneyApp::slotExecuteAction},
            {Action::SelectAllTransactions,         &KMyMoneyApp::slotExecuteAction},
            {Action::DeleteTransaction,             &KMyMoneyApp::slotDeleteTransactions},
            {Action::DuplicateTransaction,          &KMyMoneyApp::slotDuplicateTransactions},
            {Action::AddReversingTransaction,       &KMyMoneyApp::slotDuplicateTransactions},
            {Action::DisplayTransactionDetails,     &KMyMoneyApp::slotDisplayTransactionDetails},
            {Action::CopySplits,                    &KMyMoneyApp::slotCopySplits},
            {Action::MarkCleared,                   &KMyMoneyApp::slotMarkTransactions},
            {Action::MarkReconciled,                &KMyMoneyApp::slotMarkTransactions},
            {Action::MarkNotReconciled,             &KMyMoneyApp::slotMarkTransactions},
            {Action::ToggleReconciliationFlag,      &KMyMoneyApp::slotMarkTransactions},
            {Action::MoveTransactionTo,             &KMyMoneyApp::slotMoveTransactionTo},
            {Action::MatchTransaction,              &KMyMoneyApp::slotMatchTransaction},
            {Action::AcceptTransaction,             &KMyMoneyApp::slotAcceptTransaction},
            {Action::ShowTransaction,               &KMyMoneyApp::slotExecuteAction},
            {Action::TransactionOpenURL,            &KMyMoneyApp::slotTransactionOpenURL},

            {Action::StartReconciliation,           &KMyMoneyApp::slotStartReconciliation},
            {Action::FinishReconciliation,          &KMyMoneyApp::slotExecuteAction},
            {Action::PostponeReconciliation,        &KMyMoneyApp::slotExecuteAction},
            {Action::CancelReconciliation,          &KMyMoneyApp::slotExecuteAction},
            {Action::ReconciliationReport,          &KMyMoneyApp::slotReportReconciliation},

            {Action::NewScheduledTransaction,       &KMyMoneyApp::slotCreateScheduledTransaction},

            {Action::GoToPayee,                     &KMyMoneyApp::slotExecuteActionWithData},
            {Action::GoToAccount,                   &KMyMoneyApp::slotExecuteActionWithData},
            {Action::ReportOpen,                    &KMyMoneyApp::slotExecuteActionWithData},
            {Action::ReportAccountTransactions,     &KMyMoneyApp::slotExecuteAction},
            {Action::ChartAccountBalance,           &KMyMoneyApp::slotExecuteAction},

            {Action::EditFindTransaction,           &KMyMoneyApp::slotFindTransaction},
            {Action::MoveToToday,                   &KMyMoneyApp::slotMoveToToday},
        };
        // clang-format off

        for (auto connection = actionConnections.cbegin(); connection != actionConnections.cend(); ++connection)
            connect(lutActions[connection.key()], &QAction::triggered, this, connection.value());
    }

    {
        // struct for adding tooltips to actions
        struct actionInfo {
            Action  action;
            QString tip;
        };

        // clang-format off
        const QVector<actionInfo> actionInfos {
            {Action::StartReconciliation,        i18nc("@info:tooltip", "Start reconciling the currently selected account")},
            {Action::PostponeReconciliation,     i18nc("@info:tooltip", "Postpone the current reconciliation")},
            {Action::FinishReconciliation,       i18nc("@info:tooltip", "Finish the current reconciliation")},
            {Action::CancelReconciliation,       i18nc("@info:tooltip", "Cancel the current reconciliation")},
        };
        // clang-format on

        for (const auto& actionInfo : actionInfos) {
            lutActions[actionInfo.action]->setToolTip(actionInfo.tip);
        }
    }

    // *************
    // Setting some of added actions checkable
    // *************
    {
        // Some actions are checkable,
        // so set them here
        const QVector<Action> checkableActions {
            Action::ViewTransactionDetail,
            Action::ViewHideReconciled,
            Action::ViewHideCategories,
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
            Action::GetOnlineHelp,
            Action::SettingsAllMessages,
            Action::ToolPerformance,
            Action::ToolCalculator,
        };
        for (const auto& action : alwaysEnabled) {
            lutActions[action]->setEnabled(true);
        }
    }

    // *************
    // Setting keyboard shortcuts for some of added actions
    // *************
    //
        {
        // clang-format off
            const QVector<QPair<Action, QKeySequence>> actionShortcuts{
            {qMakePair(Action::EditFindTransaction,         Qt::CTRL | Qt::SHIFT | Qt::Key_F)},
            {qMakePair(Action::ViewTransactionDetail,       Qt::CTRL | Qt::Key_T)},
            {qMakePair(Action::ViewHideReconciled,          Qt::CTRL | Qt::Key_R)},
            {qMakePair(Action::ViewHideCategories,          Qt::CTRL | Qt::Key_U)},
            {qMakePair(Action::ViewShowAll,                 Qt::CTRL | Qt::SHIFT | Qt::Key_A)},
            {qMakePair(Action::StartReconciliation,         Qt::CTRL | Qt::SHIFT | Qt::Key_R)},
            {qMakePair(Action::NewTransaction,              Qt::CTRL | Qt::Key_Insert)},
            {qMakePair(Action::DuplicateTransaction,        Qt::CTRL | Qt::Key_D)},
            {qMakePair(Action::DeleteTransaction,           Qt::CTRL | Qt::Key_Delete)},
            {qMakePair(Action::EditTransaction,             Qt::CTRL | Qt::Key_E)},
            {qMakePair(Action::EditSplits,                  Qt::CTRL | Qt::SHIFT | Qt::Key_E)},
            {qMakePair(Action::CopySplits,                  Qt::CTRL | Qt::SHIFT | Qt::Key_C)},
            {qMakePair(Action::AddReversingTransaction,     Qt::CTRL | Qt::SHIFT | Qt::Key_R)},
            {qMakePair(Action::AddReversingTransaction,     Qt::CTRL | Qt::SHIFT | Qt::Key_Backspace)},
            {qMakePair(Action::ToggleReconciliationFlag,    Qt::CTRL | Qt::Key_Space)},
            {qMakePair(Action::MarkCleared,                 Qt::CTRL | Qt::ALT | Qt::Key_Space)},
            {qMakePair(Action::MarkNotReconciled,           Qt::CTRL | Qt::SHIFT | Qt::ALT | Qt::Key_Space)},
            {qMakePair(Action::MarkReconciled,              Qt::CTRL | Qt::SHIFT | Qt::Key_Space)},
            {qMakePair(Action::MoveToToday,                 Qt::CTRL | Qt::SHIFT | Qt::Key_T)},
            {qMakePair(Action::GoToPayee,                   Qt::CTRL | Qt::SHIFT | Qt::Key_P)},
            {qMakePair(Action::SelectAllTransactions,       Qt::CTRL | Qt::Key_A)},
            {qMakePair(Action::EditTabOrder,                Qt::CTRL | Qt::ALT | Qt::Key_T)},
#ifdef KMM_DEBUG
            {qMakePair(Action::NewFeature,                  Qt::CTRL | Qt::Key_G)},
#endif
            {qMakePair(Action::AssignTransactionsNumber,    Qt::CTRL | Qt::SHIFT | Qt::Key_N)},
            {qMakePair(Action::ShowFilterWidget,            Qt::CTRL | Qt::Key_F)},
            {qMakePair(Action::LedgerQuickOpen,             Qt::CTRL | Qt::ALT | Qt::Key_O)},
            };
        // clang-format on

        for (const auto& it : actionShortcuts) {
            actionCollection()->setDefaultShortcut(lutActions[it.first], it.second);
        }
    }

    // *************
    // Misc settings
    // *************

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
    auto aboutApp = aC->action(QT6_SKIP_FROM_LATIN_STRING(KStandardAction::name(KStandardAction::AboutApp)));
    aboutApp->disconnect();
    connect(aboutApp, &QAction::triggered, this, &KMyMoneyApp::slotShowCredits);

    QMenu *menuContainer;
    menuContainer = static_cast<QMenu*>(factory()->container(QStringLiteral("import"), this));
    menuContainer->setIcon(Icons::get(Icon::DocumentImport));

    menuContainer = static_cast<QMenu*>(factory()->container(QStringLiteral("export"), this));
    menuContainer->setIcon(Icons::get(Icon::DocumentExport));

    return lutActions;
}

void KMyMoneyApp::slotAddSharedAction(eMenu::Action action, QAction* defaultAction)
{
    auto toolButton = d->m_sharedActionButtons.value(action).button;
    if (toolButton == nullptr) {
        auto actionObject = pActions.value(action, nullptr);
        if (actionObject) {
            for (auto* widget : QT6_IF(actionObject->associatedObjects(), actionObject->associatedWidgets())) {
                toolButton = qobject_cast<QToolButton*>(widget);
                if (toolButton) {
                    d->m_sharedActionButtons[action].button = toolButton;
                    d->m_sharedActionButtons[action].defaultAction = actionObject;
                    connect(toolButton, &QObject::destroyed, this, [&](QObject* button) {
                        // we maybe called after we destroyed our private object already
                        if (d != nullptr) {
                            QHash<eMenu::Action, KMyMoneyApp::Private::SharedActionButtonInfo>::iterator it;
                            for (it = d->m_sharedActionButtons.begin(); it != d->m_sharedActionButtons.end();) {
                                if ((*it).button == button) {
                                    d->m_sharedActionButtons.remove(it.key());
                                    // start from beginning since container changed
                                    it = d->m_sharedActionButtons.begin();
                                    QMetaObject::invokeMethod(d->m_myMoneyView, &KMyMoneyView::setupSharedActions, Qt::QueuedConnection);
                                    continue;
                                }
                                ++it;
                            }
                        }
                    });
                    break;
                }
            }
        }
    }

    if (toolButton) {
        auto currentAction = toolButton->defaultAction();
        toolButton->setDefaultAction(defaultAction);
        toolButton->setDefaultAction(currentAction);
    }
}

#ifdef KMM_DEBUG
void KMyMoneyApp::dumpActions() const
{
    const QList<QAction*> list = actionCollection()->actions();
    for (const auto& it : list)
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

void KMyMoneyApp::initIcons()
{
    qDebug() << "System icon theme as reported by QT: " << QIcon::themeName();

    auto themeName = KMyMoneySettings::iconsTheme();
    qDebug() << "App icon theme as configured in KMyMoney: " << themeName;

#if defined(Q_OS_WIN)
    // @todo add support for dark icons, e.g. https://www.thetopsites.net/article/51334674.shtml

    themeName = QStringLiteral("breeze");                      // only breeze is available for craft packages
    qDebug() << "Running under Windows, so will be forcing the icon theme to: " << themeName;
#elif defined(Q_OS_MACOS)
    constexpr int OSX_LIGHT_MODE = 236;

    auto bg = palette().color(QPalette::Active, QPalette::Window);

    if (bg.lightness() == OSX_LIGHT_MODE) {
        themeName = QStringLiteral("breeze");
        qDebug() << "Detected macOS light mode, so will be forcing the icon theme to: " << themeName;
    }
    else {
        themeName = QStringLiteral("breeze-dark");
        qDebug() << "Detected macOS dark mode, so will be forcing the icon theme to: " << themeName;
    }
#endif

    // if it isn't default theme then set it
    if (!themeName.isEmpty() && themeName != QStringLiteral("system")) {
        QIcon::setThemeName(themeName);
        qDebug() << "Setting icon theme to: " << themeName;
    }
    else {
        themeName = QIcon::themeName();
        qDebug() << "Obeying the system-wide icon theme, currently set to: " << themeName;
    }
}

void KMyMoneyApp::saveOptions()
{
    KConfigGroup grp = d->m_config->group("General Options");
    grp.writeEntry("Geometry", size());

    grp.writeEntry("Show Statusbar", actionCollection()->action(KStandardAction::name(KStandardAction::ShowStatusbar))->isChecked());
    grp.writeEntry("Show Menu Bar", actionCollection()->action(KStandardAction::name(KStandardAction::ShowMenubar))->isChecked());

    KConfigGroup toolbarGrp = d->m_config->group("mainToolBar");
    toolBar("mainToolBar")->saveSettings(toolbarGrp);

    d->m_recentFiles->saveEntries(d->m_config->group("Recent Files"));
}

void KMyMoneyApp::readOptions()
{
    KConfigGroup grp = d->m_config->group("General Options");
    actionCollection()->action(KStandardAction::name(KStandardAction::ShowStatusbar))->setChecked(grp.readEntry("Show Statusbar", true));

    const auto showMenu = grp.readEntry("Show Menu Bar", true);
    const auto action = actionCollection()->action(KStandardAction::name(KStandardAction::ShowMenubar));
    action->setChecked(showMenu);
    // setting the action does not emit the triggered signal
    // so we set the intial state directly
    showMenu ? menuBar()->show() : menuBar()->hide();

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
    qApp->quit();
    return true;
}

/////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATION
/////////////////////////////////////////////////////////////////////
void KMyMoneyApp::slotFileInfoDialog()
{
    QPointer<KMyMoneyFileInfoDlg> dlg = new KMyMoneyFileInfoDlg(nullptr);
    dlg->exec();
    delete dlg;
}

void KMyMoneyApp::slotPerformanceTest()
{
    // dump performance report to stderr

    int measurement[2];
    QElapsedTimer timer;
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
            MyMoneyFile::instance()->transactionList(list, filter);
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
    QMap<QString, KMyMoneyPlugin::ImporterPlugin*>::const_iterator it_plugin = pPlugins.importer.cbegin();
    while (it_plugin != pPlugins.importer.cend()) {
        if ((*it_plugin)->isMyFormat(url.toLocalFile())) {
            result = true;
            break;
        }
        ++it_plugin;
    }

    // If we did not find a match, try importing it as a KMM statement file,
    // which is really just for testing.  the statement file is not exposed
    // to users.
    if (it_plugin == pPlugins.importer.cend())
        if (MyMoneyStatement::isStatementFile(url.path()))
            result = true;

    // Place code here to test for QIF and other locally-supported formats
    // (i.e. not a plugin). If you add them here, be sure to add it to
    // the webConnect function.

    return result;
}

void KMyMoneyApp::slotShowTransactionDetail()
{
    const auto show = pActions[Action::ViewTransactionDetail]->isChecked();
    // make persistent
    KMyMoneySettings::setShowRegisterDetailed(show);
    // and inform the views
    LedgerViewSettings::instance()->setShowTransactionDetails(show);
}

void KMyMoneyApp::slotDeleteTransactions()
{
    const auto file = MyMoneyFile::instance();

    // since we may jump here via code, we have to make sure to react only
    // if the action is enabled
    if (!pActions[Action::DeleteTransaction]->isEnabled())
        return;

    if (d->m_selections.selection(SelectedObjects::JournalEntry).isEmpty())
        return;

    if (MyMoneyUtils::transactionWarnLevel(d->m_selections.selection(SelectedObjects::JournalEntry)) == OneSplitReconciled) {
        if (KMessageBox::warningContinueCancel(this,
            i18n("At least one split of the selected transactions has been reconciled. "
            "Do you wish to delete the transactions anyway?"),
            i18n("Transaction already reconciled")) == KMessageBox::Cancel)
            return;

    } else {
        auto msg =
        i18np("Do you really want to delete the selected transaction?",
            "Do you really want to delete all %1 selected transactions?",
            d->m_selections.selection(SelectedObjects::JournalEntry).count());

        if (KMessageBox::questionTwoActions(this, msg, i18n("Delete transaction"), KMMYesNo::yes(), KMMYesNo::no()) == KMessageBox::SecondaryAction) {
            return;
        }
    }

    MyMoneyFileTransaction ft;
    for (const auto& journalEntryId : d->m_selections.selection(SelectedObjects::JournalEntry)) {
        const auto journalEntry = file->journalModel()->itemById(journalEntryId);
        if (!journalEntry.id().isEmpty()) {
            if (!journalEntry.transaction().id().isEmpty()) {
                if (!file->referencesClosedAccount(journalEntry.transaction())) {
                    file->removeTransaction(journalEntry.transaction());
                }
            }
        }
    }
    ft.commit();
}

void KMyMoneyApp::slotDuplicateTransactions()
{
    auto action = qobject_cast<QAction*>(sender());
    const auto actionId = d->qActionToId(action);

    const auto reverse = (actionId == eMenu::Action::AddReversingTransaction);
    const auto accountId = d->m_selections.firstSelection(SelectedObjects::Account);

    if (d->m_selections.selection(SelectedObjects::JournalEntry).isEmpty()
        || accountId.isEmpty())
        return;

    MyMoneyFileTransaction ft;
    QString lastAddedTransactionId;
    const auto file = MyMoneyFile::instance();

    try {
        for (const auto& journalEntryId : d->m_selections.selection(SelectedObjects::JournalEntry)) {
            const auto journalEntry = file->journalModel()->itemById(journalEntryId);
            if (!journalEntry.id().isEmpty()) {
                auto t = journalEntry.transaction();
                if (!t.id().isEmpty()) {
                    // wipe out any reconciliation information
                    for (auto& split : t.splits()) {
                        split.setReconcileFlag(eMyMoney::Split::State::NotReconciled);
                        split.setReconcileDate(QDate());
                        split.setBankID(QString());
                        split.removeMatch();
                    }
                    // clear invalid data
                    t.setEntryDate(QDate());
                    t.clearId();

                    if (reverse)
                        // reverse transaction
                        t.reverse();
                    else
                        // set the post date to today
                        t.setPostDate(QDate::currentDate());

                    file->addTransaction(t);
                    lastAddedTransactionId = t.id();
                }
            }
        }
        ft.commit();

        // select the new transaction in the ledger
        auto selections = d->m_selections;
        const auto indeces = file->journalModel()->indexesByTransactionId(lastAddedTransactionId);
        for (const auto& idx : indeces) {
            if (idx.data(eMyMoney::Model::JournalSplitAccountIdRole).toString() == accountId) {
                selections.setSelection(SelectedObjects::JournalEntry, idx.data(eMyMoney::Model::IdRole).toString());
            }
        }
        d->m_myMoneyView->executeAction(eMenu::Action::OpenAccount, selections);

    } catch (const MyMoneyException &e) {
        KMessageBox::detailedError(this, i18n("Unable to duplicate transaction(s)"), QString::fromLatin1(e.what()));
    }
}

void KMyMoneyApp::slotDisplayTransactionDetails()
{
    // Display popup with full transaction details, since some are never displayed in the UI
    const auto file = MyMoneyFile::instance();
    QString head =
        "<html><head><style>table, tr, th, td {border: 1px solid black; border-collapse: collapse; padding: 4px} "
        ".spec {border-bottom-width: 3px}"
        "</style></head><body>";
    QString trans;
    QString kvpt;
    QString splits;
    QString kvps;
    for (const auto& journalEntryId : d->m_selections.selection(SelectedObjects::JournalEntry)) {
        const auto journalEntry = file->journalModel()->itemById(journalEntryId);
        if (!journalEntry.id().isEmpty()) {
            const auto t = journalEntry.transaction();
            const auto commodity = t.commodity();
            const auto safraction = MyMoneyFile::instance()->security(commodity).smallestAccountFraction();
            if (!t.id().isEmpty()) {
                head += "<H1>Transaction: " + t.id() + "</H1>";
                // transaction
                trans = "<table>";
                trans += "<tr><th>Entry date</th><td>" + t.entryDate().toString(Qt::ISODate) + "</td></tr>";
                trans += "<tr><th>Post date</th><td>" + t.postDate().toString(Qt::ISODate) + "</td></tr>";
                trans += "<tr><th>Currency</th><td>" + t.commodity() + "</td></tr>";
                trans += "<tr><th>Memo</th><td>" + t.memo() + "</td></tr>";
                trans += "</table>";
                // Transaction KVPs
                if (t.pairs().count() > 0) {
                    kvpt = "<p><b>Key Value Pairs</b></p><table><tr><th>Key</th><th>Value</th></tr>";
                    for (auto it = t.pairs().cbegin(); it != t.pairs().cend(); ++it) {
                        kvpt += "<tr><th>" + it.key() + "</th><th>" + it.value() + "</th></tr>";
                    }
                    kvpt += "</table><br/><br/>";
                } else {
                    kvpt = "<br/>";
                }
                // splits: [id] payee account reconcileFlag reconcileDate action shares price value memo costCenter tagList number
                splits =
                    "<table>"
                    "<tr><th>Split</th><th>Number</th><th>Payee</th><th>Account</th><th>_CR</th><th>ReconcileDate</th>"
                    "<th>Action</th><th>Shares</th><th>Price</th><th>Value</th>"
                    "<th>BankID</th></th></tr>"
                    "<tr><th>&nbsp;</th><th colspan=\"10\" align=\"left\">Memo</th></tr>"
                    "<tr><th>&nbsp;</th><th colspan=\"10\" align=\"left\">Tags</th></tr>";
                for (const auto& split : t.splits()) {
                    // split: id payee account reconcileflag reconciledate activity shares price value memo
                    const auto splitAccount = MyMoneyFile::instance()->account(split.accountId());
                    const auto pairs = splitAccount.pairs();
                    const auto accopen = !splitAccount.isClosed();
                    const auto splitSymbol = splitAccount.accountType() == eMyMoney::Account::Type::Stock
                        ? MyMoneyFile::instance()->security(splitAccount.currencyId()).tradingSymbol()
                        : splitAccount.currencyId();
                    splits += "<tr></tr><tr><td>" + split.id() + "</td><td>";
                    splits += split.number() + "</td><td>";
                    splits += split.payeeId() + "<br/>" + MyMoneyFile::instance()->payee(split.payeeId()).name() + "</td><td>";
                    if (accopen == true) {
                        splits += split.accountId();
                    } else {
                        splits += "<s>" + split.accountId() + "</s> (Closed)";
                    }
                    splits += "<br/>" + splitAccount.name() + "</td><td>" + KMyMoneyUtils::reconcileStateToString(split.reconcileFlag());
                    splits += "</td><td>" + split.reconcileDate().toString(Qt::ISODate) + "</td><td>" + split.action() + "</td><td>";
                    splits += split.shares().formatMoney(splitAccount.fraction(), true) + "<br/>(" + splitSymbol + " ";
                    splits += QString::number(splitAccount.fraction()) + ")</td><td>";
                    splits += split.possiblyCalculatedPrice().formatMoney(safraction > splitAccount.fraction() ? safraction / splitAccount.fraction()
                                                                                                               : splitAccount.fraction(),
                                                                          true);
                    splits += "</td><td>" + split.value().formatMoney(safraction, true) + "<br/>(" + t.commodity() + " " + QString::number(safraction);
                    splits += ")</td><td>" + split.bankID() + "</td></tr><tr><td>&nbsp;</td><td colspan=\"10\">" + split.memo() + "</td></tr>";

                    // collect tags and add them to the view
                    const auto tagIdList = split.tagIdList();
                    QString tagList;
                    for (const auto& tagId : qAsConst(tagIdList)) {
                        if (!tagList.isEmpty()) {
                            tagList += QLatin1String(", ");
                        }
                        const auto tag = MyMoneyFile::instance()->tag(tagId);
                        tagList += tag.name().simplified();
                    }
                    splits += "<tr><td>&nbsp;</td><td colspan=\"10\">" + tagList + "</td></tr>";

                    if (split.pairs().count() > 0) {
                        splits += "<tr><th>KVP</th><th colspan=\"2\">Key</th><th colspan=\"8\">Value</th></tr>";
                        for (auto it = split.pairs().cbegin(); it != split.pairs().cend(); ++it) {
                            splits += "<tr><td></td><td colspan=\"2\">" + it.key() + "</td><td colspan=\"8\">" + it.value() + "</td></tr>";
                        }
                    }
                }
                splits +=
                    "</table><p align=\"center\">The Value column uses the currency of the transaction, which may not be the currency of the account "
                    "of the split.<br/>Shown in parentheses are the currency or trading symbol of the split and the fraction used to display value.</p>";
            } else {
                head += "<p>There is no selected Transaction.</p>";
            }
        } else {
            head += "<p>There appears to be nothing selected.</p>";
        }
        QString foot = "</body></html>";
        QString all = head + trans + kvpt + splits + foot;

        QDialog* txInfo = new QDialog;
        txInfo->setAttribute(Qt::WA_DeleteOnClose);

        txInfo->setMinimumSize(900, 600);
        QTextBrowser* text = new QTextBrowser();
        text->setHtml(all);
        QPushButton* doneButton = new QPushButton(i18nc("@action:button", "Close"), txInfo);
        doneButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        connect(doneButton, &QPushButton::clicked, txInfo, &QDialog::deleteLater);
        QVBoxLayout* layout = new QVBoxLayout(txInfo);
        layout->addWidget(text);
        layout->addWidget(doneButton);
        layout->setAlignment(doneButton, Qt::AlignHCenter);
        txInfo->show();
    }
}

void KMyMoneyApp::slotCopySplits()
{
    const auto file = MyMoneyFile::instance();
    const auto accountId = d->m_selections.firstSelection(SelectedObjects::Account);

    if (!accountId.isEmpty() && (d->m_selections.selection(SelectedObjects::JournalEntry).count() >= 2)) {
        int singleSplitTransactions = 0;
        int multipleSplitTransactions = 0;
        MyMoneyTransaction selectedSourceTransaction;

        QString selectedSourceSplitId;
        for (const auto& journalEntryId : d->m_selections.selection(SelectedObjects::JournalEntry)) {
            const auto journalEntry = file->journalModel()->itemById(journalEntryId);
            if (!journalEntry.id().isEmpty()) {
                const auto t = journalEntry.transaction();
                switch (t.splitCount()) {
                    case 0:
                        break;
                    case 1:
                        singleSplitTransactions++;
                        break;
                    default:
                        selectedSourceTransaction = t;
                        selectedSourceSplitId = journalEntry.split().id();
                        multipleSplitTransactions++;
                        break;
                }
            }
        }
        if (singleSplitTransactions > 0 && multipleSplitTransactions == 1) {
            MyMoneyFileTransaction ft;
            try {
                // remember the splitId that is used in the source transaction
                // as the link
                for (const auto& journalEntryId : d->m_selections.selection(SelectedObjects::JournalEntry)) {
                    const auto journalEntry = file->journalModel()->itemById(journalEntryId);
                    if (!journalEntry.id().isEmpty()) {
                        auto t = journalEntry.transaction();
                        // don't process the source transaction
                        if (selectedSourceTransaction.id() == t.id()) {
                            continue;
                        }

                        if (!t.id().isEmpty() && (t.splitCount() == 1)) {
                            // keep a copy of that one and only split
                            const auto baseSplit = t.splits().first();

                            for (const auto& split : selectedSourceTransaction.splits()) {
                                // Don't copy the source split, as we already have that
                                // as part of the destination transaction
                                if (split.id() == selectedSourceSplitId) {
                                    continue;
                                }

                                MyMoneySplit sp(split);
                                // clear the ID, reconciliation state, match information and data stored in KVP
                                // update the payee to the one in the baseSplit
                                sp.clearId();
                                sp.setReconcileFlag(eMyMoney::Split::State::NotReconciled);
                                sp.setReconcileDate(QDate());
                                sp.removeMatch();
                                sp.clear();
                                sp.setPayeeId(baseSplit.payeeId());

                                // in case it is a simple transaction consisting of two splits,
                                // we can adjust the share and value part of the second split we
                                // just created. We need to keep a possible price in mind in case
                                // of different currencies
                                if (selectedSourceTransaction.splitCount() == 2) {
                                    sp.setValue(-baseSplit.value());
                                    sp.setShares(-(baseSplit.shares() * baseSplit.possiblyCalculatedPrice()));
                                }
                                t.addSplit(sp);
                            }
                            // check if we need to add/update a VAT assignment
                            file->updateVAT(t);

                            // and store the modified transaction
                            file->modifyTransaction(t);
                        }
                    }
                }
                ft.commit();
            } catch (const MyMoneyException &) {
                qDebug() << "transactionCopySplits() failed";
            }
        }
    }
}

void KMyMoneyApp::slotMarkTransactions()
{
    auto action = qobject_cast<QAction*>(sender());
    const auto actionId = d->qActionToId(action);
    const auto file = MyMoneyFile::instance();

    static const QHash<eMenu::Action, eMyMoney::Split::State> action2state = {
        {eMenu::Action::MarkNotReconciled, eMyMoney::Split::State::NotReconciled},
        {eMenu::Action::MarkCleared, eMyMoney::Split::State::Cleared},
        {eMenu::Action::MarkReconciled, eMyMoney::Split::State::Reconciled},
        {eMenu::Action::ToggleReconciliationFlag, eMyMoney::Split::State::Unknown},
    };

    const auto flag = action2state.value(actionId, eMyMoney::Split::State::Unknown);
    if (actionId == eMenu::Action::None) {
        return;
    }

    auto cnt = d->m_selections.selection(SelectedObjects::JournalEntry).count();

    MyMoneyFileTransaction ft;
    try {
        const auto isReconciliationMode = !d->m_selections.firstSelection(SelectedObjects::ReconciliationAccount).isEmpty();
        for (const auto& journalEntryId : d->m_selections.selection(SelectedObjects::JournalEntry)) {
            // turn on signals before we modify the last entry in the list
            cnt--;
            MyMoneyFile::instance()->blockSignals(cnt != 0);

            // get a fresh copy
            auto journalEntry = file->journalModel()->itemById(journalEntryId);
            if (!journalEntry.id().isEmpty()) {
                auto t = journalEntry.transaction();
                auto sp = journalEntry.split();
                if (sp.reconcileFlag() != flag) {
                    if (flag == eMyMoney::Split::State::Unknown) {
                        if (!isReconciliationMode) {
                            // in normal mode we cycle through all states
                            // except when reconciled transactions are hidden
                            switch (sp.reconcileFlag()) {
                                case eMyMoney::Split::State::NotReconciled:
                                    sp.setReconcileFlag(eMyMoney::Split::State::Cleared);
                                    break;
                                case eMyMoney::Split::State::Cleared:
                                    sp.setReconcileFlag(KMyMoneySettings::hideReconciledTransactions() ? eMyMoney::Split::State::NotReconciled
                                                                                                       : eMyMoney::Split::State::Reconciled);
                                    t.setImported(false);
                                    break;
                                case eMyMoney::Split::State::Reconciled:
                                    sp.setReconcileFlag(eMyMoney::Split::State::NotReconciled);
                                    break;
                                default:
                                    break;
                            }
                        } else {
                            // in reconciliation mode we skip the reconciled state
                            switch (sp.reconcileFlag()) {
                                case eMyMoney::Split::State::NotReconciled:
                                    sp.setReconcileFlag(eMyMoney::Split::State::Cleared);
                                    break;
                                case eMyMoney::Split::State::Cleared:
                                    sp.setReconcileFlag(eMyMoney::Split::State::NotReconciled);
                                    break;
                                default:
                                    break;
                            }
                        }
                    } else {
                        sp.setReconcileFlag(flag);
                    }

                    t.modifySplit(sp);
                    t.setImported(false);
                    MyMoneyFile::instance()->modifyTransaction(t);
                }
            }
        }
        ft.commit();
    } catch (const MyMoneyException &e) {
        KMessageBox::detailedError(this, i18n("Unable to modify transaction"), e.what());
    }
}

void KMyMoneyApp::slotMoveTransactionTo()
{
    auto action = qobject_cast<QAction*>(sender());
    // const auto actionId = d->qActionToId(action);
    const auto file = MyMoneyFile::instance();
    const auto accountId = action->data().toString();
    auto journalEntryList = d->m_selections.selection(SelectedObjects::JournalEntry);

    if (!journalEntryList.isEmpty()) {
        MyMoneyFileTransaction ft;
        try {
            // sort the journalentrylist so that the splits of one transaction
            // are located next to each other
            std::sort(journalEntryList.begin(), journalEntryList.end());
            MyMoneyTransaction t;
            for (const auto& journalId : journalEntryList) {
                const auto journalIdx = file->journalModel()->indexById(journalId);
                // load a new transaction, otherwise reuse it
                const auto tid = journalIdx.data(eMyMoney::Model::JournalTransactionIdRole).toString();
                if (t.id() != tid) {
                    t = file->transaction(tid);
                }
                auto s = t.splitById(journalIdx.data(eMyMoney::Model::JournalSplitIdRole).toString());
                const auto acc = file->accountsModel()->itemById(s.accountId());
                if (acc.isInvest()) {
                    /// moving an investment transactions must make sure that the
                    //  necessary (security) accounts exist before the transaction is moved
                    auto toInvAcc = file->account(accountId);
                    // first determine which stock we are dealing with.
                    // fortunately, investment transactions have only one stock involved
                    QString stockAccountId;
                    QString stockSecurityId;
                    for (const auto& split : t.splits()) {
                        stockAccountId = split.accountId();
                        stockSecurityId = file->account(stockAccountId).currencyId();
                        if (!file->security(stockSecurityId).isCurrency()) {
                            s = split;
                            break;
                        }
                    }
                    // Now check the target investment account to see if it
                    // contains a stock with this id
                    QString newStockAccountId;
                    for (const auto& sAccountId : toInvAcc.accountList()) {
                        if (file->account(sAccountId).currencyId() == stockSecurityId) {
                            newStockAccountId = sAccountId;
                            break;
                        }
                    }
                    // if it doesn't exist, we need to add it as a copy of the old one
                    // no 'copyAccount()' function??
                    if (newStockAccountId.isEmpty()) {
                        newStockAccountId = d->createNewStockAccount(toInvAcc, stockAccountId);
                    }

                    // now update the split and the transaction
                    s.setAccountId(newStockAccountId);

                } else {
                    s.setAccountId(accountId);
                }
                t.modifySplit(s);
                file->modifyTransaction(t);
            }
            ft.commit();
        } catch (const MyMoneyException& e) {
            qDebug() << e.what();
        }
    }
}

void KMyMoneyApp::slotTransactionOpenURL()
{
    auto action = qobject_cast<QAction*>(sender());
    // const auto actionId = d->qActionToId(action);
    const auto file = MyMoneyFile::instance();
    const auto accountId = action->data().toString();
    const auto journalEntryList = d->m_selections.selection(SelectedObjects::JournalEntry);

    if (journalEntryList.isEmpty())
        return;
    QList<QUrl> seenUrls;
    const auto journalModel = file->journalModel();
    for (const auto& journalId : journalEntryList) {
        const auto journalIdx = journalModel->indexById(journalId);
        const auto transactionId = journalIdx.data(eMyMoney::Model::JournalTransactionIdRole).toString();
        const auto indexes = journalModel->indexesByTransactionId(transactionId);
        const auto count = indexes.count();
        for (auto i = 0; i < count; ++i) {
            const auto payeeId = indexes[i].data(eMyMoney::Model::SplitPayeeIdRole).toString();
            const auto payee = MyMoneyFile::instance()->payee(payeeId);
            const auto memo = indexes[i].data(eMyMoney::Model::SplitMemoRole).toString();
            QUrl url = payee.payeeLink(memo);
            if (seenUrls.contains(url))
                continue;
            if (openUrl(url))
                seenUrls.append(url);
        }
    }
}

bool KMyMoneyApp::openUrl(const QUrl& url)
{
    if (url.scheme() != QStringLiteral("file") || !url.path().contains(QStringLiteral(".*"))) {
        QDesktopServices::openUrl(url);
        return true;
    }

    // Use the regex pattern of the file name from the URL to find the desired file.
    static QRegularExpression rx;
    QFileInfo fi(url.toLocalFile());
    rx.setPattern(fi.fileName());
    QDir dir = fi.absoluteDir();
    dir.setFilter(QDir::Files);
    QFileInfoList list = dir.entryInfoList();
    for (int j = 0; j < list.size(); ++j) {
        QFileInfo fileInfo = list.at(j);
        QRegularExpressionMatch match = rx.match(fileInfo.fileName());
        if (match.hasMatch()) {
            QDesktopServices::openUrl(QUrl::fromLocalFile(fileInfo.absoluteFilePath()));
            return true;
        }
    }
    return false;
}

void KMyMoneyApp::slotMoveToToday()
{
    const auto file = MyMoneyFile::instance();

    // since we may jump here via code, we have to make sure to react only
    // if the action is enabled
    if (!pActions[Action::MoveToToday]->isEnabled())
        return;

    if (d->m_selections.selection(SelectedObjects::JournalEntry).isEmpty())
        return;

    if (MyMoneyUtils::transactionWarnLevel(d->m_selections.selection(SelectedObjects::JournalEntry)) == OneSplitReconciled) {
        if (KMessageBox::warningContinueCancel(this,
                                               i18n("At least one split of the selected transactions has been reconciled. "
                                                    "Do you wish to change the transactions anyway?"),
                                               i18nc("@title:window Warning dialog", "Transaction already reconciled"))
            == KMessageBox::Cancel)
            return;

    } else {
        auto msg = i18np("Do you really want to change the selected transaction?",
                         "Do you really want to change all %1 selected transactions?",
                         d->m_selections.selection(SelectedObjects::JournalEntry).count());

        if (KMessageBox::questionTwoActions(this, msg, i18nc("@title:window Confirmation dialog", "Change transaction"), KMMYesNo::yes(), KMMYesNo::no())
            == KMessageBox::SecondaryAction) {
            return;
        }
    }

    MyMoneyFileTransaction ft;
    const QDate today = QDate::currentDate();
    for (const auto& journalEntryId : d->m_selections.selection(SelectedObjects::JournalEntry)) {
        const auto journalEntry = file->journalModel()->itemById(journalEntryId);
        if (!journalEntry.id().isEmpty()) {
            if (!journalEntry.transaction().id().isEmpty()) {
                if (!file->referencesClosedAccount(journalEntry.transaction())) {
                    MyMoneyTransaction tr = file->transaction(journalEntry.transaction().id());
                    tr.setPostDate(today);
                    file->modifyTransaction(tr);
                }
            }
        }
    }
    ft.commit();
}

void KMyMoneyApp::slotMatchTransaction()
{
    auto action = qobject_cast<KDualAction*>(sender());
    if (action) {
        action->isActive() ? d->matchTransaction() : d->unmatchTransaction();
        d->updateActions(d->m_selections);
    }

}

void KMyMoneyApp::slotCreateScheduledTransaction()
{
    if (d->m_selections.selection(SelectedObjects::JournalEntry).count() == 1) {
        const auto journalEntryId = d->m_selections.firstSelection(SelectedObjects::JournalEntry);
        const auto journalEntry = MyMoneyFile::instance()->journalModel()->itemById(journalEntryId);
        // make sure to have the current selected split as first split in the schedule
        MyMoneyTransaction t = journalEntry.transaction();
        MyMoneySplit s = journalEntry.split();
        const auto splitId = s.id();
        s.clearId();
        s.setReconcileFlag(eMyMoney::Split::State::NotReconciled);
        s.setReconcileDate(QDate());
        t.removeSplits();
        t.setImported(false);
        t.addSplit(s);
        for (const auto& split : journalEntry.transaction().splits()) {
            if (split.id() != splitId) {
                auto s0 = split;
                s0.clearId();
                s0.setReconcileFlag(eMyMoney::Split::State::NotReconciled);
                s0.setReconcileDate(QDate());
                t.addSplit(s0);
            }
        }
        KEditScheduleDlg::newSchedule(t, eMyMoney::Schedule::Occurrence::Monthly);
    }
}

void KMyMoneyApp::slotAcceptTransaction()
{
    const auto file = MyMoneyFile::instance();
    MyMoneyFileTransaction ft;
    try {
        for (const auto& journalEntryId : d->m_selections.selection(SelectedObjects::JournalEntry)) {
            const auto journalEntry = file->journalModel()->itemById(journalEntryId);
            auto t = journalEntry.transaction();
            auto s = journalEntry.split();
            if (t.isImported()) {
                t.setImported(false);
                if (s.reconcileFlag() < eMyMoney::Split::State::Reconciled) {
                    s.setReconcileFlag(eMyMoney::Split::State::Cleared);
                }
                t.modifySplit(s);
                file->modifyTransaction(t);
            }
            TransactionMatcher matcher;
            matcher.accept(t, s);
        }
        ft.commit();
        d->updateActions(d->m_selections);

    } catch (const MyMoneyException &e) {
        KMessageBox::detailedError(this, i18n("Unable to accept transaction"), QString::fromLatin1(e.what()));
    }
}

void KMyMoneyApp::slotStartReconciliation()
{
    slotEnterOverdueSchedules();

    d->executeAction(eMenu::Action::StartReconciliation);
}

void KMyMoneyApp::slotReportReconciliation()
{
    auto action = qobject_cast<QAction*>(sender());
    const auto report = action->data().value<MyMoneyReconciliationReport>();

    const auto account = MyMoneyFile::instance()->accountsModel()->itemById(report.accountId);
    if (!account.id().isEmpty()) {
        KMyMoneyPlugin::pluginInterfaces().viewInterface->accountReconciled(account, report.statementDate, report.startingBalance, report.endingBalance, report.journalEntryIds);
    }
}

void KMyMoneyApp::slotEnterOverdueSchedules()
{
    const auto accountId = d->m_selections.firstSelection(SelectedObjects::Account);
    if (accountId.isEmpty())
        return;

    const auto file = MyMoneyFile::instance();
    const auto accountIdx = file->accountsModel()->indexById(accountId);
    if (!accountIdx.isValid()) {
        qDebug() << "Invalid account id in slotEnterOverdueSchedules";
        return;
    }

    auto schedules = file->scheduleList(accountId, eMyMoney::Schedule::Type::Any, eMyMoney::Schedule::Occurrence::Any, eMyMoney::Schedule::PaymentType::Any, QDate(), QDate(), true);
    if (!schedules.isEmpty()) {
        const auto accountName = accountIdx.data(eMyMoney::Model::AccountNameRole).toString();
        if (KMessageBox::questionTwoActions(
                this,
                i18n("KMyMoney has detected some overdue scheduled transactions for the account <b>%1</b>. Do you want to enter those "
                     "scheduled transactions now?",
                     accountName),
                i18n("Scheduled transactions found"),
                KMMYesNo::yes(),
                KMMYesNo::no())
            == KMessageBox::PrimaryAction) {
            KMMStringSet skipMap;
            bool processedOne(false);
            auto rc = eDialogs::ScheduleResultCode::Enter;

            do {
                processedOne = false;
                QList<MyMoneySchedule>::const_iterator it_sch;
                for (it_sch = schedules.cbegin(); (rc != eDialogs::ScheduleResultCode::Cancel) && (it_sch != schedules.cend()); ++it_sch) {
                    MyMoneySchedule sch(*(it_sch));

                    // and enter it if it is not on the skip list
                    if (!skipMap.contains((*it_sch).id())) {
                        rc = d->m_myMoneyView->enterSchedule(sch, false, true);
                        if (rc == eDialogs::ScheduleResultCode::Ignore) {
                            skipMap.insert((*it_sch).id());
                        } else {
                            processedOne = true;
                        }
                    }
                }

                // reload list (maybe this schedule needs to be added again)
                schedules = file->scheduleList(accountId, eMyMoney::Schedule::Type::Any, eMyMoney::Schedule::Occurrence::Any, eMyMoney::Schedule::PaymentType::Any, QDate(), QDate(), true);
            } while (processedOne);
        }
    }
}

void KMyMoneyApp::slotFindTransaction()
{
    if (!d->m_searchDlg) {
        d->m_searchDlg = new KSearchTransactionDlg(this);
        connect(d->m_searchDlg, &QObject::destroyed, this, [&]() {
            d->m_searchDlg = nullptr;
        });
        connect(d->m_searchDlg, &KSearchTransactionDlg::requestSelectionChange, this, &KMyMoneyApp::slotSelectionChanged);

#if 0
        connect(d->m_searchDlg, &KSearchTransactionDlg::selectTransaction,
                this, [&]() {
                    qDebug() << "Jumping to the found transaction needs to be implemented";
                });
#endif
    }
    d->m_searchDlg->show();
    d->m_searchDlg->raise();
    d->m_searchDlg->activateWindow();
}

void KMyMoneyApp::slotCloseAccount()
{
    MyMoneyFileTransaction ft;
    MyMoneyFile* file = MyMoneyFile::instance();

    try {
        const auto accountId = d->m_selections.firstSelection(SelectedObjects::Account);
        auto account = file->account(accountId);
        // in case of investment, try to close the sub-accounts first
        if (account.accountType() == eMyMoney::Account::Type::Investment) {
            d->closeSubAccounts(account);
        }
        account.setClosed(true);
        file->modifyAccount(account);
        ft.commit();

        // inform views about the closing of the account
        d->executeAction(eMenu::Action::CloseAccount);

        if (!KMyMoneySettings::showAllAccounts()) {
            KMessageBox::information(
                this,
                i18n("<qt>You have closed this account. It remains in the system because you have transactions which still refer to it, but it is not shown in "
                     "the views. You can make it visible again by going to the View menu and selecting <b>Show all accounts</b>.</qt>"),
                i18n("Information"),
                "CloseAccountInfo");
        }
        slotSelectionChanged(d->m_selections);

    } catch (const MyMoneyException& e) {
        qDebug() << e.what();
    }
}

void KMyMoneyApp::slotReopenAccount()
{
    const auto file = MyMoneyFile::instance();
    MyMoneyFileTransaction ft;
    try {
        const auto accountId = d->m_selections.firstSelection(SelectedObjects::Account);
        auto account = file->account(accountId);
        while (account.isClosed()) {
            account.setClosed(false);
            file->modifyAccount(account);
            account = file->account(account.parentAccountId());
        }
        ft.commit();
        slotSelectionChanged(d->m_selections);

    } catch (const MyMoneyException& e) {
        qDebug() << e.what();
    }
}

#if 0
void KMyMoneyApp::slotOpenAccount()
{
    QString accountId, transactionId;
    if (!d->m_selections.isEmpty(SelectedObjects::Account)) {
        accountId = d->m_selections.selection(SelectedObjects::Account).at(0);
        if (!d->m_selections.isEmpty(SelectedObjects::Transaction)) {
            transactionId = d->m_selections.selection(SelectedObjects::Transaction).at(0);
        }
        d->m_myMoneyView->executeAction(eMenu::Action::OpenAccount, QVariantList{accountId, transactionId});
    }
}

void KMyMoneyApp::slotEditTransaction()
{
    if (!d->m_selections.isEmpty(SelectedObjects::Account)) {
        const auto accountId = d->m_selections.selection(SelectedObjects::Account).at(0);
        if (!d->m_selections.isEmpty(SelectedObjects::Transaction)) {
            d->m_myMoneyView->executeAction(eMenu::Action::EditTransaction, QVariantList{accountId, d->m_selections.selection(SelectedObjects::Transaction).first()});
        }
    }
}
#endif

void KMyMoneyApp::slotExecuteActionWithData()
{
    static const QHash<eMenu::Action, SelectedObjects::Object_t> actionToType = {
        {Action::OpenAccount, SelectedObjects::Account},
        {Action::GoToAccount, SelectedObjects::Account},
        {Action::GoToPayee, SelectedObjects::Payee},
        {Action::ReportOpen, SelectedObjects::Report},
    };
    auto action = qobject_cast<QAction*>(sender());
    const auto actionId = d->qActionToId(action);

    if (actionId != eMenu::Action::None) {
        if (actionToType.contains(actionId)) {
            auto selections = d->m_selections;
            selections.setSelection(actionToType[actionId], action->data().toString());
            d->m_myMoneyView->executeAction(actionId, selections);
            action->setData(QVariant());
        } else {
            qDebug() << "Action" << static_cast<int>(actionId) << "missing in slotExecuteActionWithData";
        }
    }
}

void KMyMoneyApp::slotExecuteAction()
{
    d->executeAction(d->qActionToId(qobject_cast<QAction*>(sender())));
}

void KMyMoneyApp::slotHideReconciledTransactions()
{
    KMyMoneySettings::setHideReconciledTransactions(pActions[Action::ViewHideReconciled]->isChecked());
    LedgerViewSettings::instance()->setHideReconciledTransactions(KMyMoneySettings::hideReconciledTransactions());
}

void KMyMoneyApp::slotHideUnusedCategories()
{
    KMyMoneySettings::setHideUnusedCategory(pActions[Action::ViewHideCategories]->isChecked());
    d->m_myMoneyView->slotSettingsChanged();
}

void KMyMoneyApp::slotShowAllAccounts()
{
    KMyMoneySettings::setShowAllAccounts(pActions[Action::ViewShowAll]->isChecked());
    d->m_myMoneyView->slotSettingsChanged();
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
    dumper.writeStream(st, MyMoneyFile::instance());
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

    if (editPersonalDataDlg->exec() == QDialog::Accepted && editPersonalDataDlg != nullptr) {
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
    if (dlg->exec() == QDialog::Accepted && dlg != nullptr) {
        MyMoneyFileTransaction ft;
        TemplateLoader loader(this);
        try {
            // import the account templates
            const auto templates = dlg->templates();
            for (const auto& tmpl : qAsConst(templates)) {
                loader.importTemplate(tmpl);
            }
            ft.commit();
        } catch (const MyMoneyException &e) {
            KMessageBox::detailedError(this, i18n("Unable to import template(s)"), QString::fromLatin1(e.what()));
        }
    }
    delete dlg;
}

void KMyMoneyApp::slotSaveAccountTemplates()
{
    KMSTATUS(i18n("Exporting account templates."));

    QString savePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/templates/" + QLocale().name();
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

        QPointer <KTemplateExportDlg> dlg = new KTemplateExportDlg(this);
        if ((dlg->exec() == QDialog::Accepted) && dlg) {
            MyMoneyTemplate tmpl;
            tmpl.setTitle(dlg->title());
            tmpl.setShortDescription(dlg->shortDescription());
            tmpl.setLongDescription(dlg->longDescription());

            TemplateWriter templateWriter(this);
            if (!templateWriter.exportTemplate(tmpl, QUrl::fromLocalFile(newName))) {
                KMessageBox::error(this, templateWriter.errorMessage(), i18nc("@title:window Error display", "Template export"), QFlags<KMessageBox::Option>());
            }
        }
        delete dlg;
    }
}

bool KMyMoneyApp::okToWriteFile(const QUrl &url)
{
    Q_UNUSED(url)

    // check if the file exists and warn the user
    bool reallySaveFile = true;

    if (KMyMoneyUtils::fileExists(url)) {
        if (KMessageBox::warningTwoActions(
                this,
                QLatin1String("<qt>")
                    + i18n("The file <b>%1</b> already exists. Do you really want to overwrite it?", url.toDisplayString(QUrl::PreferLocalFile))
                    + QLatin1String("</qt>"),
                i18n("File already exists"),
                KMMYesNo::yes(),
                KMMYesNo::no())
            != KMessageBox::PrimaryAction)
            reallySaveFile = false;
    }
    return reallySaveFile;
}

void KMyMoneyApp::slotSettings()
{
    // if we already have an instance of the settings dialog, then
    // make sure all widgets show correct state and use it
    auto dlg = KConfigDialog::exists("KMyMoney-Settings");
    if (dlg) {
        const auto managers = dlg->findChildren<KConfigDialogManager*>();
        for (const auto manager : managers) {
            manager->updateWidgets();
        }
        dlg->show();
        return;
    }

    // otherwise, we have to create it
    dlg = new KSettingsKMyMoney(this, "KMyMoney-Settings", KMyMoneySettings::self());
    connect(dlg, &KSettingsKMyMoney::settingsChanged, this, &KMyMoneyApp::slotUpdateConfiguration);
    dlg->show();
}

void KMyMoneyApp::slotToggleMenuBar(bool showMenuBar)
{
    if (showMenuBar) {
        menuBar()->show();

    } else {
        const auto action = actionCollection()->action(KStandardAction::name(KStandardAction::ShowMenubar));
        const auto shortcut = action->shortcut().toString();
        KMessageBox::information(this,
                                 i18n("This will hide the menu bar completely. You can show it again by typing %1.", shortcut),
                                 i18n("Hide menu bar"),
                                 QLatin1String("HideMenuBarHint"));
        menuBar()->hide();
    }
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
        actionCollection()->action(QT6_SKIP_FROM_LATIN_STRING(KStandardAction::name(KStandardAction::SaveAs)))->setEnabled(d->canFileSaveAs());
        onlineJobAdministration::instance()->updateActions();
        onlineJobAdministration::instance()->setOnlinePlugins(pPlugins.extended);
        d->m_myMoneyView->setOnlinePlugins(&pPlugins.online);
        d->updateActions(d->m_selections);
    }

    MyMoneyUtils::clearFormatCaches();
    switch (KMyMoneySettings::initialDateFieldCursorPosition()) {
    case KMyMoneySettings::Day:
        KMyMoneyDateEdit().setInitialSection(QDateTimeEdit::DaySection);
        break;
    case KMyMoneySettings::Month:
        KMyMoneyDateEdit().setInitialSection(QDateTimeEdit::MonthSection);
        break;
    case KMyMoneySettings::Year:
        KMyMoneyDateEdit().setInitialSection(QDateTimeEdit::YearSection);
        break;
    }

    const auto ledgerViewSettings = LedgerViewSettings::instance();
    ledgerViewSettings->setHideReconciledTransactions(KMyMoneySettings::hideReconciledTransactions());
    ledgerViewSettings->setHideTransactionsBefore(KMyMoneySettings::startDate().date());

    MyMoneyTransactionFilter::setFiscalYearStart(KMyMoneySettings::firstFiscalMonth(), KMyMoneySettings::firstFiscalDay());
    MyMoneyFile::instance()->budgetsModel()->setFiscalYearStart(KMyMoneySettings::firstFiscalMonth(), KMyMoneySettings::firstFiscalDay());
    MyMoneyReport::setLineWidth(KMyMoneySettings::lineWidth());

    const auto showHeaders = KMyMoneySettings::showFancyMarker();
    QDate firstFiscalDate;
    if (KMyMoneySettings::showFiscalMarker())
        firstFiscalDate = KMyMoneySettings::firstFiscalDate();

    MyMoneyFile::instance()->specialDatesModel()->setOptions(showHeaders, firstFiscalDate);
    MyMoneyFile::instance()->schedulesJournalModel()->setPreviewPeriod(KMyMoneySettings::schedulePreview());
    MyMoneyFile::instance()->schedulesJournalModel()->setShowPlannedDate(KMyMoneySettings::showPlannedScheduleDates());

    ledgerViewSettings->setShowLedgerLens(KMyMoneySettings::ledgerLens());
    ledgerViewSettings->setShowTransactionDetails(KMyMoneySettings::showRegisterDetailed());
    ledgerViewSettings->setShowAllSplits(KMyMoneySettings::showAllSplits());
    ledgerViewSettings->setSortOrder(LedgerViewSettings::SortOrderStd, KMyMoneySettings::sortNormalView());
    ledgerViewSettings->setSortOrder(LedgerViewSettings::SortOrderInvest, KMyMoneySettings::sortNormalView());
    ledgerViewSettings->setSortOrder(LedgerViewSettings::SortOrderReconcileStd, KMyMoneySettings::sortReconcileView());
    ledgerViewSettings->setSortOrder(LedgerViewSettings::SortOrderReconcileInvest, KMyMoneySettings::sortReconcileView());
    ledgerViewSettings->setSortOrder(LedgerViewSettings::SortOrderSearch, KMyMoneySettings::sortSearchView());
    ledgerViewSettings->setShowReconciliationEntries(d->showReconciliationMarker());
    ledgerViewSettings->flushChanges();

    MyMoneyFile::instance()->journalModel()->resetRowHeightInformation();

    pActions[Action::ViewTransactionDetail]->setChecked(KMyMoneySettings::showRegisterDetailed());
    pActions[Action::ViewHideReconciled]->setChecked(KMyMoneySettings::hideReconciledTransactions());
    pActions[Action::ViewHideCategories]->setChecked(KMyMoneySettings::hideUnusedCategory());
    pActions[Action::ViewShowAll]->setChecked(KMyMoneySettings::showAllAccounts());

    // update the holiday region configuration
    setHolidayRegion(KMyMoneySettings::holidayRegion());

    d->m_myMoneyView->slotSettingsChanged();
    KMyMoneyPlugin::updateConfiguration(pPlugins);

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
        if (KMessageBox::questionTwoActions(this,
                                            i18n("The file must be saved first "
                                                 "before it can be backed up.  Do you want to continue?"),
                                            i18nc("@title:window", "Confirmation of backup"),
                                            KMMYesNo::yes(),
                                            KMMYesNo::no())
            == KMessageBox::SecondaryAction) {
            return;
        }

        slotFileSave();
    }



    if (d->m_storageInfo.url.isEmpty())
        return;

    if (!d->m_storageInfo.url.isLocalFile()) {
        KMessageBox::error(this,
                           i18n("The current implementation of the backup functionality only supports local files as source files. Your current source file is '%1'.", d->m_storageInfo.url.url()),

                           i18n("Local files only"));
        return;
    }

    QPointer<KBackupDlg> backupDlg = new KBackupDlg(this);
    int returncode = backupDlg->exec();

    if (returncode == QDialog::Accepted && backupDlg != nullptr) {
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

            // make sure to fix the LD_LIBRARY_PATH
            // to not include APPDIR subdirectories
            AlkEnvironment::removeAppImagePathFromLinkLoaderLibPath(&d->m_proc);

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

    // make sure to fix the LD_LIBRARY_PATH
    // to not include APPDIR subdirectories
    AlkEnvironment::removeAppImagePathFromLinkLoaderLibPath(&d->m_proc);

    d->m_proc.start();
}

bool KMyMoneyApp::slotBackupWriteFile()
{
    QFileInfo fi(d->m_storageInfo.url.fileName());
    QString today = QDate::currentDate().toString("-yyyy-MM-dd.") + fi.suffix();
    QString backupfile = d->m_mountpoint + '/' + d->m_storageInfo.url.fileName();
    KMyMoneyUtils::appendCorrectFileExt(backupfile, today);

#ifdef Q_OS_WIN
    // on windows, a leading slash is a problem if a drive letter follows
    // eg. "/Z:/path". In case we detect such a pattern, we simply remove
    // the leading slash
    const QRegularExpression re(QStringLiteral("/(?<path>\\w+:/.+)"),
                                QRegularExpression::CaseInsensitiveOption|QRegularExpression::UseUnicodePropertiesOption
                               );
    const auto match = re.match(backupfile);
    if (match.hasMatch() && !match.captured(QStringLiteral("path")).isEmpty()) {
        backupfile = match.captured(QStringLiteral("path"));
    }
#endif

    // check if file already exists and ask what to do
    QFileInfo fileInfo(backupfile);
    if (fileInfo.exists()) {
        int answer = KMessageBox::warningContinueCancel(this, i18n("Backup file for today exists on that device. Replace?"), i18n("Backup"), KGuiItem(i18n("&Replace")));
        if (answer == KMessageBox::Cancel) {
            return false;
        }
    } else {
        // if it does not exist, make sure the path exists
        const auto path = fileInfo.absolutePath();
        if (!QDir().mkpath(path)) {
            KMessageBox::error(this, i18nc("@info Error during backup", "Unable to create backup directory '%1'.", path));
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

    // make sure to fix the LD_LIBRARY_PATH
    // to not include APPDIR subdirectories
    AlkEnvironment::removeAppImagePathFromLinkLoaderLibPath(&d->m_proc);
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

    // make sure to fix the LD_LIBRARY_PATH
    // to not include APPDIR subdirectories
    AlkEnvironment::removeAppImagePathFromLinkLoaderLibPath(&d->m_proc);

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
    auto *job = new KIO::CommandLauncherJob(cmd, this);
    job->setUiDelegate(new KDialogJobUiDelegate(KJobUiDelegate::AutoHandlingEnabled, this));
    job->setWorkingDirectory(QString());
    job->start();
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

            int ans =
                KMessageBox::questionTwoActionsCancel(this, message, i18nc("@title:window", "Opening balance for liability"), KMMYesNo::yes(), KMMYesNo::no());
            if (ans == KMessageBox::PrimaryAction) {
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

void KMyMoneyApp::slotScheduleNew(const MyMoneyTransaction& _t, eMyMoney::Schedule::Occurrence occurrence)
{
    KEditScheduleDlg::newSchedule(_t, occurrence);
}

void KMyMoneyApp::slotPayeeNew(const QString& newnameBase, QString& id)
{
    bool ok;
    std::tie(ok, id) = KMyMoneyUtils::newPayee(newnameBase);
}

void KMyMoneyApp::slotEditTabOrder()
{
    d->executeAction(d->qActionToId(qobject_cast<QAction*>(sender())));
}

void KMyMoneyApp::slotNewFeature()
{
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
        KMessageBox::informationList(nullptr, msg, m_consistencyCheckResult, i18n("Consistency check result"));
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
    const QStringList CSSnames {QStringLiteral("kmymoney.css")};
    const QString cssDir("/html/");
    const QString embeddedCSSPath = ":" + cssDir;
    // make sure we have the local directory where the themed version is stored
    const QString themedCSSPath  = QStandardPaths::standardLocations(QStandardPaths::AppConfigLocation).first() + cssDir;
    QDir().mkpath(themedCSSPath);

    for (const auto& CSSname : CSSnames) {
        const QString defaultCSSFilename = embeddedCSSPath + CSSname;
        QFileInfo fileInfo(defaultCSSFilename);
        if (fileInfo.exists()) {
            const QString themedCSSFilename = themedCSSPath + CSSname;
            QFile::remove(themedCSSFilename);
            if (QFile::copy(defaultCSSFilename, themedCSSFilename)) {
                QFile::setPermissions(themedCSSFilename, QFileDevice::ReadOwner|QFileDevice::WriteOwner);
                QFile cssFile (themedCSSFilename);
                if (cssFile.open(QIODevice::ReadWrite)) {
                    QTextStream cssStream(&cssFile);
                    auto cssText = cssStream.readAll();
                    cssText.replace(QLatin1String("WindowText"),    KMyMoneySettings::schemeColor(SchemeColor::WindowText).name(),        Qt::CaseSensitive);
                    cssText.replace(QLatin1String("Window"),        KMyMoneySettings::schemeColor(SchemeColor::WindowBackground).name(),  Qt::CaseSensitive);
                    cssText.replace(QLatin1String("HighlightText"), KMyMoneySettings::schemeColor(SchemeColor::ListHighlightText).name(), Qt::CaseSensitive);
                    cssText.replace(QLatin1String("Highlight"),     KMyMoneySettings::schemeColor(SchemeColor::ListHighlight).name(),     Qt::CaseSensitive);
                    cssText.replace(QLatin1String("black"),         KMyMoneySettings::schemeColor(SchemeColor::ListGrid).name(),          Qt::CaseSensitive);
                    cssStream.seek(0);
                    cssStream << cssText;
                    cssFile.resize(cssFile.pos());
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
                    while (!schedule.isFinished() && (schedule.adjustedNextDueDate() <= checkDate) //
                            && rc != eDialogs::ScheduleResultCode::Ignore //
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

void KMyMoneyApp::slotEquityPriceUpdate()
{
    QPointer<KEquityPriceUpdateDlg> dlg = new KEquityPriceUpdateDlg(this);
    dlg->setSearchShortcut(pActions[eMenu::Action::ShowFilterWidget]->shortcut());

    if (dlg->exec() == QDialog::Accepted && dlg != nullptr)
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
        MyMoneyStatementReader::clearImportResults();
        while (!d->m_importUrlsQueue.isEmpty()) {
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
                // d->unlinkStatementXML();

                QMap<QString, KMyMoneyPlugin::ImporterPlugin*>::const_iterator it_plugin = pPlugins.importer.cbegin();
                while (it_plugin != pPlugins.importer.cend()) {
                    if ((*it_plugin)->isMyFormat(url)) {
                        if (!(*it_plugin)->import(url) && !(*it_plugin)->lastError().isEmpty()) {
                            QString pluginName;
                            if (pPlugins.standard.contains(it_plugin.key()))
                                pluginName = pPlugins.standard.value(it_plugin.key())->componentDisplayName();
                            KMessageBox::error(this, i18nc("%1 file location, %2 plugin name", "Unable to import %1 using %2 plugin. The plugin returned the following error: %3", url, pluginName, (*it_plugin)->lastError()), i18n("Importing error"));
                        }

                        break;
                    }
                    ++it_plugin;
                }

                // If we did not find a match, try importing it as a KMM statement file,
                // which is really just for testing.  the statement file is not exposed
                // to users.
                if (it_plugin == pPlugins.importer.cend()) {
                    if (MyMoneyStatement::isStatementFile(url)) {
                        MyMoneyStatementReader::importStatement(url);
                    }
                }
            }
            // remove the current processed item from the queue
            d->m_importUrlsQueue.dequeue();
        }

        QScopedPointer<ImportSummaryDialog> dlg(new ImportSummaryDialog(nullptr));
        dlg->setModel(MyMoneyStatementReader::importResultsModel());
        dlg->exec();
    }
}

void KMyMoneyApp::slotEnableMessages()
{
    KMessageBox::enableAllMessages();
    KMessageBox::information(this, i18n("All messages have been enabled."), i18n("All messages"));
}

void KMyMoneyApp::slotGetOnlineHelp()
{
    QDesktopServices::openUrl(QUrl("https://kmymoney.org/support.html"));
}

void KMyMoneyApp::slotWhatsNew()
{
    QDesktopServices::openUrl(QUrl("https://kmymoney.org/news/"));
}

void KMyMoneyApp::slotVisitWebsite()
{
    QDesktopServices::openUrl(QUrl("https://kmymoney.org"));
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

#if KHOLIDAYS_VERSION >= QT_VERSION_CHECK(5, 95, 0)
        KHolidays::Holiday::List holidayList = d->m_holidayRegion->rawHolidaysWithAstroSeasons(QDate::currentDate(), endDate);
#else
        KHolidays::Holiday::List holidayList = d->m_holidayRegion->holidays(QDate::currentDate(), endDate);
#endif
        KHolidays::Holiday::List::const_iterator holiday_it;
        for (holiday_it = holidayList.cbegin(); holiday_it != holidayList.cend(); ++holiday_it) {
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

bool KMyMoneyApp::event(QEvent * event) {
    if (event->type() == QEvent::PaletteChange) {
        this->initIcons();
        return true;
    }

    return KXmlGuiWindow::event(event);
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
        MyMoneyFileTransaction ft;
        auto file = MyMoneyFile::instance();
        file->unload();

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
        const auto templates = wizard.templates();
        TemplateLoader loader(this);
        for (const auto& tmpl : templates) {
            loader.importTemplate(tmpl);
        }

        ft.commit();
        KMyMoneySettings::setFirstTimeRun(false);

        d->fileAction(eKMyMoney::FileAction::Opened);
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

    qDebug() << "Open file" << url;

    if (url.scheme() != QLatin1String("sql") && !KMyMoneyUtils::fileExists(url)) {
        KMessageBox::error(this, i18n("<p><b>%1</b> is either an invalid filename or the file does not exist. You can open another file or create a new one.</p>", url.toDisplayString(QUrl::PreferLocalFile)), i18n("File not found"));
        return false;
    }

    if (d->m_storageInfo.isOpened)
        if (!slotFileClose())
            return false;

    // open the database
    d->m_storageInfo.type = eKMyMoney::StorageType::None;
    // Inform views that a new file will be loaded
    // The information that the file is really open will
    // be sent out in KMyMoneyView::switchToDefaultView()
    d->executeCustomAction(eView::Action::BlockViewDuringFileOpen);
    for (auto &plugin : pPlugins.storage) {
        try {
            if (plugin->open(url)) {
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
            } else {
                const auto msg = plugin->openErrorMessage();
                if (!msg.isEmpty()) {
                    KMessageBox::error(nullptr, msg, i18nc("@title:window", "Problem opening storage"));
                    d->executeCustomAction(eView::Action::UnblockViewAfterFileOpen);
                    return false;
                }
            }
        } catch (const MyMoneyException &e) {
            KMessageBox::detailedError(this, i18n("Cannot open file as requested."), QString::fromLatin1(e.what()));
            d->executeCustomAction(eView::Action::UnblockViewAfterFileOpen);
            return false;
        }
    }

    if(d->m_storageInfo.type == eKMyMoney::StorageType::None) {
        KMessageBox::error(this, i18n("Could not read your data source. Please check the KMyMoney settings that the necessary plugin is enabled."));
        d->executeCustomAction(eView::Action::UnblockViewAfterFileOpen);
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
                KMessageBox::detailedError(this, i18n("Failed to save your data."), e.what());
                return false;
            }
        }
    }

    KMessageBox::error(this, i18n("Couldn't find suitable plugin to save your data."));
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
        KMessageBox::error(this, i18n("Couldn't find any plugin for saving data."));
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

bool KMyMoneyApp::slotCloseViewOrFile()
{
    if (!d->m_storageInfo.isOpened)
        return true;

    // check if we have a closable view/tab
    if (d->m_myMoneyView && d->m_myMoneyView->hasClosableView()) {
        d->m_myMoneyView->closeCurrentView();
        return true;
    }
    return slotFileClose();
}

bool KMyMoneyApp::slotFileClose()
{
    if (!d->m_storageInfo.isOpened)
        return true;

    if (!d->askAboutSaving())
        return false;

    // prevent starting action checks
    // when there is no file open
    d->m_actionCollectorTimer.stop();

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
        QList<KMainWindow*>::const_iterator w_it = memberList.cbegin();
        for (; w_it != memberList.cend(); ++w_it) {
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

QObject* KMyMoneyApp::createFactoryObject(QObject* parent, const QString& objectName)
{
    return createObject(parent, objectName);
}

KMStatus::KMStatus(const QString &text)
    : m_prevText(kmymoney->slotStatusMsg(text))
{
}

KMStatus::~KMStatus()
{
    kmymoney->slotStatusMsg(m_prevText);
}

#include "kmymoney.moc"
