/*
 * Copyright 2020       Thomas Baumgart <tbaumgart@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config-kmymoney.h"

#include "templatewriter.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDir>
#include <QList>
#include <QStandardPaths>
#include <QUrl>
#include <QTemporaryFile>
#include <QSaveFile>
#include <QDebug>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>
#include <KIO/StoredTransferJob>
#include <KJobWidgets>
#include <KXmlGuiWindow>
#include <KMessageBox>
#include <KTextEdit>

// ----------------------------------------------------------------------------
// Project Includes

#include "templatesmodel.h"
#include "mymoneytemplate.h"
#include "kmymoneyutils.h"
#include "mymoneyfile.h"
#include "mymoneyaccount.h"
#include "mymoneyexception.h"


class TemplateWriterPrivate
{
  Q_DISABLE_COPY(TemplateWriterPrivate)

public:
  TemplateWriterPrivate(TemplateWriter* qq)
  : q_ptr(qq)
  {
    m_doc = QDomDocument("KMYMONEY-TEMPLATE");

    QDomProcessingInstruction instruct = m_doc.createProcessingInstruction(QString("xml"), QString("version=\"1.0\" encoding=\"utf-8\""));
    m_doc.appendChild(instruct);

    m_mainElement = m_doc.createElement("kmymoney-account-template");
    m_doc.appendChild(m_mainElement);
  }

  ~TemplateWriterPrivate()
  {
  }

  void prepareDomDocument(const MyMoneyTemplate& tmpl)
  {
    QDomElement title = m_doc.createElement("title");
    QDomText t = m_doc.createTextNode(tmpl.title());
    title.appendChild(t);
    m_mainElement.appendChild(title);

    QDomElement shortDesc = m_doc.createElement("shortdesc");
    t = m_doc.createTextNode(tmpl.shortDescription());
    shortDesc.appendChild(t);
    m_mainElement.appendChild(shortDesc);

    QDomElement longDesc = m_doc.createElement("longdesc");
    t = m_doc.createTextNode(tmpl.longDescription());
    longDesc.appendChild(t);
    m_mainElement.appendChild(longDesc);
  }

  void collectAccounts()
  {
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

    QDomElement accounts = m_doc.createElement("accounts");
    m_mainElement.appendChild(accounts);

    addAccountStructure(accounts, MyMoneyFile::instance()->asset());
    addAccountStructure(accounts, MyMoneyFile::instance()->expense());
    addAccountStructure(accounts, MyMoneyFile::instance()->income());
    addAccountStructure(accounts, MyMoneyFile::instance()->liability());
    addAccountStructure(accounts, MyMoneyFile::instance()->equity());
  }

  void addAccountStructure(QDomElement& parent, const MyMoneyAccount& acc)
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
      std::sort(list.begin(), list.end(), [](MyMoneyAccount &a1, MyMoneyAccount &a2) {return a1.name() < a2.name();} );
      QList<MyMoneyAccount>::Iterator it;
      for (it = list.begin(); it != list.end(); ++it) {
        addAccountStructure(account, *it);
      }
    }
  }

  bool saveTemplate(const QUrl &url)
  {

    auto saveToLocalFile = [&](QSaveFile* qfile)
    {
      QTextStream stream(qfile);
      stream.setCodec("UTF-8");
      stream << m_doc.toString();
      stream.flush();
    };

    QString filename;


    if (!url.isValid()) {
      m_errMsg = i18n("Invalid template URL '%1'").arg(url.toDisplayString());
      return false;
    }

    if (url.isLocalFile()) {
      filename = url.toLocalFile();
      QSaveFile qfile(filename/*, 0600*/);
      if (qfile.open(QIODevice::WriteOnly)) {
        saveToLocalFile(&qfile);
        if (!qfile.commit()) {
          m_errMsg = i18n("Unable to write changes to '%1'").arg(filename);
          return false;
        }
      } else {
        m_errMsg = i18n("Unable to open template file '%1'").arg(filename);
        return false;
      }
    } else {
      QTemporaryFile tmpfile;
      tmpfile.open();
      QSaveFile qfile(tmpfile.fileName());
      if (qfile.open(QIODevice::WriteOnly)) {
        saveToLocalFile(&qfile);
        if (!qfile.commit()) {
          m_errMsg = i18n("Unable to write changes to '%1'").arg(tmpfile.fileName());
          return false;
        }
      } else {
        m_errMsg = i18n("Unable to open template file '%1'").arg(tmpfile.fileName());
        return false;
      }

      int permission = -1;
      QFile file(tmpfile.fileName());
      file.open(QIODevice::ReadOnly);
      KIO::StoredTransferJob *putjob = KIO::storedPut(file.readAll(), url, permission, KIO::JobFlag::Overwrite);
      if (!putjob->exec()) {
        m_errMsg = i18n("Unable to upload to '%1'.<br />%2").arg(url.toDisplayString(), putjob->errorString());
        return false;
      }
      file.close();
    }
    return true;
  }

public:
  TemplateWriter*                         q_ptr;
  QMap<QString,QString>                   m_vatAccountMap;
  QDomDocument                            m_doc;
  QDomElement                             m_mainElement;
  QString                                 m_errMsg;
};


TemplateWriter::TemplateWriter(QWidget* parent) :
  QObject(parent),
  d_ptr(new TemplateWriterPrivate(this))
{
}

TemplateWriter::~TemplateWriter()
{
  Q_D(TemplateWriter);
  delete d;
}


bool TemplateWriter::exportTemplate(const MyMoneyTemplate& tmpl, const QUrl &url)
{
  Q_D(TemplateWriter);

  d->prepareDomDocument(tmpl);
  d->collectAccounts();
  return d->saveTemplate(url);
}

QString TemplateWriter::errorMessage() const
{
  Q_D(const TemplateWriter);
  return d->m_errMsg;
}
