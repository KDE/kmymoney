/***************************************************************************
                          mymoneyenums.h
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

#include <QHashFunctions>

#ifndef MYMONEYENUMS_H
#define MYMONEYENUMS_H

namespace eMyMoney {
  /**
    * Account types currently supported.
    */
  enum class Account {
    Unknown = 0,          /**< For error handling */
    Checkings,            /**< Standard checking account */
    Savings,              /**< Typical savings account */
    Cash,                 /**< Denotes a shoe-box or pillowcase stuffed
                               with cash */
    CreditCard,           /**< Credit card accounts */
    Loan,                 /**< Loan and mortgage accounts (liability) */
    CertificateDep,       /**< Certificates of Deposit */
    Investment,           /**< Investment account */
    MoneyMarket,          /**< Money Market Account */
    Asset,                /**< Denotes a generic asset account.*/
    Liability,            /**< Denotes a generic liability account.*/
    Currency,             /**< Denotes a currency trading account. */
    Income,               /**< Denotes an income account */
    Expense,              /**< Denotes an expense account */
    AssetLoan,            /**< Denotes a loan (asset of the owner of this object) */
    Stock,                /**< Denotes an security account as sub-account for an investment */
    Equity,               /**< Denotes an equity account e.g. opening/closeing balance */

    /* insert new account types above this line */
    MaxAccountTypes       /**< Denotes the number of different account types */
  };

  inline uint qHash(const Account key, uint seed) { return ::qHash(static_cast<uint>(key), seed); }

  enum class Security {
    Stock,
    MutualFund,
    Bond,
    Currency,
    None
  };

  inline uint qHash(const Security key, uint seed) { return ::qHash(static_cast<uint>(key), seed); }

  namespace Schedule {
    /**
      * This enum is used to describe all the possible schedule frequencies.
      * The special entry, Any, is used to combine all the other types.
      */
    enum class Occurrence {
      Any = 0,
      Once = 1,
      Daily = 2,
      Weekly = 4,
      Fortnightly = 8,
      EveryOtherWeek = 16,
      EveryHalfMonth = 18,
      EveryThreeWeeks = 20,
      EveryThirtyDays = 30,
      Monthly = 32,
      EveryFourWeeks = 64,
      EveryEightWeeks = 126,
      EveryOtherMonth = 128,
      EveryThreeMonths = 256,
      TwiceYearly = 1024,
      EveryOtherYear = 2048,
      Quarterly = 4096,
      EveryFourMonths = 8192,
      Yearly = 16384
    };

    /**
      * This enum is used to describe the schedule type.
      */
    enum class Type {
      Any = 0,
      Bill = 1,
      Deposit = 2,
      Transfer = 4,
      LoanPayment = 5
    };

    /**
      * This enum is used to describe the schedule's payment type.
      */
    enum class PaymentType {
      Any = 0,
      DirectDebit = 1,
      DirectDeposit = 2,
      ManualDeposit = 4,
      Other = 8,
      WriteChecque = 16,
      StandingOrder = 32,
      BankTransfer = 64
    };

    /**
      * This enum is used by the auto-commit functionality.
      *
      * Depending upon the value of m_weekendOption the schedule can
      * be entered on a different date
    **/
    enum class WeekendOption {
      MoveBefore = 0,
      MoveAfter = 1,
      MoveNothing = 2
    };
  }

  namespace TransactionFilter {
    // Make sure to keep the following enum valus in sync with the values
    // used by the GUI (for KMyMoney in kfindtransactiondlgdecl.ui)
    enum class Type {
      All = 0,
      Payments,
      Deposits,
      Transfers,
      // insert new constants above of this line
      LastType
    };

    // Make sure to keep the following enum valus in sync with the values
    // used by the GUI (for KMyMoney in kfindtransactiondlgdecl.ui)
    enum class State {
      All = 0,
      NotReconciled,
      Cleared,
      Reconciled,
      Frozen,
      // insert new constants above of this line
      LastState
    };

    // Make sure to keep the following enum valus in sync with the values
    // used by the GUI (for KMyMoney in kfindtransactiondlgdecl.ui)
    enum class Validity {
      Any = 0,
      Valid,
      Invalid,
      // insert new constants above of this line
      LastValidity
    };

    // Make sure to keep the following enum valus in sync with the values
    // used by the GUI (for KMyMoney in kfindtransactiondlgdecl.ui)
    enum class Date {
      All = 0,
      AsOfToday,
      CurrentMonth,
      CurrentYear,
      MonthToDate,
      YearToDate,
      YearToMonth,
      LastMonth,
      LastYear,
      Last7Days,
      Last30Days,
      Last3Months,
      Last6Months,
      Last12Months,
      Next7Days,
      Next30Days,
      Next3Months,
      Next6Months,
      Next12Months,
      UserDefined,
      Last3ToNext3Months,
      Last11Months,
      CurrentQuarter,
      LastQuarter,
      NextQuarter,
      CurrentFiscalYear,
      LastFiscalYear,
      Today,
      Next18Months,
      // insert new constants above of this line
      LastDateItem
    };
  }

  namespace Split {
    /**
      * This enum defines the possible reconciliation states a split
      * can be in. Possible values are as follows:
      *
      * @li NotReconciled
      * @li Cleared
      * @li Reconciled
      * @li Frozen
      *
      * Whenever a new split is created, it has the status NotReconciled. It
      * can be set to cleared when the transaction has been performed. Once the
      * account is reconciled, cleared splits will be set to Reconciled. The
      * state Frozen will be used, when the concept of books is introduced into
      * the engine and a split must not be changed anymore.
      */
    enum class State {
      Unknown = -1,
      NotReconciled = 0,
      Cleared,
      Reconciled,
      Frozen,
      // insert new values above
      MaxReconcileState
    };

    enum class InvestmentTransactionType {
      UnknownTransactionType = -1,
      BuyShares = 0,
      SellShares,
      Dividend,
      ReinvestDividend,
      Yield,
      AddShares,
      RemoveShares,
      SplitShares,
      InterestIncome///
    };

    inline uint qHash(const InvestmentTransactionType key, uint seed) { return ::qHash(static_cast<uint>(key), seed); }
  }

  namespace File {
    /**
      * notificationObject identifies the type of the object
      * for which this notification is stored
      */
    enum class Object {
      Account = 1,
      Institution,
      Payee,
      Transaction,
      Tag,
      Schedule,
      Security,
      OnlineJob
    };

    /**
      * notificationMode identifies the type of notifiation
      * (add, modify, remove)
      */
    enum class Mode {
      Add = 1,
      Modify,
      Remove
    };

  }

  /**
   * @brief Type of message
   *
   * An usually it is not easy to categorise log messages. This description is only a hint.
   */
  namespace OnlineJob {
    enum class MessageType {
      Debug, /**< Just for debug purposes. In normal scenarios the user should not see this. No need to store this message. Plugins should
        not create them at all if debug mode is not enabled. */
      Log, /**< A piece of information the user should not see during normal operation. It is not shown in any UI by default. It is stored persistantly. */
      Information, /**< Information that should be kept but without the need to burden the user. The user can
        see this during normal operation. */
      Warning, /**< A piece of information the user should see but not be enforced to do so (= no modal dialog). E.g. a task is expected to have
        direct effect but insted you have to wait a day (and that is commen behavior). */
      Error /**< Important for the user - he must be warned. E.g. a task could unexpectedly not be executed */
    };
  }
}
#endif
