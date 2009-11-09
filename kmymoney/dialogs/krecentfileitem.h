/***************************************************************************
                          krecentfileitem.h  -  description
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
#ifndef KRECENTFILEITEM_H
#define KRECENTFILEITEM_H

// ----------------------------------------------------------------------------
// QT Includes
#include <qwidget.h>
#include <q3iconview.h>
//Added by qt3to4:
#include <QPixmap>

// ----------------------------------------------------------------------------
// KDE Includes
#include <kurl.h>

// ----------------------------------------------------------------------------
// Project Includes



/**
  * @author Michael Edwardes
  */

class KRecentFileItem : public Q3IconViewItem  {
public: 
  KRecentFileItem(const QString& url, Q3IconView* parent, const QString& text, const QPixmap& icon);
  ~KRecentFileItem();
  QString fileURL(void) const { return m_url; }

  bool move( int x, int y );
  
private:
  QString m_url;
  QWidget* m_parent;
};

#endif
