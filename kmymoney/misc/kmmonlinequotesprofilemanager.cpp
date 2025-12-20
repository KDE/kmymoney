/*
    SPDX-FileCopyrightText: 2023 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kmmonlinequotesprofilemanager.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDebug>
#include <QString>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLazyLocalizedString>
#include <KLocalizedString>
#include <KSharedConfig>

// ----------------------------------------------------------------------------
// Project Includes

#include <alkimia/alkonlinequotesprofile.h>

// create the quoteprofile and make sure it uses our idea of the configuration
static struct OnlineProfileConfig {
    const char* name;
    const char* ghnsName;
    AlkOnlineQuotesProfile::Type type;
    bool checkSupport;
    KLazyLocalizedString installedTooltip;
    KLazyLocalizedString functionalTooltip;
} onlineProfileList[] = {{
                             "kmymoney5",
                             "kmymoney-quotes.knsrc",
                             AlkOnlineQuotesProfile::Type::KMyMoney5,
                             false,
                             KLazyLocalizedString(),
                             KLazyLocalizedString(),
                         },
                         {
                             "Finance::Quote",
                             "",
                             AlkOnlineQuotesProfile::Type::Script,
                             true,
                             kli18nc("@info:tooltip", "Finance::Quote not supported by the installed Alkimia library."),
                             kli18nc("@info:tooltip", "Missing or non-functioning Finance::Quote installation."),
                         }};

static QMap<QString, QString> tooltips;

struct AddProfileResult {
    AlkOnlineQuotesProfile* profile = nullptr;
    QString tooltip;
};

static AddProfileResult addProfile(const QString& name)
{
    AlkOnlineQuotesProfileManager& manager = AlkOnlineQuotesProfileManager::instance();
    AddProfileResult result;

    for (const auto& onlineProfile : onlineProfileList) {
        const QString profileName(onlineProfile.name);
        if (name == profileName) {
            result.profile = manager.profile(profileName);

            // if it does not exist, we need to add it.
            if (!result.profile) {
                auto disableProfile = [&](const KLazyLocalizedString& tooltip) {
                    delete result.profile;
                    result.profile = nullptr;
                    result.tooltip = tooltip.toString();
                };

                result.profile = new AlkOnlineQuotesProfile(profileName, onlineProfile.type, onlineProfile.ghnsName);

                if (!result.profile->typeIsSupported()) {
                    disableProfile(onlineProfile.installedTooltip);
                }
                if (result.profile) {
                    if (!result.profile->typeIsOperational()) {
                        disableProfile(onlineProfile.functionalTooltip);
                    }
                }
            }
            break; // leave for loop, we're done
        }
    }
    return result;
}
KMMOnlineQuotesProfileManager& KMMOnlineQuotesProfileManager::instance()
{
    static bool initialized(false);
    // load the available online profiles
    KMMOnlineQuotesProfileManager& manager = *(reinterpret_cast<KMMOnlineQuotesProfileManager*>(&AlkOnlineQuotesProfileManager::instance()));

    if (!initialized) {
        initialized = true;
        for (const auto& onlineProfile : onlineProfileList) {
            const QString profileName(onlineProfile.name);
            auto quoteProfile = manager.profile(profileName);
            if (!quoteProfile) {
                // create the quoteprofile and make sure it uses our idea of the configuration
                auto result = ::addProfile(profileName);
                quoteProfile = result.profile;
                tooltips.insert(profileName, result.tooltip);

                if (quoteProfile) {
                    // add profile to manager
                    quoteProfile->setKConfig(KSharedConfig::openConfig());
                    manager.addProfile(quoteProfile);
                }
            }
        }
    }
    return manager;
}

QString KMMOnlineQuotesProfileManager::availabilityHint(const QString& profileName)
{
    if (tooltips.contains(profileName)) {
        return tooltips.value(profileName);
    }
    return i18nc("@info:tooltip", "Online quote source with name '%1' unknown.", profileName);
}
