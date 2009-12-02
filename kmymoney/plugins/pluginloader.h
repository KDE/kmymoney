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

#ifdef HAVE_CONFIG_H
#include <config-kmymoney.h>
#endif

// ----------------------------------------------------------------------------
// QT Includes

#include <QObject>
#include <QByteArray>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include <kmm_plugin_export.h>

class KPluginSelector;
class KPluginInfo;

namespace KMyMoneyPlugin
{
  class Plugin;

  class KMM_PLUGINS_EXPORT PluginLoader : public QObject
  {
    Q_OBJECT
  public:
    PluginLoader(QObject* parent);
    virtual ~PluginLoader();
    static PluginLoader* instance();

    void loadPlugins();
    Plugin* getPluginFromInfo(KPluginInfo*);
    KPluginSelector* pluginSelectorWidget();

  private:
    void loadPlugin(KPluginInfo*);

  signals:
    void plug(KPluginInfo*);
    void unplug(KPluginInfo*);
    void configChanged(Plugin*);  // consfiguration of the plugin has changed not the enabled/disabled state

  private slots:
    void changed();
    void changedConfigOfPlugin( const QByteArray & );

  private:
    struct Private;
    Private* d;
  };
}

#endif /* PLUGINLOADER_H */
