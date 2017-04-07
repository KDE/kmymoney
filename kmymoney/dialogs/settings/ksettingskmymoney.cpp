/*
 * This file is part of KMyMoney, A Personal Finance Manager for KDE
 * Copyright (C) 2016 Christian Dávid <christian-david@web.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ksettingskmymoney.h"

#include <KPluginSelector>

#include "ksettingsgeneral.h"
#include "ksettingsregister.h"
#include "ksettingsgpg.h"
#include "ksettingscolors.h"
#include "ksettingsfonts.h"
#include "ksettingsicons.h"
#include "ksettingsschedules.h"
#include "ksettingsonlinequotes.h"
#include "ksettingshome.h"
#include "ksettingsforecast.h"
#include "ksettingsreports.h"

#include "pluginloader.h"

KSettingsKMyMoney::KSettingsKMyMoney(QWidget *parent, const QString &name, KCoreConfigSkeleton *config)
    : KConfigDialog(parent, name, config)
{
  // create the pages ...
  KSettingsGeneral* generalPage = new KSettingsGeneral();
  KSettingsRegister* registerPage = new KSettingsRegister();
  KSettingsHome* homePage = new KSettingsHome();
  KSettingsSchedules* schedulesPage = new KSettingsSchedules();
  KSettingsGpg* encryptionPage = new KSettingsGpg();
  KSettingsColors* colorsPage = new KSettingsColors();
  KSettingsFonts* fontsPage = new KSettingsFonts();
  KSettingsIcons* iconsPage = new KSettingsIcons();
  KSettingsOnlineQuotes* onlineQuotesPage = new KSettingsOnlineQuotes();
  KSettingsForecast* forecastPage = new KSettingsForecast();
  KPluginSelector* pluginsPage = KMyMoneyPlugin::PluginLoader::instance()->pluginSelectorWidget();
  KSettingsReports* reportsPage = new KSettingsReports();

  addPage(generalPage, i18nc("General settings", "General"), "system-run");
  addPage(homePage, i18n("Home"), "go-home");

  QString iconName;
  iconName = "view-financial-list";
  if (!QIcon::hasThemeIcon(iconName))
    iconName = "ledger";
  addPage(registerPage, i18nc("Ledger view settings", "Ledger"), iconName);

  iconName = "view-pim-calendar";
  if (!QIcon::hasThemeIcon(iconName))
    iconName = "schedule";
  addPage(schedulesPage, i18n("Scheduled transactions"), iconName);

  addPage(onlineQuotesPage, i18n("Online Quotes"), "preferences-system-network");

  iconName = "office-chart-bar";
  if (!QIcon::hasThemeIcon(iconName))
    iconName = "report";
  addPage(reportsPage, i18nc("Report settings", "Reports"), iconName);

  iconName = "view-financial-forecast";
  if (!QIcon::hasThemeIcon(iconName))
    iconName = "forecast";
  addPage(forecastPage, i18nc("Forecast settings", "Forecast"), iconName);

  addPage(encryptionPage, i18n("Encryption"), "kgpg");
  addPage(colorsPage, i18n("Colors"), "preferences-desktop-color");
  addPage(fontsPage, i18n("Fonts"), "preferences-desktop-font");
  addPage(iconsPage, i18n("Icons"), "preferences-desktop-icon");
  addPage(pluginsPage, i18n("Plugins"), "network-disconnect");

  setHelp("details.settings", "kmymoney");

  QAbstractButton* defaultButton = button(QDialogButtonBox::RestoreDefaults);
  connect(this, &KConfigDialog::rejected, schedulesPage, &KSettingsSchedules::slotResetRegion);
  connect(this, &KConfigDialog::rejected, iconsPage, &KSettingsIcons::slotResetTheme);
  connect(this, &KConfigDialog::settingsChanged, generalPage, &KSettingsGeneral::slotUpdateEquitiesVisibility);

  connect(this, &KConfigDialog::accepted, pluginsPage, &KPluginSelector::save);
  connect(defaultButton, &QAbstractButton::clicked, pluginsPage, &KPluginSelector::defaults);
}
