/*
    SPDX-FileCopyrightText: 2016 Christian Dávid <christian-david@web.de>
    SPDX-License-Identifier: GPL-2.0-or-laterrg/licenses/>.
*/

#include "interfaceloader.h"

namespace KMyMoneyPlugin {

InterfaceLoader& pluginInterfaces()
{
  static InterfaceLoader m_interfaces;
  return m_interfaces;
}

}
