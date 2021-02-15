/***************************************************************************
                          importinterface.cpp
                             -------------------
    begin                : Mon Apr 14 2008
    copyright            : (C) 2008 Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "importinterface.h"

KMyMoneyPlugin::ImportInterface::ImportInterface(QObject* parent, const char* name) :
    QObject(parent)
{
  setObjectName(name);
}

KMyMoneyPlugin::ImportInterface::~ImportInterface()
{
}
