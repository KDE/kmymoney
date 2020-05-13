/*
 * Copyright 2014-2016  Christian Dávid <christian-david@web.de>
 * Copyright 2017-2018  Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
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
  const auto onlineQuotesPage = new KSettingsOnlineQuotes();
  const auto pluginsPage = new KSettingsPlugins();

  addPage(generalPage, i18nc("General settings", "General"), Icons::get(Icon::PreferencesGeneral).name());
  addPage(homePage, i18n("Home"), Icons::get(Icon::ViewHome).name());
  addPage(registerPage, i18nc("Ledger view settings", "Ledger"), Icons::get(Icon::ViewFinancialList).name());
  addPage(schedulesPage, QString(i18n("Scheduled\ntransactions")).replace(QLatin1Char('\n'), QString::fromUtf8("\xe2\x80\xa8")), Icons::get(Icon::ViewSchedules).name());
  addPage(onlineQuotesPage, i18n("Online Quotes"), Icons::get(Icon::PreferencesNetwork).name());
  addPage(colorsPage, i18n("Colors"), Icons::get(Icon::PreferencesColor).name());
  addPage(fontsPage, i18n("Fonts"), Icons::get(Icon::PreferencesFont).name());
  addPage(iconsPage, i18n("Icons"), Icons::get(Icon::PreferencesIcon).name());
  addPage(pluginsPage, i18n("Plugins"), Icons::get(Icon::PreferencesPlugin).name(), QString(), false);

  setHelp("details.settings", "kmymoney");

  connect(this, &KConfigDialog::rejected, schedulesPage, &KSettingsSchedules::slotResetRegion);
  connect(this, &KConfigDialog::rejected, iconsPage, &KSettingsIcons::slotResetTheme);
  connect(this, &KConfigDialog::settingsChanged, generalPage, &KSettingsGeneral::slotUpdateEquitiesVisibility);

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
