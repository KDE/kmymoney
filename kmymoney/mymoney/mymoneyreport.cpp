/*
    SPDX-FileCopyrightText: 2004-2006 Ace Jones <acejones@users.sourceforge.net>
    SPDX-FileCopyrightText: 2006 Darren Gould <darren_gould@gmx.de>
    SPDX-FileCopyrightText: 2007-2010 Alvaro Soliverez <asoliverez@gmail.com>
    SPDX-FileCopyrightText: 2010-2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2018 Michael Kiefer <Michael-Kiefer@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "mymoneyreport_p.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QSet>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneymoney.h"
#include "mymoneyfile.h"
#include "mymoneyaccount.h"
#include "mymoneytransaction.h"
#include "mymoneytransactionfilter.h"
#include "mymoneyexception.h"

MyMoneyReport::MyMoneyReport() :
  MyMoneyObject(*new MyMoneyReportPrivate)
{
}

MyMoneyReport::MyMoneyReport(const QString &id) :
  MyMoneyObject(*new MyMoneyReportPrivate, id)
{
}

MyMoneyReport::MyMoneyReport(eMyMoney::Report::RowType rt,
                             unsigned ct,
                             eMyMoney::TransactionFilter::Date dl,
                             eMyMoney::Report::DetailLevel ss,
                             const QString& name,
                             const QString& comment) :
  MyMoneyObject(*new MyMoneyReportPrivate)
{
  Q_D(MyMoneyReport);
  d->m_name = name;
  d->m_comment = comment;
  d->m_detailLevel = ss;
  d->m_investmentSum = ct & eMyMoney::Report::QueryColumn::CapitalGain ? eMyMoney::Report::InvestmentSum::Sold : eMyMoney::Report::InvestmentSum::Period;
  d->m_reportType = d->rowTypeToReportType(rt);
  d->m_rowType = rt;
  d->m_dateLock = dl;

  //set report type
  if (d->m_reportType == eMyMoney::Report::ReportType::PivotTable)
    d->m_columnType = static_cast<eMyMoney::Report::ColumnType>(ct);
  if (d->m_reportType == eMyMoney::Report::ReportType::QueryTable)
    d->m_queryColumns = static_cast<eMyMoney::Report::QueryColumn>(ct);
  setDateFilter(dl);

  //throw exception if the type is inconsistent
  if (d->rowTypeToReportType(rt) == eMyMoney::Report::ReportType::Invalid ||
      d->m_reportType == eMyMoney::Report::ReportType::NoReport)
    throw MYMONEYEXCEPTION_CSTRING("Invalid report type");

  //add the corresponding account groups
  addAccountGroupsByRowType(rt);
  switch(rt) {
    case eMyMoney::Report::RowType::AssetLiability:
    case eMyMoney::Report::RowType::Account:
    case eMyMoney::Report::RowType::ExpenseIncome:
      d->m_showRowTotals = true;
      break;
    default:
      break;
  }

#ifdef DEBUG_REPORTS
  QDebug out = qDebug();
  out << _name << toString(_rt) << toString(m_reportType);
  foreach(const eMyMoney::Account::Type accountType, m_accountGroups)
    out << MyMoneyeMyMoney::Account::accountTypeToString(accountType);
  if (m_accounts.size() > 0)
    out << m_accounts;
#endif
}

MyMoneyReport::MyMoneyReport(const MyMoneyReport& other) :
  MyMoneyObject(*new MyMoneyReportPrivate(*other.d_func()), other.id()),
  MyMoneyTransactionFilter(other)
{
}

MyMoneyReport::MyMoneyReport(const QString& id, const MyMoneyReport& other) :
  MyMoneyObject(*new MyMoneyReportPrivate(*other.d_func()), id),
  MyMoneyTransactionFilter(other)
{
  Q_D(MyMoneyReport);
  d->m_movingAverageDays = 0;
  d->m_currentDateColumn = 0;
}

MyMoneyReport::~MyMoneyReport()
{
}

void MyMoneyReport::addAccountGroupsByRowType(eMyMoney::Report::RowType rt)
{
  //add the corresponding account groups
  switch(rt) {
    case eMyMoney::Report::RowType::AccountInfo:
    case eMyMoney::Report::RowType::AssetLiability:
      addAccountGroup(eMyMoney::Account::Type::Asset);
      addAccountGroup(eMyMoney::Account::Type::Liability);
      break;

    case eMyMoney::Report::RowType::Account:
      addAccountGroup(eMyMoney::Account::Type::Asset);
      addAccountGroup(eMyMoney::Account::Type::AssetLoan);
      addAccountGroup(eMyMoney::Account::Type::Cash);
      addAccountGroup(eMyMoney::Account::Type::Checkings);
      addAccountGroup(eMyMoney::Account::Type::CreditCard);
      if (m_expertMode)
        addAccountGroup(eMyMoney::Account::Type::Equity);
      addAccountGroup(eMyMoney::Account::Type::Expense);
      addAccountGroup(eMyMoney::Account::Type::Income);
      addAccountGroup(eMyMoney::Account::Type::Liability);
      addAccountGroup(eMyMoney::Account::Type::Loan);
      addAccountGroup(eMyMoney::Account::Type::Savings);
      addAccountGroup(eMyMoney::Account::Type::Stock);
      break;

    case eMyMoney::Report::RowType::ExpenseIncome:
      addAccountGroup(eMyMoney::Account::Type::Expense);
      addAccountGroup(eMyMoney::Account::Type::Income);
      break;

    //FIXME take this out once we have sorted out all issues regarding budget of assets and liabilities -- asoliverez@gmail.com
    case eMyMoney::Report::RowType::Budget:
    case eMyMoney::Report::RowType::BudgetActual:
      addAccountGroup(eMyMoney::Account::Type::Expense);
      addAccountGroup(eMyMoney::Account::Type::Income);
      break;

    //cash flow reports show splits for all account groups
    case eMyMoney::Report::RowType::CashFlow:
      addAccountGroup(eMyMoney::Account::Type::Expense);
      addAccountGroup(eMyMoney::Account::Type::Income);
      addAccountGroup(eMyMoney::Account::Type::Asset);
      addAccountGroup(eMyMoney::Account::Type::Liability);
      break;
    default:
      break;
  }
}

eMyMoney::Report::ReportType MyMoneyReport::reportType() const
{
  Q_D(const MyMoneyReport);
  return d->m_reportType;
}

void MyMoneyReport::setReportType(eMyMoney::Report::ReportType rt)
{
  Q_D(MyMoneyReport);
  d->m_reportType = rt;
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

eMyMoney::Report::RowType MyMoneyReport::rowType() const
{
  Q_D(const MyMoneyReport);
  return d->m_rowType;
}

void MyMoneyReport::setRowType(eMyMoney::Report::RowType rt)
{
  Q_D(MyMoneyReport);
  d->m_rowType = rt;
  d->m_reportType = d->rowTypeToReportType(rt);

  d->m_accountGroupFilter = false;
  d->m_accountGroups.clear();

  addAccountGroupsByRowType(rt);
}

bool MyMoneyReport::isRunningSum() const
{
  Q_D(const MyMoneyReport);
  return (d->m_rowType == eMyMoney::Report::RowType::AssetLiability);
}

eMyMoney::Report::ColumnType MyMoneyReport::columnType() const
{
  Q_D(const MyMoneyReport);
  return d->m_columnType;
}

void MyMoneyReport::setColumnType(eMyMoney::Report::ColumnType ct)
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

eMyMoney::Report::QueryColumn MyMoneyReport::queryColumns() const
{
  Q_D(const MyMoneyReport);
  return d->m_queryColumns;
}

void MyMoneyReport::setQueryColumns(eMyMoney::Report::QueryColumn qc)
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

eMyMoney::Report::DetailLevel MyMoneyReport::detailLevel() const
{
  Q_D(const MyMoneyReport);
  return d->m_detailLevel;
}

void MyMoneyReport::setDetailLevel(eMyMoney::Report::DetailLevel detail)
{
  Q_D(MyMoneyReport);
  d->m_detailLevel = detail;
}

eMyMoney::Report::InvestmentSum MyMoneyReport::investmentSum() const
{
  Q_D(const MyMoneyReport);
  return d->m_investmentSum;
}

void MyMoneyReport::setInvestmentSum(eMyMoney::Report::InvestmentSum sum)
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

eMyMoney::Report::ChartType MyMoneyReport::chartType() const
{
  Q_D(const MyMoneyReport);
  return d->m_chartType;
}

void MyMoneyReport::setChartType(eMyMoney::Report::ChartType type)
{
  Q_D(MyMoneyReport);
  d->m_chartType = type;
}

eMyMoney::Report::ChartPalette MyMoneyReport::chartPalette() const
{
  Q_D(const MyMoneyReport);
  return d->m_chartPalette;
}

void MyMoneyReport::setChartPalette(eMyMoney::Report::ChartPalette type)
{
  Q_D(MyMoneyReport);
  d->m_chartPalette = type;
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

bool MyMoneyReport::isNegExpenses() const
{
  Q_D(const MyMoneyReport);
  return d->m_negExpenses;
}

void MyMoneyReport::setNegExpenses(bool f)
{
  Q_D(MyMoneyReport);
  d->m_negExpenses = f;
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

eMyMoney::Report::DataLock MyMoneyReport::dataFilter() const
{
  Q_D(const MyMoneyReport);
  return d->m_dataLock;
}

bool MyMoneyReport::isDataUserDefined() const
{
  Q_D(const MyMoneyReport);
  return d->m_dataLock == eMyMoney::Report::DataLock::UserDefined;
}

void MyMoneyReport::setDataFilter(eMyMoney::Report::DataLock u)
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
  return d->m_dateLock == eMyMoney::TransactionFilter::Date::UserDefined;
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

void MyMoneyReport::setDateFilter(eMyMoney::TransactionFilter::Date u)
{
  Q_D(MyMoneyReport);
  d->m_dateLock = u;
  if (u != eMyMoney::TransactionFilter::Date::UserDefined)
    MyMoneyTransactionFilter::setDateFilter(u);
}

void MyMoneyReport::setDateFilter(const QDate& db, const QDate& de)
{
  MyMoneyTransactionFilter::setDateFilter(db, de);
}

void MyMoneyReport::updateDateFilter()
{
  Q_D(MyMoneyReport);
  if (d->m_dateLock != eMyMoney::TransactionFilter::Date::UserDefined) MyMoneyTransactionFilter::setDateFilter(d->m_dateLock);
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
    /// @todo port to new model code
    /// use first and last index into journal model instead
    /// of retrieving the whole list of transactions.
    QList<MyMoneyTransaction> list = MyMoneyFile::instance()->transactionList(*this);
    QDate tmpBegin, tmpEnd;

    if (!list.isEmpty()) {
      std::sort(list.begin(), list.end());
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

bool MyMoneyReport::accountGroups(QList<eMyMoney::Account::Type>& list) const
{
  Q_D(const MyMoneyReport);
  bool result = d->m_accountGroupFilter;

  if (result) {
    QList<eMyMoney::Account::Type>::const_iterator it_group = d->m_accountGroups.begin();
    while (it_group != d->m_accountGroups.end()) {
      list += (*it_group);
      ++it_group;
    }
  }
  return result;
}

void MyMoneyReport::addAccountGroup(eMyMoney::Account::Type type)
{
  Q_D(MyMoneyReport);
  if (!d->m_accountGroups.isEmpty() && type != eMyMoney::Account::Type::Unknown) {
    if (d->m_accountGroups.contains(type))
      return;
  }
  d->m_accountGroupFilter = true;
  if (type != eMyMoney::Account::Type::Unknown)
    d->m_accountGroups.push_back(type);
}

bool MyMoneyReport::includesAccountGroup(eMyMoney::Account::Type type) const
{
  Q_D(const MyMoneyReport);
  bool result = (! d->m_accountGroupFilter)
                || (isIncludingTransfers() && d->m_rowType == eMyMoney::Report::RowType::ExpenseIncome)
                || d->m_accountGroups.contains(type);

  return result;
}

bool MyMoneyReport::includes(const MyMoneyAccount& acc) const
{
  Q_D(const MyMoneyReport);
  auto result = false;

  if (includesAccountGroup(acc.accountGroup())) {
    switch (acc.accountGroup()) {
      case eMyMoney::Account::Type::Income:
      case eMyMoney::Account::Type::Expense:
        if (isTax())
          result = (acc.value("Tax") == "Yes") && includesCategory(acc.id());
        else
          result = includesCategory(acc.id());
        break;
      case eMyMoney::Account::Type::Asset:
      case eMyMoney::Account::Type::Liability:
        if (isLoansOnly())
          result = acc.isLoan() && includesAccount(acc.id());
        else if (isInvestmentsOnly())
          result = acc.isInvest() && includesAccount(acc.id());
        else if (isIncludingTransfers() && d->m_rowType == eMyMoney::Report::RowType::ExpenseIncome)
          // If transfers are included, ONLY include this account if it is NOT
          // included in the report itself!!
          result = ! includesAccount(acc.id());
        else
          result = includesAccount(acc.id());
        break;
      case eMyMoney::Account::Type::Equity:
        if (isInvestmentsOnly())
          result = (isIncludingPrice() || isIncludingAveragePrice()) && acc.isInvest() && includesAccount(acc.id());
        break;
      default:
        result = includesAccount(acc.id());
    }
  }
  return result;
}

bool MyMoneyReport::hasReferenceTo(const QString& id) const
{
  QStringList list;

  // collect all ids
  accounts(list);
  categories(list);
  payees(list);
  tags(list);

  return list.contains(id);
}

QSet<QString> MyMoneyReport::referencedObjects() const
{
  QStringList list;
  accounts(list);
  categories(list);
  payees(list);
  tags(list);
  return QSet<QString>::fromList(list);
}

int MyMoneyReport::m_lineWidth = 2;
bool MyMoneyReport::m_expertMode = false;

void MyMoneyReport::setLineWidth(int width)
{
  m_lineWidth = width;
}

int MyMoneyReport::lineWidth()
{
  return m_lineWidth;
}

void MyMoneyReport::setExpertMode(bool expertMode)
{
  m_expertMode = expertMode;
}

QString MyMoneyReport::toString(eMyMoney::Report::RowType type)
{
  switch(type) {
  case eMyMoney::Report::RowType::NoRows             : return "eMyMoney::Report::RowType::NoRows";
  case eMyMoney::Report::RowType::AssetLiability     : return "eMyMoney::Report::RowType::AssetLiability";
  case eMyMoney::Report::RowType::ExpenseIncome      : return "eMyMoney::Report::RowType::ExpenseIncome";
  case eMyMoney::Report::RowType::Category           : return "eMyMoney::Report::RowType::Category";
  case eMyMoney::Report::RowType::TopCategory        : return "eTopCategory";
  case eMyMoney::Report::RowType::Account            : return "eAccount";
  case eMyMoney::Report::RowType::Tag                : return "eTag";
  case eMyMoney::Report::RowType::Payee              : return "ePayee";
  case eMyMoney::Report::RowType::Month              : return "eMonth";
  case eMyMoney::Report::RowType::Week               : return "eWeek";
  case eMyMoney::Report::RowType::TopAccount         : return "eTopAccount";
  case eMyMoney::Report::RowType::AccountByTopAccount: return "eAccountByTopAccount";
  case eMyMoney::Report::RowType::EquityType         : return "eEquityType";
  case eMyMoney::Report::RowType::AccountType        : return "eAccountType";
  case eMyMoney::Report::RowType::Institution        : return "eInstitution";
  case eMyMoney::Report::RowType::Budget             : return "eBudget";
  case eMyMoney::Report::RowType::BudgetActual       : return "eBudgetActual";
  case eMyMoney::Report::RowType::Schedule           : return "eSchedule";
  case eMyMoney::Report::RowType::AccountInfo        : return "eAccountInfo";
  case eMyMoney::Report::RowType::AccountLoanInfo    : return "eAccountLoanInfo";
  case eMyMoney::Report::RowType::AccountReconcile   : return "eAccountReconcile";
  case eMyMoney::Report::RowType::CashFlow           : return "eCashFlow";
  default                  : return "undefined";
  }
}

QString MyMoneyReport::toString(eMyMoney::Report::ReportType type)
{
  switch(type) {
  case eMyMoney::Report::ReportType::NoReport:   return "eNoReport";
  case eMyMoney::Report::ReportType::PivotTable: return "ePivotTable";
  case eMyMoney::Report::ReportType::QueryTable: return "eQueryTable";
  case eMyMoney::Report::ReportType::InfoTable:  return "eInfoTable";
  default:          return "undefined";
  }
}
