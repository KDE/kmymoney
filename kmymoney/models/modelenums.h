/*
 * Copyright 2017-2018  Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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

#ifndef MODELENUMS_H
#define MODELENUMS_H

#include <QHashFunctions>
#include <qnamespace.h>
#include <QMetaType>

/// @todo cleanup
#if 0
namespace eAccountsModel {
    enum class Column {
        FirstColumnMarker = 0,
        Account = 0,  // CAUTION! Assumption is being made that Account column number is always 0 and you must not change this
        Type,
        Tax,
        VAT,
        CostCenter,
        TotalBalance,
        PostedValue,
        TotalValue,
        AccountNumber,
        AccountBankCode,
        LastColumnMarker
    };

    inline uint qHash(const Column key, uint seed) { return ::qHash(static_cast<uint>(key), seed); }

    enum class Role {
        ID = Qt::UserRole,                     /**< The account id is stored in this role in column 0 as a string.*/
        Favorite = Qt::UserRole + 1,           /**< The 'account is favorite' property is stored in this role in column 0 as a bool.*/
        Account = Qt::UserRole + 2,            /**< The MyMoneyAccount is stored in this role in column 0.*/
        Balance = Qt::UserRole + 3,            /**< The account balance is stored in this role in column 0 as a MyMoneyMoney object.*/
        Value = Qt::UserRole + 4,              /**< The account value (the balance converted to base currency) is stored in this role in column 0 as a MyMoneyMoney object.*/
        TotalValue = Qt::UserRole + 5,         /**< The account total value (the value of the account and of child accounts) is stored in this role in column 0 as a MyMoneyMoney object.*/
        DisplayOrder = Qt::UserRole + 9,       /**< This role is used by the filtering proxies to order the accounts for displaying.*/
        FullName = Qt::UserRole + 10,          /**< This role is used to provide the full pathname of the account */
    };

    inline uint qHash(const Role key, uint seed) { return ::qHash(static_cast<uint>(key), seed); }
}
#endif

namespace eLedgerModel {
  enum class Column {
    Number = 0,
    Date,
    Security,
    CostCenter,
    Detail,
    Reconciliation,
    Payment,
    Deposit,
    Quantity,
    Price,
    Amount,
    Value,
    Balance,

    // insert new columns above this line
    LastColumn
  };

  enum class Role {
    // Roles returning values
    PostDate = Qt::UserRole,
    PayeeName,
    Account,
    CounterAccount,
    SplitCount,
    Reconciliation,
    ReconciliationShort,
    ReconciliationLong,
    SplitShares,
    ShareAmount,
    ShareAmountSuffix,
    SplitValue,
    Memo,
    SingleLineMemo,
    Number,
    Erroneous,
    Import,
    Split,
    Transaction,

    // Roles returning ids
    TransactionId,
    SplitId,
    TransactionSplitId,
    PayeeId,
    AccountId,
    CounterAccountId,
    CostCenterId,
    ScheduleId,
    TransactionCommodity,

    // A pseudo role to emit the dataChanged() signal when
    // used with setData()
    EmitDataChanged

  };
}

/// @todo cleanup
/**
  * Make it possible to hold eAccountsModel::Column objects inside @ref QVariant objects.
  */
// Q_DECLARE_METATYPE(eAccountsModel::Column)

#endif
