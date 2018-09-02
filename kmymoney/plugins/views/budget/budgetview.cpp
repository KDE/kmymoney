/***************************************************************************
                            budgetview.cpp
                             -------------------

    copyright            : (C) 2018 by Łukasz Wojniłowicz
    email                : lukasz.wojnilowicz@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "budgetview.h"

// ----------------------------------------------------------------------------
// QT Includes

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
  setComponentName("budgetview", i18n("Budgets view"));
  // For information, announce that we have been loaded.
  qDebug("Plugins: budgetview loaded");
}

BudgetView::~BudgetView()
{
  qDebug("Plugins: budgetview unloaded");
}

void BudgetView::plug()
{
  m_view = new KBudgetView;
  viewInterface()->addView(m_view, i18n("Budgets"), View::Budget);
}

void BudgetView::unplug()
{
  viewInterface()->removeView(View::Budget);
}

K_PLUGIN_FACTORY_WITH_JSON(BudgetViewFactory, "budgetview.json", registerPlugin<BudgetView>();)

#include "budgetview.moc"
