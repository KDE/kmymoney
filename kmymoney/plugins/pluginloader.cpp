/***************************************************************************
                          pluginloader.cpp
                             -------------------
    begin                : Thu Feb 12 2009
    copyright            : (C) 2009 Cristian Onet
    email                : onet.cristian@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "pluginloader.h"

#include <memory>

// ----------------------------------------------------------------------------
// QT Includes

#include <QStringList>
#include <QCheckBox>
#include <QLayout>
#include <QByteArray>
#include <QPluginLoader>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>
#include <KPluginLoader>
#include <KPluginSelector>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyplugin.h"

namespace KMyMoneyPlugin
{

//---------------------------------------------------------------------
//
// PluginLoader
//
//---------------------------------------------------------------------
static PluginLoader* s_instance = nullptr;

PluginLoader::PluginLoader(QObject* parent)
  : QObject(parent)
{
  Q_ASSERT(s_instance == nullptr);
  s_instance = this;

  categoryKMyMoneyPlugin = i18n("KMyMoney Plugins");
  categoryOnlineTask = i18n("Online Banking Operations");
  categoryPayeeIdentifier = i18n("Payee Identifier");

#if 0
  // Initialize the PluginSelector
  auto plugins = KPluginLoader::findPlugins("kmymoney");
  
  QList< KPluginInfo > pluginInfo;
  pluginInfo.reserve(plugins.count());
  auto end = plugins.cend();
  for (auto iter = plugins.cbegin(); iter != end; ++iter) {
    pluginInfo.append(KPluginInfo{*iter});
  }
  
  //m_pluginSelector->addPlugins(pluginInfo, KPluginSelector::PluginLoadMethod::ReadConfigFile, categoryByPluginType(pluginInfo));
#endif
  m_pluginSelector = new KPluginSelector();
  m_pluginSelector->load();

  connect(m_pluginSelector, &KPluginSelector::changed, this, &PluginLoader::changed);
}
 
PluginLoader::~PluginLoader()
{
}

QString PluginLoader::categoryByPluginType(const KPluginMetaData& mataData)
{
  if (!mataData.serviceTypes().contains("KMyMoney/Plugin")) {
    QJsonObject jsonKMyMoneyData = mataData.rawData()["KMyMoney"].toObject();
    if (!jsonKMyMoneyData["OnlineTask"].isNull())
      return categoryOnlineTask;
    else if (!jsonKMyMoneyData["PayeeIdentifier"].isNull())
      return categoryPayeeIdentifier;
  }
  return categoryKMyMoneyPlugin;
}

void PluginLoader::loadPlugins()
{
    const auto plugins = KPluginLoader::findPlugins("kmymoney");
    for(const KPluginMetaData& pluginData: plugins) {
        KPluginInfo info {pluginData};
        // Add plugin to selector, just in case it was not found on construction time already.
        // Duplicates should be detected by KPluginSelector.
        m_pluginSelector->addPlugins(QList<KPluginInfo> {info}, KPluginSelector::PluginLoadMethod::ReadConfigFile, categoryByPluginType(pluginData));

        // Only load plugins which are enabled and have the right serviceType. Other serviceTypes are loaded on demand.
        if (info.isPluginEnabled() && pluginData.serviceTypes().contains("KMyMoney/Plugin"))
          loadPlugin(pluginData);
    }
}

void PluginLoader::loadPlugin(const KPluginMetaData& metaData)
{
  std::unique_ptr<QPluginLoader> loader = std::unique_ptr<QPluginLoader>(new QPluginLoader{metaData.fileName()});
  QObject* plugin = loader->instance();
  if (!plugin) {
    qWarning("Could not load plugin '%s', error: %s", qPrintable(metaData.fileName()), qPrintable(loader->errorString()));
    return;
  }
  KMyMoneyPlugin::Plugin* kmmPlugin = qobject_cast<KMyMoneyPlugin::Plugin*>(plugin);
  if (!kmmPlugin) {
    qWarning("Could not load plugin '%s'.", qPrintable(metaData.fileName()));
    return;
  }

  emit plug(kmmPlugin);
}

void PluginLoader::changed()
{
    qWarning("Plugins cannot be unloaded currently.");
//  loadPlugins();
}

PluginLoader* PluginLoader::instance()
{
  Q_ASSERT(s_instance);
  return s_instance;
}

KPluginSelector* PluginLoader::pluginSelectorWidget()
{
  return m_pluginSelector;
}

} // namespace
