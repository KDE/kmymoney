/*
    SPDX-FileCopyrightText: 2004-2006 Ace Jones <acejones@users.sourceforge.net>
    SPDX-FileCopyrightText: 2006 Darren Gould <darren_gould@gmx.de>
    SPDX-FileCopyrightText: 2007-2010 Alvaro Soliverez <asoliverez@gmail.com>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2018 Michael Kiefer <Michael-Kiefer@web.de>
    SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "xmlstoragehelper.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDate>
#include <QMap>
#include <QRegularExpression>
#include <QXmlStreamReader>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneybudget.h"
#include "mymoneyenums.h"
#include "mymoneyexception.h"
#include "mymoneymoney.h"
#include "mymoneyreport.h"
#include "mymoneytransactionfilter.h"
#include "mymoneyutils.h"


using namespace MyMoneyXmlHelper;

namespace MyMoneyXmlHelper {

enum class Node {
    Report,
    Budget,
};

QString nodeName(Node nodeID)
{
    // clang-format off
    static const QHash<Node, QString> nodeNames {
        {Node::Report,        QStringLiteral("REPORT")},
        {Node::Budget,        QStringLiteral("BUDGET")},
    };
    // clang-format on
    return nodeNames.value(nodeID);
}

uint qHash(const Node key, uint seed)
{
    return ::qHash(static_cast<uint>(key), seed);
}

QString attributeName(Attribute::Report attributeID)
{
    // clang-format off
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
        {Attribute::Report::ChartPalette,           QStringLiteral("chartpalette")},
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
        {Attribute::Report::Validity,               QStringLiteral("validity")},
        {Attribute::Report::NegExpenses,            QStringLiteral("negexpenses")},
    };
    // clang-format on
    return attributeNames.value(attributeID);
}

uint qHash(const Attribute::Report key, uint seed)
{
    Q_UNUSED(seed);
    return ::qHash(static_cast<uint>(key), 0);
}

QString elementName(Element::Report elementID)
{
    // clang-format off
    static const QMap<Element::Report, QString> elementNames{
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
        {Element::Report::Validity,     QStringLiteral("VALIDITY")},
    };
    // clang-format on
    return elementNames.value(elementID);
}

QString elementName(Element::Budget elementID)
{
    // clang-format off
    static const QMap<Element::Budget, QString> elementNames {
        {Element::Budget::Budget,   QStringLiteral("BUDGET")},
        {Element::Budget::Account,  QStringLiteral("ACCOUNT")},
        {Element::Budget::Period,   QStringLiteral("PERIOD")}
    };
    // clang-format on
    return elementNames.value(elementID);
}

QString attributeName(Attribute::Budget attributeID)
{
    // clang-format off
    static const QMap<Attribute::Budget, QString> attributeNames {
        {Attribute::Budget::ID,                 QStringLiteral("id")},
        {Attribute::Budget::Name,               QStringLiteral("name")},
        {Attribute::Budget::Start,              QStringLiteral("start")},
        {Attribute::Budget::Version,            QStringLiteral("version")},
        {Attribute::Budget::BudgetLevel,        QStringLiteral("budgetlevel")},
        {Attribute::Budget::BudgetSubAccounts,  QStringLiteral("budgetsubaccounts")},
        {Attribute::Budget::Amount,             QStringLiteral("amount")},
    };
    // clang-format on
    return attributeNames.value(attributeID);
}

uint qHash(const Attribute::Budget key, uint seed)
{
    Q_UNUSED(seed);
    return ::qHash(static_cast<uint>(key), 0);
}

QHash<eMyMoney::Report::RowType, QString> rowTypesLUT()
{
    // clang-format off
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
    // clang-format on
    return lut;
}

QString reportNames(eMyMoney::Report::RowType textID)
{
    return rowTypesLUT().value(textID);
}

eMyMoney::Report::RowType stringToRowType(const QString& text)
{
    return rowTypesLUT().key(text, eMyMoney::Report::RowType::Invalid);
}

QHash<eMyMoney::Report::ColumnType, QString> columTypesLUT()
{
    // clang-format off
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
        {eMyMoney::Report::ColumnType::Years,     QStringLiteral("years")},
    };
    // clang-format on
    return lut;
}

QString reportNames(eMyMoney::Report::ColumnType textID)
{
    return columTypesLUT().value(textID);
}

eMyMoney::Report::ColumnType stringToColumnType(const QString& text)
{
    return columTypesLUT().key(text, eMyMoney::Report::ColumnType::Invalid);
}

QHash<eMyMoney::Report::QueryColumn, QString> queryColumnsLUT()
{
    // clang-format off
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
        {eMyMoney::Report::QueryColumn::CapitalGain, QStringLiteral("capitalgain")},
    };
    // clang-format on
    return lut;
}

QString reportNamesForQC(eMyMoney::Report::QueryColumn textID)
{
    return queryColumnsLUT().value(textID);
}

eMyMoney::Report::QueryColumn stringToQueryColumn(const QString& text)
{
    return queryColumnsLUT().key(text, eMyMoney::Report::QueryColumn::End);
}

QHash<eMyMoney::Report::DetailLevel, QString> detailLevelLUT()
{
    // clang-format off
    static const QHash<eMyMoney::Report::DetailLevel, QString> lut {
        {eMyMoney::Report::DetailLevel::None,   QStringLiteral("none")},
        {eMyMoney::Report::DetailLevel::All,    QStringLiteral("all")},
        {eMyMoney::Report::DetailLevel::Top,    QStringLiteral("top")},
        {eMyMoney::Report::DetailLevel::Group,  QStringLiteral("group")},
        {eMyMoney::Report::DetailLevel::Total,  QStringLiteral("total")},
        {eMyMoney::Report::DetailLevel::End,    QStringLiteral("invalid")},
    };
    // clang-format on
    return lut;
}

QString reportNames(eMyMoney::Report::DetailLevel textID)
{
    return detailLevelLUT().value(textID);
}

eMyMoney::Report::DetailLevel stringToDetailLevel(const QString& text)
{
    return detailLevelLUT().key(text, eMyMoney::Report::DetailLevel::End);
}

QHash<eMyMoney::Report::ChartType, QString> chartTypeLUT()
{
    // clang-format off
    static const QHash<eMyMoney::Report::ChartType, QString> lut {
        {eMyMoney::Report::ChartType::None,       QStringLiteral("none")},
        {eMyMoney::Report::ChartType::Line,       QStringLiteral("line")},
        {eMyMoney::Report::ChartType::Bar,        QStringLiteral("bar")},
        {eMyMoney::Report::ChartType::Pie,        QStringLiteral("pie")},
        {eMyMoney::Report::ChartType::Ring,       QStringLiteral("ring")},
        {eMyMoney::Report::ChartType::StackedBar, QStringLiteral("stackedbar")},
    };
    // clang-format on
    return lut;
}

QString reportNames(eMyMoney::Report::ChartType textID)
{
    return chartTypeLUT().value(textID);
}

eMyMoney::Report::ChartType stringToChartType(const QString& text)
{
    return chartTypeLUT().key(text, eMyMoney::Report::ChartType::End);
}

QHash<int, QString> typeAttributeLUT()
{
    static const QHash<int, QString> lut{
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

int stringToTypeAttribute(const QString& text)
{
    return typeAttributeLUT().key(text, 4);
}

QHash<eMyMoney::Report::ChartPalette, QString> chartPaletteLUT()
{
    // clang-format off
    static const QHash<eMyMoney::Report::ChartPalette, QString> lut {
        {eMyMoney::Report::ChartPalette::Application, QStringLiteral("application")},
        {eMyMoney::Report::ChartPalette::Default,     QStringLiteral("default")},
        {eMyMoney::Report::ChartPalette::Rainbow,     QStringLiteral("rainbow")},
        {eMyMoney::Report::ChartPalette::Subdued,     QStringLiteral("subdued")},
    };
    // clang-format on
    return lut;
}

QString reportNames(eMyMoney::Report::ChartPalette textID)
{
    return chartPaletteLUT().value(textID);
}

eMyMoney::Report::ChartPalette stringToChartPalette(const QString& text)
{
    return chartPaletteLUT().key(text, eMyMoney::Report::ChartPalette::End);
}

QHash<int, QString> paletteAttributeLUT()
{
    static const QHash<int, QString> lut{
        {0, QStringLiteral("application")},
        {1, QStringLiteral("default")},
        {2, QStringLiteral("rainbow")},
        {3, QStringLiteral("subdued")},
    };
    return lut;
}

QString paletteAttributeToString(int textID)
{
    return paletteAttributeLUT().value(textID);
}

int stringToPaletteAttribute(const QString& text)
{
    return paletteAttributeLUT().key(text, 4);
}

QHash<int, QString> stateAttributeLUT()
{
    static const QHash<int, QString> lut{
        {0, QStringLiteral("all")},
        {1, QStringLiteral("notreconciled")},
        {2, QStringLiteral("cleared")},
        {3, QStringLiteral("reconciled")},
        {4, QStringLiteral("frozen")},
        {5, QStringLiteral("none")},
    };
    return lut;
}

QString stateAttributeToString(int textID)
{
    return stateAttributeLUT().value(textID);
}

int stringToStateAttribute(const QString& text)
{
    return stateAttributeLUT().key(text, 5);
}

QHash<int, QString> validityAttributeLUT()
{
    static const QHash<int, QString> lut{
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

int stringToValidityAttribute(const QString& text)
{
    return validityAttributeLUT().key(text, 0);
}

QHash<eMyMoney::TransactionFilter::Date, QString> dateLockLUT()
{
    // clang-format off
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
        {eMyMoney::TransactionFilter::Date::Next18Months,       QStringLiteral("next18months")},
    };
    // clang-format on
    return lut;
}

QString dateLockAttributeToString(eMyMoney::TransactionFilter::Date textID)
{
    return dateLockLUT().value(textID);
}

eMyMoney::TransactionFilter::Date stringToDateLockAttribute(const QString& text)
{
    return dateLockLUT().key(text, eMyMoney::TransactionFilter::Date::UserDefined);
}

QHash<eMyMoney::Report::DataLock, QString> dataLockLUT()
{
    // clang-format off
    static const QHash<eMyMoney::Report::DataLock, QString> lut {
        {eMyMoney::Report::DataLock::Automatic,   QStringLiteral("automatic")},
        {eMyMoney::Report::DataLock::UserDefined, QStringLiteral("userdefined")},
    };
    // clang-format on
    return lut;
}

QString reportNames(eMyMoney::Report::DataLock textID)
{
    return dataLockLUT().value(textID);
}

eMyMoney::Report::DataLock stringToDataLockAttribute(const QString& text)
{
    return dataLockLUT().key(text, eMyMoney::Report::DataLock::DataOptionCount);
}

QHash<eMyMoney::Account::Type, QString> accountTypeAttributeLUT()
{
    // clang-format off
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
    // clang-format on
    return lut;
}

QString accountTypeAttributeToString(eMyMoney::Account::Type type)
{
    return accountTypeAttributeLUT().value(type);
}

eMyMoney::Account::Type stringToAccountTypeAttribute(const QString& text)
{
    return accountTypeAttributeLUT().key(text, eMyMoney::Account::Type::Unknown);
}

eMyMoney::Report::ReportType rowTypeToReportType(eMyMoney::Report::RowType rowType)
{
    // clang-format off
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
    // clang-format on
    return reportTypes.value(rowType, eMyMoney::Report::ReportType::Invalid);
}

QHash<eMyMoney::Budget::Level, QString> budgetLevelLUT()
{
    // clang-format off
    static const QHash<eMyMoney::Budget::Level, QString> lut {
        {eMyMoney::Budget::Level::None,         QStringLiteral("none")},
        {eMyMoney::Budget::Level::Monthly,      QStringLiteral("monthly")},
        {eMyMoney::Budget::Level::MonthByMonth, QStringLiteral("monthbymonth")},
        {eMyMoney::Budget::Level::Yearly,       QStringLiteral("yearly")},
        {eMyMoney::Budget::Level::Max,          QStringLiteral("invalid")},
    };
    // clang-format on
    return lut;
}

QString budgetNames(eMyMoney::Budget::Level textID)
{
    return budgetLevelLUT().value(textID);
}

eMyMoney::Budget::Level stringToBudgetLevel(const QString& text)
{
    return budgetLevelLUT().key(text, eMyMoney::Budget::Level::Max);
}

QHash<eMyMoney::Budget::Level, QString> budgetLevelsLUT()
{
    // clang-format off
    static const QHash<eMyMoney::Budget::Level, QString> lut {
        {eMyMoney::Budget::Level::None,         QStringLiteral("none")},
        {eMyMoney::Budget::Level::Monthly,      QStringLiteral("monthly")},
        {eMyMoney::Budget::Level::MonthByMonth, QStringLiteral("monthbymonth")},
        {eMyMoney::Budget::Level::Yearly,       QStringLiteral("yearly")},
        {eMyMoney::Budget::Level::Max,          QStringLiteral("invalid")},
    };
    // clang-format on
    return lut;
}

QString budgetLevels(eMyMoney::Budget::Level textID)
{
    return budgetLevelsLUT().value(textID);
}

MyMoneyMoney readValueAttribute(QXmlStreamReader* reader, const QString& attribute)
{
    return MyMoneyMoney(readStringAttribute(reader, attribute));
}

inline bool hasAttribute(QXmlStreamReader* reader, const QString& attribute)
{
    return reader->attributes().hasAttribute(attribute);
}

QString readRequiredStringAttribute(QXmlStreamReader* reader, const QString& attribute)
{
    const auto attributes(reader->attributes());
    const auto haveAttribute(attributes.hasAttribute(attribute));
    const auto value(attributes.value(attribute).toString());

    if (!haveAttribute) {
        reader->raiseError(i18nc("Missing attribute %1 in xml file", "Missing attribute %1 in line %2").arg(attribute).arg(reader->lineNumber()));

    } else if (value.isEmpty()) {
        reader->raiseError(i18nc("Empty attribute %1 in xml file", "Empty attribute %1 in line %2").arg(attribute).arg(reader->lineNumber()));
    }
    return value;
}

QString readStringAttribute(QXmlStreamReader* reader, const QString& attribute, const QString& defaultValue)
{
    const auto attributes(reader->attributes());
    if (attributes.hasAttribute(attribute)) {
        return attributes.value(attribute).toString();
    }
    return defaultValue;
}

uint readUintAttribute(QXmlStreamReader* reader, const QString& attribute, uint defaultValue, int base)
{
    const auto strValue = reader->attributes().value(attribute);
    if (strValue.isEmpty()) {
        return defaultValue;
    }
    return strValue.toUInt(nullptr, base);
}

QString readId(QXmlStreamReader* reader, IdRequirement idRequirement)
{
    if (idRequirement == IdRequirement::Optional) {
        return readStringAttribute(reader, attributeName(Attribute::Report::ID));
    }

    return readRequiredStringAttribute(reader, attributeName(Attribute::Report::ID));
}

QDate readDateAttribute(QXmlStreamReader* reader, const QString& attribute)
{
    const auto dateString(readStringAttribute(reader, attribute));
    if (!dateString.isEmpty()) {
        const auto date = QDate::fromString(dateString, Qt::ISODate);
        if (!date.isNull() && date.isValid())
            return date;
    }
    return {};
}

QDateTime readDateTimeAttribute(QXmlStreamReader* reader, const QString& attribute)
{
    const auto dateString(readStringAttribute(reader, attribute));
    if (!dateString.isEmpty()) {
        const auto date = QDateTime::fromString(dateString, Qt::ISODate);
        if (!date.isNull() && date.isValid())
            return date;
    }
    return {};
}

bool readBoolAttribute(QXmlStreamReader* reader, const QString& attribute, bool defaultValue)
{
    const auto strValue = reader->attributes().value(attribute);
    if (strValue.isEmpty()) {
        return defaultValue;
    }
    struct BoolValues {
        BoolValues(const char* trueValue, const char* falseValue)
            : m_trueValue(trueValue)
            , m_falseValue(falseValue)
        {
        }
        const char* m_trueValue;
        const char* m_falseValue;
    };

    QVector<BoolValues> values = {
        {"1", "0"},
        {"on", "off"},
        {"true", "false"},
        {"yes", "no"},
    };

    const auto entries = values.count();
    for (int i = 0; i < entries; ++i) {
        if (strValue.compare(strValue, values.at(i).m_trueValue, Qt::CaseInsensitive) == 0) {
            return true;
        }
        if (strValue.compare(strValue, values.at(i).m_falseValue, Qt::CaseInsensitive) == 0) {
            return false;
        }
    }
    return defaultValue;
}

inline static QString attrValue(bool attribute)
{
    return attribute ? QStringLiteral("1") : QStringLiteral("0");
}

inline static QString attrValue(int attribute)
{
    return QString::number(attribute);
}

inline static QString attrValue(uint attribute)
{
    return QString::number(attribute);
}

MyMoneyReport readReport(QXmlStreamReader* reader)
{
    Q_ASSERT(reader->isStartElement() && (reader->name() == nodeName(Node::Report)));

    MyMoneyReport report(readId(reader));

    // The goal of this reading method is 100% backward AND 100% forward
    // compatibility.  Any report ever created with any version of KMyMoney
    // should be able to be loaded by this method (as long as it's one of the
    // report types supported in this version, of course)

    // read report's internals
    const auto type = readStringAttribute(reader, attributeName(Attribute::Report::Type));
    if (type.startsWith(QLatin1String("pivottable"))) {
        report.setReportType(eMyMoney::Report::ReportType::PivotTable);
    } else if (type.startsWith(QLatin1String("querytable"))) {
        report.setReportType(eMyMoney::Report::ReportType::QueryTable);
    } else if (type.startsWith(QLatin1String("infotable"))) {
        report.setReportType(eMyMoney::Report::ReportType::InfoTable);
    } else {
        throw MYMONEYEXCEPTION_CSTRING("Unknown report type");
    }

    report.setGroup(readStringAttribute(reader, attributeName(Attribute::Report::Group)));

    report.clearTransactionFilter();

    // read date tab
    QString datelockstr = readStringAttribute(reader, attributeName(Attribute::Report::DateLock), "userdefined");
    // Handle the pivot 1.2/query 1.1 case where the values were saved as
    // numbers
    bool ok = false;
    eMyMoney::TransactionFilter::Date dateLock = static_cast<eMyMoney::TransactionFilter::Date>(datelockstr.toUInt(&ok));
    if (!ok) {
        dateLock = stringToDateLockAttribute(datelockstr);
    }
    report.setDateFilter(dateLock);

    // read general tab
    report.setName(readStringAttribute(reader, attributeName(Attribute::Report::Name)));
    report.setComment(readStringAttribute(reader, attributeName(Attribute::Report::Comment), "Extremely old report"));
    report.setConvertCurrency(readBoolAttribute(reader, attributeName(Attribute::Report::ConvertCurrency), true));
    report.setFavorite(readBoolAttribute(reader, attributeName(Attribute::Report::Favorite), false));
    report.setSkipZero(readBoolAttribute(reader, attributeName(Attribute::Report::SkipZero), false));
    const auto rowTypeFromXML = stringToRowType(readStringAttribute(reader, attributeName(Attribute::Report::RowType)));

    if (report.reportType() == eMyMoney::Report::ReportType::PivotTable) {
        // read report's internals
        report.setIncludingBudgetActuals(readBoolAttribute(reader, attributeName(Attribute::Report::IncludesActuals), false));
        report.setIncludingForecast(readBoolAttribute(reader, attributeName(Attribute::Report::IncludesForecast), false));
        report.setIncludingPrice(readBoolAttribute(reader, attributeName(Attribute::Report::IncludesPrice), false));
        report.setIncludingAveragePrice(readBoolAttribute(reader, attributeName(Attribute::Report::IncludesAveragePrice), false));
        report.setMixedTime(readBoolAttribute(reader, attributeName(Attribute::Report::MixedTime), false));
        report.setInvestmentsOnly(readBoolAttribute(reader, attributeName(Attribute::Report::Investments), false));

        // read rows/columns tab
        if (hasAttribute(reader, attributeName(Attribute::Report::Budget))) {
            report.setBudget(readStringAttribute(reader, attributeName(Attribute::Report::Budget)), report.isIncludingBudgetActuals());
        }

        if (rowTypeFromXML != eMyMoney::Report::RowType::Invalid) {
            report.setRowType(rowTypeFromXML);
        } else {
            report.setRowType(eMyMoney::Report::RowType::ExpenseIncome);
        }

        if (hasAttribute(reader, attributeName(Attribute::Report::ShowRowTotals))) {
            report.setShowingRowTotals(readBoolAttribute(reader, attributeName(Attribute::Report::ShowRowTotals)));
        } else if (report.rowType() == eMyMoney::Report::RowType::ExpenseIncome) { // for backward compatibility
            report.setShowingRowTotals(true);
        }
        report.setShowingColumnTotals(readBoolAttribute(reader, attributeName(Attribute::Report::ShowColumnTotals), true));

        // check for reports with older settings which didn't have the detail attribute
        const auto detailLevelFromXML = stringToDetailLevel(readStringAttribute(reader, attributeName(Attribute::Report::Detail)));
        if (detailLevelFromXML != eMyMoney::Report::DetailLevel::End) {
            report.setDetailLevel(detailLevelFromXML);
        } else {
            report.setDetailLevel(eMyMoney::Report::DetailLevel::All);
        }

        report.setIncludingMovingAverage(readBoolAttribute(reader, attributeName(Attribute::Report::IncludesMovingAverage), false));
        if (report.isIncludingMovingAverage()) {
            report.setMovingAverageDays(readBoolAttribute(reader, attributeName(Attribute::Report::MovingAverageDays), true));
        }
        report.setIncludingSchedules(readBoolAttribute(reader, attributeName(Attribute::Report::IncludesSchedules), false));
        report.setIncludingTransfers(readBoolAttribute(reader, attributeName(Attribute::Report::IncludesTransfers), false));
        report.setIncludingUnusedAccounts(readBoolAttribute(reader, attributeName(Attribute::Report::IncludesUnused), false));
        report.setColumnsAreDays(readBoolAttribute(reader, attributeName(Attribute::Report::ColumnsAreDays), false));

        // read chart tab
        const auto chartTypeFromXML = stringToChartType(readStringAttribute(reader, attributeName(Attribute::Report::ChartType)));
        if (chartTypeFromXML != eMyMoney::Report::ChartType::End) {
            report.setChartType(chartTypeFromXML);
        } else {
            report.setChartType(eMyMoney::Report::ChartType::None);
        }

        const auto chartPaletteFromXML = stringToChartPalette(readStringAttribute(reader, attributeName(Attribute::Report::ChartPalette)));
        if (chartPaletteFromXML != eMyMoney::Report::ChartPalette::End) {
            report.setChartPalette(chartPaletteFromXML);
        } else {
            report.setChartPalette(eMyMoney::Report::ChartPalette::Application);
        }

        report.setChartCHGridLines(readBoolAttribute(reader, attributeName(Attribute::Report::ChartCHGridLines), true));
        report.setChartSVGridLines(readBoolAttribute(reader, attributeName(Attribute::Report::ChartSVGridLines), true));
        report.setChartDataLabels(readBoolAttribute(reader, attributeName(Attribute::Report::ChartDataLabels), true));
        report.setChartByDefault(readBoolAttribute(reader, attributeName(Attribute::Report::ChartByDefault), false));
        report.setLogYAxis(readBoolAttribute(reader, attributeName(Attribute::Report::LogYAxis), false));
        report.setNegExpenses(readBoolAttribute(reader, attributeName(Attribute::Report::NegExpenses), false));
        report.setChartLineWidth(readUintAttribute(reader, attributeName(Attribute::Report::ChartLineWidth), MyMoneyReport::lineWidth()));

        // read range tab
        const auto columnTypeFromXML = stringToColumnType(readStringAttribute(reader, attributeName(Attribute::Report::ColumnType)));
        if (columnTypeFromXML != eMyMoney::Report::ColumnType::Invalid) {
            report.setColumnType(columnTypeFromXML);
        } else {
            report.setColumnType(eMyMoney::Report::ColumnType::Months);
        }

        const auto dataLockFromXML = stringToDataLockAttribute(readStringAttribute(reader, attributeName(Attribute::Report::DataLock)));
        if (dataLockFromXML != eMyMoney::Report::DataLock::DataOptionCount) {
            report.setDataFilter(dataLockFromXML);
        } else {
            report.setDataFilter(eMyMoney::Report::DataLock::Automatic);
        }

        report.setDataRangeStart(readStringAttribute(reader, attributeName(Attribute::Report::DataRangeStart), 0));
        report.setDataRangeEnd(readStringAttribute(reader, attributeName(Attribute::Report::DataRangeEnd), "0"));
        report.setDataMajorTick(readStringAttribute(reader, attributeName(Attribute::Report::DataMajorTick), "0"));
        report.setDataMinorTick(readStringAttribute(reader, attributeName(Attribute::Report::DataMinorTick), "0"));
        report.setYLabelsPrecision(readUintAttribute(reader, attributeName(Attribute::Report::YLabelsPrecision), 2));

    } else if (report.reportType() == eMyMoney::Report::ReportType::QueryTable) {
        // read rows/columns tab
        if (rowTypeFromXML != eMyMoney::Report::RowType::Invalid) {
            report.setRowType(rowTypeFromXML);
        } else {
            report.setRowType(eMyMoney::Report::RowType::Account);
        }

        unsigned qc = 0;
        const auto columns = readStringAttribute(reader, attributeName(Attribute::Report::QueryColumns), "none").split(',');
        for (const auto& column : columns) {
            const int queryColumnFromXML = stringToQueryColumn(column);
            if (queryColumnFromXML != eMyMoney::Report::QueryColumn::End) {
                qc |= queryColumnFromXML;
            }
        }
        report.setQueryColumns(static_cast<eMyMoney::Report::QueryColumn>(qc));

        report.setTax(readBoolAttribute(reader, attributeName(Attribute::Report::Tax), false));
        report.setInvestmentsOnly(readBoolAttribute(reader, attributeName(Attribute::Report::Investments), false));
        report.setLoansOnly(readBoolAttribute(reader, attributeName(Attribute::Report::Loans), false));
        report.setHideTransactions(readBoolAttribute(reader, attributeName(Attribute::Report::HideTransactions), false));
        report.setShowingColumnTotals(readBoolAttribute(reader, attributeName(Attribute::Report::ShowColumnTotals), true));
        report.setIncludingTransfers(readBoolAttribute(reader, attributeName(Attribute::Report::IncludesTransfers), false));
        const auto detailLevelFromXML = stringToDetailLevel(readStringAttribute(reader, attributeName(Attribute::Report::Detail), "none"));
        if (detailLevelFromXML == eMyMoney::Report::DetailLevel::All) {
            report.setDetailLevel(detailLevelFromXML);
        } else {
            report.setDetailLevel(eMyMoney::Report::DetailLevel::None);
        }

        // read performance or capital gains tab
        if (report.queryColumns() & eMyMoney::Report::QueryColumn::Performance) {
            report.setInvestmentSum(static_cast<eMyMoney::Report::InvestmentSum>(
                readUintAttribute(reader, attributeName(Attribute::Report::InvestmentSum), static_cast<int>(eMyMoney::Report::InvestmentSum::Period))));
        }

        // read capital gains tab
        if (report.queryColumns() & eMyMoney::Report::QueryColumn::CapitalGain) {
            report.setInvestmentSum(static_cast<eMyMoney::Report::InvestmentSum>(
                readUintAttribute(reader, attributeName(Attribute::Report::InvestmentSum), static_cast<int>(eMyMoney::Report::InvestmentSum::Sold))));
            if (report.investmentSum() == eMyMoney::Report::InvestmentSum::Sold) {
                report.setShowSTLTCapitalGains(readBoolAttribute(reader, attributeName(Attribute::Report::ShowSTLTCapitalGains), false));
                report.setSettlementPeriod(readUintAttribute(reader, attributeName(Attribute::Report::SettlementPeriod), 3));
                report.setTermSeparator(QDate::fromString(
                    readStringAttribute(reader, attributeName(Attribute::Report::TermsSeparator), QDate::currentDate().addYears(-1).toString(Qt::ISODate)),
                    Qt::ISODate));
            }
        }
    } else if (report.reportType() == eMyMoney::Report::ReportType::InfoTable) {
        if (rowTypeFromXML != eMyMoney::Report::RowType::Invalid) {
            report.setRowType(rowTypeFromXML);
        } else {
            report.setRowType(eMyMoney::Report::RowType::AccountInfo);
        }
        report.setShowingRowTotals(readBoolAttribute(reader, attributeName(Attribute::Report::ShowRowTotals), true));
    }

    while (reader->readNextStartElement()) {
        const auto tagName = reader->name();
        if (tagName == elementName(Element::Report::Text) && hasAttribute(reader, attributeName(Attribute::Report::Pattern))) {
            QRegularExpression exp;
            const bool isRegExp(readBoolAttribute(reader, attributeName(Attribute::Report::RegEx), true));
            auto pattern(readStringAttribute(reader, attributeName(Attribute::Report::Pattern)));
            if (!isRegExp) {
                pattern = MyMoneyUtils::convertWildcardToRegularExpression(pattern);
            }
            exp.setPattern(pattern);
            exp.setPatternOptions(readBoolAttribute(reader, attributeName(Attribute::Report::CaseSensitive), true) ? QRegularExpression::NoPatternOption
                                                                                                                   : QRegularExpression::CaseInsensitiveOption);
            report.setTextFilter(exp,
                                 readBoolAttribute(reader, attributeName(Attribute::Report::RegEx), true),
                                 readBoolAttribute(reader, attributeName(Attribute::Report::InvertText), false));

        } else if ((tagName == elementName(Element::Report::Type)) && hasAttribute(reader, attributeName(Attribute::Report::Type))) {
            const auto reportType = stringToTypeAttribute(readStringAttribute(reader, attributeName(Attribute::Report::Type)));
            if (reportType != -1) {
                report.addType(reportType);
            }

        } else if ((tagName == elementName(Element::Report::State)) && hasAttribute(reader, attributeName(Attribute::Report::State))) {
            const auto state = stringToStateAttribute(readStringAttribute(reader, attributeName(Attribute::Report::State)));
            if (state != -1) {
                report.addState(state);
            }

        } else if ((tagName == elementName(Element::Report::Validity)) && hasAttribute(reader, attributeName(Attribute::Report::Validity))) {
            const auto validity = stringToValidityAttribute(readStringAttribute(reader, attributeName(Attribute::Report::Validity)));
            if (validity != -1) {
                report.addValidity(validity);
            }

        } else if (tagName == elementName(Element::Report::Number)) {
            report.setNumberFilter(readStringAttribute(reader, attributeName(Attribute::Report::From)),
                                   readStringAttribute(reader, attributeName(Attribute::Report::To)));

        } else if (tagName == elementName(Element::Report::Amount)) {
            report.setAmountFilter(readValueAttribute(reader, attributeName(Attribute::Report::From)),
                                   readValueAttribute(reader, attributeName(Attribute::Report::To)));

        } else if (tagName == elementName(Element::Report::Dates)) {
            const auto from(readDateAttribute(reader, attributeName(Attribute::Report::From)));
            const auto to(readDateAttribute(reader, attributeName(Attribute::Report::To)));
            report.setDateFilter(from, to);

        } else if (tagName == elementName(Element::Report::Payee)) {
            report.addPayee(readStringAttribute(reader, attributeName(Attribute::Report::ID)));

        } else if (tagName == elementName(Element::Report::Tag)) {
            report.addTag(readStringAttribute(reader, attributeName(Attribute::Report::ID)));

        } else if (tagName == elementName(Element::Report::Category) && hasAttribute(reader, attributeName(Attribute::Report::ID))) {
            report.addCategory(readStringAttribute(reader, attributeName(Attribute::Report::ID)));

        } else if (tagName == elementName(Element::Report::Account) && hasAttribute(reader, attributeName(Attribute::Report::ID))) {
            report.addAccount(readStringAttribute(reader, attributeName(Attribute::Report::ID)));
        }

        reader->skipCurrentElement();
    }
    return report;
}

void writeStartObject(QXmlStreamWriter* writer, const QString tagName, const QString& id)
{
    writer->writeStartElement(tagName);
    writer->writeAttribute(attributeName(Attribute::Report::ID), id);
}

void writeBudget(const MyMoneyBudget& budget, QXmlStreamWriter* writer)
{
    const auto BUDGET_VERSION = attrValue(2);

    writeStartObject(writer, nodeName(Node::Budget), budget.id());

    writer->writeAttribute(attributeName(Attribute::Budget::Name), budget.name());
    writer->writeAttribute(attributeName(Attribute::Budget::Start), MyMoneyUtils::dateToString(budget.budgetStart()));
    writer->writeAttribute(attributeName(Attribute::Budget::Version), BUDGET_VERSION);

    QMap<QString, MyMoneyBudget::AccountGroup>::const_iterator it;
    auto accounts = budget.accountsMap();
    for (it = accounts.cbegin(); it != accounts.cend(); ++it) {
        // only add the account if there is a budget entered
        // or it covers some sub accounts
        if (!(*it).balance().isZero() || (*it).budgetSubaccounts()) {
            writer->writeStartElement(elementName(Element::Budget::Account));
            writer->writeAttribute(attributeName(Attribute::Budget::ID), it.key());
            writer->writeAttribute(attributeName(Attribute::Budget::BudgetLevel), budgetLevels(it.value().budgetLevel()));
            writer->writeAttribute(attributeName(Attribute::Budget::BudgetSubAccounts), attrValue(it.value().budgetSubaccounts()));

            const QMap<QDate, MyMoneyBudget::PeriodGroup> periods = it.value().getPeriods();
            QMap<QDate, MyMoneyBudget::PeriodGroup>::const_iterator it_per;
            for (it_per = periods.begin(); it_per != periods.end(); ++it_per) {
                if (!(*it_per).amount().isZero()) {
                    writer->writeStartElement(elementName(Element::Budget::Period));
                    writer->writeAttribute(attributeName(Attribute::Budget::Amount), (*it_per).amount().toString());
                    writer->writeAttribute(attributeName(Attribute::Budget::Start), MyMoneyUtils::dateToString((*it_per).startDate()));
                    writer->writeEndElement();
                }
            }

            writer->writeEndElement();
        }
    }

    writer->writeEndElement();
}

void writeReport(const MyMoneyReport& report, QXmlStreamWriter* writer)
{
    writeStartObject(writer, nodeName(Node::Report), report.id());

    // No matter what changes, be sure to have a 'type' attribute.  Only change
    // the major type if it becomes impossible to maintain compatibility with
    // older versions of the program as new features are added to the reports.
    // Feel free to change the minor type every time a change is made here.

    // write report's internals
    if (report.reportType() == eMyMoney::Report::ReportType::PivotTable)
        writer->writeAttribute(attributeName(Attribute::Report::Type), "pivottable 1.15");
    else if (report.reportType() == eMyMoney::Report::ReportType::QueryTable)
        writer->writeAttribute(attributeName(Attribute::Report::Type), "querytable 1.15");
    else if (report.reportType() == eMyMoney::Report::ReportType::InfoTable)
        writer->writeAttribute(attributeName(Attribute::Report::Type), "infotable 1.0");

    writer->writeAttribute(attributeName(Attribute::Report::Group), report.group());

    // write general tab
    writer->writeAttribute(attributeName(Attribute::Report::Name), report.name());
    writer->writeAttribute(attributeName(Attribute::Report::Comment), report.comment());

    writer->writeAttribute(attributeName(Attribute::Report::ConvertCurrency), attrValue(report.isConvertCurrency()));
    writer->writeAttribute(attributeName(Attribute::Report::Favorite), attrValue(report.isFavorite()));
    writer->writeAttribute(attributeName(Attribute::Report::SkipZero), attrValue(report.isSkippingZero()));

    writer->writeAttribute(attributeName(Attribute::Report::DateLock), dateLockAttributeToString(report.dateRange()));
    writer->writeAttribute(attributeName(Attribute::Report::RowType), reportNames(report.rowType()));

    if (report.reportType() == eMyMoney::Report::ReportType::PivotTable) {
        // write report's internals
        writer->writeAttribute(attributeName(Attribute::Report::IncludesActuals), attrValue(report.isIncludingBudgetActuals()));
        writer->writeAttribute(attributeName(Attribute::Report::IncludesForecast), attrValue(report.isIncludingForecast()));
        writer->writeAttribute(attributeName(Attribute::Report::IncludesPrice), attrValue(report.isIncludingPrice()));
        writer->writeAttribute(attributeName(Attribute::Report::IncludesAveragePrice), attrValue(report.isIncludingAveragePrice()));
        writer->writeAttribute(attributeName(Attribute::Report::MixedTime), attrValue(report.isMixedTime()));
        writer->writeAttribute(attributeName(Attribute::Report::Investments),
                               attrValue(report.isInvestmentsOnly())); // it's setable in rows/columns tab of querytable, but here it is internal setting

        // write rows/columns tab
        if (!report.budget().isEmpty())
            writer->writeAttribute(attributeName(Attribute::Report::Budget), report.budget());

        writer->writeAttribute(attributeName(Attribute::Report::ShowRowTotals), attrValue(report.isShowingRowTotals()));
        writer->writeAttribute(attributeName(Attribute::Report::ShowColumnTotals), attrValue(report.isShowingColumnTotals()));
        writer->writeAttribute(attributeName(Attribute::Report::Detail), reportNames(report.detailLevel()));

        writer->writeAttribute(attributeName(Attribute::Report::IncludesMovingAverage), attrValue(report.isIncludingMovingAverage()));
        if (report.isIncludingMovingAverage())
            writer->writeAttribute(attributeName(Attribute::Report::MovingAverageDays), attrValue(report.movingAverageDays()));

        writer->writeAttribute(attributeName(Attribute::Report::IncludesSchedules), attrValue(report.isIncludingSchedules()));
        writer->writeAttribute(attributeName(Attribute::Report::IncludesTransfers), attrValue(report.isIncludingTransfers()));
        writer->writeAttribute(attributeName(Attribute::Report::IncludesUnused), attrValue(report.isIncludingUnusedAccounts()));
        writer->writeAttribute(attributeName(Attribute::Report::ColumnsAreDays), attrValue(report.isColumnsAreDays()));
        writer->writeAttribute(attributeName(Attribute::Report::ChartType), reportNames(report.chartType()));
        writer->writeAttribute(attributeName(Attribute::Report::ChartPalette), reportNames(report.chartPalette()));
        writer->writeAttribute(attributeName(Attribute::Report::ChartCHGridLines), attrValue(report.isChartCHGridLines()));
        writer->writeAttribute(attributeName(Attribute::Report::ChartSVGridLines), attrValue(report.isChartSVGridLines()));
        writer->writeAttribute(attributeName(Attribute::Report::ChartDataLabels), attrValue(report.isChartDataLabels()));
        writer->writeAttribute(attributeName(Attribute::Report::ChartByDefault), attrValue(report.isChartByDefault()));
        writer->writeAttribute(attributeName(Attribute::Report::LogYAxis), attrValue(report.isLogYAxis()));
        writer->writeAttribute(attributeName(Attribute::Report::NegExpenses), attrValue(report.isNegExpenses()));
        writer->writeAttribute(attributeName(Attribute::Report::ChartLineWidth), attrValue(report.chartLineWidth()));
        writer->writeAttribute(attributeName(Attribute::Report::ColumnType), reportNames(report.columnType()));
        writer->writeAttribute(attributeName(Attribute::Report::DataLock), reportNames(report.dataFilter()));
        writer->writeAttribute(attributeName(Attribute::Report::DataRangeStart), report.dataRangeStart());
        writer->writeAttribute(attributeName(Attribute::Report::DataRangeEnd), report.dataRangeEnd());
        writer->writeAttribute(attributeName(Attribute::Report::DataMajorTick), report.dataMajorTick());
        writer->writeAttribute(attributeName(Attribute::Report::DataMinorTick), report.dataMinorTick());
        writer->writeAttribute(attributeName(Attribute::Report::YLabelsPrecision), attrValue(report.yLabelsPrecision()));

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
        writer->writeAttribute(attributeName(Attribute::Report::QueryColumns), columns.join(","));

        writer->writeAttribute(attributeName(Attribute::Report::Tax), attrValue(report.isTax()));
        writer->writeAttribute(attributeName(Attribute::Report::Investments), attrValue(report.isInvestmentsOnly()));
        writer->writeAttribute(attributeName(Attribute::Report::Loans), attrValue(report.isLoansOnly()));
        writer->writeAttribute(attributeName(Attribute::Report::HideTransactions), attrValue(report.isHideTransactions()));
        writer->writeAttribute(attributeName(Attribute::Report::ShowColumnTotals), attrValue(report.isShowingColumnTotals()));
        writer->writeAttribute(attributeName(Attribute::Report::Detail), reportNames(report.detailLevel()));
        writer->writeAttribute(attributeName(Attribute::Report::IncludesTransfers), attrValue(report.isIncludingTransfers()));

        // write performance tab
        if (report.queryColumns() & eMyMoney::Report::QueryColumn::Performance || report.queryColumns() & eMyMoney::Report::QueryColumn::CapitalGain)
            writer->writeAttribute(attributeName(Attribute::Report::InvestmentSum), attrValue(static_cast<int>(report.investmentSum())));

        // write capital gains tab
        if (report.queryColumns() & eMyMoney::Report::QueryColumn::CapitalGain) {
            if (report.investmentSum() == eMyMoney::Report::InvestmentSum::Sold) {
                writer->writeAttribute(attributeName(Attribute::Report::SettlementPeriod), attrValue(report.settlementPeriod()));
                writer->writeAttribute(attributeName(Attribute::Report::ShowSTLTCapitalGains), attrValue(report.isShowingSTLTCapitalGains()));
                writer->writeAttribute(attributeName(Attribute::Report::TermsSeparator), MyMoneyUtils::dateToString(report.termSeparator()));
            }
        }
    } else if (report.reportType() == eMyMoney::Report::ReportType::InfoTable)
        writer->writeAttribute(attributeName(Attribute::Report::ShowRowTotals), attrValue(report.isShowingRowTotals()));

    //
    // Text Filter
    //

    QRegularExpression textfilter;
    bool isRegExp;
    if (report.textFilter(textfilter, isRegExp)) {
        auto pattern(textfilter.pattern());
        if (!isRegExp) {
            pattern = MyMoneyUtils::convertRegularExpressionToWildcard(pattern);
        }
        writer->writeStartElement(elementName(Element::Report::Text));
        writer->writeAttribute(attributeName(Attribute::Report::Pattern), pattern);
        writer->writeAttribute(attributeName(Attribute::Report::CaseSensitive),
                               attrValue((textfilter.patternOptions() & QRegularExpression::CaseInsensitiveOption) ? 0 : 1));
        writer->writeAttribute(attributeName(Attribute::Report::RegEx), attrValue(isRegExp));
        writer->writeAttribute(attributeName(Attribute::Report::InvertText), attrValue(report.MyMoneyTransactionFilter::isInvertingText()));
        writer->writeEndElement();
    }

    //
    // Type & State Filters
    //
    QList<int> typelist;
    if (report.types(typelist) && !typelist.empty()) {
        // iterate over payees, and add each one
        QList<int>::const_iterator it_type = typelist.constBegin();
        while (it_type != typelist.constEnd()) {
            writer->writeStartElement(elementName(Element::Report::Type));
            writer->writeAttribute(attributeName(Attribute::Report::Type), typeAttributeToString(*it_type));
            writer->writeEndElement();

            ++it_type;
        }
    }

    QList<int> statelist;
    if (report.states(statelist) && !statelist.empty()) {
        // iterate over payees, and add each one
        QList<int>::const_iterator it_state = statelist.constBegin();
        while (it_state != statelist.constEnd()) {
            writer->writeStartElement(elementName(Element::Report::State));
            writer->writeAttribute(attributeName(Attribute::Report::State), stateAttributeToString(*it_state));
            writer->writeEndElement();

            ++it_state;
        }
    }

    QList<int> validitylist;
    if (report.validities(validitylist) && !validitylist.empty()) {
        // iterate over payees, and add each one
        QList<int>::const_iterator it_validity = validitylist.constBegin();
        while (it_validity != validitylist.constEnd()) {
            writer->writeStartElement(elementName(Element::Report::Validity));
            writer->writeAttribute(attributeName(Attribute::Report::Validity), validityAttributeToString(*it_validity));
            writer->writeEndElement();

            ++it_validity;
        }
    }

    //
    // Number Filter
    //

    QString nrFrom, nrTo;
    if (report.numberFilter(nrFrom, nrTo)) {
        writer->writeStartElement(elementName(Element::Report::Number));
        writer->writeAttribute(attributeName(Attribute::Report::From), nrFrom);
        writer->writeAttribute(attributeName(Attribute::Report::To), nrTo);
        writer->writeEndElement();
    }

    //
    // Amount Filter
    //

    MyMoneyMoney from, to;
    if (report.amountFilter(from, to)) { // bool getAmountFilter(MyMoneyMoney&,MyMoneyMoney&);
        writer->writeStartElement(elementName(Element::Report::Amount));
        writer->writeAttribute(attributeName(Attribute::Report::From), from.toString());
        writer->writeAttribute(attributeName(Attribute::Report::To), to.toString());
        writer->writeEndElement();
    }

    //
    // Payees Filter
    //

    QStringList payeelist;
    if (report.payees(payeelist)) {
        if (payeelist.empty()) {
            writer->writeEmptyElement(elementName(Element::Report::Payee));
        } else {
            // iterate over payees, and add each one
            payeelist.sort();
            QStringList::const_iterator it_payee = payeelist.constBegin();
            while (it_payee != payeelist.constEnd()) {
                writer->writeStartElement(elementName(Element::Report::Payee));
                writer->writeAttribute(attributeName(Attribute::Report::ID), *it_payee);
                writer->writeEndElement();

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
            writer->writeEmptyElement(elementName(Element::Report::Tag));
        } else {
            // iterate over tags, and add each one
            QStringList::const_iterator it_tag = taglist.constBegin();
            while (it_tag != taglist.constEnd()) {
                writer->writeStartElement(elementName(Element::Report::Tag));
                writer->writeAttribute(attributeName(Attribute::Report::ID), *it_tag);
                writer->writeEndElement();

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
            writer->writeStartElement(elementName(Element::Report::AccountGroup));
            writer->writeAttribute(attributeName(Attribute::Report::Group), accountTypeAttributeToString(*it_group));
            writer->writeEndElement();

            ++it_group;
        }
    }

    //
    // Accounts Filter
    //

    QStringList accountlist;
    if (report.accounts(accountlist)) {
        // iterate over accounts, and add each one
        accountlist.sort();
        QStringList::const_iterator it_account = accountlist.constBegin();
        while (it_account != accountlist.constEnd()) {
            writer->writeStartElement(elementName(Element::Report::Account));
            writer->writeAttribute(attributeName(Attribute::Report::ID), *it_account);
            writer->writeEndElement();

            ++it_account;
        }
    }

    //
    // Categories Filter
    //

    accountlist.clear();
    if (report.categories(accountlist)) {
        // iterate over accounts, and add each one
        accountlist.sort();
        QStringList::const_iterator it_account = accountlist.constBegin();
        while (it_account != accountlist.constEnd()) {
            writer->writeStartElement(elementName(Element::Report::Category));
            writer->writeAttribute(attributeName(Attribute::Report::ID), *it_account);
            writer->writeEndElement();

            ++it_account;
        }
    }

    //
    // Date Filter
    //

    if (report.dateRange() == eMyMoney::TransactionFilter::Date::UserDefined) {
        QDate dateFrom, dateTo;
        if (report.dateFilter(dateFrom, dateTo)) {
            writer->writeStartElement(elementName(Element::Report::Dates));
            if (dateFrom.isValid())
                writer->writeAttribute(attributeName(Attribute::Report::From), MyMoneyUtils::dateToString(dateFrom));
            if (dateTo.isValid())
                writer->writeAttribute(attributeName(Attribute::Report::To), MyMoneyUtils::dateToString(dateTo));
            writer->writeEndElement();
        }
    }

    writer->writeEndElement();
}

MyMoneyBudget readBudget(QXmlStreamReader* reader)
{
    Q_ASSERT(reader->isStartElement() && (reader->name() == nodeName(Node::Budget)));

    MyMoneyBudget budget(readId(reader));

    // The goal of this reading method is 100% backward AND 100% forward
    // compatibility.  Any Budget ever created with any version of KMyMoney
    // should be able to be loaded by this method (as long as it's one of the
    // Budget types supported in this version, of course)

    budget.setName(readStringAttribute(reader, attributeName(Attribute::Budget::Name)));
    budget.setBudgetStart(readDateAttribute(reader, attributeName(Attribute::Budget::Start)));

    while (reader->readNextStartElement()) {
        const auto tagName = reader->name();
        if (tagName == elementName(Element::Budget::Account)) {
            MyMoneyBudget::AccountGroup account;
            account.setId(readStringAttribute(reader, attributeName(Attribute::Budget::ID)));
            account.setBudgetLevel(stringToBudgetLevel(readStringAttribute(reader, attributeName(Attribute::Budget::BudgetLevel))));
            account.setBudgetSubaccounts(readBoolAttribute(reader, attributeName(Attribute::Budget::BudgetSubAccounts), false));

            while (reader->readNextStartElement()) {
                if (reader->name() == elementName(Element::Budget::Period)) {
                    if (hasAttribute(reader, attributeName(Attribute::Budget::Amount)) && hasAttribute(reader, attributeName(Attribute::Budget::Start))) {
                        MyMoneyBudget::PeriodGroup pGroup;
                        pGroup.setAmount(readValueAttribute(reader, attributeName(Attribute::Budget::Amount)));
                        pGroup.setStartDate(readDateAttribute(reader, attributeName(Attribute::Budget::Start)));
                        account.addPeriod(pGroup.startDate(), pGroup);
                    }
                }
                reader->skipCurrentElement();
            }

            budget.setAccount(account, account.id());

        } else {
            reader->skipCurrentElement();
        }
    }
    return budget;
}

} // namespace
