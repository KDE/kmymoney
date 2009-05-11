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

#include <kdecompat.h>

// ----------------------------------------------------------------------------
// QT Includes

#include <qapplication.h>
#include <qpixmap.h>
#include <q3frame.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kglobal.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kapplication.h>

#if KDE_IS_VERSION(3,2,0)
#include <ksplashscreen.h>
#endif

// ----------------------------------------------------------------------------
// Project Includes

#include "kstartuplogo.h"
#include "kmymoneyglobalsettings.h"

KStartupLogo::KStartupLogo() :
  QObject(0, 0),
  m_splash(0)
{
  // splash screen setting
  if(!KMyMoneyGlobalSettings::showSplash())
    return;

  QString filename = KGlobal::dirs()->findResource("appdata", "pics/startlogo.png");
  QPixmap pm(filename);

  if(!pm.isNull()) {
#if KDE_IS_VERSION(3,2,0)
    KSplashScreen* splash = new KSplashScreen(pm);
    splash->setFixedSize(pm.size());

#else
    Q3Frame* splash = new Q3Frame(0, 0, Q3Frame::WStyle_NoBorder | Q3Frame::WStyle_StaysOnTop | Q3Frame::WStyle_Tool | Q3Frame::WWinOwnDC | Q3Frame::WStyle_Customize);
    splash->setBackgroundPixmap(pm);
    splash->setFrameShape( Q3Frame::StyledPanel );
    splash->setFrameShadow( Q3Frame::Raised );
    splash->setLineWidth( 2 );
    splash->setGeometry( QRect( (QApplication::desktop()->width()/2)-(pm.width()/2), (QApplication::desktop()->height()/2)-(pm.height()/2), pm.width(), pm.height() ) );

#endif

    splash->show();
    m_splash = splash;
  }
}

KStartupLogo::~KStartupLogo()
{
    delete m_splash;
}

#include "kstartuplogo.moc"
