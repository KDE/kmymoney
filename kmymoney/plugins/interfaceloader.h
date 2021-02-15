/*
    SPDX-FileCopyrightText: 2016 Christian Dávid <christian-david@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef INTERFACELOADER_H
#define INTERFACELOADER_H

#include "kmm_plugin_export.h"

namespace KMyMoneyPlugin { class AppInterface; }
namespace KMyMoneyPlugin { class ImportInterface; }
namespace KMyMoneyPlugin { class StatementInterface; }
namespace KMyMoneyPlugin { class ViewInterface; }

class KMyMoneyApp;

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

  friend KMyMoneyApp;
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
