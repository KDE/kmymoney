/*
 * SPDX-FileCopyrightText: 2005-2021 Thomas Baumgart <ipwizard@users.sourceforge.net>
 * SPDX-FileCopyrightText: 2015 Christian Dávid <christian-david@web.de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kmymoneyplugin.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QMap>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KToggleAction>
#include <KActionCollection>

// ----------------------------------------------------------------------------
// Project Includes
#include "interfaceloader.h"

KMyMoneyPlugin::Container pPlugins;

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

void KMyMoneyPlugin::Plugin::plug(KXMLGUIFactory* guiFactory)
{
  Q_UNUSED(guiFactory)
}

void KMyMoneyPlugin::Plugin::unplug()
{
}

void KMyMoneyPlugin::Plugin::updateActions(const SelectedObjects& selections)
{
    Q_UNUSED(selections)
}

void KMyMoneyPlugin::Plugin::updateConfiguration()
{
}

KToggleAction* KMyMoneyPlugin::Plugin::toggleAction(const QString& actionName) const
{
  static KToggleAction dummyAction(QString("Dummy"), nullptr);

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
