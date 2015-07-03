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

#ifndef ICALENDAREXPORT_H
#define ICALENDAREXPORT_H

#include "kmymoneyplugin.h"
#include "mymoneyaccount.h"
#include "mymoneykeyvaluecontainer.h"

class QStringList;
class KPluginInfo;

class KMMiCalendarExportPlugin: public KMyMoneyPlugin::Plugin
{
  Q_OBJECT

public:
  KMMiCalendarExportPlugin(QObject *parent, const QVariantList&);
  ~KMMiCalendarExportPlugin();

protected slots:
  // this is the export function called when the user selects the interface menu
  void slotFirstExport();
  // this is the export method called automatically
  void slotExport();
  // the plugin loader plugs in a plugin
  void slotPlug(KPluginInfo*);
  // the plugin loader unplugs a plugin
  void slotUnplug(KPluginInfo*);
  // the plugin's configurations has changed
  void slotUpdateConfig();

private:
  struct Private;
  Private *d;
};

#endif // ICALENDAREXPORT_H

