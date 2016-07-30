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
#include <QByteArray>
#include <KPluginMetaData>
#include <KPluginInfo>

#include "kmm_plugin_export.h"

class KPluginSelector;
class KPluginInfo;

namespace KMyMoneyPlugin
{
class Plugin;

class KMM_PLUGIN_EXPORT PluginLoader : public QObject
{
  Q_OBJECT
public:
  PluginLoader(QObject* parent);


  /**
   * Needed to delete the unique_ptr which is of incomplete type in the header file
   */
  virtual ~PluginLoader();
  static PluginLoader* instance();

  void loadPlugins();
  KPluginSelector* pluginSelectorWidget();

private:
  void loadPlugin(const KPluginMetaData& metaData);

signals:
  void plug(KMyMoneyPlugin::Plugin*);
  void unplug(KPluginInfo*);
  void configChanged(Plugin*);  // consfiguration of the plugin has changed not the enabled/disabled state

private slots:
  void changed();

private:
  KPluginSelector*  m_pluginSelector;
};
}

#endif /* PLUGINLOADER_H */
