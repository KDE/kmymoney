/***************************************************************************
                          kwelcomepage.h  -  description
                             -------------------
    begin                : Sat Sep 5 2009
    copyright            : (C) 2009 by Alvaro Soliverez <asoliverez@gmail.com>

***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef KWELCOMEPAGE_H
#define KWELCOMEPAGE_H

// ----------------------------------------------------------------------------
// QT Includes
#include <QString>
#include <QStringList>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes


/**
  * Generates welcome page
  *
  * @author Alvaro Soliverez
  *
  * @short Generates the welcome page
**/
class KWelcomePage
{
public:

  KWelcomePage();
  ~KWelcomePage();

  static const QString welcomePage();

  static const QString whatsNewPage();

protected:

  static const QStringList featuresList();
  static bool isGroupHeader(const QString& item);
  static bool isGroupItem(const QString& item);
};

#endif
