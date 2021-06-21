/*
    SPDX-FileCopyrightText: 2008 Thomas Baumgart <ipwizard@users.sourceforge.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "importinterface.h"

KMyMoneyPlugin::ImportInterface::ImportInterface(QObject* parent, const char* name)
    : QObject(parent)
{
    setObjectName(name);
}

KMyMoneyPlugin::ImportInterface::~ImportInterface()
{
}
