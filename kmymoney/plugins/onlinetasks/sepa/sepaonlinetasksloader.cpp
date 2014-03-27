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

#include "sepaonlinetasksloader.h"
#include "mymoney/onlinejobadministration.h"

#include "tasks/sepaonlinetransferimpl.h"
#include "ui/sepacredittransferedit.h"

K_PLUGIN_FACTORY(SepaOnlineTaskFactory, registerPlugin<sepaOnlineTasksLoader>();)
K_EXPORT_PLUGIN(SepaOnlineTaskFactory("sepaOnlineTasksLoader"))

sepaOnlineTasksLoader::sepaOnlineTasksLoader(QObject* parent, const QVariantList&)
: KMyMoneyPlugin::Plugin::Plugin(parent, "sepaOnlineTasksLoader")
{
  onlineJobAdministration::instance()->registerOnlineTask( new sepaOnlineTransferImpl );
  onlineJobAdministration::instance()->registerOnlineTaskEdit( new sepaCreditTransferEdit );
}
