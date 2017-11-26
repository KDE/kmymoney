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

#include <config-kmymoney.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kgpgfile.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <vector>
#include <qglobal.h>
#include <QFile>
#include <QDir>
#include <QString>
#include <QByteArray>
#include <QList>
#include <QSaveFile>
#include <QDateTime>
#include <QStringList>

// ----------------------------------------------------------------------------
// KDE Includes

#ifdef Gpgmepp_FOUND
#include <gpgme++/context.h>
#include <gpgme++/encryptionresult.h>
#include <gpgme++/decryptionresult.h>
#include <gpgme++/keylistresult.h>
#include <gpgme++/key.h>
#include <gpgme++/data.h>

class KGPGFile::Private
{
public:
  Private() {
    m_fileRead = 0;
    m_fileWrite = 0;
    GpgME::initializeLibrary();
    ctx = GpgME::Context::createForProtocol(GpgME::OpenPGP);
    if (!ctx)
      qDebug("Failed to create the GpgME context for the OpenPGP protocol");
  }

  ~Private() {
    delete ctx;
  }

  QString m_fn;
  QFile* m_fileRead;
  QSaveFile* m_fileWrite;

  GpgME::Error m_lastError;

  GpgME::Context* ctx;
  GpgME::Data m_data;

  std::vector< GpgME::Key > m_recipients;

  // the result set of the last key list job
  std::vector< GpgME::Key > m_keys;
};



KGPGFile::KGPGFile(const QString& fn, const QString& homedir, const QString& options) :
    d(new Private)
{
  // only kept for interface compatibility
  Q_UNUSED(homedir);
  Q_UNUSED(options);

  KGPGFile::setFileName(fn);
}

KGPGFile::~KGPGFile()
{
  close();
  delete d;
}

void KGPGFile::setFileName(const QString& fn)
{
  d->m_fn = fn;
  if (!fn.isEmpty() && fn[0] == '~') {
    d->m_fn = QDir::homePath() + fn.mid(1);

  } else if (QDir::isRelativePath(d->m_fn)) {
    QDir dir(fn);
    d->m_fn = dir.absolutePath();
  }
  // qDebug("setName: '%s'", d->m_fn.toLatin1().data());
}

void KGPGFile::flush()
{
  // no functionality
}

void KGPGFile::addRecipient(const QString& recipient)
{
  // skip a possible leading 0x in the id
  QString cmp = recipient;
  if (cmp.startsWith(QLatin1String("0x")))
    cmp = cmp.mid(2);

  QStringList keylist;
  keyList(keylist, false, cmp);

  if (d->m_keys.size() > 0)
    d->m_recipients.push_back(d->m_keys.front());
}

bool KGPGFile::open(OpenMode mode)
{
  if (isOpen()) {
    return false;
  }

  if (d->m_fn.isEmpty()) {
    setOpenMode(NotOpen);
    return false;
  }

  if (!d->ctx) {
    setOpenMode(NotOpen);
    return false;
  }

  setOpenMode(mode);

  if (!(isReadable() || isWritable())) {
    setOpenMode(NotOpen);
    return false;
  }

  if (isWritable()) {

    if (d->m_recipients.empty()) {
      setOpenMode(NotOpen);
      return false;
    }

    // write out in ASCII armor mode
    d->ctx->setArmor(true);
    d->m_fileWrite = new QSaveFile;

  } else if (isReadable()) {
    d->m_fileRead = new QFile;
  }

  // open the 'physical' file
  // Since some of the methods in QFile are not virtual, we need to
  // differentiate here between the QFile* and the QSaveFile* case
  if (isReadable()) {
    d->m_fileRead->setFileName(d->m_fn);
    if (!d->m_fileRead->open(mode)) {
      setOpenMode(NotOpen);
      return false;
    }
    GpgME::Data dcipher(d->m_fileRead->handle());
    d->m_lastError = d->ctx->decrypt(dcipher, d->m_data).error();
    if (d->m_lastError.encodedError()) {
      return false;
    }
    d->m_data.seek(0, SEEK_SET);

  } else if (isWritable()) {
    d->m_fileWrite->setFileName(d->m_fn);
    if (!d->m_fileWrite->open(mode)) {
      setOpenMode(NotOpen);
      return false;
    }
  }

  return true;
}

void KGPGFile::close()
{
  if (!isOpen()) {
    return;
  }

  if (!d->ctx)
    return;

  if (isWritable()) {
    d->m_data.seek(0, SEEK_SET);
    GpgME::Data dcipher(d->m_fileWrite->handle());
    d->m_lastError = d->ctx->encrypt(d->m_recipients, d->m_data, dcipher, GpgME::Context::AlwaysTrust).error();
    if (d->m_lastError.encodedError()) {
      setErrorString(QLatin1String("Failure while writing temporary file for file: '") + QLatin1String(d->m_lastError.asString()) + QLatin1String("'"));
    } else if (!d->m_fileWrite->commit()) {
      setErrorString("Failure while committing file changes.");
    }
  }

  delete d->m_fileWrite;
  delete d->m_fileRead;
  d->m_fileWrite = 0;
  d->m_fileRead = 0;
  d->m_recipients.clear();
  setOpenMode(NotOpen);
}

qint64 KGPGFile::writeData(const char *data, qint64 maxlen)
{
  if (!isOpen())
    return EOF;

  if (!isWritable())
    return EOF;

  // qDebug("write %d bytes", qint32(maxlen & 0xFFFFFFFF));

  // write out the data and make sure that we do not cross
  // size_t boundaries.
  qint64 bytesWritten = 0;
  while (maxlen) {
    qint64 len = 2 ^ 31;
    if (len > maxlen)
      len = maxlen;
    bytesWritten += d->m_data.write(data, len);
    data = &data[len];
    maxlen -= len;
  }
  // qDebug("%d bytes written", qint32(bytesWritten & 0xFFFFFFFF));
  return bytesWritten;
}

qint64 KGPGFile::readData(char *data, qint64 maxlen)
{
  if (maxlen == 0)
    return 0;

  if (!isOpen())
    return EOF;
  if (!isReadable())
    return EOF;

  // read requested block of data and make sure that we do not cross
  // size_t boundaries.
  qint64 bytesRead = 0;
  while (maxlen) {
    qint64 len = 2 ^ 31;
    if (len > maxlen)
      len = maxlen;
    bytesRead += d->m_data.read(data, len);
    data = &data[len];
    maxlen -= len;
  }
  return bytesRead;
}

QString KGPGFile::errorToString() const
{
  return QString::fromUtf8(d->m_lastError.asString());
}

bool KGPGFile::GPGAvailable()
{
  GpgME::initializeLibrary();
  bool rc = (GpgME::checkEngine(GpgME::OpenPGP) == 0);
  // qDebug("KGPGFile::GPGAvailable returns %d", rc);
  return rc;
}

bool KGPGFile::keyAvailable(const QString& name)
{
  KGPGFile file;
  QStringList keys;
  file.keyList(keys, false, name);
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

QDateTime KGPGFile::keyExpires(const QString& name)
{
  QDateTime expirationDate;

  // skip a possible leading 0x in the id
  QString cmp = name;
  if (cmp.startsWith(QLatin1String("0x")))
    cmp = cmp.mid(2);

  QStringList keylist;
  keyList(keylist, false, cmp);

  // in case we have no or more than one matching key
  // or the key does not have subkeys, we return an invalid date
  if (d->m_keys.size() == 1 && d->m_keys[0].subkeys().size() > 0 && !d->m_keys[0].subkeys()[0].neverExpires()) {
    expirationDate.setTime_t(d->m_keys[0].subkeys()[0].expirationTime());
  }
  return expirationDate;
}

void KGPGFile::keyList(QStringList& list, bool secretKeys, const QString& pattern)
{
  d->m_keys.clear();
  list.clear();
  if (d->ctx && !d->ctx->startKeyListing(pattern.toUtf8().constData(), secretKeys)) {
    GpgME::Error error;
    for (;;) {
      GpgME::Key key;
      key = d->ctx->nextKey(error);
      if (error.encodedError() != GPG_ERR_NO_ERROR)
        break;

      bool needPushBack = true;

      std::vector<GpgME::UserID> userIDs = key.userIDs();
      std::vector<GpgME::Subkey> subkeys = key.subkeys();
      for (unsigned int i = 0; i < userIDs.size(); ++i) {
        if (subkeys.size() > 0) {
          for (unsigned int j = 0; j < subkeys.size(); ++j) {
            const GpgME::Subkey& skey = subkeys[j];

            if (((skey.canEncrypt() && !secretKeys) || (skey.isSecret() && secretKeys))

                &&  !(skey.isRevoked() || skey.isExpired() || skey.isInvalid()  || skey.isDisabled())) {
              QString entry = QString("%1:%2").arg(key.shortKeyID()).arg(userIDs[i].id());
              list += entry;
              if (needPushBack) {
                d->m_keys.push_back(key);
                needPushBack = false;
              }
            } else {
              // qDebug("Skip key '%s'", key.shortKeyID());
            }
          }
        } else {
          // we have no subkey, so we operate on the main key
          if (((key.canEncrypt() && !secretKeys) || (key.hasSecret() && secretKeys))
              && !(key.isRevoked() || key.isExpired() || key.isInvalid()  || key.isDisabled())) {
            QString entry = QString("%1:%2").arg(key.shortKeyID()).arg(userIDs[i].id());
            list += entry;
            if (needPushBack) {
              d->m_keys.push_back(key);
              needPushBack = false;
            }
          } else {
            // qDebug("Skip key '%s'", key.shortKeyID());
          }
        }
      }
    }
    d->ctx->endKeyListing();
  }
}

#else // not Gpgmepp_FOUND

// NOOP implementation
KGPGFile::KGPGFile(const QString& fn, const QString& homedir, const QString& options) : d(0)
{
  Q_UNUSED(fn);
  Q_UNUSED(homedir);
  Q_UNUSED(options);
}

KGPGFile::~KGPGFile()
{
}

bool KGPGFile::open(OpenMode mode)
{
  Q_UNUSED(mode);
  return false;
}

void KGPGFile::close()
{
}

void KGPGFile::flush()
{
}

qint64 KGPGFile::readData(char *data, qint64 maxlen)
{
  Q_UNUSED(data);
  Q_UNUSED(maxlen);
  return 0;
}

qint64 KGPGFile::writeData(const char *data, qint64 maxlen)
{
  Q_UNUSED(data);
  Q_UNUSED(maxlen);
  return 0;
}

void KGPGFile::addRecipient(const QString& recipient)
{
  Q_UNUSED(recipient);
}

QString KGPGFile::errorToString() const
{
  return QString();
}

bool KGPGFile::GPGAvailable(void)
{
  return false;
}

bool KGPGFile::keyAvailable(const QString& name)
{
  Q_UNUSED(name);
  return false;
}

void KGPGFile::secretKeyList(QStringList& list)
{
  Q_UNUSED(list);
}

void KGPGFile::publicKeyList(QStringList& list)
{
  Q_UNUSED(list);
}

QDateTime KGPGFile::keyExpires(const QString& name)
{
  Q_UNUSED(name);
  return QDateTime();
}

#endif
