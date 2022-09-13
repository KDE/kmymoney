/*
    SPDX-FileCopyrightText: 2016-2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2021 Dawid Wróbel <me@dawidwrobel.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kcm_csvimporter.h"
#include <config-kmymoney-version.h>

// ----------------------------------------------------------------------------
// QT Includes

#include <QTimer>
#include <QWidget>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KAboutData>
#include <KCModule>
#include <KPluginFactory>

// ----------------------------------------------------------------------------
// Project Includes

#include "pluginsettings.h"
#include "ui_pluginsettingsdecl.h"

class KCMCSVImporterPrivate : public QObject
{
    Q_OBJECT
public:
    KCMCSVImporterPrivate()
        : ui(new Ui::PluginSettingsDecl)
        , emptyProfileListText(i18nc("@item:inlistbox No QIF profile defined", "No profile setup"))
    {
    }

    void setSelectedProfile(int idx)
    {
        if (idx >= 0 && idx < ui->qifProfileCombo->count()) {
            auto selectedProfile = ui->qifProfileCombo->itemText(idx);
            if (selectedProfile == emptyProfileListText) {
                selectedProfile.clear();
            }
            ui->kcfg_QifExportProfile->setText(selectedProfile);
        }
    }

    void loadProfiles()
    {
        // only need to do this once
        disconnect(ui->kcfg_QifExportProfile, &QLineEdit::textChanged, this, &KCMCSVImporterPrivate::loadProfiles);

        auto text = ui->kcfg_QifExportProfile->text();

        // load the profiles from the configuration into the combobox
        QStringList profileList;
        KSharedConfigPtr config = KSharedConfig::openConfig();
        KConfigGroup grp = config->group("Profiles");
        profileList = grp.readEntry("profiles", QStringList());

        if (profileList.isEmpty()) {
            profileList.append(emptyProfileListText);
            ui->qifProfileCombo->setToolTip(
                i18nc("@info:tooltip No QIF profile defined", "Please use the QIF profile editor in the QIF plugin settings to create one."));
        }
        profileList.sort();

        // prevent the combobox to send out signals
        // during the initialization phase
        QSignalBlocker block(ui->qifProfileCombo);

        ui->qifProfileCombo->addItems(profileList);
        ui->qifProfileCombo->setCurrentIndex(-1);

        auto idx = profileList.indexOf(text);
        if (idx == -1) {
            idx = 0;
        }
        ui->qifProfileCombo->setCurrentIndex(idx);
    }

    Ui::PluginSettingsDecl* ui;
    QString emptyProfileListText;
};

KCMCSVImporter::KCMCSVImporter(QWidget* parent, const QVariantList& args)
    : KCModule(parent, args)
    , d_ptr(new KCMCSVImporterPrivate)
{
    Q_D(KCMCSVImporter);

    d->ui->setupUi(this);

    // don't show the text box we need for the kcfg automatism of the profile
    d->ui->kcfg_QifExportProfile->hide();

    // initially update the combobox when the hidden text field is set
    connect(d->ui->kcfg_QifExportProfile, &QLineEdit::textChanged, d, &KCMCSVImporterPrivate::loadProfiles);

    addConfig(PluginSettings::self(), this);
    setButtons(NoAdditionalButton);

    // make sure to keep track of the user's selection in the hidden field
    connect(d->ui->qifProfileCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [&](int idx) {
        Q_D(KCMCSVImporter);
        d->setSelectedProfile(idx);
    });

    // once everything is set up, copy the combobox content to the hidden field
    QTimer::singleShot(50, this, [&]() {
        Q_D(KCMCSVImporter);
        // Check if the combo box has been loaded. There must
        // be at least one entry in the list. If none is present,
        // we simply call loadProfile() to fix that.
        if (d->ui->qifProfileCombo->count() == 0) {
            d->loadProfiles();
        }
        d->setSelectedProfile(d->ui->qifProfileCombo->currentIndex());
    });
}

KCMCSVImporter::~KCMCSVImporter()
{
    Q_D(KCMCSVImporter);
    delete d;
}

K_PLUGIN_CLASS(KCMCSVImporter)

#include "kcm_csvimporter.moc"
