/***************************************************************************
                          mymoneyreport.cpp
                             -------------------
    begin                : Sun July 4 2004
    copyright            : (C) 2004-2005 by Ace Jones
    email                : acejones@users.sourceforge.net
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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

using namespace eMyMoney;

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
  MyMoneyReport::EDetailLevel m_detailLevel;
  /**
    * Whether to sum: all, sold, bought or owned value
    */
  MyMoneyReport::EInvestmentSum m_investmentSum;
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
  MyMoneyReport::EReportType m_reportType;
  /**
    * What sort of values should show up on the ROWS of this report
    */
  MyMoneyReport::ERowType m_rowType;
  /**
    * What sort of values should show up on the COLUMNS of this report,
    * in the case of a 'PivotTable' report.  Really this is used more as a
    * QUANTITY of months or days.  Whether it's months or days is determined
    * by m_columnsAreDays.
    */
  MyMoneyReport::EColumnType m_columnType;
  /**
   * Whether the base unit of columns of this report is days.  Only applies to
   * 'PivotTable' reports.  If false, then columns are months or multiples thereof.
   */
  bool m_columnsAreDays;
  /**
     * What sort of values should show up on the COLUMNS of this report,
     * in the case of a 'QueryTable' report
     */
  MyMoneyReport::EQueryColumns m_queryColumns;

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
  QList<Account::Type> m_accountGroups;
  /**
    * Whether an account group filter has been set (see m_accountGroups)
    */
  bool m_accountGroupFilter;
  /**
    * What format should be used to draw this report as a chart
    */
  MyMoneyReport::EChartType m_chartType;
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
  MyMoneyReport::dataOptionE m_dataLock;

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
