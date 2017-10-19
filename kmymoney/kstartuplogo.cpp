/***************************************************************************
                          kstartuplogo.cpp
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes <mte@users.sourceforge.net>
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

#include "kstartuplogo.h"


// ----------------------------------------------------------------------------
// QT Includes

#include <QPainter>
#include <QCoreApplication>
#include <QSplashScreen>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KColorScheme>
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes


std::unique_ptr<QSplashScreen> createStartupLogo()
{
  KColorScheme scheme(QPalette::Active, KColorScheme::Selection);
  QPixmap logoOverlay(KLocalizedString::localizedFilePath(
                        QStandardPaths::locate(QStandardPaths::DataLocation,
                                               QStringLiteral("pics/startlogo.png"))));
  QPixmap logoPixmap(logoOverlay.size());
  logoPixmap.fill(scheme.background(KColorScheme::NormalBackground).color());
  QPainter pixmapPainter(&logoPixmap);
  pixmapPainter.drawPixmap(0, 0, logoOverlay, 0, 0, logoOverlay.width(), logoOverlay.height());
  std::unique_ptr<QSplashScreen> splash(new QSplashScreen(logoPixmap, Qt::WindowStaysOnTopHint));
  splash->showMessage(i18n("Loading %1...", QCoreApplication::applicationVersion()),  //krazy:exclude=qmethods
                      Qt::AlignLeft | Qt::AlignBottom,
                      scheme.foreground(KColorScheme::NormalText).color());
  splash->show();
  return splash;
}
