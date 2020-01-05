/***************************************************************************
 *   Copyright 2004  Martin Preuss aquamaniac@users.sourceforge.net        *
 *   Copyright 2009  Cristian Onet onet.cristian@gmail.com                 *
 *   Copyright 2010  Thomas Baumgart ipwizard@users.sourceforge.net        *
 *   Copyright 2015  Christian David christian-david@web.de                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public License as        *
 *   published by the Free Software Foundation; either version 2 of        *
 *   the License or (at your option) version 3 or any later version        *
 *   accepted by the membership of KDE e.V. (or its successor approved     *
 *   by the membership of KDE e.V.), which shall act as a proxy            *
 *   defined in Section 14 of version 3 of the license.                    *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>  *
 ***************************************************************************/
#ifdef HAVE_CONFIG_H
# include <config-kmymoney.h>
#endif

#include "mymoneybanking.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QLayout>
#include <QRadioButton>
#include <QStringList>
#include <QRegExp>
#include <QCheckBox>
#include <QLabel>
#include <QTimer>

#include <QDebug> //! @todo remove @c #include <QDebug>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocale>
#include <KMessageBox>
#include <KPluginFactory>
#include <KAction>
#include <KActionCollection>
#include <KGlobal>
#include <KStandardDirs>
#include <KMenu>
#include <KIconLoader>
#include <KGuiItem>
#include <KLineEdit>
#include <KEditListBox>
#include <KComboBox>
#include <KConfig>
#include <KConfigGroup>

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

#include "config-kmymoney-version.h"
#include "mymoney/onlinejob.h"

#include "kbaccountsettings.h"
#include "kbmapaccount.h"
#include "mymoneyfile.h"
#include "onlinejobadministration.h"
#include "kmymoneyview.h"
#include "kbpickstartdate.h"

#include "gwenkdegui.h"
#include "gwenhywfarqtoperators.h"
#include "aqbankingkmmoperators.h"

K_PLUGIN_FACTORY(KBankingFactory, registerPlugin<KBankingPlugin>();)
K_EXPORT_PLUGIN(KBankingFactory("kmm_kbanking"))

class KBankingPlugin::Private
{
public:
  Private() :
      passwordCacheTimer(0),
      jobList(),
      fileId()
  {
    QString gwenProxy = QString::fromLocal8Bit(qgetenv("GWEN_PROXY"));
    if (gwenProxy.isEmpty()) {
      QScopedPointer<KConfig> cfg(new KConfig("kioslaverc"));
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
            if (setenv("GWEN_PROXY", qPrintable(proxy), 1) == -1) {
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
    Q_UNUSED(domain);
    Q_UNUSED(level);

    const char* messageToFilter = "Job not supported with this account";
    if (strstr(message, messageToFilter) != 0)
      return 1;
    return 0;
  }

  QTimer *passwordCacheTimer;
  QMap<QString, QStringList>  jobList;
  QString                     fileId;
};

KBankingPlugin::KBankingPlugin(QObject *parent, const QVariantList&) :
    KMyMoneyPlugin::OnlinePluginExtended(parent, "KBanking"/*must be the same as X-KDE-PluginInfo-Name*/),
    d(new Private),
    m_configAction(0),
    m_importAction(0),
    // m_kbanking(), set below
    m_protocolConversionMap(),
    m_accountSettings(0),
    m_onlineJobQueue()
{
  m_kbanking = new KMyMoneyBanking(this, "KMyMoney");

  d->passwordCacheTimer = new QTimer(this);
  d->passwordCacheTimer->setSingleShot(true);
  d->passwordCacheTimer->setInterval(60000);
  connect(d->passwordCacheTimer, SIGNAL(timeout()), this, SLOT(slotClearPasswordCache()));

  if (m_kbanking) {

    //! @todo when is gwenKdeGui deleted?
    gwenKdeGui *gui = new gwenKdeGui();
    GWEN_Gui_SetGui(gui->getCInterface());
    GWEN_Logger_SetLevel(0, GWEN_LoggerLevel_Warning);

    if (m_kbanking->init() == 0) {
      // Tell the host application to load my GUI component
      setComponentData(KBankingFactory::componentData());
      setXMLFile("kmm_kbanking.rc");
      QString compileVersionSet = QLatin1String(GWENHYWFAR_VERSION_FULL_STRING "/" AQBANKING_VERSION_FULL_STRING);
      QString runtimeVersionSet = QString("%1/%2").arg(d->libVersion(&GWEN_Version), d->libVersion(&AB_Banking_GetVersion));
      qDebug() << QString("Plugins: kbanking loaded, build with (%1), run with (%2)").arg(compileVersionSet, runtimeVersionSet);

      // get certificate handling and dialog settings management
      AB_Gui_Extend(gui->getCInterface(), m_kbanking->getCInterface());

      // create actions
      createActions();

      // load protocol conversion list
      loadProtocolConversion();
      GWEN_Logger_SetLevel(AQBANKING_LOGDOMAIN, GWEN_LoggerLevel_Warning);
      GWEN_Gui_SetLogHookFn(GWEN_Gui_GetGui(), &KBankingPlugin::Private::gwenLogHook);

    } else {
      qWarning("Could not initialize KBanking online banking interface");
      delete m_kbanking;
      m_kbanking = 0;
    }
  }
}



KBankingPlugin::~KBankingPlugin()
{
  if (m_kbanking) {
    m_kbanking->fini();
    delete m_kbanking;
  }
  delete d;
}


void KBankingPlugin::loadProtocolConversion()
{
  if (m_kbanking) {
    m_protocolConversionMap.clear();
    m_protocolConversionMap["aqhbci"] = "HBCI";
    m_protocolConversionMap["aqofxconnect"] = "OFX";
    m_protocolConversionMap["aqyellownet"] = "YellowNet";
    m_protocolConversionMap["aqgeldkarte"] = "Geldkarte";
    m_protocolConversionMap["aqdtaus"] = "DTAUS";
  }
}

void KBankingPlugin::protocols(QStringList& protocolList) const
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

QWidget* KBankingPlugin::accountConfigTab(const MyMoneyAccount& acc, QString& name)
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

MyMoneyKeyValueContainer KBankingPlugin::onlineBankingSettings(const MyMoneyKeyValueContainer& current)
{
  MyMoneyKeyValueContainer kvp(current);
  kvp["provider"] = objectName();
  if (m_accountSettings) {
    m_accountSettings->loadKvp(kvp);
  }
  return kvp;
}

void KBankingPlugin::createActions()
{
  KAction *settings_aqbanking  = actionCollection()->addAction("settings_aqbanking");
  settings_aqbanking->setText(i18n("Configure Aq&Banking..."));
  connect(settings_aqbanking, SIGNAL(triggered()), this, SLOT(slotSettings()));

  KAction *file_import_aqbanking  = actionCollection()->addAction("file_import_aqbanking");
  file_import_aqbanking->setText(i18n("AqBanking importer..."));
  connect(file_import_aqbanking, SIGNAL(triggered()), this, SLOT(slotImport()));

  connect(viewInterface(), SIGNAL(viewStateChanged(bool)), action("file_import_aqbanking"), SLOT(setEnabled(bool)));
}

void KBankingPlugin::slotSettings()
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

bool KBankingPlugin::mapAccount(const MyMoneyAccount& acc, MyMoneyKeyValueContainer& settings)
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

AB_ACCOUNT_SPEC* KBankingPlugin::aqbAccount(const MyMoneyAccount& acc) const
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

AB_ACCOUNT_SPEC* KBankingPlugin::aqbAccount(const QString& accountId) const
{
  MyMoneyAccount account = MyMoneyFile::instance()->account(accountId);
  return aqbAccount(account);
}


QString KBankingPlugin::stripLeadingZeroes(const QString& s) const
{
  QString rc(s);
  QRegExp exp("^(0*)([^0].*)");
  if (exp.exactMatch(s)) {
    rc = exp.cap(2);
  }
  return rc;
}

void KBankingPlugin::setupAccountReference(const MyMoneyAccount& acc, AB_ACCOUNT_SPEC* ab_acc)
{
  MyMoneyKeyValueContainer kvp;

  if (ab_acc) {
    QString accountNumber = stripLeadingZeroes(AB_AccountSpec_GetAccountNumber(ab_acc));
    QString routingNumber = stripLeadingZeroes(AB_AccountSpec_GetBankCode(ab_acc));

    QString val = QString("%1-%2").arg(routingNumber, accountNumber);
    if (val != acc.onlineBankingSettings().value("kbanking-acc-ref")) {
      MyMoneyKeyValueContainer kvp;

      // make sure to keep our own previous settings
      const QMap<QString, QString>& vals = acc.onlineBankingSettings().pairs();
      QMap<QString, QString>::const_iterator it_p;
      for (it_p = vals.begin(); it_p != vals.end(); ++it_p) {
        if (QString(it_p.key()).startsWith("kbanking-")) {
          kvp.setValue(it_p.key(), *it_p);
        }
      }

      kvp.setValue("kbanking-acc-ref", val);
      kvp.setValue("provider", objectName());
      setAccountOnlineParameters(acc, kvp);
    }
  } else {
    // clear the connection
    setAccountOnlineParameters(acc, kvp);
  }
}

bool KBankingPlugin::accountIsMapped(const MyMoneyAccount& acc)
{
  return aqbAccount(acc) != 0;
}

bool KBankingPlugin::updateAccount(const MyMoneyAccount& acc)
{
  return updateAccount(acc, false);
}

bool KBankingPlugin::updateAccount(const MyMoneyAccount& acc, bool moreAccounts)
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

void KBankingPlugin::executeQueue()
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
void KBankingPlugin::sendOnlineJob(QList<onlineJob>& jobs)
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
        job.addJobMessage(onlineJobMessage(onlineJobMessage::error, "KBanking", "Cannot handle this request"));
        unhandledJobs.append(job);
      }
      m_onlineJobQueue.insert(m_kbanking->mappingId(job), job);
    }

    executeQueue();
  }
  jobs = m_onlineJobQueue.values() + unhandledJobs;
  m_onlineJobQueue.clear();
}

QStringList KBankingPlugin::availableJobs(QString accountId)
{
  try {
    MyMoneyAccount acc = MyMoneyFile::instance()->account(accountId);
    QString id = MyMoneyFile::instance()->value("kmm-id");
    if(id != d->fileId) {
      d->jobList.clear();
      d->fileId = id;
    }
  } catch (const MyMoneyException&) {
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

IonlineTaskSettings::ptr KBankingPlugin::settings(QString accountId, QString taskName)
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

bool KBankingPlugin::enqueTransaction(onlineJobTyped<sepaOnlineTransfer>& job)
{
  /* get AqBanking account */
  const QString accId = job.constTask()->responsibleAccount();

  AB_ACCOUNT_SPEC *abAccount = aqbAccount(accId);
  if (!abAccount) {
    job.addJobMessage(onlineJobMessage(onlineJobMessage::warning, "KBanking", i18n("<qt>"
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
    job.addJobMessage(onlineJobMessage(onlineJobMessage::error, "AqBanking",
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

void KBankingPlugin::startPasswordTimer()
{
  if (d->passwordCacheTimer->isActive())
    d->passwordCacheTimer->stop();
  d->passwordCacheTimer->start();
}

void KBankingPlugin::slotClearPasswordCache()
{
  m_kbanking->clearPasswordCache();
}

void KBankingPlugin::slotImport()
{
  if (!m_kbanking->interactiveImport())
    qWarning("Error on import dialog");
}



bool KBankingPlugin::importStatement(const MyMoneyStatement& s)
{
  return statementInterface()->import(s);
}

const MyMoneyAccount& KBankingPlugin::account(const QString& key, const QString& value) const
{
  return statementInterface()->account(key, value);
}

void KBankingPlugin::setAccountOnlineParameters(const MyMoneyAccount& acc, const MyMoneyKeyValueContainer& kvps) const
{
  return statementInterface()->setAccountOnlineParameters(acc, kvps);
}


KMyMoneyBanking::KMyMoneyBanking(KBankingPlugin* parent, const char* appname, const char* fname)
    : AB_Banking(appname, fname)
    , m_parent(parent)
    , _jobQueue(0)
{
  m_sepaKeywords  << QString("SEPA-BASISLASTSCHRIFT")
  << QString::fromUtf8("SEPA-ÜBERWEISUNG");

  QStringList versions = QString(VERSION).split(".");

  QByteArray regkey;
  const char *p = "\x08\x0f\x41\x0f\x58\x5b\x56\x4a\x09\x7b\x40\x0e\x5f\x2a\x56\x3f\x0e\x7f\x3f\x7d\x5b\x56\x56\x4b\x0a\x4d";
  const char* q = appname;
  while (*p) {
    regkey += (*q++ ^ *p++) & 0xff;
    if (!*q)
      q = appname;
  }
  registerFinTs(regkey.data(), versions[0].toLatin1());
}

int KMyMoneyBanking::init()
{
  int rv = AB_Banking::init();
  if (rv < 0)
    return rv;

  _jobQueue = AB_Transaction_List2_new();
  return 0;
}

int KMyMoneyBanking::fini()
{
  if (_jobQueue) {
    AB_Transaction_List2_freeAll(_jobQueue);
    _jobQueue = 0;
  }

  return AB_Banking::fini();
}

int KMyMoneyBanking::executeQueue(AB_IMEXPORTER_CONTEXT *ctx)
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
        // It should not be possiblie that this will happen (only if AqBanking fails heavily).
        //! @todo correct exception text
        qWarning("Executed a job which was not in queue. Please inform the KMyMoney developers.");
        abJob = AB_Transaction_List2Iterator_Next(jobIter);
        continue;
      }

      AB_TRANSACTION_STATUS abStatus = AB_Transaction_GetStatus(abJob);

      if (abStatus == AB_Transaction_StatusSent
          || abStatus == AB_Transaction_StatusPending
          || abStatus == AB_Transaction_StatusAccepted
          || abStatus == AB_Transaction_StatusRejected
          || abStatus == AB_Transaction_StatusError
          || abStatus == AB_Transaction_StatusUnknown)
        job.setJobSend();

      if (abStatus == AB_Transaction_StatusAccepted)
        job.setBankAnswer(onlineJob::acceptedByBank);
      else if (abStatus == AB_Transaction_StatusError
               || abStatus == AB_Transaction_StatusRejected
               || abStatus == AB_Transaction_StatusUnknown)
        job.setBankAnswer(onlineJob::sendingError);

      job.addJobMessage(onlineJobMessage(onlineJobMessage::debug, "KBanking", "Job was processed"));
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

void KMyMoneyBanking::clearPasswordCache()
{
  /* clear password DB */
  GWEN_Gui_SetPasswordStatus(NULL, NULL, GWEN_Gui_PasswordStatus_Remove, 0);
}

std::list<AB_TRANSACTION*> KMyMoneyBanking::getEnqueuedJobs()
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


int KMyMoneyBanking::enqueueJob(AB_TRANSACTION *j)
{
  assert(_jobQueue);
  assert(j);
  AB_Transaction_Attach(j);
  AB_Transaction_List2_PushBack(_jobQueue, j);
  return 0;
}


int KMyMoneyBanking::dequeueJob(AB_TRANSACTION *j)
{
  assert(_jobQueue);
  AB_Transaction_List2_Remove(_jobQueue, j);
  AB_Transaction_free(j);
  emit m_parent->queueChanged();
  return 0;
}

void KMyMoneyBanking::transfer()
{
  //m_parent->transfer();
}


bool KMyMoneyBanking::askMapAccount(const MyMoneyAccount& acc)
{
  MyMoneyFile* file = MyMoneyFile::instance();

  QString bankId;
  QString accountId;
  // extract some information about the bank. if we have a sortcode
  // (BLZ) we display it, otherwise the name is enough.
  try {
    const MyMoneyInstitution &bank = file->institution(acc.institutionId());
    bankId = bank.name();
    if (!bank.sortcode().isEmpty())
      bankId = bank.sortcode();
  } catch (const MyMoneyException &e) {
    // no bank assigned, we just leave the field emtpy
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

QString KMyMoneyBanking::mappingId(const MyMoneyObject& object) const
{
  QString id = MyMoneyFile::instance()->storageId() + QLatin1Char('-') + object.id();

  // AqBanking does not handle the enclosing parens, so we remove it
  id.remove('{');
  id.remove('}');
  return id;
}

bool KMyMoneyBanking::interactiveImport()
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


void KMyMoneyBanking::_xaToStatement(MyMoneyStatement &ks,
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
#if 1
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

#else
  // The variable 's' contains the old method of extracting
  // the memo which added a linefeed after each part received
  // from AqBanking. The new variable 'memo' does not have
  // this inserted linefeed. We keep the variable 's' to
  // construct the hash-value to retrieve the reference
  s.truncate(0);
  sl = AB_Transaction_GetPurpose(t);
  if (sl) {
    GWEN_STRINGLISTENTRY *se;
    bool insertLineSep = false;

    se = GWEN_StringList_FirstEntry(sl);
    while (se) {
      p = GWEN_StringListEntry_Data(se);
      assert(p);
      if (insertLineSep)
        s += '\n';
      insertLineSep = true;
      s += QString::fromUtf8(p).trimmed();
      memo += QString::fromUtf8(p).trimmed();
      se = GWEN_StringListEntry_Next(se);
    } // while

    // Sparda / Netbank hack: the software these banks use stores
    // parts of the payee name in the beginning of the purpose field
    // in case the payee name exceeds the 27 character limit. This is
    // the case, when one of the strings listed in m_sepaKeywords is part
    // of the purpose fields but does not start at the beginning. In this
    // case, the part leading up to the keyword is to be treated as the
    // tail of the payee. Also, a blank is inserted after the keyword.
    QSet<QString>::const_iterator itk;
    for (itk = m_sepaKeywords.constBegin(); itk != m_sepaKeywords.constEnd(); ++itk) {
      int idx = s.indexOf(*itk);
      if (idx >= 0) {
        if (idx > 0) {
          // re-add a possibly removed blank to name
          if (kt.m_strPayee.length() < 27)
            kt.m_strPayee += ' ';
          kt.m_strPayee += s.left(idx);
          s = s.mid(idx);
        }
        s = QString("%1 %2").arg(*itk).arg(s.mid((*itk).length()));

        // now do the same for 'memo' except for updating the payee
        idx = memo.indexOf(*itk);
        if (idx >= 0) {
          if (idx > 0) {
            memo = memo.mid(idx);
          }
        }
        memo = QString("%1 %2").arg(*itk).arg(memo.mid((*itk).length()));
        break;
      }
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
  }
#endif

  kt.m_strMemo = memo;

  // calculate the hash code and start with the payee info
  // and append the memo field
  h = MyMoneyTransaction::hash(kt.m_strPayee.trimmed());
  h = MyMoneyTransaction::hash(s, h);

  // see, if we need to extract the payee from the memo field
  const MyMoneyKeyValueContainer& kvp = acc.onlineBankingSettings();
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
    /* dead code
    else {
      if (GWEN_Time_Diff(ti, startTime) < 0)
        startTime = ti;
    }
    */
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
    kt.m_strBankID = QString("%1-%2").arg(acc.id()).arg(hash);
  }

  // store transaction
  ks.m_listTransactions += kt;
}



bool KMyMoneyBanking::importAccountInfo(AB_IMEXPORTER_ACCOUNTINFO *ai,
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
    ks.m_strRoutingNumber = m_parent->stripLeadingZeroes(p);
  }

  MyMoneyAccount kacc = m_parent->account("kbanking-acc-ref", QString("%1-%2").arg(ks.m_strRoutingNumber, ks.m_strAccountNumber));
  ks.m_accountId = kacc.id();

  // account name
  p = AB_ImExporterAccountInfo_GetAccountName(ai);
  if (p)
    ks.m_strAccountName = p;

  // account type
  switch (AB_ImExporterAccountInfo_GetAccountType(ai)) {
    case AB_AccountType_Bank:
      ks.m_eType = MyMoneyStatement::etSavings;
      break;
    case AB_AccountType_CreditCard:
      ks.m_eType = MyMoneyStatement::etCreditCard;
      break;
    case AB_AccountType_Checking:
      ks.m_eType = MyMoneyStatement::etCheckings;
      break;
    case AB_AccountType_Savings:
      ks.m_eType = MyMoneyStatement::etSavings;
      break;
    case AB_AccountType_Investment:
      ks.m_eType = MyMoneyStatement::etInvestment;
      break;
    case AB_AccountType_Cash:
      ks.m_eType = MyMoneyStatement::etNone;
      break;
    default:
      ks.m_eType = MyMoneyStatement::etNone;
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
      ks.m_dateEnd = QDate(GWEN_Date_GetYear(dt), GWEN_Date_GetMonth(dt) , GWEN_Date_GetDay(dt));
    } else {
      DBG_WARN(0, "No date for balance");
    }
  } else {
    DBG_WARN(0, "No account balance");
  }

  // clear hash map
  m_hashMap.clear();

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
