/***************************************************************************
                             kgpgfile.h
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

#ifndef KGPGFILE_H
#define KGPGFILE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QFile>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QLinkedList>
#include <QEventLoop>
#include <QByteArray>

#include <vector>

// ----------------------------------------------------------------------------
// KDE Includes

namespace GpgME {
  class Key;
  class KeyListResult;
  class Error;
  class DecryptionResult;
  class EncryptionResult;
};

// ----------------------------------------------------------------------------
// Project Includes


#ifdef HAVE_CONFIG_H
#include <config-kmymoney.h>
#endif

namespace GpgME {
  class Error;
  class KeyListResult;
}

/**
  * @author Thomas Baumgart
  */

/**
  * A class for reading and writing data to/from an
  * encrypted e.g. file.
  *
  * This class presents a QFile based object to the application
  * but reads/writes data from/to the file through an instance of GPG.
  *
  * @code
  *
  *  +------------------+   write  +-----------+      +---------+
  *  |                  |--------->|\          |----->|         |
  *  | Application code |   read   | QFile     |      | libkleo |
  *  |                  |<---------|/          |<-----|         |
  *  +------------------+          |  KGPGFile |      +---------+
  *                |               |           |               
  *                |        control|           |      +-------+
  *                +-------------->|           |----->|       |
  *                                |           |      | File  |
  *                                |           |----->|       |
  *                                |           |      +-------+
  *                                +-----------+
  * @endcode
  *
  * The @p write interface contains methods as write() and putch(), the @p read
  * interface the methods read(), getch() and ungetch(). The @p control interface
  * special methods only available with KGPGFile e.g. addRecipient(), keyAvailable() and
  * GPGAvailable(). Other, more general methods such as open(), close() and flush() are
  * not shown in the above picture.
  */
class KGPGFile : public QFile
{
  Q_OBJECT

public:
  explicit KGPGFile(const QString& fname = "",
                    const QString& homedir = "~/.gnupg",
                    const QString& options = "");

  ~KGPGFile();

  virtual bool open(OpenMode mode);
  virtual void close(void);
  virtual void flush(void);

  virtual qint64 readData(char *data, qint64 maxlen);
  virtual qint64 writeData(const char *data, qint64 maxlen);

  /**
    * Adds a recipient for whom the file should be encrypted.
    * At least one recipient must be specified using this
    * method before the file can be written to. @p recipient
    * must contain a valid name as defined by GPG. See the
    * GPG documentation for more information.
    *
    * @param recipient recipients identification (e.g. e-mail address)
    */
  void addRecipient(const QString& recipient);

  /**
    * sets the name of the file to @p fn. This method must be
    * called prior to open().
    */
  void setFileName(const QString& fn);

  /**
    * Checks whether GPG is available or not
    *
    * @retval true GPG can be started and returns a version number
    * @retval false GPG is not available
    */
  static bool GPGAvailable(void);

  /**
    * Checks whether a key for a given user-id @p name exists.
    *
    * @param name the user-id to be checked. @p name can be
    *             any reference understood by GPG (e.g. an e-mail
    *             address or a key-id)
    * @retval true key for user-id @p name was found
    * @retval false key for user-id @p not available
    */
  static bool keyAvailable(const QString& name);

  /**
    * This function returns a list of the secret keys contained
    * in the keyring. Each list item is divided into two fields
    * separated by a colon (':'). The first field contains the
    * key id, the second field the name. The list may contain
    * multiple entries with the same key-id and different names.
    *
    * Example of an entry in the list:
    *
    *    "9C59DB40B75DD3BA:Thomas Baumgart <ipwizard@users.sourceforge.net>"
    */
  static void secretKeyList(QStringList& list);

  /**
    * This function returns a list of the public keys contained
    * in the keyring. Each list item is divided into two fields
    * separated by a colon (':'). The first field contains the
    * key id, the second field the name. The list may contain
    * multiple entries with the same key-id and different names.
    *
    * Example of an entry in the list:
    *
    *    "9C59DB40B75DD3BA:Thomas Baumgart <ipwizard@users.sourceforge.net>"
    */
  static void publicKeyList(QStringList& list);

private:
  void keyList(QStringList& list, bool secretKeys = false, const QStringList& patterns = QStringList());

private:
  /// \internal d-pointer class.
  class Private;
  /// \internal d-pointer instance.
  Private* const d;

private slots:
  void slotNextKey(const GpgME::Key & key);
  void slotKeyJobResult(const GpgME::KeyListResult & result, const std::vector<GpgME::Key> & keys, const QString & auditLogAsHtml, const GpgME::Error & auditLogError );

  void slotDecryptJobResult( const GpgME::DecryptionResult & result, const QByteArray & plainText, const QString & auditLogAsHtml, const GpgME::Error & auditLogError );
  void slotEncryptJobResult( const GpgME::EncryptionResult & result, const QByteArray & cipherText, const QString & auditLogAsHtml);

};

#endif
