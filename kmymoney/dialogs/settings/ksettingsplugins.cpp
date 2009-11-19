/***************************************************************************
                          ksettingsplugins.cpp
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

#include "ksettingsplugins.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QLayout>
#include <QString>
#include <QLabel>
#include <QVBoxLayout>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>
#include <kdialog.h>
#include <kpluginselector.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoney/plugins/pluginloader.h"

KSettingsPlugins::KSettingsPlugins(QWidget* parent)
  : QWidget(parent)
{
  QVBoxLayout *layout = new QVBoxLayout(this);
  KMyMoneyPlugin::PluginLoader::instance()->pluginSelectorWidget()->setParent(this);
  layout->addWidget(KMyMoneyPlugin::PluginLoader::instance()->pluginSelectorWidget());
  layout->setSpacing(KDialog::spacingHint());
}

KSettingsPlugins::~KSettingsPlugins()
{
}

void KSettingsPlugins::slotLoadPlugins()
{
  KMyMoneyPlugin::PluginLoader::instance()->pluginSelectorWidget()->load();
}

void KSettingsPlugins::slotSavePlugins()
{
  KMyMoneyPlugin::PluginLoader::instance()->pluginSelectorWidget()->save();
}

void KSettingsPlugins::slotDefaultsPlugins()
{
  KMyMoneyPlugin::PluginLoader::instance()->pluginSelectorWidget()->defaults();
}

#include "ksettingsplugins.moc"
