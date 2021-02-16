/*
    SPDX-FileCopyrightText: 2016-2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ledgeritem.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

LedgerItem::LedgerItem()
{
}

LedgerItem::~LedgerItem()
{
}

QString LedgerItem::accountId() const
{
  return QString();
}
