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

// ----------------------------------------------------------------------------
// QT Includes

#include <QDateTime>
#include <QEventLoop>
#include <QFileInfo>
#include <QApplication>
#include <QRegExp>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QDomDocument>
#include <QDebug>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KIO/Job>
#include <KIO/TransferJob>
#include <KIO/CopyJob>
#include <KJobUiDelegate>
#include <KLocalizedString>
#include <KMessageBox>

// ----------------------------------------------------------------------------
// Some standard defined stuff collides with libofx.h
#ifdef Q_CC_MSVC
#undef ERROR
#undef DELETE
#endif

// ----------------------------------------------------------------------------
// Project Includes

namespace OfxPartner
{
bool post(const QString& request, const QMap<QString, QString>& attr, const QUrl &url, const QUrl& filename);
bool get(const QString& request, const QMap<QString, QString>& attr, const QUrl &url, const QUrl& filename);

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

  QUrl fname;

  QMap<QString, QString> attr;

  fname = QUrl("file://" + directory + kBankFilename);
  QDir dir;
  dir.mkpath(directory);

  QFileInfo i(fname.toLocalFile());
  if (needReload(i))
    get("", attr, QUrl(QStringLiteral("http://www.ofxhome.com/api.php?all=yes")), fname);
}

static void ParseFile(QMap<QString, QString>& result, const QString& fileName, const QString& bankName)
{
  QFile f(fileName);
  if (f.open(QIODevice::ReadOnly)) {
    QTextStream stream(&f);
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
    f.close();
  }
}

QStringList BankNames()
{
  QMap<QString, QString> result;

  // Make sure the index files are up to date
  ValidateIndexCache();

  ParseFile(result, directory + kBankFilename, QString());

  // Add Innovision
  result["Innovision"].clear();

  return QStringList() << result.keys();
}

QStringList FipidForBank(const QString& bank)
{
  QMap<QString, QString> result;

  ParseFile(result, directory + kBankFilename, bank);

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

OfxHomeServiceInfo ServiceInfo(const QString& fipid)
{
  OfxHomeServiceInfo result;
  memset(&result.ofxInfo, 0, sizeof(result.ofxInfo));
  result.ofxValidated = true;
  result.sslValidated = true;
  result.lastOfxValidated = QDate::currentDate().toString();
  result.lastSslValidated = result.lastOfxValidated;

  // Hard-coded values for Innovision test server
  if (fipid == "1") {
    strncpy(result.ofxInfo.fid, "00000", OFX_FID_LENGTH - 1);
    strncpy(result.ofxInfo.org, "ReferenceFI", OFX_ORG_LENGTH - 1);
    strncpy(result.ofxInfo.url, "https://ofx.innovision.com", OFX_URL_LENGTH - 1);
    result.ofxInfo.accountlist = 1;
    result.ofxInfo.statements = 1;
    result.ofxInfo.billpay = 1;
    result.ofxInfo.investments = 1;

    return result;
  }

  QMap<QString, QString> attr;

  QUrl guidFile(QString("file://%1fipid-%2.xml").arg(directory).arg(fipid));

  QFileInfo i(guidFile.toLocalFile());

  if (!i.isReadable() || i.lastModified().addDays(7) < QDateTime::currentDateTime())
    get("", attr, QUrl(QString("http://www.ofxhome.com/api.php?lookup=%1").arg(fipid)), guidFile);

  QFile f(guidFile.toLocalFile());
  if (f.open(QIODevice::ReadOnly)) {
    QTextStream stream(&f);
    stream.setCodec("UTF-8");
    QString msg;
    int errl, errc;
    QDomDocument doc;
    if (doc.setContent(stream.readAll(), &msg, &errl, &errc)) {
      const auto fid = extractNodeText(doc, "institution/fid");
      const auto org = extractNodeText(doc, "institution/org");
      const auto url = extractNodeText(doc, "institution/url");
      result.ofxValidated = (extractNodeText(doc, "institution/ofxfail").toUInt() == 0);
      result.sslValidated = (extractNodeText(doc, "institution/sslfail").toUInt() == 0);
      result.lastOfxValidated = extractNodeText(doc, "institution/lastofxvalidation");
      result.lastSslValidated = extractNodeText(doc, "institution/lastsslvalidation");

      strncpy(result.ofxInfo.fid, fid.toLatin1(), OFX_FID_LENGTH - 1);
      strncpy(result.ofxInfo.org, org.toLatin1(), OFX_ORG_LENGTH - 1);
      strncpy(result.ofxInfo.url, url.toLatin1(), OFX_URL_LENGTH - 1);

      result.ofxInfo.accountlist = true;
      result.ofxInfo.statements = true;
      result.ofxInfo.billpay = false;
      result.ofxInfo.investments = true;
    }
  }
  else
  {
    memset(&result.ofxInfo, 0, sizeof(result.ofxInfo));
    result.ofxValidated = false;
    result.sslValidated = false;
    result.lastOfxValidated.clear();
    result.lastSslValidated.clear();
    qDebug() << "OFX ServiceInfo:" << f.errorString();
  }
  return result;
}

bool get(const QString& request, const QMap<QString, QString>& attr, const QUrl &url, const QUrl& filename)
{
  Q_UNUSED(request);
  QByteArray req;
  OfxHttpRequest job("GET", url, req, attr, filename, false);

  return job.error() == 0;
}

bool post(const QString& request, const QMap<QString, QString>& attr, const QUrl &url, const QUrl& filename)
{
  QByteArray req(request.toUtf8());

  OfxHttpRequest job("POST", url, req, attr, filename, false);
  return job.error() == 0;
}

} // namespace OfxPartner

class OfxHttpRequest::Private
{
public:
  QFile  m_fpTrace;
};

OfxHttpRequest::OfxHttpRequest(const QString& type, const QUrl &url, const QByteArray &postData, const QMap<QString, QString>& metaData, const QUrl& dst, bool showProgressInfo)
    : d(new Private)
    , m_dst(dst)
    , m_error(-1)
    , m_postJob(0)
    , m_getJob(0)
{
  m_eventLoop = new QEventLoop(qApp->activeWindow());

  if (KMyMoneySettings::logOfxTransactions()) {
    QString logPath = KMyMoneySettings::logPath();
    d->m_fpTrace.setFileName(QString("%1/ofxlog.txt").arg(logPath));
    d->m_fpTrace.open(QIODevice::WriteOnly | QIODevice::Append);
  }

  KIO::JobFlag jobFlags = KIO::DefaultFlags;
  if (!showProgressInfo)
    jobFlags = KIO::HideProgressInfo;

  KIO::Job* job;
  if(type.toLower() == QStringLiteral("get")) {
    job = m_getJob = KIO::copy(url, dst, jobFlags);
  } else {
    job = m_postJob = KIO::http_post(url, postData, jobFlags);
    m_postJob->addMetaData("content-type", "Content-type: application/x-ofx");
    m_postJob->addMetaData(metaData);
    connect(job, SIGNAL(data(KIO::Job*,QByteArray)), this, SLOT(slotOfxData(KIO::Job*,QByteArray)));
    connect(job, SIGNAL(connected(KIO::Job*)), this, SLOT(slotOfxConnected(KIO::Job*)));
  }

  if (d->m_fpTrace.isOpen()) {
    QTextStream ts(&d->m_fpTrace);
    ts << "url: " << url.toDisplayString() << "\n";
    ts << "request:\n" << QString(postData) << "\n" << "response:\n";
  }

  connect(job, SIGNAL(result(KJob*)), this, SLOT(slotOfxFinished(KJob*)));

  job->start();

  qDebug("Starting eventloop");
  if (m_eventLoop)
    m_eventLoop->exec();
  qDebug("Ending eventloop");
}

OfxHttpRequest::~OfxHttpRequest()
{
  delete m_eventLoop;

  if (d->m_fpTrace.isOpen()) {
    d->m_fpTrace.close();
  }
  delete d;
}

void OfxHttpRequest::slotOfxConnected(KIO::Job*)
{
  qDebug() << "OfxHttpRequest::slotOfxConnected" << m_dst.toLocalFile();
  m_file.setFileName(m_dst.toLocalFile());
  m_file.open(QIODevice::WriteOnly);
}

void OfxHttpRequest::slotOfxData(KIO::Job*, const QByteArray& _ba)
{
  if (m_file.isOpen()) {
    m_file.write(_ba);

    if (d->m_fpTrace.isOpen()) {
      d->m_fpTrace.write(_ba);
    }
  }
}

void OfxHttpRequest::slotOfxFinished(KJob* /* e */)
{
  if (m_file.isOpen()) {
    m_file.close();
    if (d->m_fpTrace.isOpen()) {
      d->m_fpTrace.write("\nCompleted\n\n\n\n", 14);
    }
  }

  if(m_postJob) {
    m_error = m_postJob->error();
    if (m_error) {
      m_postJob->uiDelegate()->showErrorMessage();
      QFile::remove(m_dst.toLocalFile());

    } else if (m_postJob->isErrorPage()) {
      QString details;
      QFile f(m_dst.toLocalFile());
      if (f.open(QIODevice::ReadOnly)) {
        QTextStream stream(&f);
        while (!stream.atEnd()) {
          details += stream.readLine(); // line of text excluding '\n'
        }
        f.close();
      }
      KMessageBox::detailedSorry(0, i18n("The HTTP request failed."), details, i18nc("The HTTP request failed", "Failed"));
      QFile::remove(m_dst.toLocalFile());
    }

  } else if(m_getJob) {
    m_error = m_getJob->error();
    if (m_error) {
      m_getJob->uiDelegate()->showErrorMessage();
      QFile::remove(m_dst.toLocalFile());
    }
  }

  qDebug("Finishing eventloop");
  if (m_eventLoop)
    m_eventLoop->exit();
}
