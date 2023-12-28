/*
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef WIDGETENUMS_H
#define WIDGETENUMS_H

#include <qnamespace.h>

namespace eWidgets {

/*
 * IMPORTANT: Keep this in sync with sortOrderText array in transactionsortoption.cpp
 *            Don't touch the order of this list because it will break
 *            the sorting in existing data files
 */
enum class SortField {
    Unknown = 0, ///< unknown sort criteria
    PostDate = 1, ///< sort by post date
    EntryDate, ///< sort by entry date
    Payee, ///< sort by payee name
    Value, ///< sort by value
    NoSort, ///< sort by number field
    EntryOrder, ///< sort by entry order
    Type, ///< sort by CashFlowDirection
    Category, ///< sort by Category
    ReconcileState, ///< sort by reconciliation state
    Security, ///< sort by security (only useful for investment accounts)
    ReconciliationDate, ///< sort by reconciliation date
    // insert new values in front of this comment
    MaxFields,
};

namespace ValidationFeedback {
enum class MessageType {
    None,
    Positive,
    Information,
    Warning,
    Error,
};
}

namespace Selector {
enum class Role {
    Id = Qt::UserRole,      /**< The id is stored in this role in column 0 as a string.*/
    Key = Qt::UserRole + 1, /**< The key is stored in this role in column 0 as a string.*/
};
}

}

#endif
