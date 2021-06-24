/*

    SPDX-FileCopyrightText: 2014 Christian Dávid <christian-david@web.de>
    SPDX-FileCopyrightText: 2021 Dawid Wróbel <me@dawidwrobel.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "onlinepluginextended.h"

namespace KMyMoneyPlugin {

#if KCOREADDONS_VERSION < QT_VERSION_CHECK(5, 77, 0)
OnlinePluginExtended::OnlinePluginExtended(QObject* parent, const QVariantList& args)
    : Plugin(parent, args)
    ,
#else
OnlinePluginExtended::OnlinePluginExtended(QObject* parent, const KPluginMetaData& metaData, const QVariantList& args)
    : Plugin(parent, metaData, args)
    ,
#endif
    OnlinePlugin()
{
}

}
