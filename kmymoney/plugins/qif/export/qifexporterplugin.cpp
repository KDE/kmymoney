/***************************************************************************
                            qifimporterplugin.cpp
                       (based on qifimporterplugin.cpp)
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

#include "qifexporterplugin.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <KActionCollection>
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "kexportdlg.h"

#include "mymoneyqifwriter.h"


QIFExporterPlugin::QIFExporterPlugin() :
    KMyMoneyPlugin::Plugin(nullptr, "qifexport"/*must be the same as X-KDE-PluginInfo-Name*/)
{
  setComponentName("kmm_qifexport", i18n("QIF exporter"));
  setXMLFile("kmm_qifexport.rc");
  createActions();
  // For information, announce that we have been loaded.
  qDebug("KMyMoney qifexport plugin loaded");
}

QIFExporterPlugin::~QIFExporterPlugin()
{
}

void QIFExporterPlugin::createActions()
{
  m_action = actionCollection()->addAction("file_export_qif");
  m_action->setText(i18n("QIF..."));
  connect(m_action, &QAction::triggered, this, &QIFExporterPlugin::slotQifExport);
}


void QIFExporterPlugin::slotQifExport()
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

//#include "qifexporterplugin.moc"
