/*
    SPDX-FileCopyrightText: 2009 Cristian Onet <onet.cristian@gmail.com>
    SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2021 Dawid Wróbel <me@dawidwrobel.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <config-kmymoney.h>
#include "icalendarexporter.h"

// Qt includes
#include <QFileDialog>
#include <QUrl>
#include <QAction>

// KDE includes
#include <KPluginFactory>
#include <KActionCollection>
#include <KSharedConfig>
#include <KLocalizedString>

// KMyMoney includes
#include "mymoneyfile.h"
#include "pluginloader.h"

#include "schedulestoicalendar.h"
#include "icalendarsettings.h"
#include "viewinterface.h"

struct iCalendarExporter::Private {
    QAction* m_action;
    QString  m_profileName;
    QString  m_iCalendarFileEntryName;
    KMMSchedulesToiCalendar m_exporter;
};

#if KCOREADDONS_VERSION < QT_VERSION_CHECK(5, 77, 0)
iCalendarExporter::iCalendarExporter(QObject *parent, const QVariantList &args) :
    KMyMoneyPlugin::Plugin(parent, args),
#else
iCalendarExporter::iCalendarExporter(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args) :
    KMyMoneyPlugin::Plugin(parent, metaData, args),
#endif
    d(std::unique_ptr<Private>(new Private))
{
    Q_INIT_RESOURCE(icalendarexporter);

    d->m_profileName = "iCalendarPlugin";
    d->m_iCalendarFileEntryName = "iCalendarFile";

    const auto rcFileName = QLatin1String("icalendarexporter.rc");
    setXMLFile(rcFileName);

    // For ease announce that we have been loaded.
    qDebug("Plugins: icalendarexporter loaded");

    // Create the actions of this plugin
    QString actionName = i18n("Schedules to iCalendar");
    QString icalFilePath;
    // Note the below code only exists to move existing settings to the new plugin specific config
    KConfigGroup config = KSharedConfig::openConfig()->group(d->m_profileName);
    icalFilePath = config.readEntry(d->m_iCalendarFileEntryName, icalFilePath);

    // read the settings
    ICalendarSettings::self()->load();

    if (!icalFilePath.isEmpty()) {
        // move the old setting to the new config
        ICalendarSettings::setIcalendarFile(icalFilePath);
        ICalendarSettings::self()->save();
        KSharedConfig::openConfig()->deleteGroup(d->m_profileName);
    } else {
        // read it from the new config
        icalFilePath = ICalendarSettings::icalendarFile();
    }

    if (!icalFilePath.isEmpty())
        actionName = i18n("Schedules to iCalendar [%1]", icalFilePath);

    const auto &kpartgui = QStringLiteral("file_export_icalendar");
    d->m_action = actionCollection()->addAction(kpartgui);
    d->m_action->setText(actionName);
    connect(d->m_action, &QAction::triggered, this, &iCalendarExporter::slotFirstExport);
    connect(viewInterface(), &KMyMoneyPlugin::ViewInterface::viewStateChanged, action(qPrintable(kpartgui)), &QAction::setEnabled);
}

iCalendarExporter::~iCalendarExporter()
{
    actionCollection()->removeAction(d->m_action);
    qDebug("Plugins: icalendarexporter unloaded");
}

void iCalendarExporter::slotFirstExport()
{
    QPointer<QFileDialog> fileDialog = new QFileDialog(d->m_action->parentWidget(), QString(), QString(), QString("%1|%2\n").arg("*.ics").arg(i18nc("ICS (Filefilter)", "iCalendar files")));

    fileDialog->setAcceptMode(QFileDialog::AcceptSave);
    fileDialog->setWindowTitle(i18n("Export as"));

    if (fileDialog->exec() == QDialog::Accepted) {
        QUrl newURL = fileDialog->selectedUrls().front();
        if (newURL.isLocalFile()) {
            ICalendarSettings::setIcalendarFile(newURL.toLocalFile());
            ICalendarSettings::self()->save();
            slotExport();
        }
    }
    delete fileDialog;
}

void iCalendarExporter::slotExport()
{
    QString icalFilePath = ICalendarSettings::icalendarFile();
    if (!icalFilePath.isEmpty())
        d->m_exporter.exportToFile(icalFilePath);
}

void iCalendarExporter::plug(KXMLGUIFactory* guiFactory)
{
    Q_UNUSED(guiFactory)
    connect(MyMoneyFile::instance(), &MyMoneyFile::dataChanged, this, &iCalendarExporter::slotExport);
}

void iCalendarExporter::unplug()
{
    disconnect(MyMoneyFile::instance(), &MyMoneyFile::dataChanged, this, &iCalendarExporter::slotExport);
}

void iCalendarExporter::updateConfiguration()
{
    ICalendarSettings::self()->load();
    // export the schedules because the configuration has changed
    QString icalFilePath = ICalendarSettings::icalendarFile();
    if (!icalFilePath.isEmpty())
        d->m_exporter.exportToFile(icalFilePath);
}

K_PLUGIN_CLASS_WITH_JSON(iCalendarExporter, "icalendarexporter.json")

#include "icalendarexporter.moc"
