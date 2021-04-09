/*
    SPDX-FileCopyrightText: 2010-2014 Allan Anderson <agander93@gmail.com>
    SPDX-FileCopyrightText: 2016-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2018-2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <config-kmymoney.h>
#include "csvimporter.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QFile>
#ifdef IS_APPIMAGE
#include <QCoreApplication>
#include <QStandardPaths>
#endif

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

CSVImporter::CSVImporter(QObject *parent, const QVariantList &args)
    : KMyMoneyPlugin::Plugin(parent, "csvimporter"/*must be the same as X-KDE-PluginInfo-Name*/)
    , m_action(nullptr)
{
    Q_UNUSED(args);
    const auto componentName = QLatin1String("csvimporter");
    const auto rcFileName = QLatin1String("csvimporter.rc");
    setComponentName(componentName, i18n("CSV importer"));

#ifdef IS_APPIMAGE
    const QString rcFilePath = QString("%1/../share/kxmlgui5/%2/%3").arg(QCoreApplication::applicationDirPath(), componentName, rcFileName);
    setXMLFile(rcFilePath);

    const QString localRcFilePath = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation).first() + QLatin1Char('/') + componentName + QLatin1Char('/') + rcFileName;
    setLocalXMLFile(localRcFilePath);
#else
    setXMLFile(rcFileName);
#endif


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

QString CSVImporter::formatName() const
{
    return QLatin1String("CSV");
}

QString CSVImporter::formatFilenameFilter() const
{
    return "*.csv *.txt";
}

bool CSVImporter::isMyFormat(const QString& filename) const
{
    QFile f(filename);
    return f.open(QIODevice::ReadOnly | QIODevice::Text);
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

K_PLUGIN_FACTORY_WITH_JSON(CSVImporterFactory, "csvimporter.json", registerPlugin<CSVImporter>();)

#include "csvimporter.moc"
