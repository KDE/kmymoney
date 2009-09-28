/***************************************************************************
 *   Copyright (C) 2008 by Cristian Onet                                   *
 *   onet.cristian@gmail.com                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
// Qt includes
#include <qcombobox.h>

// KDE includes
#include <kgenericfactory.h>

#include "kcm_icalendarexport.h"
#include "pluginsettings.h"
#include "ui_pluginsettingsdecl.h"

class PluginSettingsWidget : public QWidget, public Ui::PluginSettingsDecl {
public:
  PluginSettingsWidget( QWidget* parent = 0 ) : QWidget(parent) {
    setupUi(this);

    kcfg_timeUnitInSeconds->addItem(i18n("Minutes"));
    kcfg_timeUnitInSeconds->addItem(i18n("Hours"));
    kcfg_timeUnitInSeconds->addItem(i18n("Days"));

    kcfg_intervalBetweenRemindersTimeUnitInSeconds->addItem(0, i18n("Minutes"));
    kcfg_intervalBetweenRemindersTimeUnitInSeconds->addItem(i18n("Hours"));
    kcfg_intervalBetweenRemindersTimeUnitInSeconds->addItem(i18n("Days"));

    kcfg_beforeAfter->addItem(i18n("Before"));
    kcfg_beforeAfter->addItem(i18n("After"));
  }
};

K_PLUGIN_FACTORY(KCMiCalendarExportFactory, 
                 registerPlugin<KCMiCalendarExport>();
                )
K_EXPORT_PLUGIN(KCMiCalendarExportFactory("kmm_icalendarexport"));

KCMiCalendarExport::KCMiCalendarExport(QWidget *parent, const QVariantList& args) : KCModule(KCMiCalendarExportFactory::componentData(), parent, args)
{
  addConfig(PluginSettings::self(), new PluginSettingsWidget(parent));
  load();
}

KCMiCalendarExport::~KCMiCalendarExport()
{
}
