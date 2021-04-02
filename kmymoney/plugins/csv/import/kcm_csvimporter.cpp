/*
    SPDX-FileCopyrightText: 2016-2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kcm_csvimporter.h"
#include <config-kmymoney-version.h>

// KDE includes
#include <KPluginFactory>
#include <KAboutData>
#include "pluginsettings.h"

PluginSettingsWidget::PluginSettingsWidget(QWidget* parent) :
    QWidget(parent)
{
    setupUi(this);
}

KCMCSVImporter::KCMCSVImporter(QWidget *parent, const QVariantList& args)
    : KCModule(parent, args)
{
    PluginSettingsWidget* w = new PluginSettingsWidget(this);
    addConfig(PluginSettings::self(), w);
    QVBoxLayout *layout = new QVBoxLayout;
    setLayout(layout);
    layout->addWidget(w);
    setButtons(NoAdditionalButton);
    load();
}

KCMCSVImporter::~KCMCSVImporter()
{
}

K_PLUGIN_FACTORY_WITH_JSON(KCMCSVImporterFactory, "kcm_csvimporter.json", registerPlugin<KCMCSVImporter>();)

#include "kcm_csvimporter.moc"
