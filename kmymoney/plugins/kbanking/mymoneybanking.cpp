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

#include <aqbanking/imexporter.h>
#include <aqbanking/jobsingletransfer.h>
#include <aqbanking/jobsepatransfer.h>
#include <aqbanking/jobgettransactions.h>
#include <aqbanking/jobgetbalance.h>
#include <aqbanking/job.h>
#include <aqbanking/abgui.h>
#include <aqbanking/dlg_setup.h>
#include <aqbanking/dlg_importer.h>
#include <aqbanking/transactionlimits.h>
#include <gwenhywfar/logger.h>
#include <gwenhywfar/debug.h>

// ----------------------------------------------------------------------------
// Project Includes

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
  Private() : passwordCacheTimer(0) {
    QString gwenProxy = QString::fromLocal8Bit(qgetenv("GWEN_PROXY"));
    if (gwenProxy.isEmpty()) {
      KConfig *cfg = new KConfig("kioslaverc");
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
      delete cfg;
    }
  }

  QTimer *passwordCacheTimer;
};

KBankingPlugin::KBankingPlugin(QObject *parent, const QVariantList&) :
    KMyMoneyPlugin::OnlinePluginExtended(parent, "KBanking"/*must be the same as X-KDE-PluginInfo-Name*/),
    d(new Private),
    m_accountSettings(0)
{
  m_kbanking = new KMyMoneyBanking(this, "KMyMoney");

  d->passwordCacheTimer = new QTimer(this);
  d->passwordCacheTimer->setSingleShot(true);
  d->passwordCacheTimer->setInterval(60000);
  connect(d->passwordCacheTimer, SIGNAL(timeout()), this, SLOT(slotClearPasswordCache()));

  if (m_kbanking) {
    if (AB_Banking_HasConf4(m_kbanking->getCInterface())) {
      qDebug("KBankingPlugin: No AqB4 config found.");
      if (AB_Banking_HasConf3(m_kbanking->getCInterface())) {
        qDebug("KBankingPlugin: No AqB3 config found.");
        if (!AB_Banking_HasConf2(m_kbanking->getCInterface())) {
          qDebug("KBankingPlugin: AqB2 config found - converting.");
          AB_Banking_ImportConf2(m_kbanking->getCInterface());
        }
      } else {
        qDebug("KBankingPlugin: AqB3 config found - converting.");
        AB_Banking_ImportConf3(m_kbanking->getCInterface());
      }
    }

    //! @todo when is gwenKdeGui deleted?
    gwenKdeGui *gui = new gwenKdeGui();
    GWEN_Gui_SetGui(gui->getCInterface());
    GWEN_Logger_SetLevel(0, GWEN_LoggerLevel_Critical);

    if (m_kbanking->init() == 0) {
      // Tell the host application to load my GUI component
      setComponentData(KBankingFactory::componentData());
      setXMLFile("kmm_kbanking.rc");
      qDebug("KMyMoney kbanking plugin loaded");

      // get certificate handling and dialog settings management
      AB_Gui_Extend(gui->getCInterface(), m_kbanking->getCInterface());

      // create actions
      createActions();

      // load protocol conversion list
      loadProtocolConversion();
      GWEN_Logger_SetLevel(AQBANKING_LOGDOMAIN, GWEN_LoggerLevel_Critical);

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


void KBankingPlugin::loadProtocolConversion(void)
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

void KBankingPlugin::createActions(void)
{
  KAction *settings_aqbanking  = actionCollection()->addAction("settings_aqbanking");
  settings_aqbanking->setText(i18n("Configure Aq&Banking..."));
  connect(settings_aqbanking, SIGNAL(triggered()), this, SLOT(slotSettings()));

  KAction *file_import_aqbanking  = actionCollection()->addAction("file_import_aqbanking");
  file_import_aqbanking->setText(i18n("AqBanking importer..."));
  connect(file_import_aqbanking, SIGNAL(triggered()), this, SLOT(slotImport()));

  connect(viewInterface(), SIGNAL(viewStateChanged(bool)), action("file_import_aqbanking"), SLOT(setEnabled(bool)));
}

void KBankingPlugin::slotSettings(void)
{
  if (m_kbanking) {
    GWEN_DIALOG *dlg;
    int rv;

    dlg = AB_SetupDialog_new(m_kbanking->getCInterface());
    if (dlg == NULL) {
      DBG_ERROR(0, "Could not create setup dialog.");
      return;
    }

    rv = GWEN_Gui_ExecDialog(dlg, 0);
    if (rv == 0) {
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
    AB_ACCOUNT* ab_acc;
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

AB_ACCOUNT* KBankingPlugin::aqbAccount(const MyMoneyAccount& acc) const
{
  if (m_kbanking == 0) {
    return 0;
  }

  // certainly looking for an expense or income account does not make sense at this point
  // so we better get out right away
  if (acc.isIncomeExpense()) {
    return 0;
  }

  AB_ACCOUNT *ab_acc = AB_Banking_GetAccountByAlias(m_kbanking->getCInterface(), m_kbanking->mappingId(acc).toUtf8().data());
  // if the account is not found, we temporarily scan for the 'old' mapping (the one w/o the file id)
  // in case we find it, we setup the new mapping in addition on the fly.
  if (!ab_acc && acc.isAssetLiability()) {
    ab_acc = AB_Banking_GetAccountByAlias(m_kbanking->getCInterface(), acc.id().toUtf8().data());
    if (ab_acc) {
      qDebug("Found old mapping for '%s' but not new. Setup new mapping", qPrintable(acc.name()));
      m_kbanking->setAccountAlias(ab_acc, m_kbanking->mappingId(acc).toUtf8().constData());
      // TODO at some point in time, we should remove the old mapping
    }
  }
  return ab_acc;
}

AB_ACCOUNT* KBankingPlugin::aqbAccount(const QString& accountId) const
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

void KBankingPlugin::setupAccountReference(const MyMoneyAccount& acc, AB_ACCOUNT* ab_acc)
{
  MyMoneyKeyValueContainer kvp;

  if (ab_acc) {
    QString accountNumber = stripLeadingZeroes(AB_Account_GetAccountNumber(ab_acc));
    QString routingNumber = stripLeadingZeroes(AB_Account_GetBankCode(ab_acc));

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
    AB_JOB *job = 0;
    int rv;
    int days;
    int year, month, day;

    /* get AqBanking account */
    AB_ACCOUNT *ba = aqbAccount(acc);
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
      if (acc.onlineBankingSettings().value("kbanking-txn-download") != "no") {
        /* create getTransactions job */
        job = AB_JobGetTransactions_new(ba);
        rv = AB_Job_CheckAvailability(job);
        if (rv) {
          DBG_ERROR(0, "Job \"GetTransactions\" is not available (%d)", rv);
          KMessageBox::error(0,
                             i18n("<qt>"
                                  "The update job is not supported by the "
                                  "bank/account/backend.\n"
                                  "</qt>"),
                             i18n("Job not Available"));
          AB_Job_free(job);
          job = 0;
        }

        if (job) {
          days = AB_JobGetTransactions_GetMaxStoreDays(job);
          QDate qd;
          if (days > 0) {
            GWEN_TIME *ti1;
            GWEN_TIME *ti2;

            ti1 = GWEN_CurrentTime();
            ti2 = GWEN_Time_fromSeconds(GWEN_Time_Seconds(ti1) - (60 * 60 * 24 * days));
            GWEN_Time_free(ti1);
            ti1 = ti2;

            if (GWEN_Time_GetBrokenDownDate(ti1, &day, &month, &year)) {
              DBG_ERROR(0, "Bad date");
              qd = QDate();
            } else
              qd = QDate(year, month + 1, day);
            GWEN_Time_free(ti1);
          }

          // get last statement request date from application account object
          // and start from the next day if the date is valid
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
          bool enqueJob = false;
          if (dateOption == 0 || (dateOption > 1 && !qd.isValid())) {
            QPointer<KBPickStartDate> psd = new KBPickStartDate(m_kbanking, qd, lastUpdate, acc.name(),
                lastUpdate.isValid() ? 2 : 3, 0, true);
            if (psd->exec() == QDialog::Accepted) {
              enqueJob = true;
              qd = psd->date();
            } else {
              qd = QDate();
            }
            delete psd;
          }

          if (enqueJob) {
            if (qd.isValid()) {
              GWEN_TIME *ti1;

              ti1 = GWEN_Time_new(qd.year(), qd.month() - 1, qd.day(), 0, 0, 0, 0);
              AB_JobGetTransactions_SetFromTime(job, ti1);
              GWEN_Time_free(ti1);
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
          AB_Job_free(job);
        }
      }

      /* create getBalance job */
      job = AB_JobGetBalance_new(ba);
      rv = AB_Job_CheckAvailability(job);
      if (!rv)
        rv = m_kbanking->enqueueJob(job);
      else
        rv = 0;

      AB_Job_free(job);
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

  // make sure we have at least one job in the queue before sending it
  if (!moreAccounts && m_kbanking->getEnqueuedJobs().size() > 0)
    executeQueue();

  return rc;
}

void KBankingPlugin::executeQueue(void)
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
  QList<onlineJob> unhandledJobs;

  if (jobs.size()) {
    foreach (onlineJob job, jobs) {
      if (germanOnlineTransfer::name() == job.task()->taskName()) {
        onlineJobTyped<germanOnlineTransfer> typedJob(job);
        enqueTransaction(typedJob);
        job = typedJob;
      } else if (sepaOnlineTransfer::name() == job.task()->taskName()) {
        onlineJobTyped<sepaOnlineTransfer> typedJob(job);
        enqueTransaction(typedJob);
        job = typedJob;
      } else {
        job.addJobMessage(onlineJobMessage(onlineJobMessage::error, "KBanking", "Cannot handle this request"));
        unhandledJobs.append(job);
      }
      m_onlineJobQueue.insert(m_kbanking->mappingId(job), job);
    }

    //emit queueChanged();
    executeQueue();
  }
  jobs = m_onlineJobQueue.values() + unhandledJobs;
  m_onlineJobQueue.clear();
}

QStringList KBankingPlugin::availableJobs(QString accountId)
{
  QStringList list = QStringList();

  try {
    MyMoneyAccount acc = MyMoneyFile::instance()->account(accountId);
  } catch (const MyMoneyException&) {
    // Exception usually means account was not found
    return QStringList();
  }

  AB_ACCOUNT* abAccount = aqbAccount(accountId);

  if (!abAccount) {
    return list;
  }

  // Check availableJobs

  // national transfer
  AB_JOB *abJob = AB_JobSingleTransfer_new(abAccount);
  if (AB_Job_CheckAvailability(abJob) == 0)
    list.append(germanOnlineTransfer::name());
  AB_Job_free(abJob);

  // sepa transfer
  abJob = AB_JobSepaTransfer_new(abAccount);
  if (AB_Job_CheckAvailability(abJob) == 0)
    list.append(sepaOnlineTransfer::name());
  AB_Job_free(abJob);

  return list;
}

/** @brief experimenting with QScopedPointer and aqBanking pointers */
class QScopedPointerAbJobDeleter
{
public:
  static void cleanup(AB_JOB* job) {
    AB_Job_free(job);
  }
};

/** @brief experimenting with QScopedPointer and aqBanking pointers */
class QScopedPointerAbAccountDeleter
{
public:
  static void cleanup(AB_ACCOUNT* account) {
    AB_Account_free(account);
  }
};

IonlineTaskSettings::ptr KBankingPlugin::settings(QString accountId, QString taskName)
{
  AB_ACCOUNT* abAcc = aqbAccount(accountId);
  if (abAcc == 0)
    return IonlineTaskSettings::ptr();

  if (germanOnlineTransfer::name() == taskName) {
    // Get Limits for germanOnlineTransfer
    QScopedPointer<AB_JOB, QScopedPointerAbJobDeleter> abJob(AB_JobSingleTransfer_new(abAcc));
    if (AB_Job_CheckAvailability(abJob.data()) != 0)
      return IonlineTaskSettings::ptr();

    const AB_TRANSACTION_LIMITS* limits = AB_Job_GetFieldLimits(abJob.data());
    return AB_TransactionLimits_toGermanOnlineTaskSettings(limits).dynamicCast<IonlineTaskSettings>();
    //! @todo needs free? because that is not possible with const AB_TRANSACTION_LIMITS*
    // AB_TransactionLimits_free( limits );
  } else if (sepaOnlineTransfer::name() == taskName) {
    // Get limits for sepaonlinetransfer
    QScopedPointer<AB_JOB, QScopedPointerAbJobDeleter> abJob(AB_JobSepaTransfer_new(abAcc));
    if (AB_Job_CheckAvailability(abJob.data()) != 0)
      return IonlineTaskSettings::ptr();
    const AB_TRANSACTION_LIMITS* limits = AB_Job_GetFieldLimits(abJob.data());
    return AB_TransactionLimits_toSepaOnlineTaskSettings(limits).dynamicCast<IonlineTaskSettings>();
  }
  return IonlineTaskSettings::ptr();
}

bool KBankingPlugin::enqueTransaction(onlineJobTyped<germanOnlineTransfer>& job)
{
  /* get AqBanking account */
  QString accId = job.constTask()->responsibleAccount();
  AB_ACCOUNT *abAccount = aqbAccount(accId);
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

  AB_JOB *abJob = AB_JobSingleTransfer_new(abAccount);
  int rv = AB_Job_CheckAvailability(abJob);
  if (rv) {
    qDebug("AB_ERROR_OFFSET is %i", AB_ERROR_OFFSET);
    job.addJobMessage(onlineJobMessage::error, "AqBanking",
                      QString("National credit transfers for account \"%1\" are not available, error code %2.").arg(MyMoneyFile::instance()->account(accId).name(), rv),
                      QString::number(rv)
                     );
    return false;
  }
  AB_TRANSACTION *abTransaction = AB_Transaction_new();

  // Recipient
  payeeIdentifiers::nationalAccount beneficiaryAcc = job.task()->beneficiaryTyped();
  AB_Transaction_SetRemoteAccount(abTransaction, beneficiaryAcc);

  // Origin Account
  AB_Transaction_SetLocalAccount(abTransaction, abAccount);

  // Purpose
  QStringList qPurpose = job.task()->purpose().split('\n', QString::SkipEmptyParts);
  GWEN_STRINGLIST *purpose = GWEN_StringList_fromQStringList(qPurpose);
  AB_Transaction_SetPurpose(abTransaction, purpose);
  GWEN_StringList_free(purpose);

  // Other
  AB_Transaction_SetTextKey(abTransaction, job.task()->textKey());
  AB_Transaction_SetValue(abTransaction, AB_Value_fromMyMoneyMoney(job.task()->value()));

  /** @todo LOW remove Debug info */
  qDebug() << "SetTransaction: " << AB_Job_SetTransaction(abJob, abTransaction);

  GWEN_DB_NODE *gwenNode = AB_Job_GetAppData(abJob);
  GWEN_DB_SetCharValue(gwenNode, GWEN_DB_FLAGS_DEFAULT, "kmmOnlineJobId", m_kbanking->mappingId(job).toLatin1().constData());

  qDebug() << "Enqueue: " << m_kbanking->enqueueJob(abJob);
  //delete localAcc;
  return true;
}

bool KBankingPlugin::enqueTransaction(onlineJobTyped<sepaOnlineTransfer>& job)
{
  /* get AqBanking account */
  const QString accId = job.constTask()->responsibleAccount();

  AB_ACCOUNT *abAccount = aqbAccount(accId);
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

  AB_JOB *abJob = AB_JobSepaTransfer_new(abAccount);
  int rv = AB_Job_CheckAvailability(abJob);
  if (rv) {
    qDebug("AB_ERROR_OFFSET is %i", AB_ERROR_OFFSET);
    job.addJobMessage(onlineJobMessage(onlineJobMessage::error, "AqBanking",
                                       QString("Sepa credit transfers for account \"%1\" are not available, error code %2.").arg(MyMoneyFile::instance()->account(accId).name(), rv)
                                      )
                     );
    return false;
  }
  AB_TRANSACTION *AbTransaction = AB_Transaction_new();

  // Recipient
  payeeIdentifiers::ibanBic beneficiaryAcc = job.constTask()->beneficiaryTyped();
  AB_Transaction_SetRemoteName(AbTransaction, GWEN_StringList_fromQString(beneficiaryAcc.ownerName()));
  AB_Transaction_SetRemoteIban(AbTransaction, beneficiaryAcc.electronicIban().toUtf8().constData());
  AB_Transaction_SetRemoteBic(AbTransaction, beneficiaryAcc.fullStoredBic().toUtf8().constData());

  // Origin Account
  AB_Transaction_SetLocalAccount(AbTransaction, abAccount);

  // Purpose
  QStringList qPurpose = job.constTask()->purpose().split('\n');
  GWEN_STRINGLIST *purpose = GWEN_StringList_fromQStringList(qPurpose);
  AB_Transaction_SetPurpose(AbTransaction, purpose);
  GWEN_StringList_free(purpose);

  // Reference
  // AqBanking duplicates the string. This should be safe.
  AB_Transaction_SetEndToEndReference(AbTransaction, job.constTask()->endToEndReference().toUtf8().constData());

  // Other Fields
  AB_Transaction_SetTextKey(AbTransaction, job.constTask()->textKey());
  AB_Transaction_SetValue(AbTransaction, AB_Value_fromMyMoneyMoney(job.constTask()->value()));

  /** @todo LOW remove Debug info */
  qDebug() << "SetTransaction: " << AB_Job_SetTransaction(abJob, AbTransaction);

  GWEN_DB_NODE *gwenNode = AB_Job_GetAppData(abJob);
  GWEN_DB_SetCharValue(gwenNode, GWEN_DB_FLAGS_DEFAULT, "kmmOnlineJobId", m_kbanking->mappingId(job).toLatin1().constData());

  qDebug() << "Enqueue: " << m_kbanking->enqueueJob(abJob);

  //delete localAcc;
  return true;
}

void KBankingPlugin::startPasswordTimer(void)
{
  if (d->passwordCacheTimer->isActive())
    d->passwordCacheTimer->stop();
  d->passwordCacheTimer->start();
}

void KBankingPlugin::slotClearPasswordCache(void)
{
  m_kbanking->clearPasswordCache();
}

void KBankingPlugin::slotImport(void)
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
}

int KMyMoneyBanking::init()
{
  int rv;

  rv = AB_Banking::init();
  if (rv < 0)
    return rv;

  rv = onlineInit();
  if (rv) {
    fprintf(stderr, "Error on online init (%d).\n", rv);
    AB_Banking::fini();
    return rv;
  }

  _jobQueue = AB_Job_List2_new();

  return 0;
}

int KMyMoneyBanking::fini()
{
  int rv;

  if (_jobQueue) {
    AB_Job_List2_FreeAll(_jobQueue);
    _jobQueue = 0;
  }

  rv = onlineFini();
  if (rv) {
    AB_Banking::fini();
    return rv;
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
  AB_JOB_LIST2_ITERATOR* jobIter = AB_Job_List2_First(_jobQueue);
  if (jobIter) {
    AB_JOB* abJob = AB_Job_List2Iterator_Data(jobIter);

    while (abJob) {
      GWEN_DB_NODE* gwenNode = AB_Job_GetAppData(abJob);
      if (gwenNode == 0) {
        qWarning("Executed AB_Job without KMyMoney id");
        abJob = AB_Job_List2Iterator_Next(jobIter);
        break;
      }
      QString jobIdent = QString::fromUtf8(GWEN_DB_GetCharValue(gwenNode, "kmmOnlineJobId", 0, ""));

      onlineJob job = m_parent->m_onlineJobQueue.value(jobIdent);
      if (job.isNull()) {
        // It should not be possiblie that this will happen (only if AqBanking fails heavily).
        //! @todo correct exception text
        qWarning("Executed a job which was not in queue. Please inform the KMyMoney developers.");
        abJob = AB_Job_List2Iterator_Next(jobIter);
        break;
      }

      AB_JOB_STATUS abStatus = AB_Job_GetStatus(abJob);

      if (abStatus == AB_Job_StatusSent
          || abStatus == AB_Job_StatusPending
          || abStatus == AB_Job_StatusFinished
          || abStatus == AB_Job_StatusError
          || abStatus == AB_Job_StatusUnknown)
        job.setJobSend();

      if (abStatus == AB_Job_StatusFinished)
        job.setBankAnswer(onlineJob::acceptedByBank);
      else if (abStatus == AB_Job_StatusError || abStatus == AB_Job_StatusUnknown)
        job.setBankAnswer(onlineJob::sendingError);

      job.addJobMessage(onlineJobMessage(onlineJobMessage::debug, "KBanking", "Job was processed"));
      m_parent->m_onlineJobQueue.insert(jobIdent, job);
      abJob = AB_Job_List2Iterator_Next(jobIter);
    }
    AB_Job_List2Iterator_free(jobIter);
  }

  AB_JOB_LIST2 *oldQ = _jobQueue;
  _jobQueue = AB_Job_List2_new();
  AB_Job_List2_FreeAll(oldQ);

  emit m_parent->queueChanged();
  m_parent->startPasswordTimer();

  return rv;
}

void KMyMoneyBanking::clearPasswordCache(void)
{
  /* clear password DB */
  GWEN_Gui_SetPasswordStatus(NULL, NULL, GWEN_Gui_PasswordStatus_Remove, 0);
}

std::list<AB_JOB*> KMyMoneyBanking::getEnqueuedJobs()
{
  AB_JOB_LIST2 *ll;
  std::list<AB_JOB*> rl;

  ll = _jobQueue;
  if (ll && AB_Job_List2_GetSize(ll)) {
    AB_JOB *j;
    AB_JOB_LIST2_ITERATOR *it;

    it = AB_Job_List2_First(ll);
    assert(it);
    j = AB_Job_List2Iterator_Data(it);
    assert(j);
    while (j) {
      rl.push_back(j);
      j = AB_Job_List2Iterator_Next(it);
    }
    AB_Job_List2Iterator_free(it);
  }
  return rl;
}


int KMyMoneyBanking::enqueueJob(AB_JOB *j)
{
  assert(_jobQueue);
  assert(j);
  AB_Job_Attach(j);
  AB_Job_List2_PushBack(_jobQueue, j);
  return 0;
}


int KMyMoneyBanking::dequeueJob(AB_JOB *j)
{
  assert(_jobQueue);
  AB_Job_List2_Remove(_jobQueue, j);
  AB_Job_free(j);
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
    AB_ACCOUNT *a;

    a = w->getAccount();
    assert(a);
    DBG_NOTICE(0,
               "Mapping application account \"%s\" to "
               "online account \"%s/%s\"",
               qPrintable(acc.name()),
               AB_Account_GetBankCode(a),
               AB_Account_GetAccountNumber(a));

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
  dlg = AB_ImporterDialog_new(getCInterface(), ctx, NULL);
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


const AB_ACCOUNT_STATUS* KMyMoneyBanking::_getAccountStatus(AB_IMEXPORTER_ACCOUNTINFO *ai)
{
  const AB_ACCOUNT_STATUS *ast;
  const AB_ACCOUNT_STATUS *best;

  best = 0;
  ast = AB_ImExporterAccountInfo_GetFirstAccountStatus(ai);
  while (ast) {
    if (!best)
      best = ast;
    else {
      const GWEN_TIME *tiBest;
      const GWEN_TIME *ti;

      tiBest = AB_AccountStatus_GetTime(best);
      ti = AB_AccountStatus_GetTime(ast);

      if (!tiBest) {
        best = ast;
      } else {
        if (ti) {
          double d;

          /* we have two times, compare them */
          d = GWEN_Time_Diff(ti, tiBest);
          if (d > 0)
            /* newer */
            best = ast;
        }
      }
    }
    ast = AB_ImExporterAccountInfo_GetNextAccountStatus(ai);
  } /* while */
  return best;
}



void KMyMoneyBanking::_xaToStatement(MyMoneyStatement &ks,
                                     const MyMoneyAccount& acc,
                                     const AB_TRANSACTION *t)
{
  const GWEN_STRINGLIST *sl;
  QString s;
  const char *p;
  const AB_VALUE *val;
  const GWEN_TIME *ti;
  const GWEN_TIME *startTime = 0;
  MyMoneyStatement::Transaction kt;
  unsigned long h;

  kt.m_fees = MyMoneyMoney();

  // bank's transaction id
  p = AB_Transaction_GetFiId(t);
  if (p)
    kt.m_strBankID = QString("ID ") + QString::fromUtf8(p);

  // payee
  s.truncate(0);
  sl = AB_Transaction_GetRemoteName(t);
  if (sl) {
    GWEN_STRINGLISTENTRY *se;

    se = GWEN_StringList_FirstEntry(sl);
    while (se) {
      p = GWEN_StringListEntry_Data(se);
      assert(p);
      s += QString::fromUtf8(p);
      se = GWEN_StringListEntry_Next(se);
    } // while
  }
  kt.m_strPayee = s;
  h = MyMoneyTransaction::hash(s.trimmed());

  // memo
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
      se = GWEN_StringListEntry_Next(se);
    } // while
  }
  kt.m_strMemo = s;
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

  kt.m_strMemo = kt.m_strMemo;
  kt.m_strPayee = kt.m_strPayee.trimmed();

  // date
  ti = AB_Transaction_GetDate(t);
  if (!ti)
    ti = AB_Transaction_GetValutaDate(t);
  if (ti) {
    int year, month, day;

    if (!startTime)
      startTime = ti;
    else {
      if (GWEN_Time_Diff(ti, startTime) < 0)
        startTime = ti;
    }

    if (!GWEN_Time_GetBrokenDownDate(ti, &day, &month, &year)) {
      kt.m_datePosted = QDate(year, month + 1, day);
    }
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

  if (startTime) {
    int year, month, day;

    if (!GWEN_Time_GetBrokenDownDate(startTime, &day, &month, &year)) {
      QDate d(year, month + 1, day);

      if (!ks.m_dateBegin.isValid())
        ks.m_dateBegin = d;
      else if (d < ks.m_dateBegin)
        ks.m_dateBegin = d;

      if (!ks.m_dateEnd.isValid())
        ks.m_dateEnd = d;
      else if (d > ks.m_dateEnd)
        ks.m_dateEnd = d;
    }
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
  QString s;
  const char *p;
  const AB_TRANSACTION *t;
  MyMoneyStatement ks;
  MyMoneyAccount kacc;
  const AB_ACCOUNT_STATUS *ast;
  const AB_VALUE *val;
  const GWEN_TIME *ti;

  DBG_INFO(0, "Importing account...");

  // account number
  p = AB_ImExporterAccountInfo_GetAccountNumber(ai);
  if (p) {
    ks.m_strAccountNumber = m_parent->stripLeadingZeroes(p);
  }

  p = AB_ImExporterAccountInfo_GetBankCode(ai);
  if (p) {
    ks.m_strRoutingNumber = m_parent->stripLeadingZeroes(p);
  }

  kacc = m_parent->account("kbanking-acc-ref", QString("%1-%2").arg(ks.m_strRoutingNumber, ks.m_strAccountNumber));
  ks.m_accountId = kacc.id();

  // account name
  p = AB_ImExporterAccountInfo_GetAccountName(ai);
  if (p)
    ks.m_strAccountName = p;

  // account type
  switch (AB_ImExporterAccountInfo_GetType(ai)) {
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
  ast = _getAccountStatus(ai);
  if (ast) {
    const AB_BALANCE *bal;

    bal = AB_AccountStatus_GetBookedBalance(ast);
    if (!bal)
      bal = AB_AccountStatus_GetNotedBalance(ast);
    if (bal) {
      val = AB_Balance_GetValue(bal);
      if (val) {
        DBG_INFO(0, "Importing balance");
        ks.m_closingBalance = MyMoneyMoney(AB_Value_GetValueAsDouble(val));
        p = AB_Value_GetCurrency(val);
        if (p)
          ks.m_strCurrency = p;
      }
      ti = AB_Balance_GetTime(bal);
      if (ti) {
        int year, month, day;

        if (!GWEN_Time_GetBrokenDownDate(ti, &day, &month, &year))
          ks.m_dateEnd = QDate(year, month + 1, day);
      } else {
        DBG_WARN(0, "No time for balance");
      }
    } else {
      DBG_WARN(0, "No account balance");
    }
  } else {
    DBG_WARN(0, "No account status");
  }

  // clear hash map
  m_hashMap.clear();

  // get all transactions
  t = AB_ImExporterAccountInfo_GetFirstTransaction(ai);
  while (t) {
    _xaToStatement(ks, kacc, t);
    t = AB_ImExporterAccountInfo_GetNextTransaction(ai);
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
