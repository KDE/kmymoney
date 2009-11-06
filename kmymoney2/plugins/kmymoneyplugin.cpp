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
#include <KToggleAction>
#include <KActionCollection>
// ----------------------------------------------------------------------------
// Project Includes


#include "kmymoneyplugin.h"

KMyMoneyPlugin::Plugin::Plugin(QObject* o, const char* name) :
  QObject(o)
{
	setObjectName(name);
}

KMyMoneyPlugin::Plugin::~Plugin()
{
}

KAction* KMyMoneyPlugin::Plugin::action(const QString& actionName) const
{
//FIXME: Port to KDE4
#if 0
  static KShortcut shortcut("");
  static KAction dummyAction(QString("Dummy"), QString(), shortcut, static_cast<const QObject*>(this), 0, static_cast<KActionCollection*>(0), "");

  KAction* p = actionCollection()->action(actionName);
  if(p)
    return p;

  qWarning("Action with name '%s' not found!", actionName);
  return &dummyAction;
#endif
  return 0;
}

KToggleAction* KMyMoneyPlugin::Plugin::toggleAction(const QString& actionName) const
{
//FIXME: Port to KDE4
#if 0
  static KShortcut shortcut("");
  static KToggleAction dummyAction( QString("Dummy"), static_cast<const QObject*>(this ) );
  dummyAction.setShortcut( shortcut );
  //static KToggleAction dummyAction(QString("Dummy"), QString(), shortcut, static_cast<const QObject*>(this), 0, static_cast<KActionCollection*>(0), "");

  KAction* q = actionCollection()->action(actionName);

  if(q) {
    KToggleAction* p = dynamic_cast<KToggleAction*>(q);
    if(!p) {
      qWarning("Action '%1' is not of type KToggleAction", actionName);
      p = &dummyAction;
    }
    return p;
  }

  qWarning("Action with name '%s' not found!", actionName);
  return &dummyAction;
#endif
  return 0;
}

KMyMoneyPlugin::ViewInterface* KMyMoneyPlugin::Plugin::viewInterface() const
{
  return parent()->findChild<ViewInterface*>();
}

KMyMoneyPlugin::StatementInterface* KMyMoneyPlugin::Plugin::statementInterface() const
{
  return parent()->findChild<StatementInterface*>();
}

KMyMoneyPlugin::ImportInterface* KMyMoneyPlugin::Plugin::importInterface() const
{
  return parent()->findChild<ImportInterface*>();
}

#include "kmymoneyplugin.moc"
