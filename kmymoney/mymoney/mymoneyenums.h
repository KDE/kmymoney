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

#ifndef MYMONEYENUMS_H
#define MYMONEYENUMS_H

#include <QHashFunctions>

namespace eMyMoney {
  /**
    * Account types currently supported.
    */
  namespace Account {
    enum class Type {
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
      Equity,               /**< Denotes an equity account e.g. opening/closing balance */

      /* insert new account types above this line */
      MaxAccountTypes       /**< Denotes the number of different account types */
    };
    inline uint qHash(const Type key, uint seed) { return ::qHash(static_cast<uint>(key), seed); }

    enum class Standard {
      Liability,
      Asset,
      Expense,
      Income,
      Equity
    };

    inline uint qHash(const Standard key, uint seed) { return ::qHash(static_cast<uint>(key), seed); }
  }

  namespace Payee {
    enum class MatchType {
      Disabled = 0,
      Name,
      Key,
      NameExact
    };
  }

  namespace Security {
    enum class Type {
      Stock,
      MutualFund,
      Bond,
      Currency,
      None
    };

    inline uint qHash(const Type key, uint seed) { return ::qHash(static_cast<uint>(key), seed); }
  }

  namespace Report {
    enum class RowType { NoRows = 0, AssetLiability, ExpenseIncome, Category, TopCategory, Account, Tag, Payee, Month, Week, TopAccount, AccountByTopAccount, EquityType, AccountType, Institution, Budget, BudgetActual, Schedule, AccountInfo, AccountLoanInfo, AccountReconcile, CashFlow, Invalid };
    inline uint qHash(const RowType key, uint seed) { return ::qHash(static_cast<uint>(key), seed); }

    enum class ColumnType { NoColumns = 0, Days = 1, Months = 1, BiMonths = 2, Quarters = 3, Weeks = 7, Years = 12, Invalid };
    inline uint qHash(const ColumnType key, uint seed) { return ::qHash(static_cast<uint>(key), seed); }

    enum class ReportType { NoReport = 0, PivotTable, QueryTable, InfoTable, Invalid };
    // if you add bits to this bitmask, start with the value currently assigned to QCend and update its value afterwards
    // also don't forget to add column names to kQueryColumnsText in mymoneyreport.cpp
    enum QueryColumn : int { None = 0x0, Begin = 0x1, Number = 0x1, Payee = 0x2, Category = 0x4, Tag = 0x8, Memo = 0x10, Account = 0x20, Reconciled = 0x40, Action = 0x80, Shares = 0x100, Price = 0x200, Performance = 0x400, Loan = 0x800, Balance = 0x1000, CapitalGain = 0x2000, End = 0x4000 };

    enum class DetailLevel { None = 0, All, Top, Group, Total, End };
    inline uint qHash(const DetailLevel key, uint seed) { return ::qHash(static_cast<uint>(key), seed); }

    enum class InvestmentSum { Period = 0, OwnedAndSold, Owned, Sold, Bought};
    enum class ChartType { None = 0, Line, Bar, Pie, Ring, StackedBar, End };
    inline uint qHash(const ChartType key, uint seed) { return ::qHash(static_cast<uint>(key), seed); }

    enum class DataLock { Automatic = 0, UserDefined, DataOptionCount };
    inline uint qHash(const DataLock key, uint seed) { return ::qHash(static_cast<uint>(key), seed); }
    enum class ChartPalette { Application = 0, Default, Rainbow, Subdued, End};
    inline uint qHash(const ChartPalette key, uint seed) { return ::qHash(static_cast<uint>(key), seed); }
  }

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

  namespace Budget {
    enum class Level {
      None = 0,
      Monthly,
      MonthByMonth,
      Yearly,
      Max
    };

    inline uint qHash(const Level key, uint seed) { return ::qHash(static_cast<uint>(key), seed); }
  }

  namespace TransactionFilter {
    // Make sure to keep the following enum values in sync with the values
    // used by the GUI (for KMyMoney in kfindtransactiondlgdecl.ui)
    enum class Type {
      All = 0,
      Payments,
      Deposits,
      Transfers,
      // insert new constants above of this line
      LastType
    };

    // Make sure to keep the following enum values in sync with the values
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

    // Make sure to keep the following enum values in sync with the values
    // used by the GUI (for KMyMoney in kfindtransactiondlgdecl.ui)
    enum class Validity {
      Any = 0,
      Valid,
      Invalid,
      // insert new constants above of this line
      LastValidity
    };

    // Make sure to keep the following enum values in sync with the values
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
    inline uint qHash(const Date key, uint seed) { return ::qHash(static_cast<uint>(key), seed); }
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

    enum class Action {
      Check,
      Deposit,
      Transfer,
      Withdrawal,
      ATM,
      Amortization,
      Interest,
      BuyShares,
      Dividend,
      ReinvestDividend,
      Yield,
      AddShares,
      SplitShares,
      InterestIncome
    };

    inline uint qHash(const Action key, uint seed) { return ::qHash(static_cast<uint>(key), seed); }
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
      OnlineJob,
      CostCenter
    };

    /**
      * notificationMode identifies the type of notification
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
      Log, /**< A piece of information the user should not see during normal operation. It is not shown in any UI by default. It is stored persistently. */
      Information, /**< Information that should be kept but without the need to burden the user. The user can
        see this during normal operation. */
      Warning, /**< A piece of information the user should see but not be enforced to do so (= no modal dialog). E.g. a task is expected to have
        direct effect but instead you have to wait a day (and that is common behavior). */
      Error /**< Important for the user - he must be warned. E.g. a task could unexpectedly not be executed */
    };

    /**
     * @brief The state of a job given by the onlinePlugin
     */
    enum class sendingState {
      noBankAnswer, /**< Used during or before sending or if sendDate().isValid() the job was successfully sent */
      acceptedByBank, /**< bank definitely confirmed the job */
      rejectedByBank, /**< bank definitely rejected this job */
      abortedByUser, /**< aborted by user during sending */
      sendingError /**< an error occurred, the job is certainly not executed by the bank */
    };
  }

  namespace Statement {
    enum class Type {
      None = 0,
      Checkings,
      Savings,
      Investment,
      CreditCard,
      Invalid
    };

    inline uint qHash(const Type key, uint seed) { return ::qHash(static_cast<uint>(key), seed); }
  }

  namespace Transaction {
    // the following members are only used for investment accounts (m_eType==etInvestment)
    // eaNone means the action, shares, and security can be ignored.
    enum class Action {
      None = 0,
      Buy,
      Sell,
      ReinvestDividend,
      CashDividend,
      Shrsin,
      Shrsout,
      Stksplit,
      Fees,
      Interest,
      Invalid
    };

    inline uint qHash(const Action key, uint seed) { return ::qHash(static_cast<uint>(key), seed); }
  }

  namespace Money {
    enum signPosition : int {
      // keep those in sync with the ones defined in klocale.h
      ParensAround = 0,
      BeforeQuantityMoney = 1,
      AfterQuantityMoney = 2,
      BeforeMoney = 3,
      AfterMoney = 4
    };
  }

}
#endif
