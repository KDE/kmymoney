/*
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef STORAGEENUMS_H
#define STORAGEENUMS_H

#include <QHashFunctions>

#include "qhashseedtype.h"

namespace eStorage {
enum class Reference {
    Account = 0,
    Institution,
    Payee,
    Transaction,
    Report,
    Budget,
    Schedule,
    Security,
    Currency,
    Price,
    Tag,
    CostCenter,
    // insert new entries above this line
    Count,
};

inline qHashSeedType qHash(const Reference key, qHashSeedType seed)
{
    return ::qHash(static_cast<uint>(key), seed);
}
}
#endif
