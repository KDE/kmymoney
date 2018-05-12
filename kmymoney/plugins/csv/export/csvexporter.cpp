/*
 * Copyright 2013-2014  Allan Anderson <agander93@gmail.com>
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

#include "csvexporter.h"
#include "csvexportdlg.h"
#include "csvwriter.h"
// ----------------------------------------------------------------------------
// QT Includes

#include <QUrl>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KPluginFactory>
#include <KActionCollection>
#include <KLocalizedString>
#include <KMessageBox>
#include <KIO/StatJob>

// ----------------------------------------------------------------------------
// Project Includes

CSVExporter::CSVExporter(QObject *parent, const QVariantList &args) :
    KMyMoneyPlugin::Plugin(parent, "csvexporter"/*must be the same as X-KDE-PluginInfo-Name*/)
{
  Q_UNUSED(args);
  setComponentName("csvexporter", i18n("CSV exporter"));
  setXMLFile("csvexporter.rc");
  createActions();
  // For information, announce that we have been loaded.
  qDebug("Plugins: csvexporter loaded");
}

CSVExporter::~CSVExporter()
{
  qDebug("Plugins: csvexporter unloaded");
}

void CSVExporter::createActions()
{
  m_action = actionCollection()->addAction("file_export_csv");
  m_action->setText(i18n("&CSV..."));
  connect(m_action, &QAction::triggered, this, &CSVExporter::slotCsvExport);
}

void CSVExporter::slotCsvExport()
{
  m_dlg = new CsvExportDlg();
  if (m_dlg->exec()) {
    if (okToWriteFile(QUrl::fromUserInput(m_dlg->filename()))) {
      m_dlg->setWindowTitle(i18nc("CSV Exporter dialog title", "CSV Exporter"));
      CsvWriter* writer = new CsvWriter;
      writer->m_plugin = this;
      connect(writer, &CsvWriter::signalProgress, m_dlg, &CsvExportDlg::slotStatusProgressBar);

      writer->write(m_dlg->filename(), m_dlg->accountId(),
                    m_dlg->accountSelected(), m_dlg->categorySelected(),
                    m_dlg->startDate(), m_dlg->endDate(),
                    m_dlg->separator());
    }
  }
}

bool CSVExporter::okToWriteFile(const QUrl &url)
{
  // check if the file exists and warn the user
  bool reallySaveFile = true;

  bool fileExists = false;

  if (url.isValid()) {
    short int detailLevel = 0; // Lowest level: file/dir/symlink/none
    KIO::StatJob* statjob = KIO::stat(url, KIO::StatJob::SourceSide, detailLevel);
    bool noerror = statjob->exec();
    if (noerror) {
      // We want a file
      fileExists = !statjob->statResult().isDir();
    }
  }

  if (fileExists) {
    if (KMessageBox::warningYesNo(0, i18n("<qt>The file <b>%1</b> already exists. Do you really want to overwrite it?</qt>", url.toDisplayString(QUrl::PreferLocalFile)), i18n("File already exists")) != KMessageBox::Yes)
      reallySaveFile = false;
  }
  return reallySaveFile;
}

K_PLUGIN_FACTORY_WITH_JSON(CSVExporterFactory, "csvexporter.json", registerPlugin<CSVExporter>();)

#include "csvexporter.moc"
