/*
    SPDX-FileCopyrightText: 2010-2014 Allan Anderson <agander93@gmail.com>
    SPDX-FileCopyrightText: 2016-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2021 Dawid Wróbel <me@dawidwrobel.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CSVIMPORTER_H
#define CSVIMPORTER_H

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// QT Includes

// Project Includes

#include "kmymoneyplugin.h"

class CSVImporter : public KMyMoneyPlugin::Plugin, public KMyMoneyPlugin::ImporterPlugin
{
    Q_OBJECT
    Q_INTERFACES(KMyMoneyPlugin::ImporterPlugin)

public:
    explicit CSVImporter(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args);
    ~CSVImporter() override;

    virtual QStringList formatMimeTypes() const override;

    /**
      * Import a file
      *
      * @param filename File to import
      *
      * @return bool Whether the import was successful.
      */
    virtual bool import(const QString& filename) override;

    /**
      * Returns the error result of the last import
      *
      * @return QString English-language name of the error encountered in the
      *  last import, or QString() if it was successful.
      *
      */
    virtual QString lastError() const override;

private:
    bool              m_silent;
    QAction*          m_action;

protected Q_SLOTS:
    void startWizardRun();

protected:
    void createActions();
};

#endif
