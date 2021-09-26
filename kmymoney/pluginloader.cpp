/*
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2021 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2021 Dawid Wróbel <me@dawidwrobel.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#include "pluginloader.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QMap>
#include <QDebug>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KPluginFactory>
#include <KPluginInfo>
#include <KPluginLoader>
#include <KPluginMetaData>
#include <KSharedConfig>
#include <KXMLGUIFactory>
#include <QJsonDocument>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyplugin.h"
#include "onlinepluginextended.h"

#include <QtPlugin>
Q_IMPORT_PLUGIN(CSVImporterCore)
Q_IMPORT_PLUGIN(CSVImporter)

namespace KMyMoneyPlugin
{
Category pluginCategory(const KPluginMetaData& pluginInfo)
{
    if (!pluginInfo.serviceTypes().contains(QStringLiteral("KMyMoney/Plugin"))) {
        auto jsonKMyMoneyData = pluginInfo.rawData()[QLatin1String("KMyMoney")].toObject();
        if (!jsonKMyMoneyData[QLatin1String("OnlineTask")].isNull())
            return OnlineBankOperations;
        else if (!jsonKMyMoneyData[QLatin1String("PayeeIdentifier")].isNull())
            return PayeeIdentifier;
    }
    return StandardPlugin;
}

bool isPluginEnabled(const KPluginMetaData& pluginData, const KConfigGroup& pluginSection)
{
    return pluginSection.readEntry(QString::fromLatin1("%1Enabled").    // we search here for e.g. "csvimporterEnabled = true"
                                   arg(pluginData.pluginId()),
                                   pluginData.isEnabledByDefault());    // if not found, then get default from plugin's json file
}

QMap<QString, PluginMetaFactory> discoverPlugins(bool onlyEnabled)
{
    QMap<QString, PluginMetaFactory> plugins;
    const auto pluginSection(KSharedConfig::openConfig()->group(QStringLiteral("Plugins"))); // section of config where plugin on/off were saved

    const auto staticPlugins = QPluginLoader::staticPlugins();
    for (auto& staticPlugin : staticPlugins) {
        QJsonObject jsonMetadata = staticPlugin.metaData().value(QStringLiteral("MetaData")).toObject();
        auto test = QJsonDocument(jsonMetadata).toJson(QJsonDocument::JsonFormat::Indented);

        KPluginMetaData pluginData(jsonMetadata, QString());
        if (pluginData.serviceTypes().contains(QStringLiteral("KMyMoney/Plugin"))) {
            if (!onlyEnabled || isPluginEnabled(pluginData, pluginSection)) {
                plugins.insert(pluginData.pluginId(), PluginMetaFactory(pluginData, nullptr, qobject_cast<KPluginFactory*>(staticPlugin.instance())));
            }
        }
    }

    const auto pluginDatas = KPluginLoader::findPlugins(QStringLiteral("kmymoney")); // that means search for plugins in "/lib64/plugins/kmymoney/"
    for (const KPluginMetaData& pluginData : pluginDatas) {
        auto test = QJsonDocument(pluginData.rawData()).toJson(QJsonDocument::JsonFormat::Indented);

        if (pluginData.serviceTypes().contains(QStringLiteral("KMyMoney/Plugin"))) {
            if (!onlyEnabled || isPluginEnabled(pluginData, pluginSection)) {
                // only use the first one found. Otherwise, always the last one would
                // win (usually the installed system version) with the QT_PLUGIN_PATH
                // env variable or the current directory having no effect on KMyMoney
                if (!plugins.contains(pluginData.pluginId())) {
                    KPluginLoader loader(pluginData.fileName());
                    auto factory = loader.factory();
                    if (!factory) {
                        qWarning("Could not load plugin '%s', error: %s", qPrintable(pluginData.fileName()), qPrintable(loader.errorString()));
                        loader.unload();
                        continue;
                    } else
                        plugins.insert(pluginData.pluginId(), PluginMetaFactory(pluginData, &loader, factory));
                } else {
                    qWarning("Could not load plugin '%s' – a plugin with the same '%s' ID already loaded",
                             qPrintable(pluginData.fileName()),
                             qPrintable(pluginData.pluginId()));
                }
            }
        }
    }

    return plugins;
}

void pluginHandling(Action action, Container& ctnPlugins, QObject* parent, KXMLGUIFactory* guiFactory)
{

    if (action == Action::Load ||
            action == Action::Reorganize) {
        KPluginLoader::forEachPlugin(QStringLiteral("kmymoney"), [&](const QString &pluginPath) {
            KPluginMetaData metadata(pluginPath);
            qDebug() << "Located shared plugin" << pluginPath << "Validity" << metadata.isValid();
        });
    }

    QMap<QString, PluginMetaFactory> referencePluginDatas;
    if (action == Action::Load ||
            action == Action::Reorganize)
        referencePluginDatas = discoverPlugins(true);

    if (action == Action::Unload ||
            action == Action::Reorganize) {
        auto& plugins = ctnPlugins.standard;
        auto& refPlugins = referencePluginDatas;
        for (auto it = plugins.begin(); it != plugins.end();) {
            if (!refPlugins.contains(it.key())) {

                ctnPlugins.online.remove(it.key());
                ctnPlugins.extended.remove(it.key());
                ctnPlugins.importer.remove(it.key());
                ctnPlugins.storage.remove(it.key());
                ctnPlugins.data.remove(it.key());

                guiFactory->removeClient(it.value());
                it.value()->unplug();
                delete it.value();
                it = plugins.erase(it);
                continue;

            }
            ++it;
        }
    }

    if (action == Action::Load ||
            action == Action::Reorganize) {

        auto& refPlugins = referencePluginDatas;
        for (auto it = refPlugins.cbegin(); it != refPlugins.cend(); ++it) {
            if (!ctnPlugins.standard.contains(it.key())) {
                qDebug() << "Loading" << (*it).pluginMetaData.fileName();
                auto factory = (*it).pluginFactory;
#if KCOREADDONS_VERSION < QT_VERSION_CHECK(5, 77, 0)
                Plugin* plugin = factory->create<Plugin>(parent, QVariantList{(*it).pluginMetaData.pluginId(), (*it).pluginMetaData.name()});
#else
                Plugin* plugin = factory->create<Plugin>(parent);
#endif
                if (!plugin) {
                    qWarning("Failed to instantiate plugin: '%s'", qPrintable((*it).pluginMetaData.fileName()));
                    auto loader = (*it).pluginLoader;
                    if (loader)
                        loader->unload();
                    continue;
                }

                ctnPlugins.standard.insert((*it).pluginMetaData.pluginId(), plugin);
                plugin->plug(guiFactory);
                guiFactory->addClient(plugin);

                auto IOnline = qobject_cast<OnlinePlugin *>(plugin);
                if (IOnline)
                    ctnPlugins.online.insert((*it).pluginMetaData.pluginId(), IOnline);

                auto IExtended = qobject_cast<OnlinePluginExtended *>(plugin);
                if (IExtended )
                    ctnPlugins.extended.insert((*it).pluginMetaData.pluginId(), IExtended);

                auto IImporter = qobject_cast<ImporterPlugin *>(plugin);
                if (IImporter)
                    ctnPlugins.importer.insert((*it).pluginMetaData.pluginId(), IImporter);

                auto IStorage = qobject_cast<StoragePlugin *>(plugin);
                if (IStorage)
                    ctnPlugins.storage.insert((*it).pluginMetaData.pluginId(), IStorage);

                auto IData = qobject_cast<DataPlugin *>(plugin);
                if (IData)
                    ctnPlugins.data.insert((*it).pluginMetaData.pluginId(), IData);
            }
        }

    }
}

void updateActions(const Container& plugins, const SelectedObjects& selections)
{
    for(const auto& plugin : plugins.standard) {
        plugin->updateActions(selections);
    }
}

void updateConfiguration(const Container& plugins)
{
    for(const auto& plugin : plugins.standard) {
        plugin->updateConfiguration();
    }
}
}
