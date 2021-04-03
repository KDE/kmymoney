/*
    SPDX-FileCopyrightText: 2004 Thomas Baumgart <ipwizard@users.sourceforge.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "mymoneytemplate.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QList>
#include <QSaveFile>
#include <QDebug>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>
#include <KMessageBox>
#include <QTemporaryFile>
#include <KXmlGuiWindow>
#include <KIO/StoredTransferJob>
#include <KJobWidgets>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyutils.h"
#include "mymoneyfile.h"
#include "mymoneyaccount.h"
#include "mymoneyexception.h"
#include "mymoneyenums.h"

MyMoneyTemplate::MyMoneyTemplate() :
    m_progressCallback(0),
    m_accountsRead(0)
{
}

MyMoneyTemplate::MyMoneyTemplate(const QUrl& url) :
    m_progressCallback(0),
    m_accountsRead(0)
{
    loadTemplate(url);
}

MyMoneyTemplate::~MyMoneyTemplate()
{
}

bool MyMoneyTemplate::loadTemplate(const QUrl &url)
{
    QString filename;
    bool downloadedFile = false;
    if (!url.isValid()) {
        qDebug("Invalid template URL '%s'", qPrintable(url.url()));
        return false;
    }

    m_source = url;
    if (url.isLocalFile()) {
        filename = url.toLocalFile();

    } else {
        downloadedFile = true;
        KIO::StoredTransferJob *transferjob = KIO::storedGet (url);
        KJobWidgets::setWindow(transferjob, KMyMoneyUtils::mainWindow());
        if (! transferjob->exec()) {
            KMessageBox::detailedError(KMyMoneyUtils::mainWindow(),
                                       i18n("Error while loading file '%1'.", url.url()),
                                       transferjob->errorString(),
                                       i18n("File access error"));
            return false;
        }
        QTemporaryFile file;
        file.setAutoRemove(false);
        file.open();
        file.write(transferjob->data());
        filename = file.fileName();
        file.close();
    }

    bool rc = true;
    QFile file(filename);
    QFileInfo info(file);
    if (!info.isFile()) {
        QString msg = i18n("<p><b>%1</b> is not a template file.</p>", filename);
        KMessageBox::error(KMyMoneyUtils::mainWindow(), msg, i18n("Filetype Error"));
        return false;
    }

    if (file.open(QIODevice::ReadOnly)) {
        QString errMsg;
        int errLine, errColumn;
        if (!m_doc.setContent(&file, &errMsg, &errLine, &errColumn)) {
            QString msg = i18n("<p>Error while reading template file <b>%1</b> in line %2, column %3</p>", filename, errLine, errColumn);
            KMessageBox::detailedError(KMyMoneyUtils::mainWindow(), msg, errMsg, i18n("Template Error"));
            rc = false;
        } else {
            rc = loadDescription();
        }
        file.close();
    } else {
        KMessageBox::sorry(KMyMoneyUtils::mainWindow(), i18n("File '%1' not found.", filename));
        rc = false;
    }

    // if a temporary file was downloaded, then it will be removed
    // with the next call. Otherwise, it stays untouched on the local
    // filesystem.
    if (downloadedFile) {
        QFile::remove(filename);
    }
    return rc;
}

bool MyMoneyTemplate::loadDescription()
{
    int validMask = 0x00;
    const int validAccount = 0x01;
    const int validTitle = 0x02;
    const int validShort = 0x04;
    const int validLong = 0x08;
    const int invalid = 0x10;
    const int validHeader = 0x0F;

    QDomElement rootElement = m_doc.documentElement();
    if (!rootElement.isNull()
            && rootElement.tagName() == "kmymoney-account-template") {
        QDomNode child = rootElement.firstChild();
        while (!child.isNull() && child.isElement()) {
            QDomElement childElement = child.toElement();
            // qDebug("MyMoneyTemplate::import: Processing child node %s", childElement.tagName().data());
            if (childElement.tagName() == "accounts") {
                m_accounts = childElement.firstChild();
                validMask |= validAccount;
            } else if (childElement.tagName() == "title") {
                m_title = childElement.text();
                validMask |= validTitle;
            } else if (childElement.tagName() == "shortdesc") {
                m_shortDesc = childElement.text();
                validMask |= validShort;
            } else if (childElement.tagName() == "longdesc") {
                m_longDesc = childElement.text();
                validMask |= validLong;
            } else {
                KMessageBox::error(KMyMoneyUtils::mainWindow(), i18n("<p>Invalid tag <b>%1</b> in template file <b>%2</b></p>", childElement.tagName(), m_source.toDisplayString()));
                validMask |= invalid;
            }
            child = child.nextSibling();
        }
    }
    return validMask == validHeader;
}

bool MyMoneyTemplate::hierarchy(QMap<QString, QTreeWidgetItem*>& list, const QString& parent, QDomNode account)
{
    bool rc = true;
    while (rc == true && !account.isNull()) {
        if (account.isElement()) {
            QDomElement accountElement = account.toElement();
            if (accountElement.tagName() == "account") {
                QString name = QString("%1:%2").arg(parent).arg(accountElement.attribute("name"));
                list[name] = 0;
                hierarchy(list, name, account.firstChild());
            }
        }
        account = account.nextSibling();
    }
    return rc;
}

void MyMoneyTemplate::hierarchy(QMap<QString, QTreeWidgetItem*>& list)
{
    bool rc = !m_accounts.isNull();
    QDomNode accounts = m_accounts;
    while (rc == true && !accounts.isNull() && accounts.isElement()) {
        QDomElement rootNode = accounts.toElement();
        QString name = rootNode.attribute("name");
        if (rootNode.tagName() == "account") {
            rootNode = rootNode.firstChild().toElement();
            eMyMoney::Account::Type type = static_cast<eMyMoney::Account::Type>(accounts.toElement().attribute("type").toUInt());
            switch (type) {
            case eMyMoney::Account::Type::Asset:
            case eMyMoney::Account::Type::Liability:
            case eMyMoney::Account::Type::Income:
            case eMyMoney::Account::Type::Expense:
            case eMyMoney::Account::Type::Equity:
                if (name.isEmpty())
                    name = MyMoneyAccount::accountTypeToString(type);
                list[name] = 0;
                rc = hierarchy(list, name, rootNode);
                break;

            default:
                rc = false;
                break;
            }
        } else {
            rc = false;
        }
        accounts = accounts.nextSibling();
    }
}

bool MyMoneyTemplate::importTemplate(void(*callback)(int, int, const QString&))
{
    m_progressCallback = callback;
    bool rc = !m_accounts.isNull();
    MyMoneyFile* file = MyMoneyFile::instance();
    signalProgress(0, m_doc.elementsByTagName("account").count(), i18n("Loading template %1", m_source.url()));
    m_accountsRead = 0;

    while (rc == true && !m_accounts.isNull() && m_accounts.isElement()) {
        QDomElement childElement = m_accounts.toElement();
        if (childElement.tagName() == "account") {
            ++m_accountsRead;
            MyMoneyAccount parent;
            switch (childElement.attribute("type").toUInt()) {
            case (uint)eMyMoney::Account::Type::Asset:
                parent = file->asset();
                break;
            case (uint)eMyMoney::Account::Type::Liability:
                parent = file->liability();
                break;
            case (uint)eMyMoney::Account::Type::Income:
                parent = file->income();
                break;
            case (uint)eMyMoney::Account::Type::Expense:
                parent = file->expense();
                break;
            case (uint)eMyMoney::Account::Type::Equity:
                parent = file->equity();
                break;

            default:
                KMessageBox::error(KMyMoneyUtils::mainWindow(), i18n("<p>Invalid top-level account type <b>%1</b> in template file <b>%2</b></p>", childElement.attribute("type"), m_source.toDisplayString()));
                rc = false;
            }

            if (rc == true) {
                if (childElement.attribute("name").isEmpty())
                    rc = createAccounts(parent, childElement.firstChild());
                else
                    rc = createAccounts(parent, childElement);
            }
        } else {
            rc = false;
        }
        m_accounts = m_accounts.nextSibling();
    }

    /*
     * Resolve imported vat account assignments
     *
     * The template account id of the assigned vat account
     * is stored temporarly in the account key/value pair
     * 'UnresolvedVatAccount' and resolved below.
     */
    QList<MyMoneyAccount> accounts;
    file->accountList(accounts);
    foreach (MyMoneyAccount acc, accounts) {
        if (!acc.pairs().contains("UnresolvedVatAccount"))
            continue;
        QString id = acc.value("UnresolvedVatAccount");
        acc.setValue("VatAccount", m_vatAccountMap[id]);
        acc.deletePair("UnresolvedVatAccount");
        MyMoneyFile::instance()->modifyAccount(acc);
    }

    signalProgress(-1, -1);
    return rc;
}

bool MyMoneyTemplate::createAccounts(MyMoneyAccount& parent, QDomNode account)
{
    bool rc = true;
    while (rc == true && !account.isNull()) {
        MyMoneyAccount acc;
        if (account.isElement()) {
            QDomElement accountElement = account.toElement();
            if (accountElement.tagName() == "account") {
                signalProgress(++m_accountsRead, 0);
                QList<MyMoneyAccount> subAccountList;
                QList<MyMoneyAccount>::ConstIterator it;
                it = subAccountList.constEnd();
                if (!parent.accountList().isEmpty()) {
                    MyMoneyFile::instance()->accountList(subAccountList, parent.accountList());
                    for (it = subAccountList.constBegin(); it != subAccountList.constEnd(); ++it) {
                        if ((*it).name() == accountElement.attribute("name")) {
                            qWarning() << "account" << (*it).name() << "already present";
                            acc = *it;
                            QString id = accountElement.attribute("id");
                            if (!id.isEmpty())
                                m_vatAccountMap[id] = acc.id();
                            break;
                        }
                    }
                }
                if (it == subAccountList.constEnd()) {
                    // not found, we need to create it
                    acc.setName(accountElement.attribute("name"));
                    acc.setAccountType(static_cast<eMyMoney::Account::Type>(accountElement.attribute("type").toUInt()));
                    setFlags(acc, account.firstChild());
                    try {
                        MyMoneyFile::instance()->addAccount(acc, parent);
                    } catch (const MyMoneyException &) {
                    }
                    QString id = accountElement.attribute("id");
                    if (!id.isEmpty())
                        m_vatAccountMap[id] = acc.id();
                }
                createAccounts(acc, account.firstChild());
            }
        }
        account = account.nextSibling();
    }
    return rc;
}

bool MyMoneyTemplate::setFlags(MyMoneyAccount& acc, QDomNode flags)
{
    bool rc = true;
    while (rc == true && !flags.isNull()) {
        if (flags.isElement()) {
            QDomElement flagElement = flags.toElement();
            if (flagElement.tagName() == "flag") {
                // make sure, we only store flags we know!
                QString value = flagElement.attribute("name");
                if (value == "Tax") {
                    acc.setValue(value, "Yes");
                } else if (value == "VatRate") {
                    acc.setValue(value, flagElement.attribute("value"));
                } else if (value == "VatAccount") {
                    // will be resolved later in importTemplate()
                    acc.setValue("UnresolvedVatAccount", flagElement.attribute("value"));
                } else if (value == "OpeningBalanceAccount") {
                    acc.setValue("OpeningBalanceAccount", "Yes");
                } else {
                    KMessageBox::error(KMyMoneyUtils::mainWindow(), i18n("<p>Invalid flag type <b>%1</b> for account <b>%3</b> in template file <b>%2</b></p>", flagElement.attribute("name"), m_source.toDisplayString(), acc.name()));
                    rc = false;
                }
                QString currency = flagElement.attribute("currency");
                if (!currency.isEmpty())
                    acc.setCurrencyId(currency);
            }
        }
        flags = flags.nextSibling();
    }
    return rc;
}

void MyMoneyTemplate::signalProgress(int current, int total, const QString& msg)
{
    if (m_progressCallback != 0)
        (*m_progressCallback)(current, total, msg);
}

bool MyMoneyTemplate::exportTemplate(void(*callback)(int, int, const QString&))
{
    m_progressCallback = callback;

    // prepare vat account map
    QList<MyMoneyAccount> accountList;
    MyMoneyFile::instance()->accountList(accountList);
    int i = 0;
    QList<MyMoneyAccount>::Iterator it;
    for (it = accountList.begin(); it != accountList.end(); ++it) {
        if (!(*it).pairs().contains("VatAccount"))
            continue;
        m_vatAccountMap[(*it).value("VatAccount")] = QString::fromLatin1("%1").arg(i++, 3, 10, QLatin1Char('0'));
    }

    m_doc = QDomDocument("KMYMONEY-TEMPLATE");

    QDomProcessingInstruction instruct = m_doc.createProcessingInstruction(QString("xml"), QString("version=\"1.0\" encoding=\"utf-8\""));
    m_doc.appendChild(instruct);

    QDomElement mainElement = m_doc.createElement("kmymoney-account-template");
    m_doc.appendChild(mainElement);

    QDomElement title = m_doc.createElement("title");
    QDomText t = m_doc.createTextNode(m_title);
    title.appendChild(t);
    mainElement.appendChild(title);

    QDomElement shortDesc = m_doc.createElement("shortdesc");
    t = m_doc.createTextNode(m_shortDesc);
    shortDesc.appendChild(t);
    mainElement.appendChild(shortDesc);

    QDomElement longDesc = m_doc.createElement("longdesc");
    t = m_doc.createTextNode(m_longDesc);
    longDesc.appendChild(t);
    mainElement.appendChild(longDesc);

    QDomElement accounts = m_doc.createElement("accounts");
    mainElement.appendChild(accounts);

    addAccountStructure(accounts, MyMoneyFile::instance()->asset());
    addAccountStructure(accounts, MyMoneyFile::instance()->expense());
    addAccountStructure(accounts, MyMoneyFile::instance()->income());
    addAccountStructure(accounts, MyMoneyFile::instance()->liability());
    addAccountStructure(accounts, MyMoneyFile::instance()->equity());

    return true;
}

const QString& MyMoneyTemplate::title() const
{
    return m_title;
}

const QString& MyMoneyTemplate::shortDescription() const
{
    return m_shortDesc;
}

const QString& MyMoneyTemplate::longDescription() const
{
    return m_longDesc;
}

void MyMoneyTemplate::setTitle(const QString &s)
{
    m_title = s;
}

void MyMoneyTemplate::setShortDescription(const QString &s)
{
    m_shortDesc = s;
}

void MyMoneyTemplate::setLongDescription(const QString &s)
{
    m_longDesc = s;
}

static bool nameLessThan(MyMoneyAccount &a1, MyMoneyAccount &a2)
{
    return a1.name() < a2.name();
}

bool MyMoneyTemplate::addAccountStructure(QDomElement& parent, const MyMoneyAccount& acc)
{
    QDomElement account = m_doc.createElement("account");
    parent.appendChild(account);

    if (MyMoneyFile::instance()->isStandardAccount(acc.id()))
        account.setAttribute(QString("name"), QString());
    else
        account.setAttribute(QString("name"), acc.name());
    account.setAttribute(QString("type"), (int)acc.accountType());

    if (acc.pairs().contains("Tax")) {
        QDomElement flag = m_doc.createElement("flag");
        flag.setAttribute(QString("name"), "Tax");
        flag.setAttribute(QString("value"), acc.value("Tax"));
        account.appendChild(flag);
    }
    if (m_vatAccountMap.contains(acc.id()))
        account.setAttribute(QString("id"), m_vatAccountMap[acc.id()]);

    if (acc.pairs().contains("VatRate")) {
        QDomElement flag = m_doc.createElement("flag");
        flag.setAttribute(QString("name"), "VatRate");
        flag.setAttribute(QString("value"), acc.value("VatRate"));
        account.appendChild(flag);
    }
    if (acc.pairs().contains("VatAccount")) {
        QDomElement flag = m_doc.createElement("flag");
        flag.setAttribute(QString("name"), "VatAccount");
        flag.setAttribute(QString("value"), m_vatAccountMap[acc.value("VatAccount")]);
        account.appendChild(flag);
    }
    if (acc.pairs().contains("OpeningBalanceAccount")) {
        QString openingBalanceAccount = acc.value("OpeningBalanceAccount");
        if (openingBalanceAccount == "Yes") {
            QDomElement flag = m_doc.createElement("flag");
            flag.setAttribute(QString("name"), "OpeningBalanceAccount");
            flag.setAttribute(QString("currency"), acc.currencyId());
            account.appendChild(flag);
        }
    }

    // any child accounts?
    if (acc.accountList().count() > 0) {
        QList<MyMoneyAccount> list;
        MyMoneyFile::instance()->accountList(list, acc.accountList(), false);
        qSort(list.begin(), list.end(), nameLessThan);
        QList<MyMoneyAccount>::Iterator it;
        for (it = list.begin(); it != list.end(); ++it) {
            addAccountStructure(account, *it);
        }
    }
    return true;
}

bool MyMoneyTemplate::saveTemplate(const QUrl &url)
{
    QString filename;

    if (!url.isValid()) {
        qDebug("Invalid template URL '%s'", qPrintable(url.url()));
        return false;
    }

    if (url.isLocalFile()) {
        filename = url.toLocalFile();
        QSaveFile qfile(filename/*, 0600*/);
        if (qfile.open(QIODevice::WriteOnly)) {
            saveToLocalFile(&qfile);
            if (!qfile.commit()) {
                throw MYMONEYEXCEPTION(QString::fromLatin1("Unable to write changes to '%1'").arg(filename));
            }
        } else {
            throw MYMONEYEXCEPTION(QString::fromLatin1("Unable to write changes to '%1'").arg(filename));
        }
    } else {
        QTemporaryFile tmpfile;
        tmpfile.open();
        QSaveFile qfile(tmpfile.fileName());
        if (qfile.open(QIODevice::WriteOnly)) {
            saveToLocalFile(&qfile);
            if (!qfile.commit()) {
                throw MYMONEYEXCEPTION(QString::fromLatin1("Unable to upload to '%1'").arg(url.toDisplayString()));
            }
        } else {
            throw MYMONEYEXCEPTION(QString::fromLatin1("Unable to upload to '%1'").arg(url.toDisplayString()));
        }
        int permission = -1;
        QFile file(tmpfile.fileName());
        file.open(QIODevice::ReadOnly);
        KIO::StoredTransferJob *putjob = KIO::storedPut(file.readAll(), url, permission, KIO::JobFlag::Overwrite);
        if (!putjob->exec()) {
            throw MYMONEYEXCEPTION(QString::fromLatin1("Unable to upload to '%1'.<br />%2").arg(url.toDisplayString(), putjob->errorString()));
        }
        file.close();
    }
    return true;
}

bool MyMoneyTemplate::saveToLocalFile(QSaveFile* qfile)
{
    QTextStream stream(qfile);
    stream.setCodec("UTF-8");
    stream << m_doc.toString();
    stream.flush();

    return true;
}
