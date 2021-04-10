/*

    SPDX-FileCopyrightText: 2014 Christian Dávid <christian-david@web.de>
    SPDX-FileCopyrightText: 2021 Dawid Wróbel <me@dawidwrobel.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "onlinepluginextended.h"

namespace KMyMoneyPlugin
{

OnlinePluginExtended::OnlinePluginExtended(QObject* parent, const QVariantList& args, const char* name)
    : Plugin(parent, args, name),
      OnlinePlugin()
{}

}
