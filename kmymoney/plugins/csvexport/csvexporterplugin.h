/***************************************************************************
                             csvexporterplugin.h
                             -------------------
    begin                : Wed Mar 20 2013
    copyright            : (C) 2013 by Allan Anderson
    email                : agander93@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CSVEXPORTERPLUGIN_H
#define CSVEXPORTERPLUGIN_H

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// QT Includes

// Project Includes

#include "kmymoneyplugin.h"

class CsvExportDlg;

class CsvExporterPlugin : public KMyMoneyPlugin::Plugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org.kmymoney.plugins.csvexport" FILE "csvexport.json")

public:
  explicit CsvExporterPlugin();
  ~CsvExporterPlugin();

  QAction*          m_action;
  bool              okToWriteFile(const QUrl &url);
  CsvExportDlg*     exporterDialog() {
    return m_dlg;
  }

private:
  CsvExportDlg*     m_dlg;

protected Q_SLOTS:
  void slotCsvExport();

protected:
  void createActions();
};

#endif
