/*
    SPDX-FileCopyrightText: 2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2023 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KMYMONEYENUMS_H
#define KMYMONEYENUMS_H

#include <QHashFunctions>

#include "qhashseedtype.h"

namespace eKMyMoney {
enum class FileAction {
    Opened,
    Saved,
    Closing,
    Closed,
    Changed,
    AboutToClose,
};

enum class StorageType {
    None,
    XML,
    SQL,
    GNC,
};

inline qHashSeedType qHash(const StorageType key, qHashSeedType seed)
{
    return ::qHash(static_cast<uint>(key), seed);
}
}
#endif
