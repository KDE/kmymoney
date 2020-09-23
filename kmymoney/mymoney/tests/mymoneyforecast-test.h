/*
 * Copyright 2007-2010  Alvaro Soliverez <asoliverez@gmail.com>
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

#ifndef MYMONEYFORECASTTEST_H
#define MYMONEYFORECASTTEST_H

#include <QObject>

#define KMM_MYMONEY_UNIT_TESTABLE friend class MyMoneyForecastTest;

#include "mymoneyfile.h"
#include "mymoneymoney.h"

#include "mymoneyforecast.h"

class MyMoneyForecastTest : public QObject
{
  Q_OBJECT
public:
  MyMoneyForecastTest();

private Q_SLOTS:
  void init();
  void cleanup();
  void testEmptyConstructor();
  void testDoForecast();
  void testDoForecastInit();
  void testGetForecastAccountList();
  void testCalculateAccountTrend();
  void testGetForecastBalance();
  void testIsForecastAccount();
  void testDoFutureScheduledForecast();
  void testDaysToMinimumBalance();
  void testDaysToZeroBalance();
  void testScheduleForecast();
  void testSkipOpeningDate();
  void testAccountMinimumBalanceDateList();
  void testAccountMaximumBalanceDateList();
  void testAccountAverageBalance();
  void testBeginForecastDate();
  void testHistoryDays();
  void testCreateBudget();
  void testLinearRegression();

protected:
  MyMoneyForecast *m;

private:
  MyMoneyStorageMgr* storage;
  MyMoneyFile* file;

  MyMoneyMoney moT1;
  MyMoneyMoney moT2;
  MyMoneyMoney moT3;
  MyMoneyMoney moT4;
  MyMoneyMoney moT5;
};

#endif
