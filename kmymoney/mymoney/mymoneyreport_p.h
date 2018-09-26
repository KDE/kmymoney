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
   * This value is calculated dynamically and thus it is not saved in the file
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
