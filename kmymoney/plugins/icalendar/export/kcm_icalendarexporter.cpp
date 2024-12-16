/*
    SPDX-FileCopyrightText: 2009 Cristian Onet onet.cristian @gmail.com
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "kcm_icalendarexporter.h"
#include <config-kmymoney-version.h>

// Qt includes
#include <QComboBox>
#include <QBoxLayout>

// KDE includes
#include <KAboutData>
#include <KLocalizedString>
#include <KPluginFactory>

#include "icalendarsettings.h"
#include "ui_icalendarsettingsdecl.h"

class PluginSettingsWidget : public QWidget, public Ui::ICalendarSettingsDecl
{
public:
    explicit PluginSettingsWidget(QWidget* parent = 0)
        : QWidget(parent)
    {
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

KCMiCalendarExporter::KCMiCalendarExporter(QObject* parent, const QVariantList& args)
    : KMMKCModule(parent, args)
{
    PluginSettingsWidget* w = new PluginSettingsWidget(widget());
    addConfig(ICalendarSettings::self(), w);
    QVBoxLayout *layout = new QVBoxLayout;
    widget()->setLayout(layout);
    layout->addWidget(w);
    load();
}

KCMiCalendarExporter::~KCMiCalendarExporter()
{
}

K_PLUGIN_CLASS(KCMiCalendarExporter)

#include "kcm_icalendarexporter.moc"
