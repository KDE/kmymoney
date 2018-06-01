/*
 * Copyright 2018  Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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

#ifndef XMLSTORAGE_H
#define XMLSTORAGE_H

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// QT Includes

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

  QAction *m_saveAsXMLaction;

  bool open(MyMoneyStorageMgr *storage, const QUrl &url) override;
  bool save(const QUrl &url) override;
  QString formatName() const override;
  QString fileExtension() const override;

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

  QString m_encryptionKeys;
private Q_SLOTS:
  void slotSaveAsXML();


};

#endif
