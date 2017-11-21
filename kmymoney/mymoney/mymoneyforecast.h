/***************************************************************************
                          mymoneyforecast.h
                             -------------------
    begin                : Wed May 30 2007
    copyright            : (C) 2007 by Alvaro Soliverez
    email                : asoliverez@gmail.com
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

#ifndef MYMONEYFORECAST_H
#define MYMONEYFORECAST_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QMetaType>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmm_mymoney_export.h"
#include "mymoneyunittestable.h"

class QDate;

class MyMoneyMoney;
class MyMoneyAccount;
class MyMoneyBudget;
class MyMoneySchedule;
class MyMoneyTransaction;

template <class T> class QList;
template <class T1, class T2> class QMap;

/**
  *
  *
  * @author Alvaro Soliverez <asoliverez@gmail.com>
  */
class MyMoneyForecastPrivate;
class KMM_MYMONEY_EXPORT MyMoneyForecast
{
  Q_DECLARE_PRIVATE(MyMoneyForecast)
  MyMoneyForecastPrivate * d_ptr;

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
  MyMoneyForecast(const MyMoneyForecast & other);
  MyMoneyForecast(MyMoneyForecast && other);
  MyMoneyForecast & operator=(MyMoneyForecast other);
  friend void swap(MyMoneyForecast& first, MyMoneyForecast& second);
  virtual ~MyMoneyForecast();

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
  int historyDays() const;

  void setAccountsCycle(int accountsCycle);
  void setForecastCycles(int forecastCycles);
  void setForecastDays(int forecastDays);
  void setBeginForecastDate(const QDate &beginForecastDate);
  void setBeginForecastDay(int beginDay);
  void setForecastMethod(int forecastMethod);
  void setHistoryStartDate(const QDate &historyStartDate);
  void setHistoryEndDate(const QDate &historyEndDate);
  void setHistoryStartDate(int daysToStartDate);
  void setHistoryEndDate(int daysToEndDate);
  void setForecastStartDate(const QDate &_startDate);
  void setForecastEndDate(const QDate &_endDate);
  void setSkipOpeningDate(bool _skip);
  void setHistoryMethod(int historyMethod);
  void setIncludeUnusedAccounts(bool _bool);
  void setForecastDone(bool _bool);
  void setIncludeFutureTransactions(bool _bool);
  void setIncludeScheduledTransactions(bool _bool);
  int accountsCycle() const;
  int forecastCycles() const;
  int forecastDays() const;
  QDate beginForecastDate() const;
  int beginForecastDay() const;
  QDate historyStartDate() const;
  QDate historyEndDate() const;
  QDate forecastStartDate() const;
  QDate forecastEndDate() const;
  bool skipOpeningDate() const;
  int historyMethod() const;
  bool isIncludingUnusedAccounts() const;
  bool isForecastDone() const;
  bool isIncludingFutureTransactions() const;
  bool isIncludingScheduledTransactions() const;

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

  /**
   * Returns the list of accounts to be forecast. Only Asset and Liability are returned.
   */
  static QList<MyMoneyAccount> forecastAccountList();
};

inline void swap(MyMoneyForecast& first, MyMoneyForecast& second) // krazy:exclude=inline
{
  using std::swap;
  swap(first.d_ptr, second.d_ptr);
}

inline MyMoneyForecast::MyMoneyForecast(MyMoneyForecast && other) : MyMoneyForecast() // krazy:exclude=inline
{
  swap(*this, other);
}

inline MyMoneyForecast & MyMoneyForecast::operator=(MyMoneyForecast other) // krazy:exclude=inline
{
  swap(*this, other);
  return *this;
}

/**
  * Make it possible to hold @ref MyMoneyForecast objects inside @ref QVariant objects.
  */
Q_DECLARE_METATYPE(MyMoneyForecast)

#endif // MYMONEYFORECAST_H

