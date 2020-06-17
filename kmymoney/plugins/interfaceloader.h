/*
 * This file is part of KMyMoney, A Personal Finance Manager by KDE
 * Copyright (C) 2016 Christian Dávid <christian-david@web.de>
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

#ifndef INTERFACELOADER_H
#define INTERFACELOADER_H

#include "kmm_plugin_export.h"

namespace KMyMoneyPlugin { class AppInterface; }
namespace KMyMoneyPlugin { class ImportInterface; }
namespace KMyMoneyPlugin { class StatementInterface; }
namespace KMyMoneyPlugin { class ViewInterface; }

class KMyMoney;

namespace KMyMoneyPlugin
{

  class Plugin;

/**
 * @internal
 *
 * This class is used as dead drop to communicate between two compile targets which cannot do
 * this directly.
 * It is only used by the classes which are named friends. To receive an instance of
 * this class @ref pluginInterfaces() is used.
 */
class InterfaceLoader {
  /**
   * @{
   * This class is owner of these objects. However, the parent is somebody else. They are deleted by destruction of the parent only.
   */
  KMyMoneyPlugin::AppInterface* appInterface;
  KMyMoneyPlugin::ViewInterface* viewInterface;
  KMyMoneyPlugin::StatementInterface* statementInterface;
  KMyMoneyPlugin::ImportInterface* importInterface;
  /** @} */

  friend KMyMoney;
  friend KMyMoneyPlugin::Plugin;
};

/**
 * @internal
 *
 * Returns an instance of @ref InterfaceLoader. It is created if needed.
 */
KMM_PLUGIN_EXPORT InterfaceLoader& pluginInterfaces();

}

#endif
