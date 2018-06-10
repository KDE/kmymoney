/*
 * Copyright 2004-2006  Ace Jones <acejones@users.sourceforge.net>
 * Copyright 2006       Darren Gould <darren_gould@gmx.de>
 * Copyright 2007-2010  Alvaro Soliverez <asoliverez@gmail.com>
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

#ifndef MYMONEYREPORT_P_H
#define MYMONEYREPORT_P_H

#include "mymoneyreport.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>
#include <QHash>
#include <QDate>
#include <QList>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyobject_p.h"
#include "mymoneyenums.h"

namespace Report
{
  enum class Element { Payee,
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
  uint qHash(const Element key, uint seed) { return ::qHash(static_cast<uint>(key), seed); }

  enum class Attribute { ID, Group, Type, Name, Comment, ConvertCurrency, Favorite,
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
  uint qHash(const Attribute key, uint seed) { return ::qHash(static_cast<uint>(key), seed); }
}

class MyMoneyReportPrivate : public MyMoneyObjectPrivate
{
public:
  MyMoneyReportPrivate() :
    m_name(QStringLiteral("Unconfigured Pivot Table Report")),
    m_detailLevel(eMyMoney::Report::DetailLevel::None),
    m_investmentSum(eMyMoney::Report::InvestmentSum::Sold),
    m_hideTransactions(false),
    m_convertCurrency(true),
    m_favorite(false),
    m_tax(false),
    m_investments(false),
    m_loans(false),
    m_reportType(rowTypeToReportType(eMyMoney::Report::RowType::ExpenseIncome)),
    m_rowType(eMyMoney::Report::RowType::ExpenseIncome),
    m_columnType(eMyMoney::Report::ColumnType::Months),
    m_columnsAreDays(false),
    m_queryColumns(eMyMoney::Report::QueryColumn::None),
    m_dateLock(eMyMoney::TransactionFilter::Date::UserDefined),
    m_accountGroupFilter(false),
    m_chartType(eMyMoney::Report::ChartType::Line),
    m_chartDataLabels(true),
    m_chartCHGridLines(true),
    m_chartSVGridLines(true),
    m_chartByDefault(false),
    m_chartLineWidth(MyMoneyReport::m_lineWidth),
    m_logYaxis(false),
    m_dataRangeStart('0'),
    m_dataRangeEnd('0'),
    m_dataMajorTick('0'),
    m_dataMinorTick('0'),
    m_yLabelsPrecision(2),
    m_dataLock(eMyMoney::Report::DataLock::Automatic),
    m_includeSchedules(false),
    m_includeTransfers(false),
    m_includeBudgetActuals(false),
    m_includeUnusedAccounts(false),
    m_showRowTotals(false),
    m_showColumnTotals(true),
    m_includeForecast(false),
    m_includeMovingAverage(false),
    m_movingAverageDays(0),
    m_includePrice(false),
    m_includeAveragePrice(false),
    m_mixedTime(false),
    m_currentDateColumn(0),
    m_settlementPeriod(3),
    m_showSTLTCapitalGains(false),
    m_tseparator(QDate::currentDate().addYears(-1)),
    m_skipZero(false)
  {
  }

  static QString getElName(const Report::Element el)
  {
    static const QHash<Report::Element, QString> elNames {
      {Report::Element::Payee,        QStringLiteral("PAYEE")},
      {Report::Element::Tag,          QStringLiteral("TAG")},
      {Report::Element::Account,      QStringLiteral("ACCOUNT")},
      {Report::Element::Text,         QStringLiteral("TEXT")},
      {Report::Element::Type,         QStringLiteral("TYPE")},
      {Report::Element::State,        QStringLiteral("STATE")},
      {Report::Element::Number,       QStringLiteral("NUMBER")},
      {Report::Element::Amount,       QStringLiteral("AMOUNT")},
      {Report::Element::Dates,        QStringLiteral("DATES")},
      {Report::Element::Category,     QStringLiteral("CATEGORY")},
      {Report::Element::AccountGroup, QStringLiteral("ACCOUNTGROUP")}
    };
    return elNames[el];
  }

  static QString getAttrName(const Report::Attribute attr)
  {
    static const QHash<Report::Attribute, QString> attrNames {
      {Report::Attribute::ID,                     QStringLiteral("id")},
      {Report::Attribute::Group,                  QStringLiteral("group")},
      {Report::Attribute::Type,                   QStringLiteral("type")},
      {Report::Attribute::Name,                   QStringLiteral("name")},
      {Report::Attribute::Comment,                QStringLiteral("comment")},
      {Report::Attribute::ConvertCurrency,        QStringLiteral("convertcurrency")},
      {Report::Attribute::Favorite,               QStringLiteral("favorite")},
      {Report::Attribute::SkipZero,               QStringLiteral("skipZero")},
      {Report::Attribute::DateLock,               QStringLiteral("datelock")},
      {Report::Attribute::DataLock,               QStringLiteral("datalock")},
      {Report::Attribute::MovingAverageDays,      QStringLiteral("movingaveragedays")},
      {Report::Attribute::IncludesActuals,        QStringLiteral("includesactuals")},
      {Report::Attribute::IncludesForecast,       QStringLiteral("includesforecast")},
      {Report::Attribute::IncludesPrice,          QStringLiteral("includesprice")},
      {Report::Attribute::IncludesAveragePrice,   QStringLiteral("includesaverageprice")},
      {Report::Attribute::IncludesMovingAverage,  QStringLiteral("includesmovingaverage")},
      {Report::Attribute::IncludesSchedules,      QStringLiteral("includeschedules")},
      {Report::Attribute::IncludesTransfers,      QStringLiteral("includestransfers")},
      {Report::Attribute::IncludesUnused,         QStringLiteral("includeunused")},
      {Report::Attribute::MixedTime,              QStringLiteral("mixedtime")},
      {Report::Attribute::Investments,            QStringLiteral("investments")},
      {Report::Attribute::Budget,                 QStringLiteral("budget")},
      {Report::Attribute::ShowRowTotals,          QStringLiteral("showrowtotals")},
      {Report::Attribute::ShowColumnTotals,       QStringLiteral("showcolumntotals")},
      {Report::Attribute::Detail,                 QStringLiteral("detail")},
      {Report::Attribute::ColumnsAreDays,         QStringLiteral("columnsaredays")},
      {Report::Attribute::ChartType,              QStringLiteral("charttype")},
      {Report::Attribute::ChartCHGridLines,       QStringLiteral("chartchgridlines")},
      {Report::Attribute::ChartSVGridLines,       QStringLiteral("chartsvgridlines")},
      {Report::Attribute::ChartDataLabels,        QStringLiteral("chartdatalabels")},
      {Report::Attribute::ChartByDefault,         QStringLiteral("chartbydefault")},
      {Report::Attribute::LogYAxis,               QStringLiteral("logYaxis")},
      {Report::Attribute::ChartLineWidth,         QStringLiteral("chartlinewidth")},
      {Report::Attribute::ColumnType,             QStringLiteral("columntype")},
      {Report::Attribute::RowType,                QStringLiteral("rowtype")},
      {Report::Attribute::DataRangeStart,         QStringLiteral("dataRangeStart")},
      {Report::Attribute::DataRangeEnd,           QStringLiteral("dataRangeEnd")},
      {Report::Attribute::DataMajorTick,          QStringLiteral("dataMajorTick")},
      {Report::Attribute::DataMinorTick,          QStringLiteral("dataMinorTick")},
      {Report::Attribute::YLabelsPrecision,       QStringLiteral("yLabelsPrecision")},
      {Report::Attribute::QueryColumns,           QStringLiteral("querycolumns")},
      {Report::Attribute::Tax,                    QStringLiteral("tax")},
      {Report::Attribute::Loans,                  QStringLiteral("loans")},
      {Report::Attribute::HideTransactions,       QStringLiteral("hidetransactions")},
      {Report::Attribute::InvestmentSum,          QStringLiteral("investmentsum")},
      {Report::Attribute::SettlementPeriod,       QStringLiteral("settlementperiod")},
      {Report::Attribute::ShowSTLTCapitalGains,   QStringLiteral("showSTLTCapitalGains")},
      {Report::Attribute::TermsSeparator,         QStringLiteral("tseparator")},
      {Report::Attribute::Pattern,                QStringLiteral("pattern")},
      {Report::Attribute::CaseSensitive,          QStringLiteral("casesensitive")},
      {Report::Attribute::RegEx,                  QStringLiteral("regex")},
      {Report::Attribute::InvertText,             QStringLiteral("inverttext")},
      {Report::Attribute::State,                  QStringLiteral("state")},
      {Report::Attribute::From,                   QStringLiteral("from")},
      {Report::Attribute::To,                     QStringLiteral("to")}
    };
    return attrNames[attr];
  }

  static QHash<eMyMoney::Report::RowType, QString> rowTypesLUT()
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

  static QString reportNames(eMyMoney::Report::RowType textID)
  {
    return rowTypesLUT().value(textID);
  }

  static eMyMoney::Report::RowType stringToRowType(const QString &text)
  {
    return rowTypesLUT().key(text, eMyMoney::Report::RowType::Invalid);
  }

  static QHash<eMyMoney::Report::ColumnType, QString> columTypesLUT()
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

  static QString reportNames(eMyMoney::Report::ColumnType textID)
  {
    return columTypesLUT().value(textID);
  }

  static eMyMoney::Report::ColumnType stringToColumnType(const QString &text)
  {
    return columTypesLUT().key(text, eMyMoney::Report::ColumnType::Invalid);
  }

  static QHash<eMyMoney::Report::QueryColumn, QString> queryColumnsLUT()
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

  static QString reportNamesForQC(eMyMoney::Report::QueryColumn textID)
  {
    return queryColumnsLUT().value(textID);
  }

  static eMyMoney::Report::QueryColumn stringToQueryColumn(const QString &text)
  {
    return queryColumnsLUT().key(text, eMyMoney::Report::QueryColumn::End);
  }

  static QHash<eMyMoney::Report::DetailLevel, QString> detailLevelLUT()
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

  static QString reportNames(eMyMoney::Report::DetailLevel textID)
  {
    return detailLevelLUT().value(textID);
  }

  static eMyMoney::Report::DetailLevel stringToDetailLevel(const QString &text)
  {
    return detailLevelLUT().key(text, eMyMoney::Report::DetailLevel::End);
  }

  static QHash<eMyMoney::Report::ChartType, QString> chartTypeLUT()
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

  static QString reportNames(eMyMoney::Report::ChartType textID)
  {
    return chartTypeLUT().value(textID);
  }

  static eMyMoney::Report::ChartType stringToChartType(const QString &text)
  {
    return chartTypeLUT().key(text, eMyMoney::Report::ChartType::End);
  }

  static QHash<int, QString> typeAttributeLUT()
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

  static QString typeAttributeToString(int textID)
  {
    return typeAttributeLUT().value(textID);
  }

  static int stringToTypeAttribute(const QString &text)
  {
    return typeAttributeLUT().key(text, 4);
  }

  static QHash<int, QString> stateAttributeLUT()
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

  static QString stateAttributeToString(int textID)
  {
    return stateAttributeLUT().value(textID);
  }

  static int stringToStateAttribute(const QString &text)
  {
    return stateAttributeLUT().key(text, 5);
  }

  static QHash<int, QString> dateLockLUT()
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

  static QString dateLockAttributeToString(int textID)
  {
    return dateLockLUT().value(textID);
  }

  static int stringToDateLockAttribute(const QString &text)
  {
    return dateLockLUT().key(text, 0);
  }

  static QHash<eMyMoney::Report::DataLock, QString> dataLockLUT()
  {
    static const QHash<eMyMoney::Report::DataLock, QString> lut {
      {eMyMoney::Report::DataLock::Automatic,   QStringLiteral("automatic")},
      {eMyMoney::Report::DataLock::UserDefined, QStringLiteral("userdefined")}
    };
    return lut;
  }

  static QString reportNames(eMyMoney::Report::DataLock textID)
  {
    return dataLockLUT().value(textID);
  }

  static eMyMoney::Report::DataLock stringToDataLockAttribute(const QString &text)
  {
    return dataLockLUT().key(text, eMyMoney::Report::DataLock::DataOptionCount);
  }

  static QHash<int, QString> accountTypeAttributeLUT()
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

  static QString accountTypeAttributeToString(int textID)
  {
    return accountTypeAttributeLUT().value(textID);
  }

  static int stringToAccountTypeAttribute(const QString &text)
  {
    return accountTypeAttributeLUT().key(text, 0);
  }

  static eMyMoney::Report::ReportType rowTypeToReportType(eMyMoney::Report::RowType rowType)
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

  /**
    * The user-assigned name of the report
    */
  QString m_name;
  /**
    * The user-assigned comment for the report, in case they want to make
    * additional notes for themselves about the report.
    */
  QString m_comment;
  /**
    * Where to group this report amongst the others in the UI view.  This
    * should be assigned by the UI system.
    */
  QString m_group;
  /**
    * How much detail to show in the accounts
    */
  eMyMoney::Report::DetailLevel m_detailLevel;
  /**
    * Whether to sum: all, sold, bought or owned value
    */
  eMyMoney::Report::InvestmentSum m_investmentSum;
  /**
    * Whether to show transactions or just totals.
    */
  bool m_hideTransactions;
  /**
    * Whether to convert all currencies to the base currency of the file (true).
    * If this is false, it's up to the report generator to decide how to handle
    * the currency.
    */
  bool m_convertCurrency;
  /**
    * Whether this is one of the users' favorite reports
    */
  bool m_favorite;
  /**
    * Whether this report should only include categories marked as "Tax"="Yes"
    */
  bool m_tax;
  /**
    * Whether this report should only include investment accounts
    */
  bool m_investments;
  /**
    * Whether this report should only include loan accounts
    * Applies only to querytable reports.  Mutually exclusive with
    * m_investments.
    */
  bool m_loans;
  /**
    * What sort of algorithm should be used to run the report
    */
  eMyMoney::Report::ReportType m_reportType;
  /**
    * What sort of values should show up on the ROWS of this report
    */
  eMyMoney::Report::RowType m_rowType;
  /**
    * What sort of values should show up on the COLUMNS of this report,
    * in the case of a 'PivotTable' report.  Really this is used more as a
    * QUANTITY of months or days.  Whether it's months or days is determined
    * by m_columnsAreDays.
    */
  eMyMoney::Report::ColumnType m_columnType;
  /**
   * Whether the base unit of columns of this report is days.  Only applies to
   * 'PivotTable' reports.  If false, then columns are months or multiples thereof.
   */
  bool m_columnsAreDays;
  /**
     * What sort of values should show up on the COLUMNS of this report,
     * in the case of a 'QueryTable' report
     */
  eMyMoney::Report::QueryColumn m_queryColumns;

  /**
    * The plain-language description of what the date range should be locked
    * to.  'userDefined' means NO locking, in any other case, the report
    * will be adjusted to match the date lock.  So if the date lock is
    * 'currentMonth', the start and end dates of the underlying filter will
    * be updated to whatever the current month is.  This updating happens
    * automatically when the report is loaded, and should also be done
    * manually by calling updateDateFilter() before generating the report
    */
  eMyMoney::TransactionFilter::Date m_dateLock;
  /**
    * Which account groups should be included in the report.  This filter
    * is applied to the individual splits AFTER a transaction has been
    * matched using the underlying filter.
    */
  QList<eMyMoney::Account::Type> m_accountGroups;
  /**
    * Whether an account group filter has been set (see m_accountGroups)
    */
  bool m_accountGroupFilter;
  /**
    * What format should be used to draw this report as a chart
    */
  eMyMoney::Report::ChartType m_chartType;
  /**
    * Whether the value of individual data points should be drawn on the chart
    */
  bool m_chartDataLabels;
  /**
    * Whether grid lines should be drawn on the chart
    */
  bool m_chartCHGridLines;
  bool m_chartSVGridLines;
  /**
    * Whether this report should be shown as a chart by default (otherwise it
    * should be shown as a textual report)
    */
  bool m_chartByDefault;
  /**
   * Width of the chart lines
   */
  uint m_chartLineWidth;

  /**
    * Whether Y axis is logarithmic or linear
    */
  bool m_logYaxis;

  /**
    * Y data range
    */
  QString m_dataRangeStart;
  QString m_dataRangeEnd;

  /**
    * Y data range division
    */
  QString m_dataMajorTick;
  QString m_dataMinorTick;

  /**
    * Y labels precision
    */
  uint m_yLabelsPrecision;

  /**
    * Whether data range should be calculated automatically or is user defined
    */
  eMyMoney::Report::DataLock m_dataLock;

  /**
    * Whether to include scheduled transactions
    */
  bool m_includeSchedules;
  /**
    * Whether to include transfers.  Only applies to Income/Expense reports
    */
  bool m_includeTransfers;
  /**
    * The id of the budget associated with this report.
    */
  QString m_budgetId;
  /**
    * Whether this report should print the actual data to go along with
    * the budget.  This is only valid if the report has a budget.
    */
  bool m_includeBudgetActuals;
  /**
    * Whether this report should include all accounts and not only
    * accounts with transactions.
    */
  bool m_includeUnusedAccounts;
  /**
   * Whether this report should include columns for row totals
   */
  bool m_showRowTotals;
  /**
   * Whether this report should include rows for column totals
   */
  bool m_showColumnTotals;
  /**
   * Whether this report should include forecast balance
   */
  bool m_includeForecast;
  /**
   * Whether this report should include moving average
   */
  bool m_includeMovingAverage;
  /**
   * The amount of days that spans each moving average
   */
  int m_movingAverageDays;
  /**
   * Whether this report should include prices
   */
  bool m_includePrice;
  /**
   * Whether this report should include moving average prices
   */
  bool m_includeAveragePrice;
  /**
   * Make the actual and forecast lines display as one
   */
  bool m_mixedTime;
  /**
   * This stores the column for the current date
   * This value is calculated dinamically and thus it is not saved in the file
   */
  int m_currentDateColumn;
  /**
   * Time in days between the settlement date and the transaction date.
   */
  uint m_settlementPeriod;
  /**
   * Controls showing short-term and long-term capital gains.
   */
  bool m_showSTLTCapitalGains;
  /**
   * Date separating shot-term from long-term gains.
   */
  QDate m_tseparator;

  /**
    * This option is for investments reports only which
    * show prices instead of balances as all other reports do.
    * <p>
    * Select this option to include prices for the given period (week, month,
    * quarter, ...) only.
    * </p>
    * <p>
    * If this option is off the last existing price is shown for a period, if
    * it is on, in a table the value is '0' shown and in a chart a linear
    * interpolation for the missing values will be performed.
    * <br>Example:
    * <br>There are prices for January and March, but there is no price for
    * February.
    * <ul>
    * <li><b>OFF</b>: shows the price for February as the last price of
    * January
    * <li><b>ON</b>: in a table the value is '0', in a chart a linear
    * interpolation for the February-price will be performed
    * (so it makes a kind of average-value using the January- and the
    * March-price in the chart)
    * </ul>
    * </p>
    */
  bool m_skipZero;
};

#endif
