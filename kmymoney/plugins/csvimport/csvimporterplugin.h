/***************************************************************************
                             csvimporterplugin.h
                             -------------------
    begin                : Sat Jan 01 2010
    copyright            : (C) 2010 by Allan Anderson
    email                : agander93@gmail.com
    copyright            : (C) 2016-2017 by Łukasz Wojniłowicz
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

#ifndef CSVIMPORTERPLUGIN_H
#define CSVIMPORTERPLUGIN_H

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// QT Includes

// Project Includes

#include "kmymoneyplugin.h"

class CSVImporter;
class CSVWizard;

class CsvImporterPlugin : public KMyMoneyPlugin::Plugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org.kmymoney.plugins.csvimport" FILE "csvimport.json")
  
public:
  explicit CsvImporterPlugin();
  ~CsvImporterPlugin();

  QAction*          m_action;
  CSVWizard*        m_wizard;
  CSVImporter*      m_importer;
private:
  bool              m_silent;
public slots:
  bool slotGetStatement(MyMoneyStatement& s);

protected slots:
  void startWizardRun();

protected:
  void createActions();
};

#endif
