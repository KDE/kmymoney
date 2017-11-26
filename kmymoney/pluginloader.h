/***************************************************************************
                          pluginloader.h
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

#ifndef PLUGINLOADER_H
#define PLUGINLOADER_H

#include <QObject>
#include <QSet>
#include <KPluginMetaData>

class KPluginSelector;
class KConfigGroup;

namespace KMyMoneyPlugin
{
class Plugin;

/**
 * @brief User Interface for plugins
 *
 * For historic reasons it is still called plugin loader even though it does not load any plugin anymore.
 */
class PluginLoader : public QObject
{
  Q_OBJECT
public:
  explicit PluginLoader(QObject* parent);


  /**
   * Needed to delete the unique_ptr which is of incomplete type in the header file
   */
  virtual ~PluginLoader();
  static PluginLoader* instance();

  KPluginSelector* pluginSelectorWidget();
  static inline bool isPluginEnabled(const KPluginMetaData& metaData, const KConfigGroup& configGroup);

public Q_SLOTS:
  /** @brief Adds the given plugins to the plugin selection ui */
  void addPluginInfo(const QVector<KPluginMetaData>& metaData);

  /** @brief Find all available plugins */
  void detectPlugins();

Q_SIGNALS:
  /** Load the shared module and emits plug() */
  void pluginEnabled(const KPluginMetaData& metaData);
  void pluginDisabled(const KPluginMetaData& metaData);
  void configChanged(Plugin*);  // configuration of the plugin has changed not the enabled/disabled state

private Q_SLOTS:
  void changed();

private:
  KPluginSelector* m_pluginSelector;

  /**
   * @{
   * Translated strings
   *
   * They are created on creation time because they are used as identifiers.
   */
  QString m_categoryKMyMoneyPlugin;
  QString m_categoryOnlineTask;
  QString m_categoryPayeeIdentifier;
  /** @} */

  /**
   * @brief All plugins which are listed in the UI
   *
   * The set contains the plugin file name.
   * Maybe it can/should be replaced by something different.
   */
  QSet<QString> m_displayedPlugins;

  QString categoryByPluginType(const KPluginMetaData& mataData);
};
}

#endif /* PLUGINLOADER_H */
