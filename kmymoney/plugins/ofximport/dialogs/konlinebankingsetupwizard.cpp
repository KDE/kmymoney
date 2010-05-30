/***************************************************************************
                          konlinebankingsetupwizard.cpp
                             -------------------
    begin                : Sat Jan 7 2006
    copyright            : (C) 2006 by Ace Jones
    email                : acejones@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <config-kmymoney.h>

#include "konlinebankingsetupwizard.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QLabel>
#include <QLayout>
#include <QRegExp>
#include <QCheckBox>
#include <QTabWidget>
#include <QTextStream>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kprogressdialog.h>
#include <kapplication.h>
#include <k3listview.h>
#include <k3listviewsearchline.h>
#include <kcombobox.h>
#include <kurlrequester.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <../ofxpartner.h>
#include <mymoneyofxconnector.h>

class KOnlineBankingSetupWizard::Private
{
public:
  QFile       m_fpTrace;
  QTextStream m_trace;
};

KOnlineBankingSetupWizard::KOnlineBankingSetupWizard(QWidget *parent):
    KOnlineBankingSetupDecl(parent),
    d(new Private),
    m_fDone(false),
    m_fInit(false),
    m_appId(0)
{
  m_appId = new OfxAppVersion(m_applicationCombo, "");
  m_headerVersion = new OfxHeaderVersion(m_headerVersionCombo, "");

  // fill the list view with banks
  KProgressDialog* dlg = new KProgressDialog(this, i18n("Loading banklist"), i18n("Getting list of banks from http://moneycentral.msn.com/\nThis may take some time depending on the available bandwidth."));
  dlg->setModal(true);
  dlg->setAllowCancel(false);
  // force to show immediately as the call to OfxPartner::BankNames()
  // does not call the processEvents() loop
  dlg->setMinimumDuration(0);
  kapp->processEvents();

  //set password field according to KDE preferences
  m_editPassword->setPasswordMode(true);

  vboxLayout1->insertWidget(0, new K3ListViewSearchLineWidget(m_listFi, autoTab));
  OfxPartner::setDirectory(KStandardDirs::locateLocal("appdata", ""));
  QStringList banks = OfxPartner::BankNames();
  QStringList::const_iterator it_bank = banks.constBegin();
  while (it_bank != banks.constEnd()) {
    new K3ListViewItem(m_listFi, (*it_bank));
    ++it_bank;
  }
  m_fInit = true;
  delete dlg;
}

KOnlineBankingSetupWizard::~KOnlineBankingSetupWizard()
{
  delete m_appId;
  delete d;
}

void KOnlineBankingSetupWizard::next(void)
{
  bool ok = true;

  switch (indexOf(currentPage())) {
    case 0:
      ok = finishFiPage();
      break;
    case 1:
      ok = finishLoginPage();
      break;
    case 2:
      m_fDone = ok = finishAccountPage();
      break;
  }

  if (ok)
    KOnlineBankingSetupDecl::next();

  setFinishEnabled(currentPage(), m_fDone);
}

bool KOnlineBankingSetupWizard::finishFiPage(void)
{
  bool result = false;

  m_bankInfo.clear();
  OfxFiServiceInfo info;

  if (m_selectionTab->currentIndex() == 0) {

    // Get the fipids for the selected bank
    Q3ListViewItem* item = m_listFi->currentItem();
    if (item) {
      QString bank = item->text(0);
      m_textDetails->clear();
      m_textDetails->append(QString("<p>Details for %1:</p>").arg(bank));
      QStringList fipids = OfxPartner::FipidForBank(bank);
      QStringList::const_iterator it_fipid = fipids.constBegin();
      while (it_fipid != fipids.constEnd()) {
        // For each fipid, get the connection details
        info = OfxPartner::ServiceInfo(*it_fipid);

        // Print them to the text browser
        QString message = QString("<p>Fipid: %1<br/>").arg(*it_fipid);

        // If the bank supports retrieving statements
        if (info.accountlist) {
          m_bankInfo.push_back(info);

          message += QString("URL: %1<br/>Org: %2<br/>Fid: %3<br/>").arg(info.url, info.org, info.fid);
          if (info.statements)
            message += i18n("Supports online statements<br/>");
          if (info.investments)
            message += i18n("Supports investments<br/>");
          if (info.billpay)
            message += i18n("Supports bill payment (but not supported by KMyMoney yet)<br/>");
        } else {
          message += i18n("Does not support online banking");
        }
        message += "</p>";
        m_textDetails->append(message);

        ++it_fipid;
      }
      result = true;
    } else
      // error!  No current item
      KMessageBox::sorry(this, i18n("Please choose a bank."));

  } else {  // manual entry of values
    if (m_fid->text().isEmpty()
        || m_url->url().isEmpty()
        || m_bankName->text().isEmpty()) {
      KMessageBox::sorry(this, i18n("Please fill all fields with values."));
    }

    m_textDetails->clear();
    m_textDetails->append(i18n("<p>Details for %1:</p>", m_bankName->text()));

    memset(&info, 0, sizeof(OfxFiServiceInfo));
    strncpy(info.fid, m_fid->text().toLatin1(), OFX_FID_LENGTH - 1);
    strncpy(info.org, m_bankName->text().toLatin1(), OFX_ORG_LENGTH - 1);
    strncpy(info.url, m_url->url().path().toLatin1(), OFX_URL_LENGTH - 1);
    info.accountlist = 1;
    info.statements = 1;
    info.billpay = 1;
    info.investments = 1;

    m_bankInfo.push_back(info);

    QString message;
    message += QString("URL: %1<br/>Org: %2<br/>Fid: %3<br/>").arg(info.url, info.org, info.fid);
    if (info.statements)
      message += i18n("Supports online statements<br/>");
    if (info.investments)
      message += i18n("Supports investments<br/>");
    if (info.billpay)
      message += i18n("Supports bill payment (but not supported by KMyMoney yet)<br/>");
    m_textDetails->append(message);
    result = true;
  }
  // make sure to display the beginning of the collected information
  m_textDetails->moveCursor(QTextCursor::Start);

  return result;
}

bool KOnlineBankingSetupWizard::finishLoginPage(void)
{
  bool result = true;

  QString username = m_editUsername->text();
  QString password = m_editPassword->text();

  m_listAccount->clear();

  // Process an account request for each fipid
  m_it_info = m_bankInfo.constBegin();
  while (m_it_info != m_bankInfo.constEnd()) {
    OfxFiLogin fi;
    memset(&fi, 0, sizeof(OfxFiLogin));
    strncpy(fi.fid, (*m_it_info).fid, OFX_FID_LENGTH - 1);
    strncpy(fi.org, (*m_it_info).org, OFX_ORG_LENGTH - 1);
    strncpy(fi.userid, username.toLatin1(), OFX_USERID_LENGTH - 1);
    strncpy(fi.userpass, password.toLatin1(), OFX_USERPASS_LENGTH - 1);

#if LIBOFX_IS_VERSION(0,9,0)
    // pretend we're Quicken 2008
    // http://ofxblog.wordpress.com/2007/06/06/ofx-appid-and-appver-for-intuit-products/
    // http://ofxblog.wordpress.com/2007/06/06/ofx-appid-and-appver-for-microsoft-money/
    QString appId = m_appId->appId();
    QRegExp exp("(.*):(.*)");
    if (exp.indexIn(appId) != -1) {
      strncpy(fi.appid, exp.cap(1).toLatin1(), OFX_APPID_LENGTH - 1);
      strncpy(fi.appver, exp.cap(2).toLatin1(), OFX_APPVER_LENGTH - 1);
    } else {
      strncpy(fi.appid, "QWIN", OFX_APPID_LENGTH - 1);
      strncpy(fi.appver, "1700", OFX_APPVER_LENGTH - 1);
    }

    QString hver = m_headerVersion->headerVersion();
    strncpy(fi.header_version, hver.toLatin1(), OFX_HEADERVERSION_LENGTH - 1);
#endif

    KUrl filename(QString("%1response.ofx").arg(KStandardDirs::locateLocal("appdata", "")));
    QByteArray req(libofx_request_accountinfo(&fi));
    OfxHttpsRequest(QString("POST"), KUrl((*m_it_info).url), req, QMap<QString, QString>(), filename, true);

    LibofxContextPtr ctx = libofx_get_new_context();
    Q_CHECK_PTR(ctx);

    ofx_set_account_cb(ctx, ofxAccountCallback, this);
    ofx_set_status_cb(ctx, ofxStatusCallback, this);
    // Add resulting accounts to the account list
    libofx_proc_file(ctx, filename.path().toLatin1(), AUTODETECT);
    libofx_free_context(ctx);

    ++m_it_info;
  }

  if (! m_listAccount->childCount()) {
    KMessageBox::sorry(this, i18n("No suitable accounts were found at this bank."));
    result = false;
  }
  return result;
}

bool KOnlineBankingSetupWizard::finishAccountPage(void)
{
  bool result = true;

  if (! m_listAccount->currentItem()) {
    KMessageBox::sorry(this, i18n("Please choose an account"));
    result = false;
  }

  return result;
}

int KOnlineBankingSetupWizard::ofxAccountCallback(struct OfxAccountData data, void * pv)
{
  KOnlineBankingSetupWizard* pthis = reinterpret_cast<KOnlineBankingSetupWizard*>(pv);
  // Put the account info in the view

  MyMoneyKeyValueContainer kvps;

  if (data.account_type_valid) {
    QString type;
    switch (data.account_type) {
      case OfxAccountData::OFX_CHECKING:  /**< A standard checking account */
        type = "CHECKING";
        break;
      case OfxAccountData::OFX_SAVINGS:   /**< A standard savings account */
        type = "SAVINGS";
        break;
      case OfxAccountData::OFX_MONEYMRKT: /**< A money market account */
        type = "MONEY MARKET";
        break;
      case OfxAccountData::OFX_CREDITLINE: /**< A line of credit */
        type = "CREDIT LINE";
        break;
      case OfxAccountData::OFX_CMA:       /**< Cash Management Account */
        type = "CMA";
        break;
      case OfxAccountData::OFX_CREDITCARD: /**< A credit card account */
        type = "CREDIT CARD";
        break;
      case OfxAccountData::OFX_INVESTMENT: /**< An investment account */
        type = "INVESTMENT";
        break;
      default:
        break;
    }
    kvps.setValue("type", type);
  }

  if (data.bank_id_valid)
    kvps.setValue("bankid", data.bank_id);

  if (data.broker_id_valid)
    kvps.setValue("bankid", data.broker_id);

  if (data.branch_id_valid)
    kvps.setValue("branchid", data.branch_id);

  if (data.account_number_valid)
    kvps.setValue("accountid", data.account_number);

  if (data.account_id_valid)
    kvps.setValue("uniqueId", data.account_id);

  kvps.setValue("username", pthis->m_editUsername->text());
  kvps.setValue("password", pthis->m_editPassword->text());

  kvps.setValue("url", (*(pthis->m_it_info)).url);
  kvps.setValue("fid", (*(pthis->m_it_info)).fid);
  kvps.setValue("org", (*(pthis->m_it_info)).org);
  kvps.setValue("fipid", "");
  Q3ListViewItem* item = pthis->m_listFi->currentItem();
  if (item)
    kvps.setValue("bankname", item->text(0));

  // I removed the bankid here, because for some users it
  // was not possible to setup the automatic account matching
  // because the bankid was left empty here as well during
  // the statement download. In case we don't have it, we
  // simply use it blank. (ipwizard 2009-06-21)
  if (/* !kvps.value("bankid").isEmpty()
  && */ !kvps.value("uniqueId").isEmpty()) {

    kvps.setValue("kmmofx-acc-ref", QString("%1-%2").arg(kvps.value("bankid"), kvps.value("uniqueId")));
  } else {
    qDebug("Cannot setup kmmofx-acc-ref for '%s'", qPrintable(kvps.value("bankname")));
  }
  kvps.setValue("protocol", "OFX");

  new ListViewItem(pthis->m_listAccount, kvps);

  return 0;
}

int KOnlineBankingSetupWizard::ofxStatusCallback(struct OfxStatusData data, void * pv)
{
  KOnlineBankingSetupWizard* pthis = reinterpret_cast<KOnlineBankingSetupWizard*>(pv);

  QString message;

  if (data.code_valid == true) {
    message += QString("#%1 %2: \"%3\"\n").arg(data.code).arg(data.name, data.description);
  }

  if (data.server_message_valid == true) {
    message += i18n("Server message: %1\n", data.server_message);
  }

  if (data.severity_valid == true) {
    switch (data.severity) {
      case OfxStatusData::INFO :
        break;
      case OfxStatusData::WARN :
        KMessageBox::detailedError(pthis, i18n("Your bank returned warnings when signing on"), i18nc("Warning 'message'", "WARNING %1", message));
        break;
      case OfxStatusData::ERROR :
        KMessageBox::detailedError(pthis, i18n("Error signing onto your bank"), i18n("ERROR %1", message));
        break;
      default:
        break;
    }
  }
  return 0;
}

bool KOnlineBankingSetupWizard::chosenSettings(MyMoneyKeyValueContainer& settings)
{
  bool result = false;;

  if (m_fDone) {
    Q3ListViewItem* qitem = m_listAccount->currentItem();
    ListViewItem* item = dynamic_cast<ListViewItem*>(qitem);
    if (item) {
      settings = *item;
      settings.deletePair("appId");
      settings.deletePair("kmmofx-headerVersion");
      QString appId = m_appId->appId();
      if (!appId.isEmpty())
        settings.setValue("appId", appId);
      QString hVer = m_headerVersion->headerVersion();
      if (!hVer.isEmpty())
        settings.setValue("kmmofx-headerVersion", hVer);
      result = true;
    }
  }

  return result;
}

KOnlineBankingSetupWizard::ListViewItem::ListViewItem(Q3ListView* parent, const MyMoneyKeyValueContainer& kvps):
    MyMoneyKeyValueContainer(kvps), Q3ListViewItem(parent)
{
  setText(0, value("accountid"));
  setText(1, value("type"));
  setText(2, value("bankid"));
  setText(3, value("branchid"));
}

void KOnlineBankingSetupWizard::ListViewItem::x(void) {}

#include "konlinebankingsetupwizard.moc"

// vim:cin:si:ai:et:ts=2:sw=2:
