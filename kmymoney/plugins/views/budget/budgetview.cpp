/***************************************************************************
                            budgetview.cpp
                             -------------------

    copyright            : (C) 2018 by Łukasz Wojniłowicz
    email                : lukasz.wojnilowicz@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

#include "budgetview.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDebug>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KPluginFactory>
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "viewinterface.h"
#include "kbudgetview.h"

BudgetView::BudgetView(QObject *parent, const QVariantList &args) :
  KMyMoneyPlugin::Plugin(parent, "budgetview"/*must be the same as X-KDE-PluginInfo-Name*/),
  m_view(nullptr)
{
  Q_UNUSED(args)
  // For information, announce that we have been loaded.
  qDebug("Plugins: budgetview loaded");
}

BudgetView::~BudgetView()
{
  qDebug("Plugins: budgetview unloaded");
}

void BudgetView::plug(KXMLGUIFactory* guiFactory)
{
  m_view = new KBudgetView;

  // Tell the host application to load my GUI component
  const auto componentName = QLatin1String("budgetview");
  const auto rcFileName = QLatin1String("budgetview.rc");
  setComponentName(componentName, i18n("Budgets view"));

  #ifdef IS_APPIMAGE
  const QString rcFilePath = QString("%1/../share/kxmlgui5/%2/%3").arg(QCoreApplication::applicationDirPath(), componentName, rcFileName);
  setXMLFile(rcFilePath);

  const QString localRcFilePath = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation).first() + QLatin1Char('/') + componentName + QLatin1Char('/') + rcFileName;
  setLocalXMLFile(localRcFilePath);
  #else
  setXMLFile(rcFileName);
  #endif

  // create my actions and menus
  m_view->createActions(guiFactory, this);

  viewInterface()->addView(m_view, i18n("Budgets"), View::Budget, Icons::Icon::Budget);
}

void BudgetView::unplug()
{
  m_view->removeActions();
  viewInterface()->removeView(View::Budget);
}

K_PLUGIN_FACTORY_WITH_JSON(BudgetViewFactory, "budgetview.json", registerPlugin<BudgetView>();)

#include "budgetview.moc"
