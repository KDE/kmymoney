/*
    SPDX-FileCopyrightText: 2009-2020 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef OFXPARTNER_H
#define OFXPARTNER_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QObject>
#include <QFile>
#include <QPointer>

// ----------------------------------------------------------------------------
// KDE Includes

#include <QUrl>
class KJob;
class QEventLoop;
namespace KIO
{
class Job;
class TransferJob;
}

// ----------------------------------------------------------------------------
// Project Includes

#include <libofx/libofx.h>

struct OfxHomeServiceInfo {
  OfxFiServiceInfo  ofxInfo;
  bool ofxValidated;
  bool sslValidated;
  QString lastOfxValidated;
  QString lastSslValidated;
};

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
OfxHomeServiceInfo ServiceInfo(const QString& fipid);
QStringList BankNames();
QStringList FipidForBank(const QString& bank);

}

class OfxHttpRequest : public QObject
{
  Q_OBJECT
public:
  OfxHttpRequest(const QString& method, const QUrl &url, const QByteArray &postData, const QMap<QString, QString>& metaData, const QUrl& dst, bool showProgressInfo = true);
  virtual ~OfxHttpRequest();

  /**
   * returns the error code provided by KIO::TransferJob or
   * KIO::Job depending on post() or get() operation. If it
   * is not set by the actual operation, it returns -1.
   */
  int error() const {
    return m_error;
  }

protected Q_SLOTS:
  void slotOfxFinished(KJob*);
  void slotOfxData(KIO::Job*, const QByteArray&);
  void slotOfxConnected(KIO::Job*);

private:
  class Private;
  Private*          d;
  QString           m_dst;
  QFile             m_file;
  int               m_error;
  KIO::TransferJob* m_postJob;
  KIO::Job*         m_getJob;
  QPointer<QEventLoop> m_eventLoop;
};
#endif // OFXPARTNER_H
