/*
    SPDX-FileCopyrightText: 2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
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
