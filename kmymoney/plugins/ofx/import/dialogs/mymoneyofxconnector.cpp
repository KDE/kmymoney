/*
    SPDX-FileCopyrightText: 2002 Ace Jones <acejones@users.sourceforge.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "mymoneyofxconnector.h"

// ----------------------------------------------------------------------------
// System Includes

#include <libofx/libofx.h>

// ----------------------------------------------------------------------------
// QT Includes

#include <QApplication>
#include <QByteArray>
#include <QDateTime>
#include <QDebug>
#include <QList>
#include <QRegExp>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>
#include <KComboBox>
#include <KPasswordDialog>
#include <KWallet>
#include <KMainWindow>
#include <KLineEdit>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyaccount.h"
#include "mymoneykeyvaluecontainer.h"
#include "mymoneyenums.h"

using KWallet::Wallet;

OfxHeaderVersion::OfxHeaderVersion(KComboBox* combo, const QString& headerVersion) :
    m_combo(combo)
{
    combo->clear();
    combo->addItem("102");
    combo->addItem("103");

    if (!headerVersion.isEmpty()) {
        combo->setCurrentItem(headerVersion);
    } else {
        combo->setCurrentItem("102");
    }
}

QString OfxHeaderVersion::headerVersion() const
{
    return m_combo->currentText();
}

OfxAppVersion::OfxAppVersion(KComboBox* combo, KLineEdit* versionEdit, const QString& appId) :
    m_combo(combo),
    m_versionEdit(versionEdit)
{
// https://ofxblog.wordpress.com/2007/06/06/ofx-appid-and-appver-for-intuit-products/
// https://ofxblog.wordpress.com/2007/06/06/ofx-appid-and-appver-for-microsoft-money/

    // Quicken
    m_appMap[i18n("Quicken Windows 2003")] = "QWIN:1200";
    m_appMap[i18n("Quicken Windows 2004")] = "QWIN:1300";
    m_appMap[i18n("Quicken Windows 2005")] = "QWIN:1400";
    m_appMap[i18n("Quicken Windows 2006")] = "QWIN:1500";
    m_appMap[i18n("Quicken Windows 2007")] = "QWIN:1600";
    m_appMap[i18n("Quicken Windows 2008")] = "QWIN:1700";
    // the following three added as found on
    // https://microsoftmoneyoffline.wordpress.com/appid-appver/ on 2013-02-28
    m_appMap[i18n("Quicken Windows 2010")] = "QWIN:1800";
    m_appMap[i18n("Quicken Windows 2011")] = "QWIN:1900";
    m_appMap[i18n("Quicken Windows 2012")] = "QWIN:2100";
    m_appMap[i18n("Quicken Windows 2013")] = "QWIN:2200";
    m_appMap[i18n("Quicken Windows 2014")] = "QWIN:2300";
    // following two added as found in previous URL on 2017-10-01
    m_appMap[i18n("Quicken Windows 2015")] = "QWIN:2400";
    m_appMap[i18n("Quicken Windows 2016")] = "QWIN:2500";
    // following three added as logical consequence on 2018-12-12
    m_appMap[i18n("Quicken Windows 2017")] = "QWIN:2600";
    m_appMap[i18n("Quicken Windows 2018")] = "QWIN:2700";
    m_appMap[i18n("Quicken Windows 2019")] = "QWIN:2800";
    // the swiss army knife entry :)
    m_appMap[i18n("Quicken Windows (Expert)")] = "QWIN:";

    // MS-Money
    m_appMap[i18n("MS-Money 2003")] = "Money:1100";
    m_appMap[i18n("MS-Money 2004")] = "Money:1200";
    m_appMap[i18n("MS-Money 2005")] = "Money:1400";
    m_appMap[i18n("MS-Money 2006")] = "Money:1500";
    m_appMap[i18n("MS-Money 2007")] = "Money:1600";
    m_appMap[i18n("MS-Money Plus")] = "Money:1700";
    m_appMap[i18n("MS-Money (Expert)")] = "Money:";

    // KMyMoney
    m_appMap["KMyMoney"] = "KMyMoney:1000";

    combo->clear();
    combo->addItems(m_appMap.keys());
    if (versionEdit)
        versionEdit->hide();

    QMap<QString, QString>::const_iterator it_a;
    // check for an exact match
    for (it_a = m_appMap.constBegin(); it_a != m_appMap.constEnd(); ++it_a) {
        if (*it_a == appId)
            break;
    }

    // not found, check if we have a manual version of this product
    QRegExp appExp("(\\w+:)(\\d+)");
    if (it_a == m_appMap.constEnd()) {
        if (appExp.exactMatch(appId)) {
            for (it_a = m_appMap.constBegin(); it_a != m_appMap.constEnd(); ++it_a) {
                if (*it_a == appExp.cap(1))
                    break;
            }
        }
    }

    // if we still haven't found it, we use a default as last resort
    if (it_a != m_appMap.constEnd()) {
        combo->setCurrentItem(it_a.key());
        if ((*it_a).endsWith(':')) {
            if (versionEdit) {
                versionEdit->show();
                versionEdit->setText(appExp.cap(2));
            } else {
                combo->setCurrentItem(i18n("Quicken Windows 2008"));
            }
        }
    } else {
        combo->setCurrentItem(i18n("Quicken Windows 2008"));
    }
}

const QString OfxAppVersion::appId() const
{
    static QString defaultAppId("QWIN:1700");

    QString app = m_combo->currentText();
    if (m_appMap[app] != defaultAppId) {
        if (m_appMap[app].endsWith(':')) {
            if (m_versionEdit) {
                return m_appMap[app] + m_versionEdit->text();
            } else {
                return QString();
            }
        }
        return m_appMap[app];
    }
    return QString();
}

bool OfxAppVersion::isValid() const
{
    QRegExp exp(".+:\\d+");
    QString app = m_combo->currentText();
    if (m_appMap[app].endsWith(':')) {
        if (m_versionEdit) {
            app = m_appMap[app] + m_versionEdit->text();
        } else {
            app.clear();
        }
    } else {
        app = m_appMap[app];
    }
    return exp.exactMatch(app);
}

MyMoneyOfxConnector::MyMoneyOfxConnector(const MyMoneyAccount& _account):
    m_account(_account)
{
    m_fiSettings = m_account.onlineBankingSettings();
}

QString MyMoneyOfxConnector::iban() const
{
    return m_fiSettings.value("bankid");
}
QString MyMoneyOfxConnector::fiorg() const
{
    return m_fiSettings.value("org");
}
QString MyMoneyOfxConnector::fiid() const
{
    return m_fiSettings.value("fid");
}
QString MyMoneyOfxConnector::clientUid() const
{
    return m_fiSettings.value("clientUid");
}
QString MyMoneyOfxConnector::username() const
{
    return m_fiSettings.value("username");
}
QString MyMoneyOfxConnector::password() const
{
    // if we don't find a password in the wallet, we use the old method
    // and retrieve it from the settings stored in the KMyMoney data storage.
    // in case we don't have a password on file, we ask the user
    QString key = OFX_PASSWORD_KEY(m_fiSettings.value("url"), m_fiSettings.value("uniqueId"));
    QString pwd = m_fiSettings.value("password");

    // now check for the wallet
    Wallet *wallet = openSynchronousWallet();
    if (wallet
            && !Wallet::keyDoesNotExist(Wallet::NetworkWallet(), Wallet::PasswordFolder(), key)) {
        wallet->setFolder(Wallet::PasswordFolder());
        wallet->readPassword(key, pwd);
    }

    if (pwd.isEmpty()) {
        QPointer<KPasswordDialog> dlg = new KPasswordDialog(0);
        dlg->setPrompt(i18n("Enter your password for account <b>%1</b>", m_account.name()));
        if (dlg->exec())
            pwd = dlg->password();
        delete dlg;
    }
    return pwd;
}
QString MyMoneyOfxConnector::accountnum() const
{
    return m_fiSettings.value("accountid");
}
QString MyMoneyOfxConnector::url() const
{
    return m_fiSettings.value("url");
}
QString MyMoneyOfxConnector::userAgent() const
{
    return m_fiSettings.value(QLatin1String("kmmofx-useragent"));
}

QDate MyMoneyOfxConnector::statementStartDate() const
{
    if ((m_fiSettings.value("kmmofx-todayMinus").toInt() != 0) && !m_fiSettings.value("kmmofx-numRequestDays").isEmpty()) {
        return QDate::currentDate().addDays(-m_fiSettings.value("kmmofx-numRequestDays").toInt());

    } else if ((m_fiSettings.value("kmmofx-lastUpdate").toInt() != 0) && !m_account.value("lastImportedTransactionDate").isEmpty()) {
        // get last statement request date from application account object
        // and start from a few days before if the date is valid
        QDate lastUpdate = QDate::fromString(m_account.value("lastImportedTransactionDate"), Qt::ISODate);
        if (lastUpdate.isValid()) {
            return lastUpdate.addDays(-3);
        }

    } else if ((m_fiSettings.value("kmmofx-pickDate").toInt() != 0) && !m_fiSettings.value("kmmofx-specificDate").isEmpty()) {
        return QDate::fromString(m_fiSettings.value("kmmofx-specificDate"));
    }
    return QDate::currentDate().addMonths(-2);
}

OfxAccountData::AccountType MyMoneyOfxConnector::accounttype() const
{
    OfxAccountData::AccountType result = OfxAccountData::OFX_CHECKING;

    QString type = m_account.onlineBankingSettings()["type"];
    if (type == "CHECKING")
        result = OfxAccountData::OFX_CHECKING;
    else if (type == "SAVINGS")
        result = OfxAccountData::OFX_SAVINGS;
    else if (type == "MONEY MARKET")
        result = OfxAccountData::OFX_MONEYMRKT;
    else if (type == "CREDIT LINE")
        result = OfxAccountData::OFX_CREDITLINE;
    else if (type == "CMA")
        result = OfxAccountData::OFX_CMA;
    else if (type == "CREDIT CARD")
        result = OfxAccountData::OFX_CREDITCARD;
    else if (type == "INVESTMENT")
        result = OfxAccountData::OFX_INVESTMENT;
    else {
        switch (m_account.accountType()) {
        case eMyMoney::Account::Type::Investment:
            result = OfxAccountData::OFX_INVESTMENT;
            break;
        case eMyMoney::Account::Type::CreditCard:
            result = OfxAccountData::OFX_CREDITCARD;
            break;
        case eMyMoney::Account::Type::Savings:
            result = OfxAccountData::OFX_SAVINGS;
            break;
        default:
            break;
        }
    }

    // This is a bit of a personalized hack.  Sometimes we may want to override the
    // ofx type for an account.  For now, I will stash it in the notes!

    QRegExp rexp("OFXTYPE:([A-Z]*)");
    if (rexp.indexIn(m_account.description()) != -1) {
        QString override = rexp.cap(1);
        qDebug() << "MyMoneyOfxConnector::accounttype() overriding to " << result;

        if (override == "BANK")
            result = OfxAccountData::OFX_CHECKING;
        else if (override == "CC")
            result = OfxAccountData::OFX_CREDITCARD;
        else if (override == "INV")
            result = OfxAccountData::OFX_INVESTMENT;
        else if (override == "MONEYMARKET")
            result = OfxAccountData::OFX_MONEYMRKT;
    }

    return result;
}

void MyMoneyOfxConnector::initRequest(OfxFiLogin* fi) const
{
    memset(fi, 0, sizeof(OfxFiLogin));
    strncpy(fi->fid, fiid().toLatin1(), OFX_FID_LENGTH - 1);
    strncpy(fi->org, fiorg().toLatin1(), OFX_ORG_LENGTH - 1);
    strncpy(fi->userid, username().toLatin1(), OFX_USERID_LENGTH - 1);
    strncpy(fi->userpass, password().toLatin1(), OFX_USERPASS_LENGTH - 1);
#ifdef LIBOFX_HAVE_CLIENTUID
    strncpy(fi->clientuid, clientUid().toLatin1(), OFX_CLIENTUID_LENGTH - 1);
#endif

    // If we don't know better, we pretend to be Quicken 2008
    // https://ofxblog.wordpress.com/2007/06/06/ofx-appid-and-appver-for-intuit-products/
    // https://ofxblog.wordpress.com/2007/06/06/ofx-appid-and-appver-for-microsoft-money/
    QString appId = m_account.onlineBankingSettings().value("appId");
    QRegExp exp("(.*):(.*)");
    if (exp.indexIn(appId) != -1) {
        strncpy(fi->appid, exp.cap(1).toLatin1(), OFX_APPID_LENGTH - 1);
        strncpy(fi->appver, exp.cap(2).toLatin1(), OFX_APPVER_LENGTH - 1);
    } else {
        strncpy(fi->appid, "QWIN", OFX_APPID_LENGTH - 1);
        strncpy(fi->appver, "1700", OFX_APPVER_LENGTH - 1);
    }

    QString headerVersion = m_account.onlineBankingSettings().value("kmmofx-headerVersion");
    if (!headerVersion.isEmpty()) {
        strncpy(fi->header_version, headerVersion.toLatin1(), OFX_HEADERVERSION_LENGTH - 1);
    }
}

const QByteArray MyMoneyOfxConnector::statementRequest() const
{
    OfxFiLogin fi;
    initRequest(&fi);

    OfxAccountData account;
    memset(&account, 0, sizeof(OfxAccountData));

    if (!iban().toLatin1().isEmpty()) {
        strncpy(account.bank_id, iban().toLatin1(), OFX_BANKID_LENGTH - 1);
        strncpy(account.broker_id, iban().toLatin1(), OFX_BROKERID_LENGTH - 1);
    }
    strncpy(account.account_number, accountnum().toLatin1(), OFX_ACCTID_LENGTH - 1);
    account.account_type = accounttype();

    QByteArray result;
    if (fi.userpass[0]) {
        char *szrequest = libofx_request_statement(&fi, &account, QDateTime(statementStartDate()).toTime_t());
        QString request = szrequest;
        // remove the trailing zero
        result = request.toUtf8();
        if(result.at(result.size()-1) == 0)
            result.truncate(result.size() - 1);
        free(szrequest);
    }

    return result;
}


#if 0

MyMoneyOfxConnector::Tag MyMoneyOfxConnector::message(const QString& _msgType, const QString& _trnType, const Tag& _request)
{
    return Tag(_msgType + "MSGSRQV1")
           .subtag(Tag(_trnType + "TRNRQ")
                   .element("TRNUID", uuid())
                   .element("CLTCOOKIE", "1")
                   .subtag(_request));
}

MyMoneyOfxConnector::Tag MyMoneyOfxConnector::investmentRequest() const
{
    QString dtnow_string = QDateTime::currentDateTime().toString(Qt::ISODate).remove(QRegExp("[^0-9]"));
    QString dtstart_string = _dtstart.toString(Qt::ISODate).remove(QRegExp("[^0-9]"));

    return message("INVSTMT", "INVSTMT", Tag("INVSTMTRQ")
                   .subtag(Tag("INVACCTFROM").element("BROKERID", fiorg()).element("ACCTID", accountnum()))
                   .subtag(Tag("INCTRAN").element("DTSTART", dtstart_string).element("INCLUDE", "Y"))
                   .element("INCOO", "Y")
                   .subtag(Tag("INCPOS").element("DTASOF", dtnow_string).element("INCLUDE", "Y"))
                   .element("INCBAL", "Y"));
}

MyMoneyOfxConnector::Tag MyMoneyOfxConnector::bankStatementRequest(const QDate& _dtstart) const
{
    QString dtstart_string = _dtstart.toString(Qt::ISODate).remove(QRegExp("[^0-9]"));

    return message("BANK", "STMT", Tag("STMTRQ")
                   .subtag(Tag("BANKACCTFROM").element("BANKID", iban()).element("ACCTID", accountnum()).element("ACCTTYPE", "CHECKING"))
                   .subtag(Tag("INCTRAN").element("DTSTART", dtstart_string).element("INCLUDE", "Y")));
}

MyMoneyOfxConnector::Tag MyMoneyOfxConnector::creditCardRequest(const QDate& _dtstart) const
{
    QString dtstart_string = _dtstart.toString(Qt::ISODate).remove(QRegExp("[^0-9]"));

    return message("CREDITCARD", "CCSTMT", Tag("CCSTMTRQ")
                   .subtag(Tag("CCACCTFROM").element("ACCTID", accountnum()))
                   .subtag(Tag("INCTRAN").element("DTSTART", dtstart_string).element("INCLUDE", "Y")));
}

MyMoneyOfxConnector::Tag MyMoneyOfxConnector::signOn() const
{
    QString dtnow_string = QDateTime::currentDateTime().toString(Qt::ISODate).remove(QRegExp("[^0-9]"));

    Tag fi("FI");
    fi.element("ORG", fiorg());
    if (!fiid().isEmpty())
        fi.element("FID", fiid());

    return Tag("SIGNONMSGSRQV1")
           .subtag(Tag("SONRQ")
                   .element("DTCLIENT", dtnow_string)
                   .element("USERID", username())
                   .element("USERPASS", password())
                   .element("LANGUAGE", "ENG")
                   .subtag(fi)
                   .element("APPID", "QWIN")
                   .element("APPVER", "1100"));
}

QString MyMoneyOfxConnector::header()
{
    return QString("OFXHEADER:100\r\n"
                   "DATA:OFXSGML\r\n"
                   "VERSION:102\r\n"
                   "SECURITY:NONE\r\n"
                   "ENCODING:USASCII\r\n"
                   "CHARSET:1252\r\n"
                   "COMPRESSION:NONE\r\n"
                   "OLDFILEUID:NONE\r\n"
                   "NEWFILEUID:%1\r\n"
                   "\r\n").arg(uuid());
}

QString MyMoneyOfxConnector::uuid()
{
    static int id = 1;
    return QDateTime::currentDateTime().toString("yyyyMMdd-hhmmsszzz-") + QString::number(id++);
}

//
// Methods to provide RESPONSES to OFX requests.  This has no real use in
// KMyMoney, but it's included for the purposes of unit testing.  This way, I
// can create a MyMoneyAccount, write it to an OFX file, import that OFX file,
// and check that everything made it through the importer.
//
// It's also a far-off dream to write an OFX server using KMyMoney as a
// backend.  It really should not be that hard, and it would fill a void in
// the open source software community.
//

const QByteArray MyMoneyOfxConnector::statementResponse(const QDate& _dtstart) const
{
    QString request;

    if (accounttype() == "CC")
        request = header() + Tag("OFX").subtag(signOnResponse()).subtag(creditCardStatementResponse(_dtstart));
    else if (accounttype() == "INV")
        request = header() + Tag("OFX").subtag(signOnResponse()).data(investmentStatementResponse(_dtstart));
    else
        request = header() + Tag("OFX").subtag(signOnResponse()).subtag(bankStatementResponse(_dtstart));

    // remove the trailing zero
    QByteArray result = request.utf8();
    result.truncate(result.size() - 1);

    return result;
}

MyMoneyOfxConnector::Tag MyMoneyOfxConnector::signOnResponse() const
{
    QString dtnow_string = QDateTime::currentDateTime().toString(Qt::ISODate).remove(QRegExp("[^0-9]"));

    Tag sonrs("SONRS");
    sonrs
    .subtag(Tag("STATUS")
            .element("CODE", "0")
            .element("SEVERITY", "INFO")
            .element("MESSAGE", "The operation succeeded.")
           )
    .element("DTSERVER", dtnow_string)
    .element("LANGUAGE", "ENG");

    Tag fi("FI");
    if (!fiorg().isEmpty())
        fi.element("ORG", fiorg());
    if (!fiid().isEmpty())
        fi.element("FID", fiid());

    if (!fi.isEmpty())
        sonrs.subtag(fi);

    return Tag("SIGNONMSGSRSV1").subtag(sonrs);
}

MyMoneyOfxConnector::Tag MyMoneyOfxConnector::messageResponse(const QString& _msgType, const QString& _trnType, const Tag& _response)
{
    return Tag(_msgType + "MSGSRSV1")
           .subtag(Tag(_trnType + "TRNRS")
                   .element("TRNUID", uuid())
                   .subtag(Tag("STATUS").element("CODE", "0").element("SEVERITY", "INFO"))
                   .element("CLTCOOKIE", "1")
                   .subtag(_response));
}

MyMoneyOfxConnector::Tag MyMoneyOfxConnector::bankStatementResponse(const QDate& _dtstart) const
{
    MyMoneyFile* file = MyMoneyFile::instance();

    QString dtstart_string = _dtstart.toString(Qt::ISODate).remove(QRegExp("[^0-9]"));
    QString dtnow_string = QDateTime::currentDateTime().toString(Qt::ISODate).remove(QRegExp("[^0-9]"));

    QString transactionlist;

    MyMoneyTransactionFilter filter;
    filter.setDateFilter(_dtstart, QDate::currentDate());
    filter.addAccount(m_account.id());
    QList<MyMoneyTransaction> transactions = file->transactionList(filter);
    QList<MyMoneyTransaction>::const_iterator it_transaction = transactions.begin();
    while (it_transaction != transactions.end()) {
        transactionlist += transaction(*it_transaction);
        ++it_transaction;
    }

    return messageResponse("BANK", "STMT", Tag("STMTRS")
                           .element("CURDEF", "USD")
                           .subtag(Tag("BANKACCTFROM").element("BANKID", iban()).element("ACCTID", accountnum()).element("ACCTTYPE", "CHECKING"))
                           .subtag(Tag("BANKTRANLIST").element("DTSTART", dtstart_string).element("DTEND", dtnow_string).data(transactionlist))
                           .subtag(Tag("LEDGERBAL").element("BALAMT", file->balance(m_account.id()).formatMoney(QString(), 2, false)).element("DTASOF", dtnow_string)));
}

MyMoneyOfxConnector::Tag MyMoneyOfxConnector::creditCardStatementResponse(const QDate& _dtstart) const
{
    MyMoneyFile* file = MyMoneyFile::instance();

    QString dtstart_string = _dtstart.toString(Qt::ISODate).remove(QRegExp("[^0-9]"));
    QString dtnow_string = QDateTime::currentDateTime().toString(Qt::ISODate).remove(QRegExp("[^0-9]"));

    QString transactionlist;

    MyMoneyTransactionFilter filter;
    filter.setDateFilter(_dtstart, QDate::currentDate());
    filter.addAccount(m_account.id());
    QList<MyMoneyTransaction> transactions = file->transactionList(filter);
    QList<MyMoneyTransaction>::const_iterator it_transaction = transactions.begin();
    while (it_transaction != transactions.end()) {
        transactionlist += transaction(*it_transaction);
        ++it_transaction;
    }

    return messageResponse("CREDITCARD", "CCSTMT", Tag("CCSTMTRS")
                           .element("CURDEF", "USD")
                           .subtag(Tag("CCACCTFROM").element("ACCTID", accountnum()))
                           .subtag(Tag("BANKTRANLIST").element("DTSTART", dtstart_string).element("DTEND", dtnow_string).data(transactionlist))
                           .subtag(Tag("LEDGERBAL").element("BALAMT", file->balance(m_account.id()).formatMoney(QString(), 2, false)).element("DTASOF", dtnow_string)));
}

QString MyMoneyOfxConnector::investmentStatementResponse(const QDate& _dtstart) const
{
    MyMoneyFile* file = MyMoneyFile::instance();

    QString dtstart_string = _dtstart.toString(Qt::ISODate).remove(QRegExp("[^0-9]"));
    QString dtnow_string = QDateTime::currentDateTime().toString(Qt::ISODate).remove(QRegExp("[^0-9]"));

    QString transactionlist;

    MyMoneyTransactionFilter filter;
    filter.setDateFilter(_dtstart, QDate::currentDate());
    filter.addAccount(m_account.id());
    filter.addAccount(m_account.accountList());
    QList<MyMoneyTransaction> transactions = file->transactionList(filter);
    QList<MyMoneyTransaction>::const_iterator it_transaction = transactions.begin();
    while (it_transaction != transactions.end()) {
        transactionlist += investmentTransaction(*it_transaction);
        ++it_transaction;
    }

    Tag securitylist("SECLIST");
    QCStringList accountids = m_account.accountList();
    QCStringList::const_iterator it_accountid = accountids.begin();
    while (it_accountid != accountids.end()) {
        MyMoneySecurity equity = file->security(file->account(*it_accountid).currencyId());

        securitylist.subtag(Tag("STOCKINFO")
                            .subtag(Tag("SECINFO")
                                    .subtag(Tag("SECID")
                                            .element("UNIQUEID", equity.id())
                                            .element("UNIQUEIDTYPE", "KMYMONEY"))
                                    .element("SECNAME", equity.name())
                                    .element("TICKER", equity.tradingSymbol())
                                    .element("FIID", equity.id())));

        ++it_accountid;
    }

    return messageResponse("INVSTMT", "INVSTMT", Tag("INVSTMTRS")
                           .element("DTASOF", dtstart_string)
                           .element("CURDEF", "USD")
                           .subtag(Tag("INVACCTFROM").element("BROKERID", fiorg()).element("ACCTID", accountnum()))
                           .subtag(Tag("INVTRANLIST").element("DTSTART", dtstart_string).element("DTEND", dtnow_string).data(transactionlist))
                          )
           + Tag("SECLISTMSGSRSV1").subtag(securitylist);
}

MyMoneyOfxConnector::Tag MyMoneyOfxConnector::transaction(const MyMoneyTransaction& _t) const
{
    // This method creates a transaction tag using ONLY the elements that importer uses

    MyMoneyFile* file = MyMoneyFile::instance();

    //Use this version for bank/cc transactions
    MyMoneySplit s = _t.splitByAccount(m_account.id(), true);

    //TODO (Ace) Write "investmentTransaction()"...
    //Use this version for inv transactions
    //MyMoneySplit s = _t.splitByAccount( m_account.accountList(), true );

    Tag result("STMTTRN");

    result
    // This is a temporary hack.  I don't use the trntype field in importing at all,
    // but libofx requires it to be there in order to import the file.
    .element("TRNTYPE", "DEBIT")
    .element("DTPOSTED", _t.postDate().toString(Qt::ISODate).remove(QRegExp("[^0-9]")))
    .element("TRNAMT", s.value().formatMoney(QString(), 2, false));

    if (! _t.bankID().isEmpty())
        result.element("FITID", _t.bankID());
    else
        result.element("FITID", _t.id());

    if (! s.number().isEmpty())
        result.element("CHECKNUM", s.number());

    if (! s.payeeId().isEmpty())
        result.element("NAME", file->payee(s.payeeId()).name());

    if (! _t.memo().isEmpty())
        result.element("MEMO", _t.memo());

    return result;
}

MyMoneyOfxConnector::Tag MyMoneyOfxConnector::investmentTransaction(const MyMoneyTransaction& _t) const
{
    MyMoneyFile* file = MyMoneyFile::instance();

    //Use this version for inv transactions
    MyMoneySplit s = _t.splitByAccount(m_account.accountList(), true);

    QByteArray stockid = file->account(s.accountId()).currencyId();

    Tag invtran("INVTRAN");
    invtran.element("FITID", _t.id()).element("DTTRADE", _t.postDate().toString(Qt::ISODate).remove(QRegExp("[^0-9]")));
    if (!_t.memo().isEmpty())
        invtran.element("MEMO", _t.memo());

    if (s.action() == MyMoneySplit::actionName(eMyMoney::Split::Action::BuyShares)) {
        if (s.shares().isNegative()) {
            return Tag("SELLSTOCK")
                   .subtag(Tag("INVSELL")
                           .subtag(invtran)
                           .subtag(Tag("SECID").element("UNIQUEID", stockid).element("UNIQUEIDTYPE", "KMYMONEY"))
                           .element("UNITS", QString(((s.shares())).formatMoney(QString(), 2, false)).remove(QRegExp("[^0-9.\\-]")))
                           .element("UNITPRICE", QString((s.value() / s.shares()).formatMoney(QString(), 2, false)).remove(QRegExp("[^0-9.]")))
                           .element("TOTAL", QString((-s.value()).formatMoney(QString(), 2, false)).remove(QRegExp("[^0-9.\\-]")))
                           .element("SUBACCTSEC", "CASH")
                           .element("SUBACCTFUND", "CASH"))
                   .element("SELLTYPE", "SELL");
        } else {
            return Tag("BUYSTOCK")
                   .subtag(Tag("INVBUY")
                           .subtag(invtran)
                           .subtag(Tag("SECID").element("UNIQUEID", stockid).element("UNIQUEIDTYPE", "KMYMONEY"))
                           .element("UNITS", QString((s.shares()).formatMoney(QString(), 2, false)).remove(QRegExp("[^0-9.\\-]")))
                           .element("UNITPRICE", QString((s.value() / s.shares()).formatMoney(QString(), 2, false)).remove(QRegExp("[^0-9.]")))
                           .element("TOTAL", QString((-(s.value())).formatMoney(QString(), 2, false)).remove(QRegExp("[^0-9.\\-]")))
                           .element("SUBACCTSEC", "CASH")
                           .element("SUBACCTFUND", "CASH"))
                   .element("BUYTYPE", "BUY");
        }
    } else if (s.action() == MyMoneySplit::actionName(eMyMoney::Split::Action::ReinvestDividend)) {
        // Should the TOTAL tag really be negative for a REINVEST?  That's very strange, but
        // it's what they look like coming from my bank, and I can't find any information to refute it.

        return Tag("REINVEST")
               .subtag(invtran)
               .subtag(Tag("SECID").element("UNIQUEID", stockid).element("UNIQUEIDTYPE", "KMYMONEY"))
               .element("INCOMETYPE", "DIV")
               .element("TOTAL", QString((-s.value()).formatMoney(QString(), 2, false)).remove(QRegExp("[^0-9.\\-]")))
               .element("SUBACCTSEC", "CASH")
               .element("UNITS", QString((s.shares()).formatMoney(QString(), 2, false)).remove(QRegExp("[^0-9.\\-]")))
               .element("UNITPRICE", QString((s.value() / s.shares()).formatMoney(QString(), 2, false)).remove(QRegExp("[^0-9.]")));
    } else if (s.action() == MyMoneySplit::actionName(eMyMoney::Split::Action::Dividend)) {
        // find the split with the category, which has the actual amount of the dividend
        QList<MyMoneySplit> splits = _t.splits();
        QList<MyMoneySplit>::const_iterator it_split = splits.begin();
        bool found = false;
        while (it_split != splits.end()) {
            QByteArray accid = (*it_split).accountId();
            MyMoneyAccount acc = file->account(accid);
            if (acc.accountType() == eMyMoney::Account::Type::Income || acc.accountType() == eMyMoney::Account::Type::Expense) {
                found = true;
                break;
            }
            ++it_split;
        }

        if (found)
            return Tag("INCOME")
                   .subtag(invtran)
                   .subtag(Tag("SECID").element("UNIQUEID", stockid).element("UNIQUEIDTYPE", "KMYMONEY"))
                   .element("INCOMETYPE", "DIV")
                   .element("TOTAL", QString((-(*it_split).value()).formatMoney(QString(), 2, false)).remove(QRegExp("[^0-9\\.\\-]")))
                   .element("SUBACCTSEC", "CASH")
                   .element("SUBACCTFUND", "CASH");
        else
            return Tag("ERROR").element("DETAILS", "Unable to determine the amount of this income transaction.");
    }

    //FIXME: Do something useful with these errors
    return Tag("ERROR").element("DETAILS", "This transaction contains an unsupported action type");
}
#endif

KWallet::Wallet *openSynchronousWallet()
{
    using KWallet::Wallet;

    // first handle the simple case in which we already use the wallet but need the object again in
    // this case the wallet access permission dialog will no longer appear so we don't need to pass
    // a valid window id or do anything special since the function call should return immediately
    const bool alreadyUsingTheWallet = Wallet::users(Wallet::NetworkWallet()).contains("KMyMoney");
    if (alreadyUsingTheWallet) {
        return Wallet::openWallet(Wallet::NetworkWallet(), 0, Wallet::Synchronous);
    }

    // search for a suitable parent for the wallet that needs to be deactivated while the
    // wallet access permission dialog is not dismissed with either accept or reject
    KWallet::Wallet *wallet = 0;
    QWidget *parentWidgetForWallet = 0;
    if (qApp->activeModalWidget()) {
        parentWidgetForWallet = qApp->activeModalWidget();
    } else if (qApp->activeWindow()) {
        parentWidgetForWallet = qApp->activeWindow();
    } else {
        QList<KMainWindow *> mainWindowList = KMainWindow::memberList();
        if (!mainWindowList.isEmpty())
            parentWidgetForWallet = mainWindowList.front();
    }
    // only open the wallet synchronously if we have a valid parent otherwise crashes could occur
    if (parentWidgetForWallet) {
        // while the wallet is being opened disable the widget to prevent input processing
        const bool enabled = parentWidgetForWallet->isEnabled();
        parentWidgetForWallet->setEnabled(false);
        wallet = Wallet::openWallet(Wallet::NetworkWallet(), parentWidgetForWallet->winId(), Wallet::Synchronous);
        parentWidgetForWallet->setEnabled(enabled);
    }
    return wallet;
}
