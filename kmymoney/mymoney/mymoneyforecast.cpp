/*
    SPDX-FileCopyrightText: 2007-2010 Alvaro Soliverez <asoliverez@gmail.com>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "mymoneyforecast.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>
#include <QList>
#include <QDebug>
#include <QDate>

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
#include "mymoneymoney.h"
#include "mymoneysplit.h"
#include "mymoneytransaction.h"
#include "mymoneytransactionfilter.h"
#include "mymoneyfinancialcalculator.h"
#include "mymoneyexception.h"
#include "mymoneyenums.h"

enum class eForecastMethod {Scheduled = 0, Historic = 1 };

/**
 * daily balances of an account
 */
typedef QMap<QDate, MyMoneyMoney> dailyBalances;

/**
 * map of trends of an account
 */
typedef QMap<int, MyMoneyMoney> trendBalances;

class MyMoneyForecastPrivate
{
  Q_DECLARE_PUBLIC(MyMoneyForecast)

public:
  explicit MyMoneyForecastPrivate(MyMoneyForecast *qq) :
    q_ptr(qq),
    m_accountsCycle(30),
    m_forecastCycles(3),
    m_forecastDays(90),
    m_beginForecastDay(0),
    m_forecastMethod(eForecastMethod::Scheduled),
    m_historyMethod(1),
    m_skipOpeningDate(true),
    m_includeUnusedAccounts(false),
    m_forecastDone(false),
    m_includeFutureTransactions(true),
    m_includeScheduledTransactions(true)
  {
  }

  eForecastMethod forecastMethod() const
  {
    return m_forecastMethod;
  }

  /**
   * Returns the list of accounts to create a budget. Only Income and Expenses are returned.
   */
  QList<MyMoneyAccount> budgetAccountList()
  {
    auto file = MyMoneyFile::instance();

    QList<MyMoneyAccount> accList;
    QStringList emptyStringList;
    //Get all accounts from the file and check if they are of the right type to calculate forecast
    file->accountList(accList, emptyStringList, false);
    QList<MyMoneyAccount>::iterator accList_t = accList.begin();
    for (; accList_t != accList.end();) {
      auto acc = *accList_t;
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

  /**
   * calculate daily forecast balance based on historic transactions
   */
  void calculateHistoricDailyBalances()
  {
    Q_Q(MyMoneyForecast);
    auto file = MyMoneyFile::instance();

    calculateAccountTrendList();

    //Calculate account daily balances
    QSet<QString>::ConstIterator it_n;
    for (it_n = m_forecastAccounts.constBegin(); it_n != m_forecastAccounts.constEnd(); ++it_n) {
      auto acc = file->account(*it_n);

      //set the starting balance of the account
      setStartingBalance(acc);

      switch (q->historyMethod()) {
      case 0:
      case 1: {
        for (QDate f_day = q->forecastStartDate(); f_day <= q->forecastEndDate();) {
          for (auto t_day = 1; t_day <= q->accountsCycle(); ++t_day) {
            MyMoneyMoney balanceDayBefore = m_accountList[acc.id()][(f_day.addDays(-1))];//balance of the day before
            MyMoneyMoney accountDailyTrend = m_accountTrendList[acc.id()][t_day]; //trend for that day
            //balance of the day is the balance of the day before multiplied by the trend for the day
            m_accountList[acc.id()][f_day] = balanceDayBefore;
            m_accountList[acc.id()][f_day] += accountDailyTrend; //movement trend for that particular day
            m_accountList[acc.id()][f_day] = m_accountList[acc.id()][f_day].convert(acc.fraction());
            //m_accountList[acc.id()][f_day] += m_accountListPast[acc.id()][f_day.addDays(-q->historyDays())];
            f_day = f_day.addDays(1);
          }
        }
      }
        break;
      case 2: {
        QDate baseDate = QDate::currentDate().addDays(-q->accountsCycle());
        for (auto t_day = 1; t_day <= q->accountsCycle(); ++t_day) {
          auto f_day = 1;
          QDate fDate = baseDate.addDays(q->accountsCycle() + 1);
          while (fDate <= q->forecastEndDate()) {

            //the calculation is based on the balance for the last month, that is then multiplied by the trend
            m_accountList[acc.id()][fDate] = m_accountListPast[acc.id()][baseDate] + (m_accountTrendList[acc.id()][t_day] * MyMoneyMoney(f_day, 1));
            m_accountList[acc.id()][fDate] = m_accountList[acc.id()][fDate].convert(acc.fraction());
            ++f_day;
            fDate = baseDate.addDays(q->accountsCycle() * f_day);
          }
          baseDate = baseDate.addDays(1);
        }
      }
      }
    }
  }

  /**
   * calculate monthly budget balance based on historic transactions
   */
  void calculateHistoricMonthlyBalances()
  {
    Q_Q(MyMoneyForecast);
    auto file = MyMoneyFile::instance();

    //Calculate account monthly balances
    QSet<QString>::ConstIterator it_n;
    for (it_n = m_forecastAccounts.constBegin(); it_n != m_forecastAccounts.constEnd(); ++it_n) {
      auto acc = file->account(*it_n);

      for (QDate f_date = q->forecastStartDate(); f_date <= q->forecastEndDate();) {
        for (auto f_day = 1; f_day <= q->accountsCycle() && f_date <= q->forecastEndDate(); ++f_day) {
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

  /**
   * calculate monthly budget balance based on historic transactions
   */
  void calculateScheduledMonthlyBalances()
  {
    Q_Q(MyMoneyForecast);
    auto file = MyMoneyFile::instance();

    //Calculate account monthly balances
    QSet<QString>::ConstIterator it_n;
    for (it_n = m_forecastAccounts.constBegin(); it_n != m_forecastAccounts.constEnd(); ++it_n) {
      auto acc = file->account(*it_n);

      for (QDate f_date = q->forecastStartDate(); f_date <= q->forecastEndDate(); f_date = f_date.addDays(1)) {
        //get the trend for the day
        MyMoneyMoney accountDailyBalance = m_accountList[acc.id()][f_date];

        //do not add if it is the beginning of the month
        //otherwise we end up with duplicated values as reported by Marko Käning
        if (f_date != QDate(f_date.year(), f_date.month(), 1))
          m_accountList[acc.id()][QDate(f_date.year(), f_date.month(), 1)] += accountDailyBalance;
      }
    }
  }

  /**
   * calculate forecast based on future and scheduled transactions
   */
  void doFutureScheduledForecast()
  {
    Q_Q(MyMoneyForecast);
    auto file = MyMoneyFile::instance();

    if (q->isIncludingFutureTransactions())
      addFutureTransactions();

    if (q->isIncludingScheduledTransactions())
      addScheduledTransactions();

    //do not show accounts with no transactions
    if (!q->isIncludingUnusedAccounts())
      purgeForecastAccountsList(m_accountList);

    //adjust value of investments to deep currency
    QSet<QString>::ConstIterator it_n;
    for (it_n = m_forecastAccounts.constBegin(); it_n != m_forecastAccounts.constEnd(); ++it_n) {
      auto acc = file->account(*it_n);

      if (acc.isInvest()) {
        //get the id of the security for that account
        MyMoneySecurity undersecurity = file->security(acc.currencyId());

        //only do it if the security is not an actual currency
        if (! undersecurity.isCurrency()) {
          //set the default value
          MyMoneyMoney rate = MyMoneyMoney::ONE;

          for (QDate it_day = QDate::currentDate(); it_day <= q->forecastEndDate();) {
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

  /**
   * add future transactions to forecast
   */
  void addFutureTransactions()
  {
    Q_Q(MyMoneyForecast);
    MyMoneyTransactionFilter filter;
    auto file = MyMoneyFile::instance();

    // collect and process all transactions that have already been entered but
    // are located in the future.
    filter.setDateFilter(q->forecastStartDate(), q->forecastEndDate());
    filter.setReportAllSplits(false);

    foreach (const auto transaction, file->transactionList(filter)) {
      foreach (const auto split, transaction.splits()) {
        if (!split.shares().isZero()) {
          auto acc = file->account(split.accountId());
          if (q->isForecastAccount(acc)) {
            dailyBalances balance;
            balance = m_accountList[acc.id()];
            //if it is income, the balance is stored as negative number
            if (acc.accountType() == eMyMoney::Account::Type::Income) {
              balance[transaction.postDate()] += (split.shares() * MyMoneyMoney::MINUS_ONE);
            } else {
              balance[transaction.postDate()] += split.shares();
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
        auto acc = file->account(*it_n);
        it_a = m_accountList.find(*it_n);
        s << "\"" << acc.name() << "\",";
        for (auto i = 0; i < 90; ++i) {
          s << "\"" << (*it_a)[i].formatMoney("") << "\",";
        }
        s << "\n";
      }
    }
#endif

  }

  /**
   * add scheduled transactions to forecast
   */
  void addScheduledTransactions()
  {
    Q_Q(MyMoneyForecast);
    auto file = MyMoneyFile::instance();

    // now process all the schedules that may have an impact
    QList<MyMoneySchedule> schedule;

    schedule = file->scheduleList(QString(), eMyMoney::Schedule::Type::Any, eMyMoney::Schedule::Occurrence::Any, eMyMoney::Schedule::PaymentType::Any,
                                  QDate(), q->forecastEndDate(), false);
    if (schedule.count() > 0) {
      QList<MyMoneySchedule>::Iterator it;
      do {
        std::sort(schedule.begin(), schedule.end());
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
        if (nextDate > q->forecastEndDate()) {
          // We're done with this schedule, let's move on to the next
          schedule.erase(it);
          continue;
        }

        // found the next schedule. process it

        auto acc = (*it).account();

        if (!acc.id().isEmpty()) {
          try {
            if (acc.accountType() != eMyMoney::Account::Type::Investment) {
              auto t = (*it).transaction();

              // only process the entry, if it is still active
              if (!(*it).isFinished() && nextDate != QDate()) {
                // make sure we have all 'starting balances' so that the autocalc works
                QMap<QString, MyMoneyMoney> balanceMap;

                foreach (const auto split, t.splits()) {
                  auto accountFromSplit = file->account(split.accountId());
                  if (q->isForecastAccount(accountFromSplit)) {
                    // collect all overdues on the first day
                    QDate forecastDate = nextDate;
                    if (QDate::currentDate() >= nextDate)
                      forecastDate = QDate::currentDate().addDays(1);

                    dailyBalances balance;
                    balance = m_accountList[accountFromSplit.id()];
                    for (QDate f_day = QDate::currentDate(); f_day < forecastDate;) {
                      balanceMap[accountFromSplit.id()] += m_accountList[accountFromSplit.id()][f_day];
                      f_day = f_day.addDays(1);
                    }
                  }
                }

                // take care of the autoCalc stuff
                q->calculateAutoLoan(*it, t, balanceMap);

                // now add the splits to the balances
                foreach (const auto split, t.splits()) {
                  auto accountFromSplit = file->account(split.accountId());
                  if (q->isForecastAccount(accountFromSplit)) {
                    dailyBalances balance;
                    balance = m_accountList[accountFromSplit.id()];
                    //auto offset = QDate::currentDate().daysTo(nextDate);
                    //if(offset <= 0) {  // collect all overdues on the first day
                    //  offset = 1;
                    //}
                    // collect all overdues on the first day
                    QDate forecastDate = nextDate;
                    if (QDate::currentDate() >= nextDate)
                      forecastDate = QDate::currentDate().addDays(1);

                    if (accountFromSplit.accountType() == eMyMoney::Account::Type::Income) {
                      balance[forecastDate] += (split.shares() * MyMoneyMoney::MINUS_ONE);
                    } else {
                      balance[forecastDate] += split.shares();
                    }
                    m_accountList[accountFromSplit.id()] = balance;
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
        auto acc = file->account(*it_n);
        it_a = m_accountList.find(*it_n);
        s << "\"" << acc.name() << "\",";
        for (auto i = 0; i < 90; ++i) {
          s << "\"" << (*it_a)[i].formatMoney("") << "\",";
        }
        s << "\n";
      }
    }
#endif
  }

  /**
   * calculate daily forecast balance based on future and scheduled transactions
   */
  void calculateScheduledDailyBalances()
  {
    Q_Q(MyMoneyForecast);
    auto file = MyMoneyFile::instance();

    //Calculate account daily balances
    QSet<QString>::ConstIterator it_n;
    for (it_n = m_forecastAccounts.constBegin(); it_n != m_forecastAccounts.constEnd(); ++it_n) {
      auto acc = file->account(*it_n);

      //set the starting balance of the account
      setStartingBalance(acc);

      for (QDate f_day = q->forecastStartDate(); f_day <= q->forecastEndDate();) {
        MyMoneyMoney balanceDayBefore = m_accountList[acc.id()][(f_day.addDays(-1))];//balance of the day before
        m_accountList[acc.id()][f_day] += balanceDayBefore; //running sum
        f_day = f_day.addDays(1);
      }
    }
  }

  /**
   * set the starting balance for an accounts
   */
  void setStartingBalance(const MyMoneyAccount& acc)
  {
    Q_Q(MyMoneyForecast);
    auto file = MyMoneyFile::instance();

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
    if (forecastMethod() == eForecastMethod::Historic && q->historyMethod() == 2) {
      //FIXME workaround for stock opening dates
      QDate openingDate;
      if (acc.accountType() == eMyMoney::Account::Type::Stock) {
        auto parentAccount = file->account(acc.parentAccountId());
        openingDate = parentAccount.openingDate();
      } else {
        openingDate = acc.openingDate();
      }

      //add opening balance only if it opened after the history start
      if (openingDate >= q->historyStartDate()) {

        MyMoneyMoney openingBalance;

        openingBalance = file->balance(acc.id(), openingDate);

        //calculate running sum
        for (QDate it_date = openingDate; it_date <= q->historyEndDate(); it_date = it_date.addDays(1)) {
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

  /**
   * Returns the day moving average for the account @a acc based on the daily balances of a given number of @p forecastTerms
   * It returns the moving average for a given @p trendDay of the forecastTerm
   * With a term of 1 month and 3 terms, it calculates the trend taking the transactions occurred
   * at that day and the day before,for the last 3 months
   */
  MyMoneyMoney accountMovingAverage(const MyMoneyAccount& acc, const qint64 trendDay, const int forecastTerms)
  {
    Q_Q(MyMoneyForecast);
    //Calculate a daily trend for the account based on the accounts of a given number of terms
    //With a term of 1 month and 3 terms, it calculates the trend taking the transactions occurred at that day and the day before,
    //for the last 3 months
    MyMoneyMoney balanceVariation;

    for (auto it_terms = 0; (trendDay + (q->accountsCycle()*it_terms)) <= q->historyDays(); ++it_terms) { //sum for each term
      MyMoneyMoney balanceBefore = m_accountListPast[acc.id()][q->historyStartDate().addDays(trendDay+(q->accountsCycle()*it_terms)-2)]; //get balance for the day before
      MyMoneyMoney balanceAfter = m_accountListPast[acc.id()][q->historyStartDate().addDays(trendDay+(q->accountsCycle()*it_terms)-1)];
      balanceVariation += (balanceAfter - balanceBefore); //add the balance variation between days
    }
    //calculate average of the variations
    return (balanceVariation / MyMoneyMoney(forecastTerms, 1)).convert(10000);
  }

  /**
   * Returns the weighted moving average for a given @p trendDay
   */
  MyMoneyMoney accountWeightedMovingAverage(const MyMoneyAccount& acc, const qint64 trendDay, const int totalWeight)
  {
    Q_Q(MyMoneyForecast);
    MyMoneyMoney balanceVariation;

    for (auto it_terms = 0, weight = 1; (trendDay + (q->accountsCycle()*it_terms)) <= q->historyDays(); ++it_terms, ++weight) { //sum for each term multiplied by weight
      MyMoneyMoney balanceBefore = m_accountListPast[acc.id()][q->historyStartDate().addDays(trendDay+(q->accountsCycle()*it_terms)-2)]; //get balance for the day before
      MyMoneyMoney balanceAfter = m_accountListPast[acc.id()][q->historyStartDate().addDays(trendDay+(q->accountsCycle()*it_terms)-1)];
      balanceVariation += ((balanceAfter - balanceBefore) * MyMoneyMoney(weight, 1));   //add the balance variation between days multiplied by its weight
    }
    //calculate average of the variations
    return (balanceVariation / MyMoneyMoney(totalWeight, 1)).convert(10000);
  }

  /**
   * Returns the linear regression for a given @p trendDay
   */
  MyMoneyMoney accountLinearRegression(const MyMoneyAccount &acc, const qint64 trendDay, const qint64 actualTerms, const MyMoneyMoney& meanTerms)
  {
    Q_Q(MyMoneyForecast);
    MyMoneyMoney meanBalance, totalBalance, totalTerms;
    totalTerms = MyMoneyMoney(actualTerms, 1);

    //calculate mean balance
    for (auto it_terms = q->forecastCycles() - actualTerms; (trendDay + (q->accountsCycle()*it_terms)) <= q->historyDays(); ++it_terms) { //sum for each term
      totalBalance += m_accountListPast[acc.id()][q->historyStartDate().addDays(trendDay+(q->accountsCycle()*it_terms)-1)];
    }
    meanBalance = totalBalance / MyMoneyMoney(actualTerms, 1);
    meanBalance = meanBalance.convert(10000);

    //calculate b1

    //first calculate x - mean x multiplied by y - mean y
    MyMoneyMoney totalXY, totalSqX;
    auto term = 1;
    for (auto it_terms = q->forecastCycles() - actualTerms; (trendDay + (q->accountsCycle()*it_terms)) <= q->historyDays(); ++it_terms, ++term) { //sum for each term
      MyMoneyMoney balance = m_accountListPast[acc.id()][q->historyStartDate().addDays(trendDay+(q->accountsCycle()*it_terms)-1)];

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

  /**
   * calculate daily forecast trend based on historic transactions
   */
  void calculateAccountTrendList()
  {
    Q_Q(MyMoneyForecast);
    auto file = MyMoneyFile::instance();
    qint64 auxForecastTerms;
    qint64 totalWeight = 0;

    //Calculate account trends
    QSet<QString>::ConstIterator it_n;
    for (it_n = m_forecastAccounts.begin(); it_n != m_forecastAccounts.end(); ++it_n) {
      auto acc = file->account(*it_n);
      m_accountTrendList[acc.id()][0] = MyMoneyMoney(); // for today, the trend is 0

      auxForecastTerms = q->forecastCycles();
      if (q->skipOpeningDate()) {

        QDate openingDate;
        if (acc.accountType() == eMyMoney::Account::Type::Stock) {
          auto parentAccount = file->account(acc.parentAccountId());
          openingDate = parentAccount.openingDate();
        } else {
          openingDate = acc.openingDate();
        }

        if (openingDate > q->historyStartDate()) { //if acc opened after forecast period
          auxForecastTerms = 1 + ((openingDate.daysTo(q->historyEndDate()) + 1) / q->accountsCycle()); // set forecastTerms to a lower value, to calculate only based on how long this account was opened
        }
      }

      switch (q->historyMethod()) {
      //moving average
      case 0: {
        for (auto t_day = 1; t_day <= q->accountsCycle(); ++t_day)
          m_accountTrendList[acc.id()][t_day] = accountMovingAverage(acc, t_day, auxForecastTerms); //moving average
        break;
      }
        //weighted moving average
      case 1: {
        //calculate total weight for moving average
        if (auxForecastTerms == q->forecastCycles()) {
          totalWeight = (auxForecastTerms * (auxForecastTerms + 1)) / 2; //totalWeight is the triangular number of auxForecastTerms
        } else {
          //if only taking a few periods, totalWeight is the sum of the weight for most recent periods
          auto i = 1;
          for (qint64 w = q->forecastCycles(); i <= auxForecastTerms; ++i, --w)
            totalWeight += w;
        }
        for (auto t_day = 1; t_day <= q->accountsCycle(); ++t_day)
          m_accountTrendList[acc.id()][t_day] = accountWeightedMovingAverage(acc, t_day, totalWeight);
        break;
      }
      case 2: {
        //calculate mean term
        MyMoneyMoney meanTerms = MyMoneyMoney((auxForecastTerms * (auxForecastTerms + 1)) / 2, 1) / MyMoneyMoney(auxForecastTerms, 1);

        for (auto t_day = 1; t_day <= q->accountsCycle(); ++t_day)
          m_accountTrendList[acc.id()][t_day] = accountLinearRegression(acc, t_day, auxForecastTerms, meanTerms);
        break;
      }
      default:
        break;
      }
    }
  }

  /**
   * set the internal list of accounts to be forecast
   */
  void setForecastAccountList()
  {
    Q_Q(MyMoneyForecast);
    //get forecast accounts
    QList<MyMoneyAccount> accList;
    accList = q->forecastAccountList();

    QList<MyMoneyAccount>::const_iterator accList_t = accList.constBegin();
    for (; accList_t != accList.constEnd(); ++accList_t) {
      m_forecastAccounts.insert((*accList_t).id());
    }
  }

  /**
   * set the internal list of accounts to create a budget
   */
  void setBudgetAccountList()
  {
    //get budget accounts
    QList<MyMoneyAccount> accList;
    accList = budgetAccountList();

    QList<MyMoneyAccount>::const_iterator accList_t = accList.constBegin();
    for (; accList_t != accList.constEnd(); ++accList_t) {
      m_forecastAccounts.insert((*accList_t).id());
    }
  }

  /**
   * get past transactions for the accounts to be forecast
   */
  void pastTransactions()
  {
    Q_Q(MyMoneyForecast);
    auto file = MyMoneyFile::instance();
    MyMoneyTransactionFilter filter;

    filter.setDateFilter(q->historyStartDate(), q->historyEndDate());
    filter.setReportAllSplits(false);

    //Check past transactions
    foreach (const auto transaction, file->transactionList(filter)) {
      foreach (const auto split, transaction.splits()) {
        if (!split.shares().isZero()) {
          auto acc = file->account(split.accountId());

          //workaround for stock accounts which have faulty opening dates
          QDate openingDate;
          if (acc.accountType() == eMyMoney::Account::Type::Stock) {
            auto parentAccount = file->account(acc.parentAccountId());
            openingDate = parentAccount.openingDate();
          } else {
            openingDate = acc.openingDate();
          }

          if (q->isForecastAccount(acc) //If it is one of the accounts we are checking, add the amount of the transaction
              && ((openingDate < transaction.postDate() && q->skipOpeningDate())
                  || !q->skipOpeningDate())) {  //don't take the opening day of the account to calculate balance
            dailyBalances balance;
            //FIXME deal with leap years
            balance = m_accountListPast[acc.id()];
            if (acc.accountType() == eMyMoney::Account::Type::Income) {//if it is income, the balance is stored as negative number
              balance[transaction.postDate()] += (split.shares() * MyMoneyMoney::MINUS_ONE);
            } else {
              balance[transaction.postDate()] += split.shares();
            }
            // check if this is a new account for us
            m_accountListPast[acc.id()] = balance;
          }
        }
      }
    }

    //purge those accounts with no transactions on the period
    if (q->isIncludingUnusedAccounts() == false)
      purgeForecastAccountsList(m_accountListPast);

    //calculate running sum
    QSet<QString>::ConstIterator it_n;
    for (it_n = m_forecastAccounts.begin(); it_n != m_forecastAccounts.end(); ++it_n) {
      auto acc = file->account(*it_n);
      m_accountListPast[acc.id()][q->historyStartDate().addDays(-1)] = file->balance(acc.id(), q->historyStartDate().addDays(-1));
      for (QDate it_date = q->historyStartDate(); it_date <= q->historyEndDate();) {
        m_accountListPast[acc.id()][it_date] += m_accountListPast[acc.id()][it_date.addDays(-1)]; //Running sum
        it_date = it_date.addDays(1);
      }
    }

    //adjust value of investments to deep currency
    for (it_n = m_forecastAccounts.begin(); it_n != m_forecastAccounts.end(); ++it_n) {
      auto acc = file->account(*it_n);

      if (acc.isInvest()) {
        //get the id of the security for that account
        MyMoneySecurity undersecurity = file->security(acc.currencyId());
        if (! undersecurity.isCurrency()) { //only do it if the security is not an actual currency
          MyMoneyMoney rate = MyMoneyMoney::ONE;    //set the default value

          for (QDate it_date = q->historyStartDate().addDays(-1) ; it_date <= q->historyEndDate();) {
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
  qint64 calculateBeginForecastDay()
  {
    Q_Q(MyMoneyForecast);
    auto fDays = q->forecastDays();
    auto beginDay = q->beginForecastDay();
    auto accCycle = q->accountsCycle();
    QDate beginDate;

    //if 0, beginDate is current date and forecastDays remains unchanged
    if (beginDay == 0) {
      q->setBeginForecastDate(QDate::currentDate());
      return fDays;
    }

    //adjust if beginDay more than days of current month
    if (QDate::currentDate().daysInMonth() < beginDay)
      beginDay = QDate::currentDate().daysInMonth();

    //if beginDay still to come, calculate and return
    if (QDate::currentDate().day() <= beginDay) {
      beginDate = QDate(QDate::currentDate().year(), QDate::currentDate().month(), beginDay);
      fDays += QDate::currentDate().daysTo(beginDate);
      q->setBeginForecastDate(beginDate);
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

    q->setBeginForecastDate(beginDate);
    return fDays;
  }

  /**
   * remove accounts from the list if the accounts has no transactions in the forecast timeframe.
   * Used for scheduled-forecast method.
   */
  void purgeForecastAccountsList(QMap<QString, dailyBalances>& accountList)
  {
    m_forecastAccounts.intersect(accountList.keys().toSet());
  }

  MyMoneyForecast *q_ptr;

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
  qint64 m_accountsCycle;

  /**
   * number of cycles to use in forecast
   */
  qint64 m_forecastCycles;

  /**
   * number of days to forecast
   */
  qint64 m_forecastDays;

  /**
   * date to start forecast
   */
  QDate m_beginForecastDate;

  /**
   * day to start forecast
   */
  qint64 m_beginForecastDay;

  /**
   * forecast method
   */
  eForecastMethod m_forecastMethod;

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

MyMoneyForecast::MyMoneyForecast() :
  d_ptr(new MyMoneyForecastPrivate(this))
{
  setHistoryStartDate(QDate::currentDate().addDays(-forecastCycles()*accountsCycle()));
  setHistoryEndDate(QDate::currentDate().addDays(-1));
}

MyMoneyForecast::MyMoneyForecast(const MyMoneyForecast& other) :
  d_ptr(new MyMoneyForecastPrivate(*other.d_func()))
{
  this->d_ptr->q_ptr = this;
}

void swap(MyMoneyForecast& first, MyMoneyForecast& second)
{
  using std::swap;
  swap(first.d_ptr, second.d_ptr);
  swap(first.d_ptr->q_ptr, second.d_ptr->q_ptr);
}

MyMoneyForecast::MyMoneyForecast(MyMoneyForecast && other) : MyMoneyForecast()
{
  swap(*this, other);
}

MyMoneyForecast & MyMoneyForecast::operator=(MyMoneyForecast other)
{
  swap(*this, other);
  return *this;
}

MyMoneyForecast::~MyMoneyForecast()
{
  Q_D(MyMoneyForecast);
  delete d;
}

void MyMoneyForecast::doForecast()
{
  Q_D(MyMoneyForecast);
  auto fDays = d->calculateBeginForecastDay();
  auto fMethod = d->forecastMethod();
  auto fAccCycle = accountsCycle();
  auto fCycles = forecastCycles();

  //validate settings
  if (fAccCycle < 1
      || fCycles < 1
      || fDays < 1) {
    throw MYMONEYEXCEPTION_CSTRING("Illegal settings when calling doForecast. Settings must be higher than 0");
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
  d->m_accountListPast.clear();
  d->m_accountList.clear();
  d->m_accountTrendList.clear();

  //set forecast accounts
  d->setForecastAccountList();

  switch (fMethod) {
  case eForecastMethod::Scheduled:
    d->doFutureScheduledForecast();
    d->calculateScheduledDailyBalances();
    break;
  case eForecastMethod::Historic:
    d->pastTransactions();
    d->calculateHistoricDailyBalances();
    break;
  default:
    break;
  }

  //flag the forecast as done
  d->m_forecastDone = true;
}

bool MyMoneyForecast::isForecastAccount(const MyMoneyAccount& acc)
{
  Q_D(MyMoneyForecast);
  if (d->m_forecastAccounts.isEmpty()) {
    d->setForecastAccountList();
  }
  return d->m_forecastAccounts.contains(acc.id());
}

QList<MyMoneyAccount> MyMoneyForecast::accountList()
{
  auto file = MyMoneyFile::instance();

  QList<MyMoneyAccount> accList;
  QStringList emptyStringList;
  //Get all accounts from the file and check if they are present
  file->accountList(accList, emptyStringList, false);
  QList<MyMoneyAccount>::iterator accList_t = accList.begin();
  for (; accList_t != accList.end();) {
    auto acc = *accList_t;
    if (!isForecastAccount(acc)) {
      accList_t = accList.erase(accList_t);    //remove the account
    } else {
      ++accList_t;
    }
  }
  return accList;
}

MyMoneyMoney MyMoneyForecast::calculateAccountTrend(const MyMoneyAccount& acc, qint64 trendDays)
{
  auto file = MyMoneyFile::instance();
  MyMoneyTransactionFilter filter;
  MyMoneyMoney netIncome;
  QDate startDate;
  QDate openingDate = acc.openingDate();

  //validate arguments
  if (trendDays < 1) {
    throw MYMONEYEXCEPTION_CSTRING("Illegal arguments when calling calculateAccountTrend. trendDays must be higher than 0");
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
  if (acc.accountGroup() == eMyMoney::Account::Type::Income
      || acc.accountGroup() == eMyMoney::Account::Type::Expense) {
    filter.addCategory(acc.id());
  } else {
    filter.addAccount(acc.id());
  }

  filter.setReportAllSplits(false);

  //add all transactions for that account
  foreach (const auto transaction, file->transactionList(filter)) {
    foreach (const auto split, transaction.splits()) {
      if (!split.shares().isZero()) {
        if (acc.id() == split.accountId()) netIncome += split.value();
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

MyMoneyMoney MyMoneyForecast::forecastBalance(const MyMoneyAccount& acc, const QDate &forecastDate)
{
  Q_D(MyMoneyForecast);
  dailyBalances balance;
  MyMoneyMoney MM_amount = MyMoneyMoney();

  //Check if acc is not a forecast account, return 0
  if (!isForecastAccount(acc)) {
    return MM_amount;
  }

  if (d->m_accountList.contains(acc.id())) {
    balance = d->m_accountList.value(acc.id());
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
MyMoneyMoney MyMoneyForecast::forecastBalance(const MyMoneyAccount& acc, qint64 offset)
{
  QDate forecastDate = QDate::currentDate().addDays(offset);
  return forecastBalance(acc, forecastDate);
}

qint64 MyMoneyForecast::daysToMinimumBalance(const MyMoneyAccount& acc)
{
  Q_D(MyMoneyForecast);
  QString minimumBalance = acc.value("minBalanceAbsolute");
  MyMoneyMoney minBalance = MyMoneyMoney(minimumBalance);
  dailyBalances balance;

  //Check if acc is not a forecast account, return -1
  if (!isForecastAccount(acc)) {
    return -1;
  }

  balance = d->m_accountList[acc.id()];

  for (QDate it_day = QDate::currentDate() ; it_day <= forecastEndDate();) {
    if (minBalance > balance[it_day]) {
      return QDate::currentDate().daysTo(it_day);
    }
    it_day = it_day.addDays(1);
  }
  return -1;
}

qint64 MyMoneyForecast::daysToZeroBalance(const MyMoneyAccount& acc)
{
  Q_D(MyMoneyForecast);
  dailyBalances balance;

  //Check if acc is not a forecast account, return -1
  if (!isForecastAccount(acc)) {
    return -2;
  }

  balance = d->m_accountList[acc.id()];

  if (acc.accountGroup() == eMyMoney::Account::Type::Asset) {
    for (QDate it_day = QDate::currentDate() ; it_day <= forecastEndDate();) {
      if (balance[it_day] < MyMoneyMoney()) {
        return QDate::currentDate().daysTo(it_day);
      }
      it_day = it_day.addDays(1);
    }
  } else if (acc.accountGroup() == eMyMoney::Account::Type::Liability) {
    for (QDate it_day = QDate::currentDate() ; it_day <= forecastEndDate();) {
      if (balance[it_day] > MyMoneyMoney()) {
        return QDate::currentDate().daysTo(it_day);
      }
      it_day = it_day.addDays(1);
    }
  }
  return -1;
}


MyMoneyMoney MyMoneyForecast::accountCycleVariation(const MyMoneyAccount& acc)
{
  Q_D(MyMoneyForecast);
  MyMoneyMoney cycleVariation;

  if (d->forecastMethod() == eForecastMethod::Historic) {
    switch (historyMethod()) {
    case 0:
    case 1: {
      for (auto t_day = 1; t_day <= accountsCycle() ; ++t_day) {
        cycleVariation += d->m_accountTrendList[acc.id()][t_day];
      }
    }
      break;
    case 2: {
      cycleVariation = d->m_accountList[acc.id()][QDate::currentDate().addDays(accountsCycle())] - d->m_accountList[acc.id()][QDate::currentDate()];
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
  qint64 daysToBeginDay;

  daysToBeginDay = QDate::currentDate().daysTo(beginForecastDate());

  for (auto t_cycle = 0; ((t_cycle * accountsCycle()) + daysToBeginDay) < forecastDays() ; ++t_cycle) {
    MyMoneyMoney minBalance = forecastBalance(acc, (t_cycle * accountsCycle() + daysToBeginDay));
    QDate minDate = QDate::currentDate().addDays(t_cycle * accountsCycle() + daysToBeginDay);
    for (auto t_day = 1; t_day <= accountsCycle() ; ++t_day) {
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
  qint64 daysToBeginDay;

  daysToBeginDay = QDate::currentDate().daysTo(beginForecastDate());

  for (auto t_cycle = 0; ((t_cycle * accountsCycle()) + daysToBeginDay) < forecastDays() ; ++t_cycle) {
    MyMoneyMoney maxBalance = forecastBalance(acc, ((t_cycle * accountsCycle()) + daysToBeginDay));
    QDate maxDate = QDate::currentDate().addDays((t_cycle * accountsCycle()) + daysToBeginDay);

    for (auto t_day = 0; t_day < accountsCycle() ; ++t_day) {
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
  for (auto f_day = 1; f_day <= forecastDays() ; ++f_day) {
    totalBalance += forecastBalance(acc, f_day);
  }
  return totalBalance / MyMoneyMoney(forecastDays(), 1);
}

void MyMoneyForecast::createBudget(MyMoneyBudget& budget, QDate historyStart, QDate historyEnd, QDate budgetStart, QDate budgetEnd, const bool returnBudget)
{
  Q_D(MyMoneyForecast);
  // clear all data except the id and name
  QString name = budget.name();
  budget = MyMoneyBudget(budget.id(), MyMoneyBudget());
  budget.setName(name);

  //check parameters
  if (historyStart > historyEnd ||
      budgetStart > budgetEnd ||
      budgetStart <= historyEnd) {
    throw MYMONEYEXCEPTION_CSTRING("Illegal parameters when trying to create budget");
  }

  //get forecast method
  auto fMethod = d->forecastMethod();

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
  d->m_forecastAccounts.clear();
  d->setBudgetAccountList();

  //calculate budget according to forecast method
  switch (fMethod) {
  case eForecastMethod::Scheduled:
    d->doFutureScheduledForecast();
    d->calculateScheduledMonthlyBalances();
    break;
  case eForecastMethod::Historic:
    d->pastTransactions(); //get all transactions for history period
    d->calculateAccountTrendList();
    d->calculateHistoricMonthlyBalances(); //add all balances of each month and put at the 1st day of each month
    break;
  default:
    break;
  }

  //flag the forecast as done
  d->m_forecastDone = true;

  //only fill the budget if it is going to be used
  if (returnBudget) {
    //setup the budget itself
    auto file = MyMoneyFile::instance();
    budget.setBudgetStart(budgetStart);

    //go through all the accounts and add them to budget
    for (auto it_nc = d->m_forecastAccounts.constBegin(); it_nc != d->m_forecastAccounts.constEnd(); ++it_nc) {
      auto acc = file->account(*it_nc);

      MyMoneyBudget::AccountGroup budgetAcc;
      budgetAcc.setId(acc.id());
      budgetAcc.setBudgetLevel(eMyMoney::Budget::Level::MonthByMonth);

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
qint64 MyMoneyForecast::historyDays() const
{
  Q_D(const MyMoneyForecast);
  return (d->m_historyStartDate.daysTo(d->m_historyEndDate) + 1);
}

void MyMoneyForecast::setAccountsCycle(qint64 accountsCycle)
{
  Q_D(MyMoneyForecast);
  d->m_accountsCycle = accountsCycle;
}

void MyMoneyForecast::setForecastCycles(qint64 forecastCycles)
{
  Q_D(MyMoneyForecast);
  d->m_forecastCycles = forecastCycles;
}

void MyMoneyForecast::setForecastDays(qint64 forecastDays)
{
  Q_D(MyMoneyForecast);
  d->m_forecastDays = forecastDays;
}

void MyMoneyForecast::setBeginForecastDate(const QDate &beginForecastDate)
{
  Q_D(MyMoneyForecast);
  d->m_beginForecastDate = beginForecastDate;
}

void MyMoneyForecast::setBeginForecastDay(qint64 beginDay)
{
  Q_D(MyMoneyForecast);
  d->m_beginForecastDay = beginDay;
}

void MyMoneyForecast::setForecastMethod(qint64 forecastMethod)
{
  Q_D(MyMoneyForecast);
  d->m_forecastMethod = static_cast<eForecastMethod>(forecastMethod);
}

void MyMoneyForecast::setHistoryStartDate(const QDate &historyStartDate)
{
  Q_D(MyMoneyForecast);
  d->m_historyStartDate = historyStartDate;
}

void MyMoneyForecast::setHistoryEndDate(const QDate &historyEndDate)
{
  Q_D(MyMoneyForecast);
  d->m_historyEndDate = historyEndDate;
}

void MyMoneyForecast::setHistoryStartDate(qint64 daysToStartDate)
{
  setHistoryStartDate(QDate::currentDate().addDays(-daysToStartDate));
}

void MyMoneyForecast::setHistoryEndDate(qint64 daysToEndDate)
{
  setHistoryEndDate(QDate::currentDate().addDays(-daysToEndDate));
}

void MyMoneyForecast::setForecastStartDate(const QDate &_startDate)
{
  Q_D(MyMoneyForecast);
  d->m_forecastStartDate = _startDate;
}

void MyMoneyForecast::setForecastEndDate(const QDate &_endDate)
{
  Q_D(MyMoneyForecast);
  d->m_forecastEndDate = _endDate;
}

void MyMoneyForecast::setSkipOpeningDate(bool _skip)
{
  Q_D(MyMoneyForecast);
  d->m_skipOpeningDate = _skip;
}

void MyMoneyForecast::setHistoryMethod(int historyMethod)
{
  Q_D(MyMoneyForecast);
  d->m_historyMethod = historyMethod;
}

void MyMoneyForecast::setIncludeUnusedAccounts(bool _bool)
{
  Q_D(MyMoneyForecast);
  d->m_includeUnusedAccounts = _bool;
}

void MyMoneyForecast::setForecastDone(bool _bool)
{
  Q_D(MyMoneyForecast);
  d->m_forecastDone = _bool;
}

void MyMoneyForecast::setIncludeFutureTransactions(bool _bool)
{
  Q_D(MyMoneyForecast);
  d->m_includeFutureTransactions = _bool;
}

void MyMoneyForecast::setIncludeScheduledTransactions(bool _bool)
{
  Q_D(MyMoneyForecast);
  d->m_includeScheduledTransactions = _bool;
}

qint64 MyMoneyForecast::accountsCycle() const
{
  Q_D(const MyMoneyForecast);
  return d->m_accountsCycle;
}

qint64 MyMoneyForecast::forecastCycles() const
{
  Q_D(const MyMoneyForecast);
  return d->m_forecastCycles;
}

qint64 MyMoneyForecast::forecastDays() const
{
  Q_D(const MyMoneyForecast);
  return d->m_forecastDays;
}

QDate MyMoneyForecast::beginForecastDate() const
{
  Q_D(const MyMoneyForecast);
  return d->m_beginForecastDate;
}

qint64 MyMoneyForecast::beginForecastDay() const
{
  Q_D(const MyMoneyForecast);
  return d->m_beginForecastDay;
}

QDate MyMoneyForecast::historyStartDate() const
{
  Q_D(const MyMoneyForecast);
  return d->m_historyStartDate;
}

QDate MyMoneyForecast::historyEndDate() const
{
  Q_D(const MyMoneyForecast);
  return d->m_historyEndDate;
}

QDate MyMoneyForecast::forecastStartDate() const
{
  Q_D(const MyMoneyForecast);
  return d->m_forecastStartDate;
}

QDate MyMoneyForecast::forecastEndDate() const
{
  Q_D(const MyMoneyForecast);
  return d->m_forecastEndDate;
}

bool MyMoneyForecast::skipOpeningDate() const
{
  Q_D(const MyMoneyForecast);
  return d->m_skipOpeningDate;
}

int MyMoneyForecast::historyMethod() const
{
  Q_D(const MyMoneyForecast);
  return d->m_historyMethod;
}

bool MyMoneyForecast::isIncludingUnusedAccounts() const
{
  Q_D(const MyMoneyForecast);
  return d->m_includeUnusedAccounts;
}

bool MyMoneyForecast::isForecastDone() const
{
  Q_D(const MyMoneyForecast);
  return d->m_forecastDone;
}

bool MyMoneyForecast::isIncludingFutureTransactions() const
{
  Q_D(const MyMoneyForecast);
  return d->m_includeFutureTransactions;
}

bool MyMoneyForecast::isIncludingScheduledTransactions() const
{
  Q_D(const MyMoneyForecast);
  return d->m_includeScheduledTransactions;
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

      calc.setPF(MyMoneySchedule::eventsPerYear(schedule.baseOccurrence()));
      eMyMoney::Schedule::Occurrence compoundingOccurrence = static_cast<eMyMoney::Schedule::Occurrence>(acc.interestCompounding());
      if (compoundingOccurrence == eMyMoney::Schedule::Occurrence::Any)
        compoundingOccurrence = schedule.baseOccurrence();

      calc.setCF(MyMoneySchedule::eventsPerYear(compoundingOccurrence));

      calc.setPv(balance.toDouble());
      calc.setIr(acc.interestRate(dueDate).abs().toDouble());
      calc.setPmt(acc.periodicPayment().toDouble());

      MyMoneyMoney interest(calc.interestDue(), 100), amortization;
      interest = interest.abs();    // make sure it's positive for now
      amortization = acc.periodicPayment() - interest;

      if (acc.accountType() == eMyMoney::Account::Type::AssetLoan) {
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

QList<MyMoneyAccount> MyMoneyForecast::forecastAccountList()
{
  auto file = MyMoneyFile::instance();

  QList<MyMoneyAccount> accList;
  //Get all accounts from the file and check if they are of the right type to calculate forecast
  file->accountList(accList);
  QList<MyMoneyAccount>::iterator accList_t = accList.begin();
  for (; accList_t != accList.end();) {
    auto acc = *accList_t;
    if (acc.isClosed()            //check the account is not closed
        || (!acc.isAssetLiability())) {
      //|| (acc.accountType() == eMyMoney::Account::Type::Investment) ) {//check that it is not an Investment account and only include Stock accounts
      //remove the account if it is not of the correct type
      accList_t = accList.erase(accList_t);
    } else {
      ++accList_t;
    }
  }
  return accList;
}
