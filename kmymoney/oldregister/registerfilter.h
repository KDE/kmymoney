/*
    SPDX-FileCopyrightText: 2006-2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef REGISTERFILTER_H
#define REGISTERFILTER_H

#include "kmm_oldregister_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ledgerfilter.h"

namespace eWidgets {
namespace eRegister {
enum class ItemState;
}
}

namespace KMyMoneyRegister
{
/**
* Used to filter items from the register.
*/
struct KMM_OLDREGISTER_EXPORT RegisterFilter {
    explicit RegisterFilter(const QString &t, LedgerFilter::State s);

    LedgerFilter::State  state;
    QString text;
};

} // namespace

#endif
