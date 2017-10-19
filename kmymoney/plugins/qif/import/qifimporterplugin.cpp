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

#include "qifimporterplugin.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <KActionCollection>
#include <KLocalizedString>
#include <KPluginFactory>

// ----------------------------------------------------------------------------
// Project Includes

#include "kimportdlg.h"
#include "mymoneyqifreader.h"
#include "statementinterface.h"

class MyMoneyStatement;

QIFImporterPlugin::QIFImporterPlugin() :
    KMyMoneyPlugin::Plugin(nullptr, "qifimport"/*must be the same as X-KDE-PluginInfo-Name*/)
{
  setComponentName("kmm_qifimport", i18n("QIF importer"));
  setXMLFile("kmm_qifimport.rc");
  createActions();
  // For information, announce that we have been loaded.
  qDebug("KMyMoney qifimport plugin loaded");
}

QIFImporterPlugin::~QIFImporterPlugin()
{
}

void QIFImporterPlugin::createActions()
{
  m_action = actionCollection()->addAction("file_import_qif");
  m_action->setText(i18n("QIF..."));
  connect(m_action, &QAction::triggered, this, &QIFImporterPlugin::slotQifImport);
}

void QIFImporterPlugin::slotQifImport()
{
  m_action->setEnabled(false);
  QPointer<KImportDlg> dlg = new KImportDlg(nullptr);

  if (dlg->exec() == QDialog::Accepted && dlg != nullptr) {
    m_qifReader = new MyMoneyQifReader;
    connect(m_qifReader, &MyMoneyQifReader::statementsReady, this, &QIFImporterPlugin::slotGetStatements);

    m_qifReader->setURL(dlg->file());
    m_qifReader->setProfile(dlg->profile());
    m_qifReader->setCategoryMapping(dlg->m_typeComboBox->currentIndex() == 0);
    if (!m_qifReader->startImport())
      delete m_qifReader;
  }
  delete dlg;
  m_action->setEnabled(true);
}

bool QIFImporterPlugin::slotGetStatements(QList<MyMoneyStatement> &statements)
{
  auto ret = true;
  foreach (const auto statement, statements)
    ret &= statementInterface()->import(statement);

  delete m_qifReader;
  return ret;
}

//#include "qifimporterplugin.moc"
