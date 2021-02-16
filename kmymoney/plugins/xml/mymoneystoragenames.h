/*
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MYMONEYSTORAGENAMES_H
#define MYMONEYSTORAGENAMES_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QHash>

// ----------------------------------------------------------------------------
// Project Includes

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
    Address,
    Identifier
  };

  enum class KVP {
    Pair
  };

  enum class Institution {
    AccountID,
    AccountIDS,
    Address
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
    IBAN,
    BIC,
    OwnerVer1,
    OwnerVer2,
    BankCode,
    Country,
    AccountNumber,
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

QString elementName(Element::Schedule elementID);
QString attributeName(Attribute::Schedule attributeID);

QString elementName(Element::OnlineJob elementID);
QString attributeName(Attribute::OnlineJob attributeID);

QString attributeName(Attribute::CostCenter attributeID);

QString tagName(Tag tagID);
QString nodeName(Node nodeID);

#endif
