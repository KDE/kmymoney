/***************************************************************************
                          appinterface.cpp
                             -------------------
    copyright            : (C) 2018 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "appinterface.h"

KMyMoneyPlugin::AppInterface::AppInterface(QObject* parent, const char* name) :
    QObject(parent)
{
  setObjectName(name);
}

KMyMoneyPlugin::AppInterface::~AppInterface()
{
}
