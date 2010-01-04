/***************************************************************************
 *   Copyright 2009  Cristian Onet onet.cristian@gmail.com                 *
 *   Copyright 2004  Martin Preuss aquamaniac@users.sourceforge.net        *
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

#include <qmessagebox.h>
#include <qlayout.h>
#include <qradiobutton.h>
#include <qstringlist.h>
#include <qregexp.h>
#include <qcheckbox.h>
#include <qlabel.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>
#include <kmessagebox.h>
#include <kgenericfactory.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kmenu.h>
#include <kiconloader.h>
#include <kguiitem.h>
#include <klineedit.h>
#include <keditlistbox.h>
#include <kcombobox.h>
#include <kconfig.h>

// ----------------------------------------------------------------------------
// Library Includes

#include <aqbanking/imexporter.h>
#include <aqbanking/jobgettransactions.h>
#include <aqbanking/jobgetbalance.h>
#include <aqbanking/job.h>
// #include <qbanking/qbpickstartdate.h>
#include <qbanking/qbgui.h>
#include <gwenhywfar/logger.h>
#include <gwenhywfar/debug.h>
//#include <kbanking/settings.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kbjobview.h"
#include "kbsettings.h"
#include "kbaccountsettings.h"
#include <kmymoney/mymoneyfile.h>
#include <kmymoney/kmymoneyview.h>
#include <kmymoney/mymoneykeyvaluecontainer.h>
#include <kbpickstartdate.h>

K_EXPORT_COMPONENT_FACTORY( kmm_kbanking,
                            KGenericFactory<KBankingPlugin>( "kmm_kbanking" ) )

class KBankingPlugin::Private
{
public:
  Private() {
    QString gwenProxy = QString::fromLocal8Bit(qgetenv("GWEN_PROXY"));
    if(gwenProxy.isEmpty()) {
      KConfig *cfg = new KConfig("kioslaverc");
      QRegExp exp("(\\w+://)?(.*)");
      QString proxy;

      KConfigGroup grp = cfg->group( "Proxy Settings" );
      int type = grp.readEntry("ProxyType", 0);
      switch(type) {
        case 0: // no proxy
          break;

        case 1: // manual specified
          proxy = grp.readEntry("httpsProxy");
          qDebug("KDE https proxy setting is '%s'", qPrintable(proxy));
          if(exp.exactMatch(proxy) != -1) {
            proxy = exp.cap(2);
            qDebug("Setting GWEN_PROXY to '%s'", qPrintable(proxy));
            if(setenv("GWEN_PROXY", qPrintable(proxy), 1) == -1) {
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
};

KBankingPlugin::KBankingPlugin(QObject *parent, const QStringList&) :
  KMyMoneyPlugin::Plugin(parent, "KBanking"/*must be the same as X-KDE-PluginInfo-Name*/),
  KMyMoneyPlugin::OnlinePlugin(),
  d(new Private),
  m_accountSettings(0)
{
  m_kbanking=new KMyMoneyBanking(this, "KMyMoney");

  if (m_kbanking) {
    QBGui *gui;

#if AQB_IS_VERSION(3,9,0,0)
    if(AB_Banking_HasConf4(m_kbanking->getCInterface(), 0)) {
      qDebug("KBankingPlugin: No AqB4 config found.");
      if(AB_Banking_HasConf3(m_kbanking->getCInterface(), 0)) {
        qDebug("KBankingPlugin: No AqB3 config found.");
        if(!AB_Banking_HasConf2(m_kbanking->getCInterface(), 0)) {
          qDebug("KBankingPlugin: AqB2 config found - converting.");
          AB_Banking_ImportConf2(m_kbanking->getCInterface(), 0);
        }
      } else {
        qDebug("KBankingPlugin: AqB3 config found - converting.");
        AB_Banking_ImportConf3(m_kbanking->getCInterface(), 0);
      }
    }
#endif

    gui=new QBGui(m_kbanking);
    GWEN_Gui_SetGui(gui->getCInterface());
    GWEN_Logger_SetLevel(0, GWEN_LoggerLevel_Info);
    GWEN_Logger_SetLevel(AQBANKING_LOGDOMAIN, GWEN_LoggerLevel_Debug);
    m_kbanking->setGui(gui);
    if (m_kbanking->init() == 0) {
      // Tell the host application to load my GUI component
      setComponentData(KGenericFactory<KBankingPlugin>::componentData());
      setXMLFile("kmm_kbanking.rc");
      qDebug("KMyMoney kbanking plugin loaded");

      // create view
      createJobView();

      // create actions
      createActions();

      // load protocol conversion list
      loadProtocolConversion();
    }
    else {
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
  if(m_kbanking) {
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
  if(m_kbanking) {
    std::list<std::string> list = m_kbanking->getActiveProviders();
    std::list<std::string>::iterator it;
    for(it = list.begin(); it != list.end(); ++it) {
      // skip the dummy
      if(*it == "aqnone")
        continue;
      QMap<QString, QString>::const_iterator it_m;
      it_m = m_protocolConversionMap.find((*it).c_str());
      if(it_m != m_protocolConversionMap.end())
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
  if(m_kbanking) {
    m_accountSettings = new KBAccountSettings(acc, 0);
    m_accountSettings->m_usePayeeAsIsButton->setChecked(true);
    m_accountSettings->m_transactionDownload->setChecked(kvp.value("kbanking-txn-download")!="no");
    m_accountSettings->m_preferredJobMethod->setCurrentItem(kvp.value("kbanking-jobexec"));
    m_accountSettings->m_preferredStatementDate->setCurrentItem(kvp.value("kbanking-statementDate"));
    if(!kvp.value("kbanking-payee-regexp").isEmpty()) {
      m_accountSettings->m_extractPayeeButton->setChecked(true);
      m_accountSettings->m_payeeRegExpEdit->setText(kvp.value("kbanking-payee-regexp"));
      m_accountSettings->m_memoRegExpEdit->setText(kvp.value("kbanking-memo-regexp"));
      m_accountSettings->m_payeeExceptions->clear();
      m_accountSettings->m_payeeExceptions->insertStringList(kvp.value("kbanking-payee-exceptions").split(';', QString::SkipEmptyParts));
    }
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
  if(m_accountSettings) {
    kvp.deletePair("kbanking-payee-regexp");
    kvp.deletePair("kbanking-memo-regexp");
    kvp.deletePair("kbanking-payee-exceptions");
    kvp.deletePair("kbanking-txn-download");
    if(m_accountSettings->m_extractPayeeButton->isChecked()
    && !m_accountSettings->m_payeeRegExpEdit->text().isEmpty()
    && !m_accountSettings->m_memoRegExpEdit->text().isEmpty()) {
      kvp["kbanking-payee-regexp"] = m_accountSettings->m_payeeRegExpEdit->text();
      kvp["kbanking-memo-regexp"] = m_accountSettings->m_memoRegExpEdit->text();
      kvp["kbanking-payee-exceptions"] = m_accountSettings->m_payeeExceptions->items().join(";");
    } else if(m_accountSettings->m_extractPayeeButton->isChecked()) {
      KMessageBox::information(0, i18n("You selected to extract the payee from the memo field but did not supply a regular expression for payee and memo extraction. The option will not be activated."), i18n("Missing information"));
    }
    if(!m_accountSettings->m_transactionDownload->isChecked())
      kvp["kbanking-txn-download"] = "no";
    kvp["kbanking-jobexec"] = QString("%1").arg(m_accountSettings->m_preferredJobMethod->currentIndex());
    kvp["kbanking-statementDate"] = QString("%1").arg(m_accountSettings->m_preferredStatementDate->currentIndex());
  }
  return kvp;
}

void KBankingPlugin::createJobView(void)
{
  KMyMoneyViewBase* view = viewInterface()->addPage(i18nc("Label for icon in KMyMoney's view pane", "Outbox"), "onlinebanking");
  QWidget* frm = dynamic_cast<QWidget*>(view->parent());
  QWidget* w = new KBJobView(m_kbanking, view, "JobView");
  viewInterface()->addWidget(view, w);
  connect(viewInterface(), SIGNAL(viewStateChanged(bool)), frm, SLOT(setEnabled(bool)));
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
  QPointer<KBankingSettings> bs = new KBankingSettings(m_kbanking);
  if (bs->init())
    qWarning("Error on ini of settings dialog.");
  else {
    bs->exec();
    if (bs && bs->fini())
      qWarning("Error on fini of settings dialog.");
  }
  delete bs;
}

bool KBankingPlugin::mapAccount(const MyMoneyAccount& acc, MyMoneyKeyValueContainer& settings)
{
  bool rc = false;
  if(m_kbanking && !acc.id().isEmpty()) {
    MyMoneyFile* file = MyMoneyFile::instance();

    QString bankId;
    QString accountId;
    // extract some information about the bank. if we have a sortcode
    // (BLZ) we display it, otherwise the name is enough.
    try {
      const MyMoneyInstitution &bank = file->institution(acc.institutionId());
      bankId = bank.name();
      if(!bank.sortcode().isEmpty())
        bankId = bank.sortcode();
    } catch(MyMoneyException *e) {
      // no bank assigned, we just leave the field emtpy
      delete e;
    }

    // extract account information. if we have an account number
    // we show it, otherwise the name will be displayed
    accountId = acc.number();
    if(accountId.isEmpty())
      accountId = m_account.name();

    // do the mapping. the return value of this method is either
    // true, when the user mapped the account or false, if he
    // decided to quit the dialog. So not really a great thing
    // to present some more information.
    m_kbanking->askMapAccount(acc.id().toUtf8().data(),
                              bankId.toUtf8().data(),
                              accountId.toUtf8().data());

    // at this point, the account should be mapped
    // so we search it and setup the account reference in the KMyMoney object
    AB_ACCOUNT* ab_acc;
    ab_acc = AB_BANKING_GETACCOUNTBYALIAS(m_kbanking->getCInterface(), acc.id().toUtf8().data());
    if(ab_acc) {
      MyMoneyAccount a(acc);
      setupAccountReference(a, ab_acc);
      settings = a.onlineBankingSettings();
      rc = true;
    }
  }
  return rc;
}

QString KBankingPlugin::stripLeadingZeroes(const QString& s) const
{
  QString rc(s);
  QRegExp exp("(0*)(.*)");
  if(exp.exactMatch(s) != -1) {
    rc = exp.cap(2);
  }
  return rc;
}

void KBankingPlugin::setupAccountReference(const MyMoneyAccount& acc, AB_ACCOUNT* ab_acc)
{
  MyMoneyKeyValueContainer kvp;

  if(ab_acc) {
    QString accountNumber = stripLeadingZeroes(AB_Account_GetAccountNumber(ab_acc));
    QString routingNumber = stripLeadingZeroes(AB_Account_GetBankCode(ab_acc));

    QString val = QString("%1-%2").arg(routingNumber, accountNumber);
    if(val != acc.onlineBankingSettings().value("kbanking-acc-ref")) {
      MyMoneyKeyValueContainer kvp;

      // make sure to keep our own previous settings
      const QMap<QString, QString>& vals = acc.onlineBankingSettings().pairs();
      QMap<QString, QString>::const_iterator it_p;
      for(it_p = vals.begin(); it_p != vals.end(); ++it_p) {
        if(QString(it_p.key()).startsWith("kbanking-")) {
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
  AB_ACCOUNT* ab_acc = 0;
  if(m_kbanking)
    ab_acc = AB_BANKING_GETACCOUNTBYALIAS(m_kbanking->getCInterface(), acc.id().toUtf8().data());
  return ab_acc != 0;
}

bool KBankingPlugin::updateAccount(const MyMoneyAccount& acc)
{
  return updateAccount(acc, false);
}

bool KBankingPlugin::updateAccount(const MyMoneyAccount& acc, bool moreAccounts)
{
  if(!m_kbanking)
    return false;

  bool rc = false;

  if (!acc.id().isEmpty()) {
    AB_ACCOUNT *ba = 0;
    AB_JOB *job = 0;
    int rv;
    int days;
    int year, month, day;
    QDate qd;

    /* get AqBanking account */
    ba=AB_BANKING_GETACCOUNTBYALIAS(m_kbanking->getCInterface(), acc.id().toUtf8().data());
    if (!ba) {
      QMessageBox::critical(0,
                            i18n("Account Not Mapped"),
                            i18n("<qt>"
                                 "<p>"
                                 "The given application account <b>%1</b> "
                                 "has not been mapped to an online "
                                 "account."
                                 "</p>"
                                 "</qt>",
                                 acc.name()),
                            QMessageBox::Ok,Qt::NoButton);
      // clear the connection between the KMyMoney account
      // and the AqBanking equivalent
      setupAccountReference(acc, 0);
    }

    if(ba) {
      setupAccountReference(acc, ba);

      if(acc.onlineBankingSettings().value("kbanking-txn-download") != "no") {
        /* create getTransactions job */
        job = AB_JobGetTransactions_new(ba);
        rv = AB_Job_CheckAvailability(job, 0);
        if (rv) {
          DBG_ERROR(0, "Job \"GetTransactions\" is not available (%d)", rv);
          QMessageBox::critical(0,
                                i18n("Job not Available"),
                                i18n("<qt>"
                                    "The update job is not supported by the "
                                    "bank/account/backend.\n"
                                    "</qt>"),
                                i18n("Dismiss"), QString());
          AB_Job_free(job);
          job = 0;
        }

        if(job) {
          days = AB_JobGetTransactions_GetMaxStoreDays(job);
          if (days > 0) {
            GWEN_TIME *ti1;
            GWEN_TIME *ti2;

            ti1=GWEN_CurrentTime();
            ti2=GWEN_Time_fromSeconds(GWEN_Time_Seconds(ti1)-(60*60*24*days));
            GWEN_Time_free(ti1);
            ti1=ti2;

            if (GWEN_Time_GetBrokenDownDate(ti1, &day, &month, &year)) {
              DBG_ERROR(0, "Bad date");
              qd=QDate();
            } else
              qd=QDate(year, month+1, day);
            GWEN_Time_free(ti1);
          }

          // get last statement request date from application account object
          // and start from the next day if the date is valid
          QDate lastUpdate = QDate::fromString(acc.value("lastImportedTransactionDate"), Qt::ISODate);
          if(lastUpdate.isValid())
            lastUpdate = lastUpdate.addDays(-3);

          int dateOption = acc.onlineBankingSettings().value("kbanking-statementDate").toInt();
          switch(dateOption) {
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
          if(dateOption == 0
          || (dateOption > 1 && !qd.isValid())) {
            QPointer<KBPickStartDate> psd = new KBPickStartDate(m_kbanking, qd, lastUpdate, acc.name(),
                                                                lastUpdate.isValid() ? 2 : 3, 0, true);
            if (psd->exec() != QDialog::Accepted) {
              AB_Job_free(job);
              delete psd;
              return rc;
            }
            qd=psd->date();
            delete psd;
          }

          if (qd.isValid()) {
            GWEN_TIME *ti1;

            ti1=GWEN_Time_new(qd.year(), qd.month()-1, qd.day(), 0, 0, 0, 0);
            AB_JobGetTransactions_SetFromTime(job, ti1);
            GWEN_Time_free(ti1);
          }

          rv=m_kbanking->enqueueJob(job);
          AB_Job_free(job);
          if (rv) {
            DBG_ERROR(0, "Error %d", rv);
            QMessageBox::critical(0,
                                  i18n("Error"),
                                  i18n("<qt>"
                                      "Could not enqueue the job.\n"
                                      "</qt>"),
                                  i18n("Dismiss"), QString());
          }
        }
      }

      /* create getBalance job */
      job = AB_JobGetBalance_new(ba);
      rv = AB_Job_CheckAvailability(job, 0);
      if(!rv)
        rv = m_kbanking->enqueueJob(job);
      else
        rv = 0;

      AB_Job_free(job);
      if (rv) {
        DBG_ERROR(0, "Error %d", rv);
        QMessageBox::critical(0,
                              i18n("Error"),
                              i18n("<qt>"
                                  "Could not enqueue the job.\n"
                                  "</qt>"),
                              i18n("Dismiss"), QString());
      }
    }

    // make sure, we have at least one job in the queue before we continue.
    if (m_kbanking->getEnqueuedJobs().size() > 0) {

      // ask if the user want's to execute this job right away or spool it
      // for later execution
      KIconLoader *ic = KIconLoader::global();
      KGuiItem executeButton(i18n("&Execute"),
                            KIcon(ic->loadIcon("tools-wizard",
                              KIconLoader::Small, KIconLoader::SizeSmall)),
                            i18n("Close this window"),
                            i18n("Use this button to close the window"));

      KGuiItem queueButton(i18n("&Queue"),
                            KIcon(ic->loadIcon("document-export",
                              KIconLoader::Small, KIconLoader::SizeSmall)),
                            i18n("Close this window"),
                            i18n("Use this button to close the window"));

      KMessageBox::ButtonCode result = KMessageBox::Cancel;
      if(!moreAccounts) {
        switch(acc.onlineBankingSettings().value("kbanking-jobexec").toInt()) {
          case 1:
            result = KMessageBox::Yes;
            break;
          case 2:
            result = KMessageBox::No;
            break;
          default:
            result = static_cast<KMessageBox::ButtonCode>(KMessageBox::questionYesNo(0,
              i18n("Do you want to execute or queue this job in the outbox?"),
              i18n("Execution"), executeButton, queueButton));
            break;
        }
      } else {
        result = KMessageBox::No;
      }


      if(result == KMessageBox::Yes) {

        AB_IMEXPORTER_CONTEXT *ctx;

        ctx=AB_ImExporterContext_new();
        rv=m_kbanking->executeQueue(ctx);
        if (!rv)
          m_kbanking->importContext(ctx, 0);
        else {
          DBG_ERROR(0, "Error: %d", rv);
        }
        AB_ImExporterContext_free(ctx);

        // let application emit signals to inform views
        m_kbanking->accountsUpdated();
      }
      rc = true;
    }
  }
  return rc;
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
:KBanking(appname, fname)
,m_parent(parent)
{
}



const AB_ACCOUNT_STATUS* KMyMoneyBanking::_getAccountStatus(AB_IMEXPORTER_ACCOUNTINFO *ai)
{
  const AB_ACCOUNT_STATUS *ast;
  const AB_ACCOUNT_STATUS *best;

  best=0;
  ast=AB_ImExporterAccountInfo_GetFirstAccountStatus(ai);
  while(ast) {
    if (!best)
      best=ast;
    else {
      const GWEN_TIME *tiBest;
      const GWEN_TIME *ti;

      tiBest=AB_AccountStatus_GetTime(best);
      ti=AB_AccountStatus_GetTime(ast);

      if (!tiBest) {
        best=ast;
      }
      else {
        if (ti) {
          double d;

          /* we have two times, compare them */
          d=GWEN_Time_Diff(ti, tiBest);
          if (d>0)
            /* newer */
            best=ast;
        }
      }
    }
    ast=AB_ImExporterAccountInfo_GetNextAccountStatus(ai);
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
  const GWEN_TIME *startTime=0;
  MyMoneyStatement::Transaction kt;
  unsigned long h;

  kt.m_fees = MyMoneyMoney();

  // bank's transaction id
  p=AB_Transaction_GetFiId(t);
  if (p)
    kt.m_strBankID=QString("ID ")+QString::fromUtf8(p);

  // payee
  s.truncate(0);
  sl=AB_Transaction_GetRemoteName(t);
  if (sl) {
    GWEN_STRINGLISTENTRY *se;

    se=GWEN_StringList_FirstEntry(sl);
    if (se) {
      p=GWEN_StringListEntry_Data(se);
      assert(p);
      s = QString::fromUtf8(p);
    }
  }
  kt.m_strPayee=s;
  h = MyMoneyTransaction::hash(s.trimmed());

  // memo
  s.truncate(0);
  sl=AB_Transaction_GetPurpose(t);
  if (sl) {
    GWEN_STRINGLISTENTRY *se;
    bool insertSpace = false;

    se=GWEN_StringList_FirstEntry(sl);
    while (se) {
      p = GWEN_StringListEntry_Data(se);
      assert(p);
      if (insertSpace)
        s += ' ';
      insertSpace = true;
      s += QString::fromUtf8(p);
      se = GWEN_StringListEntry_Next(se);
    } // while
  }
  kt.m_strMemo = s;
  h = MyMoneyTransaction::hash(s.trimmed(), h);

  // see, if we need to extract the payee from the memo field
  const MyMoneyKeyValueContainer& kvp = acc.onlineBankingSettings();
  QString rePayee = kvp.value("kbanking-payee-regexp");
  if(!rePayee.isEmpty() && kt.m_strPayee.isEmpty()) {
    QString reMemo = kvp.value("kbanking-memo-regexp");
    QStringList exceptions = kvp.value("kbanking-payee-exceptions").split(';', QString::SkipEmptyParts);

    bool needExtract = true;
    QStringList::const_iterator it_s;
    for(it_s = exceptions.constBegin(); needExtract && it_s != exceptions.constEnd(); ++it_s) {
      QRegExp exp(*it_s, Qt::CaseInsensitive);
      if(exp.indexIn(kt.m_strMemo) != -1) {
        needExtract = false;
      }
    }
    if(needExtract) {
      QRegExp expPayee(rePayee, Qt::CaseInsensitive);
      QRegExp expMemo(reMemo, Qt::CaseInsensitive);
      if(expPayee.indexIn(kt.m_strMemo) != -1) {
        kt.m_strPayee = expPayee.cap(1);
        if(expMemo.indexIn(kt.m_strMemo) != -1) {
          kt.m_strMemo = expMemo.cap(1);
        }
      }
    }
  }

  // massage whitespaces a bit:
  // - remove leading blanks
  // - remove trailing blanks
  // - reduce multiple blanks to one
  kt.m_strMemo = kt.m_strMemo.trimmed();
  kt.m_strPayee = kt.m_strPayee.trimmed();

  // date
  ti=AB_Transaction_GetDate(t);
  if (!ti)
    ti=AB_Transaction_GetValutaDate(t);
  if (ti) {
    int year, month, day;

    if (!startTime)
      startTime=ti;
    else {
      if (GWEN_Time_Diff(ti, startTime)<0)
        startTime=ti;
    }

    if (!GWEN_Time_GetBrokenDownDate(ti, &day, &month, &year)) {
      kt.m_datePosted=QDate(year, month+1, day);
    }
  } else {
    DBG_WARN(0, "No date for transaction");
  }

  // value
  val=AB_Transaction_GetValue(t);
  if (val) {
    if (ks.m_strCurrency.isEmpty()) {
      p=AB_Value_GetCurrency(val);
      if (p)
        ks.m_strCurrency=p;
    }
    else {
      p=AB_Value_GetCurrency(val);
      if (p)
        s=p;
      if (ks.m_strCurrency.toLower()!=s.toLower()) {
        // TODO: handle currency difference
        DBG_ERROR(0, "Mixed currencies currently not allowed");
      }
    }

    kt.m_amount=MyMoneyMoney(AB_Value_GetValueAsDouble(val));
    h = MyMoneyTransaction::hash(kt.m_amount.toString(), h);
  }
  else {
    DBG_WARN(0, "No value for transaction");
  }

  if (startTime) {
    int year, month, day;

    if (!GWEN_Time_GetBrokenDownDate(startTime, &day, &month, &year)) {
      QDate d(year, month+1, day);

      if (!ks.m_dateBegin.isValid())
        ks.m_dateBegin=d;
      else if (d<ks.m_dateBegin)
        ks.m_dateBegin=d;

      if (!ks.m_dateEnd.isValid())
        ks.m_dateEnd=d;
      else if (d>ks.m_dateEnd)
        ks.m_dateEnd=d;
    }
  }
  else {
    DBG_WARN(0, "No date in current transaction");
  }

  // make hash value unique in case we don't have one already
  if(kt.m_strBankID.isEmpty()) {
    QString hashBase;
    hashBase.sprintf("%s-%07lx", qPrintable(kt.m_datePosted.toString(Qt::ISODate)), h);
    int idx = 1;
    QString hash;
    for(;;) {
      hash = QString("%1-%2").arg(hashBase).arg(idx);
      QMap<QString, bool>::const_iterator it;
      it = m_hashMap.constFind(hash);
      if(it == m_hashMap.constEnd()) {
        m_hashMap[hash] = true;
        break;
      }
      ++idx;
    }
    kt.m_strBankID = QString("%1-%2").arg(acc.id()).arg(hash);
  }

  // store transaction
  ks.m_listTransactions+=kt;
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
  p=AB_ImExporterAccountInfo_GetAccountNumber(ai);
  if (p) {
    ks.m_strAccountNumber = m_parent->stripLeadingZeroes(p);
  }

  p=AB_ImExporterAccountInfo_GetBankCode(ai);
  if(p) {
    ks.m_strRoutingNumber = m_parent->stripLeadingZeroes(p);
  }

  kacc = m_parent->account("kbanking-acc-ref", QString("%1-%2").arg(ks.m_strRoutingNumber, ks.m_strAccountNumber));
  ks.m_accountId = kacc.id();

  // account name
  p=AB_ImExporterAccountInfo_GetAccountName(ai);
  if (p)
    ks.m_strAccountName=p;

  // account type
  switch(AB_ImExporterAccountInfo_GetType(ai)) {
    case AB_AccountType_Bank:
      ks.m_eType=MyMoneyStatement::etSavings;
      break;
    case AB_AccountType_CreditCard:
      ks.m_eType=MyMoneyStatement::etCreditCard;
      break;
    case AB_AccountType_Checking:
      ks.m_eType=MyMoneyStatement::etCheckings;
      break;
    case AB_AccountType_Savings:
      ks.m_eType=MyMoneyStatement::etSavings;
      break;
    case AB_AccountType_Investment:
      ks.m_eType=MyMoneyStatement::etInvestment;
      break;
    case AB_AccountType_Cash:
      ks.m_eType=MyMoneyStatement::etNone;
      break;
    default:
      ks.m_eType=MyMoneyStatement::etNone;
  }

  // account status
  ast=_getAccountStatus(ai);
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
          ks.m_dateEnd=QDate(year, month+1, day);
      }
      else {
        DBG_WARN(0, "No time for balance");
      }
    }
    else {
      DBG_WARN(0, "No account balance");
    }
  }
  else {
    DBG_WARN(0, "No account status");
  }

  // clear hash map
  m_hashMap.clear();

  // get all transactions
  t=AB_ImExporterAccountInfo_GetFirstTransaction(ai);
  while(t) {
    _xaToStatement(ks, kacc, t);
    t=AB_ImExporterAccountInfo_GetNextTransaction(ai);
  }

  // import them
  if (!m_parent->importStatement(ks)) {
    if (QMessageBox::critical(0,
                              i18n("Critical Error"),
                              i18n("Error importing statement."),
                              i18n("Continue"),
                              i18n("Abort"), 0, 0)!=0) {
      DBG_ERROR(0, "User aborted");
      return false;
    }
  }
  return true;
}


#include "mymoneybanking.moc"
