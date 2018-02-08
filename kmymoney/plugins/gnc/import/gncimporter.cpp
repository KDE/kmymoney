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
#include "mymoneyfile.h"
#include "mymoneyexception.h"
#include "mymoneystoragemgr.h"
#include "kmymoneyglobalsettings.h"

class MyMoneyStatement;

GNCImporter::GNCImporter(QObject *parent, const QVariantList &args) :
  KMyMoneyPlugin::Plugin(parent, "gncimporter"/*must be the same as X-KDE-PluginInfo-Name*/)
{
  Q_UNUSED(args)
  setComponentName("gncimporter", i18n("GnuCash importer"));
  setXMLFile("gncimporter.rc");
  createActions();
  // For information, announce that we have been loaded.
  qDebug("Plugins: gncimporter loaded");
}

GNCImporter::~GNCImporter()
{
  qDebug("Plugins: gncimporter unloaded");
}

void GNCImporter::injectExternalSettings(KMyMoneySettings* p)
{
  KMyMoneyGlobalSettings::injectExternalSettings(p);
}

void GNCImporter::createActions()
{
  m_action = actionCollection()->addAction("file_import_gnc");
  m_action->setText(i18n("GnuCash..."));
  connect(m_action, &QAction::triggered, this, &GNCImporter::slotGNCImport);
}

void GNCImporter::slotGNCImport()
{
  m_action->setEnabled(false);

  if (viewInterface()->fileOpen()) {
    KMessageBox::information(nullptr, i18n("You cannot import GnuCash data into an existing file. Please close it."));
    m_action->setEnabled(true);
    return;
  }

  auto url = QFileDialog::getOpenFileUrl(nullptr, QString(), QUrl(), i18n("GnuCash files (*.gnucash *.xac *.gnc);;All files (*)"));
  if (url.isLocalFile()) {
    auto pReader = new MyMoneyGncReader;
    if (viewInterface()->readFile(url, pReader))
      viewInterface()->slotRefreshViews();
  }

  m_action->setEnabled(true);
}

K_PLUGIN_FACTORY_WITH_JSON(GNCImporterFactory, "gncimporter.json", registerPlugin<GNCImporter>();)

#include "gncimporter.moc"
