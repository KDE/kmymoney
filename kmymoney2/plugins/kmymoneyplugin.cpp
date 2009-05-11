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

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <kcomponentdata.h>
#include <kaboutdata.h>
#include <kaction.h>

// ----------------------------------------------------------------------------
// Project Includes


#include "kmymoneyplugin.h"

KMyMoneyPlugin::Plugin::Plugin(QObject* o, const char* name) :
  QObject(o, name)
{
}

KMyMoneyPlugin::Plugin::~Plugin()
{
}

KAction* KMyMoneyPlugin::Plugin::action(const QString& actionName) const
{
  static KShortcut shortcut("");
  static KAction dummyAction(QString("Dummy"), QString(), shortcut, static_cast<const QObject*>(this), 0, static_cast<KActionCollection*>(0), "");

  KAction* p = actionCollection()->action(actionName.toLatin1());
  if(p)
    return p;

  qWarning("Action with name '%s' not found!", actionName.toLatin1());
  return &dummyAction;
}

KToggleAction* KMyMoneyPlugin::Plugin::toggleAction(const QString& actionName) const
{
  static KShortcut shortcut("");
  static KToggleAction dummyAction(QString("Dummy"), QString(), shortcut, static_cast<const QObject*>(this), 0, static_cast<KActionCollection*>(0), "");

  KAction* q = actionCollection()->action(actionName.toLatin1());

  if(q) {
    KToggleAction* p = dynamic_cast<KToggleAction*>(q);
    if(!p) {
      qWarning("Action '%s' is not of type KToggleAction", actionName.toLatin1());
      p = &dummyAction;
    }
    return p;
  }

  qWarning("Action with name '%s' not found!", actionName.toLatin1());
  return &dummyAction;
}

KMyMoneyPlugin::ViewInterface* KMyMoneyPlugin::Plugin::viewInterface() const
{
  return static_cast<ViewInterface*>( parent()->child( 0, "KMyMoneyPlugin::ViewInterface" ) );
}

KMyMoneyPlugin::StatementInterface* KMyMoneyPlugin::Plugin::statementInterface() const
{
  return static_cast<StatementInterface*>( parent()->child( 0, "KMyMoneyPlugin::StatementInterface" ) );
}

KMyMoneyPlugin::ImportInterface* KMyMoneyPlugin::Plugin::importInterface() const
{
  return static_cast<ImportInterface*>( parent()->child( 0, "KMyMoneyPlugin::ImportInterface" ) );
}

#include "kmymoneyplugin.moc"
