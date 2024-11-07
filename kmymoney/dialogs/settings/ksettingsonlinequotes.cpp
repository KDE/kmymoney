/*
    SPDX-FileCopyrightText: 2023 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ksettingsonlinequotes.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QCheckBox>
#include <QIcon>
#include <QLayout>
#include <QPushButton>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KConfig>
#include <KLocalizedString>
#include <KMessageBox>
#include <KSharedConfig>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_ksettingsonlinequotes.h"

#include <alkimia/alkonlinequotesource.h>
#include <alkimia/alkonlinequotesprofile.h>
#include <alkimia/alkonlinequoteswidget.h>

#include "icons.h"
#include "kmmonlinequotesprofilemanager.h"
#include "mymoneyfile.h"
#include "mymoneysecurity.h"

#include "kmmyesno.h"

using namespace Icons;

class KSettingsOnlineQuotesPrivate
{
    Q_DISABLE_COPY(KSettingsOnlineQuotesPrivate)

public:
    KSettingsOnlineQuotesPrivate()
        : m_quotesWidgetContainer(nullptr)
    {
    }

    ~KSettingsOnlineQuotesPrivate()
    {
        delete m_quotesWidgetContainer;
    }

    AlkOnlineQuotesWidget* m_quotesWidgetContainer;
    QWidget* m_quotesWidget;
    QWidget* m_detailsWidget;
    QPushButton* m_acceptButton;
};

KSettingsOnlineQuotes::KSettingsOnlineQuotes(QWidget *parent) :
    QWidget(parent),
    d_ptr(new KSettingsOnlineQuotesPrivate)
{
    Q_D(KSettingsOnlineQuotes);

    // initialize if it does not yet exist
    KMMOnlineQuotesProfileManager::instance();

    d->m_quotesWidgetContainer = new AlkOnlineQuotesWidget;

    auto layout = new QVBoxLayout(this);
    d->m_quotesWidget = d->m_quotesWidgetContainer->onlineQuotesWidget();
    d->m_detailsWidget = d->m_quotesWidgetContainer->quoteDetailsWidget();
    layout->addWidget(d->m_quotesWidget);
    layout->addWidget(d->m_detailsWidget);
    d->m_acceptButton = d->m_detailsWidget->findChild<QPushButton*>();
    connect(d->m_acceptButton, &QPushButton::clicked, this, [&]() {
        Q_EMIT settingsChanged(true);
    });
    setLayout(layout);
}

KSettingsOnlineQuotes::~KSettingsOnlineQuotes()
{
    Q_D(KSettingsOnlineQuotes);
    delete d;
}

void KSettingsOnlineQuotes::saveSettings()
{
    Q_D(KSettingsOnlineQuotes);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    d->m_acceptButton->animateClick(1); // a very short click (1ms)
#else
    d->m_acceptButton->animateClick();
#endif
}

void KSettingsOnlineQuotes::resetSettings()
{
    Q_D(KSettingsOnlineQuotes);
    d->m_quotesWidgetContainer->resetConfig();
}

#if 0
void KSettingsOnlineQuotes::slotDumpCSVProfile()
{
    // Not sure what this was used for, but it is a no-go
    // to base functions in the core system on code that
    // is provided by a plugin. I leave the implementation
    // here for reference in case this needs to be ported.
    //
    // ipwizard 2023-09-16

    Q_D(KSettingsOnlineQuotes);
    KSharedConfigPtr config = CSVImporterCore::configFile();
    PricesProfile profile;
    profile.m_profileName = d->m_currentItem.m_name;
    profile.m_profileType = Profile::StockPrices;
    bool profileExists = false;
    bool writeProfile = true;

    if (profile.readSettings(config))
        profileExists = true;
    else {
        profile.m_profileType = Profile::CurrencyPrices;
        if (profile.readSettings(config))
            profileExists = true;
    }

    if (profileExists)
        writeProfile = (KMessageBox::questionTwoActionsCancel(this,
                                                              i18n("CSV profile <b>%1</b> already exists.<br>"
                                                                   "Do you want to overwrite it?",
                                                                   d->m_currentItem.m_name),
                                                              i18n("CSV Profile Already Exists"),
                                                              KMMYesNo::yes(),
                                                              KMMYesNo::no())
                                == KMessageBox::PrimaryAction
                            ? true
                            : false);

    if (writeProfile) {
        QMap<QString, PricesProfile> quoteSources = WebPriceQuote::defaultCSVQuoteSources();
        profile = quoteSources.value(d->m_currentItem.m_name);
        if (profile.m_profileName.compare(d->m_currentItem.m_name, Qt::CaseInsensitive) == 0) {
            profile.writeSettings(config);
            CSVImporterCore::profilesAction(profile.type(), ProfileAction::Add, profile.m_profileName, profile.m_profileName);
        }
    }
    CSVImporterCore::profilesAction(profile.type(), ProfileAction::UpdateLastUsed, profile.m_profileName, profile.m_profileName);
}
#endif
