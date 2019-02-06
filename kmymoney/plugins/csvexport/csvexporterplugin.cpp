/***************************************************************************
                            csvexporterplugin.cpp
                       (based on ofximporterplugin.cpp)
                             -------------------
    begin                : Wed Mar 20 2013
    copyright            : (C) 2013 by Allan Anderson
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

#include "csvexporterplugin.h"
#include "csvexportdlg.h"
#include "csvwriter.h"
#include <assert.h>
// ----------------------------------------------------------------------------
// QT Includes

#include <QFile>
#include <QDebug>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KPluginFactory>
#include <KDebug>
#include <KFile>
#include <KUrl>
#include <KAction>
#include <KMessageBox>
#include <KActionCollection>
#include <KStandardDirs>
#include <KSharedConfig>
#include <KLocale>
#include <kio/netaccess.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoney.h"

K_PLUGIN_FACTORY(CsvExporterFactory, registerPlugin<CsvExporterPlugin>();)
K_EXPORT_PLUGIN(CsvExporterFactory("kmm_csvexport"))

CsvExporterPlugin::CsvExporterPlugin(QObject *parent, const QVariantList&) :
    KMyMoneyPlugin::Plugin(parent, "csvexport"/*must be the same as X-KDE-PluginInfo-Name*/)
{
  setComponentData(CsvExporterFactory::componentData());
  setXMLFile("kmm_csvexport.rc");
  createActions();
// For information, announce that we have been loaded.
  qDebug("KMyMoney csvexport plugin loaded");
}

CsvExporterPlugin::~CsvExporterPlugin()
{
}

void CsvExporterPlugin::createActions()
{
  m_action = actionCollection()->addAction("file_export_csv");
  m_action->setText(i18n("&CSV..."));
  connect(m_action, SIGNAL(triggered(bool)), this, SLOT(slotCsvExport()));
}

void CsvExporterPlugin::slotCsvExport()
{
  m_dlg = new CsvExportDlg();
  if (m_dlg->exec()) {
    if (okToWriteFile(m_dlg->filename())) {
      m_dlg->setWindowTitle(i18nc("CSV Exporter dialog title", "CSV Exporter"));
      CsvWriter* writer = new CsvWriter;
      writer->m_plugin = this;
      connect(writer, SIGNAL(signalProgress(int,int)), m_dlg, SLOT(slotStatusProgressBar(int,int)));

      writer->write(m_dlg->filename(), m_dlg->accountId(),
                    m_dlg->accountSelected(), m_dlg->categorySelected(),
                    m_dlg->startDate(), m_dlg->endDate(),
                    m_dlg->separator());
    }
  }
}

bool CsvExporterPlugin::okToWriteFile(const KUrl& url)
{
  // check if the file exists and warn the user
  bool reallySaveFile = true;

  if (KIO::NetAccess::exists(url, KIO::NetAccess::SourceSide, 0)) {
    if (KMessageBox::warningYesNo(nullptr, QString("<qt>") + i18n("The file <b>%1</b> already exists. Do you really want to overwrite it?", url.pathOrUrl()) + QString("</qt>"), i18n("File already exists")) != KMessageBox::Yes)
      reallySaveFile = false;
  }
  return reallySaveFile;
}

