/***************************************************************************
                          pivottable.cpp
                             -------------------
    begin                : Mon May 17 2004
    copyright            : (C) 2004-2005 by Ace Jones
    email                : <ace.j@hotpop.com>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Alvaro Soliverez <asoliverez@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// ----------------------------------------------------------------------------
// QT Includes
#include <qlayout.h>
#include <qdatetime.h>
#include <qregexp.h>
#include <q3dragobject.h>
#include <qclipboard.h>
#include <qapplication.h>
#include <qprinter.h>
#include <qpainter.h>
#include <qfile.h>
#include <qdom.h>
//Added by qt3to4:
#include <Q3TextStream>
#include <QList>

// ----------------------------------------------------------------------------
// KDE Includes
// This is just needed for i18n() and weekStartDay().
// Once I figure out how to handle i18n
// without using this macro directly, I'll be freed of KDE dependency.  This
// is a minor problem because we use these terms when rendering to HTML,
// and a more major problem because we need it to translate account types
// (e.g. MyMoneyAccount::Checkings) into their text representation.  We also
// use that text representation in the core data structure of the report. (Ace)

#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>
#include <kcalendarsystem.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "pivottable.h"
#include "pivotgrid.h"
#include "reportdebug.h"
#warning #Port to KDE4
//#include "kreportchartview.h"
#include "kmymoneyglobalsettings.h"
#include "kmymoneyutils.h"
#include "mymoneyforecast.h"


#include <kmymoneyutils.h>

namespace reports {

QString Debug::m_sTabs;
bool Debug::m_sEnabled = DEBUG_ENABLED_BY_DEFAULT;
QString Debug::m_sEnableKey;

Debug::Debug( const QString& _name ): m_methodName( _name ), m_enabled( m_sEnabled )
{
  if (!m_enabled && _name == m_sEnableKey)
    m_enabled = true;

  if (m_enabled)
  {
    qDebug( "%s%s(): ENTER", qPrintable(m_sTabs), qPrintable(m_methodName) );
    m_sTabs.append("--");
  }
}

Debug::~Debug()
{
  if ( m_enabled )
  {
    m_sTabs.remove(0,2);
    qDebug( "%s%s(): EXIT", qPrintable(m_sTabs), qPrintable(m_methodName) );

    if (m_methodName == m_sEnableKey)
      m_enabled = false;
  }
}

void Debug::output( const QString& _text )
{
  if ( m_enabled )
    qDebug( "%s%s(): %s", qPrintable(m_sTabs), qPrintable(m_methodName), qPrintable(_text) );
}

PivotTable::PivotTable( const MyMoneyReport& _config_f ):
  m_runningSumsCalculated(false),
  m_config_f( _config_f )
{
  init();
}

void PivotTable::init(void)
{
  DEBUG_ENTER(__PRETTY_FUNCTION__);

  //
  // Initialize locals
  //

  MyMoneyFile* file = MyMoneyFile::instance();

  //
  // Initialize member variables
  //

  m_config_f.validDateRange( m_beginDate, m_endDate );

  // If we need to calculate running sums, it does not make sense
  // to show a row total column
  if ( m_config_f.isRunningSum() )
    m_config_f.setShowingRowTotals(false);

  // if this is a months-based report
  if (! m_config_f.isColumnsAreDays())
  {
    // strip out the 'days' component of the begin and end dates.
    // we're only using these variables to contain year and month.
    m_beginDate =  QDate( m_beginDate.year(), m_beginDate.month(), 1 );
    m_endDate = QDate( m_endDate.year(), m_endDate.month(), 1 );
  }

  m_numColumns = columnValue(m_endDate) - columnValue(m_beginDate) + 2;

  //Load what types of row the report is going to show
  loadRowTypeList();

  //
  // Initialize outer groups of the grid
  //
  if ( m_config_f.rowType() == MyMoneyReport::eAssetLiability )
  {
    m_grid.insert(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Asset),PivotOuterGroup(m_numColumns));
    m_grid.insert(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Liability),PivotOuterGroup(m_numColumns,PivotOuterGroup::m_kDefaultSortOrder,true /* inverted */));
  }
  else
  {
    m_grid.insert(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Income),PivotOuterGroup(m_numColumns,PivotOuterGroup::m_kDefaultSortOrder-2));
    m_grid.insert(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Expense),PivotOuterGroup(m_numColumns,PivotOuterGroup::m_kDefaultSortOrder-1,true /* inverted */));
    //
    // Create rows for income/expense reports with all accounts included
    //
    if(m_config_f.isIncludingUnusedAccounts())
      createAccountRows();
  }

  //
  // Initialize grid totals
  //

  m_grid.m_total = PivotGridRowSet(m_numColumns);

  //
  // Get opening balances
  // (for running sum reports only)
  //

  if ( m_config_f.isRunningSum() )
    calculateOpeningBalances();

  //
  // Calculate budget mapping
  // (for budget-vs-actual reports only)
  //
  if ( m_config_f.hasBudget())
    calculateBudgetMapping();

  //
  // Populate all transactions into the row/column pivot grid
  //

  QList<MyMoneyTransaction> transactions;
  m_config_f.setReportAllSplits(false);
  m_config_f.setConsiderCategory(true);
  try {
    transactions = file->transactionList(m_config_f);
  } catch(MyMoneyException *e) {
    qDebug("ERR: %s thrown in %s(%ld)", qPrintable(e->what()), qPrintable(e->file()), e->line());
    throw e;
  }
  DEBUG_OUTPUT(QString("Found %1 matching transactions").arg(transactions.count()));


  // Include scheduled transactions if required
  if ( m_config_f.isIncludingSchedules() )
  {
    // Create a custom version of the report filter, excluding date
    // We'll use this to compare the transaction against
    MyMoneyTransactionFilter schedulefilter(m_config_f);
    schedulefilter.setDateFilter(QDate(),QDate());

    // Get the real dates from the config filter
    QDate configbegin, configend;
    m_config_f.validDateRange(configbegin, configend);

    QList<MyMoneySchedule> schedules = file->scheduleList();
    QList<MyMoneySchedule>::const_iterator it_schedule = schedules.begin();
    while ( it_schedule != schedules.end() )
    {
      // If the transaction meets the filter
      MyMoneyTransaction tx = (*it_schedule).transaction();
      if (!(*it_schedule).isFinished() && schedulefilter.match(tx) )
      {
        // Keep the id of the schedule with the transaction so that
        // we can do the autocalc later on in case of a loan payment
        tx.setValue("kmm-schedule-id", (*it_schedule).id());

        // Get the dates when a payment will be made within the report window
        QDate nextpayment = (*it_schedule).nextPayment(configbegin);
        if ( nextpayment.isValid() )
        {
          // Add one transaction for each date
          QList<QDate> paymentDates = (*it_schedule).paymentDates(nextpayment,configend);
          QList<QDate>::const_iterator it_date = paymentDates.begin();
          while ( it_date != paymentDates.end() )
          {
            //if the payment occurs in the past, enter it tomorrow
            if(QDate::currentDate() >= *it_date) {
              tx.setPostDate(QDate::currentDate().addDays(1));
            } else {
              tx.setPostDate(*it_date);
            }
            if ( tx.postDate() <= configend
               && tx.postDate() >= configbegin ) {
              transactions += tx;
            }

            DEBUG_OUTPUT(QString("Added transaction for schedule %1 on %2").arg((*it_schedule).id()).arg((*it_date).toString()));

            ++it_date;
          }
        }
      }

      ++it_schedule;
    }
  }

  // whether asset & liability transactions are actually to be considered
  // transfers
  bool al_transfers = ( m_config_f.rowType() == MyMoneyReport::eExpenseIncome ) && ( m_config_f.isIncludingTransfers() );

  //this is to store balance for loan accounts when not included in the report
  QMap<QString, MyMoneyMoney> loanBalances;

  QList<MyMoneyTransaction>::const_iterator it_transaction = transactions.begin();
  unsigned colofs = columnValue(m_beginDate) - 1;
  while ( it_transaction != transactions.end() )
  {
    QDate postdate = (*it_transaction).postDate();
    unsigned column = columnValue(postdate) - colofs;

    MyMoneyTransaction tx = (*it_transaction);

    // check if we need to call the autocalculation routine
    if(tx.isLoanPayment() && tx.hasAutoCalcSplit() && (tx.value("kmm-schedule-id").length() > 0)) {
      // make sure to consider any autocalculation for loan payments
      MyMoneySchedule sched = file->schedule(tx.value("kmm-schedule-id"));
      const MyMoneySplit& split = tx.amortizationSplit();
      if(!split.id().isEmpty()) {
        ReportAccount splitAccount = file->account(split.accountId());
        MyMoneyAccount::accountTypeE type = splitAccount.accountGroup();
        QString outergroup = KMyMoneyUtils::accountTypeToString(type);

        //if the account is included in the report, calculate the balance from the cells
        if(m_config_f.includes( splitAccount )) {
          loanBalances[splitAccount.id()] = cellBalance(outergroup, splitAccount, column, false);
        } else {
          //if it is not in the report and also not in loanBalances, get the balance from the file
          if(!loanBalances.contains(splitAccount.id())) {
            QDate dueDate = sched.nextDueDate();

            //if the payment is overdue, use current date
            if(dueDate < QDate::currentDate())
              dueDate = QDate::currentDate();

            //get the balance from the file for the date
            loanBalances[splitAccount.id()] = file->balance(splitAccount.id(), dueDate.addDays(-1));
          }
        }

        KMyMoneyUtils::calculateAutoLoan(sched, tx, loanBalances);

        //if the loan split is not included in the report, update the balance for the next occurrence
        if(!m_config_f.includes( splitAccount )) {
          QList<MyMoneySplit>::ConstIterator it_loanSplits;
          for(it_loanSplits = tx.splits().begin(); it_loanSplits != tx.splits().end(); ++it_loanSplits) {
            if((*it_loanSplits).isAmortizationSplit() && (*it_loanSplits).accountId() == splitAccount.id() )
              loanBalances[splitAccount.id()] = loanBalances[splitAccount.id()] + (*it_loanSplits).shares();
          }
        }
      }
    }

    QList<MyMoneySplit> splits = tx.splits();
    QList<MyMoneySplit>::const_iterator it_split = splits.begin();
    while ( it_split != splits.end() )
    {
      ReportAccount splitAccount = (*it_split).accountId();

      // Each split must be further filtered, because if even one split matches,
      // the ENTIRE transaction is returned with all splits (even non-matching ones)
      if ( m_config_f.includes( splitAccount ) && m_config_f.match(&(*it_split)))
      {
        // reverse sign to match common notation for cash flow direction, only for expense/income splits
        MyMoneyMoney reverse(splitAccount.isIncomeExpense() ? -1 : 1, 1);

        MyMoneyMoney value;
        // the outer group is the account class (major account type)
        MyMoneyAccount::accountTypeE type = splitAccount.accountGroup();
        QString outergroup = KMyMoneyUtils::accountTypeToString(type);

        value = (*it_split).shares();
        bool stockSplit = tx.isStockSplit();
        if(!stockSplit) {
          // retrieve the value in the account's underlying currency
          if(value != MyMoneyMoney::autoCalc) {
            value = value * reverse;
          } else {
            qDebug("PivotTable::PivotTable(): This must not happen");
            value = MyMoneyMoney();  // keep it 0 so far
          }

          // Except in the case of transfers on an income/expense report
          if ( al_transfers && ( type == MyMoneyAccount::Asset || type == MyMoneyAccount::Liability ) )
          {
            outergroup = i18n("Transfers");
            value = -value;
          }
        }
        // add the value to its correct position in the pivot table
        assignCell( outergroup, splitAccount, column, value, false, stockSplit );
      }
      ++it_split;
    }

    ++it_transaction;
  }

  //
  // Get forecast data
  //
  if(m_config_f.isIncludingForecast())
    calculateForecast();

  //
  //Insert Price data
  //
  if(m_config_f.isIncludingPrice())
    fillBasePriceUnit(ePrice);

  //
  //Insert Average Price data
  //
  if(m_config_f.isIncludingAveragePrice()) {
    fillBasePriceUnit(eActual);
    calculateMovingAverage();
  }

  //
  // Collapse columns to match column type
  //


  if ( m_config_f.columnPitch() > 1 )
    collapseColumns();

  //
  // Calculate the running sums
  // (for running sum reports only)
  //

  if ( m_config_f.isRunningSum() )
    calculateRunningSums();

  //
  // Calculate Moving Average
  //
  if ( m_config_f.isIncludingMovingAverage() )
    calculateMovingAverage();

  //
  // Calculate Budget Difference
  //

  if ( m_config_f.isIncludingBudgetActuals() )
    calculateBudgetDiff();

  //
  // Convert all values to the deep currency
  //

  convertToDeepCurrency();

  //
  // Convert all values to the base currency
  //

  if ( m_config_f.isConvertCurrency() )
    convertToBaseCurrency();

  //
  // Determine column headings
  //

  calculateColumnHeadings();

  //
  // Calculate row and column totals
  //

  calculateTotals();
}

void PivotTable::collapseColumns(void)
{
  DEBUG_ENTER(__PRETTY_FUNCTION__);

  unsigned columnpitch = m_config_f.columnPitch();
  if ( columnpitch != 1 )
  {
    unsigned sourcemonth = (m_config_f.isColumnsAreDays())
      // use the user's locale to determine the week's start
      ? (m_beginDate.dayOfWeek() + 8 - KGlobal::locale()->weekStartDay()) % 7
      : m_beginDate.month();
    unsigned sourcecolumn = 1;
    unsigned destcolumn = 1;
    while ( sourcecolumn < m_numColumns )
    {
      if ( sourcecolumn != destcolumn )
      {
#if 0
        // TODO: Clean up this rather inefficient kludge. We really should jump by an entire
        // destcolumn at a time on RS reports, and calculate the proper sourcecolumn to use,
        // allowing us to clear and accumulate only ONCE per destcolumn
        if ( m_config_f.isRunningSum() )
          clearColumn(destcolumn);
#endif
        accumulateColumn(destcolumn,sourcecolumn);
      }

      if (++sourcecolumn < m_numColumns) {
        if ((sourcemonth++ % columnpitch) == 0) {
          if (sourcecolumn != ++destcolumn)
            clearColumn (destcolumn);
        }
      }
    }
    m_numColumns = destcolumn + 1;
  }
}

void PivotTable::accumulateColumn(unsigned destcolumn, unsigned sourcecolumn)
{
  DEBUG_ENTER(__PRETTY_FUNCTION__);
  DEBUG_OUTPUT(QString("From Column %1 to %2").arg(sourcecolumn).arg(destcolumn));

  // iterate over outer groups
  PivotGrid::iterator it_outergroup = m_grid.begin();
  while ( it_outergroup != m_grid.end() )
  {
    // iterate over inner groups
    PivotOuterGroup::iterator it_innergroup = (*it_outergroup).begin();
    while ( it_innergroup != (*it_outergroup).end() )
    {
      // iterator over rows
      PivotInnerGroup::iterator it_row = (*it_innergroup).begin();
      while ( it_row != (*it_innergroup).end() )
      {
        if ( (*it_row)[eActual].count() <= sourcecolumn )
          throw new MYMONEYEXCEPTION(QString("Sourcecolumn %1 out of grid range (%2) in PivotTable::accumulateColumn").arg(sourcecolumn).arg((*it_row)[eActual].count()));
        if ( (*it_row)[eActual].count() <= destcolumn )
          throw new MYMONEYEXCEPTION(QString("Destcolumn %1 out of grid range (%2) in PivotTable::accumulateColumn").arg(sourcecolumn).arg((*it_row)[eActual].count()));

        (*it_row)[eActual][destcolumn] += (*it_row)[eActual][sourcecolumn];
        ++it_row;
      }

      ++it_innergroup;
    }
    ++it_outergroup;
  }
}

void PivotTable::clearColumn(unsigned column)
{
  DEBUG_ENTER(__PRETTY_FUNCTION__);
  DEBUG_OUTPUT(QString("Column %1").arg(column));

  // iterate over outer groups
  PivotGrid::iterator it_outergroup = m_grid.begin();
  while ( it_outergroup != m_grid.end() )
  {
    // iterate over inner groups
    PivotOuterGroup::iterator it_innergroup = (*it_outergroup).begin();
    while ( it_innergroup != (*it_outergroup).end() )
    {
      // iterator over rows
      PivotInnerGroup::iterator it_row = (*it_innergroup).begin();
      while ( it_row != (*it_innergroup).end() )
      {
        if ( (*it_row)[eActual].count() <= column )
          throw new MYMONEYEXCEPTION(QString("Column %1 out of grid range (%2) in PivotTable::accumulateColumn").arg(column).arg((*it_row)[eActual].count()));

        (*it_row++)[eActual][column] = PivotCell();
      }

      ++it_innergroup;
    }
    ++it_outergroup;
  }
}

void PivotTable::calculateColumnHeadings(void)
{
  DEBUG_ENTER(__PRETTY_FUNCTION__);

  // one column for the opening balance
  m_columnHeadings.append( "Opening" );

  unsigned columnpitch = m_config_f.columnPitch();

  // if this is a days-based report
  if (m_config_f.isColumnsAreDays())
  {
    if ( columnpitch == 1 )
    {
      QDate columnDate = m_beginDate;
      unsigned column = 1;
      while ( column++ < m_numColumns )
      {
        QString heading = KGlobal::locale()->calendar()->monthName(columnDate.month(), columnDate.year(), KCalendarSystem::ShortName) + " " + QString::number(columnDate.day());
        columnDate = columnDate.addDays(1);
        m_columnHeadings.append( heading);
      }
    }
    else
    {
      QDate day = m_beginDate;
      QDate prv = m_beginDate;

      // use the user's locale to determine the week's start
      unsigned dow = (day.dayOfWeek() +8 -KGlobal::locale()->weekStartDay())%7;

      while (day <= m_endDate)
      {
        if (((dow % columnpitch) == 0) || (day == m_endDate))
        {
          m_columnHeadings.append(QString("%1&nbsp;%2 - %3&nbsp;%4")
            .arg(KGlobal::locale()->calendar()->monthName(prv.month(), prv.year(), KCalendarSystem::ShortName))
            .arg(prv.day())
            .arg(KGlobal::locale()->calendar()->monthName(day.month(), day.year(), KCalendarSystem::ShortName))
            .arg(day.day()));
          prv = day.addDays(1);
        }
        day = day.addDays(1);
        dow++;
      }
    }
  }

  // else it's a months-based report
  else
  {
    if ( columnpitch == 12 )
    {
      unsigned year = m_beginDate.year();
      unsigned column = 1;
      while ( column++ < m_numColumns )
        m_columnHeadings.append(QString::number(year++));
    }
    else
    {
      unsigned year = m_beginDate.year();
      bool includeyear = ( m_beginDate.year() != m_endDate.year() );
      unsigned segment = ( m_beginDate.month() - 1 ) / columnpitch;
      unsigned column = 1;
      while ( column++ < m_numColumns )
      {
        QString heading = KGlobal::locale()->calendar()->monthName(1+segment*columnpitch, 2000, KCalendarSystem::ShortName);
        if ( columnpitch != 1 )
          heading += "-" + KGlobal::locale()->calendar()->monthName((1+segment)*columnpitch, 2000, KCalendarSystem::ShortName);
        if ( includeyear )
          heading += " " + QString::number(year);
        m_columnHeadings.append( heading);
        if ( ++segment >= 12/columnpitch )
        {
          segment -= 12/columnpitch;
          ++year;
        }
      }
    }
  }
}

void PivotTable::createAccountRows(void)
{
  DEBUG_ENTER(__PRETTY_FUNCTION__);
  MyMoneyFile* file = MyMoneyFile::instance();

  QList<MyMoneyAccount> accounts;
  file->accountList(accounts);

  QList<MyMoneyAccount>::const_iterator it_account = accounts.begin();

  while ( it_account != accounts.end() )
  {
    ReportAccount account = *it_account;

    // only include this item if its account group is included in this report
    // and if the report includes this account
    if ( m_config_f.includes( *it_account ) )
    {
      DEBUG_OUTPUT(QString("Includes account %1").arg(account.name()));

      // the row group is the account class (major account type)
      QString outergroup = KMyMoneyUtils::accountTypeToString(account.accountGroup());
      // place into the 'opening' column...
      assignCell( outergroup, account, 0, MyMoneyMoney() );
    }
    ++it_account;
  }
}

void PivotTable::calculateOpeningBalances( void )
{
  DEBUG_ENTER(__PRETTY_FUNCTION__);

  // First, determine the inclusive dates of the report.  Normally, that's just
  // the begin & end dates of m_config_f.  However, if either of those dates are
  // blank, we need to use m_beginDate and/or m_endDate instead.
  QDate from = m_config_f.fromDate();
  QDate to = m_config_f.toDate();
  if ( ! from.isValid() )
    from = m_beginDate;
  if ( ! to.isValid() )
    to = m_endDate;

  MyMoneyFile* file = MyMoneyFile::instance();

  QList<MyMoneyAccount> accounts;
  file->accountList(accounts);

  QList<MyMoneyAccount>::const_iterator it_account = accounts.begin();

  while ( it_account != accounts.end() )
  {
    ReportAccount account = *it_account;

    // only include this item if its account group is included in this report
    // and if the report includes this account
    if ( m_config_f.includes( *it_account ) )
    {

      //do not include account if it is closed and it has no transactions in the report period
      if(account.isClosed()) {
        //check if the account has transactions for the report timeframe
        MyMoneyTransactionFilter filter;
        filter.addAccount(account.id());
        filter.setDateFilter(m_beginDate, m_endDate);
        filter.setReportAllSplits(false);
        QList<MyMoneyTransaction> transactions = file->transactionList(filter);
        //if a closed account has no transactions in that timeframe, do not include it
        if(transactions.size() == 0 ) {
          DEBUG_OUTPUT(QString("DOES NOT INCLUDE account %1").arg(account.name()));
          ++it_account;
          continue;
        }
      }

      DEBUG_OUTPUT(QString("Includes account %1").arg(account.name()));
      // the row group is the account class (major account type)
      QString outergroup = KMyMoneyUtils::accountTypeToString(account.accountGroup());

      // extract the balance of the account for the given begin date, which is
      // the opening balance plus the sum of all transactions prior to the begin
      // date

      // this is in the underlying currency
      MyMoneyMoney value = file->balance(account.id(), from.addDays(-1));

      // place into the 'opening' column...
      assignCell( outergroup, account, 0, value );
    }
    else
    {
      DEBUG_OUTPUT(QString("DOES NOT INCLUDE account %1").arg(account.name()));
    }

    ++it_account;
  }
}

void PivotTable::calculateRunningSums( PivotInnerGroup::iterator& it_row)
{
  MyMoneyMoney runningsum = it_row.value()[eActual][0].calculateRunningSum(MyMoneyMoney(0,1));
  unsigned column = 1;
  while ( column < m_numColumns )
  {
    if ( it_row.value()[eActual].count() <= column )
      throw new MYMONEYEXCEPTION(QString("Column %1 out of grid range (%2) in PivotTable::calculateRunningSums").arg(column).arg(it_row.value()[eActual].count()));

    runningsum = it_row.value()[eActual][column].calculateRunningSum(runningsum);

    ++column;
  }
}

void PivotTable::calculateRunningSums( void )
{
  DEBUG_ENTER(__PRETTY_FUNCTION__);

  m_runningSumsCalculated = true;

  PivotGrid::iterator it_outergroup = m_grid.begin();
  while ( it_outergroup != m_grid.end() )
  {
    PivotOuterGroup::iterator it_innergroup = (*it_outergroup).begin();
    while ( it_innergroup != (*it_outergroup).end() )
    {
      PivotInnerGroup::iterator it_row = (*it_innergroup).begin();
      while ( it_row != (*it_innergroup).end() )
      {
#if 0
        MyMoneyMoney runningsum = it_row.value()[0];
        unsigned column = 1;
        while ( column < m_numColumns )
        {
        if ( it_row.value()[eActual].count() <= column )
        throw new MYMONEYEXCEPTION(QString("Column %1 out of grid range (%2) in PivotTable::calculateRunningSums").arg(column).arg(it_row.value()[eActual].count()));

          runningsum = ( it_row.value()[eActual][column] += runningsum );

          ++column;
        }
#endif
        calculateRunningSums( it_row );
        ++it_row;
      }
      ++it_innergroup;
    }
    ++it_outergroup;
  }
}

MyMoneyMoney PivotTable::cellBalance(const QString& outergroup, const ReportAccount& _row, unsigned _column, bool budget)
{
  if(m_runningSumsCalculated) {
    qDebug("You must not call PivotTable::cellBalance() after calling PivotTable::calculateRunningSums()");
    throw new MYMONEYEXCEPTION(QString("You must not call PivotTable::cellBalance() after calling PivotTable::calculateRunningSums()"));
  }

  // for budget reports, if this is the actual value, map it to the account which
  // holds its budget
  ReportAccount row = _row;
  if ( !budget && m_config_f.hasBudget() )
  {
    QString newrow = m_budgetMap[row.id()];

    // if there was no mapping found, then the budget report is not interested
    // in this account.
    if ( newrow.isEmpty() )
      return MyMoneyMoney();

    row = newrow;
  }

  // ensure the row already exists (and its parental hierarchy)
  createRow( outergroup, row, true );

  // Determine the inner group from the top-most parent account
  QString innergroup( row.topParentName() );

  if ( m_numColumns <= _column )
    throw new MYMONEYEXCEPTION(QString("Column %1 out of m_numColumns range (%2) in PivotTable::cellBalance").arg(_column).arg(m_numColumns));
  if ( m_grid[outergroup][innergroup][row][eActual].count() <= _column )
    throw new MYMONEYEXCEPTION(QString("Column %1 out of grid range (%2) in PivotTable::cellBalance").arg(_column).arg(m_grid[outergroup][innergroup][row][eActual].count()));

  MyMoneyMoney balance;
  if ( budget )
    balance = m_grid[outergroup][innergroup][row][eBudget][0].cellBalance(MyMoneyMoney());
  else
    balance = m_grid[outergroup][innergroup][row][eActual][0].cellBalance(MyMoneyMoney());

  unsigned column = 1;
  while ( column < _column)
  {
    if ( m_grid[outergroup][innergroup][row][eActual].count() <= column )
      throw new MYMONEYEXCEPTION(QString("Column %1 out of grid range (%2) in PivotTable::cellBalance").arg(column).arg(m_grid[outergroup][innergroup][row][eActual].count()));

    balance = m_grid[outergroup][innergroup][row][eActual][column].cellBalance(balance);

    ++column;
  }

  return balance;
}


void PivotTable::calculateBudgetMapping( void )
{
  DEBUG_ENTER(__PRETTY_FUNCTION__);

  MyMoneyFile* file = MyMoneyFile::instance();

  // Only do this if there is at least one budget in the file
  if ( file->countBudgets() )
  {
    // Select a budget
    //
    // It will choose the first budget in the list for the start year of the report if no budget is select
    MyMoneyBudget budget = MyMoneyBudget();
    //if no budget has been selected
    if (m_config_f.budget() == "Any" ) {
      QList<MyMoneyBudget> budgets = file->budgetList();
      QList<MyMoneyBudget>::const_iterator budgets_it = budgets.begin();
      while( budgets_it != budgets.end() ) {
        //pick the first budget that matches the report start year
        if( (*budgets_it).budgetStart().year() == QDate::currentDate().year() ) {
          budget = file->budget( (*budgets_it).id());
          break;
        }
        ++budgets_it;
      }
      //if we can't find a matching budget, take the first of the list
      if( budget.id() == "" )
        budget = budgets[0];

      //assign the budget to the report
      m_config_f.setBudget(budget.id(), m_config_f.isIncludingBudgetActuals());
    } else {
      //pick the budget selected by the user
      budget = file->budget( m_config_f.budget());
    }

    // Dump the budget
    //kDebug(2) << "Budget " << budget.name() << ": ";

    // Go through all accounts in the system to build the mapping
    QList<MyMoneyAccount> accounts;
    file->accountList(accounts);
    QList<MyMoneyAccount>::const_iterator it_account = accounts.begin();
    while ( it_account != accounts.end() )
    {
      //include only the accounts selected for the report
      if ( m_config_f.includes ( *it_account ) ) {
        QString id = ( *it_account ).id();
        QString acid = id;

        // If the budget contains this account outright
        if ( budget.contains ( id ) )
        {
          // Add it to the mapping
          m_budgetMap[acid] = id;
          // kDebug(2) << ReportAccount(acid).debugName() << " self-maps / type =" << budget.account(id).budgetLevel();
        }
        // Otherwise, search for a parent account which includes sub-accounts
        else
        {
          //if includeBudgetActuals, include all accounts regardless of whether in budget or not
          if ( m_config_f.isIncludingBudgetActuals() ) {
            m_budgetMap[acid] = id;
            // kDebug(2) << ReportAccount(acid).debugName() << " maps to " << ReportAccount(id).debugName();
          }
          do
          {
            id = file->account ( id ).parentAccountId();
            if ( budget.contains ( id ) )
            {
              if ( budget.account ( id ).budgetSubaccounts() )
              {
                m_budgetMap[acid] = id;
                // kDebug(2) << ReportAccount(acid).debugName() << " maps to " << ReportAccount(id).debugName();
                break;
              }
            }
          }
          while ( ! id.isEmpty() );
        }
      }
      ++it_account;
    } // end while looping through the accounts in the file

    // Place the budget values into the budget grid
    QList<MyMoneyBudget::AccountGroup> baccounts = budget.getaccounts();
    QList<MyMoneyBudget::AccountGroup>::const_iterator it_bacc = baccounts.begin();
    while ( it_bacc != baccounts.end() )
    {
      ReportAccount splitAccount = (*it_bacc).id();

      //include the budget account only if it is included in the report
      if ( m_config_f.includes ( splitAccount ) ) {
        MyMoneyAccount::accountTypeE type = splitAccount.accountGroup();
        QString outergroup = KMyMoneyUtils::accountTypeToString(type);

        // reverse sign to match common notation for cash flow direction, only for expense/income splits
        MyMoneyMoney reverse((splitAccount.accountType() == MyMoneyAccount::Expense) ? -1 : 1, 1);

        const QMap<QDate, MyMoneyBudget::PeriodGroup>& periods = (*it_bacc).getPeriods();
        MyMoneyMoney value = (*periods.begin()).amount() * reverse;
        MyMoneyMoney price = MyMoneyMoney(1,1);
        unsigned column = 1;

        // based on the kind of budget it is, deal accordingly
        switch ( (*it_bacc).budgetLevel() )
        {
          case MyMoneyBudget::AccountGroup::eYearly:
            // divide the single yearly value by 12 and place it in each column
            value /= MyMoneyMoney(12,1);
          case MyMoneyBudget::AccountGroup::eNone:
          case MyMoneyBudget::AccountGroup::eMax:
          case MyMoneyBudget::AccountGroup::eMonthly:
            // place the single monthly value in each column of the report
            // only add the value if columns are monthly or longer
            if(m_config_f.columnType() == MyMoneyReport::eBiMonths
               || m_config_f.columnType() == MyMoneyReport::eMonths
               || m_config_f.columnType() == MyMoneyReport::eYears
               || m_config_f.columnType() == MyMoneyReport::eQuarters) {
              value = value * MyMoneyMoney(m_config_f.columnType(), 1);
              while ( column < m_numColumns )
              {
                //only show budget values if the budget year and the column date match
                //no currency conversion is done here because that is done for all columns later
                if( budget.budgetStart().year() == columnDate(column).year() ) {
                  assignCell( outergroup, splitAccount, column, value, true /*budget*/ );
                }
                ++column;
              }
            }
            break;
          case MyMoneyBudget::AccountGroup::eMonthByMonth:
          // place each value in the appropriate column
          // budget periods are supposed to come in order just like columns
          {
            QMap<QDate, MyMoneyBudget::PeriodGroup>::const_iterator it_period = periods.begin();
            while ( it_period != periods.end() && column < m_numColumns)
            {
              if((*it_period).startDate() > columnDate(column) ) {
                ++column;
              } else {
                switch(m_config_f.columnType()) {
                  case MyMoneyReport::eYears:
                  case MyMoneyReport::eBiMonths:
                  case MyMoneyReport::eQuarters:
                  case MyMoneyReport::eMonths:
                  {
                    if((*it_period).startDate() >= m_beginDate.addDays(-m_beginDate.day() + 1)
                        && (*it_period).startDate() <= m_endDate.addDays(m_endDate.daysInMonth() - m_endDate.day() )
                        && (*it_period).startDate() > (columnDate(column).addMonths(-m_config_f.columnType()))) {
                      //no currency conversion is done here because that is done for all columns later
                      value = (*it_period).amount() * reverse;
                      assignCell( outergroup, splitAccount, column, value, true /*budget*/ );
                    }
                    ++it_period;
                    break;
                  }
                  default:
                    break;
                }
              }
            }
            break;
          }
        }
      }
      ++it_bacc;
    }
  } // end if there was a budget
}

void PivotTable::convertToBaseCurrency( void )
{
  DEBUG_ENTER(__PRETTY_FUNCTION__);

  int fraction = MyMoneyFile::instance()->baseCurrency().smallestAccountFraction();

  PivotGrid::iterator it_outergroup = m_grid.begin();
  while ( it_outergroup != m_grid.end() )
  {
    PivotOuterGroup::iterator it_innergroup = (*it_outergroup).begin();
    while ( it_innergroup != (*it_outergroup).end() )
    {
      PivotInnerGroup::iterator it_row = (*it_innergroup).begin();
      while ( it_row != (*it_innergroup).end() )
      {
        unsigned column = 1;
        while ( column < m_numColumns )
        {
          if ( it_row.value()[eActual].count() <= column )
            throw new MYMONEYEXCEPTION(QString("Column %1 out of grid range (%2) in PivotTable::convertToBaseCurrency").arg(column).arg(it_row.value()[eActual].count()));

          QDate valuedate = columnDate(column);

          //get base price for that date
          MyMoneyMoney conversionfactor = it_row.key().baseCurrencyPrice(valuedate);

          for(unsigned i = 0; i < m_rowTypeList.size(); ++i) {
            if( m_rowTypeList[i] != eAverage ) {
              //calculate base value
              MyMoneyMoney oldval = it_row.value()[ m_rowTypeList[i] ][column];
              MyMoneyMoney value = (oldval * conversionfactor).reduce();

              //convert to lowest fraction
              it_row.value()[ m_rowTypeList[i] ][column] = PivotCell(value.convert(fraction));

              DEBUG_OUTPUT_IF(conversionfactor != MyMoneyMoney(1,1) ,QString("Factor of %1, value was %2, now %3").arg(conversionfactor).arg(DEBUG_SENSITIVE(oldval)).arg(DEBUG_SENSITIVE(it_row.value()[m_rowTypeList[i]][column].toDouble())));
            }
          }


          ++column;
        }
        ++it_row;
      }
      ++it_innergroup;
    }
    ++it_outergroup;
  }
}

void PivotTable::convertToDeepCurrency( void )
{
  DEBUG_ENTER(__PRETTY_FUNCTION__);

  int fraction;

  PivotGrid::iterator it_outergroup = m_grid.begin();
  while ( it_outergroup != m_grid.end() )
  {
    PivotOuterGroup::iterator it_innergroup = (*it_outergroup).begin();
    while ( it_innergroup != (*it_outergroup).end() )
    {
      PivotInnerGroup::iterator it_row = (*it_innergroup).begin();
      while ( it_row != (*it_innergroup).end() )
      {
        unsigned column = 1;
        while ( column < m_numColumns )
        {
          if ( it_row.value()[eActual].count() <= column )
            throw new MYMONEYEXCEPTION(QString("Column %1 out of grid range (%2) in PivotTable::convertToDeepCurrency").arg(column).arg(it_row.value()[eActual].count()));

          QDate valuedate = columnDate(column);

          //get conversion factor for the account and date
          MyMoneyMoney conversionfactor = it_row.key().deepCurrencyPrice(valuedate);

          //use the fraction relevant to the account at hand
          fraction = it_row.key().fraction();

          //use base currency fraction if not initialized
          if(fraction == -1)
            fraction = MyMoneyFile::instance()->baseCurrency().smallestAccountFraction();

          //convert to deep currency
          MyMoneyMoney oldval = it_row.value()[eActual][column];
          MyMoneyMoney value = (oldval * conversionfactor).reduce();
          //reduce to lowest fraction
          it_row.value()[eActual][column] = PivotCell(value.convert(fraction));

          //convert price data
          if(m_config_f.isIncludingPrice()) {
            MyMoneyMoney oldPriceVal = it_row.value()[ePrice][column];
            MyMoneyMoney priceValue = (oldPriceVal * conversionfactor).reduce();
            it_row.value()[ePrice][column] = PivotCell(priceValue.convert(10000));
          }

          DEBUG_OUTPUT_IF(conversionfactor != MyMoneyMoney(1,1) ,QString("Factor of %1, value was %2, now %3").arg(conversionfactor).arg(DEBUG_SENSITIVE(oldval)).arg(DEBUG_SENSITIVE(it_row.value()[eActual][column].toDouble())));

          ++column;
        }
        ++it_row;
      }
      ++it_innergroup;
    }
    ++it_outergroup;
  }
}

void PivotTable::calculateTotals( void )
{
  //insert the row type that is going to be used
  for(unsigned i = 0; i < m_rowTypeList.size(); ++i) {
    for(unsigned k = 0; k < m_numColumns; ++k) {
      m_grid.m_total[ m_rowTypeList[i] ].append( PivotCell() );
    }
  }
  //
  // Outer groups
  //

  // iterate over outer groups
  PivotGrid::iterator it_outergroup = m_grid.begin();
  while ( it_outergroup != m_grid.end() )
  {
    for(unsigned i = 0; i < m_rowTypeList.size(); ++i) {
      for(unsigned k = 0; k < m_numColumns; ++k) {
        (*it_outergroup).m_total[ m_rowTypeList[i] ].append( PivotCell() );
      }
    }

    //
    // Inner Groups
    //

    PivotOuterGroup::iterator it_innergroup = (*it_outergroup).begin();
    while ( it_innergroup != (*it_outergroup).end() )
    {
      for(unsigned i = 0; i < m_rowTypeList.size(); ++i) {
        for(unsigned k = 0; k < m_numColumns; ++k) {
          (*it_innergroup).m_total[ m_rowTypeList[i] ].append( PivotCell() );
        }
      }
      //
      // Rows
      //

      PivotInnerGroup::iterator it_row = (*it_innergroup).begin();
      while ( it_row != (*it_innergroup).end() )
      {
        //
        // Columns
        //

        unsigned column = 1;
        while ( column < m_numColumns )
        {
          for(unsigned i = 0; i < m_rowTypeList.size(); ++i) {
            if ( it_row.value()[ m_rowTypeList[i] ].count() <= column )
              throw new MYMONEYEXCEPTION(QString("Column %1 out of grid range (%2) in PivotTable::calculateTotals, row columns").arg(column).arg(it_row.value()[ m_rowTypeList[i] ].count()));
            if ( (*it_innergroup).m_total[ m_rowTypeList[i] ].count() <= column )
              throw new MYMONEYEXCEPTION(QString("Column %1 out of grid range (%2) in PivotTable::calculateTotals, inner group totals").arg(column).arg((*it_innergroup).m_total[ m_rowTypeList[i] ].count()));

            //calculate total
            MyMoneyMoney value = it_row.value()[ m_rowTypeList[i] ][column];
            (*it_innergroup).m_total[ m_rowTypeList[i] ][column] += value;
            (*it_row)[ m_rowTypeList[i] ].m_total += value;
          }
          ++column;
        }
        ++it_row;
      }

      //
      // Inner Row Group Totals
      //

      unsigned column = 1;
      while ( column < m_numColumns )
      {
        for(unsigned i = 0; i < m_rowTypeList.size(); ++i) {
          if ( (*it_innergroup).m_total[ m_rowTypeList[i] ].count() <= column )
            throw new MYMONEYEXCEPTION(QString("Column %1 out of grid range (%2) in PivotTable::calculateTotals, inner group totals").arg(column).arg((*it_innergroup).m_total[ m_rowTypeList[i] ].count()));
          if ( (*it_outergroup).m_total[ m_rowTypeList[i] ].count() <= column )
            throw new MYMONEYEXCEPTION(QString("Column %1 out of grid range (%2) in PivotTable::calculateTotals, outer group totals").arg(column).arg((*it_innergroup).m_total[ m_rowTypeList[i] ].count()));

          //calculate totals
          MyMoneyMoney value = (*it_innergroup).m_total[ m_rowTypeList[i] ][column];
          (*it_outergroup).m_total[ m_rowTypeList[i] ][column] += value;
          (*it_innergroup).m_total[ m_rowTypeList[i] ].m_total += value;
        }
        ++column;
      }

      ++it_innergroup;
    }

    //
    // Outer Row Group Totals
    //

    bool invert_total = (*it_outergroup).m_inverted;
    unsigned column = 1;
    while ( column < m_numColumns )
    {
      for(unsigned i = 0; i < m_rowTypeList.size(); ++i) {
        if ( m_grid.m_total[ m_rowTypeList[i] ].count() <= column )
          throw new MYMONEYEXCEPTION(QString("Column %1 out of grid range (%2) in PivotTable::calculateTotals, grid totals").arg(column).arg((*it_innergroup).m_total[ m_rowTypeList[i] ].count()));

      //calculate actual totals
        MyMoneyMoney value = (*it_outergroup).m_total[ m_rowTypeList[i] ][column];
        (*it_outergroup).m_total[ m_rowTypeList[i] ].m_total += value;

        //so far the invert only applies to actual and budget
        if ( invert_total
             && m_rowTypeList[i] != eBudgetDiff
             &&  m_rowTypeList[i] != eForecast)
          value = -value;

        m_grid.m_total[ m_rowTypeList[i] ][column] += value;
      }
      ++column;
    }
    ++it_outergroup;
  }

  //
  // Report Totals
  //

  unsigned totalcolumn = 1;
  while ( totalcolumn < m_numColumns )
  {
    for(unsigned i = 0; i < m_rowTypeList.size(); ++i) {
      if ( m_grid.m_total[ m_rowTypeList[i] ].count() <= totalcolumn )
        throw new MYMONEYEXCEPTION(QString("Total column %1 out of grid range (%2) in PivotTable::calculateTotals, grid totals").arg(totalcolumn).arg(m_grid.m_total[ m_rowTypeList[i] ].count()));

    //calculate actual totals
      MyMoneyMoney value = m_grid.m_total[ m_rowTypeList[i] ][totalcolumn];
      m_grid.m_total[ m_rowTypeList[i] ].m_total += value;
    }
    ++totalcolumn;
  }
}

void PivotTable::assignCell( const QString& outergroup, const ReportAccount& _row, unsigned column, MyMoneyMoney value, bool budget, bool stockSplit )
{
  DEBUG_ENTER(__PRETTY_FUNCTION__);
  DEBUG_OUTPUT(QString("Parameters: %1,%2,%3,%4,%5").arg(outergroup).arg(_row.debugName()).arg(column).arg(DEBUG_SENSITIVE(value.toDouble())).arg(budget));

  // for budget reports, if this is the actual value, map it to the account which
  // holds its budget
  ReportAccount row = _row;
  if ( !budget && m_config_f.hasBudget() )
  {
    QString newrow = m_budgetMap[row.id()];

    // if there was no mapping found, then the budget report is not interested
    // in this account.
    if ( newrow.isEmpty() )
      return;

    row = newrow;
  }

  // ensure the row already exists (and its parental hierarchy)
  createRow( outergroup, row, true );

  // Determine the inner group from the top-most parent account
  QString innergroup( row.topParentName() );

  if ( m_numColumns <= column )
    throw new MYMONEYEXCEPTION(QString("Column %1 out of m_numColumns range (%2) in PivotTable::assignCell").arg(column).arg(m_numColumns));
  if ( m_grid[outergroup][innergroup][row][eActual].count() <= column )
    throw new MYMONEYEXCEPTION(QString("Column %1 out of grid range (%2) in PivotTable::assignCell").arg(column).arg(m_grid[outergroup][innergroup][row][eActual].count()));

  if(!stockSplit) {
    // Determine whether the value should be inverted before being placed in the row
    if ( m_grid[outergroup].m_inverted )
      value = -value;

    // Add the value to the grid cell
    if ( budget )
      m_grid[outergroup][innergroup][row][eBudget][column] += value;
    else
      m_grid[outergroup][innergroup][row][eActual][column] += value;
  } else {
    m_grid[outergroup][innergroup][row][eActual][column] += PivotCell::stockSplit(value);
  }

}

void PivotTable::createRow( const QString& outergroup, const ReportAccount& row, bool recursive )
{
  DEBUG_ENTER(__PRETTY_FUNCTION__);

  // Determine the inner group from the top-most parent account
  QString innergroup( row.topParentName() );

  if ( ! m_grid.contains(outergroup) )
  {
    DEBUG_OUTPUT(QString("Adding group [%1]").arg(outergroup));
    m_grid[outergroup] = PivotOuterGroup(m_numColumns);
  }

  if ( ! m_grid[outergroup].contains(innergroup) )
  {
    DEBUG_OUTPUT(QString("Adding group [%1][%2]").arg(outergroup).arg(innergroup));
    m_grid[outergroup][innergroup] = PivotInnerGroup(m_numColumns);
  }

  if ( ! m_grid[outergroup][innergroup].contains(row) )
  {
    DEBUG_OUTPUT(QString("Adding row [%1][%2][%3]").arg(outergroup).arg(innergroup).arg(row.debugName()));
    m_grid[outergroup][innergroup][row] = PivotGridRowSet(m_numColumns);

    if ( recursive && !row.isTopLevel() )
        createRow( outergroup, row.parent(), recursive );
  }
}

unsigned PivotTable::columnValue(const QDate& _date) const
{
  if (m_config_f.isColumnsAreDays())
    return (QDate().daysTo(_date));
  else
    return (_date.year() * 12 + _date.month());
}

QDate PivotTable::columnDate(int column) const
{
  if (m_config_f.isColumnsAreDays())
    return m_beginDate.addDays( m_config_f.columnPitch() * column - 1 );
  else
    return m_beginDate.addMonths( m_config_f.columnPitch() * column ).addDays(-1);
}

QString PivotTable::renderCSV( void ) const
{
  DEBUG_ENTER(__PRETTY_FUNCTION__);

  //
  // Report Title
  //

  QString result = QString("\"Report: %1\"\n").arg(m_config_f.name());
  if ( m_config_f.isConvertCurrency() )
    result += i18n("All currencies converted to %1\n").arg(MyMoneyFile::instance()->baseCurrency().name());
  else
    result += i18n("All values shown in %1 unless otherwise noted\n").arg(MyMoneyFile::instance()->baseCurrency().name());

  //
  // Table Header
  //

  result += i18n("Account");

  unsigned column = 1;
  while ( column < m_numColumns )
    result += QString(",%1").arg(QString(m_columnHeadings[column++]));

  if ( m_config_f.isShowingRowTotals() )
    result += QString(",%1").arg(i18n("Total"));

  result += "\n";

  int fraction = MyMoneyFile::instance()->baseCurrency().smallestAccountFraction();

  //
  // Outer groups
  //

  // iterate over outer groups
  PivotGrid::const_iterator it_outergroup = m_grid.begin();
  while ( it_outergroup != m_grid.end() )
  {
    //
    // Outer Group Header
    //

    result += it_outergroup.key() + "\n";

    //
    // Inner Groups
    //

    PivotOuterGroup::const_iterator it_innergroup = (*it_outergroup).begin();
    unsigned rownum = 0;
    while ( it_innergroup != (*it_outergroup).end() )
    {
      //
      // Rows
      //

      QString innergroupdata;
      PivotInnerGroup::const_iterator it_row = (*it_innergroup).begin();
      while ( it_row != (*it_innergroup).end() )
      {
        ReportAccount rowname = it_row.key();
        int fraction = rowname.fraction(rowname.currency());

        //
        // Columns
        //

        QString rowdata;
        unsigned column = 1;

        bool isUsed = false;
        for(unsigned i = 0; i < m_rowTypeList.size(); ++i)
          isUsed |= it_row.value()[ m_rowTypeList[i] ][0].isUsed();

        while ( column < m_numColumns ) {
          //show columns
          for(unsigned i = 0; i < m_rowTypeList.size(); ++i) {
            isUsed |= it_row.value()[ m_rowTypeList[i] ][column].isUsed();
            rowdata += QString(",\"%1\"").arg(it_row.value()[ m_rowTypeList[i] ][column].formatMoney(fraction, false));
          }
          column++;
        }

        if ( m_config_f.isShowingRowTotals() ) {
          for(unsigned i = 0; i < m_rowTypeList.size(); ++i)
            rowdata += QString(",\"%1\"").arg((*it_row)[ m_rowTypeList[i] ].m_total.formatMoney(fraction, false));
        }

        //
        // Row Header
        //

        if(!rowname.isClosed() || isUsed) {
          innergroupdata += "\"" + QString().fill(' ',rowname.hierarchyDepth() - 1) + rowname.name();

          // if we don't convert the currencies to the base currency and the
          // current row contains a foreign currency, then we append the currency
          // to the name of the account
          if (!m_config_f.isConvertCurrency() && rowname.isForeignCurrency() )
            innergroupdata += QString(" (%1)").arg(rowname.currencyId());

          innergroupdata += "\"";

          if ( isUsed )
            innergroupdata += rowdata;

          innergroupdata += "\n";
        }
        ++it_row;
      }

      //
      // Inner Row Group Totals
      //

      bool finishrow = true;
      QString finalRow;
      bool isUsed = false;
      if ( m_config_f.detailLevel() == MyMoneyReport::eDetailAll && ((*it_innergroup).size() > 1 ))
      {
        // Print the individual rows
        result += innergroupdata;

        if ( m_config_f.isShowingColumnTotals() )
        {
          // Start the TOTALS row
          finalRow = i18n("Total");
          isUsed = true;
        }
        else
        {
          ++rownum;
          finishrow = false;
        }
      }
      else
      {
        // Start the single INDIVIDUAL ACCOUNT row
        ReportAccount rowname = (*it_innergroup).begin().key();
        isUsed |= !rowname.isClosed();

        finalRow = "\"" + QString().fill(' ',rowname.hierarchyDepth() - 1) + rowname.name();
        if (!m_config_f.isConvertCurrency() && rowname.isForeignCurrency() )
          finalRow += QString(" (%1)").arg(rowname.currencyId());
        finalRow += "\"";
      }

      // Finish the row started above, unless told not to
      if ( finishrow )
      {
        unsigned column = 1;

        for(unsigned i = 0; i < m_rowTypeList.size(); ++i)
          isUsed |= (*it_innergroup).m_total[ m_rowTypeList[i] ][0].isUsed();

        while ( column < m_numColumns )
        {
          for(unsigned i = 0; i < m_rowTypeList.size(); ++i) {
            isUsed |= (*it_innergroup).m_total[ m_rowTypeList[i] ][column].isUsed();
            finalRow += QString(",\"%1\"").arg((*it_innergroup).m_total[ m_rowTypeList[i] ][column].formatMoney(fraction, false));
          }
          column++;
        }

        if (  m_config_f.isShowingRowTotals() ) {
          for(unsigned i = 0; i < m_rowTypeList.size(); ++i)
            finalRow += QString(",\"%1\"").arg((*it_innergroup).m_total[ m_rowTypeList[i] ].m_total.formatMoney(fraction, false));
        }

        finalRow += "\n";
      }

      if(isUsed)
      {
        result += finalRow;
        ++rownum;
      }
      ++it_innergroup;
    }

    //
    // Outer Row Group Totals
    //

    if ( m_config_f.isShowingColumnTotals() )
    {
      result += QString("%1 %2").arg(i18n("Total")).arg(it_outergroup.key());
      unsigned column = 1;
      while ( column < m_numColumns ) {
        for(unsigned i = 0; i < m_rowTypeList.size(); ++i)
          result += QString(",\"%1\"").arg((*it_outergroup).m_total[ m_rowTypeList[i] ][column].formatMoney(fraction, false));

        column++;
      }

      if (  m_config_f.isShowingRowTotals() ) {
        for(unsigned i = 0; i < m_rowTypeList.size(); ++i)
          result += QString(",\"%1\"").arg((*it_outergroup).m_total[ m_rowTypeList[i] ].m_total.formatMoney(fraction, false));
      }

      result += "\n";
    }
    ++it_outergroup;
  }

  //
  // Report Totals
  //

  if ( m_config_f.isShowingColumnTotals() )
  {
    result += i18n("Grand Total");
    unsigned totalcolumn = 1;
    while ( totalcolumn < m_numColumns ) {
      for(unsigned i = 0; i < m_rowTypeList.size(); ++i)
        result += QString(",\"%1\"").arg(m_grid.m_total[ m_rowTypeList[i] ][totalcolumn].formatMoney(fraction, false));

      totalcolumn++;
    }

    if (  m_config_f.isShowingRowTotals() ) {
      for(unsigned i = 0; i < m_rowTypeList.size(); ++i)
        result += QString(",\"%1\"").arg(m_grid.m_total[ m_rowTypeList[i] ].m_total.formatMoney(fraction, false));
    }

    result += "\n";
  }

  return result;
}

QString PivotTable::renderHTML( void ) const
{
  DEBUG_ENTER(__PRETTY_FUNCTION__);

  QString colspan = QString(" colspan=\"%1\"").arg(m_numColumns + 1 + (m_config_f.isShowingRowTotals() ? 1 : 0) );

  //
  // Report Title
  //

  QString result = QString("<h2 class=\"report\">%1</h2>\n").arg(m_config_f.name());

  //actual dates of the report
  result += QString("<div class=\"subtitle\">");
  result += i18nc("Report date range", "%1 through %2").arg(KGlobal::locale()->formatDateTime(QDateTime(m_config_f.fromDate()), KLocale::ShortDate, false)).arg(KGlobal::locale()->formatDateTime(QDateTime(m_config_f.toDate()), KLocale::ShortDate, false));
  result += QString("</div>\n");
  result += QString("<div class=\"gap\">&nbsp;</div>\n");

  //currency conversion message
  result += QString("<div class=\"subtitle\">");
  if ( m_config_f.isConvertCurrency() )
    result += i18n("All currencies converted to %1").arg(MyMoneyFile::instance()->baseCurrency().name());
  else
    result += i18n("All values shown in %1 unless otherwise noted").arg(MyMoneyFile::instance()->baseCurrency().name());
  result += QString("</div>\n");
  result += QString("<div class=\"gap\">&nbsp;</div>\n");

  // setup a leftborder for better readability of budget vs actual reports
  QString leftborder;
  if (m_rowTypeList.size() > 1)
    leftborder = " class=\"leftborder\"";

  //
  // Table Header
  //
  result += QString("\n\n<table class=\"report\" cellspacing=\"0\">\n"
       "<thead><tr class=\"itemheader\">\n<th>%1</th>").arg(i18n("Account"));

  QString headerspan;
  int span = m_rowTypeList.size();

  headerspan = QString(" colspan=\"%1\"").arg(span);

  unsigned column = 1;
  while ( column < m_numColumns )
    result += QString("<th%1>%2</th>").arg(headerspan,QString(m_columnHeadings[column++]).replace(QRegExp(" "),"<br>"));

  if ( m_config_f.isShowingRowTotals() )
    result += QString("<th%1>%2</th>").arg(headerspan).arg(i18n("Total"));

  result += "</tr></thead>\n";

  //
  // Header for multiple columns
  //
  if ( span > 1 )
  {
    result += "<tr><td></td>";

    unsigned column = 1;
    while ( column < m_numColumns )
    {
      QString lb;
      if(column != 1)
        lb = leftborder;

      for(unsigned i = 0; i < m_rowTypeList.size(); ++i) {
        result += QString("<td%2>%1</td>")
            #warning #Port to KDE4
            //.arg(i18n( m_columnTypeHeaderList[i] ))
            .arg( m_columnTypeHeaderList[i] )
            .arg(i == 0 ? lb : QString() );
      }
      column++;
    }
    if ( m_config_f.isShowingRowTotals() ) {
      for(unsigned i = 0; i < m_rowTypeList.size(); ++i) {
        result += QString("<td%2>%1</td>")
            #warning #Port to KDE4
            //.arg(i18n( m_columnTypeHeaderList[i] ))
            .arg( m_columnTypeHeaderList[i] )
            .arg(i == 0 ? leftborder : QString() );
      }
    }
    result += "</tr>";
  }


  // Skip the body of the report if the report only calls for totals to be shown
  if ( m_config_f.detailLevel() != MyMoneyReport::eDetailTotal )
  {
    //
    // Outer groups
    //

    // Need to sort the outergroups.  They can't always be sorted by name.  So we create a list of
    // map iterators, and sort that.  Then we'll iterate through the map iterators and use those as
    // before.
    //
    // I hope this doesn't bog the performance of reports, given that we're copying the entire report
    // data.  If this is a perf hit, we could change to storing outergroup pointers, I think.
    QList<PivotOuterGroup> outergroups;
    PivotGrid::const_iterator it_outergroup_map = m_grid.begin();
    while ( it_outergroup_map != m_grid.end() )
    {
      outergroups.push_back(it_outergroup_map.value());

      // copy the name into the outergroup, because we will now lose any association with
      // the map iterator
      outergroups.back().m_displayName = it_outergroup_map.key();

      ++it_outergroup_map;
    }
    #warning #Port to KDE4
    qSort(outergroups.begin(), outergroups.end());

    QList<PivotOuterGroup>::const_iterator it_outergroup = outergroups.begin();
    while ( it_outergroup != outergroups.end() )
    {
      //
      // Outer Group Header
      //

      result += QString("<tr class=\"sectionheader\"><td class=\"left\"%1>%2</td></tr>\n").arg(colspan).arg((*it_outergroup).m_displayName);

      // Skip the inner groups if the report only calls for outer group totals to be shown
      if ( m_config_f.detailLevel() != MyMoneyReport::eDetailGroup )
      {

        //
        // Inner Groups
        //

        PivotOuterGroup::const_iterator it_innergroup = (*it_outergroup).begin();
        unsigned rownum = 0;
        while ( it_innergroup != (*it_outergroup).end() )
        {
          //
          // Rows
          //

          QString innergroupdata;
          PivotInnerGroup::const_iterator it_row = (*it_innergroup).begin();
          while ( it_row != (*it_innergroup).end() )
          {
            //
            // Columns
            //

            QString rowdata;
            unsigned column = 1;
            bool isUsed = it_row.value()[eActual][0].isUsed();
            while ( column < m_numColumns )
            {
              QString lb;
              if(column != 1)
                lb = leftborder;

              for(unsigned i = 0; i < m_rowTypeList.size(); ++i) {
                rowdata += QString("<td%2>%1</td>")
                    .arg(coloredAmount(it_row.value()[ m_rowTypeList[i] ][column]))
                    .arg(i == 0 ? lb : QString());

                isUsed |= it_row.value()[ m_rowTypeList[i] ][column].isUsed();
              }

              column++;
            }

            if ( m_config_f.isShowingRowTotals() )
            {
              for(unsigned i = 0; i < m_rowTypeList.size(); ++i) {
                rowdata += QString("<td%2>%1</td>")
                    .arg(coloredAmount(it_row.value()[ m_rowTypeList[i] ].m_total))
                    .arg(i == 0 ? leftborder : QString());
              }
            }

            //
            // Row Header
            //

            ReportAccount rowname = it_row.key();

            // don't show closed accounts if they have not been used
            if(!rowname.isClosed() || isUsed) {
              innergroupdata += QString("<tr class=\"row-%1\"%2><td%3 class=\"left\" style=\"text-indent: %4.0em\">%5%6</td>")
                .arg(rownum & 0x01 ? "even" : "odd")
                .arg(rowname.isTopLevel() ? " id=\"topparent\"" : "")
                .arg("") //.arg((*it_row).m_total.isZero() ? colspan : "")  // colspan the distance if this row will be blank
                .arg(rowname.hierarchyDepth() - 1)
                .arg(rowname.name().replace(QRegExp(" "), "&nbsp;"))
                .arg((m_config_f.isConvertCurrency() || !rowname.isForeignCurrency() )?QString():QString(" (%1)").arg(rowname.currency().id()));

              // Don't print this row if it's going to be all zeros
              // TODO: Uncomment this, and deal with the case where the data
              // is zero, but the budget is non-zero
              //if ( !(*it_row).m_total.isZero() )
              innergroupdata += rowdata;

              innergroupdata += "</tr>\n";
            }

            ++it_row;
          }

          //
          // Inner Row Group Totals
          //

          bool finishrow = true;
          QString finalRow;
          bool isUsed = false;
          if ( m_config_f.detailLevel() == MyMoneyReport::eDetailAll && ((*it_innergroup).size() > 1 ))
          {
            // Print the individual rows
            result += innergroupdata;

            if ( m_config_f.isShowingColumnTotals() )
            {
              // Start the TOTALS row
              finalRow = QString("<tr class=\"row-%1\" id=\"subtotal\"><td class=\"left\">&nbsp;&nbsp;%2</td>")
                .arg(rownum & 0x01 ? "even" : "odd")
                .arg(i18n("Total"));
              // don't suppress display of totals
              isUsed = true;
            }
            else {
              finishrow = false;
              ++rownum;
            }
          }
          else
          {
            // Start the single INDIVIDUAL ACCOUNT row
            // FIXME: There is a bit of a bug here with class=leftX.  There's only a finite number
            // of classes I can define in the .CSS file, and the user can theoretically nest deeper.
            // The right solution is to use style=Xem, and calculate X.  Let's see if anyone complains
            // first :)  Also applies to the row header case above.
            // FIXED: I found it in one of my reports and changed it to the proposed method.
            // This works for me (ipwizard)
            ReportAccount rowname = (*it_innergroup).begin().key();
            isUsed |= !rowname.isClosed();
            finalRow = QString("<tr class=\"row-%1\"%2><td class=\"left\" style=\"text-indent: %3.0em;\">%5%6</td>")
              .arg(rownum & 0x01 ? "even" : "odd")
                .arg( m_config_f.detailLevel() == MyMoneyReport::eDetailAll ? "id=\"solo\"" : "" )
              .arg(rowname.hierarchyDepth() - 1)
              .arg(rowname.name().replace(QRegExp(" "), "&nbsp;"))
              .arg((m_config_f.isConvertCurrency() || !rowname.isForeignCurrency() )?QString():QString(" (%1)").arg(rowname.currency().id()));
          }

          // Finish the row started above, unless told not to
          if ( finishrow )
          {
            unsigned column = 1;
            isUsed |= (*it_innergroup).m_total[eActual][0].isUsed();
            while ( column < m_numColumns )
            {
              QString lb;
              if(column != 1)
                lb = leftborder;

              for(unsigned i = 0; i < m_rowTypeList.size(); ++i) {
                finalRow += QString("<td%2>%1</td>")
                    .arg(coloredAmount((*it_innergroup).m_total[ m_rowTypeList[i] ][column]))
                    .arg(i == 0 ? lb : QString());
                isUsed |= (*it_innergroup).m_total[ m_rowTypeList[i] ][column].isUsed();
              }

              column++;
            }

            if (  m_config_f.isShowingRowTotals() )
            {
              for(unsigned i = 0; i < m_rowTypeList.size(); ++i) {
                finalRow += QString("<td%2>%1</td>")
                    .arg(coloredAmount((*it_innergroup).m_total[ m_rowTypeList[i] ].m_total))
                    .arg(i == 0 ? leftborder : QString());
              }
            }

            finalRow += "</tr>\n";
            if(isUsed) {
              result += finalRow;
              ++rownum;
            }
          }

          ++it_innergroup;

        } // end while iterating on the inner groups

      } // end if detail level is not "group"

      //
      // Outer Row Group Totals
      //

      if ( m_config_f.isShowingColumnTotals() )
      {
        result += QString("<tr class=\"sectionfooter\"><td class=\"left\">%1&nbsp;%2</td>").arg(i18n("Total")).arg((*it_outergroup).m_displayName);
        unsigned column = 1;
        while ( column < m_numColumns )
        {
          QString lb;
          if(column != 1)
            lb = leftborder;

          for(unsigned i = 0; i < m_rowTypeList.size(); ++i) {
            result += QString("<td%2>%1</td>")
                .arg(coloredAmount((*it_outergroup).m_total[ m_rowTypeList[i] ][column]))
                .arg(i == 0 ? lb : QString());
          }

          column++;
        }

        if (  m_config_f.isShowingRowTotals() )
        {
          for(unsigned i = 0; i < m_rowTypeList.size(); ++i) {
            result += QString("<td%2>%1</td>")
                .arg(coloredAmount((*it_outergroup).m_total[ m_rowTypeList[i] ].m_total))
                .arg(i == 0 ? leftborder : QString());
          }
        }
        result += "</tr>\n";
      }

      ++it_outergroup;

    } // end while iterating on the outergroups

  } // end if detail level is not "total"

  //
  // Report Totals
  //

  if ( m_config_f.isShowingColumnTotals() )
  {
    result += QString("<tr class=\"spacer\"><td>&nbsp;</td></tr>\n");
    result += QString("<tr class=\"reportfooter\"><td class=\"left\">%1</td>").arg(i18n("Grand Total"));
    unsigned totalcolumn = 1;
    while ( totalcolumn < m_numColumns )
    {
      QString lb;
      if(totalcolumn != 1)
        lb = leftborder;

      for(unsigned i = 0; i < m_rowTypeList.size(); ++i) {
        result += QString("<td%2>%1</td>")
            .arg(coloredAmount(m_grid.m_total[ m_rowTypeList[i] ][totalcolumn]))
            .arg(i == 0 ? lb : QString());
      }

      totalcolumn++;
    }

    if (  m_config_f.isShowingRowTotals() )
    {
      for(unsigned i = 0; i < m_rowTypeList.size(); ++i) {
        result += QString("<td%2>%1</td>")
            .arg(coloredAmount(m_grid.m_total[ m_rowTypeList[i] ].m_total))
            .arg(i == 0 ? leftborder : QString());
      }
    }

    result += "</tr>\n";
  }

  result += QString("<tr class=\"spacer\"><td>&nbsp;</td></tr>\n");
  result += QString("<tr class=\"spacer\"><td>&nbsp;</td></tr>\n");
  result += "</table>\n";

  return result;
}

void PivotTable::dump( const QString& file, const QString& /* context */) const
{
  QFile g( file );
  g.open( QIODevice::WriteOnly );
  Q3TextStream(&g) << renderHTML();
  g.close();
}

#warning #Port to KDE4
#if 0
#ifdef HAVE_KDCHART
void PivotTable::drawChart( KReportChartView& _view ) const
{
#if 1 // make this "#if 1" if you want to play with the axis settings
  // not sure if 0 is X and 1 is Y.
  KDChartAxisParams xAxisParams, yAxisParams;
  KDChartAxisParams::deepCopy(xAxisParams, _view.params()->axisParams(0));
  KDChartAxisParams::deepCopy(yAxisParams, _view.params()->axisParams(1));

  // modify axis settings here
  xAxisParams.setAxisLabelsFontMinSize(12);
  xAxisParams.setAxisLabelsFontRelSize(20);
  yAxisParams.setAxisLabelsFontMinSize(12);
  yAxisParams.setAxisLabelsFontRelSize(20);

  _view.params()->setAxisParams( 0, xAxisParams );
  _view.params()->setAxisParams( 1, yAxisParams );

#endif
  _view.params()->setLegendFontRelSize(20);
  _view.params()->setLegendTitleFontRelSize(24);
  _view.params()->setLegendTitleText(i18n("Legend"));

  _view.params()->setAxisShowGrid(0,m_config_f.isChartGridLines());
  _view.params()->setAxisShowGrid(1,m_config_f.isChartGridLines());
  _view.params()->setPrintDataValues(m_config_f.isChartDataLabels());

  // whether to limit the chart to use series totals only.  Used for reports which only
  // show one dimension (pie).
  bool seriesTotals = false;

  // whether series (rows) are accounts (true) or months (false). This causes a lot
  // of complexity in the charts.  The problem is that circular reports work best with
  // an account in a COLUMN, while line/bar prefer it in a ROW.
  bool accountSeries = true;

  //what values should be shown
  bool showBudget = m_config_f.hasBudget();
  bool showForecast = m_config_f.isIncludingForecast();
  bool showActual = false;
  if( (m_config_f.isIncludingBudgetActuals()) || ( !showBudget && !showForecast) )
    showActual = true;

  _view.params()->setLineWidth( m_config_f.chartLineWidth() );

  switch( m_config_f.chartType() )
  {
  case MyMoneyReport::eChartNone:
  case MyMoneyReport::eChartEnd:
  case MyMoneyReport::eChartLine:
    _view.params()->setChartType( KDChartParams::Line );
    _view.params()->setAxisDatasets( 0,0 );
    break;
  case MyMoneyReport::eChartBar:
    _view.params()->setChartType( KDChartParams::Bar );
    _view.params()->setBarChartSubType( KDChartParams::BarNormal );
    break;
  case MyMoneyReport::eChartStackedBar:
    _view.params()->setChartType( KDChartParams::Bar );
    _view.params()->setBarChartSubType( KDChartParams::BarStacked );
    break;
  case MyMoneyReport::eChartPie:
    _view.params()->setChartType( KDChartParams::Pie );
    // Charts should only be 3D if this adds any information
    _view.params()->setThreeDPies( false );
    accountSeries = false;
    seriesTotals = true;
    break;
  case MyMoneyReport::eChartRing:
    _view.params()->setChartType( KDChartParams::Ring );
    _view.params()->setRelativeRingThickness( true );
    accountSeries = false;
    break;
  }

  // For onMouseOver events, we want to activate mouse tracking
  _view.setMouseTracking( true );

  //
  // In KDChart parlance, a 'series' (or row) is an account (or accountgroup, etc)
  // and an 'item' (or column) is a month
  //
  unsigned r;
  unsigned c;
  if ( accountSeries )
  {
    r = 1;
    c = m_numColumns - 1;
  }
  else
  {
    c = 1;
    r = m_numColumns - 1;
  }
  KDChartTableData data( r,c );

  // The KReportChartView widget needs to know whether the legend
  // corresponds to rows or columns
  _view.setAccountSeries( accountSeries );

  // Set up X axis labels (ie "abscissa" to use the technical term)
  QStringList& abscissaNames = _view.abscissaNames();
  abscissaNames.clear();
  if ( accountSeries )
  {
    unsigned column = 1;
    while ( column < m_numColumns ) {
      abscissaNames += QString(m_columnHeadings[column++]).replace("&nbsp;", " ");
    }
  }
  else
  {
    // we will set these up while putting in the chart values.
  }

  switch ( m_config_f.detailLevel() )
  {
    case MyMoneyReport::eDetailNone:
    case MyMoneyReport::eDetailEnd:
    case MyMoneyReport::eDetailAll:
    {
      unsigned rowNum = 0;

      // iterate over outer groups
      PivotGrid::const_iterator it_outergroup = m_grid.begin();
      while ( it_outergroup != m_grid.end() )
      {

        // iterate over inner groups
        PivotOuterGroup::const_iterator it_innergroup = (*it_outergroup).begin();
        while ( it_innergroup != (*it_outergroup).end() )
        {
          //
          // Rows
          //
          QString innergroupdata;
          PivotInnerGroup::const_iterator it_row = (*it_innergroup).begin();
          while ( it_row != (*it_innergroup).end() )
          {
            //Do not include investments accounts in the chart because they are merely container of stock and other accounts
            if( it_row.key().accountType() != MyMoneyAccount::Investment) {
              //iterate row types
              for(unsigned i = 0; i < m_rowTypeList.size(); ++i) {
                //skip the budget difference rowset
                if(m_rowTypeList[i] != eBudgetDiff ) {
                  rowNum = drawChartRowSet(rowNum, seriesTotals, accountSeries, data, it_row.value(), m_rowTypeList[i]);

                  //only show the column type in the header if there is more than one type
                  if(m_rowTypeList.size() > 1) {
                    _view.params()->setLegendText( rowNum-1, m_columnTypeHeaderList[i] + " - " + it_row.key().name() );
                  } else {
                    _view.params()->setLegendText( rowNum-1, it_row.key().name() );
                  }
                }
              }
            }
            ++it_row;
          }
          ++it_innergroup;
        }
        ++it_outergroup;
      }
    }
    break;

    case MyMoneyReport::eDetailTop:
    {
      unsigned rowNum = 0;

      // iterate over outer groups
      PivotGrid::const_iterator it_outergroup = m_grid.begin();
      while ( it_outergroup != m_grid.end() )
      {

        // iterate over inner groups
        PivotOuterGroup::const_iterator it_innergroup = (*it_outergroup).begin();
        while ( it_innergroup != (*it_outergroup).end() )
        {
          //iterate row types
          for(unsigned i = 0; i < m_rowTypeList.size(); ++i) {
            //skip the budget difference rowset
            if(m_rowTypeList[i] != eBudgetDiff ) {
              rowNum = drawChartRowSet(rowNum, seriesTotals, accountSeries, data, (*it_innergroup).m_total, m_rowTypeList[i]);

              //only show the column type in the header if there is more than one type
              if(m_rowTypeList.size() > 1) {
                _view.params()->setLegendText( rowNum-1, m_columnTypeHeaderList[i] + " - " + it_innergroup.key() );
              } else {
                _view.params()->setLegendText( rowNum-1, it_innergroup.key() );
              }
            }
          }
          ++it_innergroup;
        }
        ++it_outergroup;
      }
    }
    break;

    case MyMoneyReport::eDetailGroup:
    {
      unsigned rowNum = 0;

      // iterate over outer groups
      PivotGrid::const_iterator it_outergroup = m_grid.begin();
      while ( it_outergroup != m_grid.end() )
      {
        //iterate row types
        for(unsigned i = 0; i < m_rowTypeList.size(); ++i) {
          //skip the budget difference rowset
          if(m_rowTypeList[i] != eBudgetDiff ) {
            rowNum = drawChartRowSet(rowNum, seriesTotals, accountSeries, data, (*it_outergroup).m_total, m_rowTypeList[i]);

            //only show the column type in the header if there is more than one type
            if(m_rowTypeList.size() > 1) {
              _view.params()->setLegendText( rowNum-1, m_columnTypeHeaderList[i] + " - " + it_outergroup.key() );
            } else {
              _view.params()->setLegendText( rowNum-1, it_outergroup.key() );
            }
          }
        }
        ++it_outergroup;
      }

      //if selected, show totals too
      if (m_config_f.isShowingRowTotals())
      {
        //iterate row types
        for(unsigned i = 0; i < m_rowTypeList.size(); ++i) {
          //skip the budget difference rowset
          if(m_rowTypeList[i] != eBudgetDiff ) {
            rowNum = drawChartRowSet(rowNum, seriesTotals, accountSeries, data, m_grid.m_total, m_rowTypeList[i]);

            //only show the column type in the header if there is more than one type
            if(m_rowTypeList.size() > 1) {
              _view.params()->setLegendText( rowNum-1, m_columnTypeHeaderList[i] + " - " + i18n("Total") );
            } else {
              _view.params()->setLegendText( rowNum-1, i18n("Total") );
            }
          }
        }
      }
    }
    break;

    case MyMoneyReport::eDetailTotal:
    {
      unsigned rowNum = 0;

      //iterate row types
      for(unsigned i = 0; i < m_rowTypeList.size(); ++i) {
        //skip the budget difference rowset
        if(m_rowTypeList[i] != eBudgetDiff ) {
          rowNum = drawChartRowSet(rowNum, seriesTotals, accountSeries, data, m_grid.m_total, m_rowTypeList[i]);

          //only show the column type in the header if there is more than one type
          if(m_rowTypeList.size() > 1) {
            _view.params()->setLegendText( rowNum-1, m_columnTypeHeaderList[i] + " - " + i18n("Total") );
          } else {
            _view.params()->setLegendText( rowNum-1, i18n("Total") );
          }
        }
      }
    }
    break;
  }

  _view.setNewData(data);

  // make sure to show only the required number of fractional digits on the labels of the graph
  _view.params()->setDataValuesCalc(0, MyMoneyMoney::denomToPrec(MyMoneyFile::instance()->baseCurrency().smallestAccountFraction()));
  _view.refreshLabels();

#if 0
  // I have not been able to get this to work (ace)

  //
  // Set line to dashed for the future
  //

  if ( accountSeries )
  {
    // the first column of report which represents a date in the future, or one past the
    // last column if all columns are in the present day. Only relevant when accountSeries==true
    unsigned futurecolumn = columnValue(QDate::currentDate()) - columnValue(m_beginDate) + 1;

    // kDebug(2) << "futurecolumn: " << futurecolumn;
    // kDebug(2) << "m_numColumns: " << m_numColumns;

    // Properties for line charts whose values are in the future.
    KDChartPropertySet propSetFutureValue("future value", KDChartParams::KDCHART_PROPSET_NORMAL_DATA);
    propSetFutureValue.setLineStyle(KDChartPropertySet::OwnID, Qt::DotLine);
    const int idPropFutureValue = _view.params()->registerProperties(propSetFutureValue);

    for(int col = futurecolumn; col < m_numColumns; ++col) {
      _view.setProperty(0, col, idPropFutureValue);
    }

  }
#endif
}
#else
void PivotTable::drawChart( KReportChartView& ) const { }
#endif

unsigned PivotTable::drawChartRowSet(unsigned rowNum, const bool seriesTotals, const bool accountSeries, KDChartTableData& data, const PivotGridRowSet& rowSet, const ERowType rowType ) const
{
  //only add a row if one has been added before
  // TODO: This is inefficient. Really we should total up how many rows
  // there will be and allocate it all at once.
  if(rowNum > 0) {
    if ( accountSeries )
      data.expand( rowNum+1, m_numColumns-1 );
    else
      data.expand( m_numColumns-1, rowNum+1 );
  }

  // Columns
  if ( seriesTotals )
  {
    if ( accountSeries )
      data.setCell( rowNum, 0, rowSet[rowType].m_total.toDouble() );
    else
      data.setCell( 0, rowNum, rowSet[rowType].m_total.toDouble() );
  }
  else
  {
    unsigned column = 1;
    while ( column < m_numColumns )
    {
      if ( accountSeries )
        data.setCell( rowNum, column-1, rowSet[rowType][column].toDouble() );
      else
        data.setCell( column-1, rowNum, rowSet[rowType][column].toDouble() );
      ++column;
    }
  }

  return ++rowNum;
}
#endif

QString PivotTable::coloredAmount(const MyMoneyMoney& amount, const QString& currencySymbol, int prec) const
{
  QString result;
  if( amount.isNegative() )
    result += QString("<font color=\"rgb(%1,%2,%3)\">")
        .arg(KMyMoneyGlobalSettings::listNegativeValueColor().red())
        .arg(KMyMoneyGlobalSettings::listNegativeValueColor().green())
        .arg(KMyMoneyGlobalSettings::listNegativeValueColor().blue());
  result += amount.formatMoney(currencySymbol, prec);
  if( amount.isNegative() )
    result += QString("</font>");
  return result;
}

void PivotTable::calculateBudgetDiff(void)
{
  PivotGrid::iterator it_outergroup = m_grid.begin();
  while ( it_outergroup != m_grid.end() )
  {
    PivotOuterGroup::iterator it_innergroup = (*it_outergroup).begin();
    while ( it_innergroup != (*it_outergroup).end() )
    {
      PivotInnerGroup::iterator it_row = (*it_innergroup).begin();
      while ( it_row != (*it_innergroup).end() )
      {
        unsigned column = 1;
        switch( it_row.key().accountGroup() )
        {
          case MyMoneyAccount::Income:
          case MyMoneyAccount::Asset:
            while ( column < m_numColumns ) {
              it_row.value()[eBudgetDiff][column] = it_row.value()[eActual][column] - it_row.value()[eBudget][column];
              ++column;
            }
            break;
          case MyMoneyAccount::Expense:
          case MyMoneyAccount::Liability:
            while ( column < m_numColumns ) {
              it_row.value()[eBudgetDiff][column] = it_row.value()[eBudget][column] - it_row.value()[eActual][column];
              ++column;
            }
            break;
          default:
            break;
        }
        ++it_row;
      }
      ++it_innergroup;
    }
    ++it_outergroup;
  }

}

void PivotTable::calculateForecast(void)
{
  //setup forecast
  MyMoneyForecast forecast;

  //setup forecast settings

  //since this is a net worth forecast we want to include all account even those that are not in use
  forecast.setIncludeUnusedAccounts(true);

  //setup forecast dates
  if(m_endDate > QDate::currentDate()) {
    forecast.setForecastEndDate(m_endDate);
    forecast.setForecastStartDate(QDate::currentDate());
    forecast.setForecastDays(QDate::currentDate().daysTo(m_endDate));
  } else {
    forecast.setForecastStartDate(m_beginDate);
    forecast.setForecastEndDate(m_endDate);
    forecast.setForecastDays(m_beginDate.daysTo(m_endDate) + 1);
  }

  //adjust history dates if beginning date is before today
  if(m_beginDate < QDate::currentDate()) {
    forecast.setHistoryEndDate(m_beginDate.addDays(-1));
    forecast.setHistoryStartDate(forecast.historyEndDate().addDays(-forecast.accountsCycle()*forecast.forecastCycles()));
  }

  //run forecast
  forecast.doForecast();

  //go through the data and add forecast
  PivotGrid::iterator it_outergroup = m_grid.begin();
  while ( it_outergroup != m_grid.end() )
  {
    PivotOuterGroup::iterator it_innergroup = (*it_outergroup).begin();
    while ( it_innergroup != (*it_outergroup).end() )
    {
      PivotInnerGroup::iterator it_row = (*it_innergroup).begin();
      while ( it_row != (*it_innergroup).end() )
      {
        unsigned column = 1;
        QDate forecastDate = m_beginDate;
        //check whether columns are days or months
        if(m_config_f.isColumnsAreDays())
        {
          while(column < m_numColumns) {
            it_row.value()[eForecast][column] = forecast.forecastBalance(it_row.key(), forecastDate);

            forecastDate = forecastDate.addDays(1);
            ++column;
          }
        } else {
          //if columns are months
          while(column < m_numColumns) {
            //set forecastDate to last day of each month
            //TODO we really need a date manipulation util
            forecastDate = QDate(forecastDate.year(), forecastDate.month(), forecastDate.daysInMonth());
            //check that forecastDate is not over ending date
            if(forecastDate > m_endDate)
              forecastDate = m_endDate;

            //get forecast balance and set the corresponding column
            it_row.value()[eForecast][column] = forecast.forecastBalance(it_row.key(), forecastDate);

            forecastDate = forecastDate.addDays(1);
            ++column;
          }
        }
        ++it_row;
      }
      ++it_innergroup;
    }
    ++it_outergroup;
  }
}

void PivotTable::loadRowTypeList()
{
  if( (m_config_f.isIncludingBudgetActuals()) ||
       ( !m_config_f.hasBudget()
       && !m_config_f.isIncludingForecast()
       && !m_config_f.isIncludingMovingAverage()
       && !m_config_f.isIncludingPrice()
       && !m_config_f.isIncludingAveragePrice())
     ) {
    m_rowTypeList.append(eActual);
    m_columnTypeHeaderList.append(i18n("Actual"));
  }

  if (m_config_f.hasBudget()) {
    m_rowTypeList.append(eBudget);
    m_columnTypeHeaderList.append(i18n("Budget"));
  }

  if(m_config_f.isIncludingBudgetActuals()) {
    m_rowTypeList.append(eBudgetDiff);
    m_columnTypeHeaderList.append(i18n("Difference"));
  }

  if(m_config_f.isIncludingForecast()) {
    m_rowTypeList.append(eForecast);
    m_columnTypeHeaderList.append(i18n("Forecast"));
  }

  if(m_config_f.isIncludingMovingAverage()) {
    m_rowTypeList.append(eAverage);
    m_columnTypeHeaderList.append(i18n("Moving Average"));
  }

  if(m_config_f.isIncludingAveragePrice()) {
    m_rowTypeList.append(eAverage);
    m_columnTypeHeaderList.append(i18n("Moving Average Price"));
  }

  if(m_config_f.isIncludingPrice()) {
    m_rowTypeList.append(ePrice);
    m_columnTypeHeaderList.append(i18n("Price"));
  }
}


void PivotTable::calculateMovingAverage (void)
{
  int delta = m_config_f.movingAverageDays()/2;

  //go through the data and add the moving average
  PivotGrid::iterator it_outergroup = m_grid.begin();
  while ( it_outergroup != m_grid.end() )
  {
    PivotOuterGroup::iterator it_innergroup = (*it_outergroup).begin();
    while ( it_innergroup != (*it_outergroup).end() )
    {
      PivotInnerGroup::iterator it_row = (*it_innergroup).begin();
      while ( it_row != (*it_innergroup).end() )
      {
        unsigned column = 1;

        //check whether columns are days or months
        if(m_config_f.columnType() == MyMoneyReport::eDays) {
          while(column < m_numColumns) {
            MyMoneyMoney totalPrice = MyMoneyMoney( 0, 1 );

            QDate averageStart = columnDate(column).addDays(-delta);
            QDate averageEnd = columnDate(column).addDays(delta);
            for(QDate averageDate = averageStart; averageDate <= averageEnd; averageDate = averageDate.addDays(1)) {
              if(m_config_f.isConvertCurrency()) {
                totalPrice += it_row.key().deepCurrencyPrice(averageDate) * it_row.key().baseCurrencyPrice(averageDate);
              } else {
                totalPrice += it_row.key().deepCurrencyPrice(averageDate);
              }
              totalPrice = totalPrice.convert(10000);
            }

            //calculate the average price
            MyMoneyMoney averagePrice = totalPrice / MyMoneyMoney ((averageStart.daysTo(averageEnd) + 1), 1);

            //get the actual value, multiply by the average price and save that value
            MyMoneyMoney averageValue = it_row.value()[eActual][column] * averagePrice;
            it_row.value()[eAverage][column] = averageValue.convert(10000);

            ++column;
          }
        } else {
          //if columns are months
          while(column < m_numColumns) {
            QDate averageStart = columnDate(column);

            //set the right start date depending on the column type
            switch(m_config_f.columnType()) {
              case MyMoneyReport::eYears:
              {
                averageStart = QDate(columnDate(column).year(), 1, 1);
                break;
              }
              case MyMoneyReport::eBiMonths:
              {
                averageStart = QDate(columnDate(column).year(), columnDate(column).month(), 1).addMonths(-1);
                break;
              }
              case MyMoneyReport::eQuarters:
              {
                averageStart = QDate(columnDate(column).year(), columnDate(column).month(), 1).addMonths(-1);
                break;
              }
              case MyMoneyReport::eMonths:
              {
                averageStart = QDate(columnDate(column).year(), columnDate(column).month(), 1);
                break;
              }
              case MyMoneyReport::eWeeks:
              {
                averageStart = columnDate(column).addDays(-columnDate(column).dayOfWeek() + 1);
                break;
              }
              default:
                break;
            }

            //gather the actual data and calculate the average
            MyMoneyMoney totalPrice = MyMoneyMoney(0, 1);
            QDate averageEnd = columnDate(column);
            for(QDate averageDate = averageStart; averageDate <= averageEnd; averageDate = averageDate.addDays(1)) {
              if(m_config_f.isConvertCurrency()) {
                totalPrice += it_row.key().deepCurrencyPrice(averageDate) * it_row.key().baseCurrencyPrice(averageDate);
              } else {
                totalPrice += it_row.key().deepCurrencyPrice(averageDate);
              }
              totalPrice = totalPrice.convert(10000);
            }

            MyMoneyMoney averagePrice = totalPrice / MyMoneyMoney ((averageStart.daysTo(averageEnd) + 1), 1);
            MyMoneyMoney averageValue = it_row.value()[eActual][column] * averagePrice;

            //fill in the average
            it_row.value()[eAverage][column] = averageValue.convert(10000);

            ++column;
          }
        }
        ++it_row;
      }
      ++it_innergroup;
    }
    ++it_outergroup;
  }
}

void PivotTable::fillBasePriceUnit(ERowType rowType)
{
  //go through the data and add forecast
  PivotGrid::iterator it_outergroup = m_grid.begin();
  while ( it_outergroup != m_grid.end() )
  {
    PivotOuterGroup::iterator it_innergroup = ( *it_outergroup ).begin();
    while ( it_innergroup != ( *it_outergroup ).end() )
    {
      PivotInnerGroup::iterator it_row = ( *it_innergroup ).begin();
      while ( it_row != ( *it_innergroup ).end() )
      {
        unsigned column = 1;
        while ( column < m_numColumns ) {
          //insert a unit of currency for each account
          it_row.value() [rowType][column] = MyMoneyMoney ( 1, 1 );
          ++column;
        }
        ++it_row;
      }
      ++it_innergroup;
    }
    ++it_outergroup;
  }
}


} // namespace
// vim:cin:si:ai:et:ts=2:sw=2:
