/*
 * Copyright 2017       Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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

#ifndef WIDGETENUMS_H
#define WIDGETENUMS_H

#include <qnamespace.h>

namespace eWidgets {

  enum class SortField {
  Unknown = 0,      ///< unknown sort criteria
  PostDate = 1,     ///< sort by post date
  EntryDate,        ///< sort by entry date
  Payee,            ///< sort by payee name
  Value,            ///< sort by value
  NoSort,               ///< sort by number field
  EntryOrder,       ///< sort by entry order
  Type,             ///< sort by CashFlowDirection
  Category,         ///< sort by Category
  ReconcileState,   ///< sort by reconciliation state
  Security,         ///< sort by security (only useful for investment accounts)
  // insert new values in front of this line
  MaxFields
  };

  namespace eTransaction {
    enum class Column {
      Number = 0,
      Date,
      Account,
      Security,
      Detail,
      ReconcileFlag,
      Payment,
      Deposit,
      Quantity,
      Price,
      Value,
      Balance,
      // insert new values above this line
      LastColumn
    };
  }

  namespace eTransactionForm {
    enum class Column {
      Label1 = 0,
      Value1,
      Label2,
      Value2,
      // insert new values above this line
      LastColumn
    };
  }

  namespace eTabBar {
    enum class SignalEmission {
      Normal = 0,      // standard signal behaviour
      Never,           // don't signal selection of a tab at all
      Always           // always signal selection of a tab
    };
  }

  namespace eRegister {
    enum class Action {
      None = -1,
      Check = 0,
      /* these should be values which qt 3.3 never uses for QTab:
       * qt starts upwards from 0
       */
      Deposit = 12201,
      Transfer = 12202,
      Withdrawal = 12203,
      Atm,
      // insert new values above this line
      LastAction
    };

    enum class CashFlowDirection {
      Deposit = 0,          //< transaction is deposit
      Payment,              //< transaction is payment
      Unknown               //< transaction cashflow is unknown
    };

    enum class DetailColumn {
      PayeeFirst = 0,       ///< show the payee on the first row of the transaction in the details column and the account on the second
      AccountFirst          ///< show the account on the first row of the transaction in the details column and the payee on the second
    };
  }

  namespace ValidationFeedback {
    enum class MessageType {
      None,
      Positive,
      Information,
      Warning,
      Error
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
