/*
    SPDX-FileCopyrightText: 2000 Michael Edwardes <mte@users.sourceforge.net>
    SPDX-FileCopyrightText: 2007 Thomas Baumgart <ipwizard@users.sourceforge.net>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kcreditswindow.h"
#include <config-kmymoney.h>

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

#include <alkimia/alkversion.h>

// ----------------------------------------------------------------------------
// Project Includes

KAboutData initializeCreditsData()
{
    KAboutData aboutData(KAboutData::applicationData());
    if (!aboutData.homepage().isEmpty())
        return aboutData;
    QStringList features;
#ifdef ENABLE_GPG
    features << i18n("GPG encryption");
#endif
#ifdef ENABLE_ADDRESSBOOK
    features << i18n("Addressbook integration");
#endif
#ifdef ENABLE_HOLIDAYS
    features << i18n("Holiday regions integration");
#endif
#ifdef ENABLE_KBANKING
    features << i18n("Online banking");
#endif

    aboutData.setShortDescription(i18n("\nKMyMoney, the Personal Finance Manager by KDE.\n\nPlease consider contributing to this project with code and/or suggestions."));
    aboutData.setLicense(KAboutLicense::GPL);
    aboutData.setCopyrightStatement(i18n("(c) 2000-2025 The KMyMoney development team"));
    aboutData.setHomepage(QStringLiteral("https://kmymoney.org/"));
    if (!features.empty()) {
        std::sort(features.begin(), features.end());
        aboutData.setOtherText(i18n("Compiled with the following optional features:\n%1", features.join(QLatin1Char('\n'))));
    }

    aboutData.addAuthor(QLatin1String("Thomas Baumgart"), i18nc("Roles in project", "Core engine, Release Manager, Project admin"), "tbaumgart@kde.org");
    aboutData.addAuthor(QLatin1String("Ralf Habacker"), i18nc("Roles in project", "Developer, Maintainer, Windows support"), "ralf.habacker@freenet.de");
    aboutData.addAuthor(QString::fromUtf8("Dawid Wróbel"), i18nc("Roles in project", "Developer, Windows and MacOS support"), "me@dawidwrobel.com");

    aboutData.addAuthor(QLatin1String("Michael Edwardes"),
                        i18n("Inactive member. ") + i18nc("Roles in project", "Initial idea, much initial source code, Project admin"),
                        "mte@users.sourceforge.net");
    aboutData.addAuthor(QLatin1String("Alvaro Soliverez"), i18n("Inactive member. ") + i18nc("Roles in project", "Forecast, Reports"), "asoliverez@gmail.com");
    aboutData.addAuthor(QLatin1String("Ace Jones"),
                        i18n("Inactive member. ") + i18nc("Roles in project", "Reporting logic, OFX Import"),
                        "acejones@users.sourceforge.net");
    aboutData.addAuthor(QLatin1String("Tony Bloomfield"),
                        i18n("Inactive member. ") + i18nc("Roles in project", "Database backend, maintainer stable branch"),
                        "tonybloom@users.sourceforge.net");
    aboutData.addAuthor(QLatin1String("Felix Rodriguez"),
                        i18n("Inactive member. ") + i18nc("Roles in project", "Project Admin"),
                        "frodriguez@users.sourceforge.net");
    aboutData.addAuthor(QLatin1String("John C"), i18n("Inactive member. ") + i18nc("Roles in project", "Developer"), "tacoturtle@users.sourceforge.net");
    aboutData.addAuthor(QLatin1String("Fernando Vilas"), i18n("Inactive member. ") + i18nc("Roles in project", "Database backend"), "fvilas@iname.com");
    aboutData.addAuthor(QString::fromUtf8("Cristian Oneț"), i18n("Inactive member. ") + i18nc("Roles in project", "Developer"), "onet.cristian@gmail.com");
    aboutData.addAuthor(QString::fromUtf8("Christian Dávid"), i18n("Inactive member. ") + i18nc("Roles in project", "Developer"), "christian-david@web.de");
    aboutData.addAuthor(QString::fromUtf8("Łukasz Wojniłowicz"),
                        i18n("Inactive member. ") + i18nc("Roles in project", "Developer, Code refactoring"),
                        "lukasz.wojnilowicz@gmail.com");

#ifdef ENABLE_ADDRESSBOOK
    aboutData.addComponent(QString::fromUtf8("Akonadi"),
                           i18n("Addressbook integration"),
                           i18n("%1 (build against %2)").arg(i18n("Unknown"), ENABLE_ADDRESSBOOK_VERSION),
                           "https://userbase.kde.org/Akonadi");
#endif

    aboutData.addComponent(QString::fromUtf8("Alkimia"),
                           i18n("Financial framework"),
                           i18n("%1 (build against %2)").arg(alkVersionString(), ALK_VERSION_STRING),
                           "https://community.kde.org/Alkimia");

#ifdef ENABLE_KBANKING
    aboutData.addComponent(QString::fromUtf8("Aqbanking"),
                           i18n("Online banking"),
                           i18n("%1 (build against %2)").arg(i18n("Unknown"), ENABLE_AQBANKING_VERSION),
                           "https://www.aquamaniac.de/rdm/projects/aqbanking");
#endif

#ifdef ENABLE_GPG
    aboutData.addComponent(QString::fromUtf8("Gpgme"),
                           i18n("GPG encryption"),
                           i18n("%1 (build against %2)").arg(i18n("Unknown"), ENABLE_GPG_VERSION),
                           "https://gnupg.org/software/gpgme/index.html");
#endif

#ifdef ENABLE_KBANKING
    aboutData.addComponent(QString::fromUtf8("Gwenhywfar"),
                           i18n("Online banking framework"),
                           i18n("%1 (build against %2)").arg(i18n("Unknown"), ENABLE_GWENHYWFAR_VERSION),
                           "https://www.aquamaniac.de/rdm/projects/gwenhywfar");
#endif

    aboutData.addComponent(QString::fromUtf8("KDiagram"),
                           i18n("Library for creating diagrams"),
                           i18n("%1 (build against %2)").arg(i18n("Unknown"), ENABLE_KCHART_VERSION));

#ifdef ENABLE_LIBICAL
    aboutData.addComponent(QString::fromUtf8("libical"),
                           i18n("iCalendar integration"),
                           i18n("%1 (build against %2)").arg(i18n("Unknown"), ENABLE_LIBICAL_VERSION),
                           "https://github.com/libical/libical");
#endif

#ifdef ENABLE_LIBOFX
    aboutData.addComponent(QString::fromUtf8("libofx"),
                           i18n("OFX banking protocol abstraction library"),
                           i18n("%1 (build against %2)").arg(i18n("Unknown"), ENABLE_LIBOFX_VERSION),
                           "https://github.com/libofx/libofx");
#endif

#ifdef ENABLE_SQLCIPHER
    aboutData.addComponent(QString::fromUtf8("SQLCipher"),
                           i18n("SQLite database encryption"),
                           i18n("%1 (build against %2)").arg(i18n("Unknown"), ENABLE_SQLCIPHER_VERSION),
                           "https://www.zetetic.net/sqlcipher");
#endif

    aboutData.addCredit(QLatin1String("Jack Ostroff"), i18nc("Roles in project", "Documentation and user support"), "ostroffjh@users.sourceforge.net");
    aboutData.addCredit(QLatin1String("Kevin Tambascio"),
                        i18n("Inactive member. ") + i18nc("Roles in project", "Initial investment support"),
                        "ktambascio@users.sourceforge.net");
    aboutData.addCredit(QLatin1String("Javier Campos Morales"),
                        i18n("Inactive member. ") + i18nc("Roles in project", "Developer & Artist"),
                        "javi_c@users.sourceforge.net");
    aboutData.addCredit(QLatin1String("Robert Wadley"),
                        i18n("Inactive member. ") + i18nc("Roles in project", "Icons & splash screen"),
                        "rob@robntina.fastmail.us");
    aboutData.addCredit(QLatin1String("Laurent Montel"), i18nc("Roles in project", "Patches and port to kde4"), "montel@kde.org");
    aboutData.addCredit(QLatin1String("Wolfgang Rohdewald"), i18n("Inactive member. ") + i18nc("Roles in project", "Patches"), "woro@users.sourceforge.net");
    aboutData.addCredit(QString::fromUtf8("Marko Käning"),
                        i18n("Inactive member. ") + i18nc("Roles in project", "Patches, packaging and KF5-CI for OS X"),
                        "mk-lists@email.de");
    aboutData.addCredit(QString::fromUtf8("Allan Anderson ✝"), i18nc("Roles in project", "CSV import/export"), QString());
    return aboutData;
}
