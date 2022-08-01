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

class KSettingsPluginsPrivate
{
    Q_DISABLE_COPY(KSettingsPluginsPrivate)

public:
    explicit KSettingsPluginsPrivate(KSettingsPlugins* qq)
        : m_pluginSelector(new KPluginWidget(qq))
    {
    }

    ~KSettingsPluginsPrivate()
    {
        delete m_pluginSelector;
    }

    KPluginWidget* const m_pluginSelector;
    /**
     * @brief savedPluginStates This caches on/off states as in kmymoneyrc
     */
    QBitArray savedPluginStates;
};

KSettingsPlugins::KSettingsPlugins(QWidget* parent) :
    QWidget(parent),
    d_ptr(new KSettingsPluginsPrivate(this))

{
    Q_D(KSettingsPlugins);
    auto layout = new QVBoxLayout;
    setLayout(layout);  // otherwise KPluginSelector occupies very little area
    layout->addWidget(d->m_pluginSelector);

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
    d->m_pluginSelector->setConfig(grp);

    // add all plugins to selector
    for(const auto& pluginGroup : pluginGroups) {
        if (!pluginGroup.plugins.isEmpty()) {
            d->m_pluginSelector->addPlugins(pluginGroup.plugins,
                                            pluginGroup.categoryName); // at that step plugin on/off state should be fetched automatically by KPluginSelector
        }
    }

    connect(d->m_pluginSelector, &KPluginWidget::changed, this, &KSettingsPlugins::changed);
}

KSettingsPlugins::~KSettingsPlugins()
{
    Q_D(KSettingsPlugins);
    delete d;
}

void KSettingsPlugins::slotResetToDefaults()
{
    Q_D(KSettingsPlugins);
    d->m_pluginSelector->defaults();
}

void KSettingsPlugins::slotSavePluginConfiguration()
{
    Q_D(KSettingsPlugins);
    d->m_pluginSelector->save();
    emit settingsChanged(QStringLiteral("Plugins"));
}
