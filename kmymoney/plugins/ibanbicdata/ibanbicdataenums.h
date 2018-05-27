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

#ifndef IBANBICDATAENUMS_H
#define IBANBICDATAENUMS_H

#include <qnamespace.h>

namespace eIBANBIC {
  enum DataType {
    bbanLength,
    bankIdentifierPosition,
    bankIdentifierLength,
    iban2Bic,
    bankNameByBic,
    bankNameAndBic,
    extractBankIdentifier,
    isBicAllocated,
    bicModel
  };

  enum bicAllocationStatus : unsigned int {
    bicAllocated = 0,
    bicNotAllocated,
    bicAllocationUncertain
  };

  enum DisplayRole {
    InstitutionNameRole = Qt::UserRole
  };
}
#endif
