/***************************************************************************
                             ksettingsplugins.cpp
                             --------------------
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

#include "ksettingsplugins.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QVBoxLayout>
#include <QBitArray>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>
#include <KPluginSelector>
#include <KPluginInfo>
#include <KPluginMetaData>

// ----------------------------------------------------------------------------
// Project Includes

#include "pluginloader.h"

struct pluginGroupInfo {
  QList<KPluginInfo>                plugins;
  KPluginSelector::PluginLoadMethod loadMethod;
  QString                           categoryName;
};

class KSettingsPluginsPrivate
{
  Q_DISABLE_COPY(KSettingsPluginsPrivate)

public:
  KSettingsPluginsPrivate(KSettingsPlugins* qq) :
    m_pluginSelector(new KPluginSelector(qq))
  {
  }

  ~KSettingsPluginsPrivate()
  {
    delete m_pluginSelector;
  }

  /**
   * @brief This should be called after save to kmymoneyrc in order to update cached on/off states
   */
  void updateSavedPluginStates()
  {
    for (auto i = 0 ; i < pluginInfos.size(); ++i)
      savedPluginStates[i] = pluginInfos[i].isPluginEnabled();
  }

  /**
   * @brief This compares plugin on/off states from KPluginSelector with cached one
   * @return true if user changes to plugin on/off state aren't different than initial
   */
  bool isEqualToSavedStates()
  {
    for (auto i = 0 ; i < pluginInfos.size(); ++i)
      if (savedPluginStates[i] != pluginInfos[i].isPluginEnabled())
        return false;
    return true;
  }

  KPluginSelector* const m_pluginSelector;
  QList<KPluginInfo> pluginInfos;
  /**
   * @brief savedPluginStates This caches on/off states as in kmymoneyrc
   */
  QBitArray savedPluginStates;
};

KSettingsPlugins::KSettingsPlugins(QWidget* parent) :
  QWidget(parent),
  d_ptr(new KSettingsPluginsPrivate(this))

{
  Q_D(KSettingsPlugins);
  auto layout = new QVBoxLayout;
  setLayout(layout);  // otherwise KPluginSelector occupies very little area
  layout->addWidget(d->m_pluginSelector);

  auto allPluginDatas = KMyMoneyPlugin::listPlugins(false); // fetch all available KMyMoney plugins
  QVector<KPluginMetaData> standardPlugins;
  QVector<KPluginMetaData> payeePlugins;
  QVector<KPluginMetaData> onlinePlugins;

  // divide plugins in some arbitrary categories
  for (const KPluginMetaData& pluginData : allPluginDatas)
    switch (KMyMoneyPlugin::pluginCategory(pluginData)) {
      case KMyMoneyPlugin::Category::StandardPlugin:
        standardPlugins.append(pluginData);
        break;
      case KMyMoneyPlugin::Category::PayeeIdentifier:
        payeePlugins.append(pluginData);
        break;
      case KMyMoneyPlugin::Category::OnlineBankOperations:
        onlinePlugins.append(pluginData);
        break;
      default:
        break;
    }

  const QVector<pluginGroupInfo> pluginGroups {
    {KPluginInfo::fromMetaData(standardPlugins),
          KPluginSelector::PluginLoadMethod::ReadConfigFile,
          i18n("KMyMoney Plugins")},

    {KPluginInfo::fromMetaData(payeePlugins),
          KPluginSelector::PluginLoadMethod::IgnoreConfigFile,
          i18n("Payee Identifier")},

    {KPluginInfo::fromMetaData(onlinePlugins),
          KPluginSelector::PluginLoadMethod::IgnoreConfigFile,
          i18n("Online Banking Operations")}
  };

  // add all plugins to selector
  for(const auto& pluginGroup : pluginGroups) {
    if (!pluginGroup.plugins.isEmpty()) {
      d->m_pluginSelector->addPlugins(pluginGroup.plugins,
                                      pluginGroup.loadMethod,
                                      pluginGroup.categoryName); // at that step plugin on/off state should be fetched automatically by KPluginSelector
      d->pluginInfos.append(pluginGroup.plugins);                // store initial on/off state to be able to enable/disable Apply button
    }
  }
  d->savedPluginStates.resize(d->pluginInfos.size());
  d->updateSavedPluginStates();

  connect(d->m_pluginSelector, &KPluginSelector::changed, this, &KSettingsPlugins::slotPluginsSelectionChanged);
}

KSettingsPlugins::~KSettingsPlugins()
{
  Q_D(KSettingsPlugins);
  delete d;
}

void KSettingsPlugins::slotPluginsSelectionChanged(bool b)
{
  Q_D(KSettingsPlugins);
  if (b) {
    d->m_pluginSelector->updatePluginsState();
    emit changed(!d->isEqualToSavedStates());
  }
}

void KSettingsPlugins::slotResetToDefaults()
{
  Q_D(KSettingsPlugins);
  d->m_pluginSelector->defaults();
}

void KSettingsPlugins::slotSavePluginConfiguration()
{
  Q_D(KSettingsPlugins);
  if (!d->isEqualToSavedStates()) {
    d->m_pluginSelector->save();
    d->updateSavedPluginStates();
    emit settingsChanged(QStringLiteral("Plugins"));
  }
}
