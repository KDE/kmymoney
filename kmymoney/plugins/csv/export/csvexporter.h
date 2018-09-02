/*
 * Copyright 2013-2014  Allan Anderson <agander93@gmail.com>
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

#ifndef CSVEXPORTER_H
#define CSVEXPORTER_H

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// QT Includes

// Project Includes

#include "kmymoneyplugin.h"

class CsvExportDlg;

class CSVExporter : public KMyMoneyPlugin::Plugin
{
  Q_OBJECT

public:
  explicit CSVExporter(QObject *parent, const QVariantList &args);
  ~CSVExporter() override;

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
