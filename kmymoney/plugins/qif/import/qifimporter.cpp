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
#include "qifimporter.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <KPluginFactory>
#include <KActionCollection>
#include <KLocalizedString>
#include <KMessageBox>

// ----------------------------------------------------------------------------
// Project Includes

#include "kimportdlg.h"
#include "mymoneyqifreader.h"
#include "statementinterface.h"
#include "viewinterface.h"

class MyMoneyStatement;

QIFImporter::QIFImporter(QObject *parent, const QVariantList &args) :
    KMyMoneyPlugin::Plugin(parent, "qifimporter"/*must be the same as X-KDE-PluginInfo-Name*/),
    m_qifReader(nullptr)
{
  Q_UNUSED(args);
  const auto componentName = QLatin1String("qifimporter");
  const auto rcFileName = QLatin1String("qifimporter.rc");
  setComponentName(componentName, i18n("QIF importer"));

#ifdef IS_APPIMAGE
  const QString rcFilePath = QString("%1/../share/kxmlgui5/%2/%3").arg(QCoreApplication::applicationDirPath(), componentName, rcFileName);
  setXMLFile(rcFilePath);

  const QString localRcFilePath = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation).first() + QLatin1Char('/') + rcFileName;
  setLocalXMLFile(localRcFilePath);
#else
  setXMLFile(rcFileName);
#endif

  createActions();
  // For information, announce that we have been loaded.
  qDebug("Plugins: qifimporter loaded");
}

QIFImporter::~QIFImporter()
{
  delete m_qifReader;
  qDebug("Plugins: qifimporter unloaded");
}

void QIFImporter::createActions()
{
  const auto &kpartgui = QStringLiteral("file_import_qif");
  m_action = actionCollection()->addAction(kpartgui);
  m_action->setText(i18n("QIF..."));
  connect(m_action, &QAction::triggered, this, &QIFImporter::slotQifImport);
  connect(viewInterface(), &KMyMoneyPlugin::ViewInterface::viewStateChanged, action(qPrintable(kpartgui)), &QAction::setEnabled);
}

void QIFImporter::slotQifImport()
{
  QPointer<KImportDlg> dlg = new KImportDlg(nullptr);

  if (dlg->exec() == QDialog::Accepted && dlg != nullptr) {
    m_action->setEnabled(false);
    delete m_qifReader;
    m_qifReader = new MyMoneyQifReader;
    statementInterface()->resetMessages();
    connect(m_qifReader, &MyMoneyQifReader::statementsReady, this, &QIFImporter::slotGetStatements);

    m_qifReader->setURL(dlg->file());
    m_qifReader->setProfile(dlg->profile());
    m_qifReader->setCategoryMapping(dlg->m_typeComboBox->currentIndex() == 0);
    if (!m_qifReader->startImport()) {
      delete m_qifReader;
      statementInterface()->showMessages(0);
      m_action->setEnabled(true);
    }
  }
  delete dlg;
}

bool QIFImporter::slotGetStatements(const QList<MyMoneyStatement> &statements)
{
  auto ret = true;
  for (const auto& statement : statements) {
    const auto singleImportSummary = statementInterface()->import(statement);
    if (singleImportSummary.isEmpty())
      ret = false;
  }

  // inform the user about the result of the operation
  statementInterface()->showMessages(statements.count());

  // allow further QIF imports
  m_action->setEnabled(true);
  return ret;
}

K_PLUGIN_FACTORY_WITH_JSON(QIFImporterFactory, "qifimporter.json", registerPlugin<QIFImporter>();)

#include "qifimporter.moc"
