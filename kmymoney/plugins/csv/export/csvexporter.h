/*
    SPDX-FileCopyrightText: 2013-2014 Allan Anderson <agander93@gmail.com>
    SPDX-FileCopyrightText: 2021 Dawid Wróbel <me@dawidwrobel.com>
    SPDX-License-Identifier: GPL-2.0-or-later
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
    explicit CSVExporter(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args);
    ~CSVExporter() override;

    bool              okToWriteFile(const QUrl &url);
    CsvExportDlg*     exporterDialog() {
        return m_dlg;
    }

private:
    QAction*          m_action;
    CsvExportDlg*     m_dlg;

protected Q_SLOTS:
    void slotCsvExport();

protected:
    void createActions();
};

#endif
