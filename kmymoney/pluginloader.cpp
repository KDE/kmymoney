/*
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "pluginloader.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QMap>
#include <QDebug>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KPluginLoader>
#include <KPluginFactory>
#include <KPluginInfo>
#include <KPluginMetaData>
#include <KXMLGUIFactory>
#include <KSharedConfig>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyplugin.h"
#include "onlinepluginextended.h"

namespace KMyMoneyPlugin
{
  Category pluginCategory(const KPluginMetaData& pluginInfo)
  {
    if (!pluginInfo.serviceTypes().contains(QStringLiteral("KMyMoney/Plugin"))) {
      auto jsonKMyMoneyData = pluginInfo.rawData()[QLatin1String("KMyMoney")].toObject();
      if (!jsonKMyMoneyData[QLatin1String("OnlineTask")].isNull())
        return OnlineBankOperations;
      else if (!jsonKMyMoneyData[QLatin1String("PayeeIdentifier")].isNull())
        return PayeeIdentifier;
    }
    return StandardPlugin;
  }

  bool isPluginEnabled(const KPluginMetaData& pluginData, const KConfigGroup& pluginSection)
  {
    return pluginSection.readEntry(QString::fromLatin1("%1Enabled").    // we search here for e.g. "csvimporterEnabled = true"
                                   arg(pluginData.pluginId()),
                                   pluginData.isEnabledByDefault());    // if not found, then get default from plugin's json file
  }

  QMap<QString, KPluginMetaData> listPlugins(bool onlyEnabled)
  {
    QMap<QString, KPluginMetaData> plugins;
    const auto pluginDatas = KPluginLoader::findPlugins(QStringLiteral("kmymoney"));          // that means search for plugins in "/lib64/plugins/kmymoney/"
    const auto pluginSection(KSharedConfig::openConfig()->group(QStringLiteral("Plugins")));  // section of config where plugin on/off were saved

    for (const KPluginMetaData& pluginData : pluginDatas) {
      if (pluginData.serviceTypes().contains(QStringLiteral("KMyMoney/Plugin"))) {
        if (!onlyEnabled || isPluginEnabled(pluginData, pluginSection)) {
          // only use the first one found. Otherwise, always the last one
          // wins (usually the installed system version) and the QT_PLUGIN_PATH
          // env variable nor the current directory have an effect for KMyMoney
          if (!plugins.contains(pluginData.pluginId()))
            plugins.insert(pluginData.pluginId(), pluginData);
        }
      }
    }
    return plugins;
  }

  void pluginHandling(Action action, Container& ctnPlugins, QObject* parent, KXMLGUIFactory* guiFactory)
  {

    if (action == Action::Load ||
        action == Action::Reorganize) {
      KPluginLoader::forEachPlugin(QStringLiteral("kmymoney"), [&](const QString &pluginPath) {
          KPluginMetaData metadata(pluginPath);
          qDebug() << "Located plugin" << pluginPath << "Validity" << metadata.isValid();
      });
    }

    QMap<QString, KPluginMetaData> referencePluginDatas;
    if (action == Action::Load ||
        action == Action::Reorganize)
      referencePluginDatas = listPlugins(true);

    if (action == Action::Unload ||
        action == Action::Reorganize) {
      auto& plugins = ctnPlugins.standard;
      auto& refPlugins = referencePluginDatas;
      for (auto it = plugins.begin(); it != plugins.end();) {
        if (!refPlugins.contains(it.key())) {

          ctnPlugins.online.remove(it.key());
          ctnPlugins.extended.remove(it.key());
          ctnPlugins.importer.remove(it.key());
          ctnPlugins.storage.remove(it.key());
          ctnPlugins.data.remove(it.key());

          guiFactory->removeClient(it.value());
          it.value()->unplug();
          delete it.value();
          it = plugins.erase(it);
          continue;

        }
        ++it;
      }
    }

    if (action == Action::Load ||
        action == Action::Reorganize) {

      auto& refPlugins = referencePluginDatas;
      for (auto it = refPlugins.cbegin(); it != refPlugins.cend(); ++it) {
        if (!ctnPlugins.standard.contains(it.key())) {
          qDebug() << "Loading" << (*it).fileName();
          KPluginLoader loader((*it).fileName());
          auto factory = loader.factory();
          if (!factory) {
            qWarning("Could not load plugin '%s', error: %s", qPrintable((*it).fileName()), qPrintable(loader.errorString()));
            loader.unload();
            continue;
          }

          Plugin* plugin = factory->create<Plugin>(parent);
          if (!plugin) {
            qWarning("This is not KMyMoney plugin: '%s'", qPrintable((*it).fileName()));
            loader.unload();
            continue;
          }

          ctnPlugins.standard.insert((*it).pluginId(), plugin);
          plugin->plug();
          guiFactory->addClient(plugin);

          auto IOnline = qobject_cast<OnlinePlugin *>(plugin);
          if (IOnline)
            ctnPlugins.online.insert((*it).pluginId(), IOnline);

          auto IExtended = qobject_cast<OnlinePluginExtended *>(plugin);
          if (IExtended )
            ctnPlugins.extended.insert((*it).pluginId(), IExtended );

          auto IImporter = qobject_cast<ImporterPlugin *>(plugin);
          if (IImporter)
            ctnPlugins.importer.insert((*it).pluginId(), IImporter);

          auto IStorage = qobject_cast<StoragePlugin *>(plugin);
          if (IStorage)
            ctnPlugins.storage.insert((*it).pluginId(), IStorage);

          auto IData = qobject_cast<DataPlugin *>(plugin);
          if (IData)
            ctnPlugins.data.insert((*it).pluginId(), IData);

        }
      }

    }
  }
}
