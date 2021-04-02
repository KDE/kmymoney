/*
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PLUGINLOADER_H
#define PLUGINLOADER_H

// ----------------------------------------------------------------------------
// QT Includes

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
struct Container;
enum class eListing;

/**
 * @brief The Action enum is for specifying action on plugins
 */
enum Action {
    Load,         // load all enabled plugins
    Unload,       // unload all loaded plugins
    Reorganize,   // load requested and unload unneeded plugins
};

/**
 * @brief The Category enum is some arbitrary categorization of plugins
 */
enum Category {
    OnlineBankOperations,
    PayeeIdentifier,
    StandardPlugin,
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
