/*
    SPDX-FileCopyrightText: 2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
