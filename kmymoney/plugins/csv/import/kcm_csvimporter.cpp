/*
    SPDX-FileCopyrightText: 2016-2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2021 Dawid Wróbel <me@dawidwrobel.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kcm_csvimporter.h"
#include <config-kmymoney-version.h>

// KDE includes
#include "kmymoneypluginclass.h"
#include "pluginsettings.h"
#include <KAboutData>
#include <KPluginFactory>

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

K_PLUGIN_CLASS(KCMCSVImporter)

#include "kcm_csvimporter.moc"
