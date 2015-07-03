/***************************************************************************
                             ofxpartner.h
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

#ifndef OFXPARTNER_H
#define OFXPARTNER_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QObject>
// TODO: port to KF5
#if 0
#include <QHttp>
#else
// in QT5 QHttp is no longer public
// dummy used for compilation
// port this to http://doc.qt.io/qt-5/qnetworkaccessmanager.html
struct QHttp
{
  enum Error {
    NoError,
    UnknownError,
    HostNotFound,
    ConnectionRefused,
    UnexpectedClose,
    InvalidResponseHeader,
    WrongContentLength,
    Aborted,
    AuthenticationRequiredError,
    ProxyAuthenticationRequiredError
  };
  QHttp(QString) {}
};
#endif
#include <QFile>
#include <QEventLoop>
#include <QPointer>

// ----------------------------------------------------------------------------
// KDE Includes

#include <QUrl>
class KJob;
namespace KIO
{
class Job;
class TransferJob;
}

// ----------------------------------------------------------------------------
// Project Includes

#include <libofx/libofx.h>

namespace OfxPartner
{
/**
  * setup the directory where the files will be stored.
  * @a dir must end with a '/' and must exist. Call this
  * before any other of the functions of OfxPartner. The
  * default will be to store the files in the current
  * directory.
  */
void setDirectory(const QString& dir);

void ValidateIndexCache();
OfxFiServiceInfo ServiceInfo(const QString& fipid);
QStringList BankNames();
QStringList FipidForBank(const QString& bank);

}

class OfxHttpRequest : public QObject
{
  Q_OBJECT
public:
  OfxHttpRequest(const QString& method, const QUrl &url, const QByteArray &postData, const QMap<QString, QString>& metaData, const QUrl& dst, bool showProgressInfo = true);
  virtual ~OfxHttpRequest();

  QHttp::Error error() const {
    return m_error;
  }

protected slots:
  void slotOfxFinished(int, bool);

private:
  QHttp*        m_job;
  QUrl          m_dst;
  QHttp::Error  m_error;
  QPointer<QEventLoop> m_eventLoop;
};

class OfxHttpsRequest : public QObject
{
  Q_OBJECT
public:
  OfxHttpsRequest(const QString& method, const QUrl &url, const QByteArray &postData, const QMap<QString, QString>& metaData, const QUrl& dst, bool showProgressInfo = true);
  virtual ~OfxHttpsRequest();

  QHttp::Error error() const {
    return m_error;
  }

protected slots:
  void slotOfxFinished(KJob*);
  void slotOfxData(KIO::Job*, const QByteArray&);
  void slotOfxConnected(KIO::Job*);

private:
  class Private;
  Private*          d;
  QUrl              m_dst;
  QFile             m_file;
  QHttp::Error      m_error;
  KIO::TransferJob* m_job;
  QPointer<QEventLoop> m_eventLoop;
};
#endif // OFXPARTNER_H
