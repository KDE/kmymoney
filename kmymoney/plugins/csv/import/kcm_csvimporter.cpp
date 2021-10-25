/*
    SPDX-FileCopyrightText: 2016-2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2021 Dawid Wróbel <me@dawidwrobel.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kcm_csvimporter.h"
#include <config-kmymoney-version.h>

// KDE includes
#include <KPluginFactory>
#include <KAboutData>
#include "pluginsettings.h"

CSVImporterSettingsWidget::CSVImporterSettingsWidget(QWidget* parent) :
    QWidget(parent)
{
    setupUi(this);
}

KCMCSVImporter::KCMCSVImporter(QWidget *parent, const QVariantList& args)
    : KCModule(parent, args)
{
    CSVImporterSettingsWidget* w = new CSVImporterSettingsWidget(this);
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

K_PLUGIN_CLASS_WITH_JSON(KCMCSVImporter, "kcm_csvimporter.json")

#include "kcm_csvimporter.moc"
