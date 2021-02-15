/*
 * SPDX-FileCopyrightText: 2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KMYMONEYENUMS_H
#define KMYMONEYENUMS_H

#include <QHashFunctions>

namespace eKMyMoney {
  enum class FileAction {
    Opened,
    Saved,
    Closing,
    Closed,
    Changed
  };

  enum class StorageType {
    None,
    XML,
    SQL,
    GNC
  };

  inline uint qHash(const StorageType key, uint seed) { return ::qHash(static_cast<uint>(key), seed); }

}
#endif
