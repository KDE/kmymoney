/*
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2021 Dawid Wróbel <me@dawidwrobel.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef QIFEXPORTER_H
#define QIFEXPORTER_H

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// QT Includes

// Project Includes

#include "kmymoneyplugin.h"

class MyMoneyQifReader;

class QIFExporter : public KMyMoneyPlugin::Plugin
{
    Q_OBJECT

public:
    explicit QIFExporter(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args);
    ~QIFExporter() override;

    QAction          *m_action;

    MyMoneyQifReader *m_qifReader;

public Q_SLOTS:
    /**
      * Called when the user wishes to export some transaction to a
      * QIF formatted file. An account must be open for this to work.
      * Uses MyMoneyQifWriter() for the actual output.
      */
    void slotQifExport();

protected:
    void createActions();
};

#endif
