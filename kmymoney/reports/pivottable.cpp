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

#include "pivottable.h"

// ----------------------------------------------------------------------------
// QT Includes
#include <QRegExp>
#include <QFile>
#include <QTextStream>
#include <QList>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes
#include "pivotgrid.h"
#include "reportdebug.h"
#include "kreportchartview.h"
#include "kmymoneyglobalsettings.h"
#include "kmymoneyutils.h"
#include "mymoneyforecast.h"
#include "mymoneyprice.h"
#include "mymoneyfile.h"
#include "mymoneysecurity.h"
#include "mymoneybudget.h"
#include "mymoneyreport.h"
#include "mymoneyschedule.h"
#include "mymoneysplit.h"
#include "mymoneytransaction.h"
#include "mymoneyexception.h"
#include "mymoneyenums.h"

namespace KChart { class Widget; }

namespace reports
{

using KChart::Widget;

QString Debug::m_sTabs;
bool Debug::m_sEnabled = DEBUG_ENABLED_BY_DEFAULT;
QString Debug::m_sEnableKey;

Debug::Debug(const QString& _name): m_methodName(_name), m_enabled(m_sEnabled)
{
  if (!m_enabled && _name == m_sEnableKey)
    m_enabled = true;

  if (m_enabled) {
    qDebug("%s%s(): ENTER", qPrintable(m_sTabs), qPrintable(m_methodName));
    m_sTabs.append("--");
  }
}

Debug::~Debug()
{
  if (m_enabled) {
    m_sTabs.remove(0, 2);
    qDebug("%s%s(): EXIT", qPrintable(m_sTabs), qPrintable(m_methodName));

    if (m_methodName == m_sEnableKey)
      m_enabled = false;
  }
}

void Debug::output(const QString& _text)
{
  if (m_enabled)
    qDebug("%s%s(): %s", qPrintable(m_sTabs), qPrintable(m_methodName), qPrintable(_text));
}

PivotTable::PivotTable(const MyMoneyReport& _report):
    ReportTable(_report),
    m_runningSumsCalculated(false)
{
  init();
}

void PivotTable::init()
{
  DEBUG_ENTER(Q_FUNC_INFO);

  //
  // Initialize locals
  //

  MyMoneyFile* file = MyMoneyFile::instance();

  //
  // Initialize member variables
  //

  //make sure we have all subaccounts of investment accounts
  includeInvestmentSubAccounts();

  m_config.validDateRange(m_beginDate, m_endDate);

  // If we need to calculate running sums, it does not make sense
  // to show a row total column
  if (m_config.isRunningSum())
    m_config.setShowingRowTotals(false);

  if (m_config.isRunningSum() &&
      !m_config.isIncludingPrice() &&
      !m_config.isIncludingAveragePrice() &&
      !m_config.isIncludingMovingAverage())
    m_startColumn = 1;
  else
    m_startColumn = 0;

  m_numColumns = columnValue(m_endDate) - columnValue(m_beginDate) + 1 + m_startColumn; // 1 for m_beginDate values and m_startColumn for opening balance values

  //Load what types of row the report is going to show
  loadRowTypeList();

  //
  // Initialize outer groups of the grid
  //
  if (m_config.rowType() == MyMoneyReport::eAssetLiability) {
    m_grid.insert(MyMoneyAccount::accountTypeToString(eMyMoney::Account::Type::Asset), PivotOuterGroup(m_numColumns));
    m_grid.insert(MyMoneyAccount::accountTypeToString(eMyMoney::Account::Type::Liability), PivotOuterGroup(m_numColumns, PivotOuterGroup::m_kDefaultSortOrder, true /* inverted */));
  } else {
    m_grid.insert(MyMoneyAccount::accountTypeToString(eMyMoney::Account::Type::Income), PivotOuterGroup(m_numColumns, PivotOuterGroup::m_kDefaultSortOrder - 2));
    m_grid.insert(MyMoneyAccount::accountTypeToString(eMyMoney::Account::Type::Expense), PivotOuterGroup(m_numColumns, PivotOuterGroup::m_kDefaultSortOrder - 1, true /* inverted */));
    //
    // Create rows for income/expense reports with all accounts included
    //
    if (m_config.isIncludingUnusedAccounts())
      createAccountRows();
  }

  //
  // Initialize grid totals
  //

  m_grid.m_total = PivotGridRowSet(m_numColumns);

  //
  // Get opening balances
  // Only net worth report qualifies
  if (m_startColumn == 1)
    calculateOpeningBalances();

  //
  // Calculate budget mapping
  // (for budget reports only)
  //
  if (m_config.hasBudget())
    calculateBudgetMapping();

  // prices report doesn't need transactions, but it needs account stub
  // otherwise fillBasePriceUnit won't do nothing
  if (m_config.isIncludingPrice() ||
      m_config.isIncludingAveragePrice()) {
    QList<MyMoneyAccount> accounts;
    file->accountList(accounts);
    foreach (const auto acc, accounts) {
      if (acc.isInvest()) {
        const ReportAccount repAcc(acc);
        if (m_config.includes(repAcc)) {
          const auto outergroup = acc.accountTypeToString(acc.accountType());
          assignCell(outergroup, repAcc, 0, MyMoneyMoney(), false, false);    // add account stub
        }
      }
    }
  } else {
    //
    // Populate all transactions into the row/column pivot grid
    //

    QList<MyMoneyTransaction> transactions;
    m_config.setReportAllSplits(false);
    m_config.setConsiderCategory(true);
    try {
      transactions = file->transactionList(m_config);
    } catch (const MyMoneyException &e) {
      qDebug("ERR: %s thrown in %s(%ld)", qPrintable(e.what()), qPrintable(e.file()), e.line());
      throw e;
    }
    DEBUG_OUTPUT(QString("Found %1 matching transactions").arg(transactions.count()));


    // Include scheduled transactions if required
    if (m_config.isIncludingSchedules()) {
      // Create a custom version of the report filter, excluding date
      // We'll use this to compare the transaction against
      MyMoneyTransactionFilter schedulefilter(m_config);
      schedulefilter.setDateFilter(QDate(), QDate());

      // Get the real dates from the config filter
      QDate configbegin, configend;
      m_config.validDateRange(configbegin, configend);

      QList<MyMoneySchedule> schedules = file->scheduleList();
      QList<MyMoneySchedule>::const_iterator it_schedule = schedules.constBegin();
      while (it_schedule != schedules.constEnd()) {
        // If the transaction meets the filter
        MyMoneyTransaction tx = (*it_schedule).transaction();
        if (!(*it_schedule).isFinished() && schedulefilter.match(tx)) {
          // Keep the id of the schedule with the transaction so that
          // we can do the autocalc later on in case of a loan payment
          tx.setValue("kmm-schedule-id", (*it_schedule).id());

          // Get the dates when a payment will be made within the report window
          QDate nextpayment = (*it_schedule).adjustedNextPayment(configbegin);
          if (nextpayment.isValid()) {
            // Add one transaction for each date
            QList<QDate> paymentDates = (*it_schedule).paymentDates(nextpayment, configend);
            QList<QDate>::const_iterator it_date = paymentDates.constBegin();
            while (it_date != paymentDates.constEnd()) {
              //if the payment occurs in the past, enter it tomorrow
              if (QDate::currentDate() >= *it_date) {
                tx.setPostDate(QDate::currentDate().addDays(1));
              } else {
                tx.setPostDate(*it_date);
              }
              if (tx.postDate() <= configend
                  && tx.postDate() >= configbegin) {
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
    bool al_transfers = (m_config.rowType() == MyMoneyReport::eExpenseIncome) && (m_config.isIncludingTransfers());

    //this is to store balance for loan accounts when not included in the report
    QMap<QString, MyMoneyMoney> loanBalances;

    QList<MyMoneyTransaction>::const_iterator it_transaction = transactions.constBegin();
    int colofs = columnValue(m_beginDate) - m_startColumn;
    while (it_transaction != transactions.constEnd()) {
      MyMoneyTransaction tx = (*it_transaction);
      QDate postdate = tx.postDate();
      if (postdate < m_beginDate) {
        qDebug("MyMoneyFile::transactionList returned a transaction that is outside the date filter, skipping it");
        ++it_transaction;
        continue;
      }
      int column = columnValue(postdate) - colofs;

      // check if we need to call the autocalculation routine
      if (tx.isLoanPayment() && tx.hasAutoCalcSplit() && (tx.value("kmm-schedule-id").length() > 0)) {
        // make sure to consider any autocalculation for loan payments
        MyMoneySchedule sched = file->schedule(tx.value("kmm-schedule-id"));
        const MyMoneySplit& split = tx.amortizationSplit();
        if (!split.id().isEmpty()) {
          ReportAccount splitAccount(file->account(split.accountId()));
          eMyMoney::Account::Type type = splitAccount.accountGroup();
          QString outergroup = MyMoneyAccount::accountTypeToString(type);

          //if the account is included in the report, calculate the balance from the cells
          if (m_config.includes(splitAccount)) {
            loanBalances[splitAccount.id()] = cellBalance(outergroup, splitAccount, column, false);
          } else {
            //if it is not in the report and also not in loanBalances, get the balance from the file
            if (!loanBalances.contains(splitAccount.id())) {
              QDate dueDate = sched.nextDueDate();

              //if the payment is overdue, use current date
              if (dueDate < QDate::currentDate())
                dueDate = QDate::currentDate();

              //get the balance from the file for the date
              loanBalances[splitAccount.id()] = file->balance(splitAccount.id(), dueDate.addDays(-1));
            }
          }

          KMyMoneyUtils::calculateAutoLoan(sched, tx, loanBalances);

          //if the loan split is not included in the report, update the balance for the next occurrence
          if (!m_config.includes(splitAccount)) {
            foreach (const auto split, tx.splits()) {
              if (split.isAmortizationSplit() && split.accountId() == splitAccount.id())
                loanBalances[splitAccount.id()] = loanBalances[splitAccount.id()] + split.shares();
            }
          }
        }
      }

      QList<MyMoneySplit> splits = tx.splits();
      QList<MyMoneySplit>::const_iterator it_split = splits.constBegin();
      while (it_split != splits.constEnd()) {
        ReportAccount splitAccount((*it_split).accountId());

        // Each split must be further filtered, because if even one split matches,
        // the ENTIRE transaction is returned with all splits (even non-matching ones)
        if (m_config.includes(splitAccount) && m_config.match(&(*it_split))) {
          // reverse sign to match common notation for cash flow direction, only for expense/income splits
          MyMoneyMoney reverse(splitAccount.isIncomeExpense() ? -1 : 1, 1);

          MyMoneyMoney value;
          // the outer group is the account class (major account type)
          eMyMoney::Account::Type type = splitAccount.accountGroup();
          QString outergroup = MyMoneyAccount::accountTypeToString(type);

          value = (*it_split).shares();
          bool stockSplit = tx.isStockSplit();
          if (!stockSplit) {
            // retrieve the value in the account's underlying currency
            if (value != MyMoneyMoney::autoCalc) {
              value = value * reverse;
            } else {
              qDebug("PivotTable::PivotTable(): This must not happen");
              value = MyMoneyMoney();  // keep it 0 so far
            }

            // Except in the case of transfers on an income/expense report
            if (al_transfers && (type == eMyMoney::Account::Type::Asset || type == eMyMoney::Account::Type::Liability)) {
              outergroup = i18n("Transfers");
              value = -value;
            }
          }
          // add the value to its correct position in the pivot table
          assignCell(outergroup, splitAccount, column, value, false, stockSplit);
        }
        ++it_split;
      }

      ++it_transaction;
    }
  }
  //
  // Get forecast data
  //
  if (m_config.isIncludingForecast())
    calculateForecast();

  //
  //Insert Price data
  //
  if (m_config.isIncludingPrice())
    fillBasePriceUnit(ePrice);

  //
  //Insert Average Price data
  //
  if (m_config.isIncludingAveragePrice()) {
    fillBasePriceUnit(eActual);
    calculateMovingAverage();
  }

  //
  // Collapse columns to match column type
  //


  if (m_config.columnPitch() > 1)
    collapseColumns();

  //
  // Calculate the running sums
  // (for running sum reports only)
  //

  if (m_config.isRunningSum())
    calculateRunningSums();

  //
  // Calculate Moving Average
  //
  if (m_config.isIncludingMovingAverage())
    calculateMovingAverage();

  //
  // Calculate Budget Difference
  //

  if (m_config.isIncludingBudgetActuals())
    calculateBudgetDiff();

  //
  // Convert all values to the deep currency
  //

  convertToDeepCurrency();

  //
  // Convert all values to the base currency
  //

  if (m_config.isConvertCurrency())
    convertToBaseCurrency();

  //
  // Determine column headings
  //

  calculateColumnHeadings();

  //
  // Calculate row and column totals
  //

  calculateTotals();

  //
  // If using mixed time, calculate column for current date
  //
  m_config.setCurrentDateColumn(currentDateColumn());
}

void PivotTable::collapseColumns()
{
  DEBUG_ENTER(Q_FUNC_INFO);

  int columnpitch = m_config.columnPitch();
  if (columnpitch != 1) {
    int sourcemonth = (m_config.isColumnsAreDays())
                      // use the user's locale to determine the week's start
                      ? (m_beginDate.dayOfWeek() + 8 - QLocale().firstDayOfWeek()) % 7
                      : m_beginDate.month();
    int sourcecolumn = m_startColumn;
    int destcolumn = m_startColumn;
    while (sourcecolumn < m_numColumns) {
      if (sourcecolumn != destcolumn) {
#if 0
        // TODO: Clean up this rather inefficient kludge. We really should jump by an entire
        // destcolumn at a time on RS reports, and calculate the proper sourcecolumn to use,
        // allowing us to clear and accumulate only ONCE per destcolumn
        if (m_config_f.isRunningSum())
          clearColumn(destcolumn);
#endif
        accumulateColumn(destcolumn, sourcecolumn);
      }

      if (++sourcecolumn < m_numColumns) {
        if ((sourcemonth++ % columnpitch) == 0) {
          if (sourcecolumn != ++destcolumn)
            clearColumn(destcolumn);
        }
      }
    }
    m_numColumns = destcolumn + 1;
  }
}

void PivotTable::accumulateColumn(int destcolumn, int sourcecolumn)
{
  DEBUG_ENTER(Q_FUNC_INFO);
  DEBUG_OUTPUT(QString("From Column %1 to %2").arg(sourcecolumn).arg(destcolumn));

  // iterate over outer groups
  PivotGrid::iterator it_outergroup = m_grid.begin();
  while (it_outergroup != m_grid.end()) {
    // iterate over inner groups
    PivotOuterGroup::iterator it_innergroup = (*it_outergroup).begin();
    while (it_innergroup != (*it_outergroup).end()) {
      // iterator over rows
      PivotInnerGroup::iterator it_row = (*it_innergroup).begin();
      while (it_row != (*it_innergroup).end()) {
        if ((*it_row)[eActual].count() <= sourcecolumn)
          throw MYMONEYEXCEPTION(QString("Sourcecolumn %1 out of grid range (%2) in PivotTable::accumulateColumn").arg(sourcecolumn).arg((*it_row)[eActual].count()));
        if ((*it_row)[eActual].count() <= destcolumn)
          throw MYMONEYEXCEPTION(QString("Destcolumn %1 out of grid range (%2) in PivotTable::accumulateColumn").arg(sourcecolumn).arg((*it_row)[eActual].count()));

        (*it_row)[eActual][destcolumn] += (*it_row)[eActual][sourcecolumn];
        ++it_row;
      }

      ++it_innergroup;
    }
    ++it_outergroup;
  }
}

void PivotTable::clearColumn(int column)
{
  DEBUG_ENTER(Q_FUNC_INFO);
  DEBUG_OUTPUT(QString("Column %1").arg(column));

  // iterate over outer groups
  PivotGrid::iterator it_outergroup = m_grid.begin();
  while (it_outergroup != m_grid.end()) {
    // iterate over inner groups
    PivotOuterGroup::iterator it_innergroup = (*it_outergroup).begin();
    while (it_innergroup != (*it_outergroup).end()) {
      // iterator over rows
      PivotInnerGroup::iterator it_row = (*it_innergroup).begin();
      while (it_row != (*it_innergroup).end()) {
        if ((*it_row)[eActual].count() <= column)
          throw MYMONEYEXCEPTION(QString("Column %1 out of grid range (%2) in PivotTable::accumulateColumn").arg(column).arg((*it_row)[eActual].count()));

        (*it_row++)[eActual][column] = PivotCell();
      }

      ++it_innergroup;
    }
    ++it_outergroup;
  }
}

void PivotTable::calculateColumnHeadings()
{
  DEBUG_ENTER(Q_FUNC_INFO);

  // one column for the opening balance
  if (m_startColumn == 1)
    m_columnHeadings.append("Opening");

  int columnpitch = m_config.columnPitch();

  if (columnpitch == 0) {
    // output the warning but don't crash by dividing with 0
    qWarning("PivotTable::calculateColumnHeadings() Invalid column pitch");
    return;
  }

  // if this is a days-based report
  if (m_config.isColumnsAreDays()) {
    if (columnpitch == 1) {
      QDate columnDate = m_beginDate;
      int column = m_startColumn;
      while (column++ < m_numColumns) {
        QString heading = QLocale().monthName(columnDate.month(), QLocale::ShortFormat) + ' ' + QString::number(columnDate.day());
        columnDate = columnDate.addDays(1);
        m_columnHeadings.append(heading);
      }
    } else {
      QDate day = m_beginDate;
      QDate prv = m_beginDate;

      // use the user's locale to determine the week's start
      int dow = (day.dayOfWeek() + 8 - QLocale().firstDayOfWeek()) % 7;

      while (day <= m_endDate) {
        if (((dow % columnpitch) == 0) || (day == m_endDate)) {
          m_columnHeadings.append(QString("%1&nbsp;%2 - %3&nbsp;%4")
                                  .arg(QLocale().monthName(prv.month(), QLocale::ShortFormat))
                                  .arg(prv.day())
                                  .arg(QLocale().monthName(day.month(), QLocale::ShortFormat))
                                  .arg(day.day()));
          prv = day.addDays(1);
        }
        day = day.addDays(1);
        dow++;
      }
    }
  }

  // else it's a months-based report
  else {
    if (columnpitch == 12) {
      int year = m_beginDate.year();
      int column = m_startColumn;
      while (column++ < m_numColumns)
        m_columnHeadings.append(QString::number(year++));
    } else {
      int year = m_beginDate.year();
      bool includeyear = (m_beginDate.year() != m_endDate.year());
      int segment = (m_beginDate.month() - 1) / columnpitch;
      int column = m_startColumn;
      while (column++ < m_numColumns) {
        QString heading = QLocale().monthName(1 + segment * columnpitch, QLocale::ShortFormat);
        if (columnpitch != 1)
          heading += '-' + QLocale().monthName((1 + segment) * columnpitch, QLocale::ShortFormat);
        if (includeyear)
          heading += ' ' + QString::number(year);
        m_columnHeadings.append(heading);
        if (++segment >= 12 / columnpitch) {
          segment -= 12 / columnpitch;
          ++year;
        }
      }
    }
  }
}

void PivotTable::createAccountRows()
{
  DEBUG_ENTER(Q_FUNC_INFO);
  MyMoneyFile* file = MyMoneyFile::instance();

  QList<MyMoneyAccount> accounts;
  file->accountList(accounts);

  QList<MyMoneyAccount>::const_iterator it_account = accounts.constBegin();

  while (it_account != accounts.constEnd()) {
    ReportAccount account(*it_account);

    // only include this item if its account group is included in this report
    // and if the report includes this account
    if (m_config.includes(*it_account)) {
      DEBUG_OUTPUT(QString("Includes account %1").arg(account.name()));

      // the row group is the account class (major account type)
      QString outergroup = MyMoneyAccount::accountTypeToString(account.accountGroup());
      // place into the 'opening' column...
      assignCell(outergroup, account, 0, MyMoneyMoney());
    }
    ++it_account;
  }
}

void PivotTable::calculateOpeningBalances()
{
  DEBUG_ENTER(Q_FUNC_INFO);

  // First, determine the inclusive dates of the report.  Normally, that's just
  // the begin & end dates of m_config_f.  However, if either of those dates are
  // blank, we need to use m_beginDate and/or m_endDate instead.
  QDate from = m_config.fromDate();
  QDate to = m_config.toDate();
  if (! from.isValid())
    from = m_beginDate;
  if (! to.isValid())
    to = m_endDate;

  MyMoneyFile* file = MyMoneyFile::instance();

  QList<MyMoneyAccount> accounts;
  file->accountList(accounts);

  QList<MyMoneyAccount>::const_iterator it_account = accounts.constBegin();

  while (it_account != accounts.constEnd()) {
    ReportAccount account(*it_account);

    // only include this item if its account group is included in this report
    // and if the report includes this account
    if (m_config.includes(*it_account)) {

      //do not include account if it is closed and it has no transactions in the report period
      if (account.isClosed()) {
        //check if the account has transactions for the report timeframe
        MyMoneyTransactionFilter filter;
        filter.addAccount(account.id());
        filter.setDateFilter(m_beginDate, m_endDate);
        filter.setReportAllSplits(false);
        QList<MyMoneyTransaction> transactions = file->transactionList(filter);
        //if a closed account has no transactions in that timeframe, do not include it
        if (transactions.size() == 0) {
          DEBUG_OUTPUT(QString("DOES NOT INCLUDE account %1").arg(account.name()));
          ++it_account;
          continue;
        }
      }

      DEBUG_OUTPUT(QString("Includes account %1").arg(account.name()));
      // the row group is the account class (major account type)
      QString outergroup = MyMoneyAccount::accountTypeToString(account.accountGroup());

      // extract the balance of the account for the given begin date, which is
      // the opening balance plus the sum of all transactions prior to the begin
      // date

      // this is in the underlying currency
      MyMoneyMoney value = file->balance(account.id(), from.addDays(-1));

      // place into the 'opening' column...
      assignCell(outergroup, account, 0, value);
    } else {
      DEBUG_OUTPUT(QString("DOES NOT INCLUDE account %1").arg(account.name()));
    }

    ++it_account;
  }
}

void PivotTable::calculateRunningSums(PivotInnerGroup::iterator& it_row)
{
  MyMoneyMoney runningsum = it_row.value()[eActual][0].calculateRunningSum(MyMoneyMoney());
  int column = m_startColumn;
  while (column < m_numColumns) {
    if (it_row.value()[eActual].count() <= column)
      throw MYMONEYEXCEPTION(QString("Column %1 out of grid range (%2) in PivotTable::calculateRunningSums").arg(column).arg(it_row.value()[eActual].count()));

    runningsum = it_row.value()[eActual][column].calculateRunningSum(runningsum);

    ++column;
  }
}

void PivotTable::calculateRunningSums()
{
  DEBUG_ENTER(Q_FUNC_INFO);

  m_runningSumsCalculated = true;

  PivotGrid::iterator it_outergroup = m_grid.begin();
  while (it_outergroup != m_grid.end()) {
    PivotOuterGroup::iterator it_innergroup = (*it_outergroup).begin();
    while (it_innergroup != (*it_outergroup).end()) {
      PivotInnerGroup::iterator it_row = (*it_innergroup).begin();
      while (it_row != (*it_innergroup).end()) {
#if 0
        MyMoneyMoney runningsum = it_row.value()[0];
        int column = m_startColumn;
        while (column < m_numColumns) {
          if (it_row.value()[eActual].count() <= column)
            throw MYMONEYEXCEPTION(QString("Column %1 out of grid range (%2) in PivotTable::calculateRunningSums").arg(column).arg(it_row.value()[eActual].count()));

          runningsum = (it_row.value()[eActual][column] += runningsum);

          ++column;
        }
#endif
        calculateRunningSums(it_row);
        ++it_row;
      }
      ++it_innergroup;
    }
    ++it_outergroup;
  }
}

MyMoneyMoney PivotTable::cellBalance(const QString& outergroup, const ReportAccount& _row, int _column, bool budget)
{
  if (m_runningSumsCalculated) {
    qDebug("You must not call PivotTable::cellBalance() after calling PivotTable::calculateRunningSums()");
    throw MYMONEYEXCEPTION(QString("You must not call PivotTable::cellBalance() after calling PivotTable::calculateRunningSums()"));
  }

  // for budget reports, if this is the actual value, map it to the account which
  // holds its budget
  ReportAccount row = _row;
  if (!budget && m_config.hasBudget()) {
    QString newrow = m_budgetMap[row.id()];

    // if there was no mapping found, then the budget report is not interested
    // in this account.
    if (newrow.isEmpty())
      return MyMoneyMoney();

    row = ReportAccount(newrow);
  }

  // ensure the row already exists (and its parental hierarchy)
  createRow(outergroup, row, true);

  // Determine the inner group from the top-most parent account
  QString innergroup(row.topParentName());

  if (m_numColumns <= _column)
    throw MYMONEYEXCEPTION(QString("Column %1 out of m_numColumns range (%2) in PivotTable::cellBalance").arg(_column).arg(m_numColumns));
  if (m_grid[outergroup][innergroup][row][eActual].count() <= _column)
    throw MYMONEYEXCEPTION(QString("Column %1 out of grid range (%2) in PivotTable::cellBalance").arg(_column).arg(m_grid[outergroup][innergroup][row][eActual].count()));

  MyMoneyMoney balance;
  if (budget)
    balance = m_grid[outergroup][innergroup][row][eBudget][0].cellBalance(MyMoneyMoney());
  else
    balance = m_grid[outergroup][innergroup][row][eActual][0].cellBalance(MyMoneyMoney());

  int column = m_startColumn;
  while (column < _column) {
    if (m_grid[outergroup][innergroup][row][eActual].count() <= column)
      throw MYMONEYEXCEPTION(QString("Column %1 out of grid range (%2) in PivotTable::cellBalance").arg(column).arg(m_grid[outergroup][innergroup][row][eActual].count()));

    balance = m_grid[outergroup][innergroup][row][eActual][column].cellBalance(balance);

    ++column;
  }

  return balance;
}


void PivotTable::calculateBudgetMapping()
{
  DEBUG_ENTER(Q_FUNC_INFO);

  MyMoneyFile* file = MyMoneyFile::instance();

  // Only do this if there is at least one budget in the file
  if (file->countBudgets()) {
    // Select a budget
    //
    // It will choose the first budget in the list for the start year of the report if no budget is selected
    MyMoneyBudget budget = MyMoneyBudget();
    QList<MyMoneyBudget> budgets = file->budgetList();
    bool validBudget = false;

    //check that the selected budget is valid
    if (m_config.budget() != "Any") {
      QList<MyMoneyBudget>::const_iterator budgets_it = budgets.constBegin();
      while (budgets_it != budgets.constEnd()) {
        //pick the budget by id
        if ((*budgets_it).id() == m_config.budget()) {
          budget = file->budget((*budgets_it).id());
          validBudget = true;
          break;
        }
        ++budgets_it;
      }
    }

    //if no valid budget has been selected
    if (!validBudget) {
      //if the budget list is empty, just return
      if (budgets.count() == 0) {
        return;
      }

      QList<MyMoneyBudget>::const_iterator budgets_it = budgets.constBegin();
      while (budgets_it != budgets.constEnd()) {
        //pick the first budget that matches the report start year
        if ((*budgets_it).budgetStart().year() == QDate::currentDate().year()) {
          budget = file->budget((*budgets_it).id());
          break;
        }
        ++budgets_it;
      }
      //if it can't find a matching budget, take the first one on the list
      if (budget.id().isEmpty()) {
        budget = budgets[0];
      }

      //assign the budget to the report
      m_config.setBudget(budget.id(), m_config.isIncludingBudgetActuals());
    }

    // Dump the budget
    //qDebug() << "Budget " << budget.name() << ": ";

    // Go through all accounts in the system to build the mapping
    QList<MyMoneyAccount> accounts;
    file->accountList(accounts);
    QList<MyMoneyAccount>::const_iterator it_account = accounts.constBegin();
    while (it_account != accounts.constEnd()) {
      //include only the accounts selected for the report
      if (m_config.includes(*it_account)) {
        QString id = (*it_account).id();
        QString acid = id;

        // If the budget contains this account outright
        if (budget.contains(id)) {
          // Add it to the mapping
          m_budgetMap[acid] = id;
          // qDebug() << ReportAccount(acid).debugName() << " self-maps / type =" << budget.account(id).budgetLevel();
        }
        // Otherwise, search for a parent account which includes sub-accounts
        else {
          //if includeBudgetActuals, include all accounts regardless of whether in budget or not
          if (m_config.isIncludingBudgetActuals()) {
            m_budgetMap[acid] = id;
            // qDebug() << ReportAccount(acid).debugName() << " maps to " << ReportAccount(id).debugName();
          }
          do {
            id = file->account(id).parentAccountId();
            if (budget.contains(id)) {
              if (budget.account(id).budgetSubaccounts()) {
                m_budgetMap[acid] = id;
                // qDebug() << ReportAccount(acid).debugName() << " maps to " << ReportAccount(id).debugName();
                break;
              }
            }
          } while (! id.isEmpty());
        }
      }
      ++it_account;
    } // end while looping through the accounts in the file

    // Place the budget values into the budget grid
    QList<MyMoneyBudget::AccountGroup> baccounts = budget.getaccounts();
    QList<MyMoneyBudget::AccountGroup>::const_iterator it_bacc = baccounts.constBegin();
    while (it_bacc != baccounts.constEnd()) {
      ReportAccount splitAccount((*it_bacc).id());

      //include the budget account only if it is included in the report
      if (m_config.includes(splitAccount)) {
        eMyMoney::Account::Type type = splitAccount.accountGroup();
        QString outergroup = MyMoneyAccount::accountTypeToString(type);

        // reverse sign to match common notation for cash flow direction, only for expense/income splits
        MyMoneyMoney reverse((splitAccount.accountType() == eMyMoney::Account::Type::Expense) ? -1 : 1, 1);

        const QMap<QDate, MyMoneyBudget::PeriodGroup>& periods = (*it_bacc).getPeriods();

        // skip the account if it has no periods
        if (periods.count() < 1) {
          ++it_bacc;
          continue;
        }

        MyMoneyMoney value = (*periods.begin()).amount() * reverse;
        int column = m_startColumn;

        // based on the kind of budget it is, deal accordingly
        switch ((*it_bacc).budgetLevel()) {
          case MyMoneyBudget::AccountGroup::eYearly:
            // divide the single yearly value by 12 and place it in each column
            value /= MyMoneyMoney(12, 1);
            // intentional fall through

          case MyMoneyBudget::AccountGroup::eNone:
          case MyMoneyBudget::AccountGroup::eMax:
          case MyMoneyBudget::AccountGroup::eMonthly:
            // place the single monthly value in each column of the report
            // only add the value if columns are monthly or longer
            if (m_config.columnType() == MyMoneyReport::eBiMonths
                || m_config.columnType() == MyMoneyReport::eMonths
                || m_config.columnType() == MyMoneyReport::eYears
                || m_config.columnType() == MyMoneyReport::eQuarters) {
              QDate budgetDate = budget.budgetStart();
              while (column < m_numColumns && budget.budgetStart().addYears(1) > budgetDate) {
                //only show budget values if the budget year and the column date match
                //no currency conversion is done here because that is done for all columns later
                if (budgetDate > columnDate(column)) {
                  ++column;
                } else {
                  if (budgetDate >= m_beginDate.addDays(-m_beginDate.day() + 1)
                      && budgetDate <= m_endDate.addDays(m_endDate.daysInMonth() - m_endDate.day())
                      && budgetDate > (columnDate(column).addMonths(-m_config.columnType()))) {
                    assignCell(outergroup, splitAccount, column, value, true /*budget*/);
                  }
                  budgetDate = budgetDate.addMonths(1);
                }
              }
            }
            break;
          case MyMoneyBudget::AccountGroup::eMonthByMonth:
            // place each value in the appropriate column
            // budget periods are supposed to come in order just like columns
            {
              QMap<QDate, MyMoneyBudget::PeriodGroup>::const_iterator it_period = periods.begin();
              while (it_period != periods.end() && column < m_numColumns) {
                if ((*it_period).startDate() > columnDate(column)) {
                  ++column;
                } else {
                  switch (m_config.columnType()) {
                    case MyMoneyReport::eYears:
                    case MyMoneyReport::eBiMonths:
                    case MyMoneyReport::eQuarters:
                    case MyMoneyReport::eMonths: {
                        if ((*it_period).startDate() >= m_beginDate.addDays(-m_beginDate.day() + 1)
                            && (*it_period).startDate() <= m_endDate.addDays(m_endDate.daysInMonth() - m_endDate.day())
                            && (*it_period).startDate() > (columnDate(column).addMonths(-m_config.columnType()))) {
                          //no currency conversion is done here because that is done for all columns later
                          value = (*it_period).amount() * reverse;
                          assignCell(outergroup, splitAccount, column, value, true /*budget*/);
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

void PivotTable::convertToBaseCurrency()
{
  DEBUG_ENTER(Q_FUNC_INFO);

  MyMoneyFile* file = MyMoneyFile::instance();
  int fraction = file->baseCurrency().smallestAccountFraction();
  QList<ERowType> rowTypeList = m_rowTypeList;
  rowTypeList.removeOne(eAverage);

  PivotGrid::iterator it_outergroup = m_grid.begin();
  while (it_outergroup != m_grid.end()) {
    PivotOuterGroup::iterator it_innergroup = (*it_outergroup).begin();
    while (it_innergroup != (*it_outergroup).end()) {
      PivotInnerGroup::iterator it_row = (*it_innergroup).begin();
      while (it_row != (*it_innergroup).end()) {
        auto column = 0;
        while (column < m_numColumns) {
          if (it_row.value()[eActual].count() <= column)
            throw MYMONEYEXCEPTION(QString("Column %1 out of grid range (%2) in PivotTable::convertToBaseCurrency").arg(column).arg(it_row.value()[eActual].count()));

          QDate valuedate = columnDate(column);

          //get base price for that date
          MyMoneyMoney conversionfactor = it_row.key().baseCurrencyPrice(valuedate, m_config.isSkippingZero());
          int pricePrecision;
          if (it_row.key().isInvest())
            pricePrecision = file->security(it_row.key().currencyId()).pricePrecision();
          else
            pricePrecision = MyMoneyMoney::denomToPrec(fraction);

          foreach (const auto rowType, rowTypeList) {
            //calculate base value
            MyMoneyMoney oldval = it_row.value()[rowType][column];
            MyMoneyMoney value = (oldval * conversionfactor).reduce();

            //convert to lowest fraction
            if (rowType == ePrice)
              it_row.value()[rowType][column] = PivotCell(MyMoneyMoney(value.convertPrecision(pricePrecision)));
            else
              it_row.value()[rowType][column] = PivotCell(value.convert(fraction));

            DEBUG_OUTPUT_IF(conversionfactor != MyMoneyMoney::ONE , QString("Factor of %1, value was %2, now %3").arg(conversionfactor).arg(DEBUG_SENSITIVE(oldval)).arg(DEBUG_SENSITIVE(it_row.value()[rowType][column].toDouble())));
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

void PivotTable::convertToDeepCurrency()
{
  DEBUG_ENTER(Q_FUNC_INFO);
  MyMoneyFile* file = MyMoneyFile::instance();

  PivotGrid::iterator it_outergroup = m_grid.begin();
  while (it_outergroup != m_grid.end()) {
    PivotOuterGroup::iterator it_innergroup = (*it_outergroup).begin();
    while (it_innergroup != (*it_outergroup).end()) {
      PivotInnerGroup::iterator it_row = (*it_innergroup).begin();
      while (it_row != (*it_innergroup).end()) {
        auto column = 0;
        while (column < m_numColumns) {
          if (it_row.value()[eActual].count() <= column)
            throw MYMONEYEXCEPTION(QString("Column %1 out of grid range (%2) in PivotTable::convertToDeepCurrency").arg(column).arg(it_row.value()[eActual].count()));

          QDate valuedate = columnDate(column);

          //get conversion factor for the account and date
          MyMoneyMoney conversionfactor = it_row.key().deepCurrencyPrice(valuedate, m_config.isSkippingZero());

          //use the fraction relevant to the account at hand
          int fraction = it_row.key().currency().smallestAccountFraction();

          //use base currency fraction if not initialized
          if (fraction == -1)
            fraction = file->baseCurrency().smallestAccountFraction();

          //convert to deep currency
          MyMoneyMoney oldval = it_row.value()[eActual][column];
          MyMoneyMoney value = (oldval * conversionfactor).reduce();
          //reduce to lowest fraction
          it_row.value()[eActual][column] = PivotCell(value.convert(fraction));

          //convert price data
          if (m_config.isIncludingPrice()) {
            MyMoneyMoney oldPriceVal = it_row.value()[ePrice][column];
            MyMoneyMoney priceValue = (oldPriceVal * conversionfactor).reduce();
            it_row.value()[ePrice][column] = PivotCell(priceValue.convert(10000));
          }

          DEBUG_OUTPUT_IF(conversionfactor != MyMoneyMoney::ONE , QString("Factor of %1, value was %2, now %3").arg(conversionfactor).arg(DEBUG_SENSITIVE(oldval)).arg(DEBUG_SENSITIVE(it_row.value()[eActual][column].toDouble())));

          ++column;
        }
        ++it_row;
      }
      ++it_innergroup;
    }
    ++it_outergroup;
  }
}

void PivotTable::calculateTotals()
{
  //insert the row type that is going to be used
  for (int i = 0; i < m_rowTypeList.size(); ++i) {
    for (int k = 0; k < m_numColumns; ++k) {
      m_grid.m_total[ m_rowTypeList[i] ].append(PivotCell());
    }
  }
  //
  // Outer groups
  //

  // iterate over outer groups
  PivotGrid::iterator it_outergroup = m_grid.begin();
  while (it_outergroup != m_grid.end()) {
    for (int i = 0; i < m_rowTypeList.size(); ++i) {
      for (int k = 0; k < m_numColumns; ++k) {
        (*it_outergroup).m_total[ m_rowTypeList[i] ].append(PivotCell());
      }
    }

    //
    // Inner Groups
    //

    PivotOuterGroup::iterator it_innergroup = (*it_outergroup).begin();
    while (it_innergroup != (*it_outergroup).end()) {
      for (int i = 0; i < m_rowTypeList.size(); ++i) {
        for (int k = 0; k < m_numColumns; ++k) {
          (*it_innergroup).m_total[ m_rowTypeList[i] ].append(PivotCell());
        }
      }
      //
      // Rows
      //

      PivotInnerGroup::iterator it_row = (*it_innergroup).begin();
      while (it_row != (*it_innergroup).end()) {
        //
        // Columns
        //

        auto column = 0;
        while (column < m_numColumns) {
          for (int i = 0; i < m_rowTypeList.size(); ++i) {
            if (it_row.value()[ m_rowTypeList[i] ].count() <= column)
              throw MYMONEYEXCEPTION(QString("Column %1 out of grid range (%2) in PivotTable::calculateTotals, row columns").arg(column).arg(it_row.value()[ m_rowTypeList[i] ].count()));
            if ((*it_innergroup).m_total[ m_rowTypeList[i] ].count() <= column)
              throw MYMONEYEXCEPTION(QString("Column %1 out of grid range (%2) in PivotTable::calculateTotals, inner group totals").arg(column).arg((*it_innergroup).m_total[ m_rowTypeList[i] ].count()));

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

      auto column = 0;
      while (column < m_numColumns) {
        for (int i = 0; i < m_rowTypeList.size(); ++i) {
          if ((*it_innergroup).m_total[ m_rowTypeList[i] ].count() <= column)
            throw MYMONEYEXCEPTION(QString("Column %1 out of grid range (%2) in PivotTable::calculateTotals, inner group totals").arg(column).arg((*it_innergroup).m_total[ m_rowTypeList[i] ].count()));
          if ((*it_outergroup).m_total[ m_rowTypeList[i] ].count() <= column)
            throw MYMONEYEXCEPTION(QString("Column %1 out of grid range (%2) in PivotTable::calculateTotals, outer group totals").arg(column).arg((*it_innergroup).m_total[ m_rowTypeList[i] ].count()));

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

    const bool isIncomeExpense = (m_config.rowType() == MyMoneyReport::eExpenseIncome);
    const bool invert_total = (*it_outergroup).m_inverted;
    auto column = 0;
    while (column < m_numColumns) {
      for (int i = 0; i < m_rowTypeList.size(); ++i) {
        if (m_grid.m_total[ m_rowTypeList[i] ].count() <= column)
          throw MYMONEYEXCEPTION(QString("Column %1 out of grid range (%2) in PivotTable::calculateTotals, grid totals").arg(column).arg((*it_innergroup).m_total[ m_rowTypeList[i] ].count()));

        //calculate actual totals
        MyMoneyMoney value = (*it_outergroup).m_total[ m_rowTypeList[i] ][column];
        (*it_outergroup).m_total[ m_rowTypeList[i] ].m_total += value;

        //so far the invert only applies to actual and budget
        if (invert_total && m_rowTypeList[i] != eBudgetDiff && m_rowTypeList[i] != eForecast)
          value = -value;
        // forecast income expense reports should be inverted as oposed to asset/liability reports
        if (invert_total && isIncomeExpense && m_rowTypeList[i] == eForecast)
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

  auto totalcolumn = 0;
  while (totalcolumn < m_numColumns) {
    for (int i = 0; i < m_rowTypeList.size(); ++i) {
      if (m_grid.m_total[ m_rowTypeList[i] ].count() <= totalcolumn)
        throw MYMONEYEXCEPTION(QString("Total column %1 out of grid range (%2) in PivotTable::calculateTotals, grid totals").arg(totalcolumn).arg(m_grid.m_total[ m_rowTypeList[i] ].count()));

      //calculate actual totals
      MyMoneyMoney value = m_grid.m_total[ m_rowTypeList[i] ][totalcolumn];
      m_grid.m_total[ m_rowTypeList[i] ].m_total += value;
    }
    ++totalcolumn;
  }
}

void PivotTable::assignCell(const QString& outergroup, const ReportAccount& _row, int column, MyMoneyMoney value, bool budget, bool stockSplit)
{
  DEBUG_ENTER(Q_FUNC_INFO);
  DEBUG_OUTPUT(QString("Parameters: %1,%2,%3,%4,%5").arg(outergroup).arg(_row.debugName()).arg(column).arg(DEBUG_SENSITIVE(value.toDouble())).arg(budget));

  // for budget reports, if this is the actual value, map it to the account which
  // holds its budget
  ReportAccount row = _row;
  if (!budget && m_config.hasBudget()) {
    QString newrow = m_budgetMap[row.id()];

    // if there was no mapping found, then the budget report is not interested
    // in this account.
    if (newrow.isEmpty())
      return;

    row = ReportAccount(newrow);
  }

  // ensure the row already exists (and its parental hierarchy)
  createRow(outergroup, row, true);

  // Determine the inner group from the top-most parent account
  QString innergroup(row.topParentName());

  if (m_numColumns <= column)
    throw MYMONEYEXCEPTION(QString("Column %1 out of m_numColumns range (%2) in PivotTable::assignCell").arg(column).arg(m_numColumns));
  if (m_grid[outergroup][innergroup][row][eActual].count() <= column)
    throw MYMONEYEXCEPTION(QString("Column %1 out of grid range (%2) in PivotTable::assignCell").arg(column).arg(m_grid[outergroup][innergroup][row][eActual].count()));
  if (m_grid[outergroup][innergroup][row][eBudget].count() <= column)
    throw MYMONEYEXCEPTION(QString("Column %1 out of grid range (%2) in PivotTable::assignCell").arg(column).arg(m_grid[outergroup][innergroup][row][eBudget].count()));

  if (!stockSplit) {
    // Determine whether the value should be inverted before being placed in the row
    if (m_grid[outergroup].m_inverted)
      value = -value;

    // Add the value to the grid cell
    if (budget) {
      m_grid[outergroup][innergroup][row][eBudget][column] += value;
    } else {
      // If it is loading an actual value for a budget report
      // check whether it is a subaccount of a budget account (include subaccounts)
      // If so, check if is the same currency and convert otherwise
      if (m_config.hasBudget() &&
          row.id() != _row.id() &&
          row.currencyId() != _row.currencyId()) {
        ReportAccount origAcc = _row;
        MyMoneyMoney rate = origAcc.foreignCurrencyPrice(row.currencyId(), columnDate(column), false);
        m_grid[outergroup][innergroup][row][eActual][column] += (value * rate).reduce();
      } else {
        m_grid[outergroup][innergroup][row][eActual][column] += value;
      }
    }
  } else {
    m_grid[outergroup][innergroup][row][eActual][column] += PivotCell::stockSplit(value);
  }

}

void PivotTable::createRow(const QString& outergroup, const ReportAccount& row, bool recursive)
{
  DEBUG_ENTER(Q_FUNC_INFO);

  // Determine the inner group from the top-most parent account
  QString innergroup(row.topParentName());

  if (! m_grid.contains(outergroup)) {
    DEBUG_OUTPUT(QString("Adding group [%1]").arg(outergroup));
    m_grid[outergroup] = PivotOuterGroup(m_numColumns);
  }

  if (! m_grid[outergroup].contains(innergroup)) {
    DEBUG_OUTPUT(QString("Adding group [%1][%2]").arg(outergroup).arg(innergroup));
    m_grid[outergroup][innergroup] = PivotInnerGroup(m_numColumns);
  }

  if (! m_grid[outergroup][innergroup].contains(row)) {
    DEBUG_OUTPUT(QString("Adding row [%1][%2][%3]").arg(outergroup).arg(innergroup).arg(row.debugName()));
    m_grid[outergroup][innergroup][row] = PivotGridRowSet(m_numColumns);

    if (recursive && !row.isTopLevel())
      createRow(outergroup, row.parent(), recursive);
  }
}

int PivotTable::columnValue(const QDate& _date) const
{
  if (m_config.isColumnsAreDays())
    return (m_beginDate.daysTo(_date));
  else
    return (_date.year() * 12 + _date.month());
}

QDate PivotTable::columnDate(int column) const
{
  if (m_config.isColumnsAreDays())
    return m_beginDate.addDays(m_config.columnPitch() * column - m_startColumn);
  else
    return m_beginDate.addMonths(m_config.columnPitch() * column).addDays(-m_startColumn);
}

QString PivotTable::renderCSV() const
{
  DEBUG_ENTER(Q_FUNC_INFO);

  MyMoneyFile* file = MyMoneyFile::instance();
  int pricePrecision = 0;
  int currencyPrecision = 0;
  int precision = MyMoneyMoney::denomToPrec(file->baseCurrency().smallestAccountFraction());

  bool isMultipleCurrencies = false;

  //
  // Table Header
  //

  QString result = i18n("Account");

  auto column = 0;
  while (column < m_numColumns) {
    result += QString(",%1").arg(QString(m_columnHeadings[column++]));
    if (m_rowTypeList.size() > 1) {
      QString separator;
      separator = separator.fill(',', m_rowTypeList.size() - 1);
      result += separator;
    }
  }

  //show total columns
  if (m_config.isShowingRowTotals())
    result += QString(",%1").arg(i18nc("Total balance", "Total"));

  result += '\n';

  // Row Type Header
  if (m_rowTypeList.size() > 1) {
    auto column = 0;
    while (column < m_numColumns) {
      for (int i = 0; i < m_rowTypeList.size(); ++i) {
        result += QString(",%1").arg(m_columnTypeHeaderList[i]);
      }
      column++;
    }
    if (m_config.isShowingRowTotals()) {
      for (int i = 0; i < m_rowTypeList.size(); ++i) {
        result += QString(",%1").arg(m_columnTypeHeaderList[i]);
      }
    }
    result += '\n';
  }

  //
  // Outer groups
  //

  // iterate over outer groups
  PivotGrid::const_iterator it_outergroup = m_grid.begin();
  while (it_outergroup != m_grid.end()) {
    //
    // Outer Group Header
    //

    if (!(m_config.isIncludingPrice() || m_config.isIncludingAveragePrice()))
      result += it_outergroup.key() + '\n';

    //
    // Inner Groups
    //

    PivotOuterGroup::const_iterator it_innergroup = (*it_outergroup).begin();
    int rownum = 0;
    while (it_innergroup != (*it_outergroup).end()) {
      //
      // Rows
      //

      QString innergroupdata;
      PivotInnerGroup::const_iterator it_row = (*it_innergroup).begin();
      while (it_row != (*it_innergroup).end()) {
        ReportAccount rowname = it_row.key();

        //
        // Columns
        //

        QString rowdata;
        auto column = 0;

        bool isUsed = false;
        for (int i = 0; i < m_rowTypeList.size(); ++i)
          isUsed |= it_row.value()[ m_rowTypeList[i] ][0].isUsed();

        if (it_row.key().accountType() != eMyMoney::Account::Type::Investment) {
          while (column < m_numColumns) {

            //show columns
            foreach (const auto rowType, m_rowTypeList) {
              if (rowType == ePrice) {
                if (pricePrecision == 0) {
                  if (it_row.key().isInvest()) {
                    pricePrecision = file->currency(it_row.key().currencyId()).pricePrecision();
                    precision = pricePrecision;
                  } else
                    precision = MyMoneyMoney::denomToPrec(file->baseCurrency().smallestAccountFraction());
                } else
                  precision = pricePrecision;
              } else {
                if (currencyPrecision == 0) {
                  if (it_row.key().isInvest()) // stock account isn't eveluated in currency, so take investment account instead
                    currencyPrecision = MyMoneyMoney::denomToPrec(it_row.key().parent().fraction());
                  else
                    currencyPrecision = MyMoneyMoney::denomToPrec(it_row.key().fraction());
                  precision = currencyPrecision;
                } else
                  precision = currencyPrecision;
              }
              rowdata += QString(",\"%1\"").arg(it_row.value()[rowType][column].formatMoney(QString(), precision, false));
              isUsed |= it_row.value()[rowType][column].isUsed();
            }
            column++;
          }

          if (m_config.isShowingRowTotals()) {
            for (int i = 0; i < m_rowTypeList.size(); ++i)
              rowdata += QString(",\"%1\"").arg((*it_row)[ m_rowTypeList[i] ].m_total.formatMoney(QString(), precision, false));
          }
        } else {
          for (auto i = 0; i < m_numColumns + m_rowTypeList.size(); ++i)
            rowdata.append(',');;
        }

        //
        // Row Header
        //

        if (!rowname.isClosed() || isUsed) {
          innergroupdata += "\"" + QString().fill(' ', rowname.hierarchyDepth() - 1) + rowname.name();

          // if we don't convert the currencies to the base currency and the
          // current row contains a foreign currency, then we append the currency
          // to the name of the account
          if (!m_config.isConvertCurrency() && rowname.isForeignCurrency())
            innergroupdata += QString(" (%1)").arg(rowname.currencyId());

          innergroupdata += '\"';

          if (isUsed)
            innergroupdata += rowdata;

          innergroupdata += '\n';
          if (!isMultipleCurrencies && rowname.isForeignCurrency())
            isMultipleCurrencies = true;
          if (!m_containsNonBaseCurrency && rowname.isForeignCurrency())
            m_containsNonBaseCurrency = true;
        }
        ++it_row;
      }

      //
      // Inner Row Group Totals
      //

      bool finishrow = true;
      QString finalRow;
      bool isUsed = false;
      if (m_config.detailLevel() == MyMoneyReport::eDetailAll && ((*it_innergroup).size() > 1)) {
        // Print the individual rows
        result += innergroupdata;

        if (m_config.isConvertCurrency() && m_config.isShowingColumnTotals()) {
          // Start the TOTALS row
          finalRow = i18nc("Total balance", "Total");
          isUsed = true;
        } else {
          ++rownum;
          finishrow = false;
        }
      } else {
        // Start the single INDIVIDUAL ACCOUNT row
        ReportAccount rowname = (*it_innergroup).begin().key();
        isUsed |= !rowname.isClosed();

        finalRow = "\"" + QString().fill(' ', rowname.hierarchyDepth() - 1) + rowname.name();
        if (!m_config.isConvertCurrency() && rowname.isForeignCurrency())
          finalRow += QString(" (%1)").arg(rowname.currencyId());
        finalRow += "\"";
      }

      // Finish the row started above, unless told not to
      if (finishrow) {
        auto column = 0;

        for (int i = 0; i < m_rowTypeList.size(); ++i)
          isUsed |= (*it_innergroup).m_total[ m_rowTypeList[i] ][0].isUsed();

        while (column < m_numColumns) {
          for (int i = 0; i < m_rowTypeList.size(); ++i) {
            isUsed |= (*it_innergroup).m_total[ m_rowTypeList[i] ][column].isUsed();
            finalRow += QString(",\"%1\"").arg((*it_innergroup).m_total[ m_rowTypeList[i] ][column].formatMoney(QString(), precision, false));
          }
          column++;
        }

        if (m_config.isShowingRowTotals()) {
          for (int i = 0; i < m_rowTypeList.size(); ++i)
            finalRow += QString(",\"%1\"").arg((*it_innergroup).m_total[ m_rowTypeList[i] ].m_total.formatMoney(QString(), precision, false));
        }

        finalRow += '\n';
      }

      if (isUsed) {
        result += finalRow;
        ++rownum;
      }
      ++it_innergroup;
    }

    //
    // Outer Row Group Totals
    //

    if (m_config.isConvertCurrency() && m_config.isShowingColumnTotals()) {
      result += QString("%1 %2").arg(i18nc("Total balance", "Total")).arg(it_outergroup.key());
      auto column = 0;
      while (column < m_numColumns) {
        for (int i = 0; i < m_rowTypeList.size(); ++i)
          result += QString(",\"%1\"").arg((*it_outergroup).m_total[ m_rowTypeList[i] ][column].formatMoney(QString(), precision, false));

        column++;
      }

      if (m_config.isShowingRowTotals()) {
        for (int i = 0; i < m_rowTypeList.size(); ++i)
          result += QString(",\"%1\"").arg((*it_outergroup).m_total[ m_rowTypeList[i] ].m_total.formatMoney(QString(), precision, false));
      }

      result += '\n';
    }
    ++it_outergroup;
  }

  //
  // Report Totals
  //

  if (m_config.isConvertCurrency() && m_config.isShowingColumnTotals()) {
    result += i18n("Grand Total");
    auto totalcolumn = 0;
    while (totalcolumn < m_numColumns) {
      for (int i = 0; i < m_rowTypeList.size(); ++i)
        result += QString(",\"%1\"").arg(m_grid.m_total[ m_rowTypeList[i] ][totalcolumn].formatMoney(QString(), precision, false));

      totalcolumn++;
    }

    if (m_config.isShowingRowTotals()) {
      for (int i = 0; i < m_rowTypeList.size(); ++i)
        result += QString(",\"%1\"").arg(m_grid.m_total[ m_rowTypeList[i] ].m_total.formatMoney(QString(), precision, false));
    }

    result += '\n';
  }
  return result;
}

QString PivotTable::renderHTML() const
{
  DEBUG_ENTER(Q_FUNC_INFO);

  MyMoneyFile* file = MyMoneyFile::instance();
  int pricePrecision = 0;
  int currencyPrecision = 0;
  int precision = MyMoneyMoney::denomToPrec(file->baseCurrency().smallestAccountFraction());
  QString colspan = QString(" colspan=\"%1\"").arg(m_numColumns + 1 + (m_config.isShowingRowTotals() ? 1 : 0));

  // setup a leftborder for better readability of budget vs actual reports
  QString leftborder;
  if (m_rowTypeList.size() > 1)
    leftborder = " class=\"leftborder\"";

  //
  // Table Header
  //
  QString result = QString("\n\n<table class=\"report\" cellspacing=\"0\">\n"
                    "<thead><tr class=\"itemheader\">\n<th>%1</th>").arg(i18n("Account"));

  QString headerspan;
  int span = m_rowTypeList.size();

  headerspan = QString(" colspan=\"%1\"").arg(span);

  auto column = 0;
  while (column < m_numColumns)
    result += QString("<th%1>%2</th>").arg(headerspan, QString(m_columnHeadings[column++]).replace(QRegExp(" "), "<br>"));

  if (m_config.isShowingRowTotals())
    result += QString("<th%1>%2</th>").arg(headerspan).arg(i18nc("Total balance", "Total"));

  result += "</tr></thead>\n";

  //
  // Header for multiple columns
  //
  if (span > 1) {
    result += "<tr><td></td>";

    auto column = 0;
    while (column < m_numColumns) {
      QString lb;
      if (column != 0)
        lb = leftborder;

      for (int i = 0; i < m_rowTypeList.size(); ++i) {
        result += QString("<td%2>%1</td>")
                  .arg(m_columnTypeHeaderList[i])
                  .arg(i == 0 ? lb : QString());
      }
      column++;
    }
    if (m_config.isShowingRowTotals()) {
      for (int i = 0; i < m_rowTypeList.size(); ++i) {
        result += QString("<td%2>%1</td>")
                  .arg(m_columnTypeHeaderList[i])
                  .arg(i == 0 ? leftborder : QString());
      }
    }
    result += "</tr>";
  }


  // Skip the body of the report if the report only calls for totals to be shown
  if (m_config.detailLevel() != MyMoneyReport::eDetailTotal) {
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
    while (it_outergroup_map != m_grid.end()) {
      outergroups.push_back(it_outergroup_map.value());

      // copy the name into the outergroup, because we will now lose any association with
      // the map iterator
      outergroups.back().m_displayName = it_outergroup_map.key();

      ++it_outergroup_map;
    }
    qSort(outergroups.begin(), outergroups.end());

    QList<PivotOuterGroup>::const_iterator it_outergroup = outergroups.constBegin();
    while (it_outergroup != outergroups.constEnd()) {
      //
      // Outer Group Header
      //

      if (!(m_config.isIncludingPrice() || m_config.isIncludingAveragePrice()))
        result += QString("<tr class=\"sectionheader\"><td class=\"left\"%1>%2</td></tr>\n").arg(colspan).arg((*it_outergroup).m_displayName);

      // Skip the inner groups if the report only calls for outer group totals to be shown
      if (m_config.detailLevel() != MyMoneyReport::eDetailGroup) {

        //
        // Inner Groups
        //

        PivotOuterGroup::const_iterator it_innergroup = (*it_outergroup).begin();
        int rownum = 0;
        while (it_innergroup != (*it_outergroup).end()) {
          //
          // Rows
          //

          QString innergroupdata;
          PivotInnerGroup::const_iterator it_row = (*it_innergroup).begin();

          while (it_row != (*it_innergroup).end()) {
            //
            // Columns
            //

            QString rowdata;
            auto column = 0;
            pricePrecision = 0; // new row => new account => new precision
            currencyPrecision = 0;
            bool isUsed = it_row.value()[eActual][0].isUsed();
            if (it_row.key().accountType() != eMyMoney::Account::Type::Investment) {
              while (column < m_numColumns) {
                QString lb;
                if (column > 0)
                  lb = leftborder;

                foreach (const auto rowType, m_rowTypeList) {
                  if (rowType == ePrice) {
                    if (pricePrecision == 0) {
                      if (it_row.key().isInvest()) {
                        pricePrecision = file->currency(it_row.key().currencyId()).pricePrecision();
                        precision = pricePrecision;
                      } else
                        precision = MyMoneyMoney::denomToPrec(file->baseCurrency().smallestAccountFraction());
                    } else
                      precision = pricePrecision;
                  } else {
                    if (currencyPrecision == 0) {
                      if (it_row.key().isInvest()) // stock account isn't eveluated in currency, so take investment account instead
                        currencyPrecision = MyMoneyMoney::denomToPrec(it_row.key().parent().fraction());
                      else
                        currencyPrecision = MyMoneyMoney::denomToPrec(it_row.key().fraction());
                      precision = currencyPrecision;
                    } else
                      precision = currencyPrecision;
                  }
                  rowdata += QString("<td%2>%1</td>")
                      .arg(coloredAmount(it_row.value()[rowType][column], QString(), precision))
                      .arg(lb);
                  lb.clear();
                  isUsed |= it_row.value()[rowType][column].isUsed();
                }
                ++column;
              }

              if (m_config.isShowingRowTotals()) {
                for (int i = 0; i < m_rowTypeList.size(); ++i) {
                  rowdata += QString("<td%2>%1</td>")
                      .arg(coloredAmount(it_row.value()[ m_rowTypeList[i] ].m_total, QString(), precision))
                      .arg(i == 0 ? leftborder : QString());
                }
              }
            } else
              rowdata += QString(QLatin1Literal("<td colspan=%1></td>")).arg(m_numColumns + m_rowTypeList.size());

            //
            // Row Header
            //

            ReportAccount rowname = it_row.key();

            // don't show closed accounts if they have not been used
            if (!rowname.isClosed() || isUsed) {
              innergroupdata += QString("<tr class=\"row-%1\"%2><td%3 class=\"left\" style=\"text-indent: %4.0em\">%5%6</td>")
                                .arg(rownum & 0x01 ? "even" : "odd")
                                .arg(rowname.isTopLevel() ? " id=\"topparent\"" : "")
                                .arg("") //.arg((*it_row).m_total.isZero() ? colspan : "")  // colspan the distance if this row will be blank
                                .arg(rowname.hierarchyDepth() - 1)
                                .arg(rowname.name().replace(QRegExp(" "), "&nbsp;"))
                                .arg((m_config.isConvertCurrency() || !rowname.isForeignCurrency()) ? QString() : QString(" (%1)").arg(rowname.currency().id()));

              // Don't print this row if it's going to be all zeros
              // TODO: Uncomment this, and deal with the case where the data
              // is zero, but the budget is non-zero
              //if ( !(*it_row).m_total.isZero() )
              innergroupdata += rowdata;

              innergroupdata += "</tr>\n";
              if (!m_containsNonBaseCurrency && rowname.isForeignCurrency())
                m_containsNonBaseCurrency = true;
            }

            ++it_row;
          }

          //
          // Inner Row Group Totals
          //

          bool finishrow = true;
          QString finalRow;
          bool isUsed = false;
          if (m_config.detailLevel() == MyMoneyReport::eDetailAll && ((*it_innergroup).size() > 1)) {
            // Print the individual rows
            result += innergroupdata;

            if (m_config.isConvertCurrency() && m_config.isShowingColumnTotals()) {
              // Start the TOTALS row
              finalRow = QString("<tr class=\"row-%1\" id=\"subtotal\"><td class=\"left\">&nbsp;&nbsp;%2</td>")
                         .arg(rownum & 0x01 ? "even" : "odd")
                         .arg(i18nc("Total balance", "Total"));
              // don't suppress display of totals
              isUsed = true;
            } else {
              finishrow = false;
              ++rownum;
            }
          } else {
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
                       .arg(m_config.detailLevel() == MyMoneyReport::eDetailAll ? "id=\"solo\"" : "")
                       .arg(rowname.hierarchyDepth() - 1)
                       .arg(rowname.name().replace(QRegExp(" "), "&nbsp;"))
                       .arg((m_config.isConvertCurrency() || !rowname.isForeignCurrency()) ? QString() : QString(" (%1)").arg(rowname.currency().id()));
          }

          // Finish the row started above, unless told not to
          if (finishrow) {
            auto column = 0;
            isUsed |= (*it_innergroup).m_total[eActual][0].isUsed();
            while (column < m_numColumns) {
              QString lb;
              if (column != 0)
                lb = leftborder;

              for (int i = 0; i < m_rowTypeList.size(); ++i) {
                finalRow += QString("<td%2>%1</td>")
                            .arg(coloredAmount((*it_innergroup).m_total[ m_rowTypeList[i] ][column], QString(), precision))
                            .arg(i == 0 ? lb : QString());
                isUsed |= (*it_innergroup).m_total[ m_rowTypeList[i] ][column].isUsed();
              }

              column++;
            }

            if (m_config.isShowingRowTotals()) {
              for (int i = 0; i < m_rowTypeList.size(); ++i) {
                finalRow += QString("<td%2>%1</td>")
                            .arg(coloredAmount((*it_innergroup).m_total[ m_rowTypeList[i] ].m_total, QString(), precision))
                            .arg(i == 0 ? leftborder : QString());
              }
            }

            finalRow += "</tr>\n";
            if (isUsed) {
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

      if (m_config.isConvertCurrency() && m_config.isShowingColumnTotals()) {
        result += QString("<tr class=\"sectionfooter\"><td class=\"left\">%1&nbsp;%2</td>").arg(i18nc("Total balance", "Total")).arg((*it_outergroup).m_displayName);
        auto column = 0;
        while (column < m_numColumns) {
          QString lb;
          if (column != 0)
            lb = leftborder;

          for (int i = 0; i < m_rowTypeList.size(); ++i) {
            result += QString("<td%2>%1</td>")
                      .arg(coloredAmount((*it_outergroup).m_total[ m_rowTypeList[i] ][column], QString(), precision))
                      .arg(i == 0 ? lb : QString());
          }

          column++;
        }

        if (m_config.isShowingRowTotals()) {
          for (int i = 0; i < m_rowTypeList.size(); ++i) {
            result += QString("<td%2>%1</td>")
                      .arg(coloredAmount((*it_outergroup).m_total[ m_rowTypeList[i] ].m_total, QString(), precision))
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

  if (m_config.isConvertCurrency() && m_config.isShowingColumnTotals()) {
    result += QString("<tr class=\"spacer\"><td>&nbsp;</td></tr>\n");
    result += QString("<tr class=\"reportfooter\"><td class=\"left\">%1</td>").arg(i18n("Grand Total"));
    auto totalcolumn = 0;
    while (totalcolumn < m_numColumns) {
      QString lb;
      if (totalcolumn != 0)
        lb = leftborder;

      for (int i = 0; i < m_rowTypeList.size(); ++i) {
        result += QString("<td%2>%1</td>")
                  .arg(coloredAmount(m_grid.m_total[ m_rowTypeList[i] ][totalcolumn], QString(), precision))
                  .arg(i == 0 ? lb : QString());
      }

      totalcolumn++;
    }

    if (m_config.isShowingRowTotals()) {
      for (int i = 0; i < m_rowTypeList.size(); ++i) {
        result += QString("<td%2>%1</td>")
                  .arg(coloredAmount(m_grid.m_total[ m_rowTypeList[i] ].m_total, QString(), precision))
                  .arg(i == 0 ? leftborder : QString());
      }
    }

    result += "</tr>\n";
  }
  result += "</table>\n";
  return result;
}

void PivotTable::dump(const QString& file, const QString& /* context */) const
{
  QFile g(file);
  g.open(QIODevice::WriteOnly);
  QTextStream(&g) << renderHTML();
  g.close();
}

void PivotTable::drawChart(KReportChartView& chartView) const
{
  chartView.drawPivotChart(m_grid, m_config, m_numColumns, m_columnHeadings, m_rowTypeList, m_columnTypeHeaderList);
}

QString PivotTable::coloredAmount(const MyMoneyMoney& amount, const QString& currencySymbol, int prec) const
{
  const auto value = amount.formatMoney(currencySymbol, prec);
  if (amount.isNegative())
    return QString::fromLatin1("<font color=%1>%2</font>")
        .arg(KMyMoneyGlobalSettings::schemeColor(SchemeColor::Negative).name(), value);
  else
    return value;
}

void PivotTable::calculateBudgetDiff()
{
  PivotGrid::iterator it_outergroup = m_grid.begin();
  while (it_outergroup != m_grid.end()) {
    PivotOuterGroup::iterator it_innergroup = (*it_outergroup).begin();
    while (it_innergroup != (*it_outergroup).end()) {
      PivotInnerGroup::iterator it_row = (*it_innergroup).begin();
      while (it_row != (*it_innergroup).end()) {
        int column = m_startColumn;
        switch (it_row.key().accountGroup()) {
          case eMyMoney::Account::Type::Income:
          case eMyMoney::Account::Type::Asset:
            while (column < m_numColumns) {
              it_row.value()[eBudgetDiff][column] = PivotCell(it_row.value()[eActual][column] - it_row.value()[eBudget][column]);
              ++column;
            }
            break;
          case eMyMoney::Account::Type::Expense:
          case eMyMoney::Account::Type::Liability:
            while (column < m_numColumns) {
              it_row.value()[eBudgetDiff][column] = PivotCell(it_row.value()[eBudget][column] - it_row.value()[eActual][column]);
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

void PivotTable::calculateForecast()
{
  //setup forecast
  MyMoneyForecast forecast = KMyMoneyGlobalSettings::forecast();

  //since this is a net worth forecast we want to include all account even those that are not in use
  forecast.setIncludeUnusedAccounts(true);

  //setup forecast dates
  if (m_endDate > QDate::currentDate()) {
    forecast.setForecastEndDate(m_endDate);
    forecast.setForecastStartDate(QDate::currentDate());
    forecast.setForecastDays(QDate::currentDate().daysTo(m_endDate));
  } else {
    forecast.setForecastStartDate(m_beginDate);
    forecast.setForecastEndDate(m_endDate);
    forecast.setForecastDays(m_beginDate.daysTo(m_endDate) + 1);
  }

  //adjust history dates if beginning date is before today
  if (m_beginDate < QDate::currentDate()) {
    forecast.setHistoryEndDate(m_beginDate.addDays(-1));
    forecast.setHistoryStartDate(forecast.historyEndDate().addDays(-forecast.accountsCycle()*forecast.forecastCycles()));
  }

  //run forecast
  if (m_config.rowType() == MyMoneyReport::eAssetLiability) { //asset and liability
    forecast.doForecast();
  } else { //income and expenses
    MyMoneyBudget budget;
    forecast.createBudget(budget, m_beginDate.addYears(-1), m_beginDate.addDays(-1), m_beginDate, m_endDate, false);
  }

  // check if we need to copy the opening balances
  // the conditions might be too tight but those fix a reported
  // problem and should avoid side effects in other situations
  // see https://bugs.kde.org/show_bug.cgi?id=391961
  const bool copyOpeningBalances = (m_startColumn == 1) &&
                                   !m_config.isIncludingSchedules() &&
                                   (m_config.isRunningSum());

  //go through the data and add forecast
  PivotGrid::iterator it_outergroup = m_grid.begin();
  while (it_outergroup != m_grid.end()) {
    PivotOuterGroup::iterator it_innergroup = (*it_outergroup).begin();
    while (it_innergroup != (*it_outergroup).end()) {
      PivotInnerGroup::iterator it_row = (*it_innergroup).begin();
      while (it_row != (*it_innergroup).end()) {
        int column = m_startColumn;
        QDate forecastDate = m_beginDate;
        // check whether the opening balance needs to be setup
        if (copyOpeningBalances) {
          it_row.value()[eForecast][0] += it_row.value()[eActual][0];
        }
        //check whether columns are days or months
        if (m_config.isColumnsAreDays()) {
          while (column < m_numColumns) {
            it_row.value()[eForecast][column] = PivotCell(forecast.forecastBalance(it_row.key(), forecastDate));

            forecastDate = forecastDate.addDays(1);
            ++column;
          }
        } else {
          //if columns are months
          while (column < m_numColumns) {
            // the forecast balance is on the first day of the month see MyMoneyForecast::calculateScheduledMonthlyBalances()
            forecastDate = QDate(forecastDate.year(), forecastDate.month(), 1);
            //check that forecastDate is not over ending date
            if (forecastDate > m_endDate)
              forecastDate = m_endDate;

            //get forecast balance and set the corresponding column
            it_row.value()[eForecast][column] = PivotCell(forecast.forecastBalance(it_row.key(), forecastDate));

            forecastDate = forecastDate.addMonths(1);
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
  if ((m_config.isIncludingBudgetActuals()) ||
      (!m_config.hasBudget()
       && !m_config.isIncludingForecast()
       && !m_config.isIncludingMovingAverage()
       && !m_config.isIncludingPrice()
       && !m_config.isIncludingAveragePrice())
     ) {
    m_rowTypeList.append(eActual);
    m_columnTypeHeaderList.append(i18n("Actual"));
  }

  if (m_config.hasBudget()) {
    m_rowTypeList.append(eBudget);
    m_columnTypeHeaderList.append(i18n("Budget"));
  }

  if (m_config.isIncludingBudgetActuals()) {
    m_rowTypeList.append(eBudgetDiff);
    m_columnTypeHeaderList.append(i18n("Difference"));
  }

  if (m_config.isIncludingForecast()) {
    m_rowTypeList.append(eForecast);
    m_columnTypeHeaderList.append(i18n("Forecast"));
  }

  if (m_config.isIncludingMovingAverage()) {
    m_rowTypeList.append(eAverage);
    m_columnTypeHeaderList.append(i18n("Moving Average"));
  }

  if (m_config.isIncludingAveragePrice()) {
    m_rowTypeList.append(eAverage);
    m_columnTypeHeaderList.append(i18n("Moving Average Price"));
  }

  if (m_config.isIncludingPrice()) {
    m_rowTypeList.append(ePrice);
    m_columnTypeHeaderList.append(i18n("Price"));
  }
}


void PivotTable::calculateMovingAverage()
{
  int delta = m_config.movingAverageDays() / 2;

  //go through the data and add the moving average
  PivotGrid::iterator it_outergroup = m_grid.begin();
  while (it_outergroup != m_grid.end()) {
    PivotOuterGroup::iterator it_innergroup = (*it_outergroup).begin();
    while (it_innergroup != (*it_outergroup).end()) {
      PivotInnerGroup::iterator it_row = (*it_innergroup).begin();
      while (it_row != (*it_innergroup).end()) {
        int column = m_startColumn;

        //check whether columns are days or months
        if (m_config.columnType() == MyMoneyReport::eDays) {
          while (column < m_numColumns) {
            MyMoneyMoney totalPrice = MyMoneyMoney();

            QDate averageStart = columnDate(column).addDays(-delta);
            QDate averageEnd = columnDate(column).addDays(delta);
            for (QDate averageDate = averageStart; averageDate <= averageEnd; averageDate = averageDate.addDays(1)) {
              if (m_config.isConvertCurrency()) {
                totalPrice += it_row.key().deepCurrencyPrice(averageDate) * it_row.key().baseCurrencyPrice(averageDate);
              } else {
                totalPrice += it_row.key().deepCurrencyPrice(averageDate);
              }
              totalPrice = totalPrice.convert(10000);
            }

            //calculate the average price
            MyMoneyMoney averagePrice = totalPrice / MyMoneyMoney((averageStart.daysTo(averageEnd) + 1), 1);

            //get the actual value, multiply by the average price and save that value
            MyMoneyMoney averageValue = it_row.value()[eActual][column] * averagePrice;
            it_row.value()[eAverage][column] = PivotCell(averageValue.convert(10000));

            ++column;
          }
        } else {
          //if columns are months
          while (column < m_numColumns) {
            QDate averageStart = columnDate(column);

            //set the right start date depending on the column type
            switch (m_config.columnType()) {
              case MyMoneyReport::eYears: {
                  averageStart = QDate(columnDate(column).year(), 1, 1);
                  break;
                }
              case MyMoneyReport::eBiMonths: {
                  averageStart = QDate(columnDate(column).year(), columnDate(column).month(), 1).addMonths(-1);
                  break;
                }
              case MyMoneyReport::eQuarters: {
                  averageStart = QDate(columnDate(column).year(), columnDate(column).month(), 1).addMonths(-1);
                  break;
                }
              case MyMoneyReport::eMonths: {
                  averageStart = QDate(columnDate(column).year(), columnDate(column).month(), 1);
                  break;
                }
              case MyMoneyReport::eWeeks: {
                  averageStart = columnDate(column).addDays(-columnDate(column).dayOfWeek() + 1);
                  break;
                }
              default:
                break;
            }

            //gather the actual data and calculate the average
            MyMoneyMoney totalPrice = MyMoneyMoney();
            QDate averageEnd = columnDate(column);
            for (QDate averageDate = averageStart; averageDate <= averageEnd; averageDate = averageDate.addDays(1)) {
              if (m_config.isConvertCurrency()) {
                totalPrice += it_row.key().deepCurrencyPrice(averageDate) * it_row.key().baseCurrencyPrice(averageDate);
              } else {
                totalPrice += it_row.key().deepCurrencyPrice(averageDate);
              }
              totalPrice = totalPrice.convert(10000);
            }

            MyMoneyMoney averagePrice = totalPrice / MyMoneyMoney((averageStart.daysTo(averageEnd) + 1), 1);
            MyMoneyMoney averageValue = it_row.value()[eActual][column] * averagePrice;

            //fill in the average
            it_row.value()[eAverage][column] = PivotCell(averageValue.convert(10000));

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
  MyMoneyFile* file = MyMoneyFile::instance();
  QString baseCurrencyId = file->baseCurrency().id();

  //get the first price date for securities
  QMap<QString, QDate> securityDates = securityFirstPrice();

  //go through the data
  PivotGrid::iterator it_outergroup = m_grid.begin();
  while (it_outergroup != m_grid.end()) {
    PivotOuterGroup::iterator it_innergroup = (*it_outergroup).begin();
    while (it_innergroup != (*it_outergroup).end()) {
      PivotInnerGroup::iterator it_row = (*it_innergroup).begin();
      while (it_row != (*it_innergroup).end()) {
        int column = m_startColumn;

        //if it is a base currency fill all the values
        bool firstPriceExists = false;
        if (it_row.key().currencyId() == baseCurrencyId) {
          firstPriceExists = true;
        }

        while (column < m_numColumns) {
          //check whether the date for that column is on or after the first price
          if (!firstPriceExists
              && securityDates.contains(it_row.key().currencyId())
              && columnDate(column) >= securityDates.value(it_row.key().currencyId())) {
            firstPriceExists = true;
          }

          //only add the dummy value if there is a price for that date
          if (firstPriceExists) {
            //insert a unit of currency for each account
            it_row.value()[rowType][column] = PivotCell(MyMoneyMoney::ONE);
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

QMap<QString, QDate> PivotTable::securityFirstPrice()
{
  MyMoneyFile* file = MyMoneyFile::instance();
  MyMoneyPriceList priceList = file->priceList();
  QMap<QString, QDate> securityPriceDate;

  MyMoneyPriceList::const_iterator prices_it;
  for (prices_it = priceList.constBegin(); prices_it != priceList.constEnd(); ++prices_it) {
    MyMoneyPrice firstPrice = (*((*prices_it).constBegin()));

    //check the security in the from field
    //if it is there, check if it is older
    if (securityPriceDate.contains(firstPrice.from())) {
      if (securityPriceDate.value(firstPrice.from()) > firstPrice.date()) {
        securityPriceDate[firstPrice.from()] = firstPrice.date();
      }
    } else {
      securityPriceDate.insert(firstPrice.from(), firstPrice.date());
    }

    //check the security in the to field
    //if it is there, check if it is older
    if (securityPriceDate.contains(firstPrice.to())) {
      if (securityPriceDate.value(firstPrice.to()) > firstPrice.date()) {
        securityPriceDate[firstPrice.to()] = firstPrice.date();
      }
    } else {
      securityPriceDate.insert(firstPrice.to(), firstPrice.date());
    }
  }
  return securityPriceDate;
}

void PivotTable::includeInvestmentSubAccounts()
{
  // if we're not in expert mode, we need to make sure
  // that all stock accounts for the selected investment
  // account are also selected
  QStringList accountList;
  if (m_config.accounts(accountList)) {
    if (!KMyMoneyGlobalSettings::expertMode()) {
      foreach (const auto sAccount, accountList) {
        auto acc = MyMoneyFile::instance()->account(sAccount);
        if (acc.accountType() == eMyMoney::Account::Type::Investment) {
          foreach (const auto sSubAccount, acc.accountList()) {
            if (!accountList.contains(sSubAccount)) {
              m_config.addAccount(sSubAccount);
            }
          }
        }
      }
    }
  }
}

int PivotTable::currentDateColumn()
{

  //return -1 if the columns do not include the current date
  if (m_beginDate > QDate::currentDate() || m_endDate < QDate::currentDate()) {
    return -1;
  }

  //check the date of each column and return if it is the one for the current date
  //if columns are not days, return the one for the current month or year
  int column = m_startColumn;

  while (column < m_numColumns) {
    if (columnDate(column) >= QDate::currentDate()) {
      break;
    }
    column++;
  }

  //if there is no column matching the current date, return -1
  if (column == m_numColumns) {
    column = -1;
  }
  return column;
}

} // namespace
