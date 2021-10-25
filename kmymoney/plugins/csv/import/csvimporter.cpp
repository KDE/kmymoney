/*
    SPDX-FileCopyrightText: 2010-2014 Allan Anderson <agander93@gmail.com>
    SPDX-FileCopyrightText: 2016-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2018-2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2021 Dawid Wróbel <me@dawidwrobel.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <config-kmymoney.h>
#include "csvimporter.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QFile>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KPluginFactory>
#include <KActionCollection>
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "core/csvimportercore.h"
#include "csvwizard.h"
#include "statementinterface.h"
#include "viewinterface.h"

#if KCOREADDONS_VERSION < QT_VERSION_CHECK(5, 77, 0)
CSVImporter::CSVImporter(QObject *parent, const QVariantList &args)
    : KMyMoneyPlugin::Plugin(parent, args)
#else
CSVImporter::CSVImporter(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args)
    : KMyMoneyPlugin::Plugin(parent, metaData, args)
#endif
    , m_action(nullptr)
{
    Q_INIT_RESOURCE(csvimporter);

    const auto rcFileName = QLatin1String("csvimporter.rc");
    setXMLFile(rcFileName);

    createActions();
    // For information, announce that we have been loaded.
    qDebug("Plugins: csvimporter loaded");
}

CSVImporter::~CSVImporter()
{
    actionCollection()->removeAction(m_action);
    qDebug("Plugins: csvimporter unloaded");
}

void CSVImporter::createActions()
{
    const auto &kpartgui = QStringLiteral("file_import_csv");
    m_action = actionCollection()->addAction(kpartgui);
    m_action->setText(i18n("CSV..."));
    connect(m_action, &QAction::triggered, this, &CSVImporter::startWizardRun);
    connect(viewInterface(), &KMyMoneyPlugin::ViewInterface::viewStateChanged, action(qPrintable(kpartgui)), &QAction::setEnabled);
}

QStringList CSVImporter::formatMimeTypes() const
{
    return {"text/csv", "text/tab-separated-values"};
}

void CSVImporter::startWizardRun()
{
    import(QString());
}

bool CSVImporter::import(const QString& filename)
{
    QPointer<CSVWizard> wizard = new CSVWizard(this);
    wizard->presetFilename(filename);
    auto rc = false;

    if ((wizard->exec() == QDialog::Accepted) && wizard) {
        rc =  !statementInterface()->import(wizard->statement(), false).isEmpty();
    }
    wizard->deleteLater();
    return rc;
}

QString CSVImporter::lastError() const
{
    return QString();
}

K_PLUGIN_CLASS_WITH_JSON(CSVImporter, "csvimporter.json")

#include "csvimporter.moc"
