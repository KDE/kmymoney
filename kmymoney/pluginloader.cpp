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

// ----------------------------------------------------------------------------
// QT Includes

#include <QStringList>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>
#include <KPluginInfo>
#include <KPluginLoader>
#include <KPluginSelector>
#include <KSharedConfig>

// ----------------------------------------------------------------------------
// Project Includes

namespace KMyMoneyPlugin
{

//---------------------------------------------------------------------
//
// PluginLoader
//
//---------------------------------------------------------------------
static PluginLoader* s_instance = nullptr;

PluginLoader::PluginLoader(QObject* parent)
  : QObject(parent),
  m_displayedPlugins{}
{
  Q_ASSERT(s_instance == nullptr);
  s_instance = this;

  m_categoryKMyMoneyPlugin = i18n("KMyMoney Plugins");
  m_categoryOnlineTask = i18n("Online Banking Operations");
  m_categoryPayeeIdentifier = i18n("Payee Identifier");

  m_pluginSelector = new KPluginSelector();
  connect(m_pluginSelector, &KPluginSelector::configCommitted, this, &PluginLoader::changed);
}

PluginLoader::~PluginLoader()
{
  Q_ASSERT(s_instance != nullptr);

  KSharedConfigPtr config = KSharedConfig::openConfig();
  KConfigGroup group{ config->group("Plugins") };

  for (const QString& fileName: m_displayedPlugins) {
    KPluginMetaData pluginData{fileName};
    pluginDisabled(pluginData);
  }

  delete m_pluginSelector;
  s_instance = nullptr;
}

void PluginLoader::detectPlugins()
{
    addPluginInfo(KPluginLoader::findPlugins("kmymoney"));
}

inline bool PluginLoader::isPluginEnabled(const KPluginMetaData& metaData, const KConfigGroup& configGroup)
{
  //! @fixme: there is a function in KMyMoneyApp which has to have the same content
  if (metaData.serviceTypes().contains("KMyMoney/Plugin")) {
    const QString keyName{metaData.name() + "Enabled"};
    if (configGroup.hasKey(keyName))
      return configGroup.readEntry(keyName, true);
    return metaData.isEnabledByDefault();
  }
  return false;
}

void PluginLoader::addPluginInfo(const QVector<KPluginMetaData>& metaData)
{
  m_displayedPlugins.reserve(metaData.size());
  for(const auto& plugin: metaData) {
    m_displayedPlugins.insert(plugin.fileName());
    KPluginInfo info {plugin};
    KPluginSelector::PluginLoadMethod loadMethod = (plugin.serviceTypes().contains("KMyMoney/Plugin"))
      ? KPluginSelector::PluginLoadMethod::ReadConfigFile
      : KPluginSelector::PluginLoadMethod::IgnoreConfigFile;
    m_pluginSelector->addPlugins(QList<KPluginInfo> {info}, loadMethod, categoryByPluginType(plugin));
    //! @fixme The not all plugins are deactivated correctly at the moment. This has to change or the user should not get any option to enable and disable them
  }
}

QString PluginLoader::categoryByPluginType(const KPluginMetaData& mataData)
{
  if (!mataData.serviceTypes().contains("KMyMoney/Plugin")) {
    QJsonObject jsonKMyMoneyData = mataData.rawData()["KMyMoney"].toObject();
    if (!jsonKMyMoneyData["OnlineTask"].isNull())
      return m_categoryOnlineTask;
    else if (!jsonKMyMoneyData["PayeeIdentifier"].isNull())
      return m_categoryPayeeIdentifier;
  }
  return m_categoryKMyMoneyPlugin;
}

void PluginLoader::changed()
{
  KSharedConfigPtr config = KSharedConfig::openConfig();
  KConfigGroup group{ config->group("Plugins") };

  for (const QString& fileName: m_displayedPlugins) {
    KPluginMetaData pluginData{fileName};
    if (isPluginEnabled(pluginData, group)) {
      pluginEnabled(pluginData);
    } else {
      pluginDisabled(pluginData);
    }
  }
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
