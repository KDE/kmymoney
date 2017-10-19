/***************************************************************************
                            csvimporterplugin.cpp
                       (based on ofximporterplugin.cpp)
                             -------------------
    begin                : Sat Jan 01 2010
    copyright            : (C) 2010 by Allan Anderson
    email                : agander93@gmail.com
    copyright            : (C) 2016 by Łukasz Wojniłowicz
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

#include "csvimporterplugin.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QFile>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KActionCollection>
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "csvimporter.h"
#include "csvwizard.h"
#include "statementinterface.h"

CsvImporterPlugin::CsvImporterPlugin() :
    KMyMoneyPlugin::Plugin(nullptr, "csvimport"/*must be the same as X-KDE-PluginInfo-Name*/)
{
  setComponentName("kmm_csvimport", i18n("CSV importer"));
  setXMLFile("kmm_csvimport.rc");
  createActions();
  // For information, announce that we have been loaded.
  qDebug("KMyMoney csvimport plugin loaded");
}

CsvImporterPlugin::~CsvImporterPlugin()
{
}

void CsvImporterPlugin::createActions()
{
  m_action = actionCollection()->addAction("file_import_csv");
  m_action->setText(i18n("CSV..."));
  connect(m_action, SIGNAL(triggered(bool)), this, SLOT(startWizardRun()));
}

void CsvImporterPlugin::startWizardRun()
{
  m_action->setEnabled(false);
  m_importer = new CSVImporter;
  m_wizard = new CSVWizard(this, m_importer);
  m_silent = false;
  connect(m_wizard, SIGNAL(statementReady(MyMoneyStatement&)), this, SLOT(slotGetStatement(MyMoneyStatement&)));
  m_action->setEnabled(false);//  don't allow further plugins to start while this is open
}

bool CsvImporterPlugin::slotGetStatement(MyMoneyStatement& s)
{
  bool ret = statementInterface()->import(s, m_silent);
  delete m_importer;
  return ret;
}

QString CsvImporterPlugin::formatName() const
{
  return QLatin1String("CSV");
}

QString CsvImporterPlugin::formatFilenameFilter() const
{
  return "*.csv";
}

bool CsvImporterPlugin::isMyFormat(const QString& filename) const
{
  // filename is considered a CSV file if it can be opened
  // and the filename ends in ".csv".
  bool result = false;

  QFile f(filename);
  if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
    result = f.fileName().endsWith(QLatin1String(".csv"));
    f.close();
  }

  return result;
}

bool CsvImporterPlugin::import(const QString& filename)
{
  bool rc = true;
  m_importer = new CSVImporter;
  m_wizard = new CSVWizard(this, m_importer);
  m_wizard->presetFilename(filename);
  m_silent = false;
  connect(m_wizard, SIGNAL(statementReady(MyMoneyStatement&)), this, SLOT(slotGetStatement(MyMoneyStatement&)));
  return rc;
}

QString CsvImporterPlugin::lastError() const
{
  return QString();
}
