/*
    SPDX-FileCopyrightText: 2000 Michael Edwardes <mte@users.sourceforge.net>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
