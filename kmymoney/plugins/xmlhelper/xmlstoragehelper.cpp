/*
 * Copyright 2004-2006  Ace Jones <acejones@users.sourceforge.net>
 * Copyright 2006       Darren Gould <darren_gould@gmx.de>
 * Copyright 2007-2010  Alvaro Soliverez <asoliverez@gmail.com>
 * Copyright 2017-2018  Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 * Copyright 2019       Thomas Baumgart <tbaumgart@kde.org>
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

#include "xmlstoragehelper.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDomElement>
#include <QDomDocument>
#include <QDate>
#include <QMap>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneymoney.h"
#include "mymoneybudget.h"
#include "mymoneyreport.h"
#include "mymoneytransactionfilter.h"
#include "mymoneyenums.h"
#include "mymoneyexception.h"

namespace Element {

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
    AccountGroup,
    Validity
  };

  enum class Budget {
    Budget = 0,
    Account,
    Period
  };
}

namespace Attribute {

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
    Validity,
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

}

namespace MyMoneyXmlContentHandler2 {

  enum class Node {
    Report,
    Budget
  };

  QString nodeName(Node nodeID)
  {
    static const QHash<Node, QString> nodeNames {
      {Node::Report,        QStringLiteral("REPORT")},
      {Node::Budget,        QStringLiteral("BUDGET")}
    };
    return nodeNames.value(nodeID);
  }

  uint qHash(const Node key, uint seed) { return ::qHash(static_cast<uint>(key), seed); }

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
      {Element::Report::AccountGroup, QStringLiteral("ACCOUNTGROUP")},
      {Element::Report::Validity,     QStringLiteral("VALIDITY")}
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
      {Attribute::Report::To,                     QStringLiteral("to")},
      {Attribute::Report::Validity,               QStringLiteral("validity")}
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

  QHash<int, QString> validityAttributeLUT()
  {
    static const QHash<int, QString> lut {
      {0, QStringLiteral("any")},
      {1, QStringLiteral("valid")},
      {2, QStringLiteral("invalid")},
    };
    return lut;
  }

  QString validityAttributeToString(int textID)
  {
    return validityAttributeLUT().value(textID);
  }

  int stringToValidityAttribute(const QString &text)
  {
    return validityAttributeLUT().key(text, 0);
  }

  QHash<eMyMoney::TransactionFilter::Date, QString> dateLockLUT()
  {
    static const QHash<eMyMoney::TransactionFilter::Date, QString> lut {
      {eMyMoney::TransactionFilter::Date::All,                QStringLiteral("alldates")},
      {eMyMoney::TransactionFilter::Date::AsOfToday,          QStringLiteral("untiltoday")},
      {eMyMoney::TransactionFilter::Date::CurrentMonth,       QStringLiteral("currentmonth")},
      {eMyMoney::TransactionFilter::Date::CurrentYear,        QStringLiteral("currentyear")},
      {eMyMoney::TransactionFilter::Date::MonthToDate,        QStringLiteral("monthtodate")},
      {eMyMoney::TransactionFilter::Date::YearToDate,         QStringLiteral("yeartodate")},
      {eMyMoney::TransactionFilter::Date::YearToMonth,        QStringLiteral("yeartomonth")},
      {eMyMoney::TransactionFilter::Date::LastMonth,          QStringLiteral("lastmonth")},
      {eMyMoney::TransactionFilter::Date::LastYear,           QStringLiteral("lastyear")},
      {eMyMoney::TransactionFilter::Date::Last7Days,          QStringLiteral("last7days")},
      {eMyMoney::TransactionFilter::Date::Last30Days,         QStringLiteral("last30days")},
      {eMyMoney::TransactionFilter::Date::Last3Months,        QStringLiteral("last3months")},
      {eMyMoney::TransactionFilter::Date::Last6Months,        QStringLiteral("last6months")},
      {eMyMoney::TransactionFilter::Date::Last12Months,       QStringLiteral("last12months")},
      {eMyMoney::TransactionFilter::Date::Next7Days,          QStringLiteral("next7days")},
      {eMyMoney::TransactionFilter::Date::Next30Days,         QStringLiteral("next30days")},
      {eMyMoney::TransactionFilter::Date::Next3Months,        QStringLiteral("next3months")},
      {eMyMoney::TransactionFilter::Date::Next6Months,        QStringLiteral("next6months")},
      {eMyMoney::TransactionFilter::Date::Next12Months,       QStringLiteral("next12months")},
      {eMyMoney::TransactionFilter::Date::UserDefined,        QStringLiteral("userdefined")},
      {eMyMoney::TransactionFilter::Date::Last3ToNext3Months, QStringLiteral("last3tonext3months")},
      {eMyMoney::TransactionFilter::Date::Last11Months,       QStringLiteral("last11Months")},
      {eMyMoney::TransactionFilter::Date::CurrentQuarter,     QStringLiteral("currentQuarter")},
      {eMyMoney::TransactionFilter::Date::LastQuarter,        QStringLiteral("lastQuarter")},
      {eMyMoney::TransactionFilter::Date::NextQuarter,        QStringLiteral("nextQuarter")},
      {eMyMoney::TransactionFilter::Date::CurrentFiscalYear,  QStringLiteral("currentFiscalYear")},
      {eMyMoney::TransactionFilter::Date::LastFiscalYear,     QStringLiteral("lastFiscalYear")},
      {eMyMoney::TransactionFilter::Date::Today,              QStringLiteral("today")},
      {eMyMoney::TransactionFilter::Date::Next18Months,       QStringLiteral("next18months")}
    };
    return lut;
  }

  QString dateLockAttributeToString(eMyMoney::TransactionFilter::Date textID)
  {
    return dateLockLUT().value(textID);
  }

  eMyMoney::TransactionFilter::Date stringToDateLockAttribute(const QString &text)
  {
    return dateLockLUT().key(text, eMyMoney::TransactionFilter::Date::UserDefined);
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

  QHash<eMyMoney::Account::Type, QString> accountTypeAttributeLUT()
  {
    static const QHash<eMyMoney::Account::Type, QString> lut {
      {eMyMoney::Account::Type::Unknown,        QStringLiteral("unknown")},
      {eMyMoney::Account::Type::Checkings,      QStringLiteral("checkings")},
      {eMyMoney::Account::Type::Savings,        QStringLiteral("savings")},
      {eMyMoney::Account::Type::Cash,           QStringLiteral("cash")},
      {eMyMoney::Account::Type::CreditCard,     QStringLiteral("creditcard")},
      {eMyMoney::Account::Type::Loan,           QStringLiteral("loan")},
      {eMyMoney::Account::Type::CertificateDep, QStringLiteral("certificatedep")},
      {eMyMoney::Account::Type::Investment,     QStringLiteral("investment")},
      {eMyMoney::Account::Type::MoneyMarket,    QStringLiteral("moneymarket")},
      {eMyMoney::Account::Type::Asset,          QStringLiteral("asset")},
      {eMyMoney::Account::Type::Liability,      QStringLiteral("liability")},
      {eMyMoney::Account::Type::Currency,       QStringLiteral("currency")},
      {eMyMoney::Account::Type::Income,         QStringLiteral("income")},
      {eMyMoney::Account::Type::Expense,        QStringLiteral("expense")},
      {eMyMoney::Account::Type::AssetLoan,      QStringLiteral("assetloan")},
      {eMyMoney::Account::Type::Stock,          QStringLiteral("stock")},
      {eMyMoney::Account::Type::Equity,         QStringLiteral("equity")},
    };
    return lut;
  }

  QString accountTypeAttributeToString(eMyMoney::Account::Type type)
  {
    return accountTypeAttributeLUT().value(type);
  }

  eMyMoney::Account::Type stringToAccountTypeAttribute(const QString &text)
  {
    return accountTypeAttributeLUT().key(text, eMyMoney::Account::Type::Unknown);
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

  QHash<eMyMoney::Budget::Level, QString> budgetLevelsLUT()
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

  QString budgetLevels(eMyMoney::Budget::Level textID)
  {
    return budgetLevelsLUT().value(textID);
  }

  void writeBaseXML(const QString &id, QDomDocument &document, QDomElement &el)
  {
    Q_UNUSED(document);

    el.setAttribute(QStringLiteral("id"), id);
  }

  MyMoneyReport readReport(const QDomElement &node)
  {
    if (nodeName(Node::Report) != node.tagName())
      throw MYMONEYEXCEPTION_CSTRING("Node was not REPORT");

    MyMoneyReport report(node.attribute(attributeName(Attribute::Report::ID)));

      // The goal of this reading method is 100% backward AND 100% forward
      // compatibility.  Any report ever created with any version of KMyMoney
      // should be able to be loaded by this method (as long as it's one of the
      // report types supported in this version, of course)


      // read report's internals
      QString type = node.attribute(attributeName(Attribute::Report::Type));
      if (type.startsWith(QLatin1String("pivottable")))
        report.setReportType(eMyMoney::Report::ReportType::PivotTable);
      else if (type.startsWith(QLatin1String("querytable")))
        report.setReportType(eMyMoney::Report::ReportType::QueryTable);
      else if (type.startsWith(QLatin1String("infotable")))
        report.setReportType(eMyMoney::Report::ReportType::InfoTable);
      else
        throw MYMONEYEXCEPTION_CSTRING("Unknown report type");

      report.setGroup(node.attribute(attributeName(Attribute::Report::Group)));

      report.clearTransactionFilter();

      // read date tab
      QString datelockstr = node.attribute(attributeName(Attribute::Report::DateLock), "userdefined");
      // Handle the pivot 1.2/query 1.1 case where the values were saved as
      // numbers
      bool ok = false;
      eMyMoney::TransactionFilter::Date dateLock = static_cast<eMyMoney::TransactionFilter::Date>(datelockstr.toUInt(&ok));
      if (!ok) {
        dateLock = stringToDateLockAttribute(datelockstr);
      }
      report.setDateFilter(dateLock);

      // read general tab
      report.setName(node.attribute(attributeName(Attribute::Report::Name)));
      report.setComment(node.attribute(attributeName(Attribute::Report::Comment), "Extremely old report"));
      report.setConvertCurrency(node.attribute(attributeName(Attribute::Report::ConvertCurrency), "1").toUInt());
      report.setFavorite(node.attribute(attributeName(Attribute::Report::Favorite), "0").toUInt());
      report.setSkipZero(node.attribute(attributeName(Attribute::Report::SkipZero), "0").toUInt());
      const auto rowTypeFromXML = stringToRowType(node.attribute(attributeName(Attribute::Report::RowType)));

      if (report.reportType() == eMyMoney::Report::ReportType::PivotTable) {
        // read report's internals
        report.setIncludingBudgetActuals(node.attribute(attributeName(Attribute::Report::IncludesActuals), "0").toUInt());
        report.setIncludingForecast(node.attribute(attributeName(Attribute::Report::IncludesForecast), "0").toUInt());
        report.setIncludingPrice(node.attribute(attributeName(Attribute::Report::IncludesPrice), "0").toUInt());
        report.setIncludingAveragePrice(node.attribute(attributeName(Attribute::Report::IncludesAveragePrice), "0").toUInt());
        report.setMixedTime(node.attribute(attributeName(Attribute::Report::MixedTime), "0").toUInt());
        report.setInvestmentsOnly(node.attribute(attributeName(Attribute::Report::Investments), "0").toUInt());

        // read rows/columns tab
        if (node.hasAttribute(attributeName(Attribute::Report::Budget)))
          report.setBudget(node.attribute(attributeName(Attribute::Report::Budget)));

        if (rowTypeFromXML != eMyMoney::Report::RowType::Invalid)
          report.setRowType(rowTypeFromXML);
        else
          report.setRowType(eMyMoney::Report::RowType::ExpenseIncome);

        if (node.hasAttribute(attributeName(Attribute::Report::ShowRowTotals)))
          report.setShowingRowTotals(node.attribute(attributeName(Attribute::Report::ShowRowTotals)).toUInt());
        else if (report.rowType() == eMyMoney::Report::RowType::ExpenseIncome) // for backward compatibility
          report.setShowingRowTotals(true);
        report.setShowingColumnTotals(node.attribute(attributeName(Attribute::Report::ShowColumnTotals), "1").toUInt());

        //check for reports with older settings which didn't have the detail attribute
        const auto detailLevelFromXML = stringToDetailLevel(node.attribute(attributeName(Attribute::Report::Detail)));
        if (detailLevelFromXML != eMyMoney::Report::DetailLevel::End)
          report.setDetailLevel(detailLevelFromXML);
        else
          report.setDetailLevel(eMyMoney::Report::DetailLevel::All);

        report.setIncludingMovingAverage(node.attribute(attributeName(Attribute::Report::IncludesMovingAverage), "0").toUInt());
        if (report.isIncludingMovingAverage())
          report.setMovingAverageDays(node.attribute(attributeName(Attribute::Report::MovingAverageDays), "1").toUInt());
        report.setIncludingSchedules(node.attribute(attributeName(Attribute::Report::IncludesSchedules), "0").toUInt());
        report.setIncludingTransfers(node.attribute(attributeName(Attribute::Report::IncludesTransfers), "0").toUInt());
        report.setIncludingUnusedAccounts(node.attribute(attributeName(Attribute::Report::IncludesUnused), "0").toUInt());
        report.setColumnsAreDays(node.attribute(attributeName(Attribute::Report::ColumnsAreDays), "0").toUInt());

        // read chart tab
        const auto chartTypeFromXML = stringToChartType(node.attribute(attributeName(Attribute::Report::ChartType)));
        if (chartTypeFromXML != eMyMoney::Report::ChartType::End)
          report.setChartType(chartTypeFromXML);
        else
          report.setChartType(eMyMoney::Report::ChartType::None);

        report.setChartCHGridLines(node.attribute(attributeName(Attribute::Report::ChartCHGridLines), "1").toUInt());
        report.setChartSVGridLines(node.attribute(attributeName(Attribute::Report::ChartSVGridLines), "1").toUInt());
        report.setChartDataLabels(node.attribute(attributeName(Attribute::Report::ChartDataLabels), "1").toUInt());
        report.setChartByDefault(node.attribute(attributeName(Attribute::Report::ChartByDefault), "0").toUInt());
        report.setLogYAxis(node.attribute(attributeName(Attribute::Report::LogYAxis), "0").toUInt());
        report.setChartLineWidth(node.attribute(attributeName(Attribute::Report::ChartLineWidth), QString(MyMoneyReport::lineWidth())).toUInt());

        // read range tab
        const auto columnTypeFromXML = stringToColumnType(node.attribute(attributeName(Attribute::Report::ColumnType)));
        if (columnTypeFromXML != eMyMoney::Report::ColumnType::Invalid)
          report.setColumnType(columnTypeFromXML);
        else
          report.setColumnType(eMyMoney::Report::ColumnType::Months);

        const auto dataLockFromXML = stringToDataLockAttribute(node.attribute(attributeName(Attribute::Report::DataLock)));
        if (dataLockFromXML != eMyMoney::Report::DataLock::DataOptionCount)
          report.setDataFilter(dataLockFromXML);
        else
          report.setDataFilter(eMyMoney::Report::DataLock::Automatic);

        report.setDataRangeStart(node.attribute(attributeName(Attribute::Report::DataRangeStart), "0"));
        report.setDataRangeEnd(node.attribute(attributeName(Attribute::Report::DataRangeEnd), "0"));
        report.setDataMajorTick(node.attribute(attributeName(Attribute::Report::DataMajorTick), "0"));
        report.setDataMinorTick(node.attribute(attributeName(Attribute::Report::DataMinorTick), "0"));
        report.setYLabelsPrecision(node.attribute(attributeName(Attribute::Report::YLabelsPrecision), "2").toUInt());
      } else if (report.reportType() == eMyMoney::Report::ReportType::QueryTable) {
        // read rows/columns tab
        if (rowTypeFromXML != eMyMoney::Report::RowType::Invalid)
          report.setRowType(rowTypeFromXML);
        else
          report.setRowType(eMyMoney::Report::RowType::Account);

        unsigned qc = 0;
        QStringList columns = node.attribute(attributeName(Attribute::Report::QueryColumns), "none").split(',');
        foreach (const auto column, columns) {
          const int queryColumnFromXML = stringToQueryColumn(column);
          if (queryColumnFromXML != eMyMoney::Report::QueryColumn::End)
            qc |= queryColumnFromXML;
        }
        report.setQueryColumns(static_cast<eMyMoney::Report::QueryColumn>(qc));

        report.setTax(node.attribute(attributeName(Attribute::Report::Tax), "0").toUInt());
        report.setInvestmentsOnly(node.attribute(attributeName(Attribute::Report::Investments), "0").toUInt());
        report.setLoansOnly(node.attribute(attributeName(Attribute::Report::Loans), "0").toUInt());
        report.setHideTransactions(node.attribute(attributeName(Attribute::Report::HideTransactions), "0").toUInt());
        report.setShowingColumnTotals(node.attribute(attributeName(Attribute::Report::ShowColumnTotals), "1").toUInt());
        const auto detailLevelFromXML = stringToDetailLevel(node.attribute(attributeName(Attribute::Report::Detail), "none"));
        if (detailLevelFromXML == eMyMoney::Report::DetailLevel::All)
          report.setDetailLevel(detailLevelFromXML);
        else
          report.setDetailLevel(eMyMoney::Report::DetailLevel::None);

        // read performance or capital gains tab
        if (report.queryColumns() & eMyMoney::Report::QueryColumn::Performance)
          report.setInvestmentSum(static_cast<eMyMoney::Report::InvestmentSum>(node.attribute(attributeName(Attribute::Report::InvestmentSum), QString::number(static_cast<int>(eMyMoney::Report::InvestmentSum::Period))).toInt()));

        // read capital gains tab
        if (report.queryColumns() & eMyMoney::Report::QueryColumn::CapitalGain) {
          report.setInvestmentSum(static_cast<eMyMoney::Report::InvestmentSum>(node.attribute(attributeName(Attribute::Report::InvestmentSum), QString::number(static_cast<int>(eMyMoney::Report::InvestmentSum::Sold))).toInt()));
          if (report.investmentSum() == eMyMoney::Report::InvestmentSum::Sold) {
            report.setShowSTLTCapitalGains(node.attribute(attributeName(Attribute::Report::ShowSTLTCapitalGains), "0").toUInt());
            report.setSettlementPeriod(node.attribute(attributeName(Attribute::Report::SettlementPeriod), "3").toUInt());
            report.setTermSeparator(QDate::fromString(node.attribute(attributeName(Attribute::Report::TermsSeparator), QDate::currentDate().addYears(-1).toString(Qt::ISODate)),Qt::ISODate));
          }
        }
      } else if (report.reportType() == eMyMoney::Report::ReportType::InfoTable) {
        if (rowTypeFromXML != eMyMoney::Report::RowType::Invalid)
          report.setRowType(rowTypeFromXML);
        else
          report.setRowType(eMyMoney::Report::RowType::AccountInfo);

        if (node.hasAttribute(attributeName(Attribute::Report::ShowRowTotals)))
          report.setShowingRowTotals(node.attribute(attributeName(Attribute::Report::ShowRowTotals)).toUInt());
        else
          report.setShowingRowTotals(true);
      }

      QDomNode child = node.firstChild();
      while (!child.isNull() && child.isElement()) {
        QDomElement c = child.toElement();
        if (elementName(Element::Report::Text) == c.tagName() && c.hasAttribute(attributeName(Attribute::Report::Pattern))) {
          report.setTextFilter(QRegExp(c.attribute(attributeName(Attribute::Report::Pattern)),
                                c.attribute(attributeName(Attribute::Report::CaseSensitive), "1").toUInt()
                                ? Qt::CaseSensitive : Qt::CaseInsensitive,
                                c.attribute(attributeName(Attribute::Report::RegEx), "1").toUInt()
                                ? QRegExp::Wildcard : QRegExp::RegExp),
                        c.attribute(attributeName(Attribute::Report::InvertText), "0").toUInt());
        }
        if (elementName(Element::Report::Type) == c.tagName() && c.hasAttribute(attributeName(Attribute::Report::Type))) {
          const auto reportType = stringToTypeAttribute(c.attribute(attributeName(Attribute::Report::Type)));
          if (reportType != -1)
            report.addType(reportType);
        }
        if (elementName(Element::Report::State) == c.tagName() && c.hasAttribute(attributeName(Attribute::Report::State))) {
          const auto state = stringToStateAttribute(c.attribute(attributeName(Attribute::Report::State)));
          if (state != -1)
            report.addState(state);
        }
        if (elementName(Element::Report::Validity) == c.tagName() && c.hasAttribute(attributeName(Attribute::Report::Validity))) {
          const auto validity = stringToValidityAttribute(c.attribute(attributeName(Attribute::Report::Validity)));
          if (validity != -1)
            report.addValidity(validity);
        }
        if (elementName(Element::Report::Number) == c.tagName())
          report.setNumberFilter(c.attribute(attributeName(Attribute::Report::From)), c.attribute(attributeName(Attribute::Report::To)));
        if (elementName(Element::Report::Amount) == c.tagName())
          report.setAmountFilter(MyMoneyMoney(c.attribute(attributeName(Attribute::Report::From), "0/100")), MyMoneyMoney(c.attribute(attributeName(Attribute::Report::To), "0/100")));
        if (elementName(Element::Report::Dates) == c.tagName()) {
          QDate from, to;
          if (c.hasAttribute(attributeName(Attribute::Report::From)))
            from = QDate::fromString(c.attribute(attributeName(Attribute::Report::From)), Qt::ISODate);
          if (c.hasAttribute(attributeName(Attribute::Report::To)))
            to = QDate::fromString(c.attribute(attributeName(Attribute::Report::To)), Qt::ISODate);
          report.setDateFilter(from, to);
        }
        if (elementName(Element::Report::Payee) == c.tagName())
          report.addPayee(c.attribute(attributeName(Attribute::Report::ID)));
        if (elementName(Element::Report::Tag) == c.tagName())
          report.addTag(c.attribute(attributeName(Attribute::Report::ID)));
        if (elementName(Element::Report::Category) == c.tagName() && c.hasAttribute(attributeName(Attribute::Report::ID)))
          report.addCategory(c.attribute(attributeName(Attribute::Report::ID)));
        if (elementName(Element::Report::Account) == c.tagName() && c.hasAttribute(attributeName(Attribute::Report::ID)))
          report.addAccount(c.attribute(attributeName(Attribute::Report::ID)));
#if 0
        // account groups had a severe problem in versions 5.0.0 to 5.0.2.  Therefor, we don't read them
        // in anymore and rebuild them internally. They are written to the file nevertheless to maintain
        // compatibility to older versions which rely on them. I left the old code for reference here
        // ipwizard - 2019-01-13
        if (elementName(Element::Report::AccountGroup) == c.tagName() && c.hasAttribute(attributeName(Attribute::Report::Group))) {
          const auto groupType = stringToAccountTypeAttribute(c.attribute(attributeName(Attribute::Report::Group)));
          if (groupType != eMyMoney::Account::Type::Unknown)
            report.addAccountGroup(groupType);
        }
#endif
        child = child.nextSibling();
      }

    return report;
  }

  void writeReport(const MyMoneyReport &report, QDomDocument &document, QDomElement &parent)
  {
    auto el = document.createElement(nodeName(Node::Report));

      // No matter what changes, be sure to have a 'type' attribute.  Only change
      // the major type if it becomes impossible to maintain compatibility with
      // older versions of the program as new features are added to the reports.
      // Feel free to change the minor type every time a change is made here.

      // write report's internals
      if (report.reportType() == eMyMoney::Report::ReportType::PivotTable)
        el.setAttribute(attributeName(Attribute::Report::Type), "pivottable 1.15");
      else if (report.reportType() == eMyMoney::Report::ReportType::QueryTable)
        el.setAttribute(attributeName(Attribute::Report::Type), "querytable 1.14");
      else if (report.reportType() == eMyMoney::Report::ReportType::InfoTable)
        el.setAttribute(attributeName(Attribute::Report::Type), "infotable 1.0");

      el.setAttribute(attributeName(Attribute::Report::Group), report.group());
      el.setAttribute(attributeName(Attribute::Report::ID), report.id());

      // write general tab
      auto anonymous = false;
      if (anonymous) {
        el.setAttribute(attributeName(Attribute::Report::Name), report.id());
        el.setAttribute(attributeName(Attribute::Report::Comment), QString(report.comment()).fill('x'));
      } else {
        el.setAttribute(attributeName(Attribute::Report::Name), report.name());
        el.setAttribute(attributeName(Attribute::Report::Comment), report.comment());
      }
      el.setAttribute(attributeName(Attribute::Report::ConvertCurrency), report.isConvertCurrency());
      el.setAttribute(attributeName(Attribute::Report::Favorite), report.isFavorite());
      el.setAttribute(attributeName(Attribute::Report::SkipZero), report.isSkippingZero());

      el.setAttribute(attributeName(Attribute::Report::DateLock), dateLockAttributeToString(report.dateRange()));
      el.setAttribute(attributeName(Attribute::Report::RowType), reportNames(report.rowType()));

      if (report.reportType() == eMyMoney::Report::ReportType::PivotTable) {
        // write report's internals
        el.setAttribute(attributeName(Attribute::Report::IncludesActuals), report.isIncludingBudgetActuals());
        el.setAttribute(attributeName(Attribute::Report::IncludesForecast), report.isIncludingForecast());
        el.setAttribute(attributeName(Attribute::Report::IncludesPrice), report.isIncludingPrice());
        el.setAttribute(attributeName(Attribute::Report::IncludesAveragePrice), report.isIncludingAveragePrice());
        el.setAttribute(attributeName(Attribute::Report::MixedTime), report.isMixedTime());
        el.setAttribute(attributeName(Attribute::Report::Investments), report.isInvestmentsOnly()); // it's setable in rows/columns tab of querytable, but here it is internal setting

        // write rows/columns tab
        if (!report.budget().isEmpty())
          el.setAttribute(attributeName(Attribute::Report::Budget), report.budget());

        el.setAttribute(attributeName(Attribute::Report::ShowRowTotals), report.isShowingRowTotals());
        el.setAttribute(attributeName(Attribute::Report::ShowColumnTotals), report.isShowingColumnTotals());
        el.setAttribute(attributeName(Attribute::Report::Detail), reportNames(report.detailLevel()));

        el.setAttribute(attributeName(Attribute::Report::IncludesMovingAverage), report.isIncludingMovingAverage());
        if (report.isIncludingMovingAverage())
          el.setAttribute(attributeName(Attribute::Report::MovingAverageDays), report.movingAverageDays());

        el.setAttribute(attributeName(Attribute::Report::IncludesSchedules), report.isIncludingSchedules());
        el.setAttribute(attributeName(Attribute::Report::IncludesTransfers), report.isIncludingTransfers());
        el.setAttribute(attributeName(Attribute::Report::IncludesUnused), report.isIncludingUnusedAccounts());
        el.setAttribute(attributeName(Attribute::Report::ColumnsAreDays), report.isColumnsAreDays());
        el.setAttribute(attributeName(Attribute::Report::ChartType), reportNames(report.chartType()));
        el.setAttribute(attributeName(Attribute::Report::ChartCHGridLines), report.isChartCHGridLines());
        el.setAttribute(attributeName(Attribute::Report::ChartSVGridLines), report.isChartSVGridLines());
        el.setAttribute(attributeName(Attribute::Report::ChartDataLabels), report.isChartDataLabels());
        el.setAttribute(attributeName(Attribute::Report::ChartByDefault), report.isChartByDefault());
        el.setAttribute(attributeName(Attribute::Report::LogYAxis), report.isLogYAxis());
        el.setAttribute(attributeName(Attribute::Report::ChartLineWidth), report.chartLineWidth());
        el.setAttribute(attributeName(Attribute::Report::ColumnType), reportNames(report.columnType()));
        el.setAttribute(attributeName(Attribute::Report::DataLock), reportNames(report.dataFilter()));
        el.setAttribute(attributeName(Attribute::Report::DataRangeStart), report.dataRangeStart());
        el.setAttribute(attributeName(Attribute::Report::DataRangeEnd), report.dataRangeEnd());
        el.setAttribute(attributeName(Attribute::Report::DataMajorTick), report.dataMajorTick());
        el.setAttribute(attributeName(Attribute::Report::DataMinorTick), report.dataMinorTick());
        el.setAttribute(attributeName(Attribute::Report::YLabelsPrecision), report.yLabelsPrecision());

      } else if (report.reportType() == eMyMoney::Report::ReportType::QueryTable) {
        // write rows/columns tab
        QStringList columns;
        unsigned qc = report.queryColumns();
        unsigned it_qc = eMyMoney::Report::QueryColumn::Begin;
        unsigned index = 1;
        while (it_qc != eMyMoney::Report::QueryColumn::End) {
          if (qc & it_qc)
            columns += reportNamesForQC(static_cast<eMyMoney::Report::QueryColumn>(it_qc));
          it_qc *= 2;
          index++;
        }
        el.setAttribute(attributeName(Attribute::Report::QueryColumns), columns.join(","));

        el.setAttribute(attributeName(Attribute::Report::Tax), report.isTax());
        el.setAttribute(attributeName(Attribute::Report::Investments), report.isInvestmentsOnly());
        el.setAttribute(attributeName(Attribute::Report::Loans), report.isLoansOnly());
        el.setAttribute(attributeName(Attribute::Report::HideTransactions), report.isHideTransactions());
        el.setAttribute(attributeName(Attribute::Report::ShowColumnTotals), report.isShowingColumnTotals());
        el.setAttribute(attributeName(Attribute::Report::Detail), reportNames(report.detailLevel()));

        // write performance tab
        if (report.queryColumns() & eMyMoney::Report::QueryColumn::Performance || report.queryColumns() & eMyMoney::Report::QueryColumn::CapitalGain)
          el.setAttribute(attributeName(Attribute::Report::InvestmentSum), static_cast<int>(report.investmentSum()));

        // write capital gains tab
        if (report.queryColumns() & eMyMoney::Report::QueryColumn::CapitalGain) {
          if (report.investmentSum() == eMyMoney::Report::InvestmentSum::Sold) {
            el.setAttribute(attributeName(Attribute::Report::SettlementPeriod), report.settlementPeriod());
            el.setAttribute(attributeName(Attribute::Report::ShowSTLTCapitalGains), report.isShowingSTLTCapitalGains());
            el.setAttribute(attributeName(Attribute::Report::TermsSeparator), report.termSeparator().toString(Qt::ISODate));
          }
        }
      } else if (report.reportType() == eMyMoney::Report::ReportType::InfoTable)
        el.setAttribute(attributeName(Attribute::Report::ShowRowTotals), report.isShowingRowTotals());

      //
      // Text Filter
      //

      QRegExp textfilter;
      if (report.textFilter(textfilter)) {
        QDomElement f = document.createElement(elementName(Element::Report::Text));
        f.setAttribute(attributeName(Attribute::Report::Pattern), textfilter.pattern());
        f.setAttribute(attributeName(Attribute::Report::CaseSensitive), (textfilter.caseSensitivity() == Qt::CaseSensitive) ? 1 : 0);
        f.setAttribute(attributeName(Attribute::Report::RegEx), (textfilter.patternSyntax() == QRegExp::Wildcard) ? 1 : 0);
        f.setAttribute(attributeName(Attribute::Report::InvertText), report.MyMoneyTransactionFilter::isInvertingText());
        el.appendChild(f);
      }

      //
      // Type & State Filters
      //
      QList<int> typelist;
      if (report.types(typelist) && ! typelist.empty()) {
        // iterate over payees, and add each one
        QList<int>::const_iterator it_type = typelist.constBegin();
        while (it_type != typelist.constEnd()) {
          QDomElement p = document.createElement(elementName(Element::Report::Type));
          p.setAttribute(attributeName(Attribute::Report::Type), typeAttributeToString(*it_type));
          el.appendChild(p);

          ++it_type;
        }
      }

      QList<int> statelist;
      if (report.states(statelist) && ! statelist.empty()) {
        // iterate over payees, and add each one
        QList<int>::const_iterator it_state = statelist.constBegin();
        while (it_state != statelist.constEnd()) {
          QDomElement p = document.createElement(elementName(Element::Report::State));
          p.setAttribute(attributeName(Attribute::Report::State), stateAttributeToString(*it_state));
          el.appendChild(p);

          ++it_state;
        }
      }

      QList<int> validitylist;
      if (report.validities(validitylist) && ! validitylist.empty()) {
        // iterate over payees, and add each one
        QList<int>::const_iterator it_validity = validitylist.constBegin();
        while (it_validity != validitylist.constEnd()) {
          QDomElement p = document.createElement(elementName(Element::Report::Validity));
          p.setAttribute(attributeName(Attribute::Report::Validity), validityAttributeToString(*it_validity));
          el.appendChild(p);

          ++it_validity;
        }
      }

      //
      // Number Filter
      //

      QString nrFrom, nrTo;
      if (report.numberFilter(nrFrom, nrTo)) {
        QDomElement f = document.createElement(elementName(Element::Report::Number));
        f.setAttribute(attributeName(Attribute::Report::From), nrFrom);
        f.setAttribute(attributeName(Attribute::Report::To), nrTo);
        el.appendChild(f);
      }

      //
      // Amount Filter
      //

      MyMoneyMoney from, to;
      if (report.amountFilter(from, to)) {    // bool getAmountFilter(MyMoneyMoney&,MyMoneyMoney&);
        QDomElement f = document.createElement(elementName(Element::Report::Amount));
        f.setAttribute(attributeName(Attribute::Report::From), from.toString());
        f.setAttribute(attributeName(Attribute::Report::To), to.toString());
        el.appendChild(f);
      }

      //
      // Payees Filter
      //

      QStringList payeelist;
      if (report.payees(payeelist)) {
        if (payeelist.empty()) {
          QDomElement p = document.createElement(elementName(Element::Report::Payee));
          el.appendChild(p);
        } else {
          // iterate over payees, and add each one
          QStringList::const_iterator it_payee = payeelist.constBegin();
          while (it_payee != payeelist.constEnd()) {
            QDomElement p = document.createElement(elementName(Element::Report::Payee));
            p.setAttribute(attributeName(Attribute::Report::ID), *it_payee);
            el.appendChild(p);

            ++it_payee;
          }
        }
      }

      //
      // Tags Filter
      //

      QStringList taglist;
      if (report.tags(taglist)) {
        if (taglist.empty()) {
          QDomElement p = document.createElement(elementName(Element::Report::Tag));
          el.appendChild(p);
        } else {
          // iterate over tags, and add each one
          QStringList::const_iterator it_tag = taglist.constBegin();
          while (it_tag != taglist.constEnd()) {
            QDomElement p = document.createElement(elementName(Element::Report::Tag));
            p.setAttribute(attributeName(Attribute::Report::ID), *it_tag);
            el.appendChild(p);

            ++it_tag;
          }
        }
      }

      //
      // Account Groups Filter
      //

      QList<eMyMoney::Account::Type> accountgrouplist;
      if (report.accountGroups(accountgrouplist)) {
        // iterate over accounts, and add each one
        QList<eMyMoney::Account::Type>::const_iterator it_group = accountgrouplist.constBegin();
        while (it_group != accountgrouplist.constEnd()) {
          QDomElement p = document.createElement(elementName(Element::Report::AccountGroup));
          p.setAttribute(attributeName(Attribute::Report::Group), accountTypeAttributeToString(*it_group));
          el.appendChild(p);

          ++it_group;
        }
      }

      //
      // Accounts Filter
      //

      QStringList accountlist;
      if (report.accounts(accountlist)) {
        // iterate over accounts, and add each one
        QStringList::const_iterator it_account = accountlist.constBegin();
        while (it_account != accountlist.constEnd()) {
          QDomElement p = document.createElement(elementName(Element::Report::Account));
          p.setAttribute(attributeName(Attribute::Report::ID), *it_account);
          el.appendChild(p);

          ++it_account;
        }
      }

      //
      // Categories Filter
      //

      accountlist.clear();
      if (report.categories(accountlist)) {
        // iterate over accounts, and add each one
        QStringList::const_iterator it_account = accountlist.constBegin();
        while (it_account != accountlist.constEnd()) {
          QDomElement p = document.createElement(elementName(Element::Report::Category));
          p.setAttribute(attributeName(Attribute::Report::ID), *it_account);
          el.appendChild(p);

          ++it_account;
        }
      }

      //
      // Date Filter
      //

      if (report.dateRange() == eMyMoney::TransactionFilter::Date::UserDefined) {
        QDate dateFrom, dateTo;
        if (report.dateFilter(dateFrom, dateTo)) {
          QDomElement f = document.createElement(elementName(Element::Report::Dates));
          if (dateFrom.isValid())
            f.setAttribute(attributeName(Attribute::Report::From), dateFrom.toString(Qt::ISODate));
          if (dateTo.isValid())
            f.setAttribute(attributeName(Attribute::Report::To), dateTo.toString(Qt::ISODate));
          el.appendChild(f);
        }
      }


    parent.appendChild(el);
  }

  MyMoneyBudget readBudget(const QDomElement &node)
  {
    if (nodeName(Node::Budget) != node.tagName())
      throw MYMONEYEXCEPTION_CSTRING("Node was not BUDGET");

    MyMoneyBudget budget(node.attribute(QStringLiteral("id")));
    // The goal of this reading method is 100% backward AND 100% forward
    // compatibility.  Any Budget ever created with any version of KMyMoney
    // should be able to be loaded by this method (as long as it's one of the
    // Budget types supported in this version, of course)

    budget.setName(node.attribute(attributeName(Attribute::Budget::Name)));
    budget.setBudgetStart(QDate::fromString(node.attribute(attributeName(Attribute::Budget::Start)), Qt::ISODate));

    QDomNode child = node.firstChild();
    while (!child.isNull() && child.isElement()) {
      QDomElement c = child.toElement();

      MyMoneyBudget::AccountGroup account;

      if (elementName(Element::Budget::Account) == c.tagName()) {
        if (c.hasAttribute(attributeName(Attribute::Budget::ID)))
          account.setId(c.attribute(attributeName(Attribute::Budget::ID)));

        if (c.hasAttribute(attributeName(Attribute::Budget::BudgetLevel)))
          account.setBudgetLevel(stringToBudgetLevel(c.attribute(attributeName(Attribute::Budget::BudgetLevel))));

        if (c.hasAttribute(attributeName(Attribute::Budget::BudgetSubAccounts)))
          account.setBudgetSubaccounts(c.attribute(attributeName(Attribute::Budget::BudgetSubAccounts)).toUInt());
      }

      QDomNode period = c.firstChild();
      while (!period.isNull() && period.isElement()) {
        QDomElement per = period.toElement();
        MyMoneyBudget::PeriodGroup pGroup;

        if (elementName(Element::Budget::Period) == per.tagName() && per.hasAttribute(attributeName(Attribute::Budget::Amount)) && per.hasAttribute(attributeName(Attribute::Budget::Start))) {
          pGroup.setAmount(MyMoneyMoney(per.attribute(attributeName(Attribute::Budget::Amount))));
          pGroup.setStartDate(QDate::fromString(per.attribute(attributeName(Attribute::Budget::Start)), Qt::ISODate));
          account.addPeriod(pGroup.startDate(), pGroup);
        }

        period = period.nextSibling();
      }
      budget.setAccount(account, account.id());

      child = child.nextSibling();
    }


    return budget;
  }

  const int BUDGET_VERSION = 2;

  void writeBudget(const MyMoneyBudget &budget, QDomDocument &document, QDomElement &parent)
  {
    auto el = document.createElement(nodeName(Node::Budget));

    writeBaseXML(budget.id(), document, el);

    el.setAttribute(attributeName(Attribute::Budget::Name),  budget.name());
    el.setAttribute(attributeName(Attribute::Budget::Start), budget.budgetStart().toString(Qt::ISODate));
    el.setAttribute(attributeName(Attribute::Budget::Version), BUDGET_VERSION);

    QMap<QString, MyMoneyBudget::AccountGroup>::const_iterator it;
    auto accounts = budget.accountsMap();
    for (it = accounts.cbegin(); it != accounts.cend(); ++it) {
      // only add the account if there is a budget entered
      // or it covers some sub accounts
      if (!(*it).balance().isZero() || (*it).budgetSubaccounts()) {
        QDomElement domAccount = document.createElement(elementName(Element::Budget::Account));
        domAccount.setAttribute(attributeName(Attribute::Budget::ID), it.key());
        domAccount.setAttribute(attributeName(Attribute::Budget::BudgetLevel), budgetLevels(it.value().budgetLevel()));
        domAccount.setAttribute(attributeName(Attribute::Budget::BudgetSubAccounts), it.value().budgetSubaccounts());

        const QMap<QDate, MyMoneyBudget::PeriodGroup> periods = it.value().getPeriods();
        QMap<QDate, MyMoneyBudget::PeriodGroup>::const_iterator it_per;
        for (it_per = periods.begin(); it_per != periods.end(); ++it_per) {
          if (!(*it_per).amount().isZero()) {
            QDomElement domPeriod = document.createElement(elementName(Element::Budget::Period));

            domPeriod.setAttribute(attributeName(Attribute::Budget::Amount), (*it_per).amount().toString());
            domPeriod.setAttribute(attributeName(Attribute::Budget::Start), (*it_per).startDate().toString(Qt::ISODate));
            domAccount.appendChild(domPeriod);
          }
        }

        el.appendChild(domAccount);
      }
    }

    parent.appendChild(el);
  }
}

