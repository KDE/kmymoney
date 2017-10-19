/***************************************************************************
                          mymoneyforecast.h
                             -------------------
    begin                : Wed May 30 2007
    copyright            : (C) 2007 by Alvaro Soliverez
    email                : asoliverez@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MYMONEYFORECAST_H
#define MYMONEYFORECAST_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QMap>
#include <QSet>
#include <QList>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyaccount.h"
#include "mymoneymoney.h"
#include "kmm_mymoney_export.h"
#include "mymoneyunittestable.h"

/**
  *
  *
  * @author Alvaro Soliverez <asoliverez@gmail.com>
  */
class MyMoneyBudget;
class MyMoneySchedule;
class MyMoneyTransaction;
class KMM_MYMONEY_EXPORT MyMoneyForecast
{
  KMM_MYMONEY_UNIT_TESTABLE

public:
  /**
    * The default forecast ctor sets the following defaults:
    *
    * - forecastCycles = 3
    * - accountsCycle = 30
    * - forecastDays = 90
    * - beginForecastDay = 0 (today)
    * - forecastMethod = 0 (scheduled)
    * - historyMethod = 1
    * - includeFutureTransactions = true
    * - includeScheduledTransactions = true
    */
  MyMoneyForecast();
  ~MyMoneyForecast();

  /**
   * calculate forecast based on historic transactions
   */
  void doForecast();

  /**
   * Returns the list of accounts to be forecast.
   */
  QList<MyMoneyAccount> accountList();

  /**
   * Returns the balance trend for account @a acc based on a number of days @p forecastDays
   * Collects and processes all transactions in the past for the
   * same period of forecast and calculates the balance trend
   */
  static MyMoneyMoney calculateAccountTrend(const MyMoneyAccount& acc, int forecastDays);

  /**
   * Returns the forecast balance trend for account @a acc for day @p QDate
   */
  MyMoneyMoney forecastBalance(const MyMoneyAccount& acc, const QDate &forecastDate);

  /**
   * Returns the forecast balance trend for account @a acc for offset @p int
   * offset is days from current date, inside forecast days.
   * Returns 0 if offset not in range of forecast days.
   */
  MyMoneyMoney forecastBalance(const MyMoneyAccount& acc, int offset);

  /**
   * Returns true if an account @a acc is an account to be forecast
   */
  bool isForecastAccount(const MyMoneyAccount& acc);

  /**
   * returns the number of days when a given account is forecast to be below minimum balance
   * returns -1 if it will not be below minimum balance in the forecast period
   */
  int daysToMinimumBalance(const MyMoneyAccount& acc);

  /**
   * returns the number of days when a given account is forecast to be below zero if it is an asset accounts
   * or above zero if it is a liability account
   * returns -1 if it will not happen in the forecast period
   */
  int daysToZeroBalance(const MyMoneyAccount& acc);

  /**
   * amount of variation of a given account in one cycle
   */
  MyMoneyMoney accountCycleVariation(const MyMoneyAccount& acc);

  /**
   * amount of variation of a given account for the whole forecast period
   */
  MyMoneyMoney accountTotalVariation(const MyMoneyAccount& acc);

  /**
   * returns a list of the dates where the account was on its lowest balance in each cycle
   */
  QList<QDate> accountMinimumBalanceDateList(const MyMoneyAccount& acc);

  /**
   * returns a list of the dates where the account was on its highest balance in each cycle
   */
  QList<QDate> accountMaximumBalanceDateList(const MyMoneyAccount& acc);

  /**
   * returns the average balance of the account within the forecast period
   */
  MyMoneyMoney accountAverageBalance(const MyMoneyAccount& acc);

  /**
   * creates a budget based on the history of a given timeframe
   */
  void createBudget(MyMoneyBudget& budget, QDate historyStart, QDate historyEnd, QDate budgetStart, QDate budgetEnd, const bool returnBudget);

  /**
   * number of days to go back in history to calculate forecast
   */
  int historyDays() const {
    return (m_historyStartDate.daysTo(m_historyEndDate) + 1);
  }

  void setAccountsCycle(int accountsCycle)   {
    m_accountsCycle = accountsCycle;
  }
  void setForecastCycles(int forecastCycles)   {
    m_forecastCycles = forecastCycles;
  }
  void setForecastDays(int forecastDays)   {
    m_forecastDays = forecastDays;
  }
  void setBeginForecastDate(const QDate &beginForecastDate) {
    m_beginForecastDate = beginForecastDate;
  }
  void setBeginForecastDay(int beginDay)   {
    m_beginForecastDay = beginDay;
  }
  void setForecastMethod(int forecastMethod) {
    m_forecastMethod = forecastMethod;
  }
  void setHistoryStartDate(const QDate &historyStartDate) {
    m_historyStartDate = historyStartDate;
  }
  void setHistoryEndDate(const QDate &historyEndDate) {
    m_historyEndDate = historyEndDate;
  }
  void setHistoryStartDate(int daysToStartDate) {
    setHistoryStartDate(QDate::currentDate().addDays(-daysToStartDate));
  }
  void setHistoryEndDate(int daysToEndDate) {
    setHistoryEndDate(QDate::currentDate().addDays(-daysToEndDate));
  }
  void setForecastStartDate(const QDate &_startDate) {
    m_forecastStartDate = _startDate;
  }
  void setForecastEndDate(const QDate &_endDate) {
    m_forecastEndDate = _endDate;
  }
  void setSkipOpeningDate(bool _skip) {
    m_skipOpeningDate = _skip;
  }
  void setHistoryMethod(int historyMethod) {
    m_historyMethod = historyMethod;
  }
  void setIncludeUnusedAccounts(bool _bool) {
    m_includeUnusedAccounts = _bool;
  }
  void setForecastDone(bool _bool) {
    m_forecastDone = _bool;
  }
  void setIncludeFutureTransactions(bool _bool) {
    m_includeFutureTransactions = _bool;
  }
  void setIncludeScheduledTransactions(bool _bool) {
    m_includeScheduledTransactions = _bool;
  }

  int accountsCycle() const   {
    return m_accountsCycle;
  }
  int forecastCycles() const   {
    return m_forecastCycles;
  }
  int forecastDays() const {
    return m_forecastDays;
  }
  const QDate& beginForecastDate() const   {
    return m_beginForecastDate;
  }
  int beginForecastDay() const   {
    return m_beginForecastDay;
  }
  int forecastMethod() const   {
    return m_forecastMethod;
  }
  const QDate& historyStartDate() const {
    return m_historyStartDate;
  }
  const QDate& historyEndDate() const {
    return m_historyEndDate;
  }
  const QDate& forecastStartDate() const {
    return m_forecastStartDate;
  }
  const QDate& forecastEndDate() const {
    return m_forecastEndDate;
  }
  bool skipOpeningDate() const {
    return m_skipOpeningDate;
  }
  int historyMethod() const   {
    return m_historyMethod;
  }
  bool isIncludingUnusedAccounts() const {
    return m_includeUnusedAccounts;
  }
  bool isForecastDone() const {
    return m_forecastDone;
  }
  bool isIncludingFutureTransactions() const {
    return m_includeFutureTransactions;
  }
  bool isIncludingScheduledTransactions() const {
    return m_includeScheduledTransactions;
  }

  /**
    * This method modifies a scheduled loan transaction such that all
    * references to automatic calculated values are resolved to actual values.
    *
    * @param schedule const reference to the schedule the transaction is based on
    * @param transaction reference to the transaction to be checked and modified
    * @param balances QMap of (account-id,balance) pairs to be used as current balance
    *                 for the calculation of interest. If map is empty, the engine
    *                 will be interrogated for current balances.
    */
  static void calculateAutoLoan(const MyMoneySchedule& schedule, MyMoneyTransaction& transaction, const QMap<QString, MyMoneyMoney>& balances);

private:

  enum EForecastMethod {eScheduled = 0, eHistoric = 1 };

  /**
   * daily balances of an account
   */
  typedef QMap<QDate, MyMoneyMoney> dailyBalances;

  /**
   * map of trends of an account
   */
  typedef QMap<int, MyMoneyMoney> trendBalances;

  /**
   * Returns the list of accounts to be forecast. Only Asset and Liability are returned.
   */
  static QList<MyMoneyAccount> forecastAccountList();

  /**
   * Returns the list of accounts to create a budget. Only Income and Expenses are returned.
   */
  QList<MyMoneyAccount> budgetAccountList();

  /**
   * calculate daily forecast balance based on historic transactions
   */
  void calculateHistoricDailyBalances();

  /**
   * calculate monthly budget balance based on historic transactions
   */
  void calculateHistoricMonthlyBalances();

  /**
   * calculate monthly budget balance based on historic transactions
   */
  void calculateScheduledMonthlyBalances();

  /**
   * calculate forecast based on future and scheduled transactions
   */
  void doFutureScheduledForecast();

  /**
   * add future transactions to forecast
   */
  void addFutureTransactions();

  /**
   * add scheduled transactions to forecast
   */
  void addScheduledTransactions();

  /**
   * calculate daily forecast balance based on future and scheduled transactions
   */
  void calculateScheduledDailyBalances();

  /**
   * set the starting balance for an accounts
   */
  void setStartingBalance(const MyMoneyAccount& acc);

  /**
   * Returns the day moving average for the account @a acc based on the daily balances of a given number of @p forecastTerms
   * It returns the moving average for a given @p trendDay of the forecastTerm
   * With a term of 1 month and 3 terms, it calculates the trend taking the transactions occurred
   * at that day and the day before,for the last 3 months
   */
  MyMoneyMoney accountMovingAverage(const MyMoneyAccount& acc, const int trendDay, const int forecastTerms);

  /**
   * Returns the weighted moving average for a given @p trendDay
   */
  MyMoneyMoney accountWeightedMovingAverage(const MyMoneyAccount& acc, const int trendDay, const int totalWeight);

  /**
   * Returns the linear regression for a given @p trendDay
   */
  MyMoneyMoney accountLinearRegression(const MyMoneyAccount &acc, const int trendDay, const int totalWeight, const MyMoneyMoney& meanTerms);

  /**
   * calculate daily forecast trend based on historic transactions
   */
  void calculateAccountTrendList();

  /**
   * set the internal list of accounts to be forecast
   */
  void setForecastAccountList();

  /**
   * set the internal list of accounts to create a budget
   */
  void setBudgetAccountList();

  /**
   * get past transactions for the accounts to be forecast
   */
  void pastTransactions();

  /**
   * calculate the day to start forecast and sets the begin date
   * The quantity of forecast days will be counted from this date
   * Depends on the values of begin day and accounts cycle
   * The rules to calculate begin day are as follows:
   * - if beginDay is 0, begin date is current date
   * - if the day of the month set by beginDay has not passed, that will be used
   * - if adding an account cycle to beginDay, will not go past the beginDay of next month,
   *   that date will be used, otherwise it will add account cycle to beginDay until it is past current date
   * It returns the total amount of Forecast Days from current date.
   */
  int calculateBeginForecastDay();

  /**
   * remove accounts from the list if the accounts has no transactions in the forecast timeframe.
   * Used for scheduled-forecast method.
   */
  void purgeForecastAccountsList(QMap<QString, dailyBalances>& accountList);

  /**
   * daily forecast balance of accounts
   */
  QMap<QString, dailyBalances> m_accountList;

  /**
   * daily past balance of accounts
   */
  QMap<QString, dailyBalances> m_accountListPast;

  /**
   * daily forecast trends of accounts
   */
  QMap<QString, trendBalances> m_accountTrendList;

  /**
   * list of forecast account ids.
   */
  QSet<QString> m_forecastAccounts;

  /**
   * cycle of accounts in days
   */
  int m_accountsCycle;

  /**
   * number of cycles to use in forecast
   */
  int m_forecastCycles;

  /**
   * number of days to forecast
   */
  int m_forecastDays;

  /**
   * date to start forecast
   */
  QDate m_beginForecastDate;

  /**
   * day to start forecast
   */
  int m_beginForecastDay;

  /**
   * forecast method
   */
  int m_forecastMethod;

  /**
   * history method
   */
  int m_historyMethod;

  /**
   * start date of history
   */
  QDate m_historyStartDate;

  /**
   * end date of history
   */
  QDate m_historyEndDate;

  /**
   * start date of forecast
   */
  QDate m_forecastStartDate;

  /**
   * end date of forecast
   */
  QDate m_forecastEndDate;

  /**
   * skip opening date when fetching transactions of an account
   */
  bool m_skipOpeningDate;

  /**
   * include accounts with no transactions in the forecast timeframe. default is false.
   */
  bool m_includeUnusedAccounts;

  /**
   * forecast already done
   */
  bool m_forecastDone;

  /**
   * include future transactions when doing a scheduled-based forecast
   */
  bool m_includeFutureTransactions;

  /**
   * include scheduled transactions when doing a scheduled-based forecast
   */
  bool m_includeScheduledTransactions;

};

/**
  * Make it possible to hold @ref MyMoneyForecast objects inside @ref QVariant objects.
  */
Q_DECLARE_METATYPE(MyMoneyForecast)

#endif // MYMONEYFORECAST_H

