/*
    SPDX-FileCopyrightText: 2005 Thomas Baumgart <ipwizard@users.sourceforge.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
