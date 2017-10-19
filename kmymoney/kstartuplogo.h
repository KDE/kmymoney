/***************************************************************************
                          kstartuplogo.h
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

#ifndef KSTARTUPLOGO_H
#define KSTARTUPLOGO_H

#include <memory>

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// Project Includes


class QSplashScreen;

std::unique_ptr<QSplashScreen> createStartupLogo();

#endif
