/***************************************************************************
                            gncimporterplugin.cpp
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

#include "gncimporterplugin.h"

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
#include "mymoneyseqaccessmgr.h"

class MyMoneyStatement;

GNCImporterPlugin::GNCImporterPlugin() :
    KMyMoneyPlugin::Plugin(nullptr, "gncimport"/*must be the same as X-KDE-PluginInfo-Name*/)
{
  setComponentName("kmm_gncimport", i18n("GnuCash importer"));
  setXMLFile("kmm_gncimport.rc");
  createActions();
  // For information, announce that we have been loaded.
  qDebug("KMyMoney gncimport plugin loaded");
}

GNCImporterPlugin::~GNCImporterPlugin()
{
}

void GNCImporterPlugin::createActions()
{
  m_action = actionCollection()->addAction("file_import_gnc");
  m_action->setText(i18n("GnuCash..."));
  connect(m_action, &QAction::triggered, this, &GNCImporterPlugin::slotGNCImport);
}

void GNCImporterPlugin::slotGNCImport()
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
