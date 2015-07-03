/***************************************************************************
                            csvimporterplugin.cpp
                       (based on ofximporterplugin.cpp)
                             -------------------
    begin                : Sat Jan 01 2010
    copyright            : (C) 2010 by Allan Anderson
    email                : agander93@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "csvimporterplugin.h"
#include <assert.h>
// ----------------------------------------------------------------------------
// QT Includes

#include <QFile>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KPluginFactory>
#include <KDebug>
#include <KFile>
#include <QUrl>
#include <KAction>
#include <KMessageBox>
#include <KActionCollection>
#include <KStandardDirs>
#include <KSharedConfig>
#include <KLocale>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneystatementreader.h"
#include "mymoneystatement.h"
#include "csvdialog.h"
#include "investprocessing.h"

K_PLUGIN_FACTORY_WITH_JSON(CsvImporterFactory, "kmm_csvimport.json", registerPlugin<CsvImporterPlugin>();)

CsvImporterPlugin::CsvImporterPlugin(QObject *parent, const QVariantList&) :
    KMyMoneyPlugin::Plugin(parent, "csvimport"/*must be the same as X-KDE-PluginInfo-Name*/)
{
  setComponentName("kmm_csvimport", i18n("CSV importer"));
  setXMLFile("kmm_csvimport.rc");
  createActions();
  // For information, announce that we have been loaded.
  qDebug("KMyMoney csvimport plugin loaded");
}

CsvImporterPlugin::~CsvImporterPlugin()
{
}

void CsvImporterPlugin::createActions()
{
  m_action = actionCollection()->addAction("file_import_csv");
  m_action->setText(i18n("CSV..."));
  connect(m_action, SIGNAL(triggered(bool)), this, SLOT(slotImportFile()));
}

void CsvImporterPlugin::slotImportFile()
{
  m_action->setEnabled(false);
  CSVDialog *csvImporter = new CSVDialog;
  csvImporter->m_plugin = this;
  csvImporter->init();

  csvImporter->setWindowTitle(i18nc("CSV Importer dialog title", "CSV Importer"));

  connect(csvImporter, SIGNAL(statementReady(MyMoneyStatement&)), this, SLOT(slotGetStatement(MyMoneyStatement&)));
  csvImporter->show();
  m_action->setEnabled(false);//  don't allow further plugins to start while this is open
}

bool CsvImporterPlugin::slotGetStatement(MyMoneyStatement& s)
{
  return statementInterface()->import(s);
}

#include "csvimporterplugin.moc"
