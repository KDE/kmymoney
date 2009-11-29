/***************************************************************************
                             kgpgfile.cpp
                             -------------------
    copyright            : (C) 2004,2005,2009 by Thomas Baumgart <ipwizard@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


// ----------------------------------------------------------------------------
// Project Includes

#include "kgpgfile.h"

// C Includes
#include <unistd.h>

// ----------------------------------------------------------------------------
// QT Includes

#include <qglobal.h>
#include <QFile>
#include <QDir>
#include <QString>
#include <QByteArray>
#include <QBuffer>
#include <QList>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kstandarddirs.h>

#include <kleo/cryptplugwrapper.h>
#include <kleo/keylistjob.h>
#include <kleo/decryptjob.h>
#include <kleo/encryptjob.h>

#include <gpgme++/key.h>

class KGPGFile::Private {
  public:
    Private() : m_remain(0)
    {
      cw = new CryptPlugWrapper("gpg", "openpgp");
    }

    ~Private()
    {
      delete cw;
    }

    QString m_fn;
    QByteArray m_buffer;
    QEventLoop	evLoop;
    QFile* m_file;

    qint64  m_remain;

    std::vector< GpgME::Key > m_recipients;

    // the result set of the last key list job
    std::vector< GpgME::Key > m_keys;

    QStringList* keyList;
    CryptPlugWrapper* cw;
};



KGPGFile::KGPGFile(const QString& fn, const QString& homedir, const QString& options) :
  d(new Private)
{
  // only kept for interface compatibility
  Q_UNUSED(homedir);
  Q_UNUSED(options);

  setFileName(fn);
}

KGPGFile::~KGPGFile()
{
  close();
  delete d;
}

void KGPGFile::setFileName(const QString& fn)
{
  d->m_fn = fn;
  if(!fn.isEmpty() && fn[0] == '~') {
    d->m_fn = QDir::homePath()+fn.mid(1);

  } else if(QDir::isRelativePath(d->m_fn)) {
    QDir dir(fn);
    d->m_fn = dir.absolutePath();
  }
  // qDebug("setName: '%s'", d->m_fn.toLatin1().data());
}

void KGPGFile::flush(void)
{
  // no functionality
}

void KGPGFile::addRecipient(const QString& recipient)
{
  // skip a possible leading 0x in the id
  QString cmp = recipient;
  if(cmp.startsWith(QLatin1String("0x")))
    cmp = cmp.mid(2);

  QStringList patterns;
  patterns << cmp;
  QStringList keylist;
  keyList(keylist, false, patterns);

  if(d->m_keys.size() > 0)
    d->m_recipients.push_back(d->m_keys.front());
}

bool KGPGFile::open(OpenMode mode)
{
  // qDebug("KGPGFile::open(%d)", mode);
  if(isOpen()) {
    return false;
  }

  if(d->m_fn.isEmpty())
    return false;

  setOpenMode(mode);

  // qDebug("check valid access mode");
  if(!(isReadable() || isWritable()))
    return false;

  d->m_buffer.clear();

  if(isWritable()) {

    // qDebug("check recipient count");
    if(d->m_recipients.size() == 0) {
      setOpenMode(NotOpen);
      return false;
    }

    // qDebug("check access rights");
    if(!KStandardDirs::checkAccess(d->m_fn, W_OK))
      return false;
  }

  // open the 'physical' file
  d->m_file = new QFile;
  d->m_file->setFileName(d->m_fn);
  if(!d->m_file->open(mode)) {
    setOpenMode(NotOpen);
    return false;
  }
  return true;
}

void KGPGFile::close(void)
{
  if(!isOpen()) {
    return;
  }

  if(isWritable() && (d->m_buffer.size() != 0)) {

    // start the encryption job
    Kleo::EncryptJob* job = d->cw->encryptJob();
    connect(job, SIGNAL(result(const GpgME::EncryptionResult &, const QByteArray &, const QString &)),
            this, SLOT(slotEncryptJobResult(const GpgME::EncryptionResult &, const QByteArray &, const QString &)));

    const boost::shared_ptr<QBuffer> buffer(new QBuffer);
    buffer->setBuffer(&d->m_buffer);
    if(!buffer->open(QIODevice::ReadOnly)) {
      qDebug("Unable to open buffer in KGPGFile::close");
      return;
    }

    // start encryption job
    job->start(d->m_recipients, buffer, boost::shared_ptr<QIODevice>(), true);

    // wait for it to finish
    d->evLoop.exec();

    // write out the data to the file
    qint64 bytesWritten = d->m_file->write(d->m_buffer);
    if(bytesWritten != d->m_buffer.size()) {
      qDebug("Only %d of %d bytes written to file", qint32(bytesWritten & 0xFFFFFFFF), d->m_buffer.size());
  }
  }

  d->m_file->close();
  setOpenMode(NotOpen);

  d->m_recipients.clear();
  delete d->m_file;
  d->m_file = 0;
}

qint64 KGPGFile::writeData(const char *data, qint64 maxlen)
{
  if(!isOpen())
    return EOF;

  if(!isWritable())
    return EOF;

  qDebug("write %d bytes", qint32(maxlen & 0xFFFFFFFF));
  d->m_buffer.append(data, maxlen);

  return maxlen;
}

qint64 KGPGFile::readData(char *data, qint64 maxlen)
{
  if(maxlen == 0)
    return 0;

  if(!isOpen())
    return EOF;
  if(!isReadable())
    return EOF;

  // check if we need to fill the buffer
  if(d->m_buffer.isNull()) {
    d->m_buffer = d->m_file->readAll();

    Kleo::DecryptJob* job = d->cw->decryptJob();
    connect(job, SIGNAL(result(const GpgME::DecryptionResult &, const QByteArray &, const QString &, const GpgME::Error &)),
this, SLOT(slotDecryptJobResult(const GpgME::DecryptionResult &, const QByteArray &, const QString &, const GpgME::Error &)));

    const boost::shared_ptr<QBuffer> buffer(new QBuffer);
    buffer->setBuffer(&d->m_buffer);
    if(!buffer->open(QIODevice::ReadOnly))
      return EOF;

    // Start decryption job
    job->start(buffer);

    // wait for job to finish
    d->evLoop.exec();
    d->m_remain = 0;
  }

  // check if all was read
  if((d->m_buffer.size() - d->m_remain) == 0)
    return EOF;

  qint64 nread = maxlen;
  if((d->m_buffer.size() - d->m_remain) < nread) {
    nread = d->m_buffer.size() - d->m_remain;
  }
  memcpy(data, &d->m_buffer.data()[d->m_remain], nread);
  d->m_remain += nread;

  // qDebug("Read %d bytes", qint32(nread & 0xFFFFFFFF));
  return nread;
}

void KGPGFile::slotEncryptJobResult( const GpgME::EncryptionResult & result, const QByteArray & cipherText, const QString & auditLogAsHtml)
{
  Q_UNUSED(result)
  Q_UNUSED(auditLogAsHtml)

  // qDebug("Done encrypting: got %d bytes", cipherText.size());
  d->m_buffer = cipherText;
  d->evLoop.exit();
}


void KGPGFile::slotDecryptJobResult( const GpgME::DecryptionResult & result, const QByteArray & plainText, const QString & auditLogAsHtml, const GpgME::Error & auditLogError )
{
  Q_UNUSED(result)
  Q_UNUSED(auditLogAsHtml)
  Q_UNUSED(auditLogError)

  // qDebug("Done decrypting: got %d bytes", plainText.size());
  d->m_buffer = plainText;
  d->evLoop.exit();
}

bool KGPGFile::GPGAvailable(void)
{
  KGPGFile file;
  return file.d->cw->initStatus(0) == CryptPlugWrapper::InitStatus_Ok;
}

bool KGPGFile::keyAvailable(const QString& name)
{
  KGPGFile file;
  QStringList keys;
  QStringList patterns;
  patterns << name;
  file.keyList(keys, false, patterns);
  // qDebug("keyAvailable returns %d for '%s'", keys.count(), qPrintable(name));
  return keys.count() != 0;
}

void KGPGFile::publicKeyList(QStringList& list)
{
  // qDebug("Reading public keys");
  KGPGFile file;
  file.keyList(list);
}

void KGPGFile::secretKeyList(QStringList& list)
{
  // qDebug("Reading secrect keys");
  KGPGFile file;
  file.keyList(list, true);
}

void KGPGFile::keyList(QStringList& list, bool secretKeys, const QStringList& patterns)
{
  d->keyList = &list;
  list.clear();
  Kleo::KeyListJob* job = d->cw->keyListJob();

  connect(job, SIGNAL(nextKey(const GpgME::Key &)), this, SLOT(slotNextKey(const GpgME::Key &)));
  connect(job, SIGNAL(result(const GpgME::KeyListResult &, const std::vector<GpgME::Key> &, const QString &, const GpgME::Error &)), this, SLOT(slotKeyJobResult(const GpgME::KeyListResult &, const std::vector<GpgME::Key> &, const QString &, const GpgME::Error &)));

  GpgME::Error jobstatus = job->start(patterns, secretKeys);
  d->evLoop.exec();
}

void KGPGFile::slotNextKey(const GpgME::Key & key)
{
  std::vector<GpgME::UserID> userIDs = key.userIDs();
  for(unsigned int i = 0; i < userIDs.size(); ++i) {
    QString entry = QString("%1:%2").arg(key.shortKeyID()).arg(userIDs[i].id());
    *(d->keyList) += entry;
    // qDebug("Added ''%s'", qPrintable(entry));
  }
}

void KGPGFile::slotKeyJobResult(const GpgME::KeyListResult & result, const std::vector<GpgME::Key> & keys, const QString & auditLogAsHtml, const GpgME::Error & auditLogError )
{
  Q_UNUSED(result)
  Q_UNUSED(auditLogAsHtml)
  Q_UNUSED(auditLogError)

  d->m_keys = keys;
  d->evLoop.exit();
}


#include "kgpgfile.moc"
