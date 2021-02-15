/*
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <config-kmymoney.h>
#include "qifexporter.h"

// ----------------------------------------------------------------------------
// QT Includes
#ifdef IS_APPIMAGE
#include <QCoreApplication>
#include <QStandardPaths>
#endif

// ----------------------------------------------------------------------------
// KDE Includes

#include <KPluginFactory>
#include <KActionCollection>
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "kexportdlg.h"

#include "mymoneyqifwriter.h"
#include "viewinterface.h"

QIFExporter::QIFExporter(QObject *parent, const QVariantList &args) :
    KMyMoneyPlugin::Plugin(parent, "qifexporter"/*must be the same as X-KDE-PluginInfo-Name*/)
{
  Q_UNUSED(args);
  const auto componentName = QLatin1String("qifexporter");
  const auto rcFileName = QLatin1String("qifexporter.rc");
  setComponentName(componentName, i18n("QIF exporter"));

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
  qDebug("Plugins: qifexporter loaded");
}

QIFExporter::~QIFExporter()
{
  actionCollection()->removeAction(m_action);
  qDebug("Plugins: qifexporter unloaded");
}


void QIFExporter::createActions()
{
  m_action = actionCollection()->addAction(QStringLiteral("file_export_qif"));
  m_action->setText(i18n("QIF..."));
  connect(m_action, &QAction::triggered, this, &QIFExporter::slotQifExport);
  connect(viewInterface(), &KMyMoneyPlugin::ViewInterface::viewStateChanged, m_action, &QAction::setEnabled);
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
