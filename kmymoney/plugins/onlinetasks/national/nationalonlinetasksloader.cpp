/*
 * This file is part of KMyMoney, A Personal Finance Manager for KDE
 * Copyright (C) 2013 Christian Dávid <christian-david@web.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <KPluginFactory>

#include "nationalonlinetasksloader.h"

#include "nationalstorageplugin.h"
#include "mymoney/onlinejobadministration.h"

#include "tasks/germanonlinetransferimpl.h"
#include "ui/germancredittransferedit.h"
#include "converter/taskconvertergermantosepa.h"
#include "converter/taskconvertersepatogerman.h"

K_PLUGIN_FACTORY(NationalOnlineTaskFactory,
                 registerPlugin<nationalOnlineTasksLoader>();
                 registerPlugin<germanCreditTransferEdit>("creditTransferUi");
                 registerPlugin<nationalStoragePlugin>("storagePlugin");
                )
K_EXPORT_PLUGIN(NationalOnlineTaskFactory("nationalOnlineTasksLoader"))

nationalOnlineTasksLoader::nationalOnlineTasksLoader(QObject* parent, const QVariantList&)
: KMyMoneyPlugin::Plugin::Plugin(parent, "nationalOnlineTasksLoader")
{
  onlineJobAdministration::instance()->registerOnlineTask( new germanOnlineTransferImpl );
  onlineJobAdministration::instance()->registerOnlineTaskConverter( new taskConverterSepaToGerman );
  onlineJobAdministration::instance()->registerOnlineTaskConverter( new taskConverterGermanToSepa);
}
