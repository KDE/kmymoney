/*
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2022 Alexander Lohnau <alexander.lohnau@gmx.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ksettingsplugins.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QVBoxLayout>
#include <QBitArray>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KConfigGroup>
#include <KLocalizedString>
#include <KPluginMetaData>
#include <KPluginWidget>
#include <KSharedConfig>

// ----------------------------------------------------------------------------
// Project Includes

#include "pluginloader.h"

struct pluginGroupInfo {
    QVector<KPluginMetaData> plugins;
    QString                           categoryName;
};

KSettingsPlugins::KSettingsPlugins(QWidget* parent)
    : QWidget(parent)
    , m_pluginSelector(new KPluginWidget(this))

{
    auto layout = new QVBoxLayout;
    setLayout(layout);  // otherwise KPluginSelector occupies very little area
    layout->addWidget(m_pluginSelector);

    auto allPluginDatas = KMyMoneyPlugin::listPlugins(false); // fetch all available KMyMoney plugins
    QVector<KPluginMetaData> standardPlugins;
    QVector<KPluginMetaData> payeePlugins;
    QVector<KPluginMetaData> onlinePlugins;

    // divide plugins in some arbitrary categories
    for (const KPluginMetaData& pluginData : allPluginDatas)
        switch (KMyMoneyPlugin::pluginCategory(pluginData)) {
        case KMyMoneyPlugin::Category::StandardPlugin:
            standardPlugins.append(pluginData);
            break;
        case KMyMoneyPlugin::Category::PayeeIdentifier:
            payeePlugins.append(pluginData);
            break;
        case KMyMoneyPlugin::Category::OnlineBankOperations:
            onlinePlugins.append(pluginData);
            break;
        default:
            break;
        }

    const QVector<pluginGroupInfo> pluginGroups{
        {standardPlugins, i18n("KMyMoney Plugins")},
        {payeePlugins, i18n("Payee Identifier")},
        {onlinePlugins, i18n("Online Banking Operations")},
    };

    KConfigGroup grp = KSharedConfig::openConfig()->group("Plugins");
    m_pluginSelector->setConfig(grp);

    // add all plugins to selector
    for(const auto& pluginGroup : pluginGroups) {
        if (!pluginGroup.plugins.isEmpty()) {
            m_pluginSelector->addPlugins(pluginGroup.plugins,
                                         pluginGroup.categoryName); // at that step plugin on/off state should be fetched automatically by KPluginSelector
        }
    }

    connect(m_pluginSelector, &KPluginWidget::changed, this, &KSettingsPlugins::changed);
}

void KSettingsPlugins::slotResetToDefaults()
{
    m_pluginSelector->defaults();
}

void KSettingsPlugins::slotSavePluginConfiguration()
{
    m_pluginSelector->save();
    emit settingsChanged(QStringLiteral("Plugins"));
}
