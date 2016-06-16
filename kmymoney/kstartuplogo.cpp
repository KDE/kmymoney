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
#include <QStandardPaths>
#include <QSplashScreen>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kcolorscheme.h>
#include <KAboutData>
#include <KLocalizedString>

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

  QString filename = QStandardPaths::locate(QStandardPaths::DataLocation, "pics/startlogo.png");
  // TODO: port KF5
  QString localeFilename = filename;//KLocale::global()->localizedFilePath(filename);
  QPixmap logoOverlay(localeFilename);

  QPixmap logoPixmap(logoOverlay.size());
  logoPixmap.fill(KColorScheme(QPalette::Active, KColorScheme::Selection).background(KColorScheme::NormalBackground).color());
  QPainter pixmapPainter(&logoPixmap);
  pixmapPainter.drawPixmap(0, 0, logoOverlay, 0, 0, logoOverlay.width(), logoOverlay.height());

  if (!logoOverlay.isNull()) {
    QSplashScreen* splash = new QSplashScreen(logoPixmap, Qt::WindowStaysOnTopHint);
    splash->setFixedSize(logoPixmap.size());

    splash->show();
    splash->showMessage(i18n("Loading %1...", KAboutData::applicationData().version()),  //krazy:exclude=qmethods
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

