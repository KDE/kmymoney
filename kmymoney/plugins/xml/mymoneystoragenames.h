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

#ifndef MYMONEYSTORAGENAMES_H
#define MYMONEYSTORAGENAMES_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QHash>

// ----------------------------------------------------------------------------
// Project Includes

namespace MyMoneyStorageTags {

enum tagNameE { tnInstitutions, tnPayees, tnCostCenters,
                tnTags, tnAccounts, tnTransactions,
                tnSchedules, tnSecurities, tnCurrencies,
                tnPrices, tnReports, tnBudgets, tnOnlineJobs,
                tnKMMFile, tnFileInfo, tnUser
              };

extern const QHash<tagNameE, QString>    tagNames;
}

namespace MyMoneyStorageNodes {

enum ndNameE { nnInstitution, nnPayee, nnCostCenter,
               nnTag, nnAccount, nnTransaction,
               nnScheduleTX, nnSecurity, nnCurrency,
               nnPrice, nnPricePair, nnReport, nnBudget, nnOnlineJob,
               nnKeyValuePairs, nnEquity
             };

extern const QHash<ndNameE, QString>    nodeNames;
}

namespace MyMoneyStorageAttributes {

enum attrNameE { anID, anDate, anCount, anFrom, anTo,
                 anSource, anKey, anValue, anPrice,
                 anName, anEmail, anCountry, anCity,
                 anZipCode, anStreet, anTelephone
               };

extern const QHash<attrNameE, QString>    attrNames;
}

namespace MyMoneyStandardAccounts {

enum idNameE { stdAccLiability, stdAccAsset, stdAccExpense, stdAccIncome, stdAccEquity };

extern const QHash<idNameE, QString>    stdAccNames;
}

enum class StdAccName {
  Liability,
  Asset,
  Expense,
  Income,
  Equity
};

enum class Tag {
  Institutions,
  Payees,
  CostCenters,
  Tags,
  Accounts,
  Transactions,
  Schedules,
  Securities,
  Currencies,
  Prices,
  Reports,
  Budgets,
  OnlineJobs,
  KMMFile,
  FileInfo,
  User
};

enum class Node {
  Institution,
  Payee,
  CostCenter,
  Tag,
  Account,
  Transaction,
  Split,
  ScheduleTX,
  Security,
  Currency,
  Price,
  PricePair,
  Report,
  Budget,
  OnlineJob,
  KeyValuePairs,
  Equity
};

namespace Element {
  enum class General {
    Address,
    CreationDate,
    LastModifiedDate,
    Version,
    FixVersion,
    Pair
  };

  enum class Transaction {
    Split = 0,
    Splits
  };

  enum class Split {
    Split = 0,
    Tag,
    Match,
    Container,
    KeyValuePairs
  };

  enum class Account {
    SubAccount,
    SubAccounts,
    OnlineBanking
  };

  enum class Payee {
    Address
  };

  enum class KVP {
    Pair
  };

  enum class Institution {
    AccountID,
    AccountIDS,
    Address
  };

  enum class Report {
    Payee,
    Tag,
    Account,
    Text,
    Type,
    State,
    Number,
    Amount,
    Dates,
    Category,
    AccountGroup
  };

  enum class Budget {
    Budget = 0,
    Account,
    Period
  };

  enum class Schedule {
    Payment,
    Payments
  };

  enum class OnlineJob {
    OnlineTask
  };
}

namespace Attribute {
  enum class General {
    ID = 0,
    Date,
    Count,
    From,
    To,
    Source,
    Key,
    Value,
    Price,
    Name,
    Email,
    Country,
    City,
    ZipCode,
    Street,
    Telephone,
    // insert new entries above this line
    LastAttribute
  };

  enum class Transaction {
    Name = 0,
    Type,
    PostDate,
    Memo,
    EntryDate,
    Commodity,
    BankID,
    // insert new entries above this line
    LastAttribute
  };

  enum class Split {
    ID = 0,
    BankID,
    Account,
    Payee,
    Tag,
    Number,
    Action,
    Value,
    Shares,
    Price,
    Memo,
    CostCenter,
    ReconcileDate,
    ReconcileFlag,
    KMMatchedTx,
    // insert new entries above this line
    LastAttribute
  };

  enum class Account {
    ID = 0,
    Name,
    Type,
    ParentAccount,
    LastReconciled,
    LastModified,
    Institution,
    Opened,
    Number,
    Description,
    Currency,
    OpeningBalance,
    IBAN,
    BIC,
    // insert new entries above this line
    LastAttribute
  };

  enum class Payee {
    ID = 0,
    Name,
    Type,
    Reference,
    Notes,
    MatchingEnabled,
    UsingMatchKey,
    MatchIgnoreCase,
    MatchKey,
    DefaultAccountID,
    Street,
    City,
    PostCode,
    Email,
    State,
    Telephone,
    // insert new entries above this line
    LastAttribute
  };

  enum class Tag {
    ID = 0,
    Name,
    Type,
    TagColor,
    Closed,
    Notes,
    // insert new entries above this line
    LastAttribute
  };

  enum class Security {
    ID = 0,
    Name,
    Symbol,
    Type,
    RoundingMethod,
    SAF,
    PP,
    SCF,
    TradingCurrency,
    TradingMarket,
    // insert new entries above this line
    LastAttribute
  };

  enum class KVP {
    Key,
    Value,
    // insert new entries above this line
    LastAttribute
  };

  enum class Institution {
    ID = 0,
    Name,
    Manager,
    SortCode,
    Street,
    City,
    Zip,
    Telephone,
    // insert new entries above this line
    LastAttribute
  };

  enum class Report {
    ID = 0, Group, Type, Name, Comment, ConvertCurrency, Favorite,
    SkipZero, DateLock, DataLock, MovingAverageDays,
    IncludesActuals, IncludesForecast, IncludesPrice,
    IncludesAveragePrice, IncludesMovingAverage,
    IncludesSchedules, IncludesTransfers, IncludesUnused,
    MixedTime, Investments, Budget,
    ShowRowTotals, ShowColumnTotals, Detail,
    ColumnsAreDays, ChartType,
    ChartCHGridLines, ChartSVGridLines,
    ChartDataLabels, ChartByDefault,
    LogYAxis, ChartLineWidth, ColumnType, RowType,
    DataRangeStart, DataRangeEnd,
    DataMajorTick, DataMinorTick,
    YLabelsPrecision, QueryColumns,
    Tax, Loans, HideTransactions, InvestmentSum,
    SettlementPeriod, ShowSTLTCapitalGains, TermsSeparator,
    Pattern, CaseSensitive, RegEx, InvertText, State,
    From, To,
    // insert new entries above this line
    LastAttribute
  };

  enum class Budget {
    ID = 0,
    Name,
    Start,
    Version,
    BudgetLevel,
    BudgetSubAccounts,
    Amount,
    // insert new entries above this line
    LastAttribute
  };

  enum class Schedule {
    ID = 0,
    Name,
    Type,
    Occurrence,
    OccurrenceMultiplier,
    PaymentType,
    Fixed,
    AutoEnter,
    LastPayment,
    WeekendOption,
    Date,
    StartDate,
    EndDate,
    LastDayInMonth,
    // insert new entries above this line
    LastAttribute
  };

  enum class OnlineJob {
    ID = 0,
    Send,
    BankAnswerDate,
    BankAnswerState,
    IID,
    AbortedByUser,
    AcceptedByBank,
    RejectedByBank,
    SendingError,
    // insert new entries above this line
    LastAttribute
  };

  enum class CostCenter {
    ID = 0,
    Name,
    // insert new entries above this line
    LastAttribute
  };
}

QString elementName(Element::General elementID);
QString attributeName(Attribute::General attributeID);

QString elementName(Element::Transaction elementID);
QString attributeName(Attribute::Transaction attributeID);

QString elementName(Element::Split elementID);
QString attributeName(Attribute::Split attributeID);

QString elementName(Element::Account elementID);
QString attributeName(Attribute::Account attributeID);

QString elementName(Element::Payee elementID);
QString attributeName(Attribute::Payee attributeID);

QString attributeName(Attribute::Tag attributeID);

QString attributeName(Attribute::Security attributeID);

QString elementName(Element::KVP elementID);
QString attributeName(Attribute::KVP attributeID);

QString elementName(Element::Institution elementID);
QString attributeName(Attribute::Institution attributeID);

QString elementName(Element::Report elementID);
QString attributeName(Attribute::Report attributeID);

QString elementName(Element::Budget elementID);
QString attributeName(Attribute::Budget attributeID);

QString elementName(Element::Schedule elementID);
QString attributeName(Attribute::Schedule attributeID);

QString elementName(Element::OnlineJob elementID);
QString attributeName(Attribute::OnlineJob attributeID);

QString attributeName(Attribute::CostCenter attributeID);

QString stdAccName(StdAccName stdAccID);
QString tagName(Tag tagID);
QString nodeName(Node nodeID);

namespace eMyMoney { namespace Report { enum class RowType; } }
namespace eMyMoney { namespace Report { enum class ColumnType; } }
namespace eMyMoney { namespace Report { enum QueryColumn : int; } }
namespace eMyMoney { namespace Report { enum class ChartType; } }
namespace eMyMoney { namespace Report { enum class DataLock; } }
namespace eMyMoney { namespace Report { enum class ReportType; } }
namespace eMyMoney { namespace Report { enum class DetailLevel; } }

QHash<eMyMoney::Report::RowType, QString> rowTypesLUT();
QString reportNames(eMyMoney::Report::RowType textID);
eMyMoney::Report::RowType stringToRowType(const QString &text);
QHash<eMyMoney::Report::ColumnType, QString> columTypesLUT();
QString reportNames(eMyMoney::Report::ColumnType textID);
eMyMoney::Report::ColumnType stringToColumnType(const QString &text);
QHash<eMyMoney::Report::QueryColumn, QString> queryColumnsLUT();
QString reportNamesForQC(eMyMoney::Report::QueryColumn textID);
eMyMoney::Report::QueryColumn stringToQueryColumn(const QString &text);
QHash<eMyMoney::Report::DetailLevel, QString> detailLevelLUT();
QString reportNames(eMyMoney::Report::DetailLevel textID);
eMyMoney::Report::DetailLevel stringToDetailLevel(const QString &text);
QHash<eMyMoney::Report::ChartType, QString> chartTypeLUT();
QString reportNames(eMyMoney::Report::ChartType textID);
eMyMoney::Report::ChartType stringToChartType(const QString &text);
QHash<int, QString> typeAttributeLUT();
QString typeAttributeToString(int textID);
int stringToTypeAttribute(const QString &text);
QHash<int, QString> stateAttributeLUT();
QString stateAttributeToString(int textID);
int stringToStateAttribute(const QString &text);
QHash<int, QString> dateLockLUT();
QString dateLockAttributeToString(int textID);
int stringToDateLockAttribute(const QString &text);
QHash<eMyMoney::Report::DataLock, QString> dataLockLUT();
QString reportNames(eMyMoney::Report::DataLock textID);
eMyMoney::Report::DataLock stringToDataLockAttribute(const QString &text);
QHash<int, QString> accountTypeAttributeLUT();
QString accountTypeAttributeToString(int textID);
int stringToAccountTypeAttribute(const QString &text);
eMyMoney::Report::ReportType rowTypeToReportType(eMyMoney::Report::RowType rowType);

namespace eMyMoney { namespace Budget { enum class Level; } }

QHash<eMyMoney::Budget::Level, QString> budgetLevelLUT();
QString budgetNames(eMyMoney::Budget::Level textID);
eMyMoney::Budget::Level stringToBudgetLevel(const QString &text);

#endif
