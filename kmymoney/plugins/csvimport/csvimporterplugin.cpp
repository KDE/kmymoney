/***************************************************************************
                            csvimporterplugin.cpp
                       (based on ofximporterplugin.cpp)
                             -------------------
    begin                : Sat Jan 01 2010
    copyright            : (C) 2010 by Allan Anderson
    email                :
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

#include <KGenericFactory>
#include <KDebug>
#include <KFile>
#include <KUrl>
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
#include "csvimporterdlg.h"

///class Csv;

K_EXPORT_COMPONENT_FACTORY(kmm_csvimport,
                           KGenericFactory<CsvImporterPlugin>("kmm_csvimport"))

CsvImporterPlugin::CsvImporterPlugin(QObject *parent, const QStringList&) :
    KMyMoneyPlugin::Plugin(parent, "csvimport"/*must be the same as X-KDE-PluginInfo-Name*/)
{
  setComponentData(KGenericFactory<CsvImporterPlugin>::componentData());
  setXMLFile("kmm_csvimport.rc");
  createActions();
// For information, announce that we have been loaded.
  qDebug("KMyMoney csvimport plugin loaded");
}

CsvImporterPlugin::~CsvImporterPlugin()
{
}

void CsvImporterPlugin::createActions(void)
{
  m_action = actionCollection()->addAction("file_import_csv");
  m_action->setText(i18n("CSV..."));
  connect(m_action, SIGNAL(triggered(bool)), this, SLOT(slotImportFile()));
}

void CsvImporterPlugin::slotImportFile(void)
{
  m_csvDlg = new CsvImporterDlg;
  m_csvDlg->m_plugin = this;

  m_csvDlg->setWindowTitle(i18nc("CSV Importer dialog title", "CSV Importer"));
  m_csvDlg->spinBox_skip->setEnabled(true);

  m_action->setEnabled(false);//            disable csv menuitem once plugin is loaded
  connect(m_csvDlg, SIGNAL(statementReady(MyMoneyStatement&)), this, SLOT(slotGetStatement(MyMoneyStatement&)));
  m_csvDlg->show();
}

bool CsvImporterPlugin::slotGetStatement(MyMoneyStatement& s)
{
  return statementInterface()->import(s);
}
