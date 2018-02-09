/***************************************************************************
                          kmymoneyplugin.cpp
                             -------------------
    begin                : Wed Jan 5 2005
    copyright            : (C) 2005 Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kmymoneyplugin.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <KToggleAction>
#include <KActionCollection>

// ----------------------------------------------------------------------------
// Project Includes
#include "interfaceloader.h"

KMyMoneyPlugin::Plugin::Plugin(QObject* parent, const char* name) :
    QObject(),
    KXMLGUIClient()
{
  Q_UNUSED(parent)

  setObjectName(name);
}

KMyMoneyPlugin::Plugin::~Plugin()
{
}

void KMyMoneyPlugin::Plugin::plug()
{
}

void KMyMoneyPlugin::Plugin::unplug()
{
}

void KMyMoneyPlugin::Plugin::configurationChanged()
{
}

KToggleAction* KMyMoneyPlugin::Plugin::toggleAction(const QString& actionName) const
{
  static KToggleAction dummyAction(QString("Dummy"), 0);

  KToggleAction* p = dynamic_cast<KToggleAction*>(actionCollection()->action(QString(actionName.toLatin1())));
  if (!p) {
    qWarning("Action '%s' is not of type KToggleAction", qPrintable(actionName));
    p = &dummyAction;
  }

  qWarning("Action with name '%s' not found!", qPrintable(actionName));
  return p;
}

KMyMoneyPlugin::OnlinePlugin::OnlinePlugin()
{
}

KMyMoneyPlugin::OnlinePlugin::~OnlinePlugin()
{
}

KMyMoneyPlugin::AppInterface* KMyMoneyPlugin::Plugin::appInterface() const
{
  Q_CHECK_PTR(KMyMoneyPlugin::pluginInterfaces().appInterface);
  return KMyMoneyPlugin::pluginInterfaces().appInterface;
}

KMyMoneyPlugin::ViewInterface* KMyMoneyPlugin::Plugin::viewInterface() const
{
  Q_CHECK_PTR(KMyMoneyPlugin::pluginInterfaces().viewInterface);
  return KMyMoneyPlugin::pluginInterfaces().viewInterface;
}

KMyMoneyPlugin::StatementInterface* KMyMoneyPlugin::Plugin::statementInterface() const
{
  Q_CHECK_PTR(KMyMoneyPlugin::pluginInterfaces().statementInterface);
  return KMyMoneyPlugin::pluginInterfaces().statementInterface;
}

KMyMoneyPlugin::ImportInterface* KMyMoneyPlugin::Plugin::importInterface() const
{
  Q_CHECK_PTR(KMyMoneyPlugin::pluginInterfaces().importInterface);
  return KMyMoneyPlugin::pluginInterfaces().importInterface;
}

KMyMoneyPlugin::ImporterPlugin::ImporterPlugin()
{
}

KMyMoneyPlugin::ImporterPlugin::~ImporterPlugin()
{
}
