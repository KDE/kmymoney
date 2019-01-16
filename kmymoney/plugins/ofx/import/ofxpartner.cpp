/***************************************************************************
                             ofxpartner.cpp
                             ----------
    begin                : Fri Jan 23 2009
    copyright            : (C) 2009 by Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
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

#include "ofxpartner.h"
#include "kmymoneysettings.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

// ----------------------------------------------------------------------------
// QT Includes

#include <QDateTime>
#include <QEventLoop>
#include <QFileInfo>
#include <QApplication>
#include <QRegExp>
#include <QDir>
#include <QTextStream>
#include <QDomDocument>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kjob.h>
#include <kio/job.h>
#include <kio/jobuidelegate.h>
#include <klocale.h>
#include <kmessagebox.h>

// ----------------------------------------------------------------------------
// Some standard defined stuff collides with libofx.h
#ifdef Q_CC_MSVC
#undef ERROR
#undef DELETE
#endif

#define MSN 0
#define OFXHOME 1

// ----------------------------------------------------------------------------
// Project Includes

namespace OfxPartner
{
bool post(const QString& request, const QMap<QString, QString>& attr, const KUrl& url, const KUrl& filename);
bool get(const QString& request, const QMap<QString, QString>& attr, const KUrl& url, const KUrl& filename);

const QString kBankFilename = "ofx-bank-index.xml";
const QString kCcFilename = "ofx-cc-index.xml";
const QString kInvFilename = "ofx-inv-index.xml";

#define VER "9"

static QString directory;

void setDirectory(const QString& dir)
{
  directory = dir;
}

bool needReload(const QFileInfo& i)
{
  return ((!i.isReadable())
          || (i.lastModified().addDays(7) < QDateTime::currentDateTime())
          || (i.size() < 1024));
}

void ValidateIndexCache()
{
  // TODO (Ace) Check whether these files exist and are recent enough before getting them again

  KUrl fname;

  QMap<QString, QString> attr;

#if OFXHOME
  fname = directory + kBankFilename;
  QFileInfo i(fname.path());
  if (needReload(i))
    get("", attr, KUrl("http://www.ofxhome.com/api.php?all=yes"), fname);
#endif

#if MSN
  attr["content-type"] = "application/x-www-form-urlencoded";
  attr["accept"] = "*/*";

  fname = directory + kBankFilename;
  QFileInfo i(fname.path());
  if (needReload(i))
    post("T=1&S=*&R=1&O=0&TEST=0", attr, KUrl("http://moneycentral.msn.com/money/2005/mnynet/service/ols/filist.aspx?SKU=3&VER=" VER), fname);

  fname = directory + kCcFilename;
  i = QFileInfo(fname.path());
  if (needReload(i))
    post("T=2&S=*&R=1&O=0&TEST=0", attr, KUrl("http://moneycentral.msn.com/money/2005/mnynet/service/ols/filist.aspx?SKU=3&VER=" VER) , fname);

  fname = directory + kInvFilename;
  i = QFileInfo(fname.path());
  if (needReload(i))
    post("T=3&S=*&R=1&O=0&TEST=0", attr, KUrl("http://moneycentral.msn.com/money/2005/mnynet/service/ols/filist.aspx?SKU=3&VER=" VER), fname);
#endif
}

static void ParseFile(QMap<QString, QString>& result, const QString& fileName, const QString& bankName)
{
  QFile f(fileName);
  if (f.open(QIODevice::ReadOnly)) {
    QTextStream stream(&f);
#if OFXHOME
    stream.setCodec("UTF-8");
    QString msg;
    int errl, errc;
    QDomDocument doc;
    if (doc.setContent(stream.readAll(), &msg, &errl, &errc)) {
      QDomNodeList olist = doc.elementsByTagName("institutionid");
      for (int i = 0; i < olist.count(); ++i) {
        QDomNode onode = olist.item(i);
        if (onode.isElement()) {
          QDomElement elo = onode.toElement();
          QString name = elo.attribute("name");

          if (bankName.isEmpty())
            result[name].clear();

          else if (name == bankName) {
            result[elo.attribute("id")].clear();
          }
        }
      }
    }
#endif

#if MSN
    stream.setCodec("UTF-16");
    QString msg;
    int errl, errc;
    QDomDocument doc;
    if (doc.setContent(stream.readAll(), &msg, &errl, &errc)) {
      QDomNodeList olist = doc.elementsByTagName("prov");
      for (int i = 0; i < olist.count(); ++i) {
        QDomNode onode = olist.item(i);
        if (onode.isElement()) {
          bool collectGuid = false;
          QDomElement elo = onode.toElement();
          QDomNodeList ilist = onode.childNodes();
          for (int j = 0; j < ilist.count(); ++j) {
            QDomNode inode = ilist.item(j);
            QDomElement el = inode.toElement();
            if (el.tagName() == "name") {
              if (bankName.isEmpty())
                result[el.text()].clear();
              else if (el.text() == bankName) {
                collectGuid = true;
              }
            }
            if (el.tagName() == "guid" && collectGuid) {
              result[el.text()].clear();
            }
          }
        }
      }
    }
#endif
    f.close();
  }
}

QStringList BankNames()
{
  QMap<QString, QString> result;

  // Make sure the index files are up to date
  ValidateIndexCache();

  ParseFile(result, directory + kBankFilename, QString());
#if MSN
  ParseFile(result, directory + kCcFilename, QString());
  ParseFile(result, directory + kInvFilename, QString());
#endif

  // Add Innovision
  result["Innovision"].clear();

  return QStringList() << result.keys();
}

QStringList FipidForBank(const QString& bank)
{
  QMap<QString, QString> result;

  ParseFile(result, directory + kBankFilename, bank);
#if MSN
  ParseFile(result, directory + kCcFilename, bank);
  ParseFile(result, directory + kInvFilename, bank);
#endif

  // the fipid for Innovision is 1.
  if (bank == "Innovision")
    result["1"].clear();

  return QStringList() << result.keys();
}

QString extractNodeText(QDomElement& node, const QString& name)
{
  QString res;
  QRegExp exp("([^/]+)/?([^/].*)?");
  if (exp.indexIn(name) != -1) {
    QDomNodeList olist = node.elementsByTagName(exp.cap(1));
    if (olist.count()) {
      QDomNode onode = olist.item(0);
      if (onode.isElement()) {
        QDomElement elo = onode.toElement();
        if (exp.cap(2).isEmpty()) {
          res = elo.text();
        } else {
          res = extractNodeText(elo, exp.cap(2));
        }
      }
    }
  }
  return res;
}

QString extractNodeText(QDomDocument& doc, const QString& name)
{
  QString res;
  QRegExp exp("([^/]+)/?([^/].*)?");
  if (exp.indexIn(name) != -1) {
    QDomNodeList olist = doc.elementsByTagName(exp.cap(1));
    if (olist.count()) {
      QDomNode onode = olist.item(0);
      if (onode.isElement()) {
        QDomElement elo = onode.toElement();
        if (exp.cap(2).isEmpty()) {
          res = elo.text();
        } else {
          res = extractNodeText(elo, exp.cap(2));
        }
      }
    }
  }
  return res;
}

OfxFiServiceInfo ServiceInfo(const QString& fipid)
{
  OfxFiServiceInfo result;
  memset(&result, 0, sizeof(OfxFiServiceInfo));

  // Hard-coded values for Innovision test server
  if (fipid == "1") {
    strncpy(result.fid, "00000", OFX_FID_LENGTH - 1);
    strncpy(result.org, "ReferenceFI", OFX_ORG_LENGTH - 1);
    strncpy(result.url, "http://ofx.innovision.com", OFX_URL_LENGTH - 1);
    result.accountlist = 1;
    result.statements = 1;
    result.billpay = 1;
    result.investments = 1;

    return result;
  }

  QMap<QString, QString> attr;

  KUrl guidFile(QString("%1fipid-%2.xml").arg(directory).arg(fipid));

  // Apparently at some point in time, for VER=6 msn returned an online URL
  // to a static error page (http://moneycentral.msn.com/cust404.htm).
  // Increasing to VER=9 solved the problem. This may happen again in the
  // future.
  QFileInfo i(guidFile.path());

#if OFXHOME
  if (!i.isReadable() || i.lastModified().addDays(7) < QDateTime::currentDateTime())
    get("", attr, KUrl(QString("http://www.ofxhome.com/api.php?lookup=%1").arg(fipid)), guidFile);

  QFile f(guidFile.path());
  if (f.open(QIODevice::ReadOnly)) {
    QTextStream stream(&f);
    stream.setCodec("UTF-8");
    QString msg;
    int errl, errc;
    QDomDocument doc;
    if (doc.setContent(stream.readAll(), &msg, &errl, &errc)) {
      QString fid = extractNodeText(doc, "institution/fid");
      QString org = extractNodeText(doc, "institution/org");
      QString url = extractNodeText(doc, "institution/url");
      strncpy(result.fid, fid.toLatin1(), OFX_FID_LENGTH - 1);
      strncpy(result.org, org.toLatin1(), OFX_ORG_LENGTH - 1);
      strncpy(result.url, url.toLatin1(), OFX_URL_LENGTH - 1);

      result.accountlist = true;
      result.statements = true;
      result.billpay = false;
      result.investments = true;
    }
  }
#endif

#if MSN
  attr["content-type"] = "application/x-www-form-urlencoded";
  attr["accept"] = "*/*";

  if (!i.isReadable() || i.lastModified().addDays(7) < QDateTime::currentDateTime())
    get("", attr, KUrl(QString("http://moneycentral.msn.com/money/2005/mnynet/service/olsvcupd/OnlSvcBrandInfo.aspx?MSNGUID=&GUID=%1&SKU=3&VER=" VER).arg(fipid)), guidFile);

  QFile f(guidFile.path());
  if (f.open(QIODevice::ReadOnly)) {
    QTextStream stream(&f);
    stream.setCodec("UTF-16");
    QString msg;
    int errl, errc;
    QDomDocument doc;
    if (doc.setContent(stream.readAll(), &msg, &errl, &errc)) {
      QString fid = extractNodeText(doc, "ProviderSettings/FID");
      QString org = extractNodeText(doc, "ProviderSettings/Org");
      QString url = extractNodeText(doc, "ProviderSettings/ProviderURL");
      strncpy(result.fid, fid.toLatin1(), OFX_FID_LENGTH - 1);
      strncpy(result.org, org.toLatin1(), OFX_ORG_LENGTH - 1);
      strncpy(result.url, url.toLatin1(), OFX_URL_LENGTH - 1);
      result.accountlist = (extractNodeText(doc, "ProviderSettings/AcctListAvail") == "1");
      result.statements = (extractNodeText(doc, "BankingCapabilities/Bank") == "1");
      result.billpay = (extractNodeText(doc, "BillPayCapabilities/Pay") == "1");
      result.investments = (extractNodeText(doc, "InvestmentCapabilities/BrkStmt") == "1");
    }
  }
#endif

  return result;
}

bool get(const QString& request, const QMap<QString, QString>& attr, const KUrl& url, const KUrl& filename)
{
  Q_UNUSED(request);

  QByteArray req;
  OfxHttpRequest job("GET", url, req, attr, filename, true);

  return job.error() == QHttp::NoError;
}

bool post(const QString& request, const QMap<QString, QString>& attr, const KUrl& url, const KUrl& filename)
{
  QByteArray req(request.toAscii());

  OfxHttpRequest job("POST", url, req, attr, filename, true);
  return job.error() == QHttp::NoError;
}

} // namespace OfxPartner

class OfxHttpsRequest::Private
{
public:
  QFile  m_fpTrace;
};

OfxHttpsRequest::OfxHttpsRequest(const QString& type, const KUrl &url, const QByteArray &postData, const QMap<QString, QString>& metaData, const KUrl& dst, bool showProgressInfo) :
    d(new Private),
    m_dst(dst),
    m_error(QHttp::NoError)
{
  Q_UNUSED(type);
  Q_UNUSED(metaData);

  m_eventLoop = new QEventLoop(qApp->activeWindow());

  if (KMyMoneySettings::logOfxTransactions()) {
    QString logPath = KMyMoneySettings::logPath();
    d->m_fpTrace.setFileName(QString("%1/ofxlog.txt").arg(logPath));
    d->m_fpTrace.open(QIODevice::WriteOnly | QIODevice::Append);
  }

  KIO::JobFlag jobFlags = KIO::DefaultFlags;
  if (!showProgressInfo)
    jobFlags = KIO::HideProgressInfo;

  m_job = KIO::http_post(url, postData, jobFlags);
  m_job->addMetaData("content-type", "Content-type: application/x-ofx");

  if (d->m_fpTrace.isOpen()) {
    QTextStream ts(&d->m_fpTrace);
    ts << "url: " << url.prettyUrl() << "\n";
    ts << "request:\n" << QString(postData) << "\n" << "response:\n";
  }

  connect(m_job, SIGNAL(result(KJob*)), this, SLOT(slotOfxFinished(KJob*)));
  connect(m_job, SIGNAL(data(KIO::Job*,QByteArray)), this, SLOT(slotOfxData(KIO::Job*,QByteArray)));
  connect(m_job, SIGNAL(connected(KIO::Job*)), this, SLOT(slotOfxConnected(KIO::Job*)));

  qDebug("Starting eventloop");
  if (m_eventLoop)
    m_eventLoop->exec();
  qDebug("Ending eventloop");
}

OfxHttpsRequest::~OfxHttpsRequest()
{
  delete m_eventLoop;

  if (d->m_fpTrace.isOpen()) {
    d->m_fpTrace.close();
  }
  delete d;
}

void OfxHttpsRequest::slotOfxConnected(KIO::Job*)
{
  m_file.setFileName(m_dst.path());
  m_file.open(QIODevice::WriteOnly);
}

void OfxHttpsRequest::slotOfxData(KIO::Job*, const QByteArray& _ba)
{
  if (m_file.isOpen()) {
    m_file.write(_ba);

    if (d->m_fpTrace.isOpen()) {
      d->m_fpTrace.write(_ba);
    }
  }
}

void OfxHttpsRequest::slotOfxFinished(KJob* /* e */)
{
  if (m_file.isOpen()) {
    m_file.close();
    if (d->m_fpTrace.isOpen()) {
      d->m_fpTrace.write("\nCompleted\n\n\n\n", 14);
    }
  }

  int error = m_job->error();
  if (error) {
    m_job->ui()->setWindow(0);
    m_job->ui()->showErrorMessage();
//FIXME: FIX on windows
    unlink(m_dst.path().toUtf8().data());

  } else if (m_job->isErrorPage()) {
    QString details;
    QFile f(m_dst.path());
    if (f.open(QIODevice::ReadOnly)) {
      QTextStream stream(&f);
      QString line;
      while (!stream.atEnd()) {
        details += stream.readLine(); // line of text excluding '\n'
      }
      f.close();
    }
    KMessageBox::detailedSorry(0, i18n("The HTTP request failed."), details, i18nc("The HTTP request failed", "Failed"));
//FIXME: FIX on windows
    unlink(m_dst.path().toUtf8().data());
  }

  qDebug("Finishing eventloop");
  if (m_eventLoop)
    m_eventLoop->exit();
}



OfxHttpRequest::OfxHttpRequest(const QString& type, const KUrl &url, const QByteArray &postData, const QMap<QString, QString>& metaData, const KUrl& dst, bool showProgressInfo) :
    m_job(0)
{
  Q_UNUSED(showProgressInfo);

  m_eventLoop = new QEventLoop(qApp->activeWindow());
  QFile f(dst.path());
  m_error = QHttp::NoError;
  QString errorMsg;
  if (f.open(QIODevice::WriteOnly)) {
    m_job = new QHttp(url.host());
    QHttpRequestHeader header(type, url.encodedPathAndQuery());
    header.setValue("Host", url.host());
    QMap<QString, QString>::const_iterator it;
    for (it = metaData.begin(); it != metaData.end(); ++it) {
      header.setValue(it.key(), *it);
    }

    m_job->request(header, postData, &f);

    connect(m_job, SIGNAL(requestFinished(int,bool)),
            this, SLOT(slotOfxFinished(int,bool)));

    qDebug("Starting eventloop");
    m_eventLoop->exec();  // krazy:exclude=crashy
    qDebug("Ending eventloop");

    if (m_error != QHttp::NoError)
      errorMsg = m_job->errorString();

    delete m_job;
    m_job = 0;
  } else {
    m_error = QHttp::Aborted;
    errorMsg = i18n("Cannot open file %1 for writing", dst.path());
  }

  if (m_error != QHttp::NoError) {
    KMessageBox::error(0, errorMsg, i18n("OFX setup error"));
//FIXME: FIX on windows
    unlink(dst.path().toUtf8().data());
  }
}

OfxHttpRequest::~OfxHttpRequest()
{
  delete m_eventLoop;
}

void OfxHttpRequest::slotOfxFinished(int, bool rc)
{
  if (rc) {
    m_error = m_job->error();
  }
  qDebug("Finishing eventloop");
  if (m_eventLoop)
    m_eventLoop->exit();
}
