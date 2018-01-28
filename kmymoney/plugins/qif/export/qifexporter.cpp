/***************************************************************************
                            qifexporter.cpp
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

#include "qifexporter.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <KPluginFactory>
#include <KActionCollection>
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "kexportdlg.h"

#include "mymoneyqifwriter.h"

QIFExporter::QIFExporter(QObject *parent, const QVariantList &args) :
    KMyMoneyPlugin::Plugin(parent, "qifexporter"/*must be the same as X-KDE-PluginInfo-Name*/)
{
  Q_UNUSED(args);
  setComponentName("qifexporter", i18n("QIF exporter"));
  setXMLFile("qifexporter.rc");
  createActions();
  // For information, announce that we have been loaded.
  qDebug("Plugins: qifexporter loaded");
}

QIFExporter::~QIFExporter()
{
  qDebug("Plugins: qifexporter unloaded");
}


void QIFExporter::createActions()
{
  m_action = actionCollection()->addAction("file_export_qif");
  m_action->setText(i18n("QIF..."));
  connect(m_action, &QAction::triggered, this, &QIFExporter::slotQifExport);
}


void QIFExporter::slotQifExport()
{
  m_action->setEnabled(false);
  QPointer<KExportDlg> dlg = new KExportDlg(nullptr);
  if (dlg->exec() == QDialog::Accepted && dlg != nullptr) {
//    if (okToWriteFile(QUrl::fromLocalFile(dlg->filename()))) {
      MyMoneyQifWriter writer;
      connect(&writer, SIGNAL(signalProgress(int,int)), this, SLOT(slotStatusProgressBar(int,int)));

      writer.write(dlg->filename(), dlg->profile(), dlg->accountId(),
                   dlg->accountSelected(), dlg->categorySelected(),
                   dlg->startDate(), dlg->endDate());
//    }
  }
  delete dlg;
  m_action->setEnabled(true);
}

K_PLUGIN_FACTORY_WITH_JSON(QIFExporterFactory, "qifexporter.json", registerPlugin<QIFExporter>();)

#include "qifexporter.moc"
