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

#include "mymoneyreport.h"
#include "kmymoneyglobalsettings.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>
#include <QList>
#include <QDomElement>
#include <QDomDocument>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneymoney.h"
#include "mymoneyfile.h"
#include "mymoneyaccount.h"
#include "mymoneystoragenames.h"
#include "mymoneytransaction.h"
#include "mymoneytransactionfilter.h"
#include "mymoneyexception.h"

using namespace eMyMoney;
using namespace MyMoneyStorageNodes;

// define this to debug reports
// #define DEBUG_REPORTS

const QStringList MyMoneyReport::kRowTypeText = QString("none,assetliability,expenseincome,category,topcategory,account,tag,payee,month,week,topaccount,topaccount-account,equitytype,accounttype,institution,budget,budgetactual,schedule,accountinfo,accountloaninfo,accountreconcile,cashflow").split(',');
const QStringList MyMoneyReport::kColumnTypeText = QString("none,months,bimonths,quarters,4,5,6,weeks,8,9,10,11,years").split(',');

// if you add names here, don't forget to update the bitmap for EQueryColumns
// and shift the bit for eQCend one position to the left
const QStringList MyMoneyReport::kQueryColumnsText = QString("none,number,payee,category,tag,memo,account,reconcileflag,action,shares,price,performance,loan,balance,capitalgain").split(',');

const MyMoneyReport::EReportType MyMoneyReport::kTypeArray[] = { eNoReport, ePivotTable, ePivotTable, eQueryTable, eQueryTable, eQueryTable, eQueryTable, eQueryTable, eQueryTable, eQueryTable, eQueryTable, eQueryTable, eQueryTable, eQueryTable, eQueryTable, ePivotTable, ePivotTable, eInfoTable, eInfoTable, eInfoTable, eQueryTable, eQueryTable, eNoReport };
const QStringList MyMoneyReport::kDetailLevelText = QString("none,all,top,group,total,invalid").split(',');
const QStringList MyMoneyReport::kChartTypeText = QString("none,line,bar,pie,ring,stackedbar").split(',');

// This should live in mymoney/mymoneytransactionfilter.h
const QStringList kTypeText = QString("all,payments,deposits,transfers,none").split(',');
const QStringList kStateText = QString("all,notreconciled,cleared,reconciled,frozen,none").split(',');
const QStringList kDateLockText = QString("alldates,untiltoday,currentmonth,currentyear,monthtodate,yeartodate,yeartomonth,lastmonth,lastyear,last7days,last30days,last3months,last6months,last12months,next7days,next30days,next3months,next6months,next12months,userdefined,last3tonext3months,last11Months,currentQuarter,lastQuarter,nextQuarter,currentFiscalYear,lastFiscalYear,today,next18months").split(',');
const QStringList kDataLockText = QString("automatic,userdefined").split(',');
const QStringList kAccountTypeText = QString("unknown,checkings,savings,cash,creditcard,loan,certificatedep,investment,moneymarket,asset,liability,currency,income,expense,assetloan,stock,equity,invalid").split(',');

class MyMoneyReportPrivate {

public:
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
  QList<Account> m_accountGroups;
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

MyMoneyReport::MyMoneyReport() :
  d_ptr(new MyMoneyReportPrivate)
{
  Q_D(MyMoneyReport);
  d->m_name = "Unconfigured Pivot Table Report";
  d->m_detailLevel = eDetailNone;
  d->m_investmentSum = eSumSold;
  d->m_hideTransactions = false;
  d->m_convertCurrency = true;
  d->m_favorite = false;
  d->m_tax = false;
  d->m_investments = false;
  d->m_loans = false;
  d->m_reportType = kTypeArray[eExpenseIncome];
  d->m_rowType = eExpenseIncome;
  d->m_columnType = eMonths;
  d->m_columnsAreDays = false;
  d->m_queryColumns = eQCnone;
  d->m_dateLock = TransactionFilter::Date::UserDefined;
  d->m_accountGroupFilter = false;
  d->m_chartType = eChartLine;
  d->m_chartDataLabels = true;
  d->m_chartCHGridLines = true;
  d->m_chartSVGridLines = true;
  d->m_chartByDefault = false;
  d->m_logYaxis = false;
  d->m_dataRangeStart = '0';
  d->m_dataRangeEnd = '0';
  d->m_dataMajorTick = '0';
  d->m_dataMinorTick = '0';
  d->m_yLabelsPrecision = 2;
  d->m_dataLock = MyMoneyReport::automatic;
  d->m_includeSchedules = false;
  d->m_includeTransfers = false;
  d->m_includeBudgetActuals = false;
  d->m_includeUnusedAccounts = false;
  d->m_showRowTotals = false;
  d->m_showColumnTotals = true;
  d->m_includeForecast = false;
  d->m_includeMovingAverage = false;
  d->m_movingAverageDays = 0;
  d->m_includePrice = false;
  d->m_includeAveragePrice = false;
  d->m_mixedTime = false;
  d->m_currentDateColumn = 0;
  d->m_settlementPeriod = 3;
  d->m_showSTLTCapitalGains = false;
  d->m_tseparator = QDate::currentDate().addYears(-1);
  d->m_skipZero = false;
  d->m_chartLineWidth = m_lineWidth;
}

MyMoneyReport::MyMoneyReport(ERowType rt,
                             unsigned ct,
                             TransactionFilter::Date dl,
                             EDetailLevel ss,
                             const QString& name,
                             const QString& comment) :
  d_ptr(new MyMoneyReportPrivate)
{
  Q_D(MyMoneyReport);
  d->m_name = name;
  d->m_comment = comment;
  d->m_detailLevel = ss;
  d->m_investmentSum = ct & eQCcapitalgain ? eSumSold : eSumPeriod;
  d->m_hideTransactions = false;
  d->m_convertCurrency = true;
  d->m_favorite = false;
  d->m_tax = false;
  d->m_investments = false;
  d->m_loans = false;
  d->m_reportType = kTypeArray[rt];
  d->m_rowType = rt;
  d->m_columnType = eMonths;
  d->m_columnsAreDays = false;
  d->m_queryColumns = eQCnone;
  d->m_dateLock = dl;
  d->m_accountGroupFilter = false;
  d->m_chartType = eChartLine;
  d->m_chartDataLabels = true;
  d->m_chartCHGridLines = true;
  d->m_chartSVGridLines = true;
  d->m_chartByDefault = false;
  d->m_logYaxis = false;
  d->m_dataRangeStart = '0';
  d->m_dataRangeEnd = '0';
  d->m_dataMajorTick = '0';
  d->m_dataMinorTick = '0';
  d->m_yLabelsPrecision = 2;
  d->m_dataLock = MyMoneyReport::automatic;
  d->m_includeSchedules = false;
  d->m_includeTransfers = false;
  d->m_includeBudgetActuals = false;
  d->m_includeUnusedAccounts = false;
  d->m_showRowTotals = false;
  d->m_showColumnTotals = true;
  d->m_includeForecast = false;
  d->m_includeMovingAverage = false;
  d->m_movingAverageDays = 0;
  d->m_includePrice = false;
  d->m_includeAveragePrice = false;
  d->m_mixedTime = false;
  d->m_currentDateColumn = 0;
  d->m_settlementPeriod = 3;
  d->m_showSTLTCapitalGains = false;
  d->m_tseparator = QDate::currentDate().addYears(-1);
  d->m_skipZero = false;

  //set initial values
  d->m_chartLineWidth = m_lineWidth;

  //set report type
  if (d->m_reportType == ePivotTable)
    d->m_columnType = static_cast<EColumnType>(ct);
  if (d->m_reportType == eQueryTable)
    d->m_queryColumns = static_cast<EQueryColumns>(ct);
  setDateFilter(dl);

  //throw exception if the type is inconsistent
  if ((rt > static_cast<ERowType>(sizeof(kTypeArray) / sizeof(kTypeArray[0])))
      || (d->m_reportType == eNoReport))
    throw MYMONEYEXCEPTION("Invalid report type");

  //add the corresponding account groups
  if (rt == MyMoneyReport::eAssetLiability) {
    addAccountGroup(Account::Asset);
    addAccountGroup(Account::Liability);
    d->m_showRowTotals = true;
  }
  if (rt == MyMoneyReport::eAccount) {
    addAccountGroup(Account::Asset);
    addAccountGroup(Account::AssetLoan);
    addAccountGroup(Account::Cash);
    addAccountGroup(Account::Checkings);
    addAccountGroup(Account::CreditCard);
    if (KMyMoneyGlobalSettings::expertMode())
      addAccountGroup(Account::Equity);
    addAccountGroup(Account::Expense);
    addAccountGroup(Account::Income);
    addAccountGroup(Account::Liability);
    addAccountGroup(Account::Loan);
    addAccountGroup(Account::Savings);
    addAccountGroup(Account::Stock);
    d->m_showRowTotals = true;
  }
  if (rt == MyMoneyReport::eExpenseIncome) {
    addAccountGroup(Account::Expense);
    addAccountGroup(Account::Income);
    d->m_showRowTotals = true;
  }
  //FIXME take this out once we have sorted out all issues regarding budget of assets and liabilities -- asoliverez@gmail.com
  if (rt == MyMoneyReport::eBudget || rt == MyMoneyReport::eBudgetActual) {
    addAccountGroup(Account::Expense);
    addAccountGroup(Account::Income);
  }
  if (rt == MyMoneyReport::eAccountInfo) {
    addAccountGroup(Account::Asset);
    addAccountGroup(Account::Liability);
  }
  //cash flow reports show splits for all account groups
  if (rt == MyMoneyReport::eCashFlow) {
    addAccountGroup(Account::Expense);
    addAccountGroup(Account::Income);
    addAccountGroup(Account::Asset);
    addAccountGroup(Account::Liability);
  }
#ifdef DEBUG_REPORTS
  QDebug out = qDebug();
  out << _name << toString(_rt) << toString(m_reportType);
  foreach(const Account accountType, m_accountGroups)
    out << MyMoneyAccount::accountTypeToString(accountType);
  if (m_accounts.size() > 0)
    out << m_accounts;
#endif
}

MyMoneyReport::MyMoneyReport(const QDomElement& node) :
    MyMoneyObject(node),
    d_ptr(new MyMoneyReportPrivate)
{
  Q_D(MyMoneyReport);
  d->m_currentDateColumn = 0;
  // properly initialize the object before reading it
  *this = MyMoneyReport();
  if (!read(node))
    clearId();
}

MyMoneyReport::MyMoneyReport(const MyMoneyReport& other) :
  MyMoneyObject(other.id()),
  MyMoneyTransactionFilter(other),
  d_ptr(new MyMoneyReportPrivate(*other.d_func()))
{
}

MyMoneyReport::MyMoneyReport(const QString& id, const MyMoneyReport& other) :
  MyMoneyObject(id),
  MyMoneyTransactionFilter(other),
  d_ptr(new MyMoneyReportPrivate(*other.d_func()))
{
  Q_D(MyMoneyReport);
  d->m_movingAverageDays = 0;
  d->m_currentDateColumn = 0;
}

MyMoneyReport::~MyMoneyReport()
{
  Q_D(MyMoneyReport);
  delete d;
}

MyMoneyReport::EReportType MyMoneyReport::reportType() const
{
  Q_D(const MyMoneyReport);
  return d->m_reportType;
}

QString MyMoneyReport::name() const
{
  Q_D(const MyMoneyReport);
  return d->m_name;
}

void MyMoneyReport::setName(const QString& s)
{
  Q_D(MyMoneyReport);
  d->m_name = s;
}

bool MyMoneyReport::isShowingRowTotals() const
{
  Q_D(const MyMoneyReport);
  return (d->m_showRowTotals);
}

void MyMoneyReport::setShowingRowTotals(bool f)
{
  Q_D(MyMoneyReport);
  d->m_showRowTotals = f;
}

bool MyMoneyReport::isShowingColumnTotals() const
{
  Q_D(const MyMoneyReport);
  return d->m_showColumnTotals;
}

void MyMoneyReport::setShowingColumnTotals(bool f)
{
  Q_D(MyMoneyReport);
  d->m_showColumnTotals = f;
}

MyMoneyReport::ERowType MyMoneyReport::rowType() const
{
  Q_D(const MyMoneyReport);
  return d->m_rowType;
}

void MyMoneyReport::setRowType(ERowType rt)
{
  Q_D(MyMoneyReport);
  d->m_rowType = rt;
  d->m_reportType = kTypeArray[rt];

  d->m_accountGroupFilter = false;
  d->m_accountGroups.clear();

  if (rt == MyMoneyReport::eAssetLiability) {
    addAccountGroup(Account::Asset);
    addAccountGroup(Account::Liability);
  }
  if (rt == MyMoneyReport::eExpenseIncome) {
    addAccountGroup(Account::Expense);
    addAccountGroup(Account::Income);
  }
}

bool MyMoneyReport::isRunningSum() const
{
  Q_D(const MyMoneyReport);
  return (d->m_rowType == eAssetLiability);
}

MyMoneyReport::EColumnType MyMoneyReport::columnType() const
{
  Q_D(const MyMoneyReport);
  return d->m_columnType;
}

void MyMoneyReport::setColumnType(EColumnType ct)
{
  Q_D(MyMoneyReport);
  d->m_columnType = ct;
}

bool MyMoneyReport::isConvertCurrency() const
{
  Q_D(const MyMoneyReport);
  return d->m_convertCurrency;
}

void MyMoneyReport::setConvertCurrency(bool f)
{
  Q_D(MyMoneyReport);
  d->m_convertCurrency = f;
}

uint MyMoneyReport::columnPitch() const
{
  Q_D(const MyMoneyReport);
  return static_cast<uint>(d->m_columnType);
}

QString MyMoneyReport::comment() const
{
  Q_D(const MyMoneyReport);
  return d->m_comment;
}

void MyMoneyReport::setComment(const QString& comment)
{
  Q_D(MyMoneyReport);
  d->m_comment = comment;
}

MyMoneyReport::EQueryColumns MyMoneyReport::queryColumns() const
{
  Q_D(const MyMoneyReport);
  return d->m_queryColumns;
}

void MyMoneyReport::setQueryColumns(EQueryColumns qc)
{
  Q_D(MyMoneyReport);
  d->m_queryColumns = qc;
}

QString MyMoneyReport::group() const
{
  Q_D(const MyMoneyReport);
  return d->m_group;
}

void MyMoneyReport::setGroup(const QString& group)
{
  Q_D(MyMoneyReport);
  d->m_group = group;
}

bool MyMoneyReport::isFavorite() const
{
  Q_D(const MyMoneyReport);
  return d->m_favorite;
}

void MyMoneyReport::setFavorite(bool f)
{
  Q_D(MyMoneyReport);
  d->m_favorite = f;
}

bool MyMoneyReport::isTax() const
{
  Q_D(const MyMoneyReport);
  return d->m_tax;
}

void MyMoneyReport::setTax(bool f)
{
  Q_D(MyMoneyReport);
  d->m_tax = f;
}

bool MyMoneyReport::isInvestmentsOnly() const
{
  Q_D(const MyMoneyReport);
  return d->m_investments;
}

void MyMoneyReport::setInvestmentsOnly(bool f)
{
  Q_D(MyMoneyReport);
  d->m_investments = f; if (f) d->m_loans = false;
}

bool MyMoneyReport::isLoansOnly() const
{
  Q_D(const MyMoneyReport);
  return d->m_loans;
}

void MyMoneyReport::setLoansOnly(bool f)
{
  Q_D(MyMoneyReport);
  d->m_loans = f; if (f) d->m_investments = false;
}

MyMoneyReport::EDetailLevel MyMoneyReport::detailLevel() const
{
  Q_D(const MyMoneyReport);
  return d->m_detailLevel;
}

void MyMoneyReport::setDetailLevel(EDetailLevel detail)
{
  Q_D(MyMoneyReport);
  d->m_detailLevel = detail;
}

MyMoneyReport::EInvestmentSum MyMoneyReport::investmentSum() const
{
  Q_D(const MyMoneyReport);
  return d->m_investmentSum;
}

void MyMoneyReport::setInvestmentSum(EInvestmentSum sum)
{
  Q_D(MyMoneyReport);
  d->m_investmentSum = sum;
}

bool MyMoneyReport::isHideTransactions() const
{
  Q_D(const MyMoneyReport);
  return d->m_hideTransactions;
}

void MyMoneyReport::setHideTransactions(bool f)
{
  Q_D(MyMoneyReport);
  d->m_hideTransactions = f;
}

MyMoneyReport::EChartType MyMoneyReport::chartType() const
{
  Q_D(const MyMoneyReport);
  return d->m_chartType;
}

void MyMoneyReport::setChartType(EChartType type)
{
  Q_D(MyMoneyReport);
  d->m_chartType = type;
}

bool MyMoneyReport::isChartDataLabels() const
{
  Q_D(const MyMoneyReport);
  return d->m_chartDataLabels;
}

void MyMoneyReport::setChartDataLabels(bool f)
{
  Q_D(MyMoneyReport);
  d->m_chartDataLabels = f;
}

bool MyMoneyReport::isChartCHGridLines() const
{
  Q_D(const MyMoneyReport);
  return d->m_chartCHGridLines;
}

void MyMoneyReport::setChartCHGridLines(bool f)
{
  Q_D(MyMoneyReport);
  d->m_chartCHGridLines = f;
}

bool MyMoneyReport::isChartSVGridLines() const
{
  Q_D(const MyMoneyReport);
  return d->m_chartSVGridLines;
}

void MyMoneyReport::setChartSVGridLines(bool f)
{
  Q_D(MyMoneyReport);
  d->m_chartSVGridLines = f;
}

bool MyMoneyReport::isChartByDefault() const
{
  Q_D(const MyMoneyReport);
  return d->m_chartByDefault;
}

void MyMoneyReport::setChartByDefault(bool f)
{
  Q_D(MyMoneyReport);
  d->m_chartByDefault = f;
}

uint MyMoneyReport::chartLineWidth() const
{
  Q_D(const MyMoneyReport);
  return d->m_chartLineWidth;
}

void MyMoneyReport::setChartLineWidth(uint f)
{
  Q_D(MyMoneyReport);
  d->m_chartLineWidth = f;
}

bool MyMoneyReport::isLogYAxis() const
{
  Q_D(const MyMoneyReport);
  return d->m_logYaxis;
}

void MyMoneyReport::setLogYAxis(bool f)
{
  Q_D(MyMoneyReport);
  d->m_logYaxis = f;
}

QString MyMoneyReport::dataRangeStart() const
{
  Q_D(const MyMoneyReport);
  return d->m_dataRangeStart;
}

void MyMoneyReport::setDataRangeStart(const QString& f)
{
  Q_D(MyMoneyReport);
  d->m_dataRangeStart = f;
}

QString MyMoneyReport::dataRangeEnd() const
{
  Q_D(const MyMoneyReport);
  return d->m_dataRangeEnd;
}

void MyMoneyReport::setDataRangeEnd(const QString& f)
{
  Q_D(MyMoneyReport);
  d->m_dataRangeEnd = f;
}

QString MyMoneyReport::dataMajorTick() const
{
  Q_D(const MyMoneyReport);
  return d->m_dataMajorTick;
}

void MyMoneyReport::setDataMajorTick(const QString& f)
{
  Q_D(MyMoneyReport);
  d->m_dataMajorTick = f;
}

QString MyMoneyReport::dataMinorTick() const
{
  Q_D(const MyMoneyReport);
  return d->m_dataMinorTick;
}

void MyMoneyReport::setDataMinorTick(const QString& f)
{
  Q_D(MyMoneyReport);
  d->m_dataMinorTick = f;
}

uint MyMoneyReport::yLabelsPrecision() const
{
  Q_D(const MyMoneyReport);
  return d->m_yLabelsPrecision;
}

void MyMoneyReport::setYLabelsPrecision(int f)
{
  Q_D(MyMoneyReport);
  d->m_yLabelsPrecision = f;
}

bool MyMoneyReport::isIncludingSchedules() const
{
  Q_D(const MyMoneyReport);
  return d->m_includeSchedules;
}

void MyMoneyReport::setIncludingSchedules(bool f)
{
  Q_D(MyMoneyReport);
  d->m_includeSchedules = f;
}

bool MyMoneyReport::isColumnsAreDays() const
{
  Q_D(const MyMoneyReport);
  return d->m_columnsAreDays;
}

void MyMoneyReport::setColumnsAreDays(bool f)
{
  Q_D(MyMoneyReport);
  d->m_columnsAreDays = f;
}

bool MyMoneyReport::isIncludingTransfers() const
{
  Q_D(const MyMoneyReport);
  return d->m_includeTransfers;
}

void MyMoneyReport::setIncludingTransfers(bool f)
{
  Q_D(MyMoneyReport);
  d->m_includeTransfers = f;
}

bool MyMoneyReport::isIncludingUnusedAccounts() const
{
  Q_D(const MyMoneyReport);
  return d->m_includeUnusedAccounts;
}

void MyMoneyReport::setIncludingUnusedAccounts(bool f)
{
  Q_D(MyMoneyReport);
  d->m_includeUnusedAccounts = f;
}

bool MyMoneyReport::hasBudget() const
{
  Q_D(const MyMoneyReport);
  return !d->m_budgetId.isEmpty();
}

QString MyMoneyReport::budget() const
{
  Q_D(const MyMoneyReport);
  return d->m_budgetId;
}

/**
  * Sets the budget used for this report
  *
  * @param budget The ID of the budget to use, or an empty string
  * to indicate a budget is NOT included
  * @param fa Whether to display actual data alongside the budget.
  * Setting to false means the report displays ONLY the budget itself.
  * @warning For now, the budget ID is ignored.  The budget id is
  * simply checked for any non-empty string, and if so, hasBudget()
  * will return true.
  */
void MyMoneyReport::setBudget(const QString& budget, bool fa)
{
  Q_D(MyMoneyReport);
  d->m_budgetId = budget; d->m_includeBudgetActuals = fa;
}

bool MyMoneyReport::isIncludingBudgetActuals() const
{
  Q_D(const MyMoneyReport);
  return d->m_includeBudgetActuals;
}

void MyMoneyReport::setIncludingBudgetActuals(bool f)
{
  Q_D(MyMoneyReport);
  d->m_includeBudgetActuals = f;
}

bool MyMoneyReport::isIncludingForecast() const
{
  Q_D(const MyMoneyReport);
  return d->m_includeForecast;
}

void MyMoneyReport::setIncludingForecast(bool f)
{
  Q_D(MyMoneyReport);
  d->m_includeForecast = f;
}

bool MyMoneyReport::isIncludingMovingAverage() const
{
  Q_D(const MyMoneyReport);
  return d->m_includeMovingAverage;
}

void MyMoneyReport::setIncludingMovingAverage(bool f)
{
  Q_D(MyMoneyReport);
  d->m_includeMovingAverage = f;
}

int MyMoneyReport::movingAverageDays() const
{
  Q_D(const MyMoneyReport);
  return d->m_movingAverageDays;
}

void MyMoneyReport::setMovingAverageDays(int days)
{
  Q_D(MyMoneyReport);
  d->m_movingAverageDays = days;
}

bool MyMoneyReport::isIncludingPrice() const
{
  Q_D(const MyMoneyReport);
  return d->m_includePrice;
}

void MyMoneyReport::setIncludingPrice(bool f)
{
  Q_D(MyMoneyReport);
  d->m_includePrice = f;
}

bool MyMoneyReport::isIncludingAveragePrice() const
{
  Q_D(const MyMoneyReport);
  return d->m_includeAveragePrice;
}

void MyMoneyReport::setIncludingAveragePrice(bool f)
{
  Q_D(MyMoneyReport);
  d->m_includeAveragePrice = f;
}

MyMoneyReport::dataOptionE MyMoneyReport::dataFilter() const
{
  Q_D(const MyMoneyReport);
  return d->m_dataLock;
}

bool MyMoneyReport::isDataUserDefined() const
{
  Q_D(const MyMoneyReport);
  return d->m_dataLock == MyMoneyReport::userDefined;
}

void MyMoneyReport::setDataFilter(dataOptionE u)
{
  Q_D(MyMoneyReport);
  d->m_dataLock = u;
}

eMyMoney::TransactionFilter::Date MyMoneyReport::dateRange() const
{
  Q_D(const MyMoneyReport);
  return d->m_dateLock;
}

bool MyMoneyReport::isDateUserDefined() const
{
  Q_D(const MyMoneyReport);
  return d->m_dateLock == TransactionFilter::Date::UserDefined;
}

/**
  * Set the underlying date filter and LOCK that filter to the specified
  * range.  For example, if @p _u is "CurrentMonth", this report should always
  * be updated to the current month no matter when the report is run.
  *
  * This updating is not entirely automatic, you should update it yourself by
  * calling updateDateFilter.
  *
  * @param _u The date range constant (MyMoneyTransactionFilter::dateRangeE)
  *          which this report should be locked to.
  */

void MyMoneyReport::setDateFilter(TransactionFilter::Date u)
{
  Q_D(MyMoneyReport);
  d->m_dateLock = u;
  if (u != TransactionFilter::Date::UserDefined)
    MyMoneyTransactionFilter::setDateFilter(u);
}

void MyMoneyReport::setDateFilter(const QDate& db, const QDate& de)
{
  MyMoneyTransactionFilter::setDateFilter(db, de);
}

void MyMoneyReport::updateDateFilter()
{
  Q_D(MyMoneyReport);
  if (d->m_dateLock != TransactionFilter::Date::UserDefined) MyMoneyTransactionFilter::setDateFilter(d->m_dateLock);
}


bool MyMoneyReport::isMixedTime() const
{
  Q_D(const MyMoneyReport);
  return d->m_mixedTime;
}

void MyMoneyReport::setMixedTime(bool f)
{
  Q_D(MyMoneyReport);
  d->m_mixedTime = f;
}

int MyMoneyReport::currentDateColumn() const
{
  Q_D(const MyMoneyReport);
  return d->m_currentDateColumn;
}

void MyMoneyReport::setCurrentDateColumn(int f)
{
  Q_D(MyMoneyReport);
  d->m_currentDateColumn = f;
}

uint MyMoneyReport::settlementPeriod() const
{
  Q_D(const MyMoneyReport);
  return d->m_settlementPeriod;
}

void MyMoneyReport::setSettlementPeriod(uint days)
{
  Q_D(MyMoneyReport);
  d->m_settlementPeriod = days;
}

bool MyMoneyReport::isShowingSTLTCapitalGains() const
{
  Q_D(const MyMoneyReport);
  return d->m_showSTLTCapitalGains;
}

void MyMoneyReport::setShowSTLTCapitalGains(bool f)
{
  Q_D(MyMoneyReport);
  d->m_showSTLTCapitalGains = f;
}

QDate MyMoneyReport::termSeparator() const
{
  Q_D(const MyMoneyReport);
  return d->m_tseparator;
}

void MyMoneyReport::setTermSeparator(const QDate& date)
{
  Q_D(MyMoneyReport);
  d->m_tseparator = date;
}

bool MyMoneyReport::isSkippingZero() const
{
  Q_D(const MyMoneyReport);
  return d->m_skipZero;
}

void MyMoneyReport::setSkipZero(int f)
{
  Q_D(MyMoneyReport);
  d->m_skipZero = f;
}

void MyMoneyReport::clearTransactionFilter()
{
  Q_D(MyMoneyReport);
  d->m_accountGroupFilter = false;
  d->m_accountGroups.clear();

  MyMoneyTransactionFilter::clear();
}

void MyMoneyReport::assignFilter(const MyMoneyTransactionFilter& filter)
{
  MyMoneyTransactionFilter::operator=(filter);
}

void MyMoneyReport::validDateRange(QDate& db, QDate& de)
{
  db = fromDate();
  de = toDate();

  // if either begin or end date are invalid we have one of the following
  // possible date filters:
  //
  // a) begin date not set - first transaction until given end date
  // b) end date not set   - from given date until last transaction
  // c) both not set       - first transaction until last transaction
  //
  // If there is no transaction in the engine at all, we use the current
  // year as the filter criteria.

  if (!db.isValid() || !de.isValid()) {
    QList<MyMoneyTransaction> list = MyMoneyFile::instance()->transactionList(*this);
    QDate tmpBegin, tmpEnd;

    if (!list.isEmpty()) {
      qSort(list);
      // try to use the post dates
      tmpBegin = list.front().postDate();
      tmpEnd = list.back().postDate();
      // if the post dates are not valid try the entry dates
      if (!tmpBegin.isValid())
        tmpBegin = list.front().entryDate();
      if (!tmpEnd.isValid())
        tmpEnd = list.back().entryDate();
    }
    // make sure that we leave this function with valid dates no mather what
    if (!tmpBegin.isValid() || !tmpEnd.isValid() || tmpBegin > tmpEnd) {
      tmpBegin = QDate(QDate::currentDate().year(), 1, 1);   // the first date in the file
      tmpEnd = QDate(QDate::currentDate().year(), 12, 31);   // the last date in the file
    }
    if (!db.isValid())
      db = tmpBegin;
    if (!de.isValid())
      de = tmpEnd;
  }
  if (db > de)
    db = de;
}

bool MyMoneyReport::accountGroups(QList<Account>& list) const
{
  Q_D(const MyMoneyReport);
  bool result = d->m_accountGroupFilter;

  if (result) {
    QList<Account>::const_iterator it_group = d->m_accountGroups.begin();
    while (it_group != d->m_accountGroups.end()) {
      list += (*it_group);
      ++it_group;
    }
  }
  return result;
}

void MyMoneyReport::addAccountGroup(Account type)
{
  Q_D(MyMoneyReport);
  if (!d->m_accountGroups.isEmpty() && type != Account::Unknown) {
    if (d->m_accountGroups.contains(type))
      return;
  }
  d->m_accountGroupFilter = true;
  if (type != Account::Unknown)
    d->m_accountGroups.push_back(type);
}

bool MyMoneyReport::includesAccountGroup(Account type) const
{
  Q_D(const MyMoneyReport);
  bool result = (! d->m_accountGroupFilter)
                || (isIncludingTransfers() && d->m_rowType == MyMoneyReport::eExpenseIncome)
                || d->m_accountGroups.contains(type);

  return result;
}

bool MyMoneyReport::includes(const MyMoneyAccount& acc) const
{
  Q_D(const MyMoneyReport);
  auto result = false;

  if (includesAccountGroup(acc.accountGroup())) {
    switch (acc.accountGroup()) {
      case Account::Income:
      case Account::Expense:
        if (isTax())
          result = (acc.value("Tax") == "Yes") && includesCategory(acc.id());
        else
          result = includesCategory(acc.id());
        break;
      case Account::Asset:
      case Account::Liability:
        if (isLoansOnly())
          result = acc.isLoan() && includesAccount(acc.id());
        else if (isInvestmentsOnly())
          result = acc.isInvest() && includesAccount(acc.id());
        else if (isIncludingTransfers() && d->m_rowType == MyMoneyReport::eExpenseIncome)
          // If transfers are included, ONLY include this account if it is NOT
          // included in the report itself!!
          result = ! includesAccount(acc.id());
        else
          result = includesAccount(acc.id());
        break;
      case Account::Equity:
        if (isInvestmentsOnly())
          result = (isIncludingPrice() || isIncludingAveragePrice()) && acc.isInvest() && includesAccount(acc.id());
        break;
      default:
        result = includesAccount(acc.id());
    }
  }
  return result;
}

void MyMoneyReport::write(QDomElement& e, QDomDocument *doc, bool anonymous) const
{
  Q_D(const MyMoneyReport);
  // No matter what changes, be sure to have a 'type' attribute.  Only change
  // the major type if it becomes impossible to maintain compatibility with
  // older versions of the program as new features are added to the reports.
  // Feel free to change the minor type every time a change is made here.

  // write report's internals
  if (d->m_reportType == ePivotTable)
    e.setAttribute(getAttrName(Attribute::Type), "pivottable 1.15");
  else if (d->m_reportType == eQueryTable)
    e.setAttribute(getAttrName(Attribute::Type), "querytable 1.14");
  else if (d->m_reportType == eInfoTable)
    e.setAttribute(getAttrName(Attribute::Type), "infotable 1.0");

  e.setAttribute(getAttrName(Attribute::Group), d->m_group);
  e.setAttribute(getAttrName(Attribute::ID), m_id);

  // write general tab
  if (anonymous) {
    e.setAttribute(getAttrName(Attribute::Name), m_id);
    e.setAttribute(getAttrName(Attribute::Comment), QString(d->m_comment).fill('x'));
  } else {
    e.setAttribute(getAttrName(Attribute::Name), d->m_name);
    e.setAttribute(getAttrName(Attribute::Comment), d->m_comment);
  }
  e.setAttribute(getAttrName(Attribute::ConvertCurrency), d->m_convertCurrency);
  e.setAttribute(getAttrName(Attribute::Favorite), d->m_favorite);
  e.setAttribute(getAttrName(Attribute::SkipZero), d->m_skipZero);

  e.setAttribute(getAttrName(Attribute::DateLock), kDateLockText[(int)d->m_dateLock]);

  if (d->m_reportType == ePivotTable) {
    // write report's internals
    e.setAttribute(getAttrName(Attribute::IncludesActuals), d->m_includeBudgetActuals);
    e.setAttribute(getAttrName(Attribute::IncludesForecast), d->m_includeForecast);
    e.setAttribute(getAttrName(Attribute::IncludesPrice), d->m_includePrice);
    e.setAttribute(getAttrName(Attribute::IncludesAveragePrice), d->m_includeAveragePrice);
    e.setAttribute(getAttrName(Attribute::MixedTime), d->m_mixedTime);
    e.setAttribute(getAttrName(Attribute::Investments), d->m_investments); // it's setable in rows/columns tab of querytable, but here it is internal setting

    // write rows/columns tab
    if (!d->m_budgetId.isEmpty())
      e.setAttribute(getAttrName(Attribute::Budget), d->m_budgetId);

    e.setAttribute(getAttrName(Attribute::RowType), kRowTypeText[d->m_rowType]);
    e.setAttribute(getAttrName(Attribute::ShowRowTotals), d->m_showRowTotals);
    e.setAttribute(getAttrName(Attribute::ShowColumnTotals), d->m_showColumnTotals);
    e.setAttribute(getAttrName(Attribute::Detail), kDetailLevelText[d->m_detailLevel]);

    e.setAttribute(getAttrName(Attribute::IncludesMovingAverage), d->m_includeMovingAverage);
    if (d->m_includeMovingAverage)
      e.setAttribute(getAttrName(Attribute::MovingAverageDays), d->m_movingAverageDays);

    e.setAttribute(getAttrName(Attribute::IncludesSchedules), d->m_includeSchedules);
    e.setAttribute(getAttrName(Attribute::IncludesTransfers), d->m_includeTransfers);
    e.setAttribute(getAttrName(Attribute::IncludesUnused), d->m_includeUnusedAccounts);
    e.setAttribute(getAttrName(Attribute::ColumnsAreDays), d->m_columnsAreDays);

    // write chart tab
    if (d->m_chartType < 0 || d->m_chartType >= kChartTypeText.size()) {
      qDebug("m_chartType out of bounds with %d on report of type %d. Default to none.", d->m_chartType, d->m_reportType);
      e.setAttribute(getAttrName(Attribute::ChartType), kChartTypeText[eChartNone]);
    } else
      e.setAttribute(getAttrName(Attribute::ChartType), kChartTypeText[d->m_chartType]);

    e.setAttribute(getAttrName(Attribute::ChartCHGridLines), d->m_chartCHGridLines);
    e.setAttribute(getAttrName(Attribute::ChartSVGridLines), d->m_chartSVGridLines);
    e.setAttribute(getAttrName(Attribute::ChartDataLabels), d->m_chartDataLabels);
    e.setAttribute(getAttrName(Attribute::ChartByDefault), d->m_chartByDefault);
    e.setAttribute(getAttrName(Attribute::LogYAxis), d->m_logYaxis);
    e.setAttribute(getAttrName(Attribute::ChartLineWidth), d->m_chartLineWidth);
    e.setAttribute(getAttrName(Attribute::ColumnType), kColumnTypeText[d->m_columnType]);
    e.setAttribute(getAttrName(Attribute::DataLock), kDataLockText[d->m_dataLock]);
    e.setAttribute(getAttrName(Attribute::DataRangeStart), d->m_dataRangeStart);
    e.setAttribute(getAttrName(Attribute::DataRangeEnd), d->m_dataRangeEnd);
    e.setAttribute(getAttrName(Attribute::DataMajorTick), d->m_dataMajorTick);
    e.setAttribute(getAttrName(Attribute::DataMinorTick), d->m_dataMinorTick);
    e.setAttribute(getAttrName(Attribute::YLabelsPrecision), d->m_yLabelsPrecision);
  } else if (d->m_reportType == eQueryTable) {
    // write rows/columns tab
    e.setAttribute(getAttrName(Attribute::RowType), kRowTypeText[d->m_rowType]);
    QStringList columns;
    unsigned qc = d->m_queryColumns;
    unsigned it_qc = eQCbegin;
    unsigned index = 1;
    while (it_qc != eQCend) {
      if (qc & it_qc)
        columns += kQueryColumnsText[index];
      it_qc *= 2;
      index++;
    }
    e.setAttribute(getAttrName(Attribute::QueryColumns), columns.join(","));

    e.setAttribute(getAttrName(Attribute::Tax), d->m_tax);
    e.setAttribute(getAttrName(Attribute::Investments), d->m_investments);
    e.setAttribute(getAttrName(Attribute::Loans), d->m_loans);
    e.setAttribute(getAttrName(Attribute::HideTransactions), d->m_hideTransactions);
    e.setAttribute(getAttrName(Attribute::ShowColumnTotals), d->m_showColumnTotals);
    e.setAttribute(getAttrName(Attribute::Detail), kDetailLevelText[d->m_detailLevel]);

    // write performance tab
    if (d->m_queryColumns & eQCperformance || d->m_queryColumns & eQCcapitalgain)
      e.setAttribute(getAttrName(Attribute::InvestmentSum), d->m_investmentSum);

    // write capital gains tab
    if (d->m_queryColumns & eQCcapitalgain) {
      if (d->m_investmentSum == MyMoneyReport::eSumSold) {
        e.setAttribute(getAttrName(Attribute::SettlementPeriod), d->m_settlementPeriod);
        e.setAttribute(getAttrName(Attribute::ShowSTLTCapitalGains), d->m_showSTLTCapitalGains);
        e.setAttribute(getAttrName(Attribute::TermsSeparator), d->m_tseparator.toString(Qt::ISODate));
      }
    }
  } else if (d->m_reportType == eInfoTable)
    e.setAttribute(getAttrName(Attribute::ShowRowTotals), d->m_showRowTotals);

  //
  // Text Filter
  //

  QRegExp textfilter;
  if (textFilter(textfilter)) {
    QDomElement f = doc->createElement(getElName(Element::Text));
    f.setAttribute(getAttrName(Attribute::Pattern), textfilter.pattern());
    f.setAttribute(getAttrName(Attribute::CaseSensitive), (textfilter.caseSensitivity() == Qt::CaseSensitive) ? 1 : 0);
    f.setAttribute(getAttrName(Attribute::RegEx), (textfilter.patternSyntax() == QRegExp::Wildcard) ? 1 : 0);
    f.setAttribute(getAttrName(Attribute::InvertText), MyMoneyTransactionFilter::isInvertingText());
    e.appendChild(f);
  }

  //
  // Type & State Filters
  //
  QList<int> typelist;
  if (types(typelist) && ! typelist.empty()) {
    // iterate over payees, and add each one
    QList<int>::const_iterator it_type = typelist.constBegin();
    while (it_type != typelist.constEnd()) {
      QDomElement p = doc->createElement(getElName(Element::Type));
      p.setAttribute(getAttrName(Attribute::Type), kTypeText[*it_type]);
      e.appendChild(p);

      ++it_type;
    }
  }

  QList<int> statelist;
  if (states(statelist) && ! statelist.empty()) {
    // iterate over payees, and add each one
    QList<int>::const_iterator it_state = statelist.constBegin();
    while (it_state != statelist.constEnd()) {
      QDomElement p = doc->createElement(getElName(Element::State));
      p.setAttribute(getAttrName(Attribute::State), kStateText[*it_state]);
      e.appendChild(p);

      ++it_state;
    }
  }
  //
  // Number Filter
  //

  QString nrFrom, nrTo;
  if (numberFilter(nrFrom, nrTo)) {
    QDomElement f = doc->createElement(getElName(Element::Number));
    f.setAttribute(getAttrName(Attribute::From), nrFrom);
    f.setAttribute(getAttrName(Attribute::To), nrTo);
    e.appendChild(f);
  }

  //
  // Amount Filter
  //

  MyMoneyMoney from, to;
  if (amountFilter(from, to)) {    // bool getAmountFilter(MyMoneyMoney&,MyMoneyMoney&);
    QDomElement f = doc->createElement(getElName(Element::Amount));
    f.setAttribute(getAttrName(Attribute::From), from.toString());
    f.setAttribute(getAttrName(Attribute::To), to.toString());
    e.appendChild(f);
  }

  //
  // Payees Filter
  //

  QStringList payeelist;
  if (payees(payeelist)) {
    if (payeelist.empty()) {
      QDomElement p = doc->createElement(getElName(Element::Payee));
      e.appendChild(p);
    } else {
      // iterate over payees, and add each one
      QStringList::const_iterator it_payee = payeelist.constBegin();
      while (it_payee != payeelist.constEnd()) {
        QDomElement p = doc->createElement(getElName(Element::Payee));
        p.setAttribute(getAttrName(Attribute::ID), *it_payee);
        e.appendChild(p);

        ++it_payee;
      }
    }
  }

  //
  // Tags Filter
  //

  QStringList taglist;
  if (tags(taglist)) {
    if (taglist.empty()) {
      QDomElement p = doc->createElement(getElName(Element::Tag));
      e.appendChild(p);
    } else {
      // iterate over tags, and add each one
      QStringList::const_iterator it_tag = taglist.constBegin();
      while (it_tag != taglist.constEnd()) {
        QDomElement p = doc->createElement(getElName(Element::Tag));
        p.setAttribute(getAttrName(Attribute::ID), *it_tag);
        e.appendChild(p);

        ++it_tag;
      }
    }
  }

  //
  // Account Groups Filter
  //

  QList<Account> accountgrouplist;
  if (accountGroups(accountgrouplist)) {
    // iterate over accounts, and add each one
    QList<Account>::const_iterator it_group = accountgrouplist.constBegin();
    while (it_group != accountgrouplist.constEnd()) {
      QDomElement p = doc->createElement(getElName(Element::AccountGroup));
      p.setAttribute(getAttrName(Attribute::Group), kAccountTypeText[(int)*it_group]);
      e.appendChild(p);

      ++it_group;
    }
  }

  //
  // Accounts Filter
  //

  QStringList accountlist;
  if (accounts(accountlist)) {
    // iterate over accounts, and add each one
    QStringList::const_iterator it_account = accountlist.constBegin();
    while (it_account != accountlist.constEnd()) {
      QDomElement p = doc->createElement(getElName(Element::Account));
      p.setAttribute(getAttrName(Attribute::ID), *it_account);
      e.appendChild(p);

      ++it_account;
    }
  }

  //
  // Categories Filter
  //

  accountlist.clear();
  if (categories(accountlist)) {
    // iterate over accounts, and add each one
    QStringList::const_iterator it_account = accountlist.constBegin();
    while (it_account != accountlist.constEnd()) {
      QDomElement p = doc->createElement(getElName(Element::Category));
      p.setAttribute(getAttrName(Attribute::ID), *it_account);
      e.appendChild(p);

      ++it_account;
    }
  }

  //
  // Date Filter
  //

  if (d->m_dateLock == TransactionFilter::Date::UserDefined) {
    QDate dateFrom, dateTo;
    if (dateFilter(dateFrom, dateTo)) {
      QDomElement f = doc->createElement(getElName(Element::Dates));
      if (dateFrom.isValid())
        f.setAttribute(getAttrName(Attribute::From), dateFrom.toString(Qt::ISODate));
      if (dateTo.isValid())
        f.setAttribute(getAttrName(Attribute::To), dateTo.toString(Qt::ISODate));
      e.appendChild(f);
    }
  }
}

bool MyMoneyReport::read(const QDomElement& e)
{
  Q_D(MyMoneyReport);
  // The goal of this reading method is 100% backward AND 100% forward
  // compatibility.  Any report ever created with any version of KMyMoney
  // should be able to be loaded by this method (as long as it's one of the
  // report types supported in this version, of course)

  if (e.tagName().compare(nodeNames[nnReport]) != 0)
    return false;

  // read report's internals
  QString type = e.attribute(getAttrName(Attribute::Type));
  if (type.startsWith(QLatin1String("pivottable")))
    d->m_reportType = ePivotTable;
  else if (type.startsWith(QLatin1String("querytable")))
    d->m_reportType = eQueryTable;
  else if (type.startsWith(QLatin1String("infotable")))
    d->m_reportType = eInfoTable;
  else
    return false;

  d->m_group = e.attribute(getAttrName(Attribute::Group));
  m_id = e.attribute(getAttrName(Attribute::ID));

  clearTransactionFilter();

  // read date tab
  QString datelockstr = e.attribute(getAttrName(Attribute::DateLock), "userdefined");
  // Handle the pivot 1.2/query 1.1 case where the values were saved as
  // numbers
  bool ok = false;
  int i = datelockstr.toUInt(&ok);
  if (!ok) {
    i = kDateLockText.indexOf(datelockstr);
    if (i == -1)
      i = (int)TransactionFilter::Date::UserDefined;
  }
  setDateFilter(static_cast<TransactionFilter::Date>(i));

  // read general tab
  d->m_name = e.attribute(getAttrName(Attribute::Name));
  d->m_comment = e.attribute(getAttrName(Attribute::Comment), "Extremely old report");
  d->m_convertCurrency = e.attribute(getAttrName(Attribute::ConvertCurrency), "1").toUInt();
  d->m_favorite = e.attribute(getAttrName(Attribute::Favorite), "0").toUInt();
  d->m_skipZero = e.attribute(getAttrName(Attribute::SkipZero), "0").toUInt();

  if (d->m_reportType == ePivotTable) {
    // read report's internals
    d->m_includeBudgetActuals = e.attribute(getAttrName(Attribute::IncludesActuals), "0").toUInt();
    d->m_includeForecast = e.attribute(getAttrName(Attribute::IncludesForecast), "0").toUInt();
    d->m_includePrice = e.attribute(getAttrName(Attribute::IncludesPrice), "0").toUInt();
    d->m_includeAveragePrice = e.attribute(getAttrName(Attribute::IncludesAveragePrice), "0").toUInt();
    d->m_mixedTime = e.attribute(getAttrName(Attribute::MixedTime), "0").toUInt();
    d->m_investments = e.attribute(getAttrName(Attribute::Investments), "0").toUInt();

    // read rows/columns tab
    if (e.hasAttribute(getAttrName(Attribute::Budget)))
      d->m_budgetId = e.attribute(getAttrName(Attribute::Budget));

    i = kRowTypeText.indexOf(e.attribute(getAttrName(Attribute::RowType)));
    if (i != -1)
      setRowType(static_cast<ERowType>(i));
    else
      setRowType(eExpenseIncome);

    if (e.hasAttribute(getAttrName(Attribute::ShowRowTotals)))
      d->m_showRowTotals = e.attribute(getAttrName(Attribute::ShowRowTotals)).toUInt();
    else if (rowType() == eExpenseIncome) // for backward compatibility
      d->m_showRowTotals = true;
    d->m_showColumnTotals = e.attribute(getAttrName(Attribute::ShowColumnTotals), "1").toUInt();

    //check for reports with older settings which didn't have the detail attribute
    i = kDetailLevelText.indexOf(e.attribute(getAttrName(Attribute::Detail)));
    if (i != -1)
      d->m_detailLevel = static_cast<EDetailLevel>(i);
    else
      d->m_detailLevel = eDetailAll;

    d->m_includeMovingAverage = e.attribute(getAttrName(Attribute::IncludesMovingAverage), "0").toUInt();
    if (d->m_includeMovingAverage)
      d->m_movingAverageDays = e.attribute(getAttrName(Attribute::MovingAverageDays), "1").toUInt();
    d->m_includeSchedules = e.attribute(getAttrName(Attribute::IncludesSchedules), "0").toUInt();
    d->m_includeTransfers = e.attribute(getAttrName(Attribute::IncludesTransfers), "0").toUInt();
    d->m_includeUnusedAccounts = e.attribute(getAttrName(Attribute::IncludesUnused), "0").toUInt();
    d->m_columnsAreDays = e.attribute(getAttrName(Attribute::ColumnsAreDays), "0").toUInt();

    // read chart tab
    i = kChartTypeText.indexOf(e.attribute(getAttrName(Attribute::ChartType)));
    if (i != -1)
      d->m_chartType = static_cast<EChartType>(i);
    else
      d->m_chartType = eChartNone;

    d->m_chartCHGridLines = e.attribute(getAttrName(Attribute::ChartCHGridLines), "1").toUInt();
    d->m_chartSVGridLines = e.attribute(getAttrName(Attribute::ChartSVGridLines), "1").toUInt();
    d->m_chartDataLabels = e.attribute(getAttrName(Attribute::ChartDataLabels), "1").toUInt();
    d->m_chartByDefault = e.attribute(getAttrName(Attribute::ChartByDefault), "0").toUInt();
    d->m_logYaxis = e.attribute(getAttrName(Attribute::LogYAxis), "0").toUInt();
    d->m_chartLineWidth = e.attribute(getAttrName(Attribute::ChartLineWidth), QString(m_lineWidth)).toUInt();

    // read range tab
    i = kColumnTypeText.indexOf(e.attribute(getAttrName(Attribute::ColumnType)));
    if (i != -1)
      setColumnType(static_cast<EColumnType>(i));
    else
      setColumnType(eMonths);

    i = kDataLockText.indexOf(e.attribute(getAttrName(Attribute::DataLock)));
    if (i != -1)
      setDataFilter(static_cast<dataOptionE>(i));
    else
      setDataFilter(MyMoneyReport::automatic);

    d->m_dataRangeStart = e.attribute(getAttrName(Attribute::DataRangeStart), "0");
    d->m_dataRangeEnd= e.attribute(getAttrName(Attribute::DataRangeEnd), "0");
    d->m_dataMajorTick = e.attribute(getAttrName(Attribute::DataMajorTick), "0");
    d->m_dataMinorTick = e.attribute(getAttrName(Attribute::DataMinorTick), "0");
    d->m_yLabelsPrecision = e.attribute(getAttrName(Attribute::YLabelsPrecision), "2").toUInt();
  } else if (d->m_reportType == eQueryTable) {
    // read rows/columns tab
    i = kRowTypeText.indexOf(e.attribute(getAttrName(Attribute::RowType)));
    if (i != -1)
      setRowType(static_cast<ERowType>(i));
    else
      setRowType(eAccount);

    unsigned qc = 0;
    QStringList columns = e.attribute(getAttrName(Attribute::QueryColumns), "none").split(',');
    foreach (const auto column, columns) {
      i = kQueryColumnsText.indexOf(column);
      if (i > 0)
        qc |= (1 << (i - 1));
    }
    setQueryColumns(static_cast<EQueryColumns>(qc));

    d->m_tax = e.attribute(getAttrName(Attribute::Tax), "0").toUInt();
    d->m_investments = e.attribute(getAttrName(Attribute::Investments), "0").toUInt();
    d->m_loans = e.attribute(getAttrName(Attribute::Loans), "0").toUInt();
    d->m_hideTransactions = e.attribute(getAttrName(Attribute::HideTransactions), "0").toUInt();
    d->m_showColumnTotals = e.attribute(getAttrName(Attribute::ShowColumnTotals), "1").toUInt();
    d->m_detailLevel = kDetailLevelText.indexOf(e.attribute(getAttrName(Attribute::Detail), "none")) == eDetailAll ? eDetailAll : eDetailNone;

    // read performance or capital gains tab
    if (d->m_queryColumns & eQCperformance)
      d->m_investmentSum = static_cast<EInvestmentSum>(e.attribute(getAttrName(Attribute::InvestmentSum), QString().setNum(MyMoneyReport::eSumPeriod)).toInt());

    // read capital gains tab
    if (d->m_queryColumns & eQCcapitalgain) {
      d->m_investmentSum = static_cast<EInvestmentSum>(e.attribute(getAttrName(Attribute::InvestmentSum), QString().setNum(MyMoneyReport::eSumSold)).toInt());
      if (d->m_investmentSum == MyMoneyReport::eSumSold) {
        d->m_showSTLTCapitalGains = e.attribute(getAttrName(Attribute::ShowSTLTCapitalGains), "0").toUInt();
        d->m_settlementPeriod = e.attribute(getAttrName(Attribute::SettlementPeriod), "3").toUInt();
        d->m_tseparator = QDate::fromString(e.attribute(getAttrName(Attribute::TermsSeparator), QDate::currentDate().addYears(-1).toString(Qt::ISODate)),Qt::ISODate);
      }
    }
  } else if (d->m_reportType == eInfoTable) {
    if (e.hasAttribute(getAttrName(Attribute::ShowRowTotals)))
      d->m_showRowTotals = e.attribute(getAttrName(Attribute::ShowRowTotals)).toUInt();
    else
      d->m_showRowTotals = true;
  }

  QDomNode child = e.firstChild();
  while (!child.isNull() && child.isElement()) {
    QDomElement c = child.toElement();
    if (getElName(Element::Text) == c.tagName() && c.hasAttribute(getAttrName(Attribute::Pattern))) {
      setTextFilter(QRegExp(c.attribute(getAttrName(Attribute::Pattern)),
                            c.attribute(getAttrName(Attribute::CaseSensitive), "1").toUInt()
                            ? Qt::CaseSensitive : Qt::CaseInsensitive,
                            c.attribute(getAttrName(Attribute::RegEx), "1").toUInt()
                            ? QRegExp::Wildcard : QRegExp::RegExp),
                    c.attribute(getAttrName(Attribute::InvertText), "0").toUInt());
    }
    if (getElName(Element::Type) == c.tagName() && c.hasAttribute(getAttrName(Attribute::Type))) {
      i = kTypeText.indexOf(c.attribute(getAttrName(Attribute::Type)));
      if (i != -1)
        addType(i);
    }
    if (getElName(Element::State) == c.tagName() && c.hasAttribute(getAttrName(Attribute::State))) {
      i = kStateText.indexOf(c.attribute(getAttrName(Attribute::State)));
      if (i != -1)
        addState(i);
    }
    if (getElName(Element::Number) == c.tagName())
      setNumberFilter(c.attribute(getAttrName(Attribute::From)), c.attribute(getAttrName(Attribute::To)));
    if (getElName(Element::Amount) == c.tagName())
      setAmountFilter(MyMoneyMoney(c.attribute(getAttrName(Attribute::From), "0/100")), MyMoneyMoney(c.attribute(getAttrName(Attribute::To), "0/100")));
    if (getElName(Element::Dates) == c.tagName()) {
      QDate from, to;
      if (c.hasAttribute(getAttrName(Attribute::From)))
        from = QDate::fromString(c.attribute(getAttrName(Attribute::From)), Qt::ISODate);
      if (c.hasAttribute(getAttrName(Attribute::To)))
        to = QDate::fromString(c.attribute(getAttrName(Attribute::To)), Qt::ISODate);
      MyMoneyTransactionFilter::setDateFilter(from, to);
    }
    if (getElName(Element::Payee) == c.tagName())
      addPayee(c.attribute(getAttrName(Attribute::ID)));
    if (getElName(Element::Tag) == c.tagName())
      addTag(c.attribute(getAttrName(Attribute::ID)));
    if (getElName(Element::Category) == c.tagName() && c.hasAttribute(getAttrName(Attribute::ID)))
      addCategory(c.attribute(getAttrName(Attribute::ID)));
    if (getElName(Element::Account) == c.tagName() && c.hasAttribute(getAttrName(Attribute::ID)))
      addAccount(c.attribute(getAttrName(Attribute::ID)));
    if (getElName(Element::AccountGroup) == c.tagName() && c.hasAttribute(getAttrName(Attribute::Group))) {
      i = kAccountTypeText.indexOf(c.attribute(getAttrName(Attribute::Group)));
      if (i != -1)
        addAccountGroup(static_cast<Account>(i));
    }
    child = child.nextSibling();
  }
  return true;
}

void MyMoneyReport::writeXML(QDomDocument& document, QDomElement& parent) const
{
  QDomElement el = document.createElement(nodeNames[nnReport]);
  write(el, &document, false);
  parent.appendChild(el);
}

bool MyMoneyReport::hasReferenceTo(const QString& id) const
{
  QStringList list;

  // collect all ids
  accounts(list);
  categories(list);
  payees(list);
  tags(list);

  return (list.contains(id) > 0);
}

int MyMoneyReport::m_lineWidth = 2;

void MyMoneyReport::setLineWidth(int width)
{
  m_lineWidth = width;
}

QString MyMoneyReport::getElName(const Element el)
{
  static const QHash<Element, QString> elNames = {
    {Element::Payee,        QStringLiteral("PAYEE")},
    {Element::Tag,          QStringLiteral("TAG")},
    {Element::Account,      QStringLiteral("ACCOUNT")},
    {Element::Text,         QStringLiteral("TEXT")},
    {Element::Type,         QStringLiteral("TYPE")},
    {Element::State,        QStringLiteral("STATE")},
    {Element::Number,       QStringLiteral("NUMBER")},
    {Element::Amount,       QStringLiteral("AMOUNT")},
    {Element::Dates,        QStringLiteral("DATES")},
    {Element::Category,     QStringLiteral("CATEGORY")},
    {Element::AccountGroup, QStringLiteral("ACCOUNTGROUP")}
  };
  return elNames[el];
}

QString MyMoneyReport::getAttrName(const Attribute attr)
{
  static const QHash<Attribute, QString> attrNames = {
    {Attribute::ID,                     QStringLiteral("id")},
    {Attribute::Group,                  QStringLiteral("group")},
    {Attribute::Type,                   QStringLiteral("type")},
    {Attribute::Name,                   QStringLiteral("name")},
    {Attribute::Comment,                QStringLiteral("comment")},
    {Attribute::ConvertCurrency,        QStringLiteral("convertcurrency")},
    {Attribute::Favorite,               QStringLiteral("favorite")},
    {Attribute::SkipZero,               QStringLiteral("skipZero")},
    {Attribute::DateLock,               QStringLiteral("datelock")},
    {Attribute::DataLock,               QStringLiteral("datalock")},
    {Attribute::MovingAverageDays,      QStringLiteral("movingaveragedays")},
    {Attribute::IncludesActuals,        QStringLiteral("includesactuals")},
    {Attribute::IncludesForecast,       QStringLiteral("includesforecast")},
    {Attribute::IncludesPrice,          QStringLiteral("includesprice")},
    {Attribute::IncludesAveragePrice,   QStringLiteral("includesaverageprice")},
    {Attribute::IncludesMovingAverage,  QStringLiteral("includesmovingaverage")},
    {Attribute::IncludesSchedules,      QStringLiteral("includeschedules")},
    {Attribute::IncludesTransfers,      QStringLiteral("includestransfers")},
    {Attribute::IncludesUnused,         QStringLiteral("includeunused")},
    {Attribute::MixedTime,              QStringLiteral("mixedtime")},
    {Attribute::Investments,            QStringLiteral("investments")},
    {Attribute::Budget,                 QStringLiteral("budget")},
    {Attribute::ShowRowTotals,          QStringLiteral("showrowtotals")},
    {Attribute::ShowColumnTotals,       QStringLiteral("showcolumntotals")},
    {Attribute::Detail,                 QStringLiteral("detail")},
    {Attribute::ColumnsAreDays,         QStringLiteral("columnsaredays")},
    {Attribute::ChartType,              QStringLiteral("charttype")},
    {Attribute::ChartCHGridLines,       QStringLiteral("chartchgridlines")},
    {Attribute::ChartSVGridLines,       QStringLiteral("chartsvgridlines")},
    {Attribute::ChartDataLabels,        QStringLiteral("chartdatalabels")},
    {Attribute::ChartByDefault,         QStringLiteral("chartbydefault")},
    {Attribute::LogYAxis,               QStringLiteral("logYaxis")},
    {Attribute::ChartLineWidth,         QStringLiteral("chartlinewidth")},
    {Attribute::ColumnType,             QStringLiteral("columntype")},
    {Attribute::RowType,                QStringLiteral("rowtype")},
    {Attribute::DataRangeStart,         QStringLiteral("dataRangeStart")},
    {Attribute::DataRangeEnd,           QStringLiteral("dataRangeEnd")},
    {Attribute::DataMajorTick,          QStringLiteral("dataMajorTick")},
    {Attribute::DataMinorTick,          QStringLiteral("dataMinorTick")},
    {Attribute::YLabelsPrecision,       QStringLiteral("yLabelsPrecision")},
    {Attribute::QueryColumns,           QStringLiteral("querycolumns")},
    {Attribute::Tax,                    QStringLiteral("tax")},
    {Attribute::Loans,                  QStringLiteral("loans")},
    {Attribute::HideTransactions,       QStringLiteral("hidetransactions")},
    {Attribute::InvestmentSum,          QStringLiteral("investmentsum")},
    {Attribute::SettlementPeriod,       QStringLiteral("settlementperiod")},
    {Attribute::ShowSTLTCapitalGains,   QStringLiteral("showSTLTCapitalGains")},
    {Attribute::TermsSeparator,         QStringLiteral("tseparator")},
    {Attribute::Pattern,                QStringLiteral("pattern")},
    {Attribute::CaseSensitive,          QStringLiteral("casesensitive")},
    {Attribute::RegEx,                  QStringLiteral("regex")},
    {Attribute::InvertText,             QStringLiteral("inverttext")},
    {Attribute::State,                  QStringLiteral("state")},
    {Attribute::From,                   QStringLiteral("from")},
    {Attribute::To,                     QStringLiteral("to")}
  };
  return attrNames[attr];
}

QString MyMoneyReport::toString(ERowType type)
{
  switch(type) {
  case eNoRows             : return "eNoRows";
  case eAssetLiability     : return "eAssetLiability";
  case eExpenseIncome      : return "eExpenseIncome";
  case eCategory           : return "eCategory";
  case eTopCategory        : return "eTopCategory";
  case eAccount            : return "eAccount";
  case eTag                : return "eTag";
  case ePayee              : return "ePayee";
  case eMonth              : return "eMonth";
  case eWeek               : return "eWeek";
  case eTopAccount         : return "eTopAccount";
  case eAccountByTopAccount: return "eAccountByTopAccount";
  case eEquityType         : return "eEquityType";
  case eAccountType        : return "eAccountType";
  case eInstitution        : return "eInstitution";
  case eBudget             : return "eBudget";
  case eBudgetActual       : return "eBudgetActual";
  case eSchedule           : return "eSchedule";
  case eAccountInfo        : return "eAccountInfo";
  case eAccountLoanInfo    : return "eAccountLoanInfo";
  case eAccountReconcile   : return "eAccountReconcile";
  case eCashFlow           : return "eCashFlow";
  default                  : return "undefined";
  }
}

QString MyMoneyReport::toString(MyMoneyReport::EReportType type)
{
  switch(type) {
  case eNoReport:   return "eNoReport";
  case ePivotTable: return "ePivotTable";
  case eQueryTable: return "eQueryTable";
  case eInfoTable:  return "eInfoTable";
  default:          return "undefined";
  }
}
