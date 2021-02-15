/***************************************************************************
                          statementinterface.cpp
                             -------------------
    begin                : Wed Jan 5 2005
    copyright            : (C) 2005 Thomas Baumgart
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

#include "statementinterface.h"

KMyMoneyPlugin::StatementInterface::StatementInterface(QObject* parent, const char* name) :
    QObject(parent)
{
  setObjectName(name);
}

KMyMoneyPlugin::StatementInterface::~StatementInterface()
{
}
