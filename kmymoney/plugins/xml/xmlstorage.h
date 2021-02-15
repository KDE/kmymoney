/*
 * SPDX-FileCopyrightText: 2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef XMLSTORAGE_H
#define XMLSTORAGE_H

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// QT Includes

#include <QUrl>

// Project Includes

#include "kmymoneyplugin.h"

class QIODevice;

class MyMoneyStorageMgr;

class XMLStorage : public KMyMoneyPlugin::Plugin, public KMyMoneyPlugin::StoragePlugin
{
  Q_OBJECT
  Q_INTERFACES(KMyMoneyPlugin::StoragePlugin)

public:
  explicit XMLStorage(QObject *parent, const QVariantList &args);
  ~XMLStorage() override;

  MyMoneyStorageMgr *open(const QUrl &url) override;
  bool save(const QUrl &url) override;
  bool saveAs() override;
  eKMyMoney::StorageType storageType() const override;
  QString fileExtension() const override;
  QUrl openUrl() const override;

private:
  void createActions();
  void ungetString(QIODevice *qfile, char *buf, int len);

  /**
    * This method is used by saveFile() to store the data
    * either directly in the destination file if it is on
    * the local file system or in a temporary file when
    * the final destination is reached over a network
    * protocol (e.g. FTP)
    *
    * @param localFile the name of the local file
    * @param writer pointer to the formatter
    * @param plaintext whether to override any compression & encryption settings
    * @param keyList QString containing a comma separated list of keys to be used for encryption
    *            If @p keyList is empty, the file will be saved unencrypted
    *
    * @note This method will close the file when it is written.
    */
  void saveToLocalFile(const QString& localFile, IMyMoneyOperationsFormat* pWriter, bool plaintext, const QString& keyList);

  void checkRecoveryKeyValidity();

  QString m_encryptionKeys;

  QUrl fileUrl;
};

#endif
