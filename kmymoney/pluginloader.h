/***************************************************************************
                          pluginloader.h
                             -------------------
  (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef PLUGINLOADER_H
#define PLUGINLOADER_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QMap>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class KPluginMetaData;
class KXMLGUIFactory;

class QObject;
class QString;

template <class Key, class T> class QMap;

namespace KMyMoneyPlugin
{
  class Plugin;
  class OnlinePlugin;
  class OnlinePluginExtended;
  class ImporterPlugin;

  enum class eListing;

  /**
   * @brief The Action enum is for specifing action on plugins
   */
  enum Action {
    Load,         // load all enabled plugins
    Unload,       // unload all loaded plugins
    Reorganize    // load requested and unload unneeded plugins
  };

  /**
   * @brief The Category enum is some arbitrary categorization of plugins
   */
  enum Category {
    OnlineBankOperations,
    PayeeIdentifier,
    StandardPlugin
  };

  /**
   * @brief The Container struct to hold all plugin interfaces
   */
  struct Container {
    QMap<QString, Plugin*>               standard;  // this should contain all loaded plugins because every plugin should inherit Plugin class
    QMap<QString, OnlinePlugin*>         online;    // casted standard plugin, if such interface is available
    QMap<QString, OnlinePluginExtended*> extended;  // casted standard plugin, if such interface is available
    QMap<QString, ImporterPlugin*>       importer;  // casted standard plugin, if such interface is available
  };

  Category pluginCategory(const KPluginMetaData& pluginInfo);

  /**
   * @brief It lists all kmymoney plugins
   * @param onlyEnabled = true if plugins should be listed according to on/off saved state in kmymoneyrc
   * @return
   */
  QMap<QString, KPluginMetaData> listPlugins(bool onlyEnabled);

  /**
   * @brief It should be used to handle all plugin actions
   * @param action Action to be taken to all plugins
   * @param ctnPlugins Plugin container to be loaded/unloaded with plugins
   * @param parent Parent of plugins. This should be KMyMoneyApp
   * @param guiFactory GUI Factory of plugins. This should be GUI Factory of KMyMoneyApp
   */
  void pluginHandling(Action action, Container& ctnPlugins, QObject* parent, KXMLGUIFactory* guiFactory);
}

#endif
