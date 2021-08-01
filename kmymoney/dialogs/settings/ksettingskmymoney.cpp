/*
    SPDX-FileCopyrightText: 2014-2016 Christian Dávid <christian-david@web.de>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ksettingskmymoney.h"

#include <QPushButton>

#include <KLocalizedString>

#include "ksettingsgeneral.h"
#include "ksettingsregister.h"
#include "ksettingscolors.h"
#include "ksettingsfonts.h"
#include "ksettingsicons.h"
#include "ksettingsschedules.h"
#include "ksettingsonlinequotes.h"
#include "ksettingshome.h"
#include "ksettingsplugins.h"

#include "icons.h"

// ----------------------------------------------------------------------------
// Alikimia Includes
#include <alkimia/alkonlinequotesprofile.h>
#include <alkimia/alkonlinequotesprofilemanager.h>
#include <alkimia/alkonlinequoteswidget.h>

using namespace Icons;

KSettingsKMyMoney::KSettingsKMyMoney(QWidget *parent, const QString &name, KCoreConfigSkeleton *config)
    : KConfigDialog(parent, name, config)
{
    // create the pages ...
    const auto generalPage = new KSettingsGeneral();
    const auto registerPage = new KSettingsRegister();
    const auto homePage = new KSettingsHome();
    const auto schedulesPage = new KSettingsSchedules();
    const auto colorsPage = new KSettingsColors();
    const auto fontsPage = new KSettingsFonts();
    const auto iconsPage = new KSettingsIcons();
    const auto onlineQuotesPage = new AlkOnlineQuotesWidget;
    const auto pluginsPage = new KSettingsPlugins();

    addPage(generalPage, i18nc("General settings", "General"), Icons::get(Icon::PreferencesGeneral).name());
    addPage(homePage, i18n("Home"), Icons::get(Icon::Home).name());
    addPage(registerPage, i18nc("Ledger view settings", "Ledger"), Icons::get(Icon::Ledgers).name());
    addPage(schedulesPage, QString(i18n("Scheduled\ntransactions")).replace(QLatin1Char('\n'), QString::fromUtf8("\xe2\x80\xa8")), Icons::get(Icon::Schedule).name());
    addPage(onlineQuotesPage, i18n("Online Quotes"), Icons::get(Icon::PreferencesNetwork).name());
    addPage(colorsPage, i18n("Colors"), Icons::get(Icon::PreferencesColors).name());
    addPage(fontsPage, i18n("Fonts"), Icons::get(Icon::PreferencesFonts).name());
#if not defined(Q_OS_MACOS) && not defined(Q_OS_WIN)
    addPage(iconsPage, i18n("Icons"), Icons::get(Icon::PreferencesIcons).name());
#endif
    addPage(pluginsPage, i18n("Plugins"), Icons::get(Icon::PreferencesPlugins).name(), QString(), false);

    setHelp("details.settings", "kmymoney");

    connect(this, &KConfigDialog::rejected, schedulesPage, &KSettingsSchedules::slotResetRegion);
    connect(this, &KConfigDialog::rejected, iconsPage, &KSettingsIcons::slotResetTheme);

    auto defaultButton = button(QDialogButtonBox::RestoreDefaults);
    auto applyButton = button(QDialogButtonBox::Apply);
    connect(this, &KConfigDialog::accepted, pluginsPage, &KSettingsPlugins::slotSavePluginConfiguration);
    connect(applyButton, &QPushButton::clicked, pluginsPage, &KSettingsPlugins::slotSavePluginConfiguration);
    connect(defaultButton, &QPushButton::clicked, pluginsPage, &KSettingsPlugins::slotResetToDefaults);
    connect(pluginsPage, &KSettingsPlugins::changed, this, &KSettingsKMyMoney::slotPluginsChanged);
    connect(pluginsPage, &KSettingsPlugins::settingsChanged, this, &KConfigDialog::settingsChanged);
}

void KSettingsKMyMoney::slotPluginsChanged(bool changed)
{
    auto applyButton = button(QDialogButtonBox::Apply);
    applyButton->setEnabled(changed);
}
