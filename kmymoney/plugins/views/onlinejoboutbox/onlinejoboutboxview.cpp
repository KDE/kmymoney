/*

    SPDX-FileCopyrightText: 2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2021 Dawid Wróbel <me@dawidwrobel.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "onlinejoboutboxview.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <KPluginFactory>
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "viewinterface.h"
#include "konlinejoboutboxview.h"

#if KCOREADDONS_VERSION < QT_VERSION_CHECK(5, 77, 0)
OnlineJobOutboxView::OnlineJobOutboxView(QObject *parent, const QVariantList &args) :
    KMyMoneyPlugin::Plugin(parent, args),
#else
OnlineJobOutboxView::OnlineJobOutboxView(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args) :
    KMyMoneyPlugin::Plugin(parent, metaData, args),
#endif
    m_view(nullptr)
{
    // For information, announce that we have been loaded.
    qDebug("Plugins: onlinejoboutboxview loaded");
}

OnlineJobOutboxView::~OnlineJobOutboxView()
{
    qDebug("Plugins: onlinejoboutboxview unloaded");
}

void OnlineJobOutboxView::plug(KXMLGUIFactory* guiFactory)
{
    m_view = new KOnlineJobOutboxView;

    // Tell the host application to load my GUI component
    const auto componentName = QLatin1String("onlinejoboutboxview");
    const auto rcFileName = QLatin1String("onlinejoboutboxview.rc");
    setComponentName(componentName, i18nc("@item:inlistbox", "Budgets view"));

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

    viewInterface()->addView(m_view, i18nc("@item name of view", "Outbox"), View::OnlineJobOutbox, Icons::Icon::OnlineJobOutbox);
}

void OnlineJobOutboxView::unplug()
{
    viewInterface()->removeView(View::OnlineJobOutbox);
}

K_PLUGIN_CLASS_WITH_JSON(OnlineJobOutboxView, "onlinejoboutboxview.json")

#include "onlinejoboutboxview.moc"
