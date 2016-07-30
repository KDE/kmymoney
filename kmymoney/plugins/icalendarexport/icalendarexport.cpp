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

#include "icalendarexport.h"

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

struct KMMiCalendarExportPlugin::Private {
  QAction* m_action;
  QString  m_profileName;
  QString  m_iCalendarFileEntryName;
  KMMSchedulesToiCalendar m_exporter;
};

KMMiCalendarExportPlugin::KMMiCalendarExportPlugin()
    : KMyMoneyPlugin::Plugin(nullptr, "iCalendar"/*must be the same as X-KDE-PluginInfo-Name*/),
    d(new Private)
{
  d->m_profileName = "iCalendarPlugin";
  d->m_iCalendarFileEntryName = "iCalendarFile";

  // Tell the host application to load my GUI component
  setComponentName("kmm_icalendarexport", i18n("iCalendar exporter"));
  setXMLFile("kmm_icalendarexport.rc");

  // For ease announce that we have been loaded.
  qDebug("KMyMoney iCalendar plugin loaded");

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
  connect(d->m_action, SIGNAL(triggered(bool)), this, SLOT(slotFirstExport()));

  connect(KMyMoneyPlugin::PluginLoader::instance(), SIGNAL(plug(KPluginInfo*)), this, SLOT(slotPlug(KPluginInfo*)));
  connect(KMyMoneyPlugin::PluginLoader::instance(), SIGNAL(unplug(KPluginInfo*)), this, SLOT(slotUnplug(KPluginInfo*)));
  connect(KMyMoneyPlugin::PluginLoader::instance(), SIGNAL(configChanged(Plugin*)), this, SLOT(slotUpdateConfig()));
}

KMMiCalendarExportPlugin::~KMMiCalendarExportPlugin()
{
  delete d;
}

void KMMiCalendarExportPlugin::slotFirstExport()
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

void KMMiCalendarExportPlugin::slotExport()
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

void KMMiCalendarExportPlugin::slotUpdateConfig()
{
  PluginSettings::self()->load();
  // export the schedules because the configuration has changed
  QString icalFilePath = PluginSettings::icalendarFile();
  if (!icalFilePath.isEmpty())
    d->m_exporter.exportToFile(icalFilePath);
}

#include "icalendarexport.moc"
