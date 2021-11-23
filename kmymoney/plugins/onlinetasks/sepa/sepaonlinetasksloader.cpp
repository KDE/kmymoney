/*
    SPDX-FileCopyrightText: 2013-2015 Christian Dávid <christian-david@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "sepaonlinetasksloader.h"

#include <KPluginFactory>

#include "mymoney/onlinejobadministration.h"
#include "onlinetasks/sepa/sepaonlinetransferimpl.h"
#include "ui/sepacredittransferedit.h"

K_PLUGIN_FACTORY_WITH_JSON(konlinetasks_sepa_factory, "kmymoney-sepaorders.json", registerPlugin<sepaOnlineTasksLoader>();
                           registerPlugin<sepaCreditTransferEdit>();)

sepaOnlineTasksLoader::sepaOnlineTasksLoader(QObject* parent, const QVariantList& options)
    : onlineTaskFactory(parent, options)
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
