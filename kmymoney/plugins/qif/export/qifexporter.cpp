/*
 * Copyright 2017       Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 * Copyright 2019       Thomas Baumgart <tbaumgart@kde.org>
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
  qDebug("Plugins: qifexporter unloaded");
}


void QIFExporter::createActions()
{
  const auto &kpartgui = QStringLiteral("file_export_qif");
  m_action = actionCollection()->addAction(kpartgui);
  m_action->setText(i18n("QIF..."));
  connect(m_action, &QAction::triggered, this, &QIFExporter::slotQifExport);
  connect(viewInterface(), &KMyMoneyPlugin::ViewInterface::viewStateChanged, action(qPrintable(kpartgui)), &QAction::setEnabled);
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
