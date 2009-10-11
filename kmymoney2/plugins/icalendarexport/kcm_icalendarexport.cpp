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
K_EXPORT_PLUGIN(KCMiCalendarExportFactory("kmm_icalendarexport"))

KCMiCalendarExport::KCMiCalendarExport(QWidget *parent, const QVariantList& args) : KCModule(KCMiCalendarExportFactory::componentData(), parent, args)
{
  addConfig(PluginSettings::self(), new PluginSettingsWidget(this));
  load();
}

KCMiCalendarExport::~KCMiCalendarExport()
{
}
