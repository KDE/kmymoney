/***************************************************************************
 *   Copyright 2009  Cristian Onet onet.cristian@gmail.com                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public License as        *
 *   published by the Free Software Foundation; either version 2 of        *
 *   the License or (at your option) version 3 or any later version        *
 *   accepted by the membership of KDE e.V. (or its successor approved     *
 *   by the membership of KDE e.V.), which shall act as a proxy            *
 *   defined in Section 14 of version 3 of the license.                    *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>  *
 ***************************************************************************/

#include "icalendarexporter.h"

#include <QFileDialog>
#include <QUrl>
#include <QAction>

// KDE includes
#include <KPluginFactory>
#include <KPluginInfo>
#include <KActionCollection>
#include <KSharedConfig>
#include <KLocalizedString>

// KMyMoney includes
#include "mymoneyfile.h"
#include "pluginloader.h"

#include "schedulestoicalendar.h"
#include "pluginsettings.h"
#include "kmymoneyglobalsettings.h"

struct iCalendarExporter::Private {
  QAction* m_action;
  QString  m_profileName;
  QString  m_iCalendarFileEntryName;
  KMMSchedulesToiCalendar m_exporter;
};

iCalendarExporter::iCalendarExporter(QObject *parent, const QVariantList &args) :
  KMyMoneyPlugin::Plugin(parent, "icalendarexporter"/*must be the same as X-KDE-PluginInfo-Name*/),
    d(std::unique_ptr<Private>(new Private))
{
  Q_UNUSED(args);
  d->m_profileName = "iCalendarPlugin";
  d->m_iCalendarFileEntryName = "iCalendarFile";

  // Tell the host application to load my GUI component
  setComponentName("icalendarexporter", i18n("iCalendar exporter"));
  setXMLFile("icalendarexporter.rc");

  // For ease announce that we have been loaded.
  qDebug("Plugins: icalendarexporter loaded");

  // Create the actions of this plugin
  QString actionName = i18n("Schedules to iCalendar");
  QString icalFilePath;
  // Note the below code only exists to move existing settings to the new plugin specific config
  KConfigGroup config = KSharedConfig::openConfig()->group(d->m_profileName);
  icalFilePath = config.readEntry(d->m_iCalendarFileEntryName, icalFilePath);

  // read the settings
  PluginSettings::self()->load();

  if (!icalFilePath.isEmpty()) {
    // move the old setting to the new config
    PluginSettings::setIcalendarFile(icalFilePath);
    PluginSettings::self()->save();
    KSharedConfig::openConfig()->deleteGroup(d->m_profileName);
  } else {
    // read it from the new config
    icalFilePath = PluginSettings::icalendarFile();
  }

  if (!icalFilePath.isEmpty())
    actionName = i18n("Schedules to iCalendar [%1]", icalFilePath);

  d->m_action = actionCollection()->addAction("file_export_icalendar");
  d->m_action->setText(actionName);
  connect(d->m_action, &QAction::triggered, this, &iCalendarExporter::slotFirstExport);
}

iCalendarExporter::~iCalendarExporter()
{
  qDebug("Plugins: icalendarexporter unloaded");
}

void iCalendarExporter::injectExternalSettings(KMyMoneySettings* p)
{
  KMyMoneyGlobalSettings::injectExternalSettings(p);
}

void iCalendarExporter::slotFirstExport()
{
  QPointer<QFileDialog> fileDialog = new QFileDialog(d->m_action->parentWidget(), QString(), QString(), QString("%1|%2\n").arg("*.ics").arg(i18nc("ICS (Filefilter)", "iCalendar files")));

  fileDialog->setAcceptMode(QFileDialog::AcceptSave);
  fileDialog->setWindowTitle(i18n("Export as"));

  if (fileDialog->exec() == QDialog::Accepted) {
    QUrl newURL = fileDialog->selectedUrls().front();
    if (newURL.isLocalFile()) {
      PluginSettings::setIcalendarFile(newURL.toLocalFile());
      PluginSettings::self()->save();
      slotExport();
    }
  }
  delete fileDialog;
}

void iCalendarExporter::slotExport()
{
  QString icalFilePath = PluginSettings::icalendarFile();
  if (!icalFilePath.isEmpty())
    d->m_exporter.exportToFile(icalFilePath);
}

void iCalendarExporter::plug()
{
  connect(MyMoneyFile::instance(), &MyMoneyFile::dataChanged, this, &iCalendarExporter::slotExport);
}

void iCalendarExporter::unplug()
{
  disconnect(MyMoneyFile::instance(), &MyMoneyFile::dataChanged, this, &iCalendarExporter::slotExport);
}

void iCalendarExporter::configurationChanged()
{
  PluginSettings::self()->load();
  // export the schedules because the configuration has changed
  QString icalFilePath = PluginSettings::icalendarFile();
  if (!icalFilePath.isEmpty())
    d->m_exporter.exportToFile(icalFilePath);
}

K_PLUGIN_FACTORY_WITH_JSON(iCalendarExporterFactory, "icalendarexporter.json", registerPlugin<iCalendarExporter>();)

#include "icalendarexporter.moc"
