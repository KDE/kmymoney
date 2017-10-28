/***************************************************************************
                          mymoneyforecast.cpp
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

#include "mymoneyforecast.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>
#include <QList>
#include <QDebug>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyfile.h"
#include "mymoneyaccount.h"
#include "mymoneyaccountloan.h"
#include "mymoneysecurity.h"
#include "mymoneybudget.h"
#include "mymoneyschedule.h"
#include "mymoneyprice.h"
#include "mymoneysplit.h"
#include "mymoneytransaction.h"
#include "mymoneytransactionfilter.h"
#include "mymoneyfinancialcalculator.h"

MyMoneyForecast::MyMoneyForecast() :
    m_accountsCycle(30),
    m_forecastCycles(3),
    m_forecastDays(90),
    m_beginForecastDay(0),
    m_forecastMethod(eScheduled),
    m_historyMethod(1),
    m_skipOpeningDate(true),
    m_includeUnusedAccounts(false),
    m_forecastDone(false),
    m_includeFutureTransactions(true),
    m_includeScheduledTransactions(true)
{
  setHistoryStartDate(QDate::currentDate().addDays(-forecastCycles()*accountsCycle()));
  setHistoryEndDate(QDate::currentDate().addDays(-1));
}


void MyMoneyForecast::doForecast()
{
  int fDays = calculateBeginForecastDay();
  int fMethod = forecastMethod();
  int fAccCycle = accountsCycle();
  int fCycles = forecastCycles();

  //validate settings
  if (fAccCycle < 1
      || fCycles < 1
      || fDays < 1) {
    throw MYMONEYEXCEPTION("Illegal settings when calling doForecast. Settings must be higher than 0");
  }

  //initialize global variables
  setForecastDays(fDays);
  setForecastStartDate(QDate::currentDate().addDays(1));
  setForecastEndDate(QDate::currentDate().addDays(fDays));
  setAccountsCycle(fAccCycle);
  setForecastCycles(fCycles);
  setHistoryStartDate(forecastCycles() * accountsCycle());
  setHistoryEndDate(QDate::currentDate().addDays(-1)); //yesterday

  //clear all data before calculating
  m_accountListPast.clear();
  m_accountList.clear();
  m_accountTrendList.clear();

  //set forecast accounts
  setForecastAccountList();

  switch (fMethod) {
    case eScheduled:
      doFutureScheduledForecast();
      calculateScheduledDailyBalances();
      break;
    case eHistoric:
      pastTransactions();
      calculateHistoricDailyBalances();
      break;
    default:
      break;
  }

  //flag the forecast as done
  m_forecastDone = true;
}

MyMoneyForecast::~MyMoneyForecast()
{
}

void MyMoneyForecast::pastTransactions()
{
  MyMoneyFile* file = MyMoneyFile::instance();
  MyMoneyTransactionFilter filter;

  filter.setDateFilter(historyStartDate(), historyEndDate());
  filter.setReportAllSplits(false);

  QList<MyMoneyTransaction> transactions = file->transactionList(filter);
  QList<MyMoneyTransaction>::const_iterator it_t = transactions.constBegin();

  //Check past transactions
  for (; it_t != transactions.constEnd(); ++it_t) {
    const QList<MyMoneySplit>& splits = (*it_t).splits();
    QList<MyMoneySplit>::const_iterator it_s = splits.begin();
    for (; it_s != splits.end(); ++it_s) {
      if (!(*it_s).shares().isZero()) {
        MyMoneyAccount acc = file->account((*it_s).accountId());

        //workaround for stock accounts which have faulty opening dates
        QDate openingDate;
        if (acc.accountType() == eMyMoney::Account::Stock) {
          MyMoneyAccount parentAccount = file->account(acc.parentAccountId());
          openingDate = parentAccount.openingDate();
        } else {
          openingDate = acc.openingDate();
        }

        if (isForecastAccount(acc) //If it is one of the accounts we are checking, add the amount of the transaction
            && ((openingDate < (*it_t).postDate() && skipOpeningDate())
                || !skipOpeningDate())) {  //don't take the opening day of the account to calculate balance
          dailyBalances balance;
          //FIXME deal with leap years
          balance = m_accountListPast[acc.id()];
          if (acc.accountType() == eMyMoney::Account::Income) {//if it is income, the balance is stored as negative number
            balance[(*it_t).postDate()] += ((*it_s).shares() * MyMoneyMoney::MINUS_ONE);
          } else {
            balance[(*it_t).postDate()] += (*it_s).shares();
          }
          // check if this is a new account for us
          m_accountListPast[acc.id()] = balance;
        }
      }
    }
  }

  //purge those accounts with no transactions on the period
  if (isIncludingUnusedAccounts() == false)
    purgeForecastAccountsList(m_accountListPast);

  //calculate running sum
  QSet<QString>::ConstIterator it_n;
  for (it_n = m_forecastAccounts.begin(); it_n != m_forecastAccounts.end(); ++it_n) {
    MyMoneyAccount acc = file->account(*it_n);
    m_accountListPast[acc.id()][historyStartDate().addDays(-1)] = file->balance(acc.id(), historyStartDate().addDays(-1));
    for (QDate it_date = historyStartDate(); it_date <= historyEndDate();) {
      m_accountListPast[acc.id()][it_date] += m_accountListPast[acc.id()][it_date.addDays(-1)]; //Running sum
      it_date = it_date.addDays(1);
    }
  }

  //adjust value of investments to deep currency
  for (it_n = m_forecastAccounts.begin(); it_n != m_forecastAccounts.end(); ++it_n) {
    MyMoneyAccount acc = file->account(*it_n);

    if (acc.isInvest()) {
      //get the id of the security for that account
      MyMoneySecurity undersecurity = file->security(acc.currencyId());
      if (! undersecurity.isCurrency()) { //only do it if the security is not an actual currency
        MyMoneyMoney rate = MyMoneyMoney::ONE;    //set the default value

        for (QDate it_date = historyStartDate().addDays(-1) ; it_date <= historyEndDate();) {
          //get the price for the tradingCurrency that day
          const MyMoneyPrice &price = file->price(undersecurity.id(), undersecurity.tradingCurrency(), it_date);
          if (price.isValid()) {
            rate = price.rate(undersecurity.tradingCurrency());
          }
          //value is the amount of shares multiplied by the rate of the deep currency
          m_accountListPast[acc.id()][it_date] = m_accountListPast[acc.id()][it_date] * rate;
          it_date = it_date.addDays(1);
        }
      }
    }
  }
}

bool MyMoneyForecast::isForecastAccount(const MyMoneyAccount& acc)
{
  if (m_forecastAccounts.isEmpty()) {
    setForecastAccountList();
  }
  return m_forecastAccounts.contains(acc.id());
}

void MyMoneyForecast::calculateAccountTrendList()
{
  MyMoneyFile* file = MyMoneyFile::instance();
  int auxForecastTerms;
  int totalWeight = 0;

  //Calculate account trends
  QSet<QString>::ConstIterator it_n;
  for (it_n = m_forecastAccounts.begin(); it_n != m_forecastAccounts.end(); ++it_n) {
    MyMoneyAccount acc = file->account(*it_n);
    m_accountTrendList[acc.id()][0] = MyMoneyMoney(); // for today, the trend is 0

    auxForecastTerms = forecastCycles();
    if (skipOpeningDate()) {

      QDate openingDate;
      if (acc.accountType() == eMyMoney::Account::Stock) {
        MyMoneyAccount parentAccount = file->account(acc.parentAccountId());
        openingDate = parentAccount.openingDate();
      } else {
        openingDate = acc.openingDate();
      }

      if (openingDate > historyStartDate()) { //if acc opened after forecast period
        auxForecastTerms = 1 + ((openingDate.daysTo(historyEndDate()) + 1) / accountsCycle()); // set forecastTerms to a lower value, to calculate only based on how long this account was opened
      }
    }

    switch (historyMethod()) {
        //moving average
      case 0: {
          for (int t_day = 1; t_day <= accountsCycle(); t_day++)
            m_accountTrendList[acc.id()][t_day] = accountMovingAverage(acc, t_day, auxForecastTerms); //moving average
          break;
        }
        //weighted moving average
      case 1: {
          //calculate total weight for moving average
          if (auxForecastTerms == forecastCycles()) {
            totalWeight = (auxForecastTerms * (auxForecastTerms + 1)) / 2; //totalWeight is the triangular number of auxForecastTerms
          } else {
            //if only taking a few periods, totalWeight is the sum of the weight for most recent periods
            for (int i = 1, w = forecastCycles(); i <= auxForecastTerms; ++i, --w)
              totalWeight += w;
          }
          for (int t_day = 1; t_day <= accountsCycle(); t_day++)
            m_accountTrendList[acc.id()][t_day] = accountWeightedMovingAverage(acc, t_day, totalWeight);
          break;
        }
      case 2: {
          //calculate mean term
          MyMoneyMoney meanTerms = MyMoneyMoney((auxForecastTerms * (auxForecastTerms + 1)) / 2, 1) / MyMoneyMoney(auxForecastTerms, 1);

          for (int t_day = 1; t_day <= accountsCycle(); t_day++)
            m_accountTrendList[acc.id()][t_day] = accountLinearRegression(acc, t_day, auxForecastTerms, meanTerms);
          break;
        }
      default:
        break;
    }
  }
}

QList<MyMoneyAccount> MyMoneyForecast::forecastAccountList()
{
  MyMoneyFile* file = MyMoneyFile::instance();

  QList<MyMoneyAccount> accList;
  //Get all accounts from the file and check if they are of the right type to calculate forecast
  file->accountList(accList);
  QList<MyMoneyAccount>::iterator accList_t = accList.begin();
  for (; accList_t != accList.end();) {
    MyMoneyAccount acc = *accList_t;
    if (acc.isClosed()            //check the account is not closed
        || (!acc.isAssetLiability())) {
      //|| (acc.accountType() == eMyMoney::Account::Investment) ) {//check that it is not an Investment account and only include Stock accounts
      //remove the account if it is not of the correct type
      accList_t = accList.erase(accList_t);
    } else {
      ++accList_t;
    }
  }
  return accList;
}

QList<MyMoneyAccount> MyMoneyForecast::accountList()
{
  MyMoneyFile* file = MyMoneyFile::instance();

  QList<MyMoneyAccount> accList;
  QStringList emptyStringList;
  //Get all accounts from the file and check if they are present
  file->accountList(accList, emptyStringList, false);
  QList<MyMoneyAccount>::iterator accList_t = accList.begin();
  for (; accList_t != accList.end();) {
    MyMoneyAccount acc = *accList_t;
    if (!isForecastAccount(acc)) {
      accList_t = accList.erase(accList_t);    //remove the account
    } else {
      ++accList_t;
    }
  }
  return accList;
}

MyMoneyMoney MyMoneyForecast::calculateAccountTrend(const MyMoneyAccount& acc, int trendDays)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  MyMoneyTransactionFilter filter;
  MyMoneyMoney netIncome;
  QDate startDate;
  QDate openingDate = acc.openingDate();

  //validate arguments
  if (trendDays < 1) {
    throw MYMONEYEXCEPTION("Illegal arguments when calling calculateAccountTrend. trendDays must be higher than 0");
  }

  //If it is a new account, we don't take into account the first day
  //because it is usually a weird one and it would mess up the trend
  if (openingDate.daysTo(QDate::currentDate()) < trendDays) {
    startDate = (acc.openingDate()).addDays(1);
  } else {
    startDate = QDate::currentDate().addDays(-trendDays);
  }
  //get all transactions for the period
  filter.setDateFilter(startDate, QDate::currentDate());
  if (acc.accountGroup() == eMyMoney::Account::Income
      || acc.accountGroup() == eMyMoney::Account::Expense) {
    filter.addCategory(acc.id());
  } else {
    filter.addAccount(acc.id());
  }

  filter.setReportAllSplits(false);
  QList<MyMoneyTransaction> transactions = file->transactionList(filter);
  QList<MyMoneyTransaction>::const_iterator it_t = transactions.constBegin();

  //add all transactions for that account
  for (; it_t != transactions.constEnd(); ++it_t) {
    const QList<MyMoneySplit>& splits = (*it_t).splits();
    QList<MyMoneySplit>::const_iterator it_s = splits.begin();
    for (; it_s != splits.end(); ++it_s) {
      if (!(*it_s).shares().isZero()) {
        if (acc.id() == (*it_s).accountId()) netIncome += (*it_s).value();
      }
    }
  }

  //calculate trend of the account in the past period
  MyMoneyMoney accTrend;

  //don't take into account the first day of the account
  if (openingDate.daysTo(QDate::currentDate()) < trendDays) {
    accTrend = netIncome / MyMoneyMoney(openingDate.daysTo(QDate::currentDate()) - 1, 1);
  } else {
    accTrend = netIncome / MyMoneyMoney(trendDays, 1);
  }
  return accTrend;
}

MyMoneyMoney MyMoneyForecast::accountMovingAverage(const MyMoneyAccount &acc, const int trendDay, const int forecastTerms)
{
  //Calculate a daily trend for the account based on the accounts of a given number of terms
  //With a term of 1 month and 3 terms, it calculates the trend taking the transactions occurred at that day and the day before,
  //for the last 3 months
  MyMoneyMoney balanceVariation;

  for (int it_terms = 0; (trendDay + (accountsCycle()*it_terms)) <= historyDays(); ++it_terms) { //sum for each term
    MyMoneyMoney balanceBefore = m_accountListPast[acc.id()][historyStartDate().addDays(trendDay+(accountsCycle()*it_terms)-2)]; //get balance for the day before
    MyMoneyMoney balanceAfter = m_accountListPast[acc.id()][historyStartDate().addDays(trendDay+(accountsCycle()*it_terms)-1)];
    balanceVariation += (balanceAfter - balanceBefore); //add the balance variation between days
  }
  //calculate average of the variations
  return (balanceVariation / MyMoneyMoney(forecastTerms, 1)).convert(10000);
}

MyMoneyMoney MyMoneyForecast::accountWeightedMovingAverage(const MyMoneyAccount &acc, const int trendDay, const int totalWeight)
{
  MyMoneyMoney balanceVariation;

  for (int it_terms = 0, weight = 1; (trendDay + (accountsCycle()*it_terms)) <= historyDays(); ++it_terms, ++weight) { //sum for each term multiplied by weight
    MyMoneyMoney balanceBefore = m_accountListPast[acc.id()][historyStartDate().addDays(trendDay+(accountsCycle()*it_terms)-2)]; //get balance for the day before
    MyMoneyMoney balanceAfter = m_accountListPast[acc.id()][historyStartDate().addDays(trendDay+(accountsCycle()*it_terms)-1)];
    balanceVariation += ((balanceAfter - balanceBefore) * MyMoneyMoney(weight, 1));   //add the balance variation between days multiplied by its weight
  }
  //calculate average of the variations
  return (balanceVariation / MyMoneyMoney(totalWeight, 1)).convert(10000);
}

MyMoneyMoney MyMoneyForecast::accountLinearRegression(const MyMoneyAccount &acc, const int trendDay, const int actualTerms, const MyMoneyMoney& meanTerms)
{
  MyMoneyMoney meanBalance, totalBalance, totalTerms;
  totalTerms = MyMoneyMoney(actualTerms, 1);

  //calculate mean balance
  for (int it_terms = forecastCycles() - actualTerms; (trendDay + (accountsCycle()*it_terms)) <= historyDays(); ++it_terms) { //sum for each term
    totalBalance += m_accountListPast[acc.id()][historyStartDate().addDays(trendDay+(accountsCycle()*it_terms)-1)];
  }
  meanBalance = totalBalance / MyMoneyMoney(actualTerms, 1);
  meanBalance = meanBalance.convert(10000);

  //calculate b1

  //first calculate x - mean x multiplied by y - mean y
  MyMoneyMoney totalXY, totalSqX;
  for (int it_terms = forecastCycles() - actualTerms, term = 1; (trendDay + (accountsCycle()*it_terms)) <= historyDays(); ++it_terms, ++term) { //sum for each term
    MyMoneyMoney balance = m_accountListPast[acc.id()][historyStartDate().addDays(trendDay+(accountsCycle()*it_terms)-1)];

    MyMoneyMoney balMeanBal = balance - meanBalance;
    MyMoneyMoney termMeanTerm = (MyMoneyMoney(term, 1) - meanTerms);

    totalXY += (balMeanBal * termMeanTerm).convert(10000);

    totalSqX += (termMeanTerm * termMeanTerm).convert(10000);
  }
  totalXY = (totalXY / MyMoneyMoney(actualTerms, 1)).convert(10000);
  totalSqX = (totalSqX / MyMoneyMoney(actualTerms, 1)).convert(10000);

  //check zero
  if (totalSqX.isZero())
    return MyMoneyMoney();

  MyMoneyMoney linReg = (totalXY / totalSqX).convert(10000);

  return linReg;
}

void MyMoneyForecast::calculateHistoricDailyBalances()
{
  MyMoneyFile* file = MyMoneyFile::instance();

  calculateAccountTrendList();

  //Calculate account daily balances
  QSet<QString>::ConstIterator it_n;
  for (it_n = m_forecastAccounts.constBegin(); it_n != m_forecastAccounts.constEnd(); ++it_n) {
    MyMoneyAccount acc = file->account(*it_n);

    //set the starting balance of the account
    setStartingBalance(acc);

    switch (historyMethod()) {
      case 0:
      case 1: {
          for (QDate f_day = forecastStartDate(); f_day <= forecastEndDate();) {
            for (int t_day = 1; t_day <= accountsCycle(); ++t_day) {
              MyMoneyMoney balanceDayBefore = m_accountList[acc.id()][(f_day.addDays(-1))];//balance of the day before
              MyMoneyMoney accountDailyTrend = m_accountTrendList[acc.id()][t_day]; //trend for that day
              //balance of the day is the balance of the day before multiplied by the trend for the day
              m_accountList[acc.id()][f_day] = balanceDayBefore;
              m_accountList[acc.id()][f_day] += accountDailyTrend; //movement trend for that particular day
              m_accountList[acc.id()][f_day] = m_accountList[acc.id()][f_day].convert(acc.fraction());
              //m_accountList[acc.id()][f_day] += m_accountListPast[acc.id()][f_day.addDays(-historyDays())];
              f_day = f_day.addDays(1);
            }
          }
        }
        break;
      case 2: {
          QDate baseDate = QDate::currentDate().addDays(-accountsCycle());
          for (int t_day = 1; t_day <= accountsCycle(); ++t_day) {
            int f_day = 1;
            QDate fDate = baseDate.addDays(accountsCycle() + 1);
            while (fDate <= forecastEndDate()) {

              //the calculation is based on the balance for the last month, that is then multiplied by the trend
              m_accountList[acc.id()][fDate] = m_accountListPast[acc.id()][baseDate] + (m_accountTrendList[acc.id()][t_day] * MyMoneyMoney(f_day, 1));
              m_accountList[acc.id()][fDate] = m_accountList[acc.id()][fDate].convert(acc.fraction());
              ++f_day;
              fDate = baseDate.addDays(accountsCycle() * f_day);
            }
            baseDate = baseDate.addDays(1);
          }
        }
    }
  }
}

MyMoneyMoney MyMoneyForecast::forecastBalance(const MyMoneyAccount& acc, const QDate &forecastDate)
{
  dailyBalances balance;
  MyMoneyMoney MM_amount = MyMoneyMoney();

  //Check if acc is not a forecast account, return 0
  if (!isForecastAccount(acc)) {
    return MM_amount;
  }

  if (m_accountList.contains(acc.id())) {
    balance = m_accountList.value(acc.id());
  }
  if (balance.contains(forecastDate)) { //if the date is not in the forecast, it returns 0
    MM_amount = balance.value(forecastDate);
  }
  return MM_amount;
}

/**
 * Returns the forecast balance trend for account @a acc for offset @p int
 * offset is days from current date, inside forecast days.
 * Returns 0 if offset not in range of forecast days.
 */
MyMoneyMoney MyMoneyForecast::forecastBalance(const MyMoneyAccount& acc, int offset)
{
  QDate forecastDate = QDate::currentDate().addDays(offset);
  return forecastBalance(acc, forecastDate);
}

void MyMoneyForecast::doFutureScheduledForecast()
{
  MyMoneyFile* file = MyMoneyFile::instance();

  if (isIncludingFutureTransactions())
    addFutureTransactions();

  if (isIncludingScheduledTransactions())
    addScheduledTransactions();

  //do not show accounts with no transactions
  if (!isIncludingUnusedAccounts())
    purgeForecastAccountsList(m_accountList);

  //adjust value of investments to deep currency
  QSet<QString>::ConstIterator it_n;
  for (it_n = m_forecastAccounts.constBegin(); it_n != m_forecastAccounts.constEnd(); ++it_n) {
    MyMoneyAccount acc = file->account(*it_n);

    if (acc.isInvest()) {
      //get the id of the security for that account
      MyMoneySecurity undersecurity = file->security(acc.currencyId());

      //only do it if the security is not an actual currency
      if (! undersecurity.isCurrency()) {
        //set the default value
        MyMoneyMoney rate = MyMoneyMoney::ONE;

        for (QDate it_day = QDate::currentDate(); it_day <= forecastEndDate();) {
          //get the price for the tradingCurrency that day
          const MyMoneyPrice &price = file->price(undersecurity.id(), undersecurity.tradingCurrency(), it_day);
          if (price.isValid()) {
            rate = price.rate(undersecurity.tradingCurrency());
          }
          //value is the amount of shares multiplied by the rate of the deep currency
          m_accountList[acc.id()][it_day] = m_accountList[acc.id()][it_day] * rate;
          it_day = it_day.addDays(1);
        }
      }
    }
  }
}

void MyMoneyForecast::addFutureTransactions()
{
  MyMoneyTransactionFilter filter;
  MyMoneyFile* file = MyMoneyFile::instance();

  // collect and process all transactions that have already been entered but
  // are located in the future.
  filter.setDateFilter(forecastStartDate(), forecastEndDate());
  filter.setReportAllSplits(false);

  QList<MyMoneyTransaction> transactions = file->transactionList(filter);
  QList<MyMoneyTransaction>::const_iterator it_t = transactions.constBegin();

  for (; it_t != transactions.constEnd(); ++it_t) {
    const QList<MyMoneySplit>& splits = (*it_t).splits();
    QList<MyMoneySplit>::const_iterator it_s = splits.begin();
    for (; it_s != splits.end(); ++it_s) {
      if (!(*it_s).shares().isZero()) {
        MyMoneyAccount acc = file->account((*it_s).accountId());
        if (isForecastAccount(acc)) {
          dailyBalances balance;
          balance = m_accountList[acc.id()];
          //if it is income, the balance is stored as negative number
          if (acc.accountType() == eMyMoney::Account::Income) {
            balance[(*it_t).postDate()] += ((*it_s).shares() * MyMoneyMoney::MINUS_ONE);
          } else {
            balance[(*it_t).postDate()] += (*it_s).shares();
          }
          m_accountList[acc.id()] = balance;
        }
      }
    }
  }

#if 0
  QFile trcFile("forecast.csv");
  trcFile.open(QIODevice::WriteOnly);
  QTextStream s(&trcFile);

  {
    s << "Already present transactions\n";
    QMap<QString, dailyBalances>::Iterator it_a;
    QSet<QString>::ConstIterator it_n;
    for (it_n = m_nameIdx.begin(); it_n != m_nameIdx.end(); ++it_n) {
      MyMoneyAccount acc = file->account(*it_n);
      it_a = m_accountList.find(*it_n);
      s << "\"" << acc.name() << "\",";
      for (int i = 0; i < 90; ++i) {
        s << "\"" << (*it_a)[i].formatMoney("") << "\",";
      }
      s << "\n";
    }
  }
#endif

}

void MyMoneyForecast::addScheduledTransactions()
{
  MyMoneyFile* file = MyMoneyFile::instance();

  // now process all the schedules that may have an impact
  QList<MyMoneySchedule> schedule;

  schedule = file->scheduleList(QString(), eMyMoney::Schedule::Type::Any, eMyMoney::Schedule::Occurrence::Any, eMyMoney::Schedule::PaymentType::Any,
                                QDate(), forecastEndDate(), false);
  if (schedule.count() > 0) {
    QList<MyMoneySchedule>::Iterator it;
    do {
      qSort(schedule);
      it = schedule.begin();
      if (it == schedule.end())
        break;

      if ((*it).isFinished()) {
        schedule.erase(it);
        continue;
      }

      QDate date = (*it).nextPayment((*it).lastPayment());
      if (!date.isValid()) {
        schedule.erase(it);
        continue;
      }

      QDate nextDate =
        (*it).adjustedNextPayment((*it).adjustedDate((*it).lastPayment(),
                                  (*it).weekendOption()));
      if (nextDate > forecastEndDate()) {
        // We're done with this schedule, let's move on to the next
        schedule.erase(it);
        continue;
      }

      // found the next schedule. process it

      MyMoneyAccount acc = (*it).account();

      if (!acc.id().isEmpty()) {
        try {
          if (acc.accountType() != eMyMoney::Account::Investment) {
            MyMoneyTransaction t = (*it).transaction();

            // only process the entry, if it is still active
            if (!(*it).isFinished() && nextDate != QDate()) {
              // make sure we have all 'starting balances' so that the autocalc works
              QList<MyMoneySplit>::const_iterator it_s;
              QMap<QString, MyMoneyMoney> balanceMap;

              for (it_s = t.splits().constBegin(); it_s != t.splits().constEnd(); ++it_s) {
                MyMoneyAccount acc = file->account((*it_s).accountId());
                if (isForecastAccount(acc)) {
                  // collect all overdues on the first day
                  QDate forecastDate = nextDate;
                  if (QDate::currentDate() >= nextDate)
                    forecastDate = QDate::currentDate().addDays(1);

                  dailyBalances balance;
                  balance = m_accountList[acc.id()];
                  for (QDate f_day = QDate::currentDate(); f_day < forecastDate;) {
                    balanceMap[acc.id()] += m_accountList[acc.id()][f_day];
                    f_day = f_day.addDays(1);
                  }
                }
              }

              // take care of the autoCalc stuff
              calculateAutoLoan(*it, t, balanceMap);

              // now add the splits to the balances
              for (it_s = t.splits().constBegin(); it_s != t.splits().constEnd(); ++it_s) {
                MyMoneyAccount acc = file->account((*it_s).accountId());
                if (isForecastAccount(acc)) {
                  dailyBalances balance;
                  balance = m_accountList[acc.id()];
                  //int offset = QDate::currentDate().daysTo(nextDate);
                  //if(offset <= 0) {  // collect all overdues on the first day
                  //  offset = 1;
                  //}
                  // collect all overdues on the first day
                  QDate forecastDate = nextDate;
                  if (QDate::currentDate() >= nextDate)
                    forecastDate = QDate::currentDate().addDays(1);

                  if (acc.accountType() == eMyMoney::Account::Income) {
                    balance[forecastDate] += ((*it_s).shares() * MyMoneyMoney::MINUS_ONE);
                  } else {
                    balance[forecastDate] += (*it_s).shares();
                  }
                  m_accountList[acc.id()] = balance;
                }
              }
            }
          }
          (*it).setLastPayment(date);

        } catch (const MyMoneyException &e) {
          qDebug() << Q_FUNC_INFO << " Schedule " << (*it).id() << " (" << (*it).name() << "): " << e.what();

          schedule.erase(it);
        }
      } else {
        // remove schedule from list
        schedule.erase(it);
      }
    } while (1);
  }

#if 0
  {
    s << "\n\nAdded scheduled transactions\n";
    QMap<QString, dailyBalances>::Iterator it_a;
    QSet<QString>::ConstIterator it_n;
    for (it_n = m_nameIdx.begin(); it_n != m_nameIdx.end(); ++it_n) {
      MyMoneyAccount acc = file->account(*it_n);
      it_a = m_accountList.find(*it_n);
      s << "\"" << acc.name() << "\",";
      for (int i = 0; i < 90; ++i) {
        s << "\"" << (*it_a)[i].formatMoney("") << "\",";
      }
      s << "\n";
    }
  }
#endif
}

void MyMoneyForecast::calculateScheduledDailyBalances()
{
  MyMoneyFile* file = MyMoneyFile::instance();

  //Calculate account daily balances
  QSet<QString>::ConstIterator it_n;
  for (it_n = m_forecastAccounts.constBegin(); it_n != m_forecastAccounts.constEnd(); ++it_n) {
    MyMoneyAccount acc = file->account(*it_n);

    //set the starting balance of the account
    setStartingBalance(acc);

    for (QDate f_day = forecastStartDate(); f_day <= forecastEndDate();) {
      MyMoneyMoney balanceDayBefore = m_accountList[acc.id()][(f_day.addDays(-1))];//balance of the day before
      m_accountList[acc.id()][f_day] += balanceDayBefore; //running sum
      f_day = f_day.addDays(1);
    }
  }


}

int MyMoneyForecast::daysToMinimumBalance(const MyMoneyAccount& acc)
{
  QString minimumBalance = acc.value("minBalanceAbsolute");
  MyMoneyMoney minBalance = MyMoneyMoney(minimumBalance);
  dailyBalances balance;

  //Check if acc is not a forecast account, return -1
  if (!isForecastAccount(acc)) {
    return -1;
  }

  balance = m_accountList[acc.id()];

  for (QDate it_day = QDate::currentDate() ; it_day <= forecastEndDate();) {
    if (minBalance > balance[it_day]) {
      return QDate::currentDate().daysTo(it_day);
    }
    it_day = it_day.addDays(1);
  }
  return -1;
}

int MyMoneyForecast::daysToZeroBalance(const MyMoneyAccount& acc)
{
  dailyBalances balance;

  //Check if acc is not a forecast account, return -1
  if (!isForecastAccount(acc)) {
    return -2;
  }

  balance = m_accountList[acc.id()];

  if (acc.accountGroup() == eMyMoney::Account::Asset) {
    for (QDate it_day = QDate::currentDate() ; it_day <= forecastEndDate();) {
      if (balance[it_day] < MyMoneyMoney()) {
        return QDate::currentDate().daysTo(it_day);
      }
      it_day = it_day.addDays(1);
    }
  } else if (acc.accountGroup() == eMyMoney::Account::Liability) {
    for (QDate it_day = QDate::currentDate() ; it_day <= forecastEndDate();) {
      if (balance[it_day] > MyMoneyMoney()) {
        return QDate::currentDate().daysTo(it_day);
      }
      it_day = it_day.addDays(1);
    }
  }
  return -1;
}

void MyMoneyForecast::setForecastAccountList()
{

  //get forecast accounts
  QList<MyMoneyAccount> accList;
  accList = forecastAccountList();

  QList<MyMoneyAccount>::const_iterator accList_t = accList.constBegin();
  for (; accList_t != accList.constEnd(); ++accList_t) {
    m_forecastAccounts.insert((*accList_t).id());
  }
}

MyMoneyMoney MyMoneyForecast::accountCycleVariation(const MyMoneyAccount& acc)
{
  MyMoneyMoney cycleVariation;

  if (forecastMethod() == eHistoric) {
    switch (historyMethod()) {
      case 0:
      case 1: {
          for (int t_day = 1; t_day <= accountsCycle() ; ++t_day) {
            cycleVariation += m_accountTrendList[acc.id()][t_day];
          }
        }
        break;
      case 2: {
          cycleVariation = m_accountList[acc.id()][QDate::currentDate().addDays(accountsCycle())] - m_accountList[acc.id()][QDate::currentDate()];
          break;
        }
    }
  }
  return cycleVariation;
}

MyMoneyMoney MyMoneyForecast::accountTotalVariation(const MyMoneyAccount& acc)
{
  MyMoneyMoney totalVariation;

  totalVariation = forecastBalance(acc, forecastEndDate()) - forecastBalance(acc, QDate::currentDate());
  return totalVariation;
}

QList<QDate> MyMoneyForecast::accountMinimumBalanceDateList(const MyMoneyAccount& acc)
{
  QList<QDate> minBalanceList;
  int daysToBeginDay;

  daysToBeginDay = QDate::currentDate().daysTo(beginForecastDate());

  for (int t_cycle = 0; ((t_cycle * accountsCycle()) + daysToBeginDay) < forecastDays() ; ++t_cycle) {
    MyMoneyMoney minBalance = forecastBalance(acc, (t_cycle * accountsCycle() + daysToBeginDay));
    QDate minDate = QDate::currentDate().addDays(t_cycle * accountsCycle() + daysToBeginDay);
    for (int t_day = 1; t_day <= accountsCycle() ; ++t_day) {
      if (minBalance > forecastBalance(acc, (t_cycle * accountsCycle()) + daysToBeginDay + t_day)) {
        minBalance = forecastBalance(acc, (t_cycle * accountsCycle()) + daysToBeginDay + t_day);
        minDate = QDate::currentDate().addDays((t_cycle * accountsCycle()) + daysToBeginDay + t_day);
      }
    }
    minBalanceList.append(minDate);
  }
  return minBalanceList;
}

QList<QDate> MyMoneyForecast::accountMaximumBalanceDateList(const MyMoneyAccount& acc)
{
  QList<QDate> maxBalanceList;
  int daysToBeginDay;

  daysToBeginDay = QDate::currentDate().daysTo(beginForecastDate());

  for (int t_cycle = 0; ((t_cycle * accountsCycle()) + daysToBeginDay) < forecastDays() ; ++t_cycle) {
    MyMoneyMoney maxBalance = forecastBalance(acc, ((t_cycle * accountsCycle()) + daysToBeginDay));
    QDate maxDate = QDate::currentDate().addDays((t_cycle * accountsCycle()) + daysToBeginDay);

    for (int t_day = 0; t_day < accountsCycle() ; ++t_day) {
      if (maxBalance < forecastBalance(acc, (t_cycle * accountsCycle()) + daysToBeginDay + t_day)) {
        maxBalance = forecastBalance(acc, (t_cycle * accountsCycle()) + daysToBeginDay + t_day);
        maxDate = QDate::currentDate().addDays((t_cycle * accountsCycle()) + daysToBeginDay + t_day);
      }
    }
    maxBalanceList.append(maxDate);
  }
  return maxBalanceList;
}

MyMoneyMoney MyMoneyForecast::accountAverageBalance(const MyMoneyAccount& acc)
{
  MyMoneyMoney totalBalance;
  for (int f_day = 1; f_day <= forecastDays() ; ++f_day) {
    totalBalance += forecastBalance(acc, f_day);
  }
  return totalBalance / MyMoneyMoney(forecastDays(), 1);
}

int MyMoneyForecast::calculateBeginForecastDay()
{
  int fDays = forecastDays();
  int beginDay = beginForecastDay();
  int accCycle = accountsCycle();
  QDate beginDate;

  //if 0, beginDate is current date and forecastDays remains unchanged
  if (beginDay == 0) {
    setBeginForecastDate(QDate::currentDate());
    return fDays;
  }

  //adjust if beginDay more than days of current month
  if (QDate::currentDate().daysInMonth() < beginDay)
    beginDay = QDate::currentDate().daysInMonth();

  //if beginDay still to come, calculate and return
  if (QDate::currentDate().day() <= beginDay) {
    beginDate = QDate(QDate::currentDate().year(), QDate::currentDate().month(), beginDay);
    fDays += QDate::currentDate().daysTo(beginDate);
    setBeginForecastDate(beginDate);
    return fDays;
  }

  //adjust beginDay for next month
  if (QDate::currentDate().addMonths(1).daysInMonth() < beginDay)
    beginDay = QDate::currentDate().addMonths(1).daysInMonth();

  //if beginDay of next month comes before 1 interval, use beginDay next month
  if (QDate::currentDate().addDays(accCycle) >=
      (QDate(QDate::currentDate().addMonths(1).year(), QDate::currentDate().addMonths(1).month(), 1).addDays(beginDay - 1))) {
    beginDate = QDate(QDate::currentDate().addMonths(1).year(), QDate::currentDate().addMonths(1).month(), 1).addDays(beginDay - 1);
    fDays += QDate::currentDate().daysTo(beginDate);
  } else { //add intervals to current beginDay and take the first after current date
    beginDay = ((((QDate::currentDate().day() - beginDay) / accCycle) + 1) * accCycle) + beginDay;
    beginDate = QDate::currentDate().addDays(beginDay - QDate::currentDate().day());
    fDays += QDate::currentDate().daysTo(beginDate);
  }

  setBeginForecastDate(beginDate);
  return fDays;
}

void MyMoneyForecast::purgeForecastAccountsList(QMap<QString, dailyBalances>& accountList)
{
  m_forecastAccounts.intersect(accountList.keys().toSet());
}

void MyMoneyForecast::createBudget(MyMoneyBudget& budget, QDate historyStart, QDate historyEnd, QDate budgetStart, QDate budgetEnd, const bool returnBudget)
{
  // clear all data except the id and name
  QString name = budget.name();
  budget = MyMoneyBudget(budget.id(), MyMoneyBudget());
  budget.setName(name);

  //check parameters
  if (historyStart > historyEnd ||
      budgetStart > budgetEnd ||
      budgetStart <= historyEnd) {
    throw MYMONEYEXCEPTION("Illegal parameters when trying to create budget");
  }

  //get forecast method
  int fMethod = forecastMethod();

  //set start date to 1st of month and end dates to last day of month, since we deal with full months in budget
  historyStart = QDate(historyStart.year(), historyStart.month(), 1);
  historyEnd = QDate(historyEnd.year(), historyEnd.month(), historyEnd.daysInMonth());
  budgetStart = QDate(budgetStart.year(), budgetStart.month(), 1);
  budgetEnd = QDate(budgetEnd.year(), budgetEnd.month(), budgetEnd.daysInMonth());

  //set forecast parameters
  setHistoryStartDate(historyStart);
  setHistoryEndDate(historyEnd);
  setForecastStartDate(budgetStart);
  setForecastEndDate(budgetEnd);
  setForecastDays(budgetStart.daysTo(budgetEnd) + 1);
  if (budgetStart.daysTo(budgetEnd) > historyStart.daysTo(historyEnd)) {         //if history period is shorter than budget, use that one as the trend length
    setAccountsCycle(historyStart.daysTo(historyEnd));       //we set the accountsCycle to the base timeframe we will use to calculate the average (eg. 180 days, 365, etc)
  } else { //if one timeframe is larger than the other, but not enough to be 1 time larger, we take the lowest value
    setAccountsCycle(budgetStart.daysTo(budgetEnd));
  }
  setForecastCycles((historyStart.daysTo(historyEnd) / accountsCycle()));
  if (forecastCycles() == 0)   //the cycles must be at least 1
    setForecastCycles(1);

  //do not skip opening date
  setSkipOpeningDate(false);

  //clear and set accounts list we are going to use. Categories, in this case
  m_forecastAccounts.clear();
  setBudgetAccountList();

  //calculate budget according to forecast method
  switch (fMethod) {
    case eScheduled:
      doFutureScheduledForecast();
      calculateScheduledMonthlyBalances();
      break;
    case eHistoric:
      pastTransactions(); //get all transactions for history period
      calculateAccountTrendList();
      calculateHistoricMonthlyBalances(); //add all balances of each month and put at the 1st day of each month
      break;
    default:
      break;
  }

  //flag the forecast as done
  m_forecastDone = true;

  //only fill the budget if it is going to be used
  if (returnBudget) {
    //setup the budget itself
    MyMoneyFile* file = MyMoneyFile::instance();
    budget.setBudgetStart(budgetStart);

    //go through all the accounts and add them to budget
    QSet<QString>::ConstIterator it_nc;
    for (it_nc = m_forecastAccounts.constBegin(); it_nc != m_forecastAccounts.constEnd(); ++it_nc) {
      MyMoneyAccount acc = file->account(*it_nc);

      MyMoneyBudget::AccountGroup budgetAcc;
      budgetAcc.setId(acc.id());
      budgetAcc.setBudgetLevel(MyMoneyBudget::AccountGroup::eMonthByMonth);

      for (QDate f_date = forecastStartDate(); f_date <= forecastEndDate();) {
        MyMoneyBudget::PeriodGroup period;

        //add period to budget account
        period.setStartDate(f_date);
        period.setAmount(forecastBalance(acc, f_date));
        budgetAcc.addPeriod(f_date, period);

        //next month
        f_date = f_date.addMonths(1);
      }
      //add budget account to budget
      budget.setAccount(budgetAcc, acc.id());
    }
  }
}

void MyMoneyForecast::setBudgetAccountList()
{
  //get budget accounts
  QList<MyMoneyAccount> accList;
  accList = budgetAccountList();

  QList<MyMoneyAccount>::const_iterator accList_t = accList.constBegin();
  for (; accList_t != accList.constEnd(); ++accList_t) {
    m_forecastAccounts.insert((*accList_t).id());
  }
}

QList<MyMoneyAccount> MyMoneyForecast::budgetAccountList()
{
  MyMoneyFile* file = MyMoneyFile::instance();

  QList<MyMoneyAccount> accList;
  QStringList emptyStringList;
  //Get all accounts from the file and check if they are of the right type to calculate forecast
  file->accountList(accList, emptyStringList, false);
  QList<MyMoneyAccount>::iterator accList_t = accList.begin();
  for (; accList_t != accList.end();) {
    MyMoneyAccount acc = *accList_t;
    if (acc.isClosed()            //check the account is not closed
        || (!acc.isIncomeExpense())) {
      //remove the account if it is not of the correct type
      accList_t = accList.erase(accList_t);
    } else {
      ++accList_t;
    }
  }
  return accList;
}

void MyMoneyForecast::calculateHistoricMonthlyBalances()
{
  MyMoneyFile* file = MyMoneyFile::instance();

  //Calculate account monthly balances
  QSet<QString>::ConstIterator it_n;
  for (it_n = m_forecastAccounts.constBegin(); it_n != m_forecastAccounts.constEnd(); ++it_n) {
    MyMoneyAccount acc = file->account(*it_n);

    for (QDate f_date = forecastStartDate(); f_date <= forecastEndDate();) {
      for (int f_day = 1; f_day <= accountsCycle() && f_date <= forecastEndDate(); ++f_day) {
        MyMoneyMoney accountDailyTrend = m_accountTrendList[acc.id()][f_day]; //trend for that day
        //check for leap year
        if (f_date.month() == 2 && f_date.day() == 29)
          f_date = f_date.addDays(1); //skip 1 day
        m_accountList[acc.id()][QDate(f_date.year(), f_date.month(), 1)] += accountDailyTrend; //movement trend for that particular day
        f_date = f_date.addDays(1);
      }
    }
  }
}

void MyMoneyForecast::calculateScheduledMonthlyBalances()
{
  MyMoneyFile* file = MyMoneyFile::instance();

//Calculate account monthly balances
  QSet<QString>::ConstIterator it_n;
  for (it_n = m_forecastAccounts.constBegin(); it_n != m_forecastAccounts.constEnd(); ++it_n) {
    MyMoneyAccount acc = file->account(*it_n);

    for (QDate f_date = forecastStartDate(); f_date <= forecastEndDate(); f_date = f_date.addDays(1)) {
      //get the trend for the day
      MyMoneyMoney accountDailyBalance = m_accountList[acc.id()][f_date];

      //do not add if it is the beginning of the month
      //otherwise we end up with duplicated values as reported by Marko Käning
      if (f_date != QDate(f_date.year(), f_date.month(), 1))
        m_accountList[acc.id()][QDate(f_date.year(), f_date.month(), 1)] += accountDailyBalance;
    }
  }
}

void MyMoneyForecast::setStartingBalance(const MyMoneyAccount &acc)
{
  MyMoneyFile* file = MyMoneyFile::instance();

  //Get current account balance
  if (acc.isInvest()) {   //investments require special treatment
    //get the security id of that account
    MyMoneySecurity undersecurity = file->security(acc.currencyId());

    //only do it if the security is not an actual currency
    if (! undersecurity.isCurrency()) {
      //set the default value
      MyMoneyMoney rate = MyMoneyMoney::ONE;
      //get te
      const MyMoneyPrice &price = file->price(undersecurity.id(), undersecurity.tradingCurrency(), QDate::currentDate());
      if (price.isValid()) {
        rate = price.rate(undersecurity.tradingCurrency());
      }
      m_accountList[acc.id()][QDate::currentDate()] = file->balance(acc.id(), QDate::currentDate()) * rate;
    }
  } else {
    m_accountList[acc.id()][QDate::currentDate()] = file->balance(acc.id(), QDate::currentDate());
  }

  //if the method is linear regression, we have to add the opening balance to m_accountListPast
  if (forecastMethod() == eHistoric && historyMethod() == 2) {
    //FIXME workaround for stock opening dates
    QDate openingDate;
    if (acc.accountType() == eMyMoney::Account::Stock) {
      MyMoneyAccount parentAccount = file->account(acc.parentAccountId());
      openingDate = parentAccount.openingDate();
    } else {
      openingDate = acc.openingDate();
    }

    //add opening balance only if it opened after the history start
    if (openingDate >= historyStartDate()) {

      MyMoneyMoney openingBalance;

      openingBalance = file->balance(acc.id(), openingDate);

      //calculate running sum
      for (QDate it_date = openingDate; it_date <= historyEndDate(); it_date = it_date.addDays(1)) {
        //investments require special treatment
        if (acc.isInvest()) {
          //get the security id of that account
          MyMoneySecurity undersecurity = file->security(acc.currencyId());

          //only do it if the security is not an actual currency
          if (! undersecurity.isCurrency()) {
            //set the default value
            MyMoneyMoney rate = MyMoneyMoney::ONE;

            //get the rate for that specific date
            const MyMoneyPrice &price = file->price(undersecurity.id(), undersecurity.tradingCurrency(), it_date);
            if (price.isValid()) {
              rate = price.rate(undersecurity.tradingCurrency());
            }
            m_accountListPast[acc.id()][it_date] += openingBalance * rate;
          }
        } else {
          m_accountListPast[acc.id()][it_date] += openingBalance;
        }
      }
    }
  }
}

void MyMoneyForecast::calculateAutoLoan(const MyMoneySchedule& schedule, MyMoneyTransaction& transaction, const QMap<QString, MyMoneyMoney>& balances)
{
  if (schedule.type() == eMyMoney::Schedule::Type::LoanPayment) {

    //get amortization and interest autoCalc splits
    MyMoneySplit amortizationSplit = transaction.amortizationSplit();
    MyMoneySplit interestSplit = transaction.interestSplit();
    const bool interestSplitValid = !interestSplit.id().isEmpty();

    if (!amortizationSplit.id().isEmpty()) {
      MyMoneyAccountLoan acc(MyMoneyFile::instance()->account(amortizationSplit.accountId()));
      MyMoneyFinancialCalculator calc;
      QDate dueDate;

      // FIXME: setup dueDate according to when the interest should be calculated
      // current implementation: take the date of the next payment according to
      // the schedule. If the calculation is based on the payment reception, and
      // the payment is overdue then take the current date
      dueDate = schedule.nextDueDate();
      if (acc.interestCalculation() == MyMoneyAccountLoan::paymentReceived) {
        if (dueDate < QDate::currentDate())
          dueDate = QDate::currentDate();
      }

      // we need to calculate the balance at the time the payment is due

      MyMoneyMoney balance;
      if (balances.count() == 0)
        balance = MyMoneyFile::instance()->balance(acc.id(), dueDate.addDays(-1));
      else
        balance = balances[acc.id()];

      // FIXME: for now, we only support interest calculation at the end of the period
      calc.setBep();
      // FIXME: for now, we only support periodic compounding
      calc.setDisc();

      calc.setPF(MyMoneySchedule::eventsPerYear(schedule.occurrence()));
      eMyMoney::Schedule::Occurrence compoundingOccurrence = static_cast<eMyMoney::Schedule::Occurrence>(acc.interestCompounding());
      if (compoundingOccurrence == eMyMoney::Schedule::Occurrence::Any)
        compoundingOccurrence = schedule.occurrence();

      calc.setCF(MyMoneySchedule::eventsPerYear(compoundingOccurrence));

      calc.setPv(balance.toDouble());
      calc.setIr(acc.interestRate(dueDate).abs().toDouble());
      calc.setPmt(acc.periodicPayment().toDouble());

      MyMoneyMoney interest(calc.interestDue(), 100), amortization;
      interest = interest.abs();    // make sure it's positive for now
      amortization = acc.periodicPayment() - interest;

      if (acc.accountType() == eMyMoney::Account::AssetLoan) {
        interest = -interest;
        amortization = -amortization;
      }

      amortizationSplit.setShares(amortization);
      if (interestSplitValid)
        interestSplit.setShares(interest);

      // FIXME: for now we only assume loans to be in the currency of the transaction
      amortizationSplit.setValue(amortization);
      if (interestSplitValid)
        interestSplit.setValue(interest);

      transaction.modifySplit(amortizationSplit);
      if (interestSplitValid)
        transaction.modifySplit(interestSplit);
    }
  }
}

