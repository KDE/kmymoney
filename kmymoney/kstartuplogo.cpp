/***************************************************************************
                          kstartuplogo.cpp
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes
    email                : mte@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kstartuplogo.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QPixmap>
#include <QPainter>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kglobal.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kcolorscheme.h>
#include <ksplashscreen.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <k4aboutdata.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyglobalsettings.h"

KStartupLogo::KStartupLogo() :
    QObject(0),
    m_splash(0)
{
  // splash screen setting
  if (!KMyMoneyGlobalSettings::showSplash())
    return;

  QString filename = KGlobal::dirs()->findResource("appdata", "pics/startlogo.png");
  // TODO: port KF5
  QString localeFilename = filename;//KLocale::global()->localizedFilePath(filename);
  QPixmap logoOverlay(localeFilename);

  QPixmap logoPixmap(logoOverlay.size());
  logoPixmap.fill(KColorScheme(QPalette::Active, KColorScheme::Selection).background(KColorScheme::NormalBackground).color());
  QPainter pixmapPainter(&logoPixmap);
  pixmapPainter.drawPixmap(0, 0, logoOverlay, 0, 0, logoOverlay.width(), logoOverlay.height());

  if (!logoOverlay.isNull()) {
    const K4AboutData *aboutData = KCmdLineArgs::aboutData();
    KSplashScreen* splash = new KSplashScreen(logoPixmap);
    splash->setFixedSize(logoPixmap.size());

    splash->show();
    splash->showMessage(i18n("Loading %1...", aboutData->version()),  //krazy:exclude=qmethods
                        Qt::AlignLeft | Qt::AlignBottom,
                        KColorScheme(QPalette::Active, KColorScheme::Selection)
                        .foreground(KColorScheme::NormalText).color());
    m_splash = splash;
  }
}

KStartupLogo::~KStartupLogo()
{
  delete m_splash;
}

