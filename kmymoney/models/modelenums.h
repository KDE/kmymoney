/***************************************************************************
                          modelenums.h
                             -------------------
    copyright            : (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>

***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QHash>

#ifndef MODELENUMS_H
#define MODELENUMS_H

namespace eAccountsModel {
    enum class Column {
        FirstColumnMarker = 0,
        Account = 0,  // CAUTION! Assumption is being made that Account column number is always 0 and you shouldn't change this
        Type,
        Tax,
        VAT,
        CostCenter,
        TotalBalance,
        PostedValue,
        TotalValue,
        AccountNumber,
        AccountSortCode,
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
