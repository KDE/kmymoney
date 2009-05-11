/***************************************************************************
                          krecentfileitem.cpp  -  description
                             -------------------
    begin                : Wed Jul 30 2003
    copyright            : (C) 2000-2003 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// ----------------------------------------------------------------------------
// QT Includes

#include <qtooltip.h>
#include <qwidget.h>
#include <qrect.h>
#include <qpoint.h>
//Added by qt3to4:
#include <QPixmap>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes


#include "krecentfileitem.h"

KRecentFileItem::KRecentFileItem(const QString& url, Q3IconView* parent, const QString& text, const QPixmap& icon)
  : Q3IconViewItem(parent, text, icon),
    m_url(url),
    m_parent(parent)
{
  QToolTip::add(parent, rect(), url);
  // avoid moving this item around
  setDragEnabled(false);
}

KRecentFileItem::~KRecentFileItem()
{
}

bool KRecentFileItem::move( int x, int y )
{
  QRect r = rect();
  QToolTip::remove(m_parent, rect());
  r.moveTopLeft(QPoint(x,y));
  QToolTip::add(m_parent, r, m_url);
  return Q3IconViewItem::move(x,y);
}

