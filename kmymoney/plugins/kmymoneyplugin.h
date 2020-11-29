/*
 * This file is part of KMyMoney, A Personal Finance Manager by KDE
 * Copyright (C) 2005 Thomas Baumgart <ipwizard@users.sourceforge.net>
 * Copyright (C) 2015 Christian Dávid <christian-david@web.de>
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

#ifndef KMYMONEYPLUGIN_H
#define KMYMONEYPLUGIN_H

#include <kmm_plugin_export.h>

// ----------------------------------------------------------------------------
// QT Includes

#include <QMap>
#include <QObject>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KXMLGUIClient>
class KToggleAction;

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneykeyvaluecontainer.h"

class MyMoneyStorageMgr;
class MyMoneyAccount;
class KMyMoneySettings;
class IMyMoneyOperationsFormat;

namespace KMyMoneyPlugin { class AppInterface; }
namespace KMyMoneyPlugin { class ImportInterface; }
namespace KMyMoneyPlugin { class StatementInterface; }
namespace KMyMoneyPlugin { class ViewInterface; }

namespace eKMyMoney { enum class StorageType; }

/**
 * @defgroup KMyMoneyPlugin
 *
 * KMyMoney knows several types of plugins. The most common and generic one is KMyMoneyPlugin::Plugin.
 *
 * Another group of plugins are just loaded on demand and offer special functions with a tight integration into KMyMoney. Whenever possible you should use this kind of plugins.
 * At the moment this are the onlineTask and payeeIdentifierData.
 *
 * @{
 */

namespace KMyMoneyPlugin
{

/**
 * This class describes the interface between KMyMoney and it's plugins.
 *
 * The plugins are based on Qt 5's plugin system. So you must compile json information into the plugin.
 * KMyMoney looks into the folder "${PLUGIN_INSTALL_DIR}/kmymoney/" and loads all plugins found there (if the user did not deactivate the plugin).
 *
 * The json header of the plugin must comply with the requirements of KCoreAddon's KPluginMetaData class.
 * To load the plugin at start up the service type "KMyMoney/Plugin" must be set.
 *
 * @warning The plugin system for KMyMoney 5 is still in development. Especially the loading of the on-demand plugins (mainly undocumented :( ) will change.
 *
 * A basic json header is shown below.
 * @code{.json}
   {
    "KPlugin": {
        "Authors": [
            {
                "Name": "Author's Names, Second Author",
                "Email": "E-Mail 1, E-Mail 2"
            }
        ],
        "Description": "Short description for plugin list (translateable)",
        "EnabledByDefault": true,
        "Icon": "icon to be shown in plugin list",
        "Id": "a unique identifier",
        "License": "see KPluginMetaData for list of predefined licenses (translateable)",
        "Name": "Name of the plugin (translateable)",
        "ServiceTypes": [
            "KMyMoney/Plugin"
        ],
        "Version": "@PROJECT_VERSION@@PROJECT_VERSION_SUFFIX@",
     }
   }
 * @endcode
 *
 * This example assumes you are using
 * @code{.cmake}
   configure_file(${CMAKE_CURRENT_SOURCE_DIR}/... ${CMAKE_CURRENT_BINARY_DIR}/... @ONLY)
   @endcode
 * to replace the version variables using cmake.
 *
 * @see https://doc.qt.io/qt-5/plugins-howto.html
 * @see https://api.kde.org/frameworks/kcoreaddons/html/classKPluginMetaData.html
 *
 */
class KMM_PLUGIN_EXPORT Plugin : public QObject, public KXMLGUIClient
{
  Q_OBJECT
public:
  explicit Plugin(QObject* parent = nullptr, const char* name = "");
  virtual ~Plugin();

public Q_SLOTS:
  /**
   * @brief Called during plug in process
   */
  virtual void plug(KXMLGUIFactory* guiFactory);

  /**
   * @brief Called before unloading
   */
  virtual void unplug();

  /**
   * @brief Called if the configuration of the plugin was changed
   * @todo Implement
   */
  virtual void configurationChanged() ;

protected:
  /** See KMyMoneyApp::toggleAction() for a description */
  KToggleAction* toggleAction(const QString& name) const;

  // define interface classes here. The interface classes provide a mechanism
  // for the plugin to interact with KMyMoney
  // they are defined in the following form for an interface
  // named Xxx:
  //
  // XxxInterface* xxxInterface();

  AppInterface* appInterface() const;
  ViewInterface* viewInterface() const;
  StatementInterface* statementInterface() const;
  ImportInterface* importInterface() const;
};

/**
   * This class describes the interface between the KMyMoney
   * application and it's ONLINE-BANKING plugins. All online banking plugins
   * must provide this interface.
   *
   * A good tutorial on how to design and develop a plugin
   * structure for a KDE application (e.g. KMyMoney) can be found at
   * http://web.archive.org/web/20100305214125/http://developer.kde.org/documentation/tutorials/developing-a-plugin-structure/index.html
   *
 */
class KMM_PLUGIN_EXPORT OnlinePlugin
{
public:
  OnlinePlugin();
  virtual ~OnlinePlugin();

  virtual void protocols(QStringList& protocolList) const = 0;

  /**
   * This method returns a pointer to a widget representing an additional
   * tab that will be added to the KNewAccountDlg. The string referenced
   * with @a tabName will be filled with the text that should be placed
   * on the tab. It should return 0 if no additional tab is needed.
   *
   * Information about the account can be taken out of @a account.
   *
   * Once the pointer to the widget is returned to KMyMoney, it takes care
   * of destruction of all included widgets when the dialog is closed. The plugin
   * can access the widgets created after the call to storeConfigParameters()
   * happened.
   */
  virtual QWidget* accountConfigTab(const MyMoneyAccount& account, QString& tabName) = 0;

  /**
   * This method is called by the framework whenever it is time to store
   * the configuration data maintained by the plugin. The plugin should use
   * the widgets created in accountConfigTab() to extract the current values.
   *
   * @param current The @a current container contains the current settings
   */
  virtual MyMoneyKeyValueContainer onlineBankingSettings(const MyMoneyKeyValueContainer& current) = 0;

  /**
   * This method is called by the framework when the user wants to map
   * a KMyMoney account onto an online account. The KMyMoney account is identified
   * by @a acc and the online provider should store its data in @a onlineBankingSettings
   * upon success.
   *
   * @retval true if account is mapped
   * @retval false if account is not mapped
   */
  virtual bool mapAccount(const MyMoneyAccount& acc, MyMoneyKeyValueContainer& onlineBankingSettings) = 0;

  /**
   * This method is called by the framework when the user wants to update
   * a KMyMoney account with data from an online account. The KMyMoney account is identified
   * by @a acc. The online provider should read its data from acc.onlineBankingSettings().
   * @a true is returned upon success. The plugin might consider to stack the requests
   * in case @a moreAccounts is @p true. @a moreAccounts defaults to @p false.
   *
   * @retval true if account is updated
   * @retval false if account is not updated
   */
  virtual bool updateAccount(const MyMoneyAccount& acc, bool moreAccounts = false) = 0;
};

/**
  * This class describes the interface between the KMyMoney
  * application and it's IMPORTER plugins. All importer plugins
  * must provide this interface.
  *
  * A good tutorial on how to design and develop a plugin
  * structure for a KDE application (e.g. KMyMoney) can be found at
  * http://web.archive.org/web/20100305214125/http://developer.kde.org/documentation/tutorials/developing-a-plugin-structure/index.html
  *
  */
class KMM_PLUGIN_EXPORT ImporterPlugin
{
public:
  ImporterPlugin();
  virtual ~ImporterPlugin();

  /**
    * This method returns the english-language name of the format
    * this plugin imports, e.g. "OFX"
    *
    * @return QString Name of the format
    */
  virtual QString formatName() const = 0;

  /**
    * This method returns the filename filter suitable for passing to
    * KFileDialog::setFilter(), e.g. "*.ofx *.qfx" which describes how
    * files of this format are likely to be named in the file system
    *
    * @return QString Filename filter string
    */
  virtual QString formatFilenameFilter() const = 0;

  /**
    * This method returns whether this plugin is able to import
    * a particular file.
    *
    * @param filename Fully-qualified pathname to a file
    *
    * @return bool Whether the indicated file is importable by this plugin
    */
  virtual bool isMyFormat(const QString& filename) const = 0;

  /**
    * Import a file
    *
    * @param filename File to import
    *
    * @return bool Whether the import was successful.
    */
  virtual bool import(const QString& filename) = 0;

  /**
    * Returns the error result of the last import
    *
    * @return QString English-language name of the error encountered in the
    *  last import, or QString() if it was successful.
    *
    */
  virtual QString lastError() const = 0;

};

/**
  * This class describes the interface between the KMyMoney
  * application and it's STORAGE plugins. All storage plugins
  * must provide this interface.
  *
  */
class KMM_PLUGIN_EXPORT StoragePlugin
{
public:
  StoragePlugin() = default;
  virtual ~StoragePlugin() = default;

  /**
   * @brief Loads file into storage
   * @param url URL of the file
   * @return true if successfully opened
   */
  virtual bool open(const QUrl &url) = 0;

  /**
   * @brief Saves storage into file
   * @param url URL of the file
   * @return true if successfully saved
   */
  virtual bool save(const QUrl &url) = 0;

  /**
   * @brief Saves storage into file
   * @param url URL of the file
   * @return true if successfully saved
   */
  virtual bool saveAs() = 0;

  /**
   * @brief Storage identifier
   * @return Storage identifier
   */
  virtual eKMyMoney::StorageType storageType() const = 0;

  virtual QString fileExtension() const = 0;

  /**
   * @brief returns the full URL used to open the database (incl. password)
   * @return QUrl to re-open the database
   */
  virtual QUrl openUrl() const = 0;
};

/**
  * This class describes the interface between the KMyMoney
  * application and its data plugins. All data plugins
  * must provide this interface.
  *
  */
class KMM_PLUGIN_EXPORT DataPlugin
{
public:
  DataPlugin() = default;
  virtual ~DataPlugin() = default;

  /**
   * @brief Gets data from data service
   * @param arg Item name to retrieve data for
   * @param type Data type to retrieve for item
   * @return a data like int or QString
   */
  virtual QVariant requestData(const QString &arg, uint type) = 0;
};

class OnlinePluginExtended;

/**
 * @brief The Container struct to hold all plugin interfaces
 */
struct Container {
  QMap<QString, Plugin*>               standard;  // this should contain all loaded plugins because every plugin should inherit Plugin class
  QMap<QString, OnlinePlugin*>         online;    // casted standard plugin, if such interface is available
  QMap<QString, OnlinePluginExtended*> extended;  // casted standard plugin, if such interface is available
  QMap<QString, ImporterPlugin*>       importer;  // casted standard plugin, if such interface is available
  QMap<QString, StoragePlugin*>        storage;   // casted standard plugin, if such interface is available
  QMap<QString, DataPlugin*>           data;      // casted standard plugin, if such interface is available
};

} // end of namespace

/**
 * @brief Structure of plugins objects by their interfaces
 */
KMM_PLUGIN_EXPORT extern KMyMoneyPlugin::Container pPlugins;

Q_DECLARE_INTERFACE(KMyMoneyPlugin::OnlinePlugin, "org.kmymoney.plugin.onlineplugin")
Q_DECLARE_INTERFACE(KMyMoneyPlugin::ImporterPlugin, "org.kmymoney.plugin.importerplugin")
Q_DECLARE_INTERFACE(KMyMoneyPlugin::StoragePlugin, "org.kmymoney.plugin.storageplugin")
Q_DECLARE_INTERFACE(KMyMoneyPlugin::DataPlugin, "org.kmymoney.plugin.dataplugin")


/** @} */

#endif
