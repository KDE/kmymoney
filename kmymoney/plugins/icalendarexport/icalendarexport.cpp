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

#include <config-kmymoney.h>

#include "icalendarexport.h"

// KDE includes
#include <kgenericfactory.h>
#include <kaction.h>
#include <kfiledialog.h>
#include <kplugininfo.h>
#include <kurl.h>
#include <kactioncollection.h>

// KMyMoney includes
#include "mymoneyfile.h"
#include "pluginloader.h"

#include "schedulestoicalendar.h"
#include "pluginsettings.h"

typedef KGenericFactory<KMMiCalendarExportPlugin> icalendarexportFactory;

K_EXPORT_COMPONENT_FACTORY(kmm_icalendarexport, icalendarexportFactory("kmm_icalendarexport"))

struct KMMiCalendarExportPlugin::Private {
  KAction* m_action;
  QString  m_profileName;
  QString  m_iCalendarFileEntryName;
  KMMSchedulesToiCalendar m_exporter;
};

KMMiCalendarExportPlugin::KMMiCalendarExportPlugin(QObject *parent, const QStringList&)
    : KMyMoneyPlugin::Plugin(parent, "iCalendar"/*must be the same as X-KDE-PluginInfo-Name*/),
    d(new Private)
{
  d->m_profileName = "iCalendarPlugin";
  d->m_iCalendarFileEntryName = "iCalendarFile";

  // Tell the host application to load my GUI component
  setComponentData(icalendarexportFactory::componentData());
  setXMLFile("kmm_icalendarexport.rc");

  // For ease announce that we have been loaded.
  qDebug("KMyMoney iCalendar plugin loaded");

  // Create the actions of this plugin
  QString actionName = i18n("Schedules to icalendar");
  QString icalFilePath;
  // Note the below code only exists to move existing settings to the new plugin specific config
  KConfigGroup config = KGlobal::config()->group(d->m_profileName);
  icalFilePath = config.readEntry(d->m_iCalendarFileEntryName, icalFilePath);

  // read the settings
  PluginSettings::self()->readConfig();

  if (!icalFilePath.isEmpty()) {
    // move the old setting to the new config
    PluginSettings::setIcalendarFile(icalFilePath);
    PluginSettings::self()->writeConfig();
    KGlobal::config()->deleteGroup(d->m_profileName);
  } else {
    // read it from the new config
    icalFilePath = PluginSettings::icalendarFile();
  }

  if (!icalFilePath.isEmpty())
    actionName = i18n("Schedules to icalendar [%1]", icalFilePath);

  d->m_action = actionCollection()->addAction("file_export_icalendar");
  d->m_action->setText(actionName);

  connect(KMyMoneyPlugin::PluginLoader::instance(), SIGNAL(plug(KPluginInfo*)), this, SLOT(slotPlug(KPluginInfo*)));
  connect(KMyMoneyPlugin::PluginLoader::instance(), SIGNAL(unplug(KPluginInfo*)), this, SLOT(slotUnplug(KPluginInfo*)));
  connect(KMyMoneyPlugin::PluginLoader::instance(), SIGNAL(configChanged(Plugin*)), this, SLOT(slotUpdateConfig()));
}

KMMiCalendarExportPlugin::~KMMiCalendarExportPlugin()
{
  delete d;
}

void KMMiCalendarExportPlugin::slotFirstExport(void)
{
  QPointer<KFileDialog> fileDialog = new KFileDialog(KUrl("kfiledialog:///kmymoney-export"), QString("%1|%2\n").arg("*.ics").arg(i18nc("ICS (Filefilter)", "iCalendar files")), d->m_action->parentWidget());

  fileDialog->setOperationMode(KFileDialog::Saving);
  fileDialog->setCaption(i18n("Export as"));

  if (fileDialog->exec() == QDialog::Accepted) {
    KUrl newURL = fileDialog->selectedUrl();
    if (newURL.isLocalFile()) {
      PluginSettings::setIcalendarFile(newURL.toLocalFile());
      slotExport();
    }
  }
  delete fileDialog;
}

void KMMiCalendarExportPlugin::slotExport(void)
{
  QString icalFilePath = PluginSettings::icalendarFile();
  if (!icalFilePath.isEmpty())
    d->m_exporter.exportToFile(icalFilePath);
}

void KMMiCalendarExportPlugin::slotPlug(KPluginInfo* info)
{
  if (info->pluginName() == objectName()) {
    connect(MyMoneyFile::instance(), SIGNAL(dataChanged()), this, SLOT(slotExport()));
  }
}

void KMMiCalendarExportPlugin::slotUnplug(KPluginInfo* info)
{
  if (info->pluginName() == objectName()) {
    disconnect(MyMoneyFile::instance(), SIGNAL(dataChanged()), this, SLOT(slotExport()));
  }
}

void KMMiCalendarExportPlugin::slotUpdateConfig(void)
{
  PluginSettings::self()->readConfig();
  // export the schedules because the configuration has changed
  QString icalFilePath = PluginSettings::icalendarFile();
  if (!icalFilePath.isEmpty())
    d->m_exporter.exportToFile(icalFilePath);
}

#include "icalendarexport.moc"
