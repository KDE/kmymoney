/*
    SPDX-FileCopyrightText: 2006 Ace Jones <acejones@users.sourceforge.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "konlinebankingsetupwizard.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QCheckBox>
#include <QLabel>
#include <QProgressDialog>
#include <QRegExp>
#include <QStandardPaths>
#include <QTabWidget>
#include <QTextCodec>
#include <QTextStream>
#include <QTimer>
#include <QUuid>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>
#include <KMessageBox>
#include <KListWidgetSearchLine>
#include <KComboBox>
#include <KUrlRequester>
#include <KWallet>

// ----------------------------------------------------------------------------
// Project Includes

#include "../ofxpartner.h"
#include "kmymoneysettings.h"
#include "mymoneyofxconnector.h"
#include "passwordtoggle.h"

using KWallet::Wallet;

class KOnlineBankingSetupWizard::Private
{
public:
    Private() : m_prevPage(-1), m_wallet(0), m_walletIsOpen(false) {}
    QFile       m_fpTrace;
    QTextStream m_trace;
    int         m_prevPage;
    Wallet      *m_wallet;
    bool        m_walletIsOpen;
};

KOnlineBankingSetupWizard::KOnlineBankingSetupWizard(QWidget *parent):
    QWizard(parent),
    d(new Private),
    m_fDone(false),
    m_fInit(false),
    m_appId(0)
{
    setupUi(this);

    m_applicationEdit->hide();
    m_headerVersionEdit->hide();
#ifndef LIBOFX_HAVE_CLIENTUID
    m_editClientUid->setEnabled(false);
    m_clientUidLabel->setEnabled(false);
#endif

    m_appId = new OfxAppVersion(m_applicationCombo, m_applicationEdit, "");
    m_headerVersion = new OfxHeaderVersion(m_headerVersionCombo, "");

    // fill the list view with banks
    QProgressDialog* dlg = new QProgressDialog(this);
    dlg->setWindowTitle(i18n("Loading banklist"));
    dlg->setLabelText(i18n("Getting list of banks from https://www.ofxhome.com/\nThis may take some time depending on the available bandwidth."));
    dlg->setModal(true);
    dlg->setCancelButton(0);
    // force to show immediately as the call to OfxPartner::BankNames()
    // does not call the processEvents() loop
    dlg->setMinimumDuration(0);
    QCoreApplication::processEvents();

    //set password field according to KDE preferences
    m_editPassword->setPasswordMode(true);
    new PasswordToggle(m_editPassword);

    // make sure to not exceed data fields
    m_editUsername->setMaxLength(OFX_USERID_LENGTH-1);
    m_editPassword->setMaxLength(OFX_USERPASS_LENGTH-1);

    KListWidgetSearchLine* searchLine = new KListWidgetSearchLine(autoTab, m_listFi);
    vboxLayout1->insertWidget(0, searchLine);
    QTimer::singleShot(20, searchLine, SLOT(setFocus()));

    OfxPartner::setDirectory(QStandardPaths::writableLocation(QStandardPaths::DataLocation) + QLatin1Char('/') + "");
    m_listFi->addItems(OfxPartner::BankNames());
    m_fInit = true;
    delete dlg;

    checkNextButton();
    connect(this, &KOnlineBankingSetupWizard::currentIdChanged, this, &KOnlineBankingSetupWizard::checkNextButton);
    connect(this, &KOnlineBankingSetupWizard::currentIdChanged, this, &KOnlineBankingSetupWizard::newPage);

    connect(m_listFi, &QListWidget::itemSelectionChanged, this, &KOnlineBankingSetupWizard::checkNextButton);
    connect(m_listAccount, &QTreeWidget::itemSelectionChanged, this, &KOnlineBankingSetupWizard::checkNextButton);
    connect(m_selectionTab, &QTabWidget::currentChanged, this, &KOnlineBankingSetupWizard::checkNextButton);
    connect(m_fid, &QLineEdit::textEdited, this, &KOnlineBankingSetupWizard::checkNextButton);
    connect(m_bankName, &QLineEdit::textEdited, this, &KOnlineBankingSetupWizard::checkNextButton);
    connect(m_url, &KUrlRequester::textChanged, this, &KOnlineBankingSetupWizard::checkNextButton);
    connect(m_editUsername, &QLineEdit::textEdited, this, &KOnlineBankingSetupWizard::checkNextButton);
    connect(m_editPassword, &QLineEdit::textEdited, this, &KOnlineBankingSetupWizard::checkNextButton);
    connect(m_applicationEdit, &QLineEdit::textEdited, this, &KOnlineBankingSetupWizard::checkNextButton);
    connect(m_applicationCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &KOnlineBankingSetupWizard::applicationSelectionChanged);

    // setup text on buttons
    setButtonText(QWizard::NextButton, i18nc("Go to next page of the wizard", "&Next"));
    setButtonText(QWizard::BackButton, KStandardGuiItem::back().text());

    // setup icons
    button(QWizard::FinishButton)->setIcon(KStandardGuiItem::ok().icon());
    button(QWizard::CancelButton)->setIcon(KStandardGuiItem::cancel().icon());
    button(QWizard::NextButton)->setIcon(KStandardGuiItem::forward(KStandardGuiItem::UseRTL).icon());
    button(QWizard::BackButton)->setIcon(KStandardGuiItem::back(KStandardGuiItem::UseRTL).icon());

    m_problemMessages->setHidden(true);
    m_problemMessages->setWordWrap(true);
}

KOnlineBankingSetupWizard::~KOnlineBankingSetupWizard()
{
    delete m_appId;
    delete d;
}

void KOnlineBankingSetupWizard::applicationSelectionChanged()
{
    m_applicationEdit->setVisible(m_appId->appId().endsWith(':'));
    checkNextButton();
}

void KOnlineBankingSetupWizard::walletOpened(bool ok)
{
    if (ok && (d->m_wallet->hasFolder(KWallet::Wallet::PasswordFolder()) ||
               d->m_wallet->createFolder(KWallet::Wallet::PasswordFolder())) &&
            d->m_wallet->setFolder(KWallet::Wallet::PasswordFolder())) {
        d->m_walletIsOpen = true;
    } else {
        qDebug("Wallet was not opened");
    }
    m_storePassword->setEnabled(d->m_walletIsOpen);
}

void KOnlineBankingSetupWizard::checkNextButton()
{
    bool enableButton = false;
    switch (currentId()) {
    case 0:
        if (m_selectionTab->currentIndex() == 0) {
            enableButton = (m_listFi->currentItem() != 0)
                           && m_listFi->currentItem()->isSelected();
        } else {
            enableButton = !(m_url->url().isEmpty()
                             || m_bankName->text().isEmpty());
        }
        break;

    case 1:
        enableButton = !(m_editUsername->text().isEmpty()
                         || m_editPassword->text().isEmpty()
                         || !m_appId->isValid());
        break;

    case 2:
        enableButton = (m_listAccount->currentItem() != 0)
                       && m_listAccount->currentItem()->isSelected();
        break;
    }
    button(QWizard::NextButton)->setEnabled(enableButton);
}

void KOnlineBankingSetupWizard::newPage(int id)
{
    QWidget* focus = 0;

    m_problemMessages->setHidden(true);
    bool ok = true;
    if ((id - d->m_prevPage) == 1) { // one page forward?
        switch (d->m_prevPage) {
        case 0:
            ok = finishFiPage();
            // open the KDE wallet if not already opened
            if (ok && !d->m_wallet) {
                d->m_wallet = Wallet::openWallet(Wallet::NetworkWallet(), winId(), Wallet::Asynchronous);
                connect(d->m_wallet, &KWallet::Wallet::walletOpened, this, &KOnlineBankingSetupWizard::walletOpened);
            }
            focus = m_editUsername;
            break;
        case 1:
            ok = finishLoginPage();
            focus = m_listAccount;
            break;
        case 2:
            m_fDone = ok = finishAccountPage();
            break;
        }

        if (ok) {
            if (focus) {
                focus->setFocus();
            }
        } else {
            // force to go back to prev page
            back();
        }
    } else {
        // going backwards, we're never done
        m_fDone = false;
    }

    button(QWizard::FinishButton)->setEnabled(m_fDone);

    // hide cancel and back button on last page
    button(QWizard::CancelButton)->setVisible(!m_fDone);
    button(QWizard::BackButton)->setVisible(!m_fDone);

    if (ok)
        d->m_prevPage = id;
}

bool KOnlineBankingSetupWizard::finishFiPage()
{
    bool result = false;

    m_bankInfo.clear();
    OfxHomeServiceInfo info;

    if (m_selectionTab->currentIndex() == 0) {

        // Get the fipids for the selected bank
        QListWidgetItem* item = m_listFi->currentItem();
        if (item && item->isSelected()) {
            QString bank = item->text();
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
                if (info.ofxInfo.accountlist) {
                    m_bankInfo.push_back(info.ofxInfo);

                    message += QString("URL: %1<br/>Org: %2<br/>Fid: %3<br/>").arg(info.ofxInfo.url, info.ofxInfo.org, info.ofxInfo.fid);

                    if (info.ofxInfo.statements)
                        message += i18n("Supports online statements<br/>");
                    if (info.ofxInfo.investments)
                        message += i18n("Supports investments<br/>");
                    if (info.ofxInfo.billpay)
                        message += i18n("Supports bill payment (but not supported by KMyMoney yet)<br/>");

                    QString problemMessage;
                    if (!info.ofxValidated)
                        problemMessage += i18n("OFX host failed. Last successful access was on '%1'. ", info.lastOfxValidated);
                    if (!info.sslValidated)
                        problemMessage += i18n("Certificate verification of OFX host failed. Last successful verification was on '%1'.", info.lastSslValidated);

                    if (!problemMessage.isEmpty()) {
                        m_problemMessages->setText(problemMessage);
                        m_problemMessages->animatedShow();
                    }

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

        memset(&info.ofxInfo, 0, sizeof(OfxFiServiceInfo));
        strncpy(info.ofxInfo.fid, m_fid->text().toLatin1(), OFX_FID_LENGTH - 1);
        strncpy(info.ofxInfo.org, m_bankName->text().toLatin1(), OFX_ORG_LENGTH - 1);
        strncpy(info.ofxInfo.url, m_url->url().url().toLatin1(), OFX_URL_LENGTH - 1);
        info.ofxInfo.accountlist = 1;
        info.ofxInfo.statements = 1;
        info.ofxInfo.billpay = 1;
        info.ofxInfo.investments = 1;

        m_bankInfo.push_back(info.ofxInfo);

        QString message;
        message += QString("<p>URL: %1<br/>Org: %2<br/>Fid: %3<br/>").arg(info.ofxInfo.url, info.ofxInfo.org, info.ofxInfo.fid);
        if (info.ofxInfo.statements)
            message += i18n("Supports online statements<br/>");
        if (info.ofxInfo.investments)
            message += i18n("Supports investments<br/>");
        if (info.ofxInfo.billpay)
            message += i18n("Supports bill payment (but not supported by KMyMoney yet)<br/>");
        message += "</p>";
        m_textDetails->append(message);
        result = true;
    }
    // make sure to display the beginning of the collected information
    m_textDetails->moveCursor(QTextCursor::Start);

    return result;
}

bool KOnlineBankingSetupWizard::finishLoginPage()
{
    bool result = true;

    const auto username = m_editUsername->text();
    const auto password = m_editPassword->text();
    const auto clientUid = m_editClientUid->text();
    const auto userAgent = m_userAgent->text();

    m_listAccount->clear();

    // Process an account request for each fipid
    m_it_info = m_bankInfo.constBegin();
    while (m_it_info != m_bankInfo.constEnd()) {
        OfxFiLogin fi;
        memset(&fi, 0, sizeof(OfxFiLogin));
        Q_ASSERT(sizeof(fi.fid) == sizeof((*m_it_info).fid));
        Q_ASSERT(sizeof(fi.org) == sizeof((*m_it_info).org));
        memcpy(fi.fid, (*m_it_info).fid, OFX_FID_LENGTH - 1);
        memcpy(fi.org, (*m_it_info).org, OFX_ORG_LENGTH - 1);
        strncpy(fi.userid, username.toLatin1().left(OFX_USERID_LENGTH - 1), OFX_USERID_LENGTH - 1);
        strncpy(fi.userpass, password.toLatin1(), OFX_USERPASS_LENGTH - 1);
#ifdef LIBOFX_HAVE_CLIENTUID
        strncpy(fi.clientuid, clientUid.toLatin1(), OFX_CLIENTUID_LENGTH - 1);
#endif

        // pretend we're Quicken 2008
        // https://ofxblog.wordpress.com/2007/06/06/ofx-appid-and-appver-for-intuit-products/
        // https://ofxblog.wordpress.com/2007/06/06/ofx-appid-and-appver-for-microsoft-money/
        QString appId = m_appId->appId();
        QRegExp exp("(.*):(.*)");
        if (exp.indexIn(appId) != -1) {
            strncpy(fi.appid, exp.cap(1).toLatin1(), OFX_APPID_LENGTH - 1);
            if (exp.cap(2).isEmpty()) {
                strncpy(fi.appver, m_applicationEdit->text().toLatin1(), OFX_APPVER_LENGTH - 1);
            } else {
                strncpy(fi.appver, exp.cap(2).toLatin1(), OFX_APPVER_LENGTH - 1);
            }
        } else {
            strncpy(fi.appid, "QWIN", OFX_APPID_LENGTH - 1);
            strncpy(fi.appver, "1700", OFX_APPVER_LENGTH - 1);
        }

        QString hver = m_headerVersion->headerVersion();
        strncpy(fi.header_version, hver.toLatin1(), OFX_HEADERVERSION_LENGTH - 1);

        auto codec = QTextCodec::codecForName("Windows-1251");
        auto request = codec->toUnicode(libofx_request_accountinfo(&fi));

        // Check if we need to tweak the request for specific institutions
        MyMoneyOfxConnector::institutionSpecificRequestAdjustment(request);

        QUrl filename(QString("file://%1response.ofx").arg(QStandardPaths::writableLocation(QStandardPaths::DataLocation) + QLatin1Char('/')));

        // because the event loop is running while the request is performed disable the back button
        // (this function is not reentrant so the application might crash when back/next are used)
        QAbstractButton *backButton = button(QWizard::BackButton);
        bool backButtonState = backButton->isEnabled();
        backButton->setEnabled(false);

        QMap<QString, QString> metaData;
        if (!userAgent.isEmpty()) {
            metaData.insert(QLatin1String("UserAgent"), userAgent);
        }

        OfxHttpRequest(QString("POST"), QUrl((*m_it_info).url), codec->fromUnicode(request), metaData, filename, false);
        backButton->setEnabled(backButtonState);

        LibofxContextPtr ctx = libofx_get_new_context();
        Q_CHECK_PTR(ctx);

        ofx_set_account_cb(ctx, ofxAccountCallback, this);
        ofx_set_status_cb(ctx, ofxStatusCallback, this);
        // Add resulting accounts to the account list
        libofx_proc_file(ctx, filename.toLocalFile().toLatin1(), AUTODETECT);
        libofx_free_context(ctx);

        ++m_it_info;
    }

    if (! m_listAccount->topLevelItem(0)) {
        KMessageBox::sorry(this, i18n("No suitable accounts were found at this bank."));
        result = false;
    } else {
        m_listAccount->resizeColumnToContents(0);
        m_listAccount->resizeColumnToContents(1);
        m_listAccount->resizeColumnToContents(2);
        m_listAccount->resizeColumnToContents(3);
    }
    return result;
}

bool KOnlineBankingSetupWizard::finishAccountPage()
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
#ifdef LIBOFX_HAVE_CLIENTUID
    kvps.setValue("clientUid", pthis->m_editClientUid->text());
#endif

    kvps.setValue("url", (*(pthis->m_it_info)).url);
    kvps.setValue("fid", (*(pthis->m_it_info)).fid);
    kvps.setValue("org", (*(pthis->m_it_info)).org);
    kvps.setValue("fipid", "");
    QListWidgetItem* item = pthis->m_listFi->currentItem();
    if (item)
        kvps.setValue("bankname", item->text());

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

    if (data.code_valid) {
        message += QString("#%1 %2: \"%3\"\n").arg(data.code).arg(data.name, data.description);
    }

    if (data.server_message_valid) {
        message += i18n("Server message: %1\n", data.server_message);
    }

    if (data.severity_valid) {
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
    bool result = false;

    if (m_fDone) {
        QTreeWidgetItem* qitem = m_listAccount->currentItem();
        ListViewItem* item = dynamic_cast<ListViewItem*>(qitem);
        if (item && item->isSelected()) {
            settings = *item;
            settings.deletePair("appId");
            settings.deletePair("kmmofx-headerVersion");
            QString appId = m_appId->appId();
            if (!appId.isEmpty()) {
                if (appId.endsWith(':')) {
                    appId += m_applicationEdit->text();
                }
                settings.setValue("appId", appId);
            }
            QString hVer = m_headerVersion->headerVersion();
            if (!hVer.isEmpty())
                settings.setValue("kmmofx-headerVersion", hVer);
            if (m_storePassword->isChecked()) {
                if (d->m_walletIsOpen) {
                    QString key = OFX_PASSWORD_KEY(settings.value("url"), settings.value("uniqueId"));
                    d->m_wallet->writePassword(key, settings.value("password"));
                    settings.deletePair("password");
                }
            } else {
                settings.deletePair("password");
            }
            result = true;
        }
    }

    return result;
}

KOnlineBankingSetupWizard::ListViewItem::ListViewItem(QTreeWidget* parent, const MyMoneyKeyValueContainer& kvps):
    MyMoneyKeyValueContainer(kvps), QTreeWidgetItem(parent)
{
    setText(0, value("accountid"));
    setText(1, value("type"));
    setText(2, value("bankid"));
    setText(3, value("branchid"));
}
