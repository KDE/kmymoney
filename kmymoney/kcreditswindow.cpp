/***************************************************************************
                          kcreditswindow.cpp
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes <mte@users.sourceforge.net>
                           (C) 2007 by Thomas Baumgart <ipwizard@users.sourceforge.net>
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kcreditswindow.h"
#include <config-kmymoney.h>

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

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

  aboutData.setShortDescription(i18n("\nKMyMoney, the Personal Finance Manager by KDE.\n\nPlease consider contributing to this project with code and/or suggestions."));
  aboutData.setLicense(KAboutLicense::GPL);
  aboutData.setCopyrightStatement(i18n("(c) 2000-2019 The KMyMoney development team"));
  aboutData.setHomepage(QStringLiteral("https://kmymoney.org/"));
  if (!features.empty())
    aboutData.setOtherText(i18n("Compiled with the following optional features:\n%1", features.join(QLatin1Char('\n'))));

  aboutData.addAuthor(i18n("Thomas Baumgart"),    i18n("Core engine, Release Manager, Project admin"), "ipwizard@users.sourceforge.net");
  aboutData.addAuthor(i18n("Łukasz Wojniłowicz"), i18n("Developer"), "lukasz.wojnilowicz@gmail.com");
  aboutData.addAuthor(i18n("Ralf Habacker"),      i18n("Developer"), "ralf.habacker@freenet.de");
  aboutData.addAuthor(i18n("Cristian Oneț"),      i18n("Developer"), "onet.cristian@gmail.com");
  aboutData.addAuthor(i18n("Christian Dávid"),    i18n("Developer"), "christian-david@web.de");

  aboutData.addAuthor(i18n("Michael Edwardes"), i18n("Inactive member. ") + i18n("Initial idea, much initial source code, Project admin"), "mte@users.sourceforge.net");
  aboutData.addAuthor(i18n("Alvaro Soliverez"), i18n("Inactive member. ") + i18n("Forecast, Reports"), "asoliverez@gmail.com");
  aboutData.addAuthor(i18n("Ace Jones"),        i18n("Inactive member. ") + i18n("Reporting logic, OFX Import"), "acejones@users.sourceforge.net");
  aboutData.addAuthor(i18n("Tony Bloomfield"),  i18n("Inactive member. ") + i18n("Database backend, maintainer stable branch"), "tonybloom@users.sourceforge.net");
  aboutData.addAuthor(i18n("Felix Rodriguez"),  i18n("Inactive member. ") + i18n("Project Admin"), "frodriguez@users.sourceforge.net");
  aboutData.addAuthor(i18n("John C"),           i18n("Inactive member. ") + i18n("Developer"), "tacoturtle@users.sourceforge.net");
  aboutData.addAuthor(i18n("Fernando Vilas"),   i18n("Inactive member. ") + i18n("Database backend"), "fvilas@iname.com");

  aboutData.addCredit(i18n("Jack Ostroff"),           i18n("Documentation and user support"), "ostroffjh@users.sourceforge.net");
  aboutData.addCredit(i18n("Kevin Tambascio"),        i18n("Initial investment support"), "ktambascio@users.sourceforge.net");
  aboutData.addCredit(i18n("Javier Campos Morales"),  i18n("Developer & Artist"), "javi_c@users.sourceforge.net");
  aboutData.addCredit(i18n("Robert Wadley"),          i18n("Icons & splash screen"), "rob@robntina.fastmail.us");
  aboutData.addCredit(i18n("Laurent Montel"),         i18n("Patches and port to kde4"), "montel@kde.org");
  aboutData.addCredit(i18n("Wolfgang Rohdewald"),     i18n("Patches"), "woro@users.sourceforge.net");
  aboutData.addCredit(i18n("Marko Käning"),           i18n("Patches, packaging and KF5-CI for OS X"), "mk-lists@email.de");
  aboutData.addCredit(i18n("Allan Anderson ✝"),       i18n("CSV import/export"), QString());
  return aboutData;
}
