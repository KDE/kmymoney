/*
 * This file is part of KMyMoney, A Personal Finance Manager by KDE
 * Copyright (C) 2013-2015 Christian Dávid <christian-david@web.de>
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

#include "sepaonlinetasksloader.h"

#include <KPluginFactory>

#include "mymoney/onlinejobadministration.h"
#include "onlinetasks/sepa/sepaonlinetransferimpl.h"
#include "ui/sepacredittransferedit.h"

K_PLUGIN_FACTORY_WITH_JSON(sepaOnlineTasksFactory,
                           "kmymoney-sepaorders.json",
                           registerPlugin<sepaOnlineTasksLoader>("sepaOnlineTasks");
                           registerPlugin<sepaCreditTransferEdit>("sepaCreditTransferUi");
                          )

sepaOnlineTasksLoader::sepaOnlineTasksLoader(QObject* parent, const QVariantList& /*options*/)
  : QObject(parent),
  onlineTaskFactory()
{
}

onlineTask* sepaOnlineTasksLoader::createOnlineTask(const QString& taskId) const
{
  if (taskId == sepaOnlineTransferImpl::name())
    return new sepaOnlineTransferImpl;
  
  return nullptr;
}

// Needed for K_PLUGIN_FACTORY
#include "sepaonlinetasksloader.moc"
