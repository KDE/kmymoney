/*
    SPDX-FileCopyrightText: 2005-2021 Thomas Baumgart <ipwizard@users.sourceforge.net>
    SPDX-FileCopyrightText: 2015 Christian Dávid <christian-david@web.de>
    SPDX-FileCopyrightText: 2021 Dawid Wróbel <me@dawidwrobel.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kmymoneyplugin.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QMap>
#include <QMimeDatabase>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KActionCollection>
#include <KPluginMetaData>
#include <KToggleAction>

// ----------------------------------------------------------------------------
// Project Includes
#include "interfaceloader.h"

KMyMoneyPlugin::Container pPlugins;

KMyMoneyPlugin::Plugin::Plugin(QObject* parent, const KPluginMetaData& metaData, const QVariantList& args)
    : QObject()
    , KXMLGUIClient()
{
    Q_UNUSED(parent)
    Q_UNUSED(args)

    setObjectName(metaData.pluginId());
    m_componentDisplayName = metaData.name();
    setComponentName(metaData.pluginId(), m_componentDisplayName);
}

KMyMoneyPlugin::Plugin::~Plugin()
{
}

QString KMyMoneyPlugin::Plugin::componentDisplayName() const
{
    return m_componentDisplayName;
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

bool KMyMoneyPlugin::ImporterPlugin::isMyFormat(const QString& filename) const
{
    QMimeDatabase db;
    QMimeType mime = db.mimeTypeForFile(filename);

    if (!mime.isDefault()) {
        const QStringList formatTypes = formatMimeTypes();
        for (const auto& mimeTypeName : qAsConst(formatTypes))
            if (mime.inherits(mimeTypeName)) {
                return true;
            }
    }
    return false;
}

KMyMoneyPlugin::ImporterPlugin::~ImporterPlugin()
{
}

QString KMyMoneyPlugin::StoragePlugin::openErrorMessage() const
{
    return {};
}
