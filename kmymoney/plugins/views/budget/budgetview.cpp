/*

    SPDX-FileCopyrightText: 2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2021 Dawid Wróbel <me@dawidwrobel.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

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

#if KCOREADDONS_VERSION < QT_VERSION_CHECK(5, 77, 0)
BudgetView::BudgetView(QObject *parent, const QVariantList &args) :
    KMyMoneyPlugin::Plugin(parent, args),
#else
BudgetView::BudgetView(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args) :
    KMyMoneyPlugin::Plugin(parent, metaData, args),
#endif
    m_view(nullptr)
{
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
    const auto rcFileName = QLatin1String("budgetview.rc");
    setXMLFile(rcFileName);

    // create my actions and menus
    m_view->createActions(guiFactory, this);

    viewInterface()->addView(m_view, i18n("Budgets"), View::Budget, Icons::Icon::Budgets);
}

void BudgetView::unplug()
{
    m_view->removeActions();
    viewInterface()->removeView(View::Budget);
}

K_PLUGIN_CLASS_WITH_JSON(BudgetView, "budgetview.json")

#include "budgetview.moc"
