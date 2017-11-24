/***************************************************************************
                          mymoneytemplate.cpp  -  description
                             -------------------
    begin                : Sat Aug 14 2004
    copyright            : (C) 2004 by Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "mymoneytemplate.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QList>
#include <QSaveFile>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>
#include <KMessageBox>
#include <QTemporaryFile>
#include <KXmlGuiWindow>

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

  if (!url.isValid()) {
    qDebug("Invalid template URL '%s'", qPrintable(url.url()));
    return false;
  }

  m_source = url;
  if (url.isLocalFile()) {
    filename = url.toLocalFile();

  } else {
    bool rc = false;
    // TODO: port to kf5
    //rc = KIO::NetAccess::download(url, filename, KMyMoneyUtils::mainWindow());
    if (!rc) {
      KMessageBox::detailedError(KMyMoneyUtils::mainWindow(),
                                 i18n("Error while loading file '%1'.", url.url()),
                                 // TODO: port to kf5
                                 QString(),//KIO::NetAccess::lastErrorString(),
                                 i18n("File access error"));
      return false;
    }
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

  // if a temporary file was constructed by NetAccess::download,
  // then it will be removed with the next call. Otherwise, it
  // stays untouched on the local filesystem
  // TODO: port to kf5
  //KIO::NetAccess::removeTempFile(filename);
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
        eMyMoney::Account type = static_cast<eMyMoney::Account>(accounts.toElement().attribute("type").toUInt());
        switch (type) {
        case eMyMoney::Account::Asset:
        case eMyMoney::Account::Liability:
        case eMyMoney::Account::Income:
        case eMyMoney::Account::Expense:
        case eMyMoney::Account::Equity:
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
        case (uint)eMyMoney::Account::Asset:
          parent = file->asset();
          break;
        case (uint)eMyMoney::Account::Liability:
          parent = file->liability();
          break;
        case (uint)eMyMoney::Account::Income:
          parent = file->income();
          break;
        case (uint)eMyMoney::Account::Expense:
          parent = file->expense();
          break;
        case (uint)eMyMoney::Account::Equity:
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
              acc = *it;
              break;
            }
          }
        }
        if (it == subAccountList.constEnd()) {
          // not found, we need to create it
          acc.setName(accountElement.attribute("name"));
          acc.setAccountType(static_cast<eMyMoney::Account>(accountElement.attribute("type").toUInt()));
          setFlags(acc, account.firstChild());
          try {
            MyMoneyFile::instance()->addAccount(acc, parent);
          } catch (const MyMoneyException &) {
          }
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
          acc.setValue(value.toLatin1(), "Yes");
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

  // FIXME: add tax flag stuff
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
        throw MYMONEYEXCEPTION(i18n("Unable to write changes to '%1'", filename));
      }
    } else {
      throw MYMONEYEXCEPTION(i18n("Unable to write changes to '%1'", filename));
    }
  } else {
    QTemporaryFile tmpfile;
    QSaveFile qfile(tmpfile.fileName());
    if (qfile.open(QIODevice::WriteOnly)) {
      saveToLocalFile(&qfile);
      if (!qfile.commit()) {
        throw MYMONEYEXCEPTION(i18n("Unable to upload to '%1'", url.url()));
      }
    } else {
      throw MYMONEYEXCEPTION(i18n("Unable to upload to '%1'", url.url()));
    }
    // TODO: port to kf5
    //if (!KIO::NetAccess::upload(tmpfile.fileName(), url, 0))
    //  throw MYMONEYEXCEPTION(i18n("Unable to upload to '%1'", url.url()));
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
