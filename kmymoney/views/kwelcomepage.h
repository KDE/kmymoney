/***************************************************************************
                          kwelcomepage.h  -  description
                             -------------------
    begin                : Sat Sep 5 2009
    copyright            : (C) 2009 by Alvaro Soliverez <asoliverez@gmail.com>

***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   SPDX-License-Identifier: GPL-2.0-or-later
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
