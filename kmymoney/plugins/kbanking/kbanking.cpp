/*
    SPDX-FileCopyrightText: 2004 Martin Preuss aquamaniac @users.sourceforge.net
    SPDX-FileCopyrightText: 2009 Cristian Onet onet.cristian @gmail.com
    SPDX-FileCopyrightText: 2010-2019 Thomas Baumgart tbaumgart @kde.org
    SPDX-FileCopyrightText: 2015 Christian David christian-david @web.de
    SPDX-FileCopyrightText: 2021 Dawid Wróbel <me@dawidwrobel.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <config-kmymoney.h>

#include "kbanking.h"

#include <memory>

// ----------------------------------------------------------------------------
// QT Includes

#include <QCheckBox>
#include <QDebug>
#include <QLabel>
#include <QLayout>
#include <QRadioButton>
#include <QRegExp>
#include <QRegularExpression>
#include <QStringList>
#include <QTimer>
#include <QUuid>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KPluginFactory>
#include <KLocalizedString>
#include <KMessageBox>
#include <KActionCollection>
#include <QMenu>
#include <KGuiItem>
#include <KLineEdit>
#include <KComboBox>
#include <KConfig>
#include <KConfigGroup>
#include <KAboutData>

// ----------------------------------------------------------------------------
// Library Includes

#include <aqbanking/banking.h>
#include <aqbanking/types/imexporter_context.h>
#include <aqbanking/types/transaction.h>
#include <aqbanking/types/transactionlimits.h>
#include <aqbanking/gui/abgui.h>
#include <aqbanking/version.h>
#include <gwenhywfar/logger.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/version.h>
#include <gwenhywfar/gwenhywfar.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoney/onlinejob.h"

#include "kbaccountsettings.h"
#include "kbmapaccount.h"
#include "mymoneyfile.h"
#include "onlinejobadministration.h"
#include "kmymoneyview.h"
#include "kbpickstartdate.h"
#include "mymoneyinstitution.h"
#include "mymoneytransactionfilter.h"
#include "mymoneyexception.h"
#include "mymoneysecurity.h"

#include "gwenkdegui.h"
#include "gwenhywfarqtoperators.h"
#include "aqbankingkmmoperators.h"
#include "mymoneystatement.h"
#include "statementinterface.h"
#include "viewinterface.h"

#ifdef KMM_DEBUG
#include "chiptandialog.h"
#include "phototandialog.h"

#include "phototan-demo.cpp"
#endif

class KBanking::Private
{
public:
    Private()
        : passwordCacheTimer(nullptr)
        , gui(nullptr)
        , jobList()
        , fileId()
    {
        QString gwenProxy = QString::fromLocal8Bit(qgetenv("GWEN_PROXY"));
        if (gwenProxy.isEmpty()) {
            std::unique_ptr<KConfig> cfg = std::unique_ptr<KConfig>(new KConfig("kioslaverc"));
            QRegExp exp("(\\w+://)?([^/]{2}.+:\\d+)");
            QString proxy;

            KConfigGroup grp = cfg->group("Proxy Settings");
            int type = grp.readEntry("ProxyType", 0);
            switch (type) {
            case 0: // no proxy
                break;

            case 1: // manual specified
                proxy = grp.readEntry("httpsProxy");
                qDebug("KDE https proxy setting is '%s'", qPrintable(proxy));
                if (exp.exactMatch(proxy)) {
                    proxy = exp.cap(2);
                    qDebug("Setting GWEN_PROXY to '%s'", qPrintable(proxy));
                    if (!qputenv("GWEN_PROXY", qPrintable(proxy))) {
                        qDebug("Unable to setup GWEN_PROXY");
                    }
                }
                break;

            default: // other currently not supported
                qDebug("KDE proxy setting of type %d not supported", type);
                break;
            }
        }
    }

    QString libVersion(void (*version)(int*, int*, int*, int*))
    {
        int major, minor, patch, build;
        version(&major, &minor, &patch, &build);
        return QString("%1.%2.%3.%4").arg(major).arg(minor).arg(patch).arg(build);
    }
    /**
     * KMyMoney asks for accounts over and over again which causes a lot of "Job not supported with this account" error messages.
     * This function filters messages with that string.
     */
    static int gwenLogHook(GWEN_GUI* gui, const char* domain, GWEN_LOGGER_LEVEL level, const char* message) {
        Q_UNUSED(gui);

        const char* messageToFilter = "Job not supported with this account";
        if (!strstr(message, messageToFilter)) {
            // emulate AqBanking log format here as much as possible
            const auto time = QDateTime::currentDateTime();
            qDebug("%d:%s:%s %s", level, qPrintable(time.toString(Qt::ISODate).replace('T', ' ')), domain, message);
        }
        return 1;
    }

    QTimer *passwordCacheTimer;
    gwenKdeGui* gui;
    QMap<QString, QStringList> jobList;
    QString fileId;
    QSet<QAction*> actions;
};


#if KCOREADDONS_VERSION < QT_VERSION_CHECK(5, 77, 0)
KBanking::KBanking(QObject *parent, const QVariantList &args) :
    OnlinePluginExtended(parent, args)
#else
KBanking::KBanking(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args) :
    OnlinePluginExtended(parent, metaData, args)
#endif
    , d(new Private)
    , m_configAction(nullptr)
    , m_importAction(nullptr)
    , m_kbanking(nullptr)
    , m_accountSettings(nullptr)
    , m_statementCount(0)
{
    Q_UNUSED(args)
    QString compileVersionSet = QLatin1String(GWENHYWFAR_VERSION_FULL_STRING "/" AQBANKING_VERSION_FULL_STRING);
    QString runtimeVersionSet = QString("%1/%2").arg(d->libVersion(&GWEN_Version), d->libVersion(&AB_Banking_GetVersion));
    qDebug() << QString("Plugins: kbanking loaded, build with (%1), run with (%2)").arg(compileVersionSet, runtimeVersionSet);
}

KBanking::~KBanking()
{
    delete d;
    qDebug("Plugins: kbanking unloaded");
}

void KBanking::plug(KXMLGUIFactory* guiFactory)
{
    Q_UNUSED(guiFactory)
    m_kbanking = new KBankingExt(this, "KMyMoney");

    d->passwordCacheTimer = new QTimer(this);
    d->passwordCacheTimer->setSingleShot(true);
    d->passwordCacheTimer->setInterval(60000);
    connect(d->passwordCacheTimer, &QTimer::timeout, this, &KBanking::slotClearPasswordCache);

    if (m_kbanking) {
        d->gui = new gwenKdeGui;
        GWEN_Gui_SetGui(d->gui->getCInterface());
        GWEN_Gui_SetLogHookFn(d->gui->getCInterface(), &KBanking::Private::gwenLogHook);
        if (qEnvironmentVariableIsEmpty("GWEN_LOGLEVEL")) {
            GWEN_Logger_SetLevel(GWEN_LOGDOMAIN, GWEN_LoggerLevel_Warning);
        }

        if (m_kbanking->init() == 0) {
            // Tell the host application to load my GUI component
            const auto rcFileName = QLatin1String("kbanking.rc");
            setXMLFile(rcFileName);

            // get certificate handling and dialog settings management
            AB_Gui_Extend(d->gui->getCInterface(), m_kbanking->getCInterface());

            // create actions
            createActions();

            // load protocol conversion list
            loadProtocolConversion();
            if (qEnvironmentVariableIsEmpty("AQBANKING_LOGLEVEL"))
                GWEN_Logger_SetLevel(AQBANKING_LOGDOMAIN, GWEN_LoggerLevel_Warning);

        } else {
            qWarning("Could not initialize KBanking online banking interface");
            delete m_kbanking;
            m_kbanking = 0;
        }
    }
}

void KBanking::unplug()
{
    d->passwordCacheTimer->deleteLater();
    if (m_kbanking) {
        m_kbanking->fini();
        delete m_kbanking;
    }
    delete d->gui;
    d->gui = nullptr;

    // remove and delete the actions for this plugin
    for (const auto& action : qAsConst(d->actions)) {
        actionCollection()->removeAction(action);
    }
    qDebug("Plugins: kbanking unplugged");
}


void KBanking::loadProtocolConversion()
{
    if (m_kbanking) {
        m_protocolConversionMap = {
            {"aqhbci", "HBCI"},
            {"aqofxconnect", "OFX"},
            {"aqyellownet", "YellowNet"},
            {"aqgeldkarte", "Geldkarte"},
            {"aqdtaus", "DTAUS"},
        };
    }
}


void KBanking::protocols(QStringList& protocolList) const
{
    if (m_kbanking) {
        std::list<std::string> list = m_kbanking->getActiveProviders();
        std::list<std::string>::iterator it;
        for (it = list.begin(); it != list.end(); ++it) {
            // skip the dummy
            if (*it == "aqnone")
                continue;
            QMap<QString, QString>::const_iterator it_m;
            it_m = m_protocolConversionMap.find((*it).c_str());
            if (it_m != m_protocolConversionMap.end())
                protocolList << (*it_m);
            else
                protocolList << (*it).c_str();
        }
    }
}


QWidget* KBanking::accountConfigTab(const MyMoneyAccount& acc, QString& name)
{
    const MyMoneyKeyValueContainer& kvp = acc.onlineBankingSettings();
    name = i18n("Online settings");
    if (m_kbanking) {
        m_accountSettings = new KBAccountSettings(acc, 0);
        m_accountSettings->loadUi(kvp);
        return m_accountSettings;
    }
    QLabel* label = new QLabel(i18n("KBanking module not correctly initialized"), 0);
    label->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
    return label;
}


MyMoneyKeyValueContainer KBanking::onlineBankingSettings(const MyMoneyKeyValueContainer& current)
{
    MyMoneyKeyValueContainer kvp(current);
    kvp["provider"] = objectName().toLower();
    if (m_accountSettings) {
        m_accountSettings->loadKvp(kvp);
    }
    return kvp;
}


void KBanking::createActions()
{
    QAction *settings_aqbanking = actionCollection()->addAction("settings_aqbanking");
    settings_aqbanking->setText(i18n("Configure Aq&Banking..."));
    connect(settings_aqbanking, &QAction::triggered, this, &KBanking::slotSettings);
    d->actions.insert(settings_aqbanking);

    QAction *file_import_aqbanking = actionCollection()->addAction("file_import_aqbanking");
    file_import_aqbanking->setText(i18n("AqBanking importer..."));
    connect(file_import_aqbanking, &QAction::triggered, this, &KBanking::slotImport);
    d->actions.insert(file_import_aqbanking);

    Q_CHECK_PTR(viewInterface());
    connect(viewInterface(), &KMyMoneyPlugin::ViewInterface::viewStateChanged, action("file_import_aqbanking"), &QAction::setEnabled);

#ifdef KMM_DEBUG
    QAction *openChipTanDialog = actionCollection()->addAction("open_chiptan_dialog");
    openChipTanDialog->setText("Open ChipTan Dialog");
    connect(openChipTanDialog, &QAction::triggered, this, [&]() {
        auto dlg = new chipTanDialog();
        dlg->setHhdCode("0F04871100030333555414312C32331D");
        dlg->setInfoText("<html><h1>Test Graphic for debugging</h1><p>The encoded data is</p><p>Account Number: <b>335554</b><br/>Amount: <b>1,23</b></p></html>");
        connect(dlg, &QDialog::accepted, dlg, &chipTanDialog::deleteLater);
        connect(dlg, &QDialog::rejected, dlg, &chipTanDialog::deleteLater);
        dlg->show();
    });
    d->actions.insert(openChipTanDialog);

    QAction *openPhotoTanDialog = actionCollection()->addAction("open_phototan_dialog");
    openPhotoTanDialog->setText("Open PhotoTan Dialog");
    connect(openPhotoTanDialog, &QAction::triggered, this, [&]() {
        auto dlg = new photoTanDialog();
        QImage img;
        img.loadFromData(photoTan, sizeof(photoTan), "PNG");
        img = img.scaled(300, 300, Qt::KeepAspectRatio);
        dlg->setPicture(QPixmap::fromImage(img));
        dlg->setInfoText("<html><h1>Test Graphic for debugging</h1><p>The encoded data is</p><p>unknown</p></html>");
        connect(dlg, &QDialog::accepted, dlg, &chipTanDialog::deleteLater);
        connect(dlg, &QDialog::rejected, dlg, &chipTanDialog::deleteLater);
        dlg->show();
    });
    d->actions.insert(openPhotoTanDialog);
#endif
}

void KBanking::slotSettings()
{
    if (m_kbanking) {
        GWEN_DIALOG* dlg = AB_Banking_CreateSetupDialog(m_kbanking->getCInterface());
        if (dlg == NULL) {
            DBG_ERROR(0, "Could not create setup dialog.");
            return;
        }

        if (GWEN_Gui_ExecDialog(dlg, 0) == 0) {
            DBG_ERROR(0, "Aborted by user");
            GWEN_Dialog_free(dlg);
            return;
        }
        GWEN_Dialog_free(dlg);
    }
}


bool KBanking::mapAccount(const MyMoneyAccount& acc, MyMoneyKeyValueContainer& settings)
{
    bool rc = false;
    if (m_kbanking && !acc.id().isEmpty()) {
        m_kbanking->askMapAccount(acc);

        // at this point, the account should be mapped
        // so we search it and setup the account reference in the KMyMoney object
        AB_ACCOUNT_SPEC* ab_acc;
        ab_acc = aqbAccount(acc);
        if (ab_acc) {
            MyMoneyAccount a(acc);
            setupAccountReference(a, ab_acc);
            settings = a.onlineBankingSettings();
            rc = true;
        }
    }
    return rc;
}


AB_ACCOUNT_SPEC* KBanking::aqbAccount(const MyMoneyAccount& acc) const
{
    if (m_kbanking == 0) {
        return 0;
    }

    // certainly looking for an expense or income account does not make sense at this point
    // so we better get out right away
    if (acc.isIncomeExpense()) {
        return 0;
    }

    AB_ACCOUNT_SPEC *ab_acc = AB_Banking_GetAccountSpecByAlias(m_kbanking->getCInterface(), m_kbanking->mappingId(acc).toUtf8().data());
    // if the account is not found, we temporarily scan for the 'old' mapping (the one w/o the file id)
    // in case we find it, we setup the new mapping in addition on the fly.
    if (!ab_acc && acc.isAssetLiability()) {
        ab_acc = AB_Banking_GetAccountSpecByAlias(m_kbanking->getCInterface(), acc.id().toUtf8().data());
        if (ab_acc) {
            qDebug("Found old mapping for '%s' but not new. Setup new mapping", qPrintable(acc.name()));
            m_kbanking->setAccountAlias(ab_acc, m_kbanking->mappingId(acc).toUtf8().constData());
            // TODO at some point in time, we should remove the old mapping
        }
    }
    return ab_acc;
}


AB_ACCOUNT_SPEC* KBanking::aqbAccount(const QString& accountId) const
{
    MyMoneyAccount account = MyMoneyFile::instance()->account(accountId);
    return aqbAccount(account);
}


QString KBanking::stripLeadingZeroes(const QString& s) const
{
    QString rc(s);
    QRegExp exp("^(0*)([^0].*)");
    if (exp.exactMatch(s)) {
        rc = exp.cap(2);
    }
    return rc;
}

void KBanking::setupAccountReference(const MyMoneyAccount& acc, AB_ACCOUNT_SPEC* ab_acc)
{
    MyMoneyKeyValueContainer kvp;

    if (ab_acc) {
        QString accountNumber = stripLeadingZeroes(AB_AccountSpec_GetAccountNumber(ab_acc));
        QString bankCode = stripLeadingZeroes(AB_AccountSpec_GetBankCode(ab_acc));

        QString val = QString("%1-%2-%3").arg(bankCode, accountNumber).arg(AB_AccountSpec_GetType(ab_acc));

        if (val != acc.onlineBankingSettings().value("kbanking-acc-ref")) {
            kvp.clear();

            // make sure to keep our own previous settings
            const QMap<QString, QString>& vals = acc.onlineBankingSettings().pairs();
            QMap<QString, QString>::const_iterator it_p;
            for (it_p = vals.begin(); it_p != vals.end(); ++it_p) {
                if (QString(it_p.key()).startsWith("kbanking-")) {
                    kvp.setValue(it_p.key(), *it_p);
                }
            }

            kvp.setValue("kbanking-acc-ref", val);
            kvp.setValue("provider", objectName().toLower());
            setAccountOnlineParameters(acc, kvp);
        }
    } else {
        // clear the connection
        setAccountOnlineParameters(acc, kvp);
    }
}


bool KBanking::accountIsMapped(const MyMoneyAccount& acc)
{
    return aqbAccount(acc) != 0;
}


bool KBanking::updateAccount(const MyMoneyAccount& acc)
{
    return updateAccount(acc, false);
}


bool KBanking::updateAccount(const MyMoneyAccount& acc, bool moreAccounts)
{
    if (!m_kbanking)
        return false;

    bool rc = false;

    if (!acc.id().isEmpty()) {
        AB_TRANSACTION *job = 0;
        int rv;

        /* get AqBanking account */
        AB_ACCOUNT_SPEC *ba = aqbAccount(acc);
        // Update the connection between the KMyMoney account and the AqBanking equivalent.
        // If the account is not found anymore ba == 0 and the connection is removed.
        setupAccountReference(acc, ba);

        if (!ba) {
            KMessageBox::error(0,
                               i18n("<qt>"
                                    "The given application account <b>%1</b> "
                                    "has not been mapped to an online "
                                    "account."
                                    "</qt>",
                                    acc.name()),
                               i18n("Account Not Mapped"));
        } else {
            bool enqueJob = true;
            if (acc.onlineBankingSettings().value("kbanking-txn-download") != "no") {
                /* create getTransactions job */
                if (AB_AccountSpec_GetTransactionLimitsForCommand(ba, AB_Transaction_CommandGetTransactions)) {
                    /* there are transaction limits for this job, so it is allowed */
                    job = AB_Transaction_new();
                    AB_Transaction_SetUniqueAccountId(job, AB_AccountSpec_GetUniqueId(ba));
                    AB_Transaction_SetCommand(job, AB_Transaction_CommandGetTransactions);
                }

                if (job) {
                    int days = 0 /* TODO in AqBanking AB_JobGetTransactions_GetMaxStoreDays(job)*/;
                    QDate qd;
                    if (days > 0) {
                        GWEN_DATE *dt;

                        dt=GWEN_Date_CurrentDate();
                        GWEN_Date_SubDays(dt, days);
                        qd = QDate(GWEN_Date_GetYear(dt), GWEN_Date_GetMonth(dt), GWEN_Date_GetDay(dt));
                        GWEN_Date_free(dt);
                    }

                    // get last statement request date from application account object
                    // and start from a few days before if the date is valid
                    QDate lastUpdate = QDate::fromString(acc.value("lastImportedTransactionDate"), Qt::ISODate);
                    if (lastUpdate.isValid())
                        lastUpdate = lastUpdate.addDays(-3);

                    int dateOption = acc.onlineBankingSettings().value("kbanking-statementDate").toInt();
                    switch (dateOption) {
                    case 0: // Ask user
                        break;
                    case 1: // No date
                        qd = QDate();
                        break;
                    case 2: // Last download
                        qd = lastUpdate;
                        break;
                    case 3: // First possible
                        // qd is already setup
                        break;
                    }

                    // the pick start date option dialog is needed in
                    // case the dateOption is 0 or the date option is > 1
                    // and the qd is invalid
                    if (dateOption == 0 || (dateOption > 1 && !qd.isValid())) {
                        QPointer<KBPickStartDate> psd = new KBPickStartDate(m_kbanking, qd, lastUpdate, acc.name(),
                                lastUpdate.isValid() ? 2 : 3, 0, true);
                        if (psd->exec() == QDialog::Accepted) {
                            qd = psd->date();
                        } else {
                            enqueJob = false;
                        }
                        delete psd;
                    }

                    if (enqueJob) {
                        if (qd.isValid()) {
                            GWEN_DATE *dt;

                            dt=GWEN_Date_fromGregorian(qd.year(), qd.month(), qd.day());
                            AB_Transaction_SetFirstDate(job, dt);
                            GWEN_Date_free(dt);
                        }

                        rv = m_kbanking->enqueueJob(job);
                        if (rv) {
                            DBG_ERROR(0, "Error %d", rv);
                            KMessageBox::error(0,
                                               i18n("<qt>"
                                                    "Could not enqueue the job.\n"
                                                    "</qt>"),
                                               i18n("Error"));
                        }
                    }
                    AB_Transaction_free(job);
                }
            }

            if (enqueJob) {
                /* create getBalance job */
                if (AB_AccountSpec_GetTransactionLimitsForCommand(ba, AB_Transaction_CommandGetBalance)) {
                    /* there are transaction limits for this job, so it is allowed */
                    job = AB_Transaction_new();
                    AB_Transaction_SetUniqueAccountId(job, AB_AccountSpec_GetUniqueId(ba));
                    AB_Transaction_SetCommand(job, AB_Transaction_CommandGetBalance);
                    rv = m_kbanking->enqueueJob(job);
                    AB_Transaction_free(job);
                    if (rv) {
                        DBG_ERROR(0, "Error %d", rv);
                        KMessageBox::error(0,
                                           i18n("<qt>"
                                                "Could not enqueue the job.\n"
                                                "</qt>"),
                                           i18n("Error"));
                    } else {
                        rc = true;
                        emit queueChanged();
                    }
                }
            }
        }
    }

    // make sure we have at least one job in the queue before sending it
    if (!moreAccounts && m_kbanking->getEnqueuedJobs().size() > 0)
        executeQueue();

    return rc;
}


void KBanking::executeQueue()
{
    if (m_kbanking && m_kbanking->getEnqueuedJobs().size() > 0) {
        AB_IMEXPORTER_CONTEXT *ctx;
        ctx = AB_ImExporterContext_new();
        int rv = m_kbanking->executeQueue(ctx);
        if (!rv) {
            m_kbanking->importContext(ctx, 0);
        } else {
            DBG_ERROR(0, "Error: %d", rv);
        }
        AB_ImExporterContext_free(ctx);
    }
}


/** @todo improve error handling, e.g. by adding a .isValid to nationalTransfer
 * @todo use new onlineJob system
 */
void KBanking::sendOnlineJob(QList<onlineJob>& jobs)
{
    Q_CHECK_PTR(m_kbanking);

    m_onlineJobQueue.clear();
    QList<onlineJob> unhandledJobs;

    if (!jobs.isEmpty()) {
        foreach (onlineJob job, jobs) {
            if (sepaOnlineTransfer::name() == job.task()->taskName()) {
                onlineJobTyped<sepaOnlineTransfer> typedJob(job);
                enqueTransaction(typedJob);
                job = typedJob;
            } else {
                job.addJobMessage(onlineJobMessage(eMyMoney::OnlineJob::MessageType::Error, "KBanking", "Cannot handle this request"));
                unhandledJobs.append(job);
            }
            m_onlineJobQueue.insert(m_kbanking->mappingId(job), job);
        }

        executeQueue();
    }
    jobs = m_onlineJobQueue.values();
    jobs += unhandledJobs;
    m_onlineJobQueue.clear();
}


QStringList KBanking::availableJobs(QString accountId) const
{
    try {
        MyMoneyAccount acc = MyMoneyFile::instance()->account(accountId);
        QString id = MyMoneyFile::instance()->value("kmm-id");
        if(id != d->fileId) {
            d->jobList.clear();
            d->fileId = id;
        }
    } catch (const MyMoneyException &) {
        // Exception usually means account was not found
        return QStringList();
    }

    if(d->jobList.contains(accountId)) {
        return d->jobList[accountId];
    }

    QStringList list;
    AB_ACCOUNT_SPEC* abAccount = aqbAccount(accountId);

    if (!abAccount) {
        return list;
    }

    // Check availableJobs

    // sepa transfer
    if (AB_AccountSpec_GetTransactionLimitsForCommand(abAccount, AB_Transaction_CommandSepaTransfer)) {
        list.append(sepaOnlineTransfer::name());
    }

    d->jobList[accountId] = list;
    return list;
}


/** @brief experimenting with QScopedPointer and aqBanking pointers */
class QScopedPointerAbJobDeleter
{
public:
    static void cleanup(AB_TRANSACTION* job) {
        AB_Transaction_free(job);
    }
};


/** @brief experimenting with QScopedPointer and aqBanking pointers */
class QScopedPointerAbAccountDeleter
{
public:
    static void cleanup(AB_ACCOUNT_SPEC* account) {
        AB_AccountSpec_free(account);
    }
};


IonlineTaskSettings::ptr KBanking::settings(QString accountId, QString taskName)
{
    AB_ACCOUNT_SPEC* abAcc = aqbAccount(accountId);
    if (abAcc == 0)
        return IonlineTaskSettings::ptr();

    if (sepaOnlineTransfer::name() == taskName) {
        // Get limits for sepaonlinetransfer
        const AB_TRANSACTION_LIMITS *limits=AB_AccountSpec_GetTransactionLimitsForCommand(abAcc, AB_Transaction_CommandSepaTransfer);
        if (limits==NULL)
            return IonlineTaskSettings::ptr();
        return AB_TransactionLimits_toSepaOnlineTaskSettings(limits).dynamicCast<IonlineTaskSettings>();
    }
    return IonlineTaskSettings::ptr();
}


bool KBanking::enqueTransaction(onlineJobTyped<sepaOnlineTransfer>& job)
{
    /* get AqBanking account */
    const QString accId = job.constTask()->responsibleAccount();

    AB_ACCOUNT_SPEC *abAccount = aqbAccount(accId);
    if (!abAccount) {
        job.addJobMessage(onlineJobMessage(eMyMoney::OnlineJob::MessageType::Warning, "KBanking", i18n("<qt>"
                                           "The given application account <b>%1</b> "
                                           "has not been mapped to an online "
                                           "account."
                                           "</qt>",
                                           MyMoneyFile::instance()->account(accId).name())));
        return false;
    }
    //setupAccountReference(acc, ba); // needed?

    if (AB_AccountSpec_GetTransactionLimitsForCommand(abAccount, AB_Transaction_CommandSepaTransfer)==NULL) {
        qDebug("AB_ERROR_OFFSET is %i", AB_ERROR_OFFSET);
        job.addJobMessage(onlineJobMessage(eMyMoney::OnlineJob::MessageType::Error, "AqBanking",
                                           QString("Sepa credit transfers for account \"%1\" are not available.").arg(MyMoneyFile::instance()->account(accId).name())
                                          )
                         );
        return false;
    }


    AB_TRANSACTION *abJob = AB_Transaction_new();

    /* command */
    AB_Transaction_SetCommand(abJob, AB_Transaction_CommandSepaTransfer);

    // Origin Account
    AB_Transaction_SetUniqueAccountId(abJob, AB_AccountSpec_GetUniqueId(abAccount));

    // Recipient
    payeeIdentifiers::ibanBic beneficiaryAcc = job.constTask()->beneficiaryTyped();
    AB_Transaction_SetRemoteName(abJob, beneficiaryAcc.ownerName().toUtf8().constData());
    AB_Transaction_SetRemoteIban(abJob, beneficiaryAcc.electronicIban().toUtf8().constData());
    AB_Transaction_SetRemoteBic(abJob, beneficiaryAcc.fullStoredBic().toUtf8().constData());

    // Origin Account
    AB_Transaction_SetLocalAccount(abJob, abAccount);

    // Purpose
    AB_Transaction_SetPurpose(abJob, job.constTask()->purpose().toUtf8().constData());

    // Reference
    // AqBanking duplicates the string. This should be safe.
    AB_Transaction_SetEndToEndReference(abJob, job.constTask()->endToEndReference().toUtf8().constData());

    // Other Fields
    AB_Transaction_SetTextKey(abJob, job.constTask()->textKey());
    AB_Transaction_SetValue(abJob, AB_Value_fromMyMoneyMoney(job.constTask()->value()));

    /** @todo LOW remove Debug info */
    AB_Transaction_SetStringIdForApplication(abJob, m_kbanking->mappingId(job).toUtf8().constData());

    qDebug() << "Enqueue: " << m_kbanking->enqueueJob(abJob);

    AB_Transaction_free(abJob);

    //delete localAcc;
    return true;
}


void KBanking::startPasswordTimer()
{
    if (d->passwordCacheTimer->isActive())
        d->passwordCacheTimer->stop();
    d->passwordCacheTimer->start();
}


void KBanking::slotClearPasswordCache()
{
    m_kbanking->clearPasswordCache();
}


void KBanking::slotImport()
{
    m_statementCount = 0;
    statementInterface()->resetMessages();

    if (!m_kbanking->interactiveImport())
        qWarning("Error on import dialog");
    else
        statementInterface()->showMessages(m_statementCount);
}


bool KBanking::importStatement(const MyMoneyStatement& s)
{
    m_statementCount++;
    return !statementInterface()->import(s).isEmpty();
}


MyMoneyAccount KBanking::account(const QString& key, const QString& value) const
{
    return statementInterface()->account(key, value);
}


void KBanking::setAccountOnlineParameters(const MyMoneyAccount& acc, const MyMoneyKeyValueContainer& kvps) const
{
    return statementInterface()->setAccountOnlineParameters(acc, kvps);
}


KBankingExt::KBankingExt(KBanking* parent, const char* appname, const char* fname)
    : AB_Banking(appname, fname)
    , m_parent(parent)
    , _jobQueue(0)
{
    m_sepaKeywords = {QString::fromUtf8("SEPA-BASISLASTSCHRIFT"), QString::fromUtf8("SEPA-ÜBERWEISUNG")};
    QRegularExpression exp("(\\d+\\.\\d+\\.\\d+).*");
    QRegularExpressionMatch match = exp.match(KAboutData::applicationData().version());

    QByteArray regkey;
    const char *p = "\x08\x0f\x41\x0f\x58\x5b\x56\x4a\x09\x7b\x40\x0e\x5f\x2a\x56\x3f\x0e\x7f\x3f\x7d\x5b\x56\x56\x4b\x0a\x4d";
    const char* q = appname;
    while (*p) {
        regkey += (*q++ ^ *p++) & 0xff;
        if (!*q)
            q = appname;
    }
    registerFinTs(regkey.data(), match.captured(1).remove(QLatin1Char('.')).left(5).toLatin1());
}


int KBankingExt::init()
{
    int rv = AB_Banking::init();
    if (rv < 0)
        return rv;

    _jobQueue = AB_Transaction_List2_new();
    return 0;
}


int KBankingExt::fini()
{
    if (_jobQueue) {
        AB_Transaction_List2_freeAll(_jobQueue);
        _jobQueue = 0;
    }

    return AB_Banking::fini();
}


int KBankingExt::executeQueue(AB_IMEXPORTER_CONTEXT *ctx)
{
    m_parent->startPasswordTimer();

    int rv = AB_Banking::executeJobs(_jobQueue, ctx);
    if (rv != 0) {
        qDebug() << "Sending queue by aqbanking got error no " << rv;
    }

    /** check result of each job */
    AB_TRANSACTION_LIST2_ITERATOR* jobIter = AB_Transaction_List2_First(_jobQueue);
    if (jobIter) {
        AB_TRANSACTION* abJob = AB_Transaction_List2Iterator_Data(jobIter);

        while (abJob) {
            const char *stringIdForApp=AB_Transaction_GetStringIdForApplication(abJob);

            if (!(stringIdForApp && *stringIdForApp)) {
                qWarning("Executed AB_Job without KMyMoney id");
                abJob = AB_Transaction_List2Iterator_Next(jobIter);
                continue;
            }
            QString jobIdent = QString::fromUtf8(stringIdForApp);

            onlineJob job = m_parent->m_onlineJobQueue.value(jobIdent);
            if (job.isNull()) {
                // It should not be possible that this will happen (only if AqBanking fails heavily).
                //! @todo correct exception text
                qWarning("Executed a job which was not in queue. Please inform the KMyMoney developers.");
                abJob = AB_Transaction_List2Iterator_Next(jobIter);
                continue;
            }

            AB_TRANSACTION_STATUS abStatus = AB_Transaction_GetStatus(abJob);

            if (abStatus == AB_Transaction_StatusSent //
                    || abStatus == AB_Transaction_StatusPending //
                    || abStatus == AB_Transaction_StatusAccepted //
                    || abStatus == AB_Transaction_StatusRejected //
                    || abStatus == AB_Transaction_StatusError //
                    || abStatus == AB_Transaction_StatusUnknown)
                job.setJobSend();

            if (abStatus == AB_Transaction_StatusAccepted)
                job.setBankAnswer(eMyMoney::OnlineJob::sendingState::acceptedByBank);
            else if (abStatus == AB_Transaction_StatusError //
                     || abStatus == AB_Transaction_StatusRejected //
                     || abStatus == AB_Transaction_StatusUnknown)
                job.setBankAnswer(eMyMoney::OnlineJob::sendingState::sendingError);

            job.addJobMessage(onlineJobMessage(eMyMoney::OnlineJob::MessageType::Debug, "KBanking", "Job was processed"));
            m_parent->m_onlineJobQueue.insert(jobIdent, job);
            abJob = AB_Transaction_List2Iterator_Next(jobIter);
        }
        AB_Transaction_List2Iterator_free(jobIter);
    }

    AB_TRANSACTION_LIST2 *oldQ = _jobQueue;
    _jobQueue = AB_Transaction_List2_new();
    AB_Transaction_List2_freeAll(oldQ);

    emit m_parent->queueChanged();
    m_parent->startPasswordTimer();

    return rv;
}


void KBankingExt::clearPasswordCache()
{
    /* clear password DB */
    GWEN_Gui_SetPasswordStatus(NULL, NULL, GWEN_Gui_PasswordStatus_Remove, 0);
}


std::list<AB_TRANSACTION*> KBankingExt::getEnqueuedJobs()
{
    AB_TRANSACTION_LIST2 *ll;
    std::list<AB_TRANSACTION*> rl;

    ll = _jobQueue;
    if (ll && AB_Transaction_List2_GetSize(ll)) {
        AB_TRANSACTION *j;
        AB_TRANSACTION_LIST2_ITERATOR *it;

        it = AB_Transaction_List2_First(ll);
        assert(it);
        j = AB_Transaction_List2Iterator_Data(it);
        assert(j);
        while (j) {
            rl.push_back(j);
            j = AB_Transaction_List2Iterator_Next(it);
        }
        AB_Transaction_List2Iterator_free(it);
    }
    return rl;
}


int KBankingExt::enqueueJob(AB_TRANSACTION *j)
{
    assert(_jobQueue);
    assert(j);
    AB_Transaction_Attach(j);
    AB_Transaction_List2_PushBack(_jobQueue, j);
    return 0;
}


int KBankingExt::dequeueJob(AB_TRANSACTION *j)
{
    assert(_jobQueue);
    AB_Transaction_List2_Remove(_jobQueue, j);
    AB_Transaction_free(j);
    emit m_parent->queueChanged();
    return 0;
}


void KBankingExt::transfer()
{
    //m_parent->transfer();
}


bool KBankingExt::askMapAccount(const MyMoneyAccount& acc)
{
    MyMoneyFile* file = MyMoneyFile::instance();

    QString bankId;
    QString accountId;
    // extract some information about the bank. if we have a bank code
    // (BLZ) we display it, otherwise the name is enough.
    try {
        const MyMoneyInstitution &bank = file->institution(acc.institutionId());
        bankId = bank.name();
        if (!bank.bankcode().isEmpty())
            bankId = bank.bankcode();
    } catch (const MyMoneyException &e) {
        // no bank assigned, we just leave the field empty
    }

    // extract account information. if we have an account number
    // we show it, otherwise the name will be displayed
    accountId = acc.number();
    if (accountId.isEmpty())
        accountId = acc.name();

    // do the mapping. the return value of this method is either
    // true, when the user mapped the account or false, if he
    // decided to quit the dialog. So not really a great thing
    // to present some more information.

    KBMapAccount *w;
    w = new KBMapAccount(this,
                         bankId.toUtf8().constData(),
                         accountId.toUtf8().constData());
    if (w->exec() == QDialog::Accepted) {
        AB_ACCOUNT_SPEC *a;

        a = w->getAccount();
        assert(a);
        DBG_NOTICE(0,
                   "Mapping application account \"%s\" to "
                   "online account \"%s/%s\"",
                   qPrintable(acc.name()),
                   AB_AccountSpec_GetBankCode(a),
                   AB_AccountSpec_GetAccountNumber(a));

        // TODO remove the following line once we don't need backward compatibility
        setAccountAlias(a, acc.id().toUtf8().constData());
        qDebug("Setup mapping to '%s'", acc.id().toUtf8().constData());

        setAccountAlias(a, mappingId(acc).toUtf8().constData());
        qDebug("Setup mapping to '%s'", mappingId(acc).toUtf8().constData());

        delete w;
        return true;
    }

    delete w;
    return false;
}


QString KBankingExt::mappingId(const MyMoneyObject& object) const
{
    return MyMoneyFile::instance()->storageId().toString(QUuid::WithoutBraces) + QLatin1Char('-') + object.id();
}


bool KBankingExt::interactiveImport()
{
    AB_IMEXPORTER_CONTEXT *ctx;
    GWEN_DIALOG *dlg;
    int rv;

    ctx = AB_ImExporterContext_new();
    dlg = AB_Banking_CreateImporterDialog(getCInterface(), ctx, NULL);
    if (dlg == NULL) {
        DBG_ERROR(0, "Could not create importer dialog.");
        AB_ImExporterContext_free(ctx);
        return false;
    }

    rv = GWEN_Gui_ExecDialog(dlg, 0);
    if (rv == 0) {
        DBG_ERROR(0, "Aborted by user");
        GWEN_Dialog_free(dlg);
        AB_ImExporterContext_free(ctx);
        return false;
    }

    if (!importContext(ctx, 0)) {
        DBG_ERROR(0, "Error on importContext");
        GWEN_Dialog_free(dlg);
        AB_ImExporterContext_free(ctx);
        return false;
    }

    GWEN_Dialog_free(dlg);
    AB_ImExporterContext_free(ctx);
    return true;
}



void KBankingExt::_xaToStatement(MyMoneyStatement &ks,
                                 const MyMoneyAccount& acc,
                                 const AB_TRANSACTION *t)
{
    QString s;
    QString memo;
    const char *p;
    const AB_VALUE *val;
    const GWEN_DATE *dt;
    const GWEN_DATE *startDate = 0;
    MyMoneyStatement::Transaction kt;
    unsigned long h;

    kt.m_fees = MyMoneyMoney();

    // bank's transaction id
    p = AB_Transaction_GetFiId(t);
    if (p)
        kt.m_strBankID = QString("ID ") + QString::fromUtf8(p);

    // payee
    p = AB_Transaction_GetRemoteName(t);
    if (p)
        kt.m_strPayee = QString::fromUtf8(p);

    // memo

    p = AB_Transaction_GetPurpose(t);
    if (p && *p) {
        QString tmpMemo;

        s=QString::fromUtf8(p).trimmed();
        tmpMemo=QString::fromUtf8(p).trimmed();
        tmpMemo.replace('\n', ' ');

        memo=tmpMemo;
    }

    // in case we have some SEPA fields filled with information
    // we add them to the memo field
    p = AB_Transaction_GetEndToEndReference(t);
    if (p) {
        s += QString(", EREF: %1").arg(p);
        if(memo.length())
            memo.append('\n');
        memo.append(QString("EREF: %1").arg(p));
    }
    p = AB_Transaction_GetCustomerReference(t);
    if (p) {
        s += QString(", CREF: %1").arg(p);
        if(memo.length())
            memo.append('\n');
        memo.append(QString("CREF: %1").arg(p));
    }
    p = AB_Transaction_GetMandateId(t);
    if (p) {
        s += QString(", MREF: %1").arg(p);
        if(memo.length())
            memo.append('\n');
        memo.append(QString("MREF: %1").arg(p));
    }
    p = AB_Transaction_GetCreditorSchemeId(t);
    if (p) {
        s += QString(", CRED: %1").arg(p);
        if(memo.length())
            memo.append('\n');
        memo.append(QString("CRED: %1").arg(p));
    }
    p = AB_Transaction_GetOriginatorId(t);
    if (p) {
        s += QString(", DEBT: %1").arg(p);
        if(memo.length())
            memo.append('\n');
        memo.append(QString("DEBT: %1").arg(p));
    }

    const MyMoneyKeyValueContainer& kvp = acc.onlineBankingSettings();
    // check if we need the version with or without linebreaks
    if (kvp.value("kbanking-memo-removelinebreaks").compare(QLatin1String("no"))) {
        kt.m_strMemo = memo;
    } else {
        kt.m_strMemo = s;
    }

    // calculate the hash code and start with the payee info
    // and append the memo field
    h = MyMoneyTransaction::hash(kt.m_strPayee.trimmed());
    h = MyMoneyTransaction::hash(s, h);

    // see, if we need to extract the payee from the memo field
    QString rePayee = kvp.value("kbanking-payee-regexp");
    if (!rePayee.isEmpty() && kt.m_strPayee.isEmpty()) {
        QString reMemo = kvp.value("kbanking-memo-regexp");
        QStringList exceptions = kvp.value("kbanking-payee-exceptions").split(';', QString::SkipEmptyParts);

        bool needExtract = true;
        QStringList::const_iterator it_s;
        for (it_s = exceptions.constBegin(); needExtract && it_s != exceptions.constEnd(); ++it_s) {
            QRegExp exp(*it_s, Qt::CaseInsensitive);
            if (exp.indexIn(kt.m_strMemo) != -1) {
                needExtract = false;
            }
        }
        if (needExtract) {
            QRegExp expPayee(rePayee, Qt::CaseInsensitive);
            QRegExp expMemo(reMemo, Qt::CaseInsensitive);
            if (expPayee.indexIn(kt.m_strMemo) != -1) {
                kt.m_strPayee = expPayee.cap(1);
                if (expMemo.indexIn(kt.m_strMemo) != -1) {
                    kt.m_strMemo = expMemo.cap(1);
                }
            }
        }
    }

    kt.m_strPayee = kt.m_strPayee.trimmed();

    // date
    dt = AB_Transaction_GetDate(t);
    if (!dt)
        dt = AB_Transaction_GetValutaDate(t);
    if (dt) {
        if (!startDate)
            startDate = dt;
        kt.m_datePosted = QDate(GWEN_Date_GetYear(dt), GWEN_Date_GetMonth(dt), GWEN_Date_GetDay(dt));
    } else {
        DBG_WARN(0, "No date for transaction");
    }

    // value
    val = AB_Transaction_GetValue(t);
    if (val) {
        if (ks.m_strCurrency.isEmpty()) {
            p = AB_Value_GetCurrency(val);
            if (p)
                ks.m_strCurrency = p;
        } else {
            p = AB_Value_GetCurrency(val);
            if (p)
                s = p;
            if (ks.m_strCurrency.toLower() != s.toLower()) {
                // TODO: handle currency difference
                DBG_ERROR(0, "Mixed currencies currently not allowed");
            }
        }

        kt.m_amount = MyMoneyMoney(AB_Value_GetValueAsDouble(val));
        // The initial implementation of this feature was based on
        // a denominator of 100. Since the denominator might be
        // different nowadays, we make sure to use 100 for the
        // duplicate detection
        QString tmpVal = kt.m_amount.formatMoney(100, false);
        tmpVal.remove(QRegExp("[,\\.]"));
        tmpVal += QLatin1String("/100");
        h = MyMoneyTransaction::hash(tmpVal, h);
    } else {
        DBG_WARN(0, "No value for transaction");
    }

    if (startDate) {
        QDate d(QDate(GWEN_Date_GetYear(startDate), GWEN_Date_GetMonth(startDate), GWEN_Date_GetDay(startDate)));

        if (!ks.m_dateBegin.isValid())
            ks.m_dateBegin = d;
        else if (d < ks.m_dateBegin)
            ks.m_dateBegin = d;

        if (!ks.m_dateEnd.isValid())
            ks.m_dateEnd = d;
        else if (d > ks.m_dateEnd)
            ks.m_dateEnd = d;
    } else {
        DBG_WARN(0, "No date in current transaction");
    }

    // add information about remote account to memo in case we have something
    const char *remoteAcc = AB_Transaction_GetRemoteAccountNumber(t);
    const char *remoteBankCode = AB_Transaction_GetRemoteBankCode(t);
    if (remoteAcc && remoteBankCode) {
        kt.m_strMemo += QString("\n%1/%2").arg(remoteBankCode, remoteAcc);
    }

    // make hash value unique in case we don't have one already
    if (kt.m_strBankID.isEmpty()) {
        QString hashBase;
        hashBase.sprintf("%s-%07lx", qPrintable(kt.m_datePosted.toString(Qt::ISODate)), h);
        int idx = 1;
        QString hash;
        for (;;) {
            hash = QString("%1-%2").arg(hashBase).arg(idx);
            QMap<QString, bool>::const_iterator it;
            it = m_hashMap.constFind(hash);
            if (it == m_hashMap.constEnd()) {
                m_hashMap[hash] = true;
                break;
            }
            ++idx;
        }
        kt.m_strBankID = QString("%1-%2").arg(acc.id(), hash);
    }

    // store transaction
    ks.m_listTransactions += kt;
}


void KBankingExt::_slToStatement(MyMoneyStatement &ks,
                                 const MyMoneyAccount& acc,
                                 const AB_SECURITY *sy)
{
    MyMoneyFile* file = MyMoneyFile::instance();
    const char *p;
    const AB_VALUE *val;
    const GWEN_TIME *ti;
    MyMoneyStatement::Security ksy;
    MyMoneyStatement::Price kpr;
    MyMoneyStatement::Transaction kt;

    // security name
    p = AB_Security_GetName(sy);
    if (p)
        ksy.m_strName = QString::fromUtf8(p);

    // security id
    p = AB_Security_GetUniqueId(sy);
    if (p) {
        ksy.m_strId = QString::fromUtf8(p);
        ksy.m_strSymbol = QString::fromUtf8(p);
        kpr.m_strSecurity = QString::fromUtf8(p);
    }

    // date
    ti = AB_Security_GetUnitPriceDate(sy);
    if (ti) {
        int year, month, day;

        if (!GWEN_Time_GetBrokenDownDate(ti, &day, &month, &year)) {
            kpr.m_date = QDate(year, month + 1, day);
        }
    }

    // value
    val = AB_Security_GetUnitPriceValue(sy);
    if (val)
        kpr.m_amount = MyMoneyMoney(AB_Value_GetValueAsDouble(val));

    // generate dummy booking in case online balance does not match
    MyMoneySecurity security;
    MyMoneyAccount sacc;
    foreach (const auto sAccount, file->account(acc.id()).accountList()) {
        sacc=file->account(sAccount);
        security=file->security(sacc.currencyId());
        if ((!ksy.m_strSymbol.isEmpty() && QString::compare(ksy.m_strSymbol, security.tradingSymbol(), Qt::CaseInsensitive) == 0) ||
                (!ksy.m_strName.isEmpty() && QString::compare(ksy.m_strName, security.name(), Qt::CaseInsensitive) == 0)) {
            if (sacc.balance().toDouble() != AB_Value_GetValueAsDouble(AB_Security_GetUnits(sy))) {
                qDebug("Creating dummy correction booking for '%s' with %f shares", qPrintable(security.tradingSymbol()),
                       AB_Value_GetValueAsDouble(AB_Security_GetUnits(sy))-sacc.balance().toDouble());
                kt.m_fees = MyMoneyMoney();
                kt.m_strMemo = "Dummy booking added by KMyMoney to reflect online balance - please adjust";
                kt.m_datePosted = QDate::currentDate();
                kt.m_strSymbol=security.tradingSymbol();
                kt.m_strSecurity=security.name();
                kt.m_strBrokerageAccount=acc.name();

                kt.m_shares=MyMoneyMoney(AB_Value_GetValueAsDouble(AB_Security_GetUnits(sy))-sacc.balance().toDouble());
                if (AB_Value_GetValueAsDouble(AB_Security_GetUnits(sy)) > sacc.balance().toDouble())
                    kt.m_eAction = eMyMoney::Transaction::Action::Shrsin;
                else
                    kt.m_eAction = eMyMoney::Transaction::Action::Shrsout;

                // store transaction
                ks.m_listTransactions += kt;
            }
            else
                qDebug("Online balance matches balance in KMyMoney account!");
        }
    }

    // store security
    ks.m_listSecurities += ksy;

    // store prices
    ks.m_listPrices += kpr;
}


bool KBankingExt::importAccountInfo(AB_IMEXPORTER_CONTEXT *ctx,
                                    AB_IMEXPORTER_ACCOUNTINFO *ai,
                                    uint32_t /*flags*/)
{
    const char *p;

    DBG_INFO(0, "Importing account...");

    // account number
    MyMoneyStatement ks;
    p = AB_ImExporterAccountInfo_GetAccountNumber(ai);
    if (p) {
        ks.m_strAccountNumber = m_parent->stripLeadingZeroes(p);
    }

    p = AB_ImExporterAccountInfo_GetBankCode(ai);
    if (p) {
        ks.m_strBankCode = m_parent->stripLeadingZeroes(p);
    }

    MyMoneyAccount kacc;
    if (!ks.m_strAccountNumber.isEmpty() || !ks.m_strBankCode.isEmpty()) {
        kacc = m_parent->account("kbanking-acc-ref", QString("%1-%2-%3").arg(ks.m_strBankCode, ks.m_strAccountNumber).arg(AB_ImExporterAccountInfo_GetAccountType(ai)));
        ks.m_accountId = kacc.id();
    }

    // account name
    p = AB_ImExporterAccountInfo_GetAccountName(ai);
    if (p)
        ks.m_strAccountName = p;

    // account type
    switch (AB_ImExporterAccountInfo_GetAccountType(ai)) {
    case AB_AccountType_Bank:
        ks.m_eType = eMyMoney::Statement::Type::Savings;
        break;
    case AB_AccountType_CreditCard:
        ks.m_eType = eMyMoney::Statement::Type::CreditCard;
        break;
    case AB_AccountType_Checking:
        ks.m_eType = eMyMoney::Statement::Type::Checkings;
        break;
    case AB_AccountType_Savings:
        ks.m_eType = eMyMoney::Statement::Type::Savings;
        break;
    case AB_AccountType_Investment:
        ks.m_eType = eMyMoney::Statement::Type::Investment;
        break;
    case AB_AccountType_Cash:
    default:
        ks.m_eType = eMyMoney::Statement::Type::None;
    }

    // account status
    const AB_BALANCE *bal = AB_Balance_List_GetLatestByType(AB_ImExporterAccountInfo_GetBalanceList(ai), AB_Balance_TypeBooked);
    if (!bal)
        bal = AB_Balance_List_GetLatestByType(AB_ImExporterAccountInfo_GetBalanceList(ai), AB_Balance_TypeNoted);
    if (bal) {
        const AB_VALUE* val = AB_Balance_GetValue(bal);
        if (val) {
            DBG_INFO(0, "Importing balance");
            ks.m_closingBalance = AB_Value_toMyMoneyMoney(val);
            p = AB_Value_GetCurrency(val);
            if (p)
                ks.m_strCurrency = p;
        }
        const GWEN_DATE* dt = AB_Balance_GetDate(bal);
        if (dt) {
            ks.m_dateEnd = QDate(GWEN_Date_GetYear(dt), GWEN_Date_GetMonth(dt), GWEN_Date_GetDay(dt));
        } else {
            DBG_WARN(0, "No date for balance");
        }
    } else {
        DBG_WARN(0, "No account balance");
    }

    // clear hash map
    m_hashMap.clear();

    // get all securities
    const AB_SECURITY* s = AB_ImExporterContext_GetFirstSecurity(ctx);
    while (s) {
        qDebug("Found security '%s'", AB_Security_GetName(s));
        _slToStatement(ks, kacc, s);
        s = AB_Security_List_Next(s);
    }

    // get all transactions
    const AB_TRANSACTION* t = AB_ImExporterAccountInfo_GetFirstTransaction(ai, AB_Transaction_TypeStatement, 0);
    while (t) {
        _xaToStatement(ks, kacc, t);
        t = AB_Transaction_List_FindNextByType(t, AB_Transaction_TypeStatement, 0);
    }

    // import them
    if (!m_parent->importStatement(ks)) {
        if (KMessageBox::warningYesNo(0,
                                      i18n("Error importing statement. Do you want to continue?"),
                                      i18n("Critical Error")) == KMessageBox::No) {
            DBG_ERROR(0, "User aborted");
            return false;
        }
    }
    return true;
}

K_PLUGIN_CLASS_WITH_JSON(KBanking, "kbanking.json")

#include "kbanking.moc"
