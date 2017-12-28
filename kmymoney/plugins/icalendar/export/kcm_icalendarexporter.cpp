/***************************************************************************
 *   Copyright 2009  Cristian Onet onet.cristian@gmail.com                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public License as        *
 *   published by the Free Software Foundation; either version 2 of        *
 *   the License or (at your option) version 3 or any later version        *
 *   accepted by the membership of KDE e.V. (or its successor approved     *
 *   by the membership of KDE e.V.), which shall act as a proxy            *
 *   defined in Section 14 of version 3 of the license.                    *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>  *
 ***************************************************************************/

#include "kcm_icalendarexporter.h"
#include <config-kmymoney-version.h>

// Qt includes
#include <QComboBox>
#include <QBoxLayout>

// KDE includes
#include <KPluginFactory>
#include <KLocalizedString>
#include <KAboutData>

#include "pluginsettings.h"
#include "ui_pluginsettingsdecl.h"

class PluginSettingsWidget : public QWidget, public Ui::PluginSettingsDecl
{
public:
  PluginSettingsWidget(QWidget* parent = 0) : QWidget(parent) {
    setupUi(this);

    kcfg_timeUnitInSeconds->addItem(i18n("Minutes"));
    kcfg_timeUnitInSeconds->addItem(i18n("Hours"));
    kcfg_timeUnitInSeconds->addItem(i18nc("Time unit", "Days"));

    kcfg_intervalBetweenRemindersTimeUnitInSeconds->addItem(i18n("Minutes"));
    kcfg_intervalBetweenRemindersTimeUnitInSeconds->addItem(i18n("Hours"));
    kcfg_intervalBetweenRemindersTimeUnitInSeconds->addItem(i18nc("Time unit", "Days"));

    kcfg_beforeAfter->addItem(i18n("Before"));
    kcfg_beforeAfter->addItem(i18n("After"));
  }
};

KCMiCalendarExporter::KCMiCalendarExporter(QWidget *parent, const QVariantList& args)
  : KCModule(parent, args)
{
  KAboutData *about = new KAboutData(QStringLiteral("kmm_printcheck"),
                                    i18n("KMyMoney print check"),
                                    QStringLiteral(VERSION), QString(),
                                    KAboutLicense::GPL,
                                    i18n("Copyright 2009" ) );
  about->addAuthor( QString::fromUtf8("Cristian Oneț") );

  setAboutData( about );

  PluginSettingsWidget *w = new PluginSettingsWidget(this);
  addConfig(PluginSettings::self(), w);
  QVBoxLayout *layout = new QVBoxLayout;
  setLayout(layout);
  layout->addWidget(w);
  load();
}

KCMiCalendarExporter::~KCMiCalendarExporter()
{
}

K_PLUGIN_FACTORY_WITH_JSON(KCMiCalendarExporterFactory,
                           "kcm_icalendarexporter.json",
                           registerPlugin<KCMiCalendarExporter>();
                           )

#include "kcm_icalendarexporter.moc"
