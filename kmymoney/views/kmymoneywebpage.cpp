/***************************************************************************
                          kmymoneywebpage.cpp
                             -------------------
        copyright            : (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <config-kmymoney.h>

#include "kmymoneywebpage.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

bool MyQWebEnginePage::acceptNavigationRequest(const QUrl &url, NavigationType type, bool)
{
  if (type == NavigationTypeLinkClicked)
    #ifdef ENABLE_WEBENGINE
    emit urlChanged(url);
    #else
    emit linkClicked(url);
    #endif
  return false;
}
