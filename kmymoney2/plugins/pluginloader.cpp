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

// ----------------------------------------------------------------------------
// QT Includes

#include <qstringlist.h>
#include <qcheckbox.h>
#include <qlayout.h>
//Added by qt3to4:
#include <Q3CString>

// ----------------------------------------------------------------------------
// KDE Includes

#include <ktrader.h>
#include <kparts/componentfactory.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kconfig.h>
#include <kpluginselector.h>
#include <klocale.h>
#include <KServiceTypeTrader>
#include <KService>
// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyplugin.h"
#include "pluginloader.h"

namespace KMyMoneyPlugin {

//---------------------------------------------------------------------
//
// PluginLoader
//
//---------------------------------------------------------------------
static PluginLoader* s_instance = 0;

typedef QMap<QString, Plugin*> PluginsMap;

struct PluginLoader::Private
{
  QObject*          m_parent;
  KPluginInfo::List m_pluginList;
  KPluginSelector*  m_pluginSelector;
  PluginsMap        m_loadedPlugins;
};

PluginLoader::PluginLoader(QObject* parent)
{
  Q_ASSERT( s_instance == 0 );
  s_instance = this;

  d = new Private;

  d->m_parent = parent;

  KService::List offers = KServiceTypeTrader::self()->query("KMyMoneyPlugin");
  d->m_pluginList = KPluginInfo::fromServices(offers);

  d->m_pluginSelector = new KPluginSelector(NULL);
#warning "port to kde4"
  //d->m_pluginSelector->setShowEmptyConfigPage(false);
  d->m_pluginSelector->addPlugins(d->m_pluginList);
  d->m_pluginSelector->load();

  connect(d->m_pluginSelector, SIGNAL(changed(bool)), this, SLOT(changed()));
  connect(d->m_pluginSelector, SIGNAL(configCommitted(const Q3CString &)), this, SLOT(changedConfigOfPlugin(const Q3CString &)));
}

PluginLoader::~PluginLoader()
{
  delete d;
}

void PluginLoader::loadPlugins()
{
  for( KPluginInfo::List::Iterator it = d->m_pluginList.begin(); it != d->m_pluginList.end(); ++it )
    loadPlugin( &(*it) );
}

void PluginLoader::loadPlugin(KPluginInfo* info)
{
  if (info->isPluginEnabled()) {
    Plugin* plugin = getPluginFromInfo(info);

    if (!plugin) {
      // the plugin is enabled but it is not loaded
      KService::Ptr service = info->service();
      int error = 0;
#warning "port to kde4"
#if 0
      Plugin* plugin = KService::createInstance<Plugin>(service, d->m_parent, info->name(), QStringList(), &error);
      if (plugin) {
        kDebug() << "KMyMoneyPlugin::PluginLoader: Loaded plugin " << plugin->name();
        d->m_loadedPlugins.insert(info->name(), plugin);
        emit PluginLoader::instance()->plug(info);
      }
      else {
        kWarning() << "KMyMoneyPlugin::PluginLoader:: createInstanceFromService returned 0 for "
                    << info->name()
                    << " with error number "
                    << error << endl;
#warning "port to kde4"
#if 0
	if (error == KParts::ComponentFactory::ErrNoLibrary)
          kWarning() << "KLibLoader says: "
                      << KLibLoader::self()->lastErrorMessage() << endl;
#endif
      }
#endif      
    }
  }
  else {
    if (getPluginFromInfo(info) != NULL) {
      // everybody interested should say goodbye to the plugin
      emit PluginLoader::instance()->unplug(info);
      d->m_loadedPlugins.erase(info->name());
    }
  }
}

void PluginLoader::changed()
{
  loadPlugins();
}

void PluginLoader::changedConfigOfPlugin(const Q3CString & name)
{
  PluginsMap::iterator itPlugin = d->m_loadedPlugins.find(QString(name));
  if (itPlugin != d->m_loadedPlugins.end())
    configChanged(*itPlugin);
}

Plugin* PluginLoader::getPluginFromInfo(KPluginInfo* info)
{
  PluginsMap::iterator itPlugin = d->m_loadedPlugins.find(info->name());
  if (itPlugin != d->m_loadedPlugins.end())
    return *itPlugin;
  else
    return NULL;
}

PluginLoader* PluginLoader::instance()
{
  Q_ASSERT( s_instance != 0);
  return s_instance;
}

KPluginSelector* PluginLoader::pluginSelectorWidget()
{
  return d->m_pluginSelector;
}

} // namespace

#include "pluginloader.moc"
