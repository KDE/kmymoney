/***************************************************************************
                          mymoneyreport.cpp
                             -------------------
    begin                : Sun July 4 2004
    copyright            : (C) 2004-2005 by Ace Jones
    email                : acejones@users.sourceforge.net
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
#include <kmymoneyglobalsettings.h>

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>
#include <QList>
#include <QtDebug>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyfile.h"
#include "mymoneystoragenames.h"

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

MyMoneyReport::MyMoneyReport() :
    m_name("Unconfigured Pivot Table Report"),
    m_detailLevel(eDetailNone),
    m_investmentSum(eSumSold),
    m_hideTransactions(false),
    m_convertCurrency(true),
    m_favorite(false),
    m_tax(false),
    m_investments(false),
    m_loans(false),
    m_reportType(kTypeArray[eExpenseIncome]),
    m_rowType(eExpenseIncome),
    m_columnType(eMonths),
    m_columnsAreDays(false),
    m_queryColumns(eQCnone),
    m_dateLock(MyMoneyTransactionFilter::userDefined),
    m_accountGroupFilter(false),
    m_chartType(eChartLine),
    m_chartDataLabels(true),
    m_chartCHGridLines(true),
    m_chartSVGridLines(true),
    m_chartByDefault(false),
    m_logYaxis(false),
    m_dataRangeStart('0'),
    m_dataRangeEnd('0'),
    m_dataMajorTick('0'),
    m_dataMinorTick('0'),
    m_yLabelsPrecision(2),
    m_dataLock(MyMoneyReport::automatic),
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
  m_chartLineWidth = m_lineWidth;
}

MyMoneyReport::MyMoneyReport(const QString& id, const MyMoneyReport& right) :
    MyMoneyObject(id),
    m_movingAverageDays(0),
    m_currentDateColumn(0)
{
  *this = right;
  setId(id);
}

MyMoneyReport::MyMoneyReport(ERowType _rt, unsigned _ct, dateOptionE _dl, EDetailLevel _ss, const QString& _name, const QString& _comment) :
    m_name(_name),
    m_comment(_comment),
    m_detailLevel(_ss),
    m_investmentSum(_ct & eQCcapitalgain ? eSumSold : eSumPeriod),
    m_hideTransactions(false),
    m_convertCurrency(true),
    m_favorite(false),
    m_tax(false),
    m_investments(false),
    m_loans(false),
    m_reportType(kTypeArray[_rt]),
    m_rowType(_rt),
    m_columnType(eMonths),
    m_columnsAreDays(false),
    m_queryColumns(eQCnone),
    m_dateLock(_dl),
    m_accountGroupFilter(false),
    m_chartType(eChartLine),
    m_chartDataLabels(true),
    m_chartCHGridLines(true),
    m_chartSVGridLines(true),
    m_chartByDefault(false),
    m_logYaxis(false),
    m_dataRangeStart('0'),
    m_dataRangeEnd('0'),
    m_dataMajorTick('0'),
    m_dataMinorTick('0'),
    m_yLabelsPrecision(2),
    m_dataLock(MyMoneyReport::automatic),
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
  //set initial values
  m_chartLineWidth = m_lineWidth;

  //set report type
  if (m_reportType == ePivotTable)
    m_columnType = static_cast<EColumnType>(_ct);
  if (m_reportType == eQueryTable)
    m_queryColumns = static_cast<EQueryColumns>(_ct);
  setDateFilter(_dl);

  //throw exception if the type is inconsistent
  if ((_rt > static_cast<ERowType>(sizeof(kTypeArray) / sizeof(kTypeArray[0])))
      || (m_reportType == eNoReport))
    throw MYMONEYEXCEPTION("Invalid report type");

  //add the corresponding account groups
  if (_rt == MyMoneyReport::eAssetLiability) {
    addAccountGroup(MyMoneyAccount::Asset);
    addAccountGroup(MyMoneyAccount::Liability);
    m_showRowTotals = true;
  }
  if (_rt == MyMoneyReport::eAccount) {
    addAccountGroup(MyMoneyAccount::Asset);
    addAccountGroup(MyMoneyAccount::AssetLoan);
    addAccountGroup(MyMoneyAccount::Cash);
    addAccountGroup(MyMoneyAccount::Checkings);
    addAccountGroup(MyMoneyAccount::CreditCard);
    if (KMyMoneyGlobalSettings::expertMode())
      addAccountGroup(MyMoneyAccount::Equity);
    addAccountGroup(MyMoneyAccount::Expense);
    addAccountGroup(MyMoneyAccount::Income);
    addAccountGroup(MyMoneyAccount::Liability);
    addAccountGroup(MyMoneyAccount::Loan);
    addAccountGroup(MyMoneyAccount::Savings);
    addAccountGroup(MyMoneyAccount::Stock);
    m_showRowTotals = true;
  }
  if (_rt == MyMoneyReport::eExpenseIncome) {
    addAccountGroup(MyMoneyAccount::Expense);
    addAccountGroup(MyMoneyAccount::Income);
    m_showRowTotals = true;
  }
  //FIXME take this out once we have sorted out all issues regarding budget of assets and liabilities -- asoliverez@gmail.com
  if (_rt == MyMoneyReport::eBudget || _rt == MyMoneyReport::eBudgetActual) {
    addAccountGroup(MyMoneyAccount::Expense);
    addAccountGroup(MyMoneyAccount::Income);
  }
  if (_rt == MyMoneyReport::eAccountInfo) {
    addAccountGroup(MyMoneyAccount::Asset);
    addAccountGroup(MyMoneyAccount::Liability);
  }
  //cash flow reports show splits for all account groups
  if (_rt == MyMoneyReport::eCashFlow) {
    addAccountGroup(MyMoneyAccount::Expense);
    addAccountGroup(MyMoneyAccount::Income);
    addAccountGroup(MyMoneyAccount::Asset);
    addAccountGroup(MyMoneyAccount::Liability);
  }
#ifdef DEBUG_REPORTS
  QDebug out = qDebug();
  out << _name << toString(_rt) << toString(m_reportType);
  foreach(const MyMoneyAccount::accountTypeE accountType, m_accountGroups)
    out << MyMoneyAccount::accountTypeToString(accountType);
  if (m_accounts.size() > 0)
    out << m_accounts;
#endif
}

MyMoneyReport::MyMoneyReport(const QDomElement& node) :
    MyMoneyObject(node),
    m_currentDateColumn(0)
{
  // properly initialize the object before reading it
  *this = MyMoneyReport();
  if (!read(node))
    clearId();
}

void MyMoneyReport::clearTransactionFilter()
{
  m_accountGroupFilter = false;
  m_accountGroups.clear();

  MyMoneyTransactionFilter::clear();
}

void MyMoneyReport::validDateRange(QDate& _db, QDate& _de)
{
  _db = fromDate();
  _de = toDate();

  // if either begin or end date are invalid we have one of the following
  // possible date filters:
  //
  // a) begin date not set - first transaction until given end date
  // b) end date not set   - from given date until last transaction
  // c) both not set       - first transaction until last transaction
  //
  // If there is no transaction in the engine at all, we use the current
  // year as the filter criteria.

  if (!_db.isValid() || !_de.isValid()) {
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
    if (!_db.isValid())
      _db = tmpBegin;
    if (!_de.isValid())
      _de = tmpEnd;
  }
  if (_db > _de)
    _db = _de;
}

void MyMoneyReport::setRowType(ERowType _rt)
{
  m_rowType = _rt;
  m_reportType = kTypeArray[_rt];

  m_accountGroupFilter = false;
  m_accountGroups.clear();

  if (_rt == MyMoneyReport::eAssetLiability) {
    addAccountGroup(MyMoneyAccount::Asset);
    addAccountGroup(MyMoneyAccount::Liability);
  }
  if (_rt == MyMoneyReport::eExpenseIncome) {
    addAccountGroup(MyMoneyAccount::Expense);
    addAccountGroup(MyMoneyAccount::Income);
  }
}

bool MyMoneyReport::accountGroups(QList<MyMoneyAccount::accountTypeE>& list) const

{
  bool result = m_accountGroupFilter;

  if (result) {
    QList<MyMoneyAccount::accountTypeE>::const_iterator it_group = m_accountGroups.begin();
    while (it_group != m_accountGroups.end()) {
      list += (*it_group);
      ++it_group;
    }
  }
  return result;
}

void MyMoneyReport::addAccountGroup(MyMoneyAccount::accountTypeE type)
{
  if (!m_accountGroups.isEmpty() && type != MyMoneyAccount::UnknownAccountType) {
    if (m_accountGroups.contains(type))
      return;
  }
  m_accountGroupFilter = true;
  if (type != MyMoneyAccount::UnknownAccountType)
    m_accountGroups.push_back(type);
}

bool MyMoneyReport::includesAccountGroup(MyMoneyAccount::accountTypeE type) const
{
  bool result = (! m_accountGroupFilter)
                || (isIncludingTransfers() && m_rowType == MyMoneyReport::eExpenseIncome)
                || m_accountGroups.contains(type);

  return result;
}

bool MyMoneyReport::includes(const MyMoneyAccount& acc) const
{
  bool result = false;

  if (includesAccountGroup(acc.accountGroup())) {
    switch (acc.accountGroup()) {
      case MyMoneyAccount::Income:
      case MyMoneyAccount::Expense:
        if (isTax())
          result = (acc.value("Tax") == "Yes") && includesCategory(acc.id());
        else
          result = includesCategory(acc.id());
        break;
      case MyMoneyAccount::Asset:
      case MyMoneyAccount::Liability:
        if (isLoansOnly())
          result = acc.isLoan() && includesAccount(acc.id());
        else if (isInvestmentsOnly())
          result = acc.isInvest() && includesAccount(acc.id());
        else if (isIncludingTransfers() && m_rowType == MyMoneyReport::eExpenseIncome)
          // If transfers are included, ONLY include this account if it is NOT
          // included in the report itself!!
          result = ! includesAccount(acc.id());
        else
          result = includesAccount(acc.id());
        break;
      case MyMoneyAccount::Equity:
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
  // No matter what changes, be sure to have a 'type' attribute.  Only change
  // the major type if it becomes impossible to maintain compatibility with
  // older versions of the program as new features are added to the reports.
  // Feel free to change the minor type every time a change is made here.

  // write report's internals
  if (m_reportType == ePivotTable)
    e.setAttribute(getAttrName(anType), "pivottable 1.15");
  else if (m_reportType == eQueryTable)
    e.setAttribute(getAttrName(anType), "querytable 1.14");
  else if (m_reportType == eInfoTable)
    e.setAttribute(getAttrName(anType), "infotable 1.0");

  e.setAttribute(getAttrName(anGroup), m_group);
  e.setAttribute(getAttrName(anID), m_id);

  // write general tab
  if (anonymous) {
    e.setAttribute(getAttrName(anName), m_id);
    e.setAttribute(getAttrName(anComment), QString(m_comment).fill('x'));
  } else {
    e.setAttribute(getAttrName(anName), m_name);
    e.setAttribute(getAttrName(anComment), m_comment);
  }
  e.setAttribute(getAttrName(anConvertCurrency), m_convertCurrency);
  e.setAttribute(getAttrName(anFavorite), m_favorite);
  e.setAttribute(getAttrName(anSkipZero), m_skipZero);

  e.setAttribute(getAttrName(anDateLock), kDateLockText[m_dateLock]);

  if (m_reportType == ePivotTable) {
    // write report's internals
    e.setAttribute(getAttrName(anIncludesActuals), m_includeBudgetActuals);
    e.setAttribute(getAttrName(anIncludesForecast), m_includeForecast);
    e.setAttribute(getAttrName(anIncludesPrice), m_includePrice);
    e.setAttribute(getAttrName(anIncludesAveragePrice), m_includeAveragePrice);
    e.setAttribute(getAttrName(anMixedTime), m_mixedTime);
    e.setAttribute(getAttrName(anInvestments), m_investments); // it's setable in rows/columns tab of querytable, but here it is internal setting

    // write rows/columns tab
    if (!m_budgetId.isEmpty())
      e.setAttribute(getAttrName(anBudget), m_budgetId);

    e.setAttribute(getAttrName(anRowType), kRowTypeText[m_rowType]);
    e.setAttribute(getAttrName(anShowRowTotals), m_showRowTotals);
    e.setAttribute(getAttrName(anShowColumnTotals), m_showColumnTotals);
    e.setAttribute(getAttrName(anDetail), kDetailLevelText[m_detailLevel]);

    e.setAttribute(getAttrName(anIncludesMovingAverage), m_includeMovingAverage);
    if (m_includeMovingAverage)
      e.setAttribute(getAttrName(anMovingAverageDays), m_movingAverageDays);

    e.setAttribute(getAttrName(anIncludesSchedules), m_includeSchedules);
    e.setAttribute(getAttrName(anIncludesTransfers), m_includeTransfers);
    e.setAttribute(getAttrName(anIncludesUnused), m_includeUnusedAccounts);
    e.setAttribute(getAttrName(anColumnsAreDays), m_columnsAreDays);

    // write chart tab
    if (m_chartType < 0 || m_chartType >= kChartTypeText.size()) {
      qDebug("m_chartType out of bounds with %d on report of type %d. Default to none.", m_chartType, m_reportType);
      e.setAttribute(getAttrName(anChartType), kChartTypeText[eChartNone]);
    } else
      e.setAttribute(getAttrName(anChartType), kChartTypeText[m_chartType]);

    e.setAttribute(getAttrName(anChartCHGridLines), m_chartCHGridLines);
    e.setAttribute(getAttrName(anChartSVGridLines), m_chartSVGridLines);
    e.setAttribute(getAttrName(anChartDataLabels), m_chartDataLabels);
    e.setAttribute(getAttrName(anChartByDefault), m_chartByDefault);
    e.setAttribute(getAttrName(anLogYAxis), m_logYaxis);
    e.setAttribute(getAttrName(anChartLineWidth), m_chartLineWidth);
    e.setAttribute(getAttrName(anColumnType), kColumnTypeText[m_columnType]);
    e.setAttribute(getAttrName(anDataLock), kDataLockText[m_dataLock]);
    e.setAttribute(getAttrName(anDataRangeStart), m_dataRangeStart);
    e.setAttribute(getAttrName(anDataRangeEnd), m_dataRangeEnd);
    e.setAttribute(getAttrName(anDataMajorTick), m_dataMajorTick);
    e.setAttribute(getAttrName(anDataMinorTick), m_dataMinorTick);
    e.setAttribute(getAttrName(anYLabelsPrecision), m_yLabelsPrecision);
  } else if (m_reportType == eQueryTable) {
    // write rows/columns tab
    e.setAttribute(getAttrName(anRowType), kRowTypeText[m_rowType]);
    QStringList columns;
    unsigned qc = m_queryColumns;
    unsigned it_qc = eQCbegin;
    unsigned index = 1;
    while (it_qc != eQCend) {
      if (qc & it_qc)
        columns += kQueryColumnsText[index];
      it_qc *= 2;
      index++;
    }
    e.setAttribute(getAttrName(anQueryColumns), columns.join(","));

    e.setAttribute(getAttrName(anTax), m_tax);
    e.setAttribute(getAttrName(anInvestments), m_investments);
    e.setAttribute(getAttrName(anLoans), m_loans);
    e.setAttribute(getAttrName(anHideTransactions), m_hideTransactions);
    e.setAttribute(getAttrName(anShowColumnTotals), m_showColumnTotals);
    e.setAttribute(getAttrName(anDetail), kDetailLevelText[m_detailLevel]);

    // write performance tab
    if (m_queryColumns & eQCperformance || m_queryColumns & eQCcapitalgain)
      e.setAttribute(getAttrName(anInvestmentSum), m_investmentSum);

    // write capital gains tab
    if (m_queryColumns & eQCcapitalgain) {
      if (m_investmentSum == MyMoneyReport::eSumSold) {
        e.setAttribute(getAttrName(anSettlementPeriod), m_settlementPeriod);
        e.setAttribute(getAttrName(anShowSTLTCapitalGains), m_showSTLTCapitalGains);
        e.setAttribute(getAttrName(anTermsSeparator), m_tseparator.toString(Qt::ISODate));
      }
    }
  } else if (m_reportType == eInfoTable)
    e.setAttribute(getAttrName(anShowRowTotals), m_showRowTotals);

  //
  // Text Filter
  //

  QRegExp textfilter;
  if (textFilter(textfilter)) {
    QDomElement f = doc->createElement(getElName(enText));
    f.setAttribute(getAttrName(anPattern), textfilter.pattern());
    f.setAttribute(getAttrName(anCaseSensitive), (textfilter.caseSensitivity() == Qt::CaseSensitive) ? 1 : 0);
    f.setAttribute(getAttrName(anRegEx), (textfilter.patternSyntax() == QRegExp::Wildcard) ? 1 : 0);
    f.setAttribute(getAttrName(anInvertText), m_invertText);
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
      QDomElement p = doc->createElement(getElName(enType));
      p.setAttribute(getAttrName(anType), kTypeText[*it_type]);
      e.appendChild(p);

      ++it_type;
    }
  }

  QList<int> statelist;
  if (states(statelist) && ! statelist.empty()) {
    // iterate over payees, and add each one
    QList<int>::const_iterator it_state = statelist.constBegin();
    while (it_state != statelist.constEnd()) {
      QDomElement p = doc->createElement(getElName(enState));
      p.setAttribute(getAttrName(anState), kStateText[*it_state]);
      e.appendChild(p);

      ++it_state;
    }
  }
  //
  // Number Filter
  //

  QString nrFrom, nrTo;
  if (numberFilter(nrFrom, nrTo)) {
    QDomElement f = doc->createElement(getElName(enNumber));
    f.setAttribute(getAttrName(anFrom), nrFrom);
    f.setAttribute(getAttrName(anTo), nrTo);
    e.appendChild(f);
  }

  //
  // Amount Filter
  //

  MyMoneyMoney from, to;
  if (amountFilter(from, to)) {    // bool getAmountFilter(MyMoneyMoney&,MyMoneyMoney&);
    QDomElement f = doc->createElement(getElName(enAmount));
    f.setAttribute(getAttrName(anFrom), from.toString());
    f.setAttribute(getAttrName(anTo), to.toString());
    e.appendChild(f);
  }

  //
  // Payees Filter
  //

  QStringList payeelist;
  if (payees(payeelist)) {
    if (payeelist.empty()) {
      QDomElement p = doc->createElement(getElName(enPayee));
      e.appendChild(p);
    } else {
      // iterate over payees, and add each one
      QStringList::const_iterator it_payee = payeelist.constBegin();
      while (it_payee != payeelist.constEnd()) {
        QDomElement p = doc->createElement(getElName(enPayee));
        p.setAttribute(getAttrName(anID), *it_payee);
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
      QDomElement p = doc->createElement(getElName(enTag));
      e.appendChild(p);
    } else {
      // iterate over tags, and add each one
      QStringList::const_iterator it_tag = taglist.constBegin();
      while (it_tag != taglist.constEnd()) {
        QDomElement p = doc->createElement(getElName(enTag));
        p.setAttribute(getAttrName(anID), *it_tag);
        e.appendChild(p);

        ++it_tag;
      }
    }
  }

  //
  // Account Groups Filter
  //

  QList<MyMoneyAccount::accountTypeE> accountgrouplist;
  if (accountGroups(accountgrouplist)) {
    // iterate over accounts, and add each one
    QList<MyMoneyAccount::accountTypeE>::const_iterator it_group = accountgrouplist.constBegin();
    while (it_group != accountgrouplist.constEnd()) {
      QDomElement p = doc->createElement(getElName(enAccountGroup));
      p.setAttribute(getAttrName(anGroup), kAccountTypeText[*it_group]);
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
      QDomElement p = doc->createElement(getElName(enAccount));
      p.setAttribute(getAttrName(anID), *it_account);
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
      QDomElement p = doc->createElement(getElName(enCategory));
      p.setAttribute(getAttrName(anID), *it_account);
      e.appendChild(p);

      ++it_account;
    }
  }

  //
  // Date Filter
  //

  if (m_dateLock == MyMoneyTransactionFilter::userDefined) {
    QDate dateFrom, dateTo;
    if (dateFilter(dateFrom, dateTo)) {
      QDomElement f = doc->createElement(getElName(enDates));
      if (dateFrom.isValid())
        f.setAttribute(getAttrName(anFrom), dateFrom.toString(Qt::ISODate));
      if (dateTo.isValid())
        f.setAttribute(getAttrName(anTo), dateTo.toString(Qt::ISODate));
      e.appendChild(f);
    }
  }
}

bool MyMoneyReport::read(const QDomElement& e)
{
  // The goal of this reading method is 100% backward AND 100% forward
  // compatibility.  Any report ever created with any version of KMyMoney
  // should be able to be loaded by this method (as long as it's one of the
  // report types supported in this version, of course)

  if (e.tagName().compare(nodeNames[nnReport]) != 0)
    return false;

  // read report's internals
  QString type = e.attribute(getAttrName(anType));
  if (type.startsWith(QLatin1String("pivottable")))
    m_reportType = ePivotTable;
  else if (type.startsWith(QLatin1String("querytable")))
    m_reportType = eQueryTable;
  else if (type.startsWith(QLatin1String("infotable")))
    m_reportType = eInfoTable;
  else
    return false;

  m_group = e.attribute(getAttrName(anGroup));
  m_id = e.attribute(getAttrName(anID));

  clearTransactionFilter();

  // read date tab
  QString datelockstr = e.attribute(getAttrName(anDateLock), "userdefined");
  // Handle the pivot 1.2/query 1.1 case where the values were saved as
  // numbers
  bool ok = false;
  int i = datelockstr.toUInt(&ok);
  if (!ok) {
    i = kDateLockText.indexOf(datelockstr);
    if (i == -1)
      i = MyMoneyTransactionFilter::userDefined;
  }
  setDateFilter(static_cast<dateOptionE>(i));

  // read general tab
  m_name = e.attribute(getAttrName(anName));
  m_comment = e.attribute(getAttrName(anComment), "Extremely old report");
  m_convertCurrency = e.attribute(getAttrName(anConvertCurrency), "1").toUInt();
  m_favorite = e.attribute(getAttrName(anFavorite), "0").toUInt();
  m_skipZero = e.attribute(getAttrName(anSkipZero), "0").toUInt();

  if (m_reportType == ePivotTable) {
    // read report's internals
    m_includeBudgetActuals = e.attribute(getAttrName(anIncludesActuals), "0").toUInt();
    m_includeForecast = e.attribute(getAttrName(anIncludesForecast), "0").toUInt();
    m_includePrice = e.attribute(getAttrName(anIncludesPrice), "0").toUInt();
    m_includeAveragePrice = e.attribute(getAttrName(anIncludesAveragePrice), "0").toUInt();
    m_mixedTime = e.attribute(getAttrName(anMixedTime), "0").toUInt();
    m_investments = e.attribute(getAttrName(anInvestments), "0").toUInt();

    // read rows/columns tab
    if (e.hasAttribute(getAttrName(anBudget)))
      m_budgetId = e.attribute(getAttrName(anBudget));

    i = kRowTypeText.indexOf(e.attribute(getAttrName(anRowType)));
    if (i != -1)
      setRowType(static_cast<ERowType>(i));
    else
      setRowType(eExpenseIncome);

    if (e.hasAttribute(getAttrName(anShowRowTotals)))
      m_showRowTotals = e.attribute(getAttrName(anShowRowTotals)).toUInt();
    else if (rowType() == eExpenseIncome) // for backward compatibility
      m_showRowTotals = true;
    m_showColumnTotals = e.attribute(getAttrName(anShowColumnTotals), "1").toUInt();

    //check for reports with older settings which didn't have the detail attribute
    i = kDetailLevelText.indexOf(e.attribute(getAttrName(anDetail)));
    if (i != -1)
      m_detailLevel = static_cast<EDetailLevel>(i);
    else
      m_detailLevel = eDetailAll;

    m_includeMovingAverage = e.attribute(getAttrName(anIncludesMovingAverage), "0").toUInt();
    if (m_includeMovingAverage)
      m_movingAverageDays = e.attribute(getAttrName(anMovingAverageDays), "1").toUInt();
    m_includeSchedules = e.attribute(getAttrName(anIncludesSchedules), "0").toUInt();
    m_includeTransfers = e.attribute(getAttrName(anIncludesTransfers), "0").toUInt();
    m_includeUnusedAccounts = e.attribute(getAttrName(anIncludesUnused), "0").toUInt();
    m_columnsAreDays = e.attribute(getAttrName(anColumnsAreDays), "0").toUInt();

    // read chart tab
    i = kChartTypeText.indexOf(e.attribute(getAttrName(anChartType)));
    if (i != -1)
      m_chartType = static_cast<EChartType>(i);
    else
      m_chartType = eChartNone;

    m_chartCHGridLines = e.attribute(getAttrName(anChartCHGridLines), "1").toUInt();
    m_chartSVGridLines = e.attribute(getAttrName(anChartSVGridLines), "1").toUInt();
    m_chartDataLabels = e.attribute(getAttrName(anChartDataLabels), "1").toUInt();
    m_chartByDefault = e.attribute(getAttrName(anChartByDefault), "0").toUInt();
    m_logYaxis = e.attribute(getAttrName(anLogYAxis), "0").toUInt();
    m_chartLineWidth = e.attribute(getAttrName(anChartLineWidth), QString(m_lineWidth)).toUInt();

    // read range tab
    i = kColumnTypeText.indexOf(e.attribute(getAttrName(anColumnType)));
    if (i != -1)
      setColumnType(static_cast<EColumnType>(i));
    else
      setColumnType(eMonths);

    i = kDataLockText.indexOf(e.attribute(getAttrName(anDataLock)));
    if (i != -1)
      setDataFilter(static_cast<dataOptionE>(i));
    else
      setDataFilter(MyMoneyReport::automatic);

    m_dataRangeStart = e.attribute(getAttrName(anDataRangeStart), "0");
    m_dataRangeEnd = e.attribute(getAttrName(anDataRangeEnd), "0");
    m_dataMajorTick = e.attribute(getAttrName(anDataMajorTick), "0");
    m_dataMinorTick = e.attribute(getAttrName(anDataMinorTick), "0");
    m_yLabelsPrecision = e.attribute(getAttrName(anYLabelsPrecision), "2").toUInt();
  } else if (m_reportType == eQueryTable) {
    // read rows/columns tab
    i = kRowTypeText.indexOf(e.attribute(getAttrName(anRowType)));
    if (i != -1)
      setRowType(static_cast<ERowType>(i));
    else
      setRowType(eAccount);

    unsigned qc = 0;
    QStringList columns = e.attribute(getAttrName(anQueryColumns), "none").split(',');
    foreach (const auto column, columns) {
      i = kQueryColumnsText.indexOf(column);
      if (i > 0)
        qc |= (1 << (i - 1));
    }
    setQueryColumns(static_cast<EQueryColumns>(qc));

    m_tax = e.attribute(getAttrName(anTax), "0").toUInt();
    m_investments = e.attribute(getAttrName(anInvestments), "0").toUInt();
    m_loans = e.attribute(getAttrName(anLoans), "0").toUInt();
    m_hideTransactions = e.attribute(getAttrName(anHideTransactions), "0").toUInt();
    m_showColumnTotals = e.attribute(getAttrName(anShowColumnTotals), "1").toUInt();
    m_detailLevel = kDetailLevelText.indexOf(e.attribute(getAttrName(anDetail), "none")) == eDetailAll ? eDetailAll : eDetailNone;

    // read performance or capital gains tab
    if (m_queryColumns & eQCperformance)
      m_investmentSum = static_cast<EInvestmentSum>(e.attribute(getAttrName(anInvestmentSum), QString().setNum(MyMoneyReport::eSumPeriod)).toInt());

    // read capital gains tab
    if (m_queryColumns & eQCcapitalgain) {
      m_investmentSum = static_cast<EInvestmentSum>(e.attribute(getAttrName(anInvestmentSum), QString().setNum(MyMoneyReport::eSumSold)).toInt());
      if (m_investmentSum == MyMoneyReport::eSumSold) {
        m_showSTLTCapitalGains = e.attribute(getAttrName(anShowSTLTCapitalGains), "0").toUInt();
        m_settlementPeriod = e.attribute(getAttrName(anSettlementPeriod), "3").toUInt();
        m_tseparator = QDate::fromString(e.attribute(getAttrName(anTermsSeparator), QDate::currentDate().addYears(-1).toString(Qt::ISODate)),Qt::ISODate);
      }
    }
  } else if (m_reportType == eInfoTable) {
    if (e.hasAttribute(getAttrName(anShowRowTotals)))
      m_showRowTotals = e.attribute(getAttrName(anShowRowTotals)).toUInt();
    else
      m_showRowTotals = true;
  }

  QDomNode child = e.firstChild();
  while (!child.isNull() && child.isElement()) {
    QDomElement c = child.toElement();
    if (getElName(enText) == c.tagName() && c.hasAttribute(getAttrName(anPattern))) {
      setTextFilter(QRegExp(c.attribute(getAttrName(anPattern)),
                            c.attribute(getAttrName(anCaseSensitive), "1").toUInt()
                            ? Qt::CaseSensitive : Qt::CaseInsensitive,
                            c.attribute(getAttrName(anRegEx), "1").toUInt()
                            ? QRegExp::Wildcard : QRegExp::RegExp),
                    c.attribute(getAttrName(anInvertText), "0").toUInt());
    }
    if (getElName(enType) == c.tagName() && c.hasAttribute(getAttrName(anType))) {
      i = kTypeText.indexOf(c.attribute(getAttrName(anType)));
      if (i != -1)
        addType(i);
    }
    if (getElName(enState) == c.tagName() && c.hasAttribute(getAttrName(anState))) {
      i = kStateText.indexOf(c.attribute(getAttrName(anState)));
      if (i != -1)
        addState(i);
    }
    if (getElName(enNumber) == c.tagName())
      setNumberFilter(c.attribute(getAttrName(anFrom)), c.attribute(getAttrName(anTo)));
    if (getElName(enAmount) == c.tagName())
      setAmountFilter(MyMoneyMoney(c.attribute(getAttrName(anFrom), "0/100")), MyMoneyMoney(c.attribute(getAttrName(anTo), "0/100")));
    if (getElName(enDates) == c.tagName()) {
      QDate from, to;
      if (c.hasAttribute(getAttrName(anFrom)))
        from = QDate::fromString(c.attribute(getAttrName(anFrom)), Qt::ISODate);
      if (c.hasAttribute(getAttrName(anTo)))
        to = QDate::fromString(c.attribute(getAttrName(anTo)), Qt::ISODate);
      MyMoneyTransactionFilter::setDateFilter(from, to);
    }
    if (getElName(enPayee) == c.tagName())
      addPayee(c.attribute(getAttrName(anID)));
    if (getElName(enTag) == c.tagName())
      addTag(c.attribute(getAttrName(anID)));
    if (getElName(enCategory) == c.tagName() && c.hasAttribute(getAttrName(anID)))
      addCategory(c.attribute(getAttrName(anID)));
    if (getElName(enAccount) == c.tagName() && c.hasAttribute(getAttrName(anID)))
      addAccount(c.attribute(getAttrName(anID)));
    if (getElName(enAccountGroup) == c.tagName() && c.hasAttribute(getAttrName(anGroup))) {
      i = kAccountTypeText.indexOf(c.attribute(getAttrName(anGroup)));
      if (i != -1)
        addAccountGroup(static_cast<MyMoneyAccount::accountTypeE>(i));
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

const QString MyMoneyReport::getElName(const elNameE _el)
{
  static const QHash<elNameE, QString> elNames = {
    {enPayee, QStringLiteral("PAYEE")},
    {enTag, QStringLiteral("TAG")},
    {enAccount, QStringLiteral("ACCOUNT")},
    {enText, QStringLiteral("TEXT")},
    {enType, QStringLiteral("TYPE")},
    {enState, QStringLiteral("STATE")},
    {enNumber, QStringLiteral("NUMBER")},
    {enAmount, QStringLiteral("AMOUNT")},
    {enDates, QStringLiteral("DATES")},
    {enCategory, QStringLiteral("CATEGORY")},
    {enAccountGroup, QStringLiteral("ACCOUNTGROUP")}
  };
  return elNames[_el];
}

const QString MyMoneyReport::getAttrName(const attrNameE _attr)
{
  static const QHash<attrNameE, QString> attrNames = {
    {anID, QStringLiteral("id")},
    {anGroup, QStringLiteral("group")},
    {anType, QStringLiteral("type")},
    {anName, QStringLiteral("name")},
    {anComment, QStringLiteral("comment")},
    {anConvertCurrency, QStringLiteral("convertcurrency")},
    {anFavorite, QStringLiteral("favorite")},
    {anSkipZero, QStringLiteral("skipZero")},
    {anDateLock, QStringLiteral("datelock")},
    {anDataLock, QStringLiteral("datalock")},
    {anMovingAverageDays, QStringLiteral("movingaveragedays")},
    {anIncludesActuals, QStringLiteral("includesactuals")},
    {anIncludesForecast, QStringLiteral("includesforecast")},
    {anIncludesPrice, QStringLiteral("includesprice")},
    {anIncludesAveragePrice, QStringLiteral("includesaverageprice")},
    {anIncludesMovingAverage, QStringLiteral("includesmovingaverage")},
    {anIncludesSchedules, QStringLiteral("includeschedules")},
    {anIncludesTransfers, QStringLiteral("includestransfers")},
    {anIncludesUnused, QStringLiteral("includeunused")},
    {anMixedTime, QStringLiteral("mixedtime")},
    {anInvestments, QStringLiteral("investments")},
    {anBudget, QStringLiteral("budget")},
    {anShowRowTotals, QStringLiteral("showrowtotals")},
    {anShowColumnTotals, QStringLiteral("showcolumntotals")},
    {anDetail, QStringLiteral("detail")},
    {anColumnsAreDays, QStringLiteral("columnsaredays")},
    {anChartType, QStringLiteral("charttype")},
    {anChartCHGridLines, QStringLiteral("chartchgridlines")},
    {anChartSVGridLines, QStringLiteral("chartsvgridlines")},
    {anChartDataLabels, QStringLiteral("chartdatalabels")},
    {anChartByDefault, QStringLiteral("chartbydefault")},
    {anLogYAxis, QStringLiteral("logYaxis")},
    {anChartLineWidth, QStringLiteral("chartlinewidth")},
    {anColumnType, QStringLiteral("columntype")},
    {anRowType, QStringLiteral("rowtype")},
    {anDataRangeStart, QStringLiteral("dataRangeStart")},
    {anDataRangeEnd, QStringLiteral("dataRangeEnd")},
    {anDataMajorTick, QStringLiteral("dataMajorTick")},
    {anDataMinorTick, QStringLiteral("dataMinorTick")},
    {anYLabelsPrecision, QStringLiteral("yLabelsPrecision")},
    {anQueryColumns, QStringLiteral("querycolumns")},
    {anTax, QStringLiteral("tax")},
    {anLoans, QStringLiteral("loans")},
    {anHideTransactions, QStringLiteral("hidetransactions")},
    {anInvestmentSum, QStringLiteral("investmentsum")},
    {anSettlementPeriod, QStringLiteral("settlementperiod")},
    {anShowSTLTCapitalGains, QStringLiteral("showSTLTCapitalGains")},
    {anTermsSeparator, QStringLiteral("tseparator")},
    {anPattern, QStringLiteral("pattern")},
    {anCaseSensitive, QStringLiteral("casesensitive")},
    {anRegEx, QStringLiteral("regex")},
    {anInvertText, QStringLiteral("inverttext")},
    {anState, QStringLiteral("state")},
    {anFrom, QStringLiteral("from")},
    {anTo, QStringLiteral("to")}
  };
  return attrNames[_attr];
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
