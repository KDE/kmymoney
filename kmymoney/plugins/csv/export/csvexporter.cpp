/*
    SPDX-FileCopyrightText: 2013-2014 Allan Anderson <agander93@gmail.com>
    SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <config-kmymoney.h>
#include "csvexporter.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QUrl>
#ifdef IS_APPIMAGE
#include <QCoreApplication>
#include <QStandardPaths>
#endif

// ----------------------------------------------------------------------------
// KDE Includes

#include <KActionCollection>
#include <KIO/StatJob>
#include <KLocalizedString>
#include <KMessageBox>
#include <KPluginFactory>
#include <kio_version.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "csvexportdlg.h"
#include "csvwriter.h"
#include "viewinterface.h"

CSVExporter::CSVExporter(QObject *parent, const QVariantList &args)
    : KMyMoneyPlugin::Plugin(parent, "csvexporter"/*must be the same as X-KDE-PluginInfo-Name*/)
    , m_action(nullptr)
{
    Q_UNUSED(args);
    const auto componentName = QLatin1String("csvexporter");
    const auto rcFileName = QLatin1String("csvexporter.rc");
    setComponentName(componentName, i18n("CSV exporter"));

#ifdef IS_APPIMAGE
    const QString rcFilePath = QString("%1/../share/kxmlgui5/%2/%3").arg(QCoreApplication::applicationDirPath(), componentName, rcFileName);
    setXMLFile(rcFilePath);

    const QString localRcFilePath = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation).first() + QLatin1Char('/') + componentName + QLatin1Char('/') + rcFileName;
    setLocalXMLFile(localRcFilePath);
#else
    setXMLFile(rcFileName);
#endif

    createActions();
    // For information, announce that we have been loaded.
    qDebug("Plugins: csvexporter loaded");
}

CSVExporter::~CSVExporter()
{
    actionCollection()->removeAction(m_action);
    qDebug("Plugins: csvexporter unloaded");
}

void CSVExporter::createActions()
{
    const auto &kpartgui = QStringLiteral("file_export_csv");
    m_action = actionCollection()->addAction(kpartgui);
    m_action->setText(i18n("&CSV..."));
    connect(m_action, &QAction::triggered, this, &CSVExporter::slotCsvExport);
    connect(viewInterface(), &KMyMoneyPlugin::ViewInterface::viewStateChanged, action(qPrintable(kpartgui)), &QAction::setEnabled);
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
#if KIO_VERSION < QT_VERSION_CHECK(5, 70, 0)
        short int detailLevel = 0; // Lowest level: file/dir/symlink/none
        KIO::StatJob* statjob = KIO::stat(url, KIO::StatJob::SourceSide, detailLevel);
#else
        auto statjob = KIO::statDetails(url, KIO::StatJob::SourceSide, KIO::StatNoDetails);
#endif
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
