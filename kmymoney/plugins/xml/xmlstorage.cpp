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

#include "xmlstorage.h"

#include <memory>
#include <config-kmymoney.h>

// ----------------------------------------------------------------------------
// QT Includes

#include <QTimer>
#include <QFile>
#include <QTemporaryFile>
#include <QFileDialog>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KPluginFactory>
#include <KActionCollection>
#include <KLocalizedString>
#include <KMessageBox>
#include <KCompressionDevice>
#include <KIO/StoredTransferJob>
#include <KBackup>

// ----------------------------------------------------------------------------
// Project Includes

#include "appinterface.h"
#include "viewinterface.h"
#include "mymoneyfile.h"
#include "mymoneystoragemgr.h"
#include "mymoneyexception.h"
#include "mymoneystoragebin.h"
#include "mymoneystoragexml.h"
#include "mymoneystorageanon.h"
#include "icons.h"
#include "kmymoneysettings.h"
#include "kmymoneyutils.h"
#include "kgpgfile.h"
#include "kgpgkeyselectiondlg.h"
#include "kmymoneyenums.h"

using namespace Icons;

static constexpr KCompressionDevice::CompressionType const& COMPRESSION_TYPE = KCompressionDevice::GZip;
// static constexpr char recoveryKeyId[] = "0xD2B08440";
static constexpr char recoveryKeyId[] = "59B0F826D2B08440";

// define the default period to warn about an expiring recoverkey to 30 days
// but allows to override this setting during build time
#ifndef RECOVER_KEY_EXPIRATION_WARNING
#define RECOVER_KEY_EXPIRATION_WARNING 30
#endif

XMLStorage::XMLStorage(QObject *parent, const QVariantList &args) :
  KMyMoneyPlugin::Plugin(parent, "xmlstorage"/*must be the same as X-KDE-PluginInfo-Name*/)
{
  Q_UNUSED(args)
  setComponentName("xmlstorage", i18n("XML storage"));
  // For information, announce that we have been loaded.
  qDebug("Plugins: xmlstorage loaded");
  checkRecoveryKeyValidity();
}

XMLStorage::~XMLStorage()
{
  qDebug("Plugins: xmlstorage unloaded");
}

MyMoneyStorageMgr *XMLStorage::open(const QUrl &url)
{
  fileUrl.clear();

  if (url.scheme() == QLatin1String("sql"))
    return nullptr;

  QString fileName;
  auto downloadedFile = false;
  if (url.isLocalFile()) {
    fileName = url.toLocalFile();
  } else {
    fileName = KMyMoneyUtils::downloadFile(url);
    downloadedFile = true;
  }

  if (!KMyMoneyUtils::fileExists(QUrl::fromLocalFile(fileName)))
    throw MYMONEYEXCEPTION(QString::fromLatin1("Error opening the file.\n"
                                               "Requested file: '%1'.\n"
                                               "Downloaded file: '%2'").arg(qPrintable(url.url()), fileName));


  QFile file(fileName);
  if (!file.open(QIODevice::ReadOnly))
    throw MYMONEYEXCEPTION(QString::fromLatin1("Cannot read the file: %1").arg(fileName));

  QByteArray qbaFileHeader(2, '\0');
  const auto sFileToShort = QString::fromLatin1("File %1 is too short.").arg(fileName);
  if (file.read(qbaFileHeader.data(), 2) != 2)
    throw MYMONEYEXCEPTION(sFileToShort);

  file.close();

  // There's a problem with the KFilterDev and KGPGFile classes:
  // One supports the at(n) member but not ungetch() together with
  // read() and the other does not provide an at(n) method but
  // supports read() that considers the ungetch() buffer. QFile
  // supports everything so this is not a problem. We solve the problem
  // for now by keeping track of which method can be used.
  auto haveAt = true;
  auto isEncrypted = false;

  QIODevice* qfile = nullptr;
  QString sFileHeader(qbaFileHeader);
  if (sFileHeader == QString("\037\213")) {        // gzipped?
    qfile = new KCompressionDevice(fileName, COMPRESSION_TYPE);
  } else if (sFileHeader == QString("--") ||        // PGP ASCII armored?
             sFileHeader == QString("\205\001") ||  // PGP binary?
             sFileHeader == QString("\205\002")) {  // PGP binary?
    if (KGPGFile::GPGAvailable()) {
      qfile = new KGPGFile(fileName);
      haveAt = false;
      isEncrypted = true;
    } else {
      throw MYMONEYEXCEPTION(QString::fromLatin1("GPG is not available for decryption of file <b>%1</b>").arg(fileName));
    }
  } else {
    // we can't use file directly, as we delete qfile later on
    qfile = new QFile(file.fileName());
  }

  if (!qfile->open(QIODevice::ReadOnly)) {
    delete qfile;
    throw MYMONEYEXCEPTION(QString::fromLatin1("Cannot read the file: %1").arg(fileName));
  }

  qbaFileHeader.resize(8);
  if (qfile->read(qbaFileHeader.data(), 8) != 8)
    throw MYMONEYEXCEPTION(sFileToShort);

  if (haveAt)
    qfile->seek(0);
  else
    ungetString(qfile, qbaFileHeader.data(), 8);

  // Ok, we got the first block of 8 bytes. Read in the two
  // unsigned long int's by preserving endianess. This is
  // achieved by reading them through a QDataStream object
  qint32 magic0, magic1;
  QDataStream s(&qbaFileHeader, QIODevice::ReadOnly);
  s >> magic0;
  s >> magic1;

  // If both magic numbers match (we actually read in the
  // text 'KMyMoney' then we assume a binary file and
  // construct a reader for it. Otherwise, we construct
  // an XML reader object.
  //
  // The expression magic0 < 30 is only used to create
  // a binary reader if we assume an old binary file. This
  // should be removed at some point. An alternative is to
  // check the beginning of the file against an pattern
  // of the XML file (e.g. '?<xml' ).
  if ((magic0 == MAGIC_0_50 && magic1 == MAGIC_0_51) ||
      magic0 < 30) {
    // we do not support this file format anymore
    throw MYMONEYEXCEPTION(QString::fromLatin1("<qt>File <b>%1</b> contains the old binary format used by KMyMoney. Please use an older version of KMyMoney (0.8.x) that still supports this format to convert it to the new XML based format.</qt>").arg(fileName));
  }

  // Scan the first 70 bytes to see if we find something
  // we know. For now, we support our own XML format and
  // GNUCash XML format. If the file is smaller, then it
  // contains no valid data and we reject it anyway.
  qbaFileHeader.resize(70);
  if (qfile->read(qbaFileHeader.data(), 70) != 70)
    throw MYMONEYEXCEPTION(sFileToShort);

  if (haveAt)
    qfile->seek(0);
  else
    ungetString(qfile, qbaFileHeader.data(), 70);

  QRegExp kmyexp("<!DOCTYPE KMYMONEY-FILE>");
  QByteArray txt(qbaFileHeader, 70);
  if (kmyexp.indexIn(txt) == -1)
    return nullptr;

  // attach the storage before reading the file, since the online
  // onlineJobAdministration object queries the engine during
  // loading.

  auto storage = new MyMoneyStorageMgr;
  MyMoneyStorageXML pReader;
  pReader.setProgressCallback(appInterface()->progressCallback());
  pReader.readFile(qfile, storage, MyMoneyFile::instance());
  pReader.setProgressCallback(0);

  qfile->close();
  delete qfile;

  // if a temporary file was downloaded, then it will be removed
  // with the next call. Otherwise, it stays untouched on the local
  // filesystem.
  if (downloadedFile)
    QFile::remove(fileName);

  // make sure we setup the encryption key correctly
  if (isEncrypted) {
    MyMoneyFile::instance()->attachStorage(storage);
    if (MyMoneyFile::instance()->value("kmm-encryption-key").isEmpty()) {
      // encapsulate transactions to the engine to be able to commit/rollback
      MyMoneyFileTransaction ft;
      MyMoneyFile::instance()->setValue("kmm-encryption-key", KMyMoneySettings::gpgRecipientList().join(","));
      ft.commit();
    }
    MyMoneyFile::instance()->detachStorage();
  }

  fileUrl = url;
  //write the directory used for this file as the default one for next time.
  appInterface()->writeLastUsedDir(url.toDisplayString(QUrl::RemoveFilename | QUrl::PreferLocalFile | QUrl::StripTrailingSlash));

  return storage;
}

QUrl XMLStorage::openUrl() const
{
  return fileUrl;
}

bool XMLStorage::save(const QUrl &url)
{
  QString filename = url.toLocalFile();

  if (!appInterface()->fileOpen()) {
    KMessageBox::error(nullptr, i18n("Tried to access a file when it has not been opened"));
    return false;
  }

  std::unique_ptr<IMyMoneyOperationsFormat> storageWriter;

  // If this file ends in ".ANON.XML" then this should be written using the
  // anonymous writer.
  bool plaintext = filename.right(4).toLower() == ".xml";
  if (filename.right(9).toLower() == ".anon.xml")
    storageWriter = std::make_unique<MyMoneyStorageANON>();
  else
    storageWriter = std::make_unique<MyMoneyStorageXML>();

  QString keyList;
  if (!appInterface()->filenameURL().isEmpty())
    keyList = MyMoneyFile::instance()->value("kmm-encryption-key");
  if (keyList.isEmpty())
    keyList = m_encryptionKeys;

  // actually, url should be the parameter to this function
  // but for now, this would involve too many changes
  auto rc = true;
  try {
    if (! url.isValid()) {
      throw MYMONEYEXCEPTION(QString::fromLatin1("Malformed URL '%1'").arg(url.url()));
    }

    if (url.isLocalFile()) {
      filename = url.toLocalFile();
      try {
        const unsigned int nbak = KMyMoneySettings::autoBackupCopies();
        if (nbak) {
            KBackup::numberedBackupFile(filename, QString(), QStringLiteral("~"), nbak);
          }
        saveToLocalFile(filename, storageWriter.get(), plaintext, keyList);
      } catch (const MyMoneyException &e) {
        qWarning("Unable to write changes to: %s\nReason: %s", qPrintable(filename), e.what());
        throw;
      }
    } else {

      QTemporaryFile tmpfile;
      tmpfile.open(); // to obtain the name
      tmpfile.close();
      saveToLocalFile(tmpfile.fileName(), storageWriter.get(), plaintext, keyList);

      Q_CONSTEXPR int permission = -1;
      QFile file(tmpfile.fileName());
      file.open(QIODevice::ReadOnly);
      KIO::StoredTransferJob *putjob = KIO::storedPut(file.readAll(), url, permission, KIO::JobFlag::Overwrite);
      if (!putjob->exec()) {
        throw MYMONEYEXCEPTION(QString::fromLatin1("Unable to upload to '%1'.<br />%2").arg(url.toDisplayString(), putjob->errorString()));
      }
      file.close();
    }
  } catch (const MyMoneyException &e) {
    KMessageBox::error(nullptr, QString::fromLatin1(e.what()));
    MyMoneyFile::instance()->setDirty();
    rc = false;
  }
  return rc;
}

bool XMLStorage::saveAs()
{
  auto rc = false;
  QStringList m_additionalGpgKeys;
  m_encryptionKeys.clear();

  QString selectedKeyName;
  if (KGPGFile::GPGAvailable() && KMyMoneySettings::writeDataEncrypted()) {
    // fill the secret key list and combo box
    QStringList keyList;
    KGPGFile::secretKeyList(keyList);

    QPointer<KGpgKeySelectionDlg> dlg = new KGpgKeySelectionDlg(nullptr);
    dlg->setSecretKeys(keyList, KMyMoneySettings::gpgRecipient());
    dlg->setAdditionalKeys(KMyMoneySettings::gpgRecipientList());
    rc = dlg->exec();
    if ((rc == QDialog::Accepted) && (dlg != 0)) {
      m_additionalGpgKeys = dlg->additionalKeys();
      selectedKeyName = dlg->secretKey();
    }
    delete dlg;
    if (rc != QDialog::Accepted) {
      return rc;
    }
  }

  QString prevDir; // don't prompt file name if not a native file
  if (appInterface()->isNativeFile())
    prevDir = appInterface()->readLastUsedDir();

  QPointer<QFileDialog> dlg =
    new QFileDialog(nullptr, i18n("Save As"), prevDir,
                    QString(QLatin1String("%2 (%1);;")).arg(QStringLiteral("*.kmy")).arg(i18nc("KMyMoney (Filefilter)", "KMyMoney files")) +
                    QString(QLatin1String("%2 (%1);;")).arg(QStringLiteral("*.anon.xml")).arg(i18nc("Anonymous (Filefilter)", "Anonymous files")) +
                    QString(QLatin1String("%2 (%1);;")).arg(QStringLiteral("*.xml")).arg(i18nc("XML (Filefilter)", "XML files")) +
                    QString(QLatin1String("%2 (%1);;")).arg(QStringLiteral("*")).arg(i18nc("All files (Filefilter)", "All files")));
  dlg->setAcceptMode(QFileDialog::AcceptSave);
  connect(dlg, &QFileDialog::filterSelected, this, [&](const QString txt) {
    // for some reason, txt sometimes contains the filter expression only
    // e.g. "*.xml" and in some others it contains the full text with
    // the filter expression appended in parenthesis e.g.
    // "KMyMoney files (*.xml)". The following logic extracts the
    // filter and sets the default suffix based on it.
    QRegularExpression filter(QStringLiteral("\\*\\.(?<extension>[a-z\\.]+)"));
    const auto match = filter.match(txt);
    if (match.hasMatch()) {
      dlg->setDefaultSuffix(match.captured(QStringLiteral("extension")));
    } else {
      dlg->setDefaultSuffix(QString());
    }

  });

  if (dlg->exec() == QDialog::Accepted && dlg != 0) {
    QUrl newURL = dlg->selectedUrls().first();
    if (!newURL.fileName().isEmpty()) {
      QString newName = newURL.toDisplayString(QUrl::PreferLocalFile);

      // append extension if not present
      if (!newName.endsWith(QLatin1String(".kmy"), Qt::CaseInsensitive) &&
          !newName.endsWith(QLatin1String(".xml"), Qt::CaseInsensitive))
        newName.append(QLatin1String(".kmy"));
      newURL = QUrl::fromUserInput(newName);

      // If this is the anonymous file export, just save it, don't actually take the
      // name, or remember it! Don't even try to encrypt it
      if (newName.endsWith(QLatin1String(".anon.xml"), Qt::CaseInsensitive))
        rc = save(newURL);
      else {
        appInterface()->writeFilenameURL(newURL);
        QRegExp keyExp(".* \\((.*)\\)");
        if (keyExp.indexIn(selectedKeyName) != -1) {
          m_encryptionKeys = keyExp.cap(1);
          if (!m_additionalGpgKeys.isEmpty()) {
            if (!m_encryptionKeys.isEmpty())
              m_encryptionKeys.append(QLatin1Char(','));
            m_encryptionKeys.append(m_additionalGpgKeys.join(QLatin1Char(',')));
          }
        }
        // clear out any existing keys so that the new ones will be used
        MyMoneyFileTransaction ft;
        try {
          MyMoneyFile::instance()->deletePair("kmm-encryption-key");
          ft.commit();
        } catch(MyMoneyException& e) {
          ; // do nothing
        }
        rc = save(newURL);

        appInterface()->addToRecentFiles(newURL);
        //write the directory used for this file as the default one for next time.
        appInterface()->writeLastUsedDir(newURL.toDisplayString(QUrl::RemoveFilename | QUrl::PreferLocalFile | QUrl::StripTrailingSlash));
        appInterface()->writeLastUsedFile(newName);
      }
    }
  }
  (*appInterface()->progressCallback())(0,0, i18nc("Application is ready to use", "Ready."));
  delete dlg;
  return rc;
}

eKMyMoney::StorageType XMLStorage::storageType() const
{
  return eKMyMoney::StorageType::XML;
}

QString XMLStorage::fileExtension() const
{
  return i18n("KMyMoney files (*.kmy *.xml)");
}

void XMLStorage::ungetString(QIODevice *qfile, char *buf, int len)
{
  buf = &buf[len-1];
  while (len--) {
    qfile->ungetChar(*buf--);
  }
}

void XMLStorage::saveToLocalFile(const QString& localFile, IMyMoneyOperationsFormat* pWriter, bool plaintext, const QString& keyList)
{
  // Check GPG encryption
  bool encryptFile = true;
  bool encryptRecover = false;
  if (!keyList.isEmpty()) {
    if (!KGPGFile::GPGAvailable()) {
      KMessageBox::sorry(nullptr, i18n("GPG does not seem to be installed on your system. Please make sure that GPG can be found using the standard search path. This time, encryption is disabled."), i18n("GPG not found"));
      encryptFile = false;
    } else {
      if (KMyMoneySettings::encryptRecover()) {
        encryptRecover = true;
        if (!KGPGFile::keyAvailable(QString(recoveryKeyId))) {
          KMessageBox::sorry(nullptr, i18n("<p>You have selected to encrypt your data also with the KMyMoney recover key, but the key with id</p><p><center><b>%1</b></center></p><p>has not been found in your keyring at this time. Please make sure to import this key into your keyring. You can find it on the <a href=\"https://kmymoney.org/\">KMyMoney web-site</a>. This time your data will not be encrypted with the KMyMoney recover key.</p>", QString(recoveryKeyId)), i18n("GPG Key not found"));
          encryptRecover = false;
        }
      }

      for(const QString& key: keyList.split(',', QString::SkipEmptyParts)) {
        if (!KGPGFile::keyAvailable(key)) {
          KMessageBox::sorry(nullptr, i18n("<p>You have specified to encrypt your data for the user-id</p><p><center><b>%1</b>.</center></p><p>Unfortunately, a valid key for this user-id was not found in your keyring. Please make sure to import a valid key for this user-id. This time, encryption is disabled.</p>", key), i18n("GPG Key not found"));
          encryptFile = false;
          break;
        }
      }

      if (encryptFile == true) {
        QString msg = i18n("<p>You have configured to save your data in encrypted form using GPG. Make sure you understand that you might lose all your data if you encrypt it, but cannot decrypt it later on. If unsure, answer <b>No</b>.</p>");
        if (KMessageBox::questionYesNo(nullptr, msg, i18n("Store GPG encrypted"), KStandardGuiItem::yes(), KStandardGuiItem::no(), "StoreEncrypted") == KMessageBox::No) {
          encryptFile = false;
        }
      }
    }
  }

  // Permissions to apply to new file
  QFileDevice::Permissions fmode = QFileDevice::ReadUser | QFileDevice::WriteUser;

  // Create a temporary file if needed
  QString writeFile = localFile;
  QTemporaryFile tmpFile(writeFile);
  if (QFile::exists(localFile)) {
    tmpFile.open();
    writeFile = tmpFile.fileName();
    tmpFile.close();
    // Since file is going to be replaced, stash the original permissions so they can be restored
    fmode = QFile::permissions(localFile);
  }

  /**
   * @brief Automatically restore settings when scope is left
   */
  struct restorePreviousSettingsHelper {
    restorePreviousSettingsHelper()
      : m_signalsWereBlocked{MyMoneyFile::instance()->signalsBlocked()}
    {
      MyMoneyFile::instance()->blockSignals(true);
    }

    ~restorePreviousSettingsHelper()
    {
      MyMoneyFile::instance()->blockSignals(m_signalsWereBlocked);
    }
    const bool m_signalsWereBlocked;
  } restoreHelper;

  MyMoneyFileTransaction ft;
  MyMoneyFile::instance()->deletePair("kmm-encryption-key");
  std::unique_ptr<QIODevice> device;

  if (!keyList.isEmpty() && encryptFile && !plaintext) {
    std::unique_ptr<KGPGFile> kgpg = std::unique_ptr<KGPGFile>(new KGPGFile{writeFile});
    if (kgpg) {
      for(const QString& key: keyList.split(',', QString::SkipEmptyParts)) {
        kgpg->addRecipient(key.toLatin1());
      }

      if (encryptRecover) {
        kgpg->addRecipient(recoveryKeyId);
      }
      MyMoneyFile::instance()->setValue("kmm-encryption-key", keyList);
      device = std::unique_ptr<decltype(device)::element_type>(kgpg.release());
    }
  } else {
    QFile *file = new QFile(writeFile);
    // The second parameter of KCompressionDevice means that KCompressionDevice will delete the QFile object
    device = std::unique_ptr<decltype(device)::element_type>(new KCompressionDevice{file, true, (plaintext) ? KCompressionDevice::None : COMPRESSION_TYPE});
  }

  ft.commit();

  if (!device || !device->open(QIODevice::WriteOnly)) {
    throw MYMONEYEXCEPTION(QString::fromLatin1("Unable to open file '%1' for writing.").arg(localFile));
  }

  pWriter->setProgressCallback(appInterface()->progressCallback());
  pWriter->writeFile(device.get(), MyMoneyFile::instance()->storage(), MyMoneyFile::instance());
  device->close();

  // Check for errors if possible, only possible for KGPGFile
  QFileDevice *fileDevice = qobject_cast<QFileDevice*>(device.get());
  if (fileDevice && fileDevice->error() != QFileDevice::NoError) {
    throw MYMONEYEXCEPTION(QString::fromLatin1("Failure while writing to '%1'").arg(localFile));
  }

  if (writeFile != localFile) {
    // This simple comparison is possible because the strings are equal if no temporary file was created.
    // If a temporary file was created, it is made in a way that the name is definitely different. So no
    // symlinks etc. have to be evaluated.

    // on Windows QTemporaryFile does not release file handle even after close()
    // so QFile::rename(writeFile, localFile) will fail since Windows does not allow moving files in use
    // as a workaround QFile::copy is used instead of QFile::rename below
    // writeFile (i.e. tmpFile) will be deleted by QTemporaryFile dtor when it falls out of scope
    if (!QFile::remove(localFile) || !QFile::copy(writeFile, localFile))
      throw MYMONEYEXCEPTION(QString::fromLatin1("Failure while writing to '%1'").arg(localFile));
  }
  QFile::setPermissions(localFile, fmode);
  pWriter->setProgressCallback(0);
}

void XMLStorage::checkRecoveryKeyValidity()
{
// check if the recovery key is still valid or expires soon

if (KMyMoneySettings::writeDataEncrypted() && KMyMoneySettings::encryptRecover()) {
  if (KGPGFile::GPGAvailable()) {
    KGPGFile file;
    QDateTime expirationDate = file.keyExpires(QLatin1String(recoveryKeyId));
    if (expirationDate.isValid() && QDateTime::currentDateTime().daysTo(expirationDate) <= RECOVER_KEY_EXPIRATION_WARNING) {
      bool skipMessage = false;

      //get global config object for our app.
      KSharedConfigPtr kconfig = KSharedConfig::openConfig();
      KConfigGroup grp;
      QDate lastWarned;
      if (kconfig) {
        grp = kconfig->group("General Options");
        lastWarned = grp.readEntry("LastRecoverKeyExpirationWarning", QDate());
        if (QDate::currentDate() == lastWarned) {
          skipMessage = true;
        }
      }
      if (!skipMessage) {
        if (kconfig) {
          grp.writeEntry("LastRecoverKeyExpirationWarning", QDate::currentDate());
        }
        KMessageBox::information(nullptr, i18np("You have configured KMyMoney to use GPG to protect your data and to encrypt your data also with the KMyMoney recover key. This key is about to expire in %1 day. Please update the key from a keyserver using your GPG frontend (e.g. KGPG).", "You have configured KMyMoney to use GPG to protect your data and to encrypt your data also with the KMyMoney recover key. This key is about to expire in %1 days. Please update the key from a keyserver using your GPG frontend (e.g. KGPG).", QDateTime::currentDateTime().daysTo(expirationDate)), i18n("Recover key expires soon"));
      }
    }
  }
}
}

K_PLUGIN_FACTORY_WITH_JSON(XMLStorageFactory, "xmlstorage.json", registerPlugin<XMLStorage>();)

#include "xmlstorage.moc"
