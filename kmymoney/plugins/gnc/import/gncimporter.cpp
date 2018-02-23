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

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneygncreader.h"
#include "viewinterface.h"
#include "appinterface.h"
#include "mymoneyfile.h"
#include "mymoneyexception.h"
#include "mymoneystoragemgr.h"

class MyMoneyStatement;

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

bool GNCImporter::open(MyMoneyStorageMgr *storage, const QUrl &url)
{
  Q_UNUSED(url)
  Q_UNUSED(storage)
  return false;
}

bool GNCImporter::save(const QUrl &url)
{
  Q_UNUSED(url)
  return false;
}

IMyMoneyOperationsFormat* GNCImporter::reader()
{
  return new MyMoneyGncReader;
}

QString GNCImporter::formatName() const
{
  return QStringLiteral("GNC");
}

QString GNCImporter::fileExtension() const
{
  return i18n("GnuCash files (*.gnucash *.xac *.gnc)");
}

K_PLUGIN_FACTORY_WITH_JSON(GNCImporterFactory, "gncimporter.json", registerPlugin<GNCImporter>();)

#include "gncimporter.moc"
