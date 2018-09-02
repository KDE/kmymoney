/***************************************************************************
                            gncimporter.cpp
                             -------------------

    copyright            : (C) 2017 by Łukasz Wojniłowicz
    email                : lukasz.wojnilowicz@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "gncimporter.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QFileDialog>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KActionCollection>
#include <KLocalizedString>
#include <KPluginFactory>
#include <KMessageBox>
#include <KCompressionDevice>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneygncreader.h"
#include "viewinterface.h"
#include "appinterface.h"
#include "mymoneyfile.h"
#include "mymoneyexception.h"
#include "mymoneystoragemgr.h"
#include "kmymoneyenums.h"

class MyMoneyStatement;

static constexpr KCompressionDevice::CompressionType const& COMPRESSION_TYPE = KCompressionDevice::GZip;

GNCImporter::GNCImporter(QObject *parent, const QVariantList &args) :
  KMyMoneyPlugin::Plugin(parent, "gncimporter"/*must be the same as X-KDE-PluginInfo-Name*/)
{
  Q_UNUSED(args)
  setComponentName("gncimporter", i18n("GnuCash importer"));
  // For information, announce that we have been loaded.
  qDebug("Plugins: gncimporter loaded");
}

GNCImporter::~GNCImporter()
{
  qDebug("Plugins: gncimporter unloaded");
}

MyMoneyStorageMgr *GNCImporter::open(const QUrl &url)
{ 
  if (url.scheme() == QLatin1String("sql"))
    return nullptr;

  if (!url.isLocalFile())
    return nullptr;

  const auto fileName = url.toLocalFile();
  const auto sFileToShort = QString::fromLatin1("File %1 is too short.").arg(fileName);

  QFile file(fileName);
  if (!file.open(QIODevice::ReadOnly))
    throw MYMONEYEXCEPTION(QString::fromLatin1("Cannot read the file: %1").arg(fileName));

  QByteArray qbaFileHeader(2, '\0');
  if (file.read(qbaFileHeader.data(), 2) != 2)
    throw MYMONEYEXCEPTION(sFileToShort);

  file.close();

  QIODevice* qfile = nullptr;
  QString sFileHeader(qbaFileHeader);
  if (sFileHeader == QString("\037\213"))        // gzipped?
    qfile = new KCompressionDevice(fileName, COMPRESSION_TYPE);
  else
    return nullptr;

  if (!qfile->open(QIODevice::ReadOnly)) {
    delete qfile;
    throw MYMONEYEXCEPTION(QString::fromLatin1("Cannot read the file: %1").arg(fileName));
  }

  // Scan the first 70 bytes to see if we find something
  // we know. For now, we support our own XML format and
  // GNUCash XML format. If the file is smaller, then it
  // contains no valid data and we reject it anyway.
  qbaFileHeader.resize(70);
  if (qfile->read(qbaFileHeader.data(), 70) != 70)
    throw MYMONEYEXCEPTION(sFileToShort);

  QString txt(qbaFileHeader);

  QRegExp gncexp("<gnc-v(\\d+)");
  if (!(gncexp.indexIn(txt) != -1)) {
    delete qfile;
    return nullptr;
  }

  MyMoneyGncReader pReader;
  qfile->seek(0);

  auto storage = new MyMoneyStorageMgr;
  pReader.setProgressCallback(appInterface()->progressCallback());
  pReader.readFile(qfile, storage);
  pReader.setProgressCallback(0);

  qfile->close();
  delete qfile;
  return storage;
}

bool GNCImporter::save(const QUrl &url)
{
  Q_UNUSED(url)
  return false;
}

bool GNCImporter::saveAs()
{
  return false;
}

eKMyMoney::StorageType GNCImporter::storageType() const
{
  return eKMyMoney::StorageType::GNC;
}

QString GNCImporter::fileExtension() const
{
  return i18n("GnuCash files (*.gnucash *.xac *.gnc)");
}

K_PLUGIN_FACTORY_WITH_JSON(GNCImporterFactory, "gncimporter.json", registerPlugin<GNCImporter>();)

#include "gncimporter.moc"
