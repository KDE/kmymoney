/*
 * Copyright 2018       Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
