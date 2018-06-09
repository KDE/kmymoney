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

#include "mymoneyreport_p.h"

// ----------------------------------------------------------------------------
// QT Includes

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

using namespace MyMoneyStorageNodes;

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
  if (rt == eMyMoney::Report::RowType::AssetLiability) {
    addAccountGroup(eMyMoney::Account::Type::Asset);
    addAccountGroup(eMyMoney::Account::Type::Liability);
    d->m_showRowTotals = true;
  }
  if (rt == eMyMoney::Report::RowType::Account) {
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
    d->m_showRowTotals = true;
  }
  if (rt == eMyMoney::Report::RowType::ExpenseIncome) {
    addAccountGroup(eMyMoney::Account::Type::Expense);
    addAccountGroup(eMyMoney::Account::Type::Income);
    d->m_showRowTotals = true;
  }
  //FIXME take this out once we have sorted out all issues regarding budget of assets and liabilities -- asoliverez@gmail.com
  if (rt == eMyMoney::Report::RowType::Budget || rt == eMyMoney::Report::RowType::BudgetActual) {
    addAccountGroup(eMyMoney::Account::Type::Expense);
    addAccountGroup(eMyMoney::Account::Type::Income);
  }
  if (rt == eMyMoney::Report::RowType::AccountInfo) {
    addAccountGroup(eMyMoney::Account::Type::Asset);
    addAccountGroup(eMyMoney::Account::Type::Liability);
  }
  //cash flow reports show splits for all account groups
  if (rt == eMyMoney::Report::RowType::CashFlow) {
    addAccountGroup(eMyMoney::Account::Type::Expense);
    addAccountGroup(eMyMoney::Account::Type::Income);
    addAccountGroup(eMyMoney::Account::Type::Asset);
    addAccountGroup(eMyMoney::Account::Type::Liability);
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

  if (rt == eMyMoney::Report::RowType::AssetLiability) {
    addAccountGroup(eMyMoney::Account::Type::Asset);
    addAccountGroup(eMyMoney::Account::Type::Liability);
  }
  if (rt == eMyMoney::Report::RowType::ExpenseIncome) {
    addAccountGroup(eMyMoney::Account::Type::Expense);
    addAccountGroup(eMyMoney::Account::Type::Income);
  }
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

void MyMoneyReport::write(QDomElement& e, QDomDocument *doc, bool anonymous) const
{
  Q_D(const MyMoneyReport);
  // No matter what changes, be sure to have a 'type' attribute.  Only change
  // the major type if it becomes impossible to maintain compatibility with
  // older versions of the program as new features are added to the reports.
  // Feel free to change the minor type every time a change is made here.

  // write report's internals
  if (d->m_reportType == eMyMoney::Report::ReportType::PivotTable)
    e.setAttribute(d->getAttrName(Report::Attribute::Type), "pivottable 1.15");
  else if (d->m_reportType == eMyMoney::Report::ReportType::QueryTable)
    e.setAttribute(d->getAttrName(Report::Attribute::Type), "querytable 1.14");
  else if (d->m_reportType == eMyMoney::Report::ReportType::InfoTable)
    e.setAttribute(d->getAttrName(Report::Attribute::Type), "infotable 1.0");

  e.setAttribute(d->getAttrName(Report::Attribute::Group), d->m_group);
  e.setAttribute(d->getAttrName(Report::Attribute::ID), d->m_id);

  // write general tab
  if (anonymous) {
    e.setAttribute(d->getAttrName(Report::Attribute::Name), d->m_id);
    e.setAttribute(d->getAttrName(Report::Attribute::Comment), QString(d->m_comment).fill('x'));
  } else {
    e.setAttribute(d->getAttrName(Report::Attribute::Name), d->m_name);
    e.setAttribute(d->getAttrName(Report::Attribute::Comment), d->m_comment);
  }
  e.setAttribute(d->getAttrName(Report::Attribute::ConvertCurrency), d->m_convertCurrency);
  e.setAttribute(d->getAttrName(Report::Attribute::Favorite), d->m_favorite);
  e.setAttribute(d->getAttrName(Report::Attribute::SkipZero), d->m_skipZero);

  e.setAttribute(d->getAttrName(Report::Attribute::DateLock), d->dateLockAttributeToString(static_cast<int>(d->m_dateLock)));

  if (d->m_reportType == eMyMoney::Report::ReportType::PivotTable) {
    // write report's internals
    e.setAttribute(d->getAttrName(Report::Attribute::IncludesActuals), d->m_includeBudgetActuals);
    e.setAttribute(d->getAttrName(Report::Attribute::IncludesForecast), d->m_includeForecast);
    e.setAttribute(d->getAttrName(Report::Attribute::IncludesPrice), d->m_includePrice);
    e.setAttribute(d->getAttrName(Report::Attribute::IncludesAveragePrice), d->m_includeAveragePrice);
    e.setAttribute(d->getAttrName(Report::Attribute::MixedTime), d->m_mixedTime);
    e.setAttribute(d->getAttrName(Report::Attribute::Investments), d->m_investments); // it's setable in rows/columns tab of querytable, but here it is internal setting

    // write rows/columns tab
    if (!d->m_budgetId.isEmpty())
      e.setAttribute(d->getAttrName(Report::Attribute::Budget), d->m_budgetId);

    e.setAttribute(d->getAttrName(Report::Attribute::RowType), d->reportNames(d->m_rowType));
    e.setAttribute(d->getAttrName(Report::Attribute::ShowRowTotals), d->m_showRowTotals);
    e.setAttribute(d->getAttrName(Report::Attribute::ShowColumnTotals), d->m_showColumnTotals);
    e.setAttribute(d->getAttrName(Report::Attribute::Detail), d->reportNames(d->m_detailLevel));

    e.setAttribute(d->getAttrName(Report::Attribute::IncludesMovingAverage), d->m_includeMovingAverage);
    if (d->m_includeMovingAverage)
      e.setAttribute(d->getAttrName(Report::Attribute::MovingAverageDays), d->m_movingAverageDays);

    e.setAttribute(d->getAttrName(Report::Attribute::IncludesSchedules), d->m_includeSchedules);
    e.setAttribute(d->getAttrName(Report::Attribute::IncludesTransfers), d->m_includeTransfers);
    e.setAttribute(d->getAttrName(Report::Attribute::IncludesUnused), d->m_includeUnusedAccounts);
    e.setAttribute(d->getAttrName(Report::Attribute::ColumnsAreDays), d->m_columnsAreDays);
    e.setAttribute(d->getAttrName(Report::Attribute::ChartType), d->reportNames(d->m_chartType));
    e.setAttribute(d->getAttrName(Report::Attribute::ChartCHGridLines), d->m_chartCHGridLines);
    e.setAttribute(d->getAttrName(Report::Attribute::ChartSVGridLines), d->m_chartSVGridLines);
    e.setAttribute(d->getAttrName(Report::Attribute::ChartDataLabels), d->m_chartDataLabels);
    e.setAttribute(d->getAttrName(Report::Attribute::ChartByDefault), d->m_chartByDefault);
    e.setAttribute(d->getAttrName(Report::Attribute::LogYAxis), d->m_logYaxis);
    e.setAttribute(d->getAttrName(Report::Attribute::ChartLineWidth), d->m_chartLineWidth);
    e.setAttribute(d->getAttrName(Report::Attribute::ColumnType), d->reportNames(d->m_columnType));
    e.setAttribute(d->getAttrName(Report::Attribute::DataLock), d->reportNames(d->m_dataLock));
    e.setAttribute(d->getAttrName(Report::Attribute::DataRangeStart), d->m_dataRangeStart);
    e.setAttribute(d->getAttrName(Report::Attribute::DataRangeEnd), d->m_dataRangeEnd);
    e.setAttribute(d->getAttrName(Report::Attribute::DataMajorTick), d->m_dataMajorTick);
    e.setAttribute(d->getAttrName(Report::Attribute::DataMinorTick), d->m_dataMinorTick);
    e.setAttribute(d->getAttrName(Report::Attribute::YLabelsPrecision), d->m_yLabelsPrecision);
  } else if (d->m_reportType == eMyMoney::Report::ReportType::QueryTable) {
    // write rows/columns tab
    e.setAttribute(d->getAttrName(Report::Attribute::RowType), d->reportNames(d->m_rowType));
    QStringList columns;
    unsigned qc = d->m_queryColumns;
    unsigned it_qc = eMyMoney::Report::QueryColumn::Begin;
    unsigned index = 1;
    while (it_qc != eMyMoney::Report::QueryColumn::End) {
      if (qc & it_qc)
        columns += d->reportNamesForQC(static_cast<eMyMoney::Report::QueryColumn>(it_qc));
      it_qc *= 2;
      index++;
    }
    e.setAttribute(d->getAttrName(Report::Attribute::QueryColumns), columns.join(","));

    e.setAttribute(d->getAttrName(Report::Attribute::Tax), d->m_tax);
    e.setAttribute(d->getAttrName(Report::Attribute::Investments), d->m_investments);
    e.setAttribute(d->getAttrName(Report::Attribute::Loans), d->m_loans);
    e.setAttribute(d->getAttrName(Report::Attribute::HideTransactions), d->m_hideTransactions);
    e.setAttribute(d->getAttrName(Report::Attribute::ShowColumnTotals), d->m_showColumnTotals);
    e.setAttribute(d->getAttrName(Report::Attribute::Detail), d->reportNames(d->m_detailLevel));

    // write performance tab
    if (d->m_queryColumns & eMyMoney::Report::QueryColumn::Performance || d->m_queryColumns & eMyMoney::Report::QueryColumn::CapitalGain)
      e.setAttribute(d->getAttrName(Report::Attribute::InvestmentSum), static_cast<int>(d->m_investmentSum));

    // write capital gains tab
    if (d->m_queryColumns & eMyMoney::Report::QueryColumn::CapitalGain) {
      if (d->m_investmentSum == eMyMoney::Report::InvestmentSum::Sold) {
        e.setAttribute(d->getAttrName(Report::Attribute::SettlementPeriod), d->m_settlementPeriod);
        e.setAttribute(d->getAttrName(Report::Attribute::ShowSTLTCapitalGains), d->m_showSTLTCapitalGains);
        e.setAttribute(d->getAttrName(Report::Attribute::TermsSeparator), d->m_tseparator.toString(Qt::ISODate));
      }
    }
  } else if (d->m_reportType == eMyMoney::Report::ReportType::InfoTable)
    e.setAttribute(d->getAttrName(Report::Attribute::ShowRowTotals), d->m_showRowTotals);

  //
  // Text Filter
  //

  QRegExp textfilter;
  if (textFilter(textfilter)) {
    QDomElement f = doc->createElement(d->getElName(Report::Element::Text));
    f.setAttribute(d->getAttrName(Report::Attribute::Pattern), textfilter.pattern());
    f.setAttribute(d->getAttrName(Report::Attribute::CaseSensitive), (textfilter.caseSensitivity() == Qt::CaseSensitive) ? 1 : 0);
    f.setAttribute(d->getAttrName(Report::Attribute::RegEx), (textfilter.patternSyntax() == QRegExp::Wildcard) ? 1 : 0);
    f.setAttribute(d->getAttrName(Report::Attribute::InvertText), MyMoneyTransactionFilter::isInvertingText());
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
      QDomElement p = doc->createElement(d->getElName(Report::Element::Type));
      p.setAttribute(d->getAttrName(Report::Attribute::Type), d->typeAttributeToString(*it_type));
      e.appendChild(p);

      ++it_type;
    }
  }

  QList<int> statelist;
  if (states(statelist) && ! statelist.empty()) {
    // iterate over payees, and add each one
    QList<int>::const_iterator it_state = statelist.constBegin();
    while (it_state != statelist.constEnd()) {
      QDomElement p = doc->createElement(d->getElName(Report::Element::State));
      p.setAttribute(d->getAttrName(Report::Attribute::State), d->stateAttributeToString(*it_state));
      e.appendChild(p);

      ++it_state;
    }
  }
  //
  // Number Filter
  //

  QString nrFrom, nrTo;
  if (numberFilter(nrFrom, nrTo)) {
    QDomElement f = doc->createElement(d->getElName(Report::Element::Number));
    f.setAttribute(d->getAttrName(Report::Attribute::From), nrFrom);
    f.setAttribute(d->getAttrName(Report::Attribute::To), nrTo);
    e.appendChild(f);
  }

  //
  // Amount Filter
  //

  MyMoneyMoney from, to;
  if (amountFilter(from, to)) {    // bool getAmountFilter(MyMoneyMoney&,MyMoneyMoney&);
    QDomElement f = doc->createElement(d->getElName(Report::Element::Amount));
    f.setAttribute(d->getAttrName(Report::Attribute::From), from.toString());
    f.setAttribute(d->getAttrName(Report::Attribute::To), to.toString());
    e.appendChild(f);
  }

  //
  // Payees Filter
  //

  QStringList payeelist;
  if (payees(payeelist)) {
    if (payeelist.empty()) {
      QDomElement p = doc->createElement(d->getElName(Report::Element::Payee));
      e.appendChild(p);
    } else {
      // iterate over payees, and add each one
      QStringList::const_iterator it_payee = payeelist.constBegin();
      while (it_payee != payeelist.constEnd()) {
        QDomElement p = doc->createElement(d->getElName(Report::Element::Payee));
        p.setAttribute(d->getAttrName(Report::Attribute::ID), *it_payee);
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
      QDomElement p = doc->createElement(d->getElName(Report::Element::Tag));
      e.appendChild(p);
    } else {
      // iterate over tags, and add each one
      QStringList::const_iterator it_tag = taglist.constBegin();
      while (it_tag != taglist.constEnd()) {
        QDomElement p = doc->createElement(d->getElName(Report::Element::Tag));
        p.setAttribute(d->getAttrName(Report::Attribute::ID), *it_tag);
        e.appendChild(p);

        ++it_tag;
      }
    }
  }

  //
  // Account Groups Filter
  //

  QList<eMyMoney::Account::Type> accountgrouplist;
  if (accountGroups(accountgrouplist)) {
    // iterate over accounts, and add each one
    QList<eMyMoney::Account::Type>::const_iterator it_group = accountgrouplist.constBegin();
    while (it_group != accountgrouplist.constEnd()) {
      QDomElement p = doc->createElement(d->getElName(Report::Element::AccountGroup));
      p.setAttribute(d->getAttrName(Report::Attribute::Group), d->accountTypeAttributeToString(static_cast<int>(*it_group)));
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
      QDomElement p = doc->createElement(d->getElName(Report::Element::Account));
      p.setAttribute(d->getAttrName(Report::Attribute::ID), *it_account);
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
      QDomElement p = doc->createElement(d->getElName(Report::Element::Category));
      p.setAttribute(d->getAttrName(Report::Attribute::ID), *it_account);
      e.appendChild(p);

      ++it_account;
    }
  }

  //
  // Date Filter
  //

  if (d->m_dateLock == eMyMoney::TransactionFilter::Date::UserDefined) {
    QDate dateFrom, dateTo;
    if (dateFilter(dateFrom, dateTo)) {
      QDomElement f = doc->createElement(d->getElName(Report::Element::Dates));
      if (dateFrom.isValid())
        f.setAttribute(d->getAttrName(Report::Attribute::From), dateFrom.toString(Qt::ISODate));
      if (dateTo.isValid())
        f.setAttribute(d->getAttrName(Report::Attribute::To), dateTo.toString(Qt::ISODate));
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
  QString type = e.attribute(d->getAttrName(Report::Attribute::Type));
  if (type.startsWith(QLatin1String("pivottable")))
    d->m_reportType = eMyMoney::Report::ReportType::PivotTable;
  else if (type.startsWith(QLatin1String("querytable")))
    d->m_reportType = eMyMoney::Report::ReportType::QueryTable;
  else if (type.startsWith(QLatin1String("infotable")))
    d->m_reportType = eMyMoney::Report::ReportType::InfoTable;
  else
    return false;

  d->m_group = e.attribute(d->getAttrName(Report::Attribute::Group));
  d->m_id = e.attribute(d->getAttrName(Report::Attribute::ID));

  clearTransactionFilter();

  // read date tab
  QString datelockstr = e.attribute(d->getAttrName(Report::Attribute::DateLock), "userdefined");
  // Handle the pivot 1.2/query 1.1 case where the values were saved as
  // numbers
  bool ok = false;
  int i = datelockstr.toUInt(&ok);
  if (!ok) {
    i = d->stringToDateLockAttribute(datelockstr);
    if (i == -1)
      i = (int)eMyMoney::TransactionFilter::Date::UserDefined;
  }
  setDateFilter(static_cast<eMyMoney::TransactionFilter::Date>(i));

  // read general tab
  d->m_name = e.attribute(d->getAttrName(Report::Attribute::Name));
  d->m_comment = e.attribute(d->getAttrName(Report::Attribute::Comment), "Extremely old report");
  d->m_convertCurrency = e.attribute(d->getAttrName(Report::Attribute::ConvertCurrency), "1").toUInt();
  d->m_favorite = e.attribute(d->getAttrName(Report::Attribute::Favorite), "0").toUInt();
  d->m_skipZero = e.attribute(d->getAttrName(Report::Attribute::SkipZero), "0").toUInt();

  if (d->m_reportType == eMyMoney::Report::ReportType::PivotTable) {
    // read report's internals
    d->m_includeBudgetActuals = e.attribute(d->getAttrName(Report::Attribute::IncludesActuals), "0").toUInt();
    d->m_includeForecast = e.attribute(d->getAttrName(Report::Attribute::IncludesForecast), "0").toUInt();
    d->m_includePrice = e.attribute(d->getAttrName(Report::Attribute::IncludesPrice), "0").toUInt();
    d->m_includeAveragePrice = e.attribute(d->getAttrName(Report::Attribute::IncludesAveragePrice), "0").toUInt();
    d->m_mixedTime = e.attribute(d->getAttrName(Report::Attribute::MixedTime), "0").toUInt();
    d->m_investments = e.attribute(d->getAttrName(Report::Attribute::Investments), "0").toUInt();

    // read rows/columns tab
    if (e.hasAttribute(d->getAttrName(Report::Attribute::Budget)))
      d->m_budgetId = e.attribute(d->getAttrName(Report::Attribute::Budget));

    const auto rowTypeFromXML = d->stringToRowType(e.attribute(d->getAttrName(Report::Attribute::RowType)));
    if (rowTypeFromXML != eMyMoney::Report::RowType::Invalid)
      setRowType(rowTypeFromXML);
    else
      setRowType(eMyMoney::Report::RowType::ExpenseIncome);

    if (e.hasAttribute(d->getAttrName(Report::Attribute::ShowRowTotals)))
      d->m_showRowTotals = e.attribute(d->getAttrName(Report::Attribute::ShowRowTotals)).toUInt();
    else if (rowType() == eMyMoney::Report::RowType::ExpenseIncome) // for backward compatibility
      d->m_showRowTotals = true;
    d->m_showColumnTotals = e.attribute(d->getAttrName(Report::Attribute::ShowColumnTotals), "1").toUInt();

    //check for reports with older settings which didn't have the detail attribute
    const auto detailLevelFromXML = d->stringToDetailLevel(e.attribute(d->getAttrName(Report::Attribute::Detail)));
    if (detailLevelFromXML != eMyMoney::Report::DetailLevel::End)
      d->m_detailLevel = detailLevelFromXML;
    else
      d->m_detailLevel = eMyMoney::Report::DetailLevel::All;

    d->m_includeMovingAverage = e.attribute(d->getAttrName(Report::Attribute::IncludesMovingAverage), "0").toUInt();
    if (d->m_includeMovingAverage)
      d->m_movingAverageDays = e.attribute(d->getAttrName(Report::Attribute::MovingAverageDays), "1").toUInt();
    d->m_includeSchedules = e.attribute(d->getAttrName(Report::Attribute::IncludesSchedules), "0").toUInt();
    d->m_includeTransfers = e.attribute(d->getAttrName(Report::Attribute::IncludesTransfers), "0").toUInt();
    d->m_includeUnusedAccounts = e.attribute(d->getAttrName(Report::Attribute::IncludesUnused), "0").toUInt();
    d->m_columnsAreDays = e.attribute(d->getAttrName(Report::Attribute::ColumnsAreDays), "0").toUInt();

    // read chart tab
    const auto chartTypeFromXML = d->stringToChartType(e.attribute(d->getAttrName(Report::Attribute::ChartType)));
    if (chartTypeFromXML != eMyMoney::Report::ChartType::End)
      d->m_chartType = chartTypeFromXML;
    else
      d->m_chartType = eMyMoney::Report::ChartType::None;

    d->m_chartCHGridLines = e.attribute(d->getAttrName(Report::Attribute::ChartCHGridLines), "1").toUInt();
    d->m_chartSVGridLines = e.attribute(d->getAttrName(Report::Attribute::ChartSVGridLines), "1").toUInt();
    d->m_chartDataLabels = e.attribute(d->getAttrName(Report::Attribute::ChartDataLabels), "1").toUInt();
    d->m_chartByDefault = e.attribute(d->getAttrName(Report::Attribute::ChartByDefault), "0").toUInt();
    d->m_logYaxis = e.attribute(d->getAttrName(Report::Attribute::LogYAxis), "0").toUInt();
    d->m_chartLineWidth = e.attribute(d->getAttrName(Report::Attribute::ChartLineWidth), QString(m_lineWidth)).toUInt();

    // read range tab
    const auto columnTypeFromXML = d->stringToColumnType(e.attribute(d->getAttrName(Report::Attribute::ColumnType)));
    if (columnTypeFromXML != eMyMoney::Report::ColumnType::Invalid)
      setColumnType(columnTypeFromXML);
    else
      setColumnType(eMyMoney::Report::ColumnType::Months);

    const auto dataLockFromXML = d->stringToDataLockAttribute(e.attribute(d->getAttrName(Report::Attribute::DataLock)));
    if (dataLockFromXML != eMyMoney::Report::DataLock::DataOptionCount)
      setDataFilter(dataLockFromXML);
    else
      setDataFilter(eMyMoney::Report::DataLock::Automatic);

    d->m_dataRangeStart = e.attribute(d->getAttrName(Report::Attribute::DataRangeStart), "0");
    d->m_dataRangeEnd= e.attribute(d->getAttrName(Report::Attribute::DataRangeEnd), "0");
    d->m_dataMajorTick = e.attribute(d->getAttrName(Report::Attribute::DataMajorTick), "0");
    d->m_dataMinorTick = e.attribute(d->getAttrName(Report::Attribute::DataMinorTick), "0");
    d->m_yLabelsPrecision = e.attribute(d->getAttrName(Report::Attribute::YLabelsPrecision), "2").toUInt();
  } else if (d->m_reportType == eMyMoney::Report::ReportType::QueryTable) {
    // read rows/columns tab
    const auto rowTypeFromXML = d->stringToRowType(e.attribute(d->getAttrName(Report::Attribute::RowType)));
    if (rowTypeFromXML != eMyMoney::Report::RowType::Invalid)
      setRowType(rowTypeFromXML);
    else
      setRowType(eMyMoney::Report::RowType::Account);

    unsigned qc = 0;
    QStringList columns = e.attribute(d->getAttrName(Report::Attribute::QueryColumns), "none").split(',');
    foreach (const auto column, columns) {
      const int queryColumnFromXML = d->stringToQueryColumn(column);
      i = d->stringToQueryColumn(column);
      if (queryColumnFromXML != eMyMoney::Report::QueryColumn::End)
        qc |= queryColumnFromXML;
    }
    setQueryColumns(static_cast<eMyMoney::Report::QueryColumn>(qc));

    d->m_tax = e.attribute(d->getAttrName(Report::Attribute::Tax), "0").toUInt();
    d->m_investments = e.attribute(d->getAttrName(Report::Attribute::Investments), "0").toUInt();
    d->m_loans = e.attribute(d->getAttrName(Report::Attribute::Loans), "0").toUInt();
    d->m_hideTransactions = e.attribute(d->getAttrName(Report::Attribute::HideTransactions), "0").toUInt();
    d->m_showColumnTotals = e.attribute(d->getAttrName(Report::Attribute::ShowColumnTotals), "1").toUInt();
    const auto detailLevelFromXML = d->stringToDetailLevel(e.attribute(d->getAttrName(Report::Attribute::Detail), "none"));
    if (detailLevelFromXML == eMyMoney::Report::DetailLevel::All)
      d->m_detailLevel = detailLevelFromXML;
    else
      d->m_detailLevel = eMyMoney::Report::DetailLevel::None;

    // read performance or capital gains tab
    if (d->m_queryColumns & eMyMoney::Report::QueryColumn::Performance)
      d->m_investmentSum = static_cast<eMyMoney::Report::InvestmentSum>(e.attribute(d->getAttrName(Report::Attribute::InvestmentSum), QString::number(static_cast<int>(eMyMoney::Report::InvestmentSum::Period))).toInt());

    // read capital gains tab
    if (d->m_queryColumns & eMyMoney::Report::QueryColumn::CapitalGain) {
      d->m_investmentSum = static_cast<eMyMoney::Report::InvestmentSum>(e.attribute(d->getAttrName(Report::Attribute::InvestmentSum), QString::number(static_cast<int>(eMyMoney::Report::InvestmentSum::Sold))).toInt());
      if (d->m_investmentSum == eMyMoney::Report::InvestmentSum::Sold) {
        d->m_showSTLTCapitalGains = e.attribute(d->getAttrName(Report::Attribute::ShowSTLTCapitalGains), "0").toUInt();
        d->m_settlementPeriod = e.attribute(d->getAttrName(Report::Attribute::SettlementPeriod), "3").toUInt();
        d->m_tseparator = QDate::fromString(e.attribute(d->getAttrName(Report::Attribute::TermsSeparator), QDate::currentDate().addYears(-1).toString(Qt::ISODate)),Qt::ISODate);
      }
    }
  } else if (d->m_reportType == eMyMoney::Report::ReportType::InfoTable) {
    if (e.hasAttribute(d->getAttrName(Report::Attribute::ShowRowTotals)))
      d->m_showRowTotals = e.attribute(d->getAttrName(Report::Attribute::ShowRowTotals)).toUInt();
    else
      d->m_showRowTotals = true;
  }

  QDomNode child = e.firstChild();
  while (!child.isNull() && child.isElement()) {
    QDomElement c = child.toElement();
    if (d->getElName(Report::Element::Text) == c.tagName() && c.hasAttribute(d->getAttrName(Report::Attribute::Pattern))) {
      setTextFilter(QRegExp(c.attribute(d->getAttrName(Report::Attribute::Pattern)),
                            c.attribute(d->getAttrName(Report::Attribute::CaseSensitive), "1").toUInt()
                            ? Qt::CaseSensitive : Qt::CaseInsensitive,
                            c.attribute(d->getAttrName(Report::Attribute::RegEx), "1").toUInt()
                            ? QRegExp::Wildcard : QRegExp::RegExp),
                    c.attribute(d->getAttrName(Report::Attribute::InvertText), "0").toUInt());
    }
    if (d->getElName(Report::Element::Type) == c.tagName() && c.hasAttribute(d->getAttrName(Report::Attribute::Type))) {
      i = d->stringToTypeAttribute(c.attribute(d->getAttrName(Report::Attribute::Type)));
      if (i != -1)
        addType(i);
    }
    if (d->getElName(Report::Element::State) == c.tagName() && c.hasAttribute(d->getAttrName(Report::Attribute::State))) {
      i = d->stringToStateAttribute(c.attribute(d->getAttrName(Report::Attribute::State)));
      if (i != -1)
        addState(i);
    }
    if (d->getElName(Report::Element::Number) == c.tagName())
      setNumberFilter(c.attribute(d->getAttrName(Report::Attribute::From)), c.attribute(d->getAttrName(Report::Attribute::To)));
    if (d->getElName(Report::Element::Amount) == c.tagName())
      setAmountFilter(MyMoneyMoney(c.attribute(d->getAttrName(Report::Attribute::From), "0/100")), MyMoneyMoney(c.attribute(d->getAttrName(Report::Attribute::To), "0/100")));
    if (d->getElName(Report::Element::Dates) == c.tagName()) {
      QDate from, to;
      if (c.hasAttribute(d->getAttrName(Report::Attribute::From)))
        from = QDate::fromString(c.attribute(d->getAttrName(Report::Attribute::From)), Qt::ISODate);
      if (c.hasAttribute(d->getAttrName(Report::Attribute::To)))
        to = QDate::fromString(c.attribute(d->getAttrName(Report::Attribute::To)), Qt::ISODate);
      MyMoneyTransactionFilter::setDateFilter(from, to);
    }
    if (d->getElName(Report::Element::Payee) == c.tagName())
      addPayee(c.attribute(d->getAttrName(Report::Attribute::ID)));
    if (d->getElName(Report::Element::Tag) == c.tagName())
      addTag(c.attribute(d->getAttrName(Report::Attribute::ID)));
    if (d->getElName(Report::Element::Category) == c.tagName() && c.hasAttribute(d->getAttrName(Report::Attribute::ID)))
      addCategory(c.attribute(d->getAttrName(Report::Attribute::ID)));
    if (d->getElName(Report::Element::Account) == c.tagName() && c.hasAttribute(d->getAttrName(Report::Attribute::ID)))
      addAccount(c.attribute(d->getAttrName(Report::Attribute::ID)));
    if (d->getElName(Report::Element::AccountGroup) == c.tagName() && c.hasAttribute(d->getAttrName(Report::Attribute::Group))) {
      i = d->stringToAccountTypeAttribute(c.attribute(d->getAttrName(Report::Attribute::Group)));
      if (i != -1)
        addAccountGroup(static_cast<eMyMoney::Account::Type>(i));
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

  return list.contains(id);
}

int MyMoneyReport::m_lineWidth = 2;
bool MyMoneyReport::m_expertMode = false;

void MyMoneyReport::setLineWidth(int width)
{
  m_lineWidth = width;
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
