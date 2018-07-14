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

#include "mymoneystoragenames.h"

#include <QMap>

#include "mymoneyenums.h"

QString stdAccName(StdAccName stdAccID)
{
  static const QHash<StdAccName, QString> stdAccNames {
    {StdAccName::Liability, QStringLiteral("AStd::Liability")},
    {StdAccName::Asset,     QStringLiteral("AStd::Asset")},
    {StdAccName::Expense,   QStringLiteral("AStd::Expense")},
    {StdAccName::Income,    QStringLiteral("AStd::Income")},
    {StdAccName::Equity,    QStringLiteral("AStd::Equity")},
  };
  return stdAccNames.value(stdAccID);
}

uint qHash(const StdAccName key, uint seed) { return ::qHash(static_cast<uint>(key), seed); }

QString tagName(Tag tagID)
{
  static const QHash<Tag, QString> tagNames {
    {Tag::Institutions, QStringLiteral("INSTITUTIONS")},
    {Tag::Payees,       QStringLiteral("PAYEES")},
    {Tag::CostCenters,  QStringLiteral("COSTCENTERS")},
    {Tag::Tags,         QStringLiteral("TAGS")},
    {Tag::Accounts,     QStringLiteral("ACCOUNTS")},
    {Tag::Transactions, QStringLiteral("TRANSACTIONS")},
    {Tag::Schedules,    QStringLiteral("SCHEDULES")},
    {Tag::Securities,   QStringLiteral("SECURITIES")},
    {Tag::Currencies,   QStringLiteral("CURRENCIES")},
    {Tag::Prices,       QStringLiteral("PRICES")},
    {Tag::Reports,      QStringLiteral("REPORTS")},
    {Tag::Budgets,      QStringLiteral("BUDGETS")},
    {Tag::OnlineJobs,   QStringLiteral("ONLINEJOBS")},
    {Tag::KMMFile,      QStringLiteral("KMYMONEY-FILE")},
    {Tag::FileInfo,     QStringLiteral("FILEINFO")},
    {Tag::User,         QStringLiteral("USER")}
  };
  return tagNames.value(tagID);
}

uint qHash(const Tag key, uint seed) { return ::qHash(static_cast<uint>(key), seed); }

QString nodeName(Node nodeID)
{
  static const QHash<Node, QString> nodeNames {
    {Node::Institution,   QStringLiteral("INSTITUTION")},
    {Node::Payee,         QStringLiteral("PAYEE")},
    {Node::CostCenter,    QStringLiteral("COSTCENTER")},
    {Node::Tag,           QStringLiteral("TAG")},
    {Node::Account,       QStringLiteral("ACCOUNT")},
    {Node::Transaction,   QStringLiteral("TRANSACTION")},
    {Node::Split,         QStringLiteral("SPLIT")},
    {Node::ScheduleTX,    QStringLiteral("SCHEDULED_TX")},
    {Node::Security,      QStringLiteral("SECURITY")},
    {Node::Currency,      QStringLiteral("CURRENCY")},
    {Node::Price,         QStringLiteral("PRICE")},
    {Node::PricePair,     QStringLiteral("PRICEPAIR")},
    {Node::Report,        QStringLiteral("REPORT")},
    {Node::Budget,        QStringLiteral("BUDGET")},
    {Node::OnlineJob,     QStringLiteral("ONLINEJOB")},
    {Node::KeyValuePairs, QStringLiteral("KEYVALUEPAIRS")},
    {Node::Equity,        QStringLiteral("EQUITY")},
  };
  return nodeNames.value(nodeID);
}

uint qHash(const Node key, uint seed) { return ::qHash(static_cast<uint>(key), seed); }

namespace Element {
  uint qHash(const General key, uint seed) { return ::qHash(static_cast<uint>(key), seed); }
  uint qHash(const Transaction key, uint seed) { return ::qHash(static_cast<uint>(key), seed); }
  uint qHash(const Account key, uint seed) { return ::qHash(static_cast<uint>(key), seed); }
  uint qHash(const Payee key, uint seed) { return ::qHash(static_cast<uint>(key), seed); }
  uint qHash(const KVP key, uint seed) { return ::qHash(static_cast<uint>(key), seed); }
  uint qHash(const Institution key, uint seed) { return ::qHash(static_cast<uint>(key), seed); }
  uint qHash(const Report key, uint seed) { return ::qHash(static_cast<uint>(key), seed); }
  uint qHash(const Budget key, uint seed) { return ::qHash(static_cast<uint>(key), seed); }
  uint qHash(const Schedule key, uint seed) { return ::qHash(static_cast<uint>(key), seed); }
  uint qHash(const OnlineJob key, uint seed) { return ::qHash(static_cast<uint>(key), seed); }
}

namespace Attribute {
  uint qHash(const General key, uint seed) { return ::qHash(static_cast<uint>(key), seed); }
  uint qHash(const Transaction key, uint seed) { return ::qHash(static_cast<uint>(key), seed); }
  uint qHash(const Account key, uint seed) { return ::qHash(static_cast<uint>(key), seed); }
  uint qHash(const Payee key, uint seed) { return ::qHash(static_cast<uint>(key), seed); }
  uint qHash(const Tag key, uint seed) { return ::qHash(static_cast<uint>(key), seed); }
  uint qHash(const Security key, uint seed) { return ::qHash(static_cast<uint>(key), seed); }
  uint qHash(const KVP key, uint seed) { return ::qHash(static_cast<uint>(key), seed); }
  uint qHash(const Institution key, uint seed) { return ::qHash(static_cast<uint>(key), seed); }
  uint qHash(const Report key, uint seed) { return ::qHash(static_cast<uint>(key), seed); }
  uint qHash(const Budget key, uint seed) { return ::qHash(static_cast<uint>(key), seed); }
  uint qHash(const Schedule key, uint seed) { return ::qHash(static_cast<uint>(key), seed); }
  uint qHash(const OnlineJob key, uint seed) { return ::qHash(static_cast<uint>(key), seed); }
}

QString elementName(Element::General elementID)
{
  static const QMap<Element::General, QString> elementNames {
    {Element::General::Address,           QStringLiteral("ADDRESS")},
    {Element::General::CreationDate,      QStringLiteral("CREATION_DATE")},
    {Element::General::LastModifiedDate,  QStringLiteral("LAST_MODIFIED_DATE")},
    {Element::General::Version,           QStringLiteral("VERSION")},
    {Element::General::FixVersion,        QStringLiteral("FIXVERSION")},
    {Element::General::Pair,              QStringLiteral("PAIR")}
  };
  return elementNames.value(elementID);
}

QString attributeName(Attribute::General attributeID)
{
  static const QMap<Attribute::General, QString> attributeNames {
    {Attribute::General::ID,        QStringLiteral("id")},
    {Attribute::General::Date,      QStringLiteral("date")},
    {Attribute::General::Count,     QStringLiteral("count")},
    {Attribute::General::From,      QStringLiteral("from")},
    {Attribute::General::To,        QStringLiteral("to")},
    {Attribute::General::Source,    QStringLiteral("source")},
    {Attribute::General::Key,       QStringLiteral("key")},
    {Attribute::General::Value,     QStringLiteral("value")},
    {Attribute::General::Price,     QStringLiteral("price")},
    {Attribute::General::Name,      QStringLiteral("name")},
    {Attribute::General::Email,     QStringLiteral("email")},
    {Attribute::General::Country,   QStringLiteral("county")},
    {Attribute::General::City,      QStringLiteral("city")},
    {Attribute::General::ZipCode,   QStringLiteral("zipcode")},
    {Attribute::General::Street,    QStringLiteral("street")},
    {Attribute::General::Telephone, QStringLiteral("telephone")}
  };
  return attributeNames.value(attributeID);
}

QString elementName(Element::Transaction elementID)
{
  static const QMap<Element::Transaction, QString> elementNames {
    {Element::Transaction::Split,  QStringLiteral("SPLIT")},
    {Element::Transaction::Splits, QStringLiteral("SPLITS")}
  };
  return elementNames.value(elementID);
}

QString attributeName(Attribute::Transaction attributeID)
{
  static const QMap<Attribute::Transaction, QString> attributeNames {
    {Attribute::Transaction::Name,       QStringLiteral("name")},
    {Attribute::Transaction::Type,       QStringLiteral("type")},
    {Attribute::Transaction::PostDate,   QStringLiteral("postdate")},
    {Attribute::Transaction::Memo,       QStringLiteral("memo")},
    {Attribute::Transaction::EntryDate,  QStringLiteral("entrydate")},
    {Attribute::Transaction::Commodity,  QStringLiteral("commodity")},
    {Attribute::Transaction::BankID,     QStringLiteral("bankid")},
  };
  return attributeNames.value(attributeID);
}

QString elementName(Element::Split elementID)
{
  static const QMap<Element::Split, QString> elementNames {
    {Element::Split::Split,          QStringLiteral("SPLIT")},
    {Element::Split::Tag,            QStringLiteral("TAG")},
    {Element::Split::Match,          QStringLiteral("MATCH")},
    {Element::Split::Container,      QStringLiteral("CONTAINER")},
    {Element::Split::KeyValuePairs,  QStringLiteral("KEYVALUEPAIRS")}
  };
  return elementNames.value(elementID);
}

QString attributeName(Attribute::Split attributeID)
{
  static const QMap<Attribute::Split, QString> attributeNames {
    {Attribute::Split::ID,             QStringLiteral("id")},
    {Attribute::Split::BankID,         QStringLiteral("bankid")},
    {Attribute::Split::Account,        QStringLiteral("account")},
    {Attribute::Split::Payee,          QStringLiteral("payee")},
    {Attribute::Split::Tag,            QStringLiteral("tag")},
    {Attribute::Split::Number,         QStringLiteral("number")},
    {Attribute::Split::Action,         QStringLiteral("action")},
    {Attribute::Split::Value,          QStringLiteral("value")},
    {Attribute::Split::Shares,         QStringLiteral("shares")},
    {Attribute::Split::Price,          QStringLiteral("price")},
    {Attribute::Split::Memo,           QStringLiteral("memo")},
    {Attribute::Split::CostCenter,     QStringLiteral("costcenter")},
    {Attribute::Split::ReconcileDate,  QStringLiteral("reconciledate")},
    {Attribute::Split::ReconcileFlag,  QStringLiteral("reconcileflag")},
    {Attribute::Split::KMMatchedTx,    QStringLiteral("kmm-matched-tx")}
  };
  return attributeNames.value(attributeID);
}

QString elementName(Element::Account elementID)
{
  static const QMap<Element::Account, QString> elementNames {
    {Element::Account::SubAccount,     QStringLiteral("SUBACCOUNT")},
    {Element::Account::SubAccounts,    QStringLiteral("SUBACCOUNTS")},
    {Element::Account::OnlineBanking,  QStringLiteral("ONLINEBANKING")}
  };
  return elementNames.value(elementID);
}

QString attributeName(Attribute::Account attributeID)
{
  static const QHash<Attribute::Account, QString> attributeNames {
    {Attribute::Account::ID,             QStringLiteral("id")},
    {Attribute::Account::Name,           QStringLiteral("name")},
    {Attribute::Account::Type,           QStringLiteral("type")},
    {Attribute::Account::ParentAccount,  QStringLiteral("parentaccount")},
    {Attribute::Account::LastReconciled, QStringLiteral("lastreconciled")},
    {Attribute::Account::LastModified,   QStringLiteral("lastmodified")},
    {Attribute::Account::Institution,    QStringLiteral("institution")},
    {Attribute::Account::Opened,         QStringLiteral("opened")},
    {Attribute::Account::Number,         QStringLiteral("number")},
    {Attribute::Account::Type,           QStringLiteral("type")},
    {Attribute::Account::Description,    QStringLiteral("description")},
    {Attribute::Account::Currency,       QStringLiteral("currency")},
    {Attribute::Account::OpeningBalance, QStringLiteral("openingbalance")},
    {Attribute::Account::IBAN,           QStringLiteral("iban")},
    {Attribute::Account::BIC,            QStringLiteral("bic")},
  };
  return attributeNames.value(attributeID);
}

QString elementName(Element::Payee elementID)
{
  static const QMap<Element::Payee, QString> elementNames {
    {Element::Payee::Address, QStringLiteral("ADDRESS")}
  };
  return elementNames.value(elementID);
}

QString attributeName(Attribute::Payee attributeID)
{
  static const QMap<Attribute::Payee, QString> attributeNames {
    {Attribute::Payee::Name,             QStringLiteral("name")},
    {Attribute::Payee::Type,             QStringLiteral("type")},
    {Attribute::Payee::Reference,        QStringLiteral("reference")},
    {Attribute::Payee::Notes,            QStringLiteral("notes")},
    {Attribute::Payee::MatchingEnabled,  QStringLiteral("matchingenabled")},
    {Attribute::Payee::UsingMatchKey,    QStringLiteral("usingmatchkey")},
    {Attribute::Payee::MatchIgnoreCase,  QStringLiteral("matchignorecase")},
    {Attribute::Payee::MatchKey,         QStringLiteral("matchkey")},
    {Attribute::Payee::DefaultAccountID, QStringLiteral("defaultaccountid")},
    {Attribute::Payee::Street,           QStringLiteral("street")},
    {Attribute::Payee::City,             QStringLiteral("city")},
    {Attribute::Payee::PostCode,         QStringLiteral("postcode")},
    {Attribute::Payee::Email,            QStringLiteral("email")},
    {Attribute::Payee::State,            QStringLiteral("state")},
    {Attribute::Payee::Telephone,        QStringLiteral("telephone")}
  };
  return attributeNames.value(attributeID);
}

QString attributeName(Attribute::Tag attributeID)
{
  static const QMap<Attribute::Tag, QString> attributeNames {
    {Attribute::Tag::Name,     QStringLiteral("name")},
    {Attribute::Tag::Type,     QStringLiteral("type")},
    {Attribute::Tag::TagColor, QStringLiteral("tagcolor")},
    {Attribute::Tag::Closed,   QStringLiteral("closed")},
    {Attribute::Tag::Notes,    QStringLiteral("notes")},
  };
  return attributeNames.value(attributeID);
}

QString attributeName(Attribute::Security attributeID)
{
  static const QMap<Attribute::Security, QString> attributeNames {
    {Attribute::Security::Name,             QStringLiteral("name")},
    {Attribute::Security::Symbol,           QStringLiteral("symbol")},
    {Attribute::Security::Type,             QStringLiteral("type")},
    {Attribute::Security::RoundingMethod,   QStringLiteral("rounding-method")},
    {Attribute::Security::SAF,              QStringLiteral("saf")},
    {Attribute::Security::PP,               QStringLiteral("pp")},
    {Attribute::Security::SCF,              QStringLiteral("scf")},
    {Attribute::Security::TradingCurrency,  QStringLiteral("trading-currency")},
    {Attribute::Security::TradingMarket,    QStringLiteral("trading-market")}
  };
  return attributeNames.value(attributeID);
}

QString elementName(Element::KVP elementID)
{
  static const QMap<Element::KVP, QString> elementNames {
    {Element::KVP::Pair, QStringLiteral("PAIR")}
  };
  return elementNames.value(elementID);
}

QString attributeName(Attribute::KVP attributeID)
{
  static const QMap<Attribute::KVP, QString> attributeNames {
    {Attribute::KVP::Key,   QStringLiteral("key")},
    {Attribute::KVP::Value, QStringLiteral("value")}
  };
  return attributeNames.value(attributeID);
}

QString elementName(Element::Institution elementID)
{
  static const QMap<Element::Institution, QString> elementNames {
    {Element::Institution::AccountID,  QStringLiteral("ACCOUNTID")},
    {Element::Institution::AccountIDS, QStringLiteral("ACCOUNTIDS")},
    {Element::Institution::Address,    QStringLiteral("ADDRESS")}
  };
  return elementNames.value(elementID);
}

QString attributeName(Attribute::Institution attributeID)
{
  static const QMap<Attribute::Institution, QString> attributeNames {
    {Attribute::Institution::ID,         QStringLiteral("id")},
    {Attribute::Institution::Name,       QStringLiteral("name")},
    {Attribute::Institution::Manager,    QStringLiteral("manager")},
    {Attribute::Institution::SortCode,   QStringLiteral("sortcode")},
    {Attribute::Institution::Street,     QStringLiteral("street")},
    {Attribute::Institution::City,       QStringLiteral("city")},
    {Attribute::Institution::Zip,        QStringLiteral("zip")},
    {Attribute::Institution::Telephone,  QStringLiteral("telephone")}
  };
  return attributeNames.value(attributeID);
}

QString elementName(Element::Report elementID)
{
  static const QMap<Element::Report, QString> elementNames {
    {Element::Report::Payee,        QStringLiteral("PAYEE")},
    {Element::Report::Tag,          QStringLiteral("TAG")},
    {Element::Report::Account,      QStringLiteral("ACCOUNT")},
    {Element::Report::Text,         QStringLiteral("TEXT")},
    {Element::Report::Type,         QStringLiteral("TYPE")},
    {Element::Report::State,        QStringLiteral("STATE")},
    {Element::Report::Number,       QStringLiteral("NUMBER")},
    {Element::Report::Amount,       QStringLiteral("AMOUNT")},
    {Element::Report::Dates,        QStringLiteral("DATES")},
    {Element::Report::Category,     QStringLiteral("CATEGORY")},
    {Element::Report::AccountGroup, QStringLiteral("ACCOUNTGROUP")}
  };
  return elementNames.value(elementID);
}

QString attributeName(Attribute::Report attributeID)
{
  static const QMap<Attribute::Report, QString> attributeNames {
    {Attribute::Report::ID,                     QStringLiteral("id")},
    {Attribute::Report::Group,                  QStringLiteral("group")},
    {Attribute::Report::Type,                   QStringLiteral("type")},
    {Attribute::Report::Name,                   QStringLiteral("name")},
    {Attribute::Report::Comment,                QStringLiteral("comment")},
    {Attribute::Report::ConvertCurrency,        QStringLiteral("convertcurrency")},
    {Attribute::Report::Favorite,               QStringLiteral("favorite")},
    {Attribute::Report::SkipZero,               QStringLiteral("skipZero")},
    {Attribute::Report::DateLock,               QStringLiteral("datelock")},
    {Attribute::Report::DataLock,               QStringLiteral("datalock")},
    {Attribute::Report::MovingAverageDays,      QStringLiteral("movingaveragedays")},
    {Attribute::Report::IncludesActuals,        QStringLiteral("includesactuals")},
    {Attribute::Report::IncludesForecast,       QStringLiteral("includesforecast")},
    {Attribute::Report::IncludesPrice,          QStringLiteral("includesprice")},
    {Attribute::Report::IncludesAveragePrice,   QStringLiteral("includesaverageprice")},
    {Attribute::Report::IncludesMovingAverage,  QStringLiteral("includesmovingaverage")},
    {Attribute::Report::IncludesSchedules,      QStringLiteral("includeschedules")},
    {Attribute::Report::IncludesTransfers,      QStringLiteral("includestransfers")},
    {Attribute::Report::IncludesUnused,         QStringLiteral("includeunused")},
    {Attribute::Report::MixedTime,              QStringLiteral("mixedtime")},
    {Attribute::Report::Investments,            QStringLiteral("investments")},
    {Attribute::Report::Budget,                 QStringLiteral("budget")},
    {Attribute::Report::ShowRowTotals,          QStringLiteral("showrowtotals")},
    {Attribute::Report::ShowColumnTotals,       QStringLiteral("showcolumntotals")},
    {Attribute::Report::Detail,                 QStringLiteral("detail")},
    {Attribute::Report::ColumnsAreDays,         QStringLiteral("columnsaredays")},
    {Attribute::Report::ChartType,              QStringLiteral("charttype")},
    {Attribute::Report::ChartCHGridLines,       QStringLiteral("chartchgridlines")},
    {Attribute::Report::ChartSVGridLines,       QStringLiteral("chartsvgridlines")},
    {Attribute::Report::ChartDataLabels,        QStringLiteral("chartdatalabels")},
    {Attribute::Report::ChartByDefault,         QStringLiteral("chartbydefault")},
    {Attribute::Report::LogYAxis,               QStringLiteral("logYaxis")},
    {Attribute::Report::ChartLineWidth,         QStringLiteral("chartlinewidth")},
    {Attribute::Report::ColumnType,             QStringLiteral("columntype")},
    {Attribute::Report::RowType,                QStringLiteral("rowtype")},
    {Attribute::Report::DataRangeStart,         QStringLiteral("dataRangeStart")},
    {Attribute::Report::DataRangeEnd,           QStringLiteral("dataRangeEnd")},
    {Attribute::Report::DataMajorTick,          QStringLiteral("dataMajorTick")},
    {Attribute::Report::DataMinorTick,          QStringLiteral("dataMinorTick")},
    {Attribute::Report::YLabelsPrecision,       QStringLiteral("yLabelsPrecision")},
    {Attribute::Report::QueryColumns,           QStringLiteral("querycolumns")},
    {Attribute::Report::Tax,                    QStringLiteral("tax")},
    {Attribute::Report::Loans,                  QStringLiteral("loans")},
    {Attribute::Report::HideTransactions,       QStringLiteral("hidetransactions")},
    {Attribute::Report::InvestmentSum,          QStringLiteral("investmentsum")},
    {Attribute::Report::SettlementPeriod,       QStringLiteral("settlementperiod")},
    {Attribute::Report::ShowSTLTCapitalGains,   QStringLiteral("showSTLTCapitalGains")},
    {Attribute::Report::TermsSeparator,         QStringLiteral("tseparator")},
    {Attribute::Report::Pattern,                QStringLiteral("pattern")},
    {Attribute::Report::CaseSensitive,          QStringLiteral("casesensitive")},
    {Attribute::Report::RegEx,                  QStringLiteral("regex")},
    {Attribute::Report::InvertText,             QStringLiteral("inverttext")},
    {Attribute::Report::State,                  QStringLiteral("state")},
    {Attribute::Report::From,                   QStringLiteral("from")},
    {Attribute::Report::To,                     QStringLiteral("to")}
  };
  return attributeNames.value(attributeID);
}

QString elementName(Element::Budget elementID)
{
  static const QMap<Element::Budget, QString> elementNames {
    {Element::Budget::Budget,   QStringLiteral("BUDGET")},
    {Element::Budget::Account,  QStringLiteral("ACCOUNT")},
    {Element::Budget::Period,   QStringLiteral("PERIOD")}
  };
  return elementNames.value(elementID);
}

QString attributeName(Attribute::Budget attributeID)
{
  static const QMap<Attribute::Budget, QString> attributeNames {
    {Attribute::Budget::ID,                 QStringLiteral("id")},
    {Attribute::Budget::Name,               QStringLiteral("name")},
    {Attribute::Budget::Start,              QStringLiteral("start")},
    {Attribute::Budget::Version,            QStringLiteral("version")},
    {Attribute::Budget::BudgetLevel,        QStringLiteral("budgetlevel")},
    {Attribute::Budget::BudgetSubAccounts,  QStringLiteral("budgetsubaccounts")},
    {Attribute::Budget::Amount,             QStringLiteral("amount")}
  };
  return attributeNames.value(attributeID);
}

QString elementName(Element::Schedule elementID)
{
  static const QMap<Element::Schedule, QString> elementNames {
    {Element::Schedule::Payment,  QStringLiteral("PAYMENT")},
    {Element::Schedule::Payments, QStringLiteral("PAYMENTS")}
  };
  return elementNames.value(elementID);
}

QString attributeName(Attribute::Schedule attributeID)
{
  static const QMap<Attribute::Schedule, QString> attributeNames {
    {Attribute::Schedule::Name,                 QStringLiteral("name")},
    {Attribute::Schedule::Type,                 QStringLiteral("type")},
    {Attribute::Schedule::Occurrence,           QStringLiteral("occurence")}, // krazy:exclude=spelling
    {Attribute::Schedule::OccurrenceMultiplier, QStringLiteral("occurenceMultiplier")}, // krazy:exclude=spelling
    {Attribute::Schedule::PaymentType,          QStringLiteral("paymentType")},
    {Attribute::Schedule::Fixed,                QStringLiteral("fixed")},
    {Attribute::Schedule::AutoEnter,            QStringLiteral("autoEnter")},
    {Attribute::Schedule::LastPayment,          QStringLiteral("lastPayment")},
    {Attribute::Schedule::WeekendOption,        QStringLiteral("weekendOption")},
    {Attribute::Schedule::Date,                 QStringLiteral("date")},
    {Attribute::Schedule::StartDate,            QStringLiteral("startDate")},
    {Attribute::Schedule::EndDate,              QStringLiteral("endDate")},
    {Attribute::Schedule::LastDayInMonth,       QStringLiteral("lastDayInMonth")}
  };
  return attributeNames.value(attributeID);
}

QString elementName(Element::OnlineJob elementID)
{
  static const QMap<Element::OnlineJob, QString> elementNames {
    {Element::OnlineJob::OnlineTask, QStringLiteral("onlineTask")}
  };
  return elementNames.value(elementID);
}

QString attributeName(Attribute::OnlineJob attributeID)
{
  static const QMap<Attribute::OnlineJob, QString> attributeNames {
    {Attribute::OnlineJob::Send,             QStringLiteral("send")},
    {Attribute::OnlineJob::BankAnswerDate,   QStringLiteral("bankAnswerDate")},
    {Attribute::OnlineJob::BankAnswerState,  QStringLiteral("bankAnswerState")},
    {Attribute::OnlineJob::IID,              QStringLiteral("iid")},
    {Attribute::OnlineJob::AbortedByUser,    QStringLiteral("abortedByUser")},
    {Attribute::OnlineJob::AcceptedByBank,   QStringLiteral("acceptedByBank")},
    {Attribute::OnlineJob::RejectedByBank,   QStringLiteral("rejectedByBank")},
    {Attribute::OnlineJob::SendingError,     QStringLiteral("sendingError")},
  };
  return attributeNames.value(attributeID);
}

QString attributeName(Attribute::CostCenter attributeID)
{
  static const QMap<Attribute::CostCenter, QString> attributeNames {
    {Attribute::CostCenter::Name, QStringLiteral("name")},
  };
  return attributeNames.value(attributeID);
}

QHash<eMyMoney::Report::RowType, QString> rowTypesLUT()
{
  static const QHash<eMyMoney::Report::RowType, QString> lut {
    {eMyMoney::Report::RowType::NoRows,               QStringLiteral("none")},
    {eMyMoney::Report::RowType::AssetLiability,       QStringLiteral("assetliability")},
    {eMyMoney::Report::RowType::ExpenseIncome,        QStringLiteral("expenseincome")},
    {eMyMoney::Report::RowType::Category,             QStringLiteral("category")},
    {eMyMoney::Report::RowType::TopCategory,          QStringLiteral("topcategory")},
    {eMyMoney::Report::RowType::Account,              QStringLiteral("account")},
    {eMyMoney::Report::RowType::Tag,                  QStringLiteral("tag")},
    {eMyMoney::Report::RowType::Payee,                QStringLiteral("payee")},
    {eMyMoney::Report::RowType::Month,                QStringLiteral("month")},
    {eMyMoney::Report::RowType::Week,                 QStringLiteral("week")},
    {eMyMoney::Report::RowType::TopAccount,           QStringLiteral("topaccount")},
    {eMyMoney::Report::RowType::AccountByTopAccount,  QStringLiteral("topaccount-account")},
    {eMyMoney::Report::RowType::EquityType,           QStringLiteral("equitytype")},
    {eMyMoney::Report::RowType::AccountType,          QStringLiteral("accounttype")},
    {eMyMoney::Report::RowType::Institution,          QStringLiteral("institution")},
    {eMyMoney::Report::RowType::Budget,               QStringLiteral("budget")},
    {eMyMoney::Report::RowType::BudgetActual,         QStringLiteral("budgetactual")},
    {eMyMoney::Report::RowType::Schedule,             QStringLiteral("schedule")},
    {eMyMoney::Report::RowType::AccountInfo,          QStringLiteral("accountinfo")},
    {eMyMoney::Report::RowType::AccountLoanInfo,      QStringLiteral("accountloaninfo")},
    {eMyMoney::Report::RowType::AccountReconcile,     QStringLiteral("accountreconcile")},
    {eMyMoney::Report::RowType::CashFlow,             QStringLiteral("cashflow")},
  };
  return lut;
}

QString reportNames(eMyMoney::Report::RowType textID)
{
  return rowTypesLUT().value(textID);
}

eMyMoney::Report::RowType stringToRowType(const QString &text)
{
  return rowTypesLUT().key(text, eMyMoney::Report::RowType::Invalid);
}

QHash<eMyMoney::Report::ColumnType, QString> columTypesLUT()
{
  static const QHash<eMyMoney::Report::ColumnType, QString> lut {
    {eMyMoney::Report::ColumnType::NoColumns, QStringLiteral("none")},
    {eMyMoney::Report::ColumnType::Months,    QStringLiteral("months")},
    {eMyMoney::Report::ColumnType::BiMonths,  QStringLiteral("bimonths")},
    {eMyMoney::Report::ColumnType::Quarters,  QStringLiteral("quarters")},
//      {eMyMoney::Report::ColumnType::,        QStringLiteral("4")}
//      {eMyMoney::Report::ColumnType::,        QStringLiteral("5")}
//      {eMyMoney::Report::ColumnType::,        QStringLiteral("6")}
    {eMyMoney::Report::ColumnType::Weeks,     QStringLiteral("weeks")},
//      {eMyMoney::Report::ColumnType::,        QStringLiteral("8")}
//      {eMyMoney::Report::ColumnType::,        QStringLiteral("9")}
//      {eMyMoney::Report::ColumnType::,        QStringLiteral("10")}
//      {eMyMoney::Report::ColumnType::,        QStringLiteral("11")}
    {eMyMoney::Report::ColumnType::Years,     QStringLiteral("years")}
  };
  return lut;
}

QString reportNames(eMyMoney::Report::ColumnType textID)
{
  return columTypesLUT().value(textID);
}

eMyMoney::Report::ColumnType stringToColumnType(const QString &text)
{
  return columTypesLUT().key(text, eMyMoney::Report::ColumnType::Invalid);
}

QHash<eMyMoney::Report::QueryColumn, QString> queryColumnsLUT()
{
  static const QHash<eMyMoney::Report::QueryColumn, QString> lut {
    {eMyMoney::Report::QueryColumn::None,        QStringLiteral("none")},
    {eMyMoney::Report::QueryColumn::Number,      QStringLiteral("number")},
    {eMyMoney::Report::QueryColumn::Payee,       QStringLiteral("payee")},
    {eMyMoney::Report::QueryColumn::Category,    QStringLiteral("category")},
    {eMyMoney::Report::QueryColumn::Tag,         QStringLiteral("tag")},
    {eMyMoney::Report::QueryColumn::Memo,        QStringLiteral("memo")},
    {eMyMoney::Report::QueryColumn::Account,     QStringLiteral("account")},
    {eMyMoney::Report::QueryColumn::Reconciled,  QStringLiteral("reconcileflag")},
    {eMyMoney::Report::QueryColumn::Action,      QStringLiteral("action")},
    {eMyMoney::Report::QueryColumn::Shares,      QStringLiteral("shares")},
    {eMyMoney::Report::QueryColumn::Price,       QStringLiteral("price")},
    {eMyMoney::Report::QueryColumn::Performance, QStringLiteral("performance")},
    {eMyMoney::Report::QueryColumn::Loan,        QStringLiteral("loan")},
    {eMyMoney::Report::QueryColumn::Balance,     QStringLiteral("balance")},
    {eMyMoney::Report::QueryColumn::CapitalGain, QStringLiteral("capitalgain")}
  };
  return lut;
}

QString reportNamesForQC(eMyMoney::Report::QueryColumn textID)
{
  return queryColumnsLUT().value(textID);
}

eMyMoney::Report::QueryColumn stringToQueryColumn(const QString &text)
{
  return queryColumnsLUT().key(text, eMyMoney::Report::QueryColumn::End);
}

QHash<eMyMoney::Report::DetailLevel, QString> detailLevelLUT()
{
  static const QHash<eMyMoney::Report::DetailLevel, QString> lut {
    {eMyMoney::Report::DetailLevel::None,   QStringLiteral("none")},
    {eMyMoney::Report::DetailLevel::All,    QStringLiteral("all")},
    {eMyMoney::Report::DetailLevel::Top,    QStringLiteral("top")},
    {eMyMoney::Report::DetailLevel::Group,  QStringLiteral("group")},
    {eMyMoney::Report::DetailLevel::Total,  QStringLiteral("total")},
    {eMyMoney::Report::DetailLevel::End,    QStringLiteral("invalid")}
  };
  return lut;
}

QString reportNames(eMyMoney::Report::DetailLevel textID)
{
  return detailLevelLUT().value(textID);
}

eMyMoney::Report::DetailLevel stringToDetailLevel(const QString &text)
{
  return detailLevelLUT().key(text, eMyMoney::Report::DetailLevel::End);
}

QHash<eMyMoney::Report::ChartType, QString> chartTypeLUT()
{
  static const QHash<eMyMoney::Report::ChartType, QString> lut {
    {eMyMoney::Report::ChartType::None,       QStringLiteral("none")},
    {eMyMoney::Report::ChartType::Line,       QStringLiteral("line")},
    {eMyMoney::Report::ChartType::Bar,        QStringLiteral("bar")},
    {eMyMoney::Report::ChartType::Pie,        QStringLiteral("pie")},
    {eMyMoney::Report::ChartType::Ring,       QStringLiteral("ring")},
    {eMyMoney::Report::ChartType::StackedBar, QStringLiteral("stackedbar")}
  };
  return lut;
}

QString reportNames(eMyMoney::Report::ChartType textID)
{
  return chartTypeLUT().value(textID);
}

eMyMoney::Report::ChartType stringToChartType(const QString &text)
{
  return chartTypeLUT().key(text, eMyMoney::Report::ChartType::End);
}

QHash<int, QString> typeAttributeLUT()
{
  static const QHash<int, QString> lut {
    {0, QStringLiteral("all")},
    {1, QStringLiteral("payments")},
    {2, QStringLiteral("deposits")},
    {3, QStringLiteral("transfers")},
    {4, QStringLiteral("none")},
  };
  return lut;
}

QString typeAttributeToString(int textID)
{
  return typeAttributeLUT().value(textID);
}

int stringToTypeAttribute(const QString &text)
{
  return typeAttributeLUT().key(text, 4);
}

QHash<int, QString> stateAttributeLUT()
{
  static const QHash<int, QString> lut {
    {0, QStringLiteral("all")},
    {1, QStringLiteral("notreconciled")},
    {2, QStringLiteral("cleared")},
    {3, QStringLiteral("reconciled")},
    {4, QStringLiteral("frozen")},
    {5, QStringLiteral("none")}
  };
  return lut;
}

QString stateAttributeToString(int textID)
{
  return stateAttributeLUT().value(textID);
}

int stringToStateAttribute(const QString &text)
{
  return stateAttributeLUT().key(text, 5);
}

QHash<int, QString> dateLockLUT()
{
  static const QHash<int, QString> lut {
    {0, QStringLiteral("alldates")},
    {1, QStringLiteral("untiltoday")},
    {2, QStringLiteral("currentmonth")},
    {3, QStringLiteral("currentyear")},
    {4, QStringLiteral("monthtodate")},
    {5, QStringLiteral("yeartodate")},
    {6, QStringLiteral("yeartomonth")},
    {7, QStringLiteral("lastmonth")},
    {8, QStringLiteral("lastyear")},
    {9, QStringLiteral("last7days")},
    {10, QStringLiteral("last30days")},
    {11, QStringLiteral("last3months")},
    {12, QStringLiteral("last6months")},
    {13, QStringLiteral("last12months")},
    {14, QStringLiteral("next7days")},
    {15, QStringLiteral("next30days")},
    {16, QStringLiteral("next3months")},
    {17, QStringLiteral("next6months")},
    {18, QStringLiteral("next12months")},
    {19, QStringLiteral("userdefined")},
    {20, QStringLiteral("last3tonext3months")},
    {21, QStringLiteral("last11Months")},
    {22, QStringLiteral("currentQuarter")},
    {23, QStringLiteral("lastQuarter")},
    {24, QStringLiteral("nextQuarter")},
    {25, QStringLiteral("currentFiscalYear")},
    {26, QStringLiteral("lastFiscalYear")},
    {27, QStringLiteral("today")},
    {28, QStringLiteral("next18months")}
  };
  return lut;
}

QString dateLockAttributeToString(int textID)
{
  return dateLockLUT().value(textID);
}

int stringToDateLockAttribute(const QString &text)
{
  return dateLockLUT().key(text, 0);
}

QHash<eMyMoney::Report::DataLock, QString> dataLockLUT()
{
  static const QHash<eMyMoney::Report::DataLock, QString> lut {
    {eMyMoney::Report::DataLock::Automatic,   QStringLiteral("automatic")},
    {eMyMoney::Report::DataLock::UserDefined, QStringLiteral("userdefined")}
  };
  return lut;
}

QString reportNames(eMyMoney::Report::DataLock textID)
{
  return dataLockLUT().value(textID);
}

eMyMoney::Report::DataLock stringToDataLockAttribute(const QString &text)
{
  return dataLockLUT().key(text, eMyMoney::Report::DataLock::DataOptionCount);
}

QHash<int, QString> accountTypeAttributeLUT()
{
  static const QHash<int, QString> lut {
    {0, QStringLiteral("unknown")},
    {1, QStringLiteral("checkings")},
    {2, QStringLiteral("savings")},
    {3, QStringLiteral("cash")},
    {4, QStringLiteral("creditcard")},
    {5, QStringLiteral("loan")},
    {6, QStringLiteral("certificatedep")},
    {7, QStringLiteral("investment")},
    {8, QStringLiteral("moneymarket")},
    {10, QStringLiteral("asset")},
    {11, QStringLiteral("liability")},
    {12, QStringLiteral("currency")},
    {13, QStringLiteral("income")},
    {14, QStringLiteral("expense")},
    {15, QStringLiteral("assetloan")},
    {16, QStringLiteral("stock")},
    {17, QStringLiteral("equity")},
    {18, QStringLiteral("invalid")}
  };
  return lut;
}

QString accountTypeAttributeToString(int textID)
{
  return accountTypeAttributeLUT().value(textID);
}

int stringToAccountTypeAttribute(const QString &text)
{
  return accountTypeAttributeLUT().key(text, 0);
}

eMyMoney::Report::ReportType rowTypeToReportType(eMyMoney::Report::RowType rowType)
{
  static const QHash<eMyMoney::Report::RowType, eMyMoney::Report::ReportType> reportTypes {
    {eMyMoney::Report::RowType::NoRows,               eMyMoney::Report::ReportType::NoReport},
    {eMyMoney::Report::RowType::AssetLiability,       eMyMoney::Report::ReportType::PivotTable},
    {eMyMoney::Report::RowType::ExpenseIncome,        eMyMoney::Report::ReportType::PivotTable},
    {eMyMoney::Report::RowType::Category,             eMyMoney::Report::ReportType::QueryTable},
    {eMyMoney::Report::RowType::TopCategory,          eMyMoney::Report::ReportType::QueryTable},
    {eMyMoney::Report::RowType::Account,              eMyMoney::Report::ReportType::QueryTable},
    {eMyMoney::Report::RowType::Tag,                  eMyMoney::Report::ReportType::QueryTable},
    {eMyMoney::Report::RowType::Payee,                eMyMoney::Report::ReportType::QueryTable},
    {eMyMoney::Report::RowType::Month,                eMyMoney::Report::ReportType::QueryTable},
    {eMyMoney::Report::RowType::Week,                 eMyMoney::Report::ReportType::QueryTable},
    {eMyMoney::Report::RowType::TopAccount,           eMyMoney::Report::ReportType::QueryTable},
    {eMyMoney::Report::RowType::AccountByTopAccount,  eMyMoney::Report::ReportType::QueryTable},
    {eMyMoney::Report::RowType::EquityType,           eMyMoney::Report::ReportType::QueryTable},
    {eMyMoney::Report::RowType::AccountType,          eMyMoney::Report::ReportType::QueryTable},
    {eMyMoney::Report::RowType::Institution,          eMyMoney::Report::ReportType::QueryTable},
    {eMyMoney::Report::RowType::Budget,               eMyMoney::Report::ReportType::PivotTable},
    {eMyMoney::Report::RowType::BudgetActual,         eMyMoney::Report::ReportType::PivotTable},
    {eMyMoney::Report::RowType::Schedule,             eMyMoney::Report::ReportType::InfoTable},
    {eMyMoney::Report::RowType::AccountInfo,          eMyMoney::Report::ReportType::InfoTable},
    {eMyMoney::Report::RowType::AccountLoanInfo,      eMyMoney::Report::ReportType::InfoTable},
    {eMyMoney::Report::RowType::AccountReconcile,     eMyMoney::Report::ReportType::QueryTable},
    {eMyMoney::Report::RowType::CashFlow,             eMyMoney::Report::ReportType::QueryTable},
  };
  return reportTypes.value(rowType, eMyMoney::Report::ReportType::Invalid);
}

QHash<eMyMoney::Budget::Level, QString> budgetLevelLUT()
{
  static const QHash<eMyMoney::Budget::Level, QString> lut {
    {eMyMoney::Budget::Level::None,         QStringLiteral("none")},
    {eMyMoney::Budget::Level::Monthly,      QStringLiteral("monthly")},
    {eMyMoney::Budget::Level::MonthByMonth, QStringLiteral("monthbymonth")},
    {eMyMoney::Budget::Level::Yearly,       QStringLiteral("yearly")},
    {eMyMoney::Budget::Level::Max,          QStringLiteral("invalid")},
  };
  return lut;
}

QString budgetNames(eMyMoney::Budget::Level textID)
{
  return budgetLevelLUT().value(textID);
}

eMyMoney::Budget::Level stringToBudgetLevel(const QString &text)
{
  return budgetLevelLUT().key(text, eMyMoney::Budget::Level::Max);
}
